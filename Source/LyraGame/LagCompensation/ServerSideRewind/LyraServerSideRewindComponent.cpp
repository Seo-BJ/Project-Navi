// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/ServerSideRewind/LyraServerSideRewindComponent.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
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

ULyraServerSideRewindComponent::ULyraServerSideRewindComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FServerSideRewindResult ULyraServerSideRewindComponent::ServerSideRewind(
	AActor* TargetActor,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime
	)
{
	FLyraServerSideRewindRequest Request;
	Request.TargetActor = TargetActor;
	Request.TraceStart = TraceStart;
	Request.HitLocation = HitLocation;
	Request.HitTime = HitTime;
	return ServerSideRewind(Request);
}

FServerSideRewindResult ULyraServerSideRewindComponent::ServerSideRewind(const FLyraServerSideRewindRequest& Request)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraSSR_Dispatch);
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_Dispatch);

	FServerSideRewindResult Result;
	ELyraSnapShotMode ResolvedMode = ELyraSnapShotMode::None;
	ULyraSnapShotComponent* SnapShotComponent = nullptr;
	AActor* TargetActor = Request.TargetActor.Get();
	if (IsValid(TargetActor))
	{
		SnapShotComponent = ResolveSnapShotComponent(TargetActor);
		if (IsValid(SnapShotComponent))
		{
			ResolvedMode = SnapShotComponent->GetSnapShotMode();
			switch (ResolvedMode)
			{
			case ELyraSnapShotMode::BoxComponent:
				Result = ServerSideRewindBoxComponent(Cast<ULyraSnapShotComponent_BoxComponent>(SnapShotComponent), Request);
				break;
			case ELyraSnapShotMode::SkeletalMesh:
				Result = ServerSideRewindSkeletalMesh(Cast<ULyraSnapShotComponent_SkeletalMesh>(SnapShotComponent), Request);
				break;
			case ELyraSnapShotMode::AnimNode:
				Result = ServerSideRewindAnimNode(Cast<ULyraSnapShotComponent_AnimNode>(SnapShotComponent), Request);
				break;
			default:
				break;
			}
		}
	}

	return Result;
}

ULyraSnapShotComponent* ULyraServerSideRewindComponent::ResolveSnapShotComponent(AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return nullptr;
	}

	if (ULyraSnapShotComponent_BoxComponent* BoxComponent = TargetActor->FindComponentByClass<ULyraSnapShotComponent_BoxComponent>(); IsValid(BoxComponent))
	{
		return BoxComponent;
	}
	if (ULyraSnapShotComponent_AnimNode* AnimNodeComponent = TargetActor->FindComponentByClass<ULyraSnapShotComponent_AnimNode>(); IsValid(AnimNodeComponent))
	{
		return AnimNodeComponent;
	}
	if (ULyraSnapShotComponent_SkeletalMesh* SkeletalMeshComponent = TargetActor->FindComponentByClass<ULyraSnapShotComponent_SkeletalMesh>(); IsValid(SkeletalMeshComponent))
	{
		return SkeletalMeshComponent;
	}
	if (ULyraSnapShotComponent* SnapShotComponent = TargetActor->FindComponentByClass<ULyraSnapShotComponent>(); IsValid(SnapShotComponent))
	{
		if (!SnapShotComponent->IsValidLowLevel())
		{
			UE_LOG(LogLyraServerSideRewind, Warning, TEXT("[%s] Invalid low-level Snapshot component pointer."), *GetNameSafe(TargetActor));
			return nullptr;
		}
		return SnapShotComponent;
	}

	return nullptr;
}

FServerSideRewindResult ULyraServerSideRewindComponent::ServerSideRewindBoxComponent(ULyraSnapShotComponent_BoxComponent* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return FServerSideRewindResult();
	}

	FLyraBoxSnapShotFrame FrameToCheck;
	if (!SnapShotComponent->TryGetSnapShotAtTime(Request.HitTime, FrameToCheck))
	{
		return FServerSideRewindResult();
	}

	FrameToCheck.HitActor = Request.TargetActor.Get();
	return ConfirmBoxHit(SnapShotComponent, FrameToCheck, Request);
}

FServerSideRewindResult ULyraServerSideRewindComponent::ConfirmBoxHit(
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

	FLyraBoxSnapShotFrame CurrentFrame;
	SnapShotComponent->CaptureCurrentSnapShot(TargetActor, CurrentFrame);
	if (CurrentFrame.HitBoxInfo.IsEmpty())
	{
		return FServerSideRewindResult();
	}

	bool bShouldRestore = false;
	ON_SCOPE_EXIT
	{
		if (bShouldRestore)
		{
			SnapShotComponent->RestoreSnapShot(TargetActor, CurrentFrame);
			SetMeshCollisionEnabledType(TargetActor, ECollisionEnabled::QueryAndPhysics);
		}
	};

	SnapShotComponent->ApplySnapShot(TargetActor, FrameToCheck);
	SetMeshCollisionEnabledType(TargetActor, ECollisionEnabled::NoCollision);
	bShouldRestore = true;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(LyraSSR_BoxConfirmHit), true);
	const FVector TraceEnd = Request.TraceStart + (Request.HitLocation - Request.TraceStart) * 1.25f;

#if ENABLE_DRAW_DEBUG
	if (bDrawBoxHitResult)
	{
		DrawDebugLine(World, Request.TraceStart, TraceEnd, FColor::Red, false, BoxHitDebugDrawTime, 0, 1.0f);
		DrawDebugPoint(World, Request.HitLocation, 10.0f, FColor::Cyan, false, BoxHitDebugDrawTime);
	}
#endif

	FHitResult ConfirmHitResult;
	bool bHitSuccess = false;
	bool bHeadShot = false;

	const TMap<FName, TObjectPtr<UBoxComponent>>& HitBoxes = Target->GetHitCollisionBoxes();
	const TObjectPtr<UBoxComponent>* FoundBoxPtr = HitBoxes.Find(FName("head"));
	if (FoundBoxPtr && IsValid(FoundBoxPtr->Get()))
	{
		TArray<UBoxComponent*> HeadBoxes;
		HeadBoxes.Add(FoundBoxPtr->Get());

		if (PerformBoxHitCheck(HeadBoxes, Request.TraceStart, TraceEnd, TraceParams, ConfirmHitResult))
		{
			bHitSuccess = true;
			bHeadShot = true;
		}
	}

	if (!bHitSuccess)
	{
		TArray<UBoxComponent*> AllBoxes;
		for (const TPair<FName, TObjectPtr<UBoxComponent>>& Pair : HitBoxes)
		{
			if (IsValid(Pair.Value.Get()))
			{
				AllBoxes.Add(Pair.Value.Get());
			}
		}

		if (PerformBoxHitCheck(AllBoxes, Request.TraceStart, TraceEnd, TraceParams, ConfirmHitResult))
		{
			bHitSuccess = true;
			bHeadShot = false;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bDrawBoxHitResult)
	{
		VisualizeBoxConfirmHit(Request.TraceStart, TraceEnd, bHitSuccess, ConfirmHitResult, TargetActor);
	}
#endif

	return FServerSideRewindResult{ bHitSuccess, bHeadShot };
}

bool ULyraServerSideRewindComponent::PerformBoxHitCheck(const TArray<UBoxComponent*>& BoxesToCheck, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	for (UBoxComponent* Box : BoxesToCheck)
	{
		if (IsValid(Box))
		{
			Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Box->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Block);
		}
	}

	const bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, Lyra_TraceChannel_LagCompensation_HitBox, Params);

	for (UBoxComponent* Box : BoxesToCheck)
	{
		if (IsValid(Box))
		{
			Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Box->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Ignore);
		}
	}

	return bHit;
}

void ULyraServerSideRewindComponent::SetMeshCollisionEnabledType(AActor* TargetActor, ECollisionEnabled::Type CollisionEnabled) const
{
	ACharacter* HitCharacter = Cast<ACharacter>(TargetActor);
	if (!IsValid(HitCharacter))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = HitCharacter->GetMesh();
	UCapsuleComponent* Capsule = HitCharacter->GetCapsuleComponent();
	if (IsValid(Mesh))
	{
		Mesh->SetCollisionEnabled(CollisionEnabled);
	}
	if (IsValid(Capsule))
	{
		Capsule->SetCollisionEnabled(CollisionEnabled);
	}
}

FServerSideRewindResult ULyraServerSideRewindComponent::ServerSideRewindSkeletalMesh(ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return FServerSideRewindResult();
	}

	FLyraMeshSnapShotFrame FrameToCheck;
	if (!SnapShotComponent->TryGetSnapShotAtTime(Request.HitTime, FrameToCheck))
	{
		return FServerSideRewindResult();
	}

	FrameToCheck.HitActor = Request.TargetActor.Get();
	return ConfirmMeshHit(SnapShotComponent, FrameToCheck, Request);
}

FServerSideRewindResult ULyraServerSideRewindComponent::ConfirmMeshHit(
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

	static const TSet<FName> HeadFilter = { FName("head") };
	bool bHit = PerformPhysicsAssetCollision(SnapShotComponent, TargetActor, FrameToCheck, Request.TraceStart, TraceEnd, HeadFilter, OutHit);
	bool bHeadShot = bHit;

	if (!bHit)
	{
		static const TSet<FName> EmptyFilter;
		bHit = PerformPhysicsAssetCollision(SnapShotComponent, TargetActor, FrameToCheck, Request.TraceStart, TraceEnd, EmptyFilter, OutHit);
	}

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

	return FServerSideRewindResult{ bHit, bHeadShot };
}

FServerSideRewindResult ULyraServerSideRewindComponent::ServerSideRewindAnimNode(ULyraSnapShotComponent_AnimNode* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const
{
	if (!IsValid(SnapShotComponent) || !IsValid(Request.TargetActor.Get()))
	{
		return FServerSideRewindResult();
	}

	FAnimSSRRewindPose RewindPose;
	if (!SnapShotComponent->EvaluatePoseAtServerTime(Request.HitTime, RewindPose))
	{
		return FServerSideRewindResult();
	}

	return ConfirmAnimNodeHit(SnapShotComponent, RewindPose, Request);
}

FServerSideRewindResult ULyraServerSideRewindComponent::ConfirmAnimNodeHit(
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

	static const TSet<FName> HeadFilter = { FName("head") };
	bool bHit = PerformPhysicsAssetCollision(SkeletalMeshComp, RewindPose.MeshComponentTransform, RewindPose.ComponentSpaceTransforms, Request.TraceStart, TraceEnd, HeadFilter, OutHit);
	bool bHeadShot = bHit;

	if (!bHit)
	{
		static const TSet<FName> EmptyFilter;
		bHit = PerformPhysicsAssetCollision(SkeletalMeshComp, RewindPose.MeshComponentTransform, RewindPose.ComponentSpaceTransforms, Request.TraceStart, TraceEnd, EmptyFilter, OutHit);
	}

#if ENABLE_DRAW_DEBUG
	const ULyraLagCompensationDeveloperSettings* LagCompSettings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	if (IsValid(LagCompSettings) && LagCompSettings->bDrawMeshLagCompensation)
	{
		VisualizeMeshConfirmHit(Request.TraceStart, TraceEnd, bHit, OutHit, TargetActor);
		SnapShotComponent->DrawDebugPose(RewindPose, bHit ? FColor::Green : FColor::Red);
		SnapShotComponent->DrawCurrentDebugPose(FColor::Blue);
	}
#endif

	return FServerSideRewindResult{ bHit, bHeadShot };
}

bool ULyraServerSideRewindComponent::PerformPhysicsAssetCollision(
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

bool ULyraServerSideRewindComponent::PerformPhysicsAssetCollision(
	USkeletalMeshComponent* SkeletalMeshComp,
	const FTransform& MeshTransform,
	const TArray<FTransform>& ComponentSpaceTransforms,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	const TSet<FName>& BoneFilter,
	FHitResult& OutHit
	) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraSSR_MeshPhysicsAssetCollision);
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
			const FVector ClosestPoint = FMath::ClosestPointOnSegment(SphereCenter, TraceStart, TraceEnd);

			if (FVector::DistSquared(SphereCenter, ClosestPoint) <= FMath::Square(SphereRadius))
			{
				const float Distance = FVector::Dist(TraceStart, ClosestPoint);
				return FillHitAndReturn(ClosestPoint, (ClosestPoint - SphereCenter).GetSafeNormal(), Distance, BodySetup->BoneName);
			}
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
				return FillHitAndReturn(WorldImpact, WorldNormal, HitT * TraceLength, BodySetup->BoneName);
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

				const FVector ImpactPointOnSurface = ClosestPointOnCapsule + SurfaceNormal * Radius;
				const float Distance = FVector::Dist(TraceStart, ImpactPointOnSurface);
				return FillHitAndReturn(ImpactPointOnSurface, SurfaceNormal, Distance, BodySetup->BoneName);
			}
		}
	}

	return false;
}

#if ENABLE_DRAW_DEBUG
void ULyraServerSideRewindComponent::VisualizeBoxConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* TargetActor) const
{
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
					DrawDebugBox(GetWorld(), BoxComponent->GetComponentLocation(), BoxComponent->GetScaledBoxExtent(), BoxComponent->GetComponentQuat(), FColor::Green, false, BoxHitDebugDrawTime);
				}
			}
		}
	}
}

void ULyraServerSideRewindComponent::DrawDebugBoxHitResult(const FHitResult& HitResult, bool bConfirmHit) const
{
	if (HitResult.Component.IsValid())
	{
		if (UBoxComponent* Box = Cast<UBoxComponent>(HitResult.Component.Get()))
		{
			const FColor DrawColor = bConfirmHit ? FColor::Blue : FColor::Red;
			DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), DrawColor, false, BoxHitDebugDrawTime);
		}
	}
}

void ULyraServerSideRewindComponent::VisualizeMeshConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, const AActor* TargetActor) const
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
