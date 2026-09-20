// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/ServerSideRewind/LyraServerSideRewindSubsystem.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "LagCompensation/ILagCompensationTarget.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_AnimNode.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_BoxComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_SkeletalMesh.h"
#include "Misc/ScopeExit.h"
#include "Physics/LyraCollisionChannels.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicsEngine/SphereElem.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraServerSideRewind, Log, All);

DECLARE_STATS_GROUP(TEXT("Lyra Server Side Rewind"), STATGROUP_LyraServerSideRewind, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("SSR - Dispatch"), STAT_LyraSSR_Dispatch, STATGROUP_LyraServerSideRewind);
DECLARE_CYCLE_STAT(TEXT("SSR - BoxConfirmHit"), STAT_LyraSSR_BoxConfirmHit, STATGROUP_LyraServerSideRewind);
DECLARE_CYCLE_STAT(TEXT("SSR - MeshConfirmHit"), STAT_LyraSSR_MeshConfirmHit, STATGROUP_LyraServerSideRewind);
DECLARE_CYCLE_STAT(TEXT("SSR - MeshPhysicsAssetCollision"), STAT_LyraSSR_MeshPhysicsAssetCollision, STATGROUP_LyraServerSideRewind);

namespace
{
	/**
	 * 바디가 실제로 쓰는 단순 충돌 물리 재질.
	 * 엔진 우선순위(바디 인스턴스 오버라이드 > 컴포넌트 오버라이드 > BodySetup)를 따르는 BodyInstance 값을 우선 쓰고,
	 * 바디 인스턴스가 없을 때(물리 상태 미생성 등)만 PA의 BodySetup 재질을 쓴다. 클라 트레이스의 HitResult.PhysMaterial과 같은 기준이다.
	 */
	UPhysicalMaterial* ResolveBodyPhysicalMaterial(USkeletalMeshComponent* SkeletalMeshComp, const USkeletalBodySetup* BodySetup)
	{
		if (!IsValid(BodySetup))
		{
			return nullptr;
		}
		if (IsValid(SkeletalMeshComp))
		{
			if (const FBodyInstance* BodyInstance = SkeletalMeshComp->GetBodyInstance(BodySetup->BoneName))
			{
				return BodyInstance->GetSimplePhysicalMaterial();
			}
		}
		return BodySetup->GetPhysMaterial();
	}

	/** PhysicsAsset 해석 판정(Mesh/AnimNode 방식) 결과를 SSR 결과로 옮긴다. */
	FServerSideRewindResult MakeServerSideRewindResultFromPhysicsHit(bool bHit, const FHitResult& Hit)
	{
		FServerSideRewindResult Result;
		Result.bHitConfirmed = bHit;
		if (bHit)
		{
			Result.ConfirmedHit = Hit;
		}
		return Result;
	}

	bool ShouldDrawBoxLagCompensation()
	{
		const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
		return IsValid(Settings) && Settings->bDrawBoxLagCompensation;
	}

	float GetBoxLagCompensationDrawDuration()
	{
		const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
		return IsValid(Settings) ? Settings->DrawBoxLagCompensationDuration : 4.0f;
	}
}

bool ULyraServerSideRewindSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return Super::ShouldCreateSubsystem(Outer)
		&& IsValid(World)
		&& DoesSupportWorldType(World->WorldType)
		&& World->GetNetMode() != NM_Client;
}

bool ULyraServerSideRewindSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ServerSideRewind(const FLyraServerSideRewindRequest& Request)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_RewindTotal);
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_Dispatch);

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World) && World->GetNetMode() != NM_Client,
		TEXT("ServerSideRewind must run in an authority World.")))
	{
		return {};
	}

	AActor* TargetActor = Request.TargetActor.Get();
	if (!IsValid(TargetActor))
	{
		return {};
	}
	if (!ensureMsgf(TargetActor->HasAuthority(),
		TEXT("ServerSideRewind target '%s' must have authority."), *GetNameSafe(TargetActor)))
	{
		return {};
	}
	if (!ensureMsgf(TargetActor->GetWorld() == World,
		TEXT("ServerSideRewind target '%s' belongs to a different World."), *GetNameSafe(TargetActor)))
	{
		return {};
	}

	ULyraSnapShotComponent* SnapShotComponent = ResolveSnapShotComponent(TargetActor);
	if (!IsValid(SnapShotComponent))
	{
		return {};
	}

	switch (SnapShotComponent->GetSnapShotMode())
	{
	case ELyraSnapShotMode::BoxComponent:
		return ServerSideRewindBoxComponent(CastChecked<ULyraSnapShotComponent_BoxComponent>(SnapShotComponent), Request);
	case ELyraSnapShotMode::SkeletalMesh:
		return ServerSideRewindSkeletalMesh(CastChecked<ULyraSnapShotComponent_SkeletalMesh>(SnapShotComponent), Request);
	case ELyraSnapShotMode::AnimNode:
		return ServerSideRewindAnimNode(CastChecked<ULyraSnapShotComponent_AnimNode>(SnapShotComponent), Request);
	default:
		return {};
	}
}

ULyraSnapShotComponent* ULyraServerSideRewindSubsystem::ResolveSnapShotComponent(AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return nullptr;
	}

	TInlineComponentArray<ULyraSnapShotComponent*, 4> SnapShotComponents(TargetActor);
	return SelectUniqueActiveLyraSnapShotComponent(SnapShotComponents);
}

ULyraSnapShotComponent* SelectUniqueActiveLyraSnapShotComponent(TConstArrayView<ULyraSnapShotComponent*> Components)
{
	ULyraSnapShotComponent* ActiveComponent = nullptr;
	for (ULyraSnapShotComponent* Candidate : Components)
	{
		if (IsValid(Candidate) && Candidate->IsSSRActive())
		{
			if (ActiveComponent != nullptr)
			{
				return nullptr;
			}
			ActiveComponent = Candidate;
		}
	}
	return ActiveComponent;
}

ULyraServerSideRewindSubsystem::FBoxRewindState ULyraServerSideRewindSubsystem::CaptureBoxRewindState(
	TConstArrayView<UBoxComponent*> HitBoxes,
	TConstArrayView<UPrimitiveComponent*> NormalCollisionComponents)
{
	FBoxRewindState RewindState;
	RewindState.BoxStates.Reserve(HitBoxes.Num());
	for (UBoxComponent* HitBox : HitBoxes)
	{
		if (!IsValid(HitBox))
		{
			continue;
		}

		FBoxRestoreState& State = RewindState.BoxStates.AddDefaulted_GetRef();
		State.Component = HitBox;
		State.WorldTransform = HitBox->GetComponentTransform();
		State.UnscaledBoxExtent = HitBox->GetUnscaledBoxExtent();
		State.CollisionEnabled = HitBox->GetCollisionEnabled();
		State.LagCompensationResponse = HitBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox);
	}

	RewindState.PrimitiveStates.Reserve(NormalCollisionComponents.Num());
	for (UPrimitiveComponent* Component : NormalCollisionComponents)
	{
		if (!IsValid(Component))
		{
			continue;
		}

		FPrimitiveCollisionRestoreState& State = RewindState.PrimitiveStates.AddDefaulted_GetRef();
		State.Component = Component;
		State.CollisionEnabled = Component->GetCollisionEnabled();
	}

	return RewindState;
}

void ULyraServerSideRewindSubsystem::SuspendBoxRewindCollision(const FBoxRewindState& RewindState)
{
	for (const FBoxRestoreState& State : RewindState.BoxStates)
	{
		if (UBoxComponent* HitBox = State.Component.Get(); IsValid(HitBox))
		{
			HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HitBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Ignore);
		}
	}

	for (const FPrimitiveCollisionRestoreState& State : RewindState.PrimitiveStates)
	{
		if (UPrimitiveComponent* Component = State.Component.Get(); IsValid(Component))
		{
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULyraServerSideRewindSubsystem::EnableOnlyBoxesForTrace(
	const FBoxRewindState& RewindState,
	TConstArrayView<UBoxComponent*> BoxesToEnable)
{
	for (const FBoxRestoreState& State : RewindState.BoxStates)
	{
		if (UBoxComponent* HitBox = State.Component.Get(); IsValid(HitBox))
		{
			HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HitBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Ignore);
		}
	}

	for (UBoxComponent* HitBox : BoxesToEnable)
	{
		if (IsValid(HitBox))
		{
			HitBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Block);
			HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}

bool ULyraServerSideRewindSubsystem::RestoreBoxRewindState(const FBoxRewindState& RewindState)
{
	bool bSucceeded = true;
	for (const FBoxRestoreState& State : RewindState.BoxStates)
	{
		UBoxComponent* HitBox = State.Component.Get();
		if (!IsValid(HitBox))
		{
			bSucceeded = false;
			continue;
		}

		HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitBox->SetWorldTransform(State.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
		HitBox->SetBoxExtent(State.UnscaledBoxExtent, false);
		HitBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, State.LagCompensationResponse);
		HitBox->SetCollisionEnabled(State.CollisionEnabled);

		bSucceeded &= HitBox->GetComponentTransform().Equals(State.WorldTransform);
		bSucceeded &= HitBox->GetUnscaledBoxExtent().Equals(State.UnscaledBoxExtent);
		bSucceeded &= HitBox->GetCollisionEnabled() == State.CollisionEnabled;
		bSucceeded &= HitBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == State.LagCompensationResponse;
	}

	for (const FPrimitiveCollisionRestoreState& State : RewindState.PrimitiveStates)
	{
		UPrimitiveComponent* Component = State.Component.Get();
		if (!IsValid(Component))
		{
			bSucceeded = false;
			continue;
		}

		Component->SetCollisionEnabled(State.CollisionEnabled);
		bSucceeded &= Component->GetCollisionEnabled() == State.CollisionEnabled;
	}

	return bSucceeded;
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ServerSideRewindBoxComponent(ULyraSnapShotComponent_BoxComponent* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	FServerSideRewindResult Result;
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return Result;
	}

	FLyraBoxSnapShotFrame FrameToCheck;
	Result.Diagnostics.bSnapshotLookupSucceeded = SnapShotComponent->TryGetSnapShotAtTime(Request.HitTime, FrameToCheck);
	if (!Result.Diagnostics.bSnapshotLookupSucceeded)
	{
		return Result;
	}

	FrameToCheck.HitActor = Request.TargetActor.Get();
	FServerSideRewindResult HitResult = ConfirmBoxHit(SnapShotComponent, FrameToCheck, Request);
	HitResult.Diagnostics.bSnapshotLookupSucceeded = Result.Diagnostics.bSnapshotLookupSucceeded;
	HitResult.Diagnostics.bUsedInterpolation = Result.Diagnostics.bUsedInterpolation;
	return HitResult;
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ConfirmBoxHit(
	ULyraSnapShotComponent_BoxComponent* SnapShotComponent,
	const FLyraBoxSnapShotFrame& FrameToCheck,
	const FLyraServerSideRewindRequest& Request
	) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_BoxConfirmHit);

	AActor* TargetActor = Request.TargetActor.Get();
	ILagCompensationTarget* Target = IsValid(TargetActor) ? Cast<ILagCompensationTarget>(TargetActor) : nullptr;
	UWorld* World = GetWorld();
	if (!IsValid(SnapShotComponent) || !Target || !IsValid(World))
	{
		return FServerSideRewindResult();
	}

	const TMap<FName, TObjectPtr<UBoxComponent>>& HitBoxes = Target->GetHitCollisionBoxes();
	TArray<UBoxComponent*> AllBoxes;
	AllBoxes.Reserve(HitBoxes.Num());
	for (const TPair<FName, TObjectPtr<UBoxComponent>>& Pair : HitBoxes)
	{
		if (IsValid(Pair.Value.Get()))
		{
			AllBoxes.Add(Pair.Value.Get());
		}
	}
	if (AllBoxes.IsEmpty())
	{
		return FServerSideRewindResult();
	}

	TArray<UPrimitiveComponent*> NormalCollisionComponents;
	if (ACharacter* HitCharacter = Cast<ACharacter>(TargetActor); IsValid(HitCharacter))
	{
		if (USkeletalMeshComponent* Mesh = HitCharacter->GetMesh(); IsValid(Mesh))
		{
			NormalCollisionComponents.Add(Mesh);
		}
		if (UCapsuleComponent* Capsule = HitCharacter->GetCapsuleComponent(); IsValid(Capsule))
		{
			NormalCollisionComponents.Add(Capsule);
		}
	}

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(LyraSSR_BoxConfirmHit), true);
	TraceParams.bReturnPhysicalMaterial = true;
	if (AActor* InstigatorActor = Request.InstigatorActor.Get(); IsValid(InstigatorActor))
	{
		// 사격 시작점이 쏜 사람 몸 안쪽일 수 있으므로 자기 자신과 부착 액터(무기 등)는 무시한다. (클라 WeaponTrace와 같은 조건)
		TraceParams.AddIgnoredActor(InstigatorActor);
		TArray<AActor*> AttachedActors;
		InstigatorActor->GetAttachedActors(AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
	const FVector TraceEnd = Request.TraceStart + (Request.HitLocation - Request.TraceStart) * 1.25f;
	FServerSideRewindResult Result;
	bool bRestoreSucceeded = true;
	{
		const FBoxRewindState RewindState = CaptureBoxRewindState(AllBoxes, NormalCollisionComponents);
		ON_SCOPE_EXIT
		{
			bRestoreSucceeded = RestoreBoxRewindState(RewindState);
		};

		SuspendBoxRewindCollision(RewindState);
		SnapShotComponent->ApplySnapShot(TargetActor, FrameToCheck);

#if ENABLE_DRAW_DEBUG
		if (ShouldDrawBoxLagCompensation())
		{
			const float DrawDuration = GetBoxLagCompensationDrawDuration();
			DrawDebugLine(World, Request.TraceStart, TraceEnd, FColor::Red, false, DrawDuration, 0, 1.0f);
			DrawDebugPoint(World, Request.HitLocation, 10.0f, FColor::Cyan, false, DrawDuration);
		}
#endif

		FHitResult ConfirmHitResult;
		bool bHitSuccess = false;

		// 대상의 히트 박스를 모두 켜고 한 번만 트레이스해, 선분이 가장 먼저 닿은 박스를 피격 부위로 쓴다.
		// (head 박스만 먼저 보던 방식은 몸을 지나 머리에 닿아도 헤드샷이 되는 편향이 있었다)
		EnableOnlyBoxesForTrace(RewindState, AllBoxes);
		if (PerformBoxHitCheck(Request.TraceStart, TraceEnd, TraceParams, ConfirmHitResult))
		{
			// HitBox 채널은 다른 캐릭터 메시 등도 Block하므로, 가장 먼저 맞은 것이 대상의 박스일 때만 인정한다.
			UBoxComponent* HitBox = Cast<UBoxComponent>(ConfirmHitResult.GetComponent());
			if (IsValid(HitBox) && AllBoxes.Contains(HitBox))
			{
				bHitSuccess = true;

				// 트레이스 결과를 그대로 서버 HitResult로 쓴다. 부위는 박스의 물리 재질 태그로 정해진다(TraceParams.bReturnPhysicalMaterial).
				// 박스 트레이스는 BoneName이 비어 있으므로 디버깅용으로 히트 박스 이름을 넣는다.
				Result.ConfirmedHit = ConfirmHitResult;
				for (const TPair<FName, TObjectPtr<UBoxComponent>>& Pair : HitBoxes)
				{
					if (Pair.Value.Get() == HitBox)
					{
						Result.ConfirmedHit.BoneName = Pair.Key;
						break;
					}
				}
			}
		}

#if ENABLE_DRAW_DEBUG
		if (ShouldDrawBoxLagCompensation())
		{
			VisualizeBoxConfirmHit(Request.TraceStart, TraceEnd, bHitSuccess, ConfirmHitResult, TargetActor);
		}
#endif

		Result.bHitConfirmed = bHitSuccess;
	}

	Result.Diagnostics.bStateRestoreSucceeded = bRestoreSucceeded;
	return Result;
}

bool ULyraServerSideRewindSubsystem::PerformBoxHitCheck(const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	return World->LineTraceSingleByChannel(OutHit, Start, End, Lyra_TraceChannel_LagCompensation_HitBox, Params);
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ServerSideRewindSkeletalMesh(ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	FServerSideRewindResult Result;
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return Result;
	}

	FLyraMeshSnapShotFrame FrameToCheck;
	if (!SnapShotComponent->TryGetSnapShotAtTime(Request.HitTime, FrameToCheck, &Result.Diagnostics))
	{
		return Result;
	}

	FrameToCheck.HitActor = Request.TargetActor.Get();
	FServerSideRewindResult HitResult = ConfirmMeshHit(SnapShotComponent, FrameToCheck, Request);
	HitResult.Diagnostics = Result.Diagnostics;
	return HitResult;
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ConfirmMeshHit(
	ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent,
	const FLyraMeshSnapShotFrame& FrameToCheck,
	const FLyraServerSideRewindRequest& Request
	) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_MeshConfirmHit);

	AActor* TargetActor = Request.TargetActor.Get();
	if (!IsValid(SnapShotComponent) || !IsValid(TargetActor))
	{
		return FServerSideRewindResult();
	}

	const FVector TraceEnd = Request.TraceStart + (Request.HitLocation - Request.TraceStart) * 1.25f;
	FHitResult OutHit;

	// 모든 바디를 한 번에 검사해 가장 가까운 바디를 피격 부위로 쓴다.
	static const TSet<FName> NoBoneFilter;
	const bool bHit = PerformPhysicsAssetCollision(SnapShotComponent, TargetActor, FrameToCheck, Request.TraceStart, TraceEnd, NoBoneFilter, OutHit);

#if ENABLE_DRAW_DEBUG
	const ULyraLagCompensationDeveloperSettings* LagCompSettings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	if (IsValid(LagCompSettings) && LagCompSettings->bDrawMeshLagCompensation)
	{
		VisualizeMeshConfirmHit(Request.TraceStart, TraceEnd, bHit, OutHit, TargetActor);
		SnapShotComponent->DrawDebugPose(FrameToCheck, bHit ? FColor::Green : FColor::Red);

		FLyraMeshSnapShotFrame CurrentFrame;
		SnapShotComponent->CaptureCurrentSnapShot(TargetActor, CurrentFrame);
		SnapShotComponent->DrawDebugPose(CurrentFrame, FColor::Blue);
	}
#endif

	return MakeServerSideRewindResultFromPhysicsHit(bHit, OutHit);
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ServerSideRewindAnimNode(ULyraSnapShotComponent_AnimNode* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	FServerSideRewindResult Result;
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return Result;
	}

	FAnimSSRRewindPose RewindPose;
	if (!SnapShotComponent->EvaluatePoseAtServerTime(Request.HitTime, RewindPose, &Result.Diagnostics))
	{
		return Result;
	}

	FServerSideRewindResult HitResult = ConfirmAnimNodeHit(SnapShotComponent, RewindPose, Request);
	HitResult.Diagnostics = Result.Diagnostics;
	return HitResult;
}

FServerSideRewindResult ULyraServerSideRewindSubsystem::ConfirmAnimNodeHit(
	ULyraSnapShotComponent_AnimNode* SnapShotComponent,
	const FAnimSSRRewindPose& RewindPose,
	const FLyraServerSideRewindRequest& Request
	) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_MeshConfirmHit);

	AActor* TargetActor = Request.TargetActor.Get();
	if (!IsValid(SnapShotComponent) || !IsValid(TargetActor))
	{
		return FServerSideRewindResult();
	}

	USkeletalMeshComponent* SkeletalMeshComp = SnapShotComponent->GetSkeletalMesh(TargetActor);
	if (!IsValid(SkeletalMeshComp))
	{
		return FServerSideRewindResult();
	}

	const FVector TraceEnd = Request.TraceStart + (Request.HitLocation - Request.TraceStart) * 1.25f;
	FHitResult OutHit;

	// 모든 바디를 한 번에 검사해 가장 가까운 바디를 피격 부위로 쓴다.
	static const TSet<FName> NoBoneFilter;
	const bool bHit = PerformPhysicsAssetCollision(SkeletalMeshComp, RewindPose.MeshComponentTransform, RewindPose.ComponentSpaceTransforms, Request.TraceStart, TraceEnd, NoBoneFilter, OutHit);

#if ENABLE_DRAW_DEBUG
	const ULyraLagCompensationDeveloperSettings* LagCompSettings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	if (IsValid(LagCompSettings) && LagCompSettings->bDrawMeshLagCompensation)
	{
		VisualizeMeshConfirmHit(Request.TraceStart, TraceEnd, bHit, OutHit, TargetActor);
		SnapShotComponent->DrawDebugPose(RewindPose, bHit ? FColor::Green : FColor::Red);
		SnapShotComponent->DrawCurrentDebugPose(FColor::Blue);
	}
#endif

	return MakeServerSideRewindResultFromPhysicsHit(bHit, OutHit);
}

bool ULyraServerSideRewindSubsystem::PerformPhysicsAssetCollision(
	ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent,
	AActor* TargetActor,
	const FLyraMeshSnapShotFrame& Frame,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	const TSet<FName>& BoneFilter,
	FHitResult& OutHit
	) const
{
	if (!IsValid(SnapShotComponent) || !IsValid(TargetActor))
	{
		return false;
	}

	USkeletalMeshComponent* SkeletalMeshComp = SnapShotComponent->GetSkeletalMesh(TargetActor);
	if (!IsValid(SkeletalMeshComp) || !IsValid(SkeletalMeshComp->GetSkeletalMeshAsset()))
	{
		return false;
	}

	return PerformPhysicsAssetCollision(SkeletalMeshComp, Frame.MeshTransform, Frame.BoneTransforms, TraceStart, TraceEnd, BoneFilter, OutHit);
}

bool ULyraServerSideRewindSubsystem::PerformPhysicsAssetCollision(
	USkeletalMeshComponent* SkeletalMeshComp,
	const FTransform& MeshTransform,
	const TArray<FTransform>& ComponentSpaceTransforms,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	const TSet<FName>& BoneFilter,
	FHitResult& OutHit
	) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_Rewind_PhysicsCollision);
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_MeshPhysicsAssetCollision);

	if (!IsValid(SkeletalMeshComp) || !IsValid(SkeletalMeshComp->GetSkeletalMeshAsset()))
	{
		return false;
	}

	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset))
	{
		return false;
	}

	const float TraceLength = (TraceEnd - TraceStart).Size();
	if (TraceLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector TraceDir = (TraceEnd - TraceStart) / TraceLength;

	// 처음 교차한 바디에서 멈추지 않고, 시작점에서 가장 가까운 교차를 고른다.
	// (PhysicsAsset의 바디 순서는 선분 방향과 무관하므로 첫 교차가 실제로 먼저 맞은 바디라는 보장이 없다)
	const USkeletalBodySetup* NearestBodySetup = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	auto ConsiderHit = [&](const FVector& ImpactPoint, const FVector& Normal, float Distance, const USkeletalBodySetup* HitBodySetup)
	{
		if (Distance >= NearestDistance)
		{
			return;
		}
		NearestDistance = Distance;
		NearestBodySetup = HitBodySetup;
		OutHit.Location = ImpactPoint;
		OutHit.ImpactPoint = ImpactPoint;
		OutHit.Normal = Normal;
		OutHit.ImpactNormal = Normal;
		OutHit.Distance = Distance;
		OutHit.BoneName = HitBodySetup->BoneName;
		OutHit.Component = SkeletalMeshComp;
	};

	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (!IsValid(BodySetup))
		{
			continue;
		}
		if (BoneFilter.Num() > 0 && !BoneFilter.Contains(BodySetup->BoneName))
		{
			continue;
		}

		const int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BodySetup->BoneName);
		if (BoneIndex == INDEX_NONE || !ComponentSpaceTransforms.IsValidIndex(BoneIndex))
		{
			continue;
		}

		const FTransform BoneWorldTransform = ComponentSpaceTransforms[BoneIndex] * MeshTransform;

		for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
		{
			const FVector SphereCenter = BoneWorldTransform.TransformPosition(SphereElem.Center);
			const float SphereRadius = SphereElem.Radius * BoneWorldTransform.GetScale3D().GetMax();

			// 선분-구 교차: 구에 들어가는 지점(Entry)을 명중점으로 쓴다. 시작점이 구 안이면 시작점.
			const FVector ToCenter = SphereCenter - TraceStart;
			const float Projection = FVector::DotProduct(ToCenter, TraceDir);
			const float DistSqToLine = ToCenter.SizeSquared() - FMath::Square(Projection);
			const float RadiusSq = FMath::Square(SphereRadius);
			if (DistSqToLine > RadiusSq)
			{
				continue;
			}
			const float HalfChord = FMath::Sqrt(RadiusSq - DistSqToLine);
			if (Projection + HalfChord < 0.0f || Projection - HalfChord > TraceLength)
			{
				continue;
			}
			const float EntryDistance = FMath::Max(Projection - HalfChord, 0.0f);
			const FVector EntryPoint = TraceStart + TraceDir * EntryDistance;
			ConsiderHit(EntryPoint, (EntryPoint - SphereCenter).GetSafeNormal(), EntryDistance, BodySetup);
		}

		for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
		{
			FTransform BoxTransform(BoxElem.Rotation, BoxElem.Center);
			BoxTransform = BoxTransform * BoneWorldTransform;

			const FVector BoxExtent = FVector(BoxElem.X, BoxElem.Y, BoxElem.Z) * 0.5f * BoxTransform.GetScale3D();
			const FVector LocalStart = BoxTransform.InverseTransformPosition(TraceStart);
			const FVector LocalEnd = BoxTransform.InverseTransformPosition(TraceEnd);
			const FBox LocalBox(-BoxExtent, BoxExtent);

			float HitT = 0.0f;
			FVector HitLocal = FVector::ZeroVector;
			FVector HitNormalLocal = FVector::ZeroVector;
			if (FMath::LineExtentBoxIntersection(LocalBox, LocalStart, LocalEnd, FVector::ZeroVector, HitLocal, HitNormalLocal, HitT))
			{
				const FVector WorldImpact = BoxTransform.TransformPosition(HitLocal);
				const FVector WorldNormal = BoxTransform.TransformVector(HitNormalLocal).GetSafeNormal();
				ConsiderHit(WorldImpact, WorldNormal, HitT * TraceLength, BodySetup);
			}
		}

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

			FVector ClosestPointOnLine;
			FVector ClosestPointOnCapsule;
			FMath::SegmentDistToSegmentSafe(TraceStart, TraceEnd, CapsuleBottom, CapsuleTop, ClosestPointOnLine, ClosestPointOnCapsule);

			if (FVector::DistSquared(ClosestPointOnLine, ClosestPointOnCapsule) <= FMath::Square(Radius))
			{
				FVector SurfaceNormal = (ClosestPointOnLine - ClosestPointOnCapsule).GetSafeNormal();
				if (SurfaceNormal.IsNearlyZero())
				{
					SurfaceNormal = -((TraceEnd - TraceStart).GetSafeNormal());
				}

				// 캡슐은 진입점 대신 최근접점 기준 표면점으로 근사한다(가장 가까운 바디 비교용으로 충분).
				const FVector ImpactPointOnSurface = ClosestPointOnCapsule + SurfaceNormal * Radius;
				const float Distance = FVector::Dist(TraceStart, ImpactPointOnSurface);
				ConsiderHit(ImpactPointOnSurface, SurfaceNormal, Distance, BodySetup);
			}
		}
	}

	if (NearestBodySetup == nullptr)
	{
		return false;
	}

	// 서버 HitResult로 그대로 쓸 수 있도록 나머지 필드를 채운다.
	// (Component만 넣으면 GetActor()가 비어 있으므로 HitObjectHandle도 지정한다)
	OutHit.bBlockingHit = true;
	OutHit.HitObjectHandle = FActorInstanceHandle(SkeletalMeshComp->GetOwner());
	OutHit.TraceStart = TraceStart;
	OutHit.TraceEnd = TraceEnd;
	OutHit.Time = NearestDistance / TraceLength;
	OutHit.PhysMaterial = ResolveBodyPhysicalMaterial(SkeletalMeshComp, NearestBodySetup);
	return true;
}

#if ENABLE_DRAW_DEBUG
void ULyraServerSideRewindSubsystem::VisualizeBoxConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* TargetActor) const
{
	const float DrawDuration = GetBoxLagCompensationDrawDuration();
	if (bSuccess)
	{
		DrawDebugBoxHitResult(HitResult, true);
		UE_LOG(LogLyraServerSideRewind, Log, TEXT("Box SSR hit success. Actor=%s Bone=%s"), *GetNameSafe(TargetActor), *HitResult.BoneName.ToString());
	}
	else
	{
		UE_LOG(LogLyraServerSideRewind, Warning, TEXT("Box SSR hit failed. Actor=%s TraceStart=%s TraceEnd=%s"), *GetNameSafe(TargetActor), *Start.ToString(), *End.ToString());

		ILagCompensationTarget* Target = IsValid(TargetActor) ? Cast<ILagCompensationTarget>(TargetActor) : nullptr;
		if (Target)
		{
			for (const TPair<FName, TObjectPtr<UBoxComponent>>& HitBoxPair : Target->GetHitCollisionBoxes())
			{
				if (UBoxComponent* BoxComponent = HitBoxPair.Value.Get(); IsValid(BoxComponent))
				{
					DrawDebugBox(GetWorld(), BoxComponent->GetComponentLocation(), BoxComponent->GetScaledBoxExtent(), BoxComponent->GetComponentQuat(), FColor::Green, false, DrawDuration);
				}
			}
		}
	}
}

void ULyraServerSideRewindSubsystem::DrawDebugBoxHitResult(const FHitResult& HitResult, bool bConfirmHit) const
{
	if (HitResult.Component.IsValid())
	{
		if (UBoxComponent* Box = Cast<UBoxComponent>(HitResult.Component.Get()))
		{
			const FColor DrawColor = bConfirmHit ? FColor::Blue : FColor::Red;
			DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), DrawColor, false, GetBoxLagCompensationDrawDuration());
		}
	}
}

void ULyraServerSideRewindSubsystem::VisualizeMeshConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, const AActor* TargetActor) const
{
	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float DrawDebugDuration = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;

	if (bSuccess)
	{
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Blue, false, DrawDebugDuration);
		DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Blue, false, DrawDebugDuration, 0, 2.0f);
		UE_LOG(LogLyraServerSideRewind, Log, TEXT("Mesh SSR hit success. Actor=%s Bone=%s"), *GetNameSafe(TargetActor), *HitResult.BoneName.ToString());
	}
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, DrawDebugDuration, 0, 2.0f);
		UE_LOG(LogLyraServerSideRewind, Warning, TEXT("Mesh SSR hit failed. Actor=%s Start=%s End=%s"), *GetNameSafe(TargetActor), *Start.ToString(), *End.ToString());
	}
}
#endif
