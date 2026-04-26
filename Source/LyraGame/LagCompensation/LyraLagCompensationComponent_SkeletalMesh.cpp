// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLagCompensationComponent_SkeletalMesh.h"
#include "LyraLagCompensationSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Development/LyraDeveloperSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicsEngine/SphereElem.h"

// 프로파일링 그룹 정의
DECLARE_STATS_GROUP(TEXT("Lyra Lag Compensation"), STATGROUP_LyraLag, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("LagComp - UpdateHistory"), STAT_LyraLag_UpdateHistory, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - HistorySearch"), STAT_LyraLag_HistorySearch, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - PerformCollision"), STAT_LyraLag_PerformCollision, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - Interpolate"), STAT_LyraLag_Interpolate, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - CacheFrame"), STAT_LyraLag_CacheFrame, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - Rewind"), STAT_LyraLag_Rewind, STATGROUP_LyraLag);

DECLARE_DWORD_COUNTER_STAT(TEXT("LagComp - History Frames"), STAT_LyraLag_HistoryCount, STATGROUP_LyraLag);
DECLARE_MEMORY_STAT(TEXT("LagComp - History Memory"), STAT_LyraLag_HistoryMemory, STATGROUP_LyraLag);

ULyraLagCompensationComponent_SkeletalMesh::ULyraLagCompensationComponent_SkeletalMesh()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 틱 그룹을 물리가 끝난 뒤로 설정하여 최신 포즈를 캡처하도록 함
	PrimaryComponentTick.TickGroup = TG_PostPhysics; 
}

USkeletalMeshComponent* ULyraLagCompensationComponent_SkeletalMesh::GetSkeletalMesh(AActor* InActor) const
{
	if (!IsValid(InActor)) return nullptr;

	if (ACharacter* Character = Cast<ACharacter>(InActor))
	{
		return Character->GetMesh();
	}

	return InActor->FindComponentByClass<USkeletalMeshComponent>();
}

void ULyraLagCompensationComponent_SkeletalMesh::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULyraLagCompensationComponent_SkeletalMesh::UpdateFrameHistory()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_UpdateHistory);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_UpdateHistory);

	FMeshFramePackage ThisFrame;
	CacheCurrentFrame(GetOwner(), ThisFrame);

	FrameHistory.EmplaceFirst(MoveTemp(ThisFrame));

	if (FrameHistory.Num() >= 2)
	{
		float HistoryLength = FrameHistory.First().Time - FrameHistory.Last().Time;
		while (HistoryLength > MaxRecordTime && FrameHistory.Num() >= 2)
		{
			FrameHistory.PopLast();
			HistoryLength = FrameHistory.First().Time - FrameHistory.Last().Time;
		}
	}

	const int32 HistoryCount = FrameHistory.Num();
	const SIZE_T HistoryMemory = FrameHistory.GetAllocatedSize();

	INC_DWORD_STAT_BY(STAT_LyraLag_HistoryCount, HistoryCount);
	SET_MEMORY_STAT(STAT_LyraLag_HistoryMemory, HistoryMemory);
}

void ULyraLagCompensationComponent_SkeletalMesh::CacheCurrentFrame(AActor* HitActor, FMeshFramePackage& OutPackage)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_CacheFrame);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_CacheFrame);

	if (!IsValid(HitActor)) return;
	
	OutPackage.Time = GetWorld()->GetTimeSeconds();
	OutPackage.HitActor = HitActor;

	USkeletalMeshComponent* Mesh = GetSkeletalMesh(HitActor);

	if (IsValid(Mesh))
	{
		// 1. 메시의 실제 월드 트랜스폼을 저장
		OutPackage.MeshTransform = Mesh->GetComponentTransform();

		// 2. Component Space Transform을 가져옴
		OutPackage.BoneTransforms = Mesh->GetComponentSpaceTransforms();

		// 서버에서 데이터가 비어있을 경우에만 포즈 강제 업데이트 (필요시 주석 해제)
		if (OutPackage.BoneTransforms.Num() == 0)
		{
			// Mesh->RefreshBoneTransforms();
			OutPackage.BoneTransforms = Mesh->GetComponentSpaceTransforms();
		}
	}
}

FServerSideRewindResult ULyraLagCompensationComponent_SkeletalMesh::ServerSideRewind(AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_ServerSideRewind);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_Rewind);

	if (!IsValid(HitActor)) return FServerSideRewindResult();

	// 1. 적절한 과거 프레임 찾기 (보간 포함)
	FMeshFramePackage FrameToCheck = GetHitTimeFrame(HitTime);

	// 2. 판정 수행
	return ConfirmHit(FrameToCheck, HitActor, TraceStart, HitLocation);
}

FMeshFramePackage ULyraLagCompensationComponent_SkeletalMesh::GetHitTimeFrame(float HitTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_HistorySearch);

	// 이 컴포넌트는 피격자(owner)에 부착되어 있으므로 자신의 FrameHistory에서 검색.

	const TDeque<FMeshFramePackage>& History = this->FrameHistory;
	if (History.Num() == 0)
	{
		return FMeshFramePackage();
	}

	const float OldestTime = History.Last().Time;
	const float NewestTime = History.First().Time;

	if (OldestTime > HitTime)
	{
		return FMeshFramePackage();
	}
	if (NewestTime <= HitTime)
	{
		return History.First();
	}

	// 탐색 (최신 -> 과거): 인덱스 0(First) 부터 인덱스 증가 방향(Last 쪽)으로
	const int32 Count = History.Num();
	int32 YoungerIdx = 0;
	while (YoungerIdx + 1 < Count && History[YoungerIdx + 1].Time > HitTime)
	{
		++YoungerIdx;
	}

	// YoungerIdx: HitTime보다 미래인 프레임 중 가장 오래된 것
	// YoungerIdx+1: HitTime보다 과거인 프레임
	if (YoungerIdx + 1 < Count)
	{
		return InterpolateFrame(History[YoungerIdx + 1], History[YoungerIdx], HitTime);
	}

	return History.Last();
}

FMeshFramePackage ULyraLagCompensationComponent_SkeletalMesh::InterpolateFrame(const FMeshFramePackage& Older, const FMeshFramePackage& Younger, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_Interpolate);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_Interpolate);

	const float DeltaTime = Younger.Time - Older.Time;
	const float Alpha = FMath::Clamp((HitTime - Older.Time) / DeltaTime, 0.f, 1.f);

	FMeshFramePackage InterpPackage;
	InterpPackage.Time = HitTime;
	InterpPackage.HitActor = Older.HitActor;

	// 액터 트랜스폼 보간
	InterpPackage.MeshTransform.Blend(Older.MeshTransform, Younger.MeshTransform, Alpha);

	int32 NumBones = Older.BoneTransforms.Num();
	InterpPackage.BoneTransforms.SetNum(NumBones);

	for (int32 i = 0; i < NumBones; ++i)
	{
		// FTransform::Blend vs Lerp
		// Blend 함수가 위치 Lerp + 회전 Slerp + 스케일 Lerp를 수행함
		InterpPackage.BoneTransforms[i].Blend(Older.BoneTransforms[i], Younger.BoneTransforms[i], Alpha);
	}

	return InterpPackage;
}

FServerSideRewindResult ULyraLagCompensationComponent_SkeletalMesh::ConfirmHit(const FMeshFramePackage& FrameToCheck, AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	const ULyraLagCompensationDeveloperSettings* LagCompSettings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const bool bDrawDebug = LagCompSettings->bDrawMeshLagCompensation;

	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	FHitResult OutHit;

	// 1단계: head 본만 검사 → 맞으면 헤드샷 확정
	static const TSet<FName> HeadFilter = { FName("head") };
	bool bHit = PerformPhysicsAssetCollision(HitActor, FrameToCheck, TraceStart, TraceEnd, HeadFilter, OutHit);
	bool bHeadShot = bHit;

	// 2단계: 못 맞췄으면 전체 본 검사 → 맞으면 바디샷
	if (!bHit)
	{
		static const TSet<FName> EmptyFilter;
		bHit = PerformPhysicsAssetCollision(HitActor, FrameToCheck, TraceStart, TraceEnd, EmptyFilter, OutHit);
	}

	if (bDrawDebug)
	{
		VisualizeConfirmHit(TraceStart, TraceEnd, bHit, OutHit, HitActor);

		// 1. 되감기 된 판정용 포즈 시각화 (성공: 초록, 실패: 빨강)
		DrawDebugPose(FrameToCheck, bHit ? FColor::Green : FColor::Red);

		// 2. 현재 서버에서의 실제 포즈 시각화 (파란색) - 리와인드 거리 확인용
		FMeshFramePackage CurrentFrame;
		CacheCurrentFrame(HitActor, CurrentFrame);
		DrawDebugPose(CurrentFrame, FColor::Blue);
	}

	return FServerSideRewindResult { bHit, bHeadShot };
}

bool ULyraLagCompensationComponent_SkeletalMesh::PerformPhysicsAssetCollision(const TObjectPtr<AActor> HitActor,
	const FMeshFramePackage& Frame, const FVector& TraceStart, const FVector& TraceEnd, const TSet<FName>& BoneFilter, FHitResult& OutHit) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_PerformCollision);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_PerformCollision);

	if (!IsValid(HitActor)) return false;

	USkeletalMeshComponent* SkeletalMeshComp = GetSkeletalMesh(HitActor);
	if (!IsValid(SkeletalMeshComp) || !SkeletalMeshComp->GetSkeletalMeshAsset()) return false;

	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset)) return false;

	const float TraceLength = (TraceEnd - TraceStart).Size();
	if (TraceLength <= KINDA_SMALL_NUMBER) return false;

	// hit 발견 시 OutHit를 채우고 즉시 true 반환하기 위한 헬퍼.
	// closest 비교 없이 첫 hit으로 종료. ServerSideRewind는 "맞췄는가" 만 필요.
	auto FillHitAndReturn = [&](const FVector& ImpactPoint, const FVector& Normal, float Distance, FName BoneName) -> bool
	{
		OutHit.Location = ImpactPoint;
		OutHit.ImpactPoint = ImpactPoint;
		OutHit.Normal = Normal;
		OutHit.ImpactNormal = Normal;
		OutHit.Distance = Distance;
		OutHit.BoneName = BoneName;
		OutHit.Component = SkeletalMeshComp;
		return true;
	};

	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (!IsValid(BodySetup)) continue;
		if (BoneFilter.Num() > 0 && !BoneFilter.Contains(BodySetup->BoneName)) continue;

		const int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BodySetup->BoneName);
		if (BoneIndex == INDEX_NONE || !Frame.BoneTransforms.IsValidIndex(BoneIndex)) continue;

		const FTransform BoneWorldTransform = Frame.BoneTransforms[BoneIndex] * Frame.MeshTransform;

		// 1. Sphere
		for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
		{
			const FVector SphereCenter = BoneWorldTransform.TransformPosition(SphereElem.Center);
			const float SphereRadius = SphereElem.Radius * BoneWorldTransform.GetScale3D().GetMax();
			const FVector ClosestPoint = FMath::ClosestPointOnSegment(SphereCenter, TraceStart, TraceEnd);

			if (FVector::DistSquared(SphereCenter, ClosestPoint) <= FMath::Square(SphereRadius))
			{
				const float Distance = FVector::Dist(TraceStart, ClosestPoint);
				return FillHitAndReturn(ClosestPoint, (ClosestPoint - SphereCenter).GetSafeNormal(), Distance, BodySetup->BoneName);
			}
		}

		// 2. Box
		for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
		{
			FTransform BoxTransform(BoxElem.Rotation, BoxElem.Center);
			BoxTransform = BoxTransform * BoneWorldTransform;

			const FVector BoxExtent = FVector(BoxElem.X, BoxElem.Y, BoxElem.Z) * 0.5f * BoxTransform.GetScale3D();
			const FVector LocalStart = BoxTransform.InverseTransformPosition(TraceStart);
			const FVector LocalEnd = BoxTransform.InverseTransformPosition(TraceEnd);
			const FBox LocalBox(-BoxExtent, BoxExtent);

			float HitT;
			FVector HitLocal, HitNormalLocal;
			if (FMath::LineExtentBoxIntersection(LocalBox, LocalStart, LocalEnd, FVector::ZeroVector, HitLocal, HitNormalLocal, HitT))
			{
				const FVector WorldImpact = BoxTransform.TransformPosition(HitLocal);
				const FVector WorldNormal = BoxTransform.TransformVector(HitNormalLocal).GetSafeNormal();
				return FillHitAndReturn(WorldImpact, WorldNormal, HitT * TraceLength, BodySetup->BoneName);
			}
		}

		// 3. Sphyl: 두 선분(trace ↔ capsule 축) 최단거리 < Radius 이면 hit
		for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			FTransform CapsuleTransform(SphylElem.Rotation, SphylElem.Center);
			CapsuleTransform = CapsuleTransform * BoneWorldTransform;

			const FVector Scale = CapsuleTransform.GetScale3D();
			const float Radius = SphylElem.Radius * FMath::Max(Scale.X, Scale.Y);
			const float HalfHeight = SphylElem.Length * 0.5f * Scale.Z;

			const FVector UpDir = CapsuleTransform.GetUnitAxis(EAxis::Z);
			const FVector CapsuleTop = CapsuleTransform.GetLocation() + UpDir * HalfHeight;
			const FVector CapsuleBottom = CapsuleTransform.GetLocation() - UpDir * HalfHeight;

			FVector ClosestPointOnLine, ClosestPointOnCapsule;
			FMath::SegmentDistToSegmentSafe(TraceStart, TraceEnd, CapsuleBottom, CapsuleTop, ClosestPointOnLine, ClosestPointOnCapsule);

			if (FVector::DistSquared(ClosestPointOnLine, ClosestPointOnCapsule) <= FMath::Square(Radius))
			{
				FVector SurfaceNormal = (ClosestPointOnLine - ClosestPointOnCapsule).GetSafeNormal();
				if (SurfaceNormal.IsNearlyZero())
				{
					SurfaceNormal = -((TraceEnd - TraceStart).GetSafeNormal());
				}
				const FVector ImpactPointOnSurface = ClosestPointOnCapsule + SurfaceNormal * Radius;
				const float Distance = FVector::Dist(TraceStart, ImpactPointOnSurface);

#if ENABLE_DRAW_DEBUG
				// [임시] 원래 점(선분 위, 내부일 수 있음) vs 보정된 표면점 비교
				const float DebugDuration = 5.0f;
				DrawDebugSphere(GetWorld(), ClosestPointOnLine, 3.0f, 12, FColor::Yellow, false, DebugDuration, 0, 0.5f); // 보정 전 (선분 위)
				DrawDebugPoint(GetWorld(), ClosestPointOnCapsule, 8.0f, FColor::Cyan, false, DebugDuration);    // 캡슐 중심선 위
				DrawDebugSphere(GetWorld(), ImpactPointOnSurface, 3.0f, 12, FColor::Green, false, DebugDuration, 0, 0.5f); // 보정 후 (표면)
				DrawDebugLine(GetWorld(), ClosestPointOnLine, ImpactPointOnSurface, FColor::White, false, DebugDuration, 0, 1.0f);
				DrawDebugDirectionalArrow(GetWorld(), ImpactPointOnSurface, ImpactPointOnSurface + SurfaceNormal * 20.0f,
					15.0f, FColor::Magenta, false, DebugDuration, 0, 1.5f);
#endif

				return FillHitAndReturn(ImpactPointOnSurface, SurfaceNormal, Distance, BodySetup->BoneName);
			}
		}
	}

	return false;
}

#if ENABLE_DRAW_DEBUG
void ULyraLagCompensationComponent_SkeletalMesh::VisualizeConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, const AActor* HitActor) const
{
	const float DrawDebugDuration = GetDefault<ULyraLagCompensationDeveloperSettings>()->DrawMeshLagCompensationDuration;

	if (bSuccess)
	{
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Blue, false, DrawDebugDuration);
		DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Blue, false, DrawDebugDuration, 0, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("[MeshLagComp] Hit SUCCESS: %s (Bone: %s) Duration: %.1f"), *HitActor->GetName(), *HitResult.BoneName.ToString(), DrawDebugDuration);
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, DrawDebugDuration, 0, 2.0f);
		UE_LOG(LogTemp, Warning, TEXT("[MeshLagComp] Hit FAILED: %s Start: %s End: %s Duration: %.1f"), *HitActor->GetName(), *Start.ToString(), *End.ToString(), DrawDebugDuration);
	}
}

void ULyraLagCompensationComponent_SkeletalMesh::DrawDebugPose(const FMeshFramePackage& Package, FColor Color) const
{
	if (!IsValid(Package.HitActor)) return;

	USkeletalMeshComponent* SkeletalMeshComp = GetSkeletalMesh(Package.HitActor);
	if (!IsValid(SkeletalMeshComp)) return;

	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset)) return;
	
	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float LifeTime = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;
	
	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (!IsValid(BodySetup)) continue;

		int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BodySetup->BoneName);
		if (BoneIndex == INDEX_NONE || !Package.BoneTransforms.IsValidIndex(BoneIndex)) continue;

		// 과거 시점의 본 월드 트랜스폼 계산
		FTransform BoneWorldTransform = Package.BoneTransforms[BoneIndex] * Package.MeshTransform;

		// 1. 캡슐(Capsule/Sphyl) 시각화
		for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			FTransform CapsuleTransform(SphylElem.Rotation, SphylElem.Center);
			CapsuleTransform = CapsuleTransform * BoneWorldTransform;

			float Radius = SphylElem.Radius * CapsuleTransform.GetScale3D().GetMax();
			float HalfHeight = SphylElem.Length * 0.5f * CapsuleTransform.GetScale3D().Z;
			
			DrawDebugCapsule(GetWorld(), CapsuleTransform.GetLocation(), HalfHeight + Radius, Radius, CapsuleTransform.GetRotation(), Color, false, LifeTime, 0, 0.5f);
		}

		// 2. 구체(Sphere) 시각화
		for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
		{
			FVector SphereCenter = BoneWorldTransform.TransformPosition(SphereElem.Center);
			float Radius = SphereElem.Radius * BoneWorldTransform.GetScale3D().GetMax();

			DrawDebugSphere(GetWorld(), SphereCenter, Radius, 12, Color, false, LifeTime, 0, 0.5f);
		}

		// 3. 박스(Box) 시각화
		for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
		{
			FTransform BoxTransform(BoxElem.Rotation, BoxElem.Center);
			BoxTransform = BoxTransform * BoneWorldTransform;
			FVector BoxExtent = FVector(BoxElem.X, BoxElem.Y, BoxElem.Z) * 0.5f * BoxTransform.GetScale3D();

			DrawDebugBox(GetWorld(), BoxTransform.GetLocation(), BoxExtent, BoxTransform.GetRotation(), Color, false, LifeTime, 0, 0.5f);
		}
	}
}
#endif
