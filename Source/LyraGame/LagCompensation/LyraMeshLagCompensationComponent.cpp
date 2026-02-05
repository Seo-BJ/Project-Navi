// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraMeshLagCompensationComponent.h"
#include "Weapons/LyraWeaponDebugSettings.h"
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

// 세부 항목 정의 (CPU 연산 시간 측정용)
DECLARE_CYCLE_STAT(TEXT("LagComp - UpdateHistory"), STAT_LyraLag_UpdateHistory, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - PerformCollision"), STAT_LyraLag_PerformCollision, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - Interpolate"), STAT_LyraLag_Interpolate, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - CacheFrame"), STAT_LyraLag_CacheFrame, STATGROUP_LyraLag);
DECLARE_CYCLE_STAT(TEXT("LagComp - Rewind"), STAT_LyraLag_Rewind, STATGROUP_LyraLag);

// 메모리 카운터 (선택 사항: 배열 크기 추적용)
DECLARE_DWORD_COUNTER_STAT(TEXT("LagComp - History Frames"), STAT_LyraLag_HistoryCount, STATGROUP_LyraLag);

// 기존 로그 카테고리 재사용 또는 새로 정의 (여기선 재사용 가정하거나 임시 정의)
// DEFINE_LOG_CATEGORY(LogLagCompensation); 
// 만약 링크 에러나면 별도 카테고리 정의 필요

ULyraMeshLagCompensationComponent::ULyraMeshLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 틱 그룹을 물리가 끝난 뒤로 설정하여 최신 포즈를 캡처하도록 함
	PrimaryComponentTick.TickGroup = TG_PostPhysics; 
}

USkeletalMeshComponent* ULyraMeshLagCompensationComponent::GetSkeletalMesh(AActor* InActor) const
{
	if (!IsValid(InActor)) return nullptr;

	if (ACharacter* Character = Cast<ACharacter>(InActor))
	{
		return Character->GetMesh();
	}

	return InActor->FindComponentByClass<USkeletalMeshComponent>();
}

void ULyraMeshLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetOwner()->HasAuthority()) return;

	UpdateFrameHistory();
}

void ULyraMeshLagCompensationComponent::UpdateFrameHistory()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_UpdateHistory);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_UpdateHistory);

	FMeshFramePackage ThisFrame;
	CacheCurrentFrame(GetOwner(), ThisFrame);
	
	FrameHistory.AddHead(ThisFrame);

	// 메모리 관련 통계 업데이트 (현재 저장된 프레임 수)
	INC_DWORD_STAT_BY(STAT_LyraLag_HistoryCount, FrameHistory.Num());

	// 오래된 데이터 삭제
	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	while (HistoryLength > MaxRecordTime)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}
}

void ULyraMeshLagCompensationComponent::CacheCurrentFrame(AActor* HitActor, FMeshFramePackage& OutPackage)
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

FServerSideRewindResult ULyraMeshLagCompensationComponent::ServerSideRewind(AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_ServerSideRewind);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_Rewind);

	if (!IsValid(HitActor)) return FServerSideRewindResult();

	// 1. 적절한 과거 프레임 찾기 (보간 포함)
	FMeshFramePackage FrameToCheck = GetHitTimeFrame(HitActor, HitTime);
	
	// 2. 판정 수행
	return ConfirmHit(FrameToCheck, HitActor, TraceStart, HitLocation);
}

FMeshFramePackage ULyraMeshLagCompensationComponent::GetHitTimeFrame(AActor* HitActor, float HitTime)
{
	// HitActor에 붙은 컴포넌트를 가져와야 함 (자신의 History가 아니라 타겟의 History를 봐야 하므로)
	// 하지만 현재 구조상 ServerSideRewind는 공격자의 컴포넌트에서 호출됨.
	// 따라서 타겟 Actor의 ULyraMeshLagCompensationComponent를 찾아야 함.
	
	ULyraMeshLagCompensationComponent* TargetComp = HitActor->FindComponentByClass<ULyraMeshLagCompensationComponent>();
	if (!TargetComp || TargetComp->FrameHistory.Num() == 0)
	{
		return FMeshFramePackage();
	}

	const TDoubleLinkedList<FMeshFramePackage>& History = TargetComp->FrameHistory;
	const float OldestTime = History.GetTail()->GetValue().Time;
	const float NewestTime = History.GetHead()->GetValue().Time;

	// 범위 밖 처리
	if (OldestTime > HitTime) return FMeshFramePackage();
	if (NewestTime <= HitTime) return History.GetHead()->GetValue();

	// 탐색 (최신 -> 과거)
	auto* Node = History.GetHead();
	while (Node->GetNextNode() && Node->GetNextNode()->GetValue().Time > HitTime)
	{
		Node = Node->GetNextNode();
	}
	// Node: HitTime보다 미래인 프레임 중 가장 오래된 것 (Younger)
	// Node->Next: HitTime보다 과거인 프레임 (Older)

	if (Node->GetNextNode())
	{
		return InterpolateFrame(Node->GetNextNode()->GetValue(), Node->GetValue(), HitTime);
	}
	
	return History.GetTail()->GetValue();
}

FMeshFramePackage ULyraMeshLagCompensationComponent::InterpolateFrame(const FMeshFramePackage& Older, const FMeshFramePackage& Younger, float HitTime)
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

void ULyraMeshLagCompensationComponent::RewindFrame(AActor* HitActor, const FMeshFramePackage& Package)
{
	if (!HitActor) return;
	
	USkeletalMeshComponent* Mesh = nullptr;
	if (ACharacter* Character = Cast<ACharacter>(HitActor))
	{
		Mesh = Character->GetMesh();
	}
	else
	{
		Mesh = HitActor->FindComponentByClass<USkeletalMeshComponent>();
	}

	if (Mesh && Package.BoneTransforms.Num() > 0)
	{
		// 1. 포즈 강제 주입
		// 주의: 본 개수가 맞는지 확인 필요 (LOD 변경 등으로 다를 수 있음)
		if (Mesh->GetNumComponentSpaceTransforms() == Package.BoneTransforms.Num())
		{
			// bNeedToFlipSpace 등은 내부적으로 처리됨. 단순히 배열 덮어쓰기.
			// 하지만 protected 멤버라 직접 접근 불가할 수 있음 -> GetBoneSpaceTransforms는 const ref 반환, Set은 없음?
			// 언리얼 엔진 버전에 따라 다름. 보통 직접 Set하는 API가 없으면 아래와 같이 함:
			
			// A. PoseableMeshComponent라면 쉽지만, Character Mesh는 SkeletalMeshComponent임.
			// B. SnapshotPose 사용 (권장)
			// 하지만 여기선 Raw Data가 있으므로, 직접 접근이 어렵다면
			// 각 본별로 SetBoneTransform을 루프 돌려야 할 수도 있음 (느림)
			
			// 최적화: BoneSpaceTransforms를 직접 수정할 수 없다면, 
			// SkeletalMeshComponent에는 보통 외부에서 포즈를 주입하는 기능이 제한적임.
			// 대안: Animation Blueprint의 'Modify Bone' 등을 쓰는게 아니라
			// 물리 바디를 직접 움직여야 함.
			
			// 사실 렉 보상의 핵심은 '렌더링'이 아니라 '물리'임.
			// 따라서 UpdateBodiesFromComponentSpaceTransforms 를 쓰면 됨.
			// 이 함수는 Component Space Transform 배열을 인자로 받음.
			
			//	Mesh->UpdateBodiesFromComponentSpaceTransforms(Package.BoneTransforms, ETeleportType::ResetPhysics);
		}
	}
}

FServerSideRewindResult ULyraMeshLagCompensationComponent::ConfirmHit(const FMeshFramePackage& FrameToCheck, AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	const ULyraWeaponDebugSettings* WeaponDebugSettings = GetDefault<ULyraWeaponDebugSettings>();
	const bool bDrawDebug = WeaponDebugSettings->bDrawMeshLagCompensation;
	const float DrawDebugDuration = WeaponDebugSettings->DrawMeshLagCompensationDuration;

	UE_LOG(LogTemp, Warning, TEXT("[MeshLagComp] ConfirmHit ENTERED (bDrawDebug: %s)"), bDrawDebug ? TEXT("TRUE") : TEXT("FALSE"));
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	FHitResult OutHit;
	
	bool bHit = PerformPhysicsAssetCollision(HitActor, FrameToCheck, TraceStart, TraceEnd, OutHit);

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

	return FServerSideRewindResult { bHit, (bHit && OutHit.BoneName == FName("head")) };
}

bool ULyraMeshLagCompensationComponent::PerformPhysicsAssetCollision(const TObjectPtr<AActor> HitActor,
	const FMeshFramePackage& Frame, const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLag_PerformCollision);
	SCOPE_CYCLE_COUNTER(STAT_LyraLag_PerformCollision);

	if (!IsValid(HitActor))
	{
		return false;
	}

	// 스켈레탈 메시 컴포넌트 가져오기
	USkeletalMeshComponent* SkeletalMeshComp = GetSkeletalMesh(HitActor);
	if (!IsValid(SkeletalMeshComp) || !SkeletalMeshComp->GetSkeletalMeshAsset())
	{
		return false;
	}

	// 피직스 애셋 가져오기
	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset))
	{
		return false;
	}

	const bool bDrawDebug = GetDefault<ULyraWeaponDebugSettings>()->bDrawMeshLagCompensation;

	// 디버그 로그
	if (bDrawDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("[MeshLagComp] PerformPhysicsAssetCollision: BoneCount=%d, Start=%s, End=%s"), 
			Frame.BoneTransforms.Num(), *TraceStart.ToString(), *TraceEnd.ToString());
	}

	bool bHit = false;
	float ClosestHitDistance = FLT_MAX;
	FHitResult ClosestHit;

	FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();
	float TraceLength = (TraceEnd - TraceStart).Size();

	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (!IsValid(BodySetup)) continue;

		int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BodySetup->BoneName);
		if (BoneIndex == INDEX_NONE || !Frame.BoneTransforms.IsValidIndex(BoneIndex)) continue;

		FTransform BoneWorldTransform = Frame.BoneTransforms[BoneIndex] * Frame.MeshTransform;

		// 1. 구체(Sphere) 충돌 검사 - 더 엄격한 선분 체크
		for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
		{
			FVector SphereCenter = BoneWorldTransform.TransformPosition(SphereElem.Center);
			float SphereRadius = SphereElem.Radius * BoneWorldTransform.GetScale3D().GetMax();

			// 선분 상의 가장 가까운 점 찾기
			FVector ClosestPoint = FMath::ClosestPointOnSegment(SphereCenter, TraceStart, TraceEnd);
			float DistanceSq = FVector::DistSquared(SphereCenter, ClosestPoint);

			if (DistanceSq <= FMath::Square(SphereRadius))
			{
				float T = FVector::DotProduct(ClosestPoint - TraceStart, TraceDir);
				float Offset = FMath::Sqrt(FMath::Square(SphereRadius) - DistanceSq);
				float HitT = T - Offset; // 실제 표면 거리

				if (HitT >= 0 && HitT <= TraceLength && HitT < ClosestHitDistance)
				{
					ClosestHitDistance = HitT;
					ClosestHit.Location = TraceStart + TraceDir * HitT;
					ClosestHit.ImpactPoint = ClosestHit.Location;
					ClosestHit.Normal = (ClosestHit.Location - SphereCenter).GetSafeNormal();
					ClosestHit.ImpactNormal = ClosestHit.Normal;
					ClosestHit.Distance = HitT;
					ClosestHit.BoneName = BodySetup->BoneName;
					ClosestHit.Component = SkeletalMeshComp;
					bHit = true;
				}
			}
		}

		// 2. 박스(Box) 충돌 검사
		for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
		{
			FTransform BoxTransform(BoxElem.Rotation, BoxElem.Center);
			BoxTransform = BoxTransform * BoneWorldTransform;
            
			FVector BoxExtent = FVector(BoxElem.X, BoxElem.Y, BoxElem.Z) * 0.5f * BoxTransform.GetScale3D();
			
			FVector LocalStart = BoxTransform.InverseTransformPosition(TraceStart);
			FVector LocalEnd = BoxTransform.InverseTransformPosition(TraceEnd);
			FBox LocalBox(-BoxExtent, BoxExtent);

			float HitT;
			FVector HitLocation, HitNormal;

			// LineBoxIntersection이 더 가볍고 정확할 수 있음
			if (FMath::LineExtentBoxIntersection(LocalBox, LocalStart, LocalEnd, FVector::ZeroVector, HitLocation, HitNormal, HitT))
			{
				float WorldDistance = HitT * TraceLength;
				if (WorldDistance < ClosestHitDistance)
				{
					ClosestHitDistance = WorldDistance;
					ClosestHit.Distance = WorldDistance;
					ClosestHit.Location = BoxTransform.TransformPosition(HitLocation);
					ClosestHit.ImpactPoint = ClosestHit.Location;
					ClosestHit.Normal = BoxTransform.TransformVector(HitNormal).GetSafeNormal();
					ClosestHit.ImpactNormal = ClosestHit.Normal;
					ClosestHit.BoneName = BodySetup->BoneName;
					ClosestHit.Component = SkeletalMeshComp;
					bHit = true;
				}
			}
		}

		// 3. 캡슐(Sphyl) 충돌 검사
		for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			FTransform CapsuleTransform(SphylElem.Rotation, SphylElem.Center);
			CapsuleTransform = CapsuleTransform * BoneWorldTransform;
    
			FVector Scale = CapsuleTransform.GetScale3D();
			float Radius = SphylElem.Radius * FMath::Max(Scale.X, Scale.Y);
			float HalfHeight = SphylElem.Length * 0.5f * Scale.Z;
    
			FVector UpDirection = CapsuleTransform.GetUnitAxis(EAxis::Z);
			FVector CapsuleTop = CapsuleTransform.GetLocation() + UpDirection * HalfHeight;
			FVector CapsuleBottom = CapsuleTransform.GetLocation() - UpDirection * HalfHeight;
    
			FVector ClosestPointOnLine, ClosestPointOnCapsule;
			FMath::SegmentDistToSegmentSafe(TraceStart, TraceEnd, CapsuleBottom, CapsuleTop, ClosestPointOnLine, ClosestPointOnCapsule);
    
			float DistanceSq = FVector::DistSquared(ClosestPointOnLine, ClosestPointOnCapsule);
    
			if (DistanceSq <= FMath::Square(Radius))
			{
				float HitDistance = FVector::Dist(ClosestPointOnLine, TraceStart);
				if (HitDistance < ClosestHitDistance)
				{
					ClosestHitDistance = HitDistance;
					ClosestHit.Location = ClosestPointOnLine;
					ClosestHit.ImpactPoint = ClosestPointOnLine;
					ClosestHit.Normal = (ClosestPointOnLine - ClosestPointOnCapsule).GetSafeNormal();
					ClosestHit.ImpactNormal = ClosestHit.Normal;
					ClosestHit.Distance = HitDistance;
					ClosestHit.BoneName = BodySetup->BoneName;
					ClosestHit.Component = SkeletalMeshComp;
					bHit = true;
				}
			}
		}
	}

	if (bHit)
	{
		OutHit = ClosestHit;
	}

	return bHit;
}

void ULyraMeshLagCompensationComponent::VisualizeConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* HitActor)
{
	const float DrawDebugDuration = GetDefault<ULyraWeaponDebugSettings>()->DrawMeshLagCompensationDuration;

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

void ULyraMeshLagCompensationComponent::DrawDebugPose(const FMeshFramePackage& Package, FColor Color) const
{
	if (!IsValid(Package.HitActor)) return;

	USkeletalMeshComponent* SkeletalMeshComp = GetSkeletalMesh(Package.HitActor);
	if (!IsValid(SkeletalMeshComp)) return;

	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset)) return;
	
	const ULyraWeaponDebugSettings* Settings = GetDefault<ULyraWeaponDebugSettings>();
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

			// 요청하신 형식대로 DrawDebugCapsule 호출 (전체 높이 = HalfHeight + Radius)
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
