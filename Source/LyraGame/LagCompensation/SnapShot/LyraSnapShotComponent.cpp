// Fill out your copyright notice in the Description page of Project Settings.

#include "LyraSnapShotComponent.h"

#include "Character/LyraCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"

ULyraSnapShotComponent::ULyraSnapShotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraSnapShotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor) && OwnerActor->HasAuthority())
	{
		UpdateSnapShotHistory();
	}
}

void ULyraSnapShotComponent::DrawCurrentPhysicsAssetPose(const ALyraCharacter* TargetCharacter, FColor Color) const
{
	if (!IsValid(TargetCharacter))
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMeshComp = TargetCharacter->GetMesh();
	if (!IsValid(SkeletalMeshComp))
	{
		return;
	}

	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float LifeTime = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;

	DrawPhysicsAssetPose(
		SkeletalMeshComp,
		SkeletalMeshComp->GetComponentTransform(),
		SkeletalMeshComp->GetComponentSpaceTransforms(),
		Color,
		LifeTime);
}

void ULyraSnapShotComponent::DrawPhysicsAssetPose(const USkeletalMeshComponent* SkeletalMeshComp,
	const FTransform& MeshTransform, const TArray<FTransform>& ComponentSpaceTransforms, FColor Color,
	float LifeTime) const
{
	if (!IsValid(GetWorld()) || !IsValid(SkeletalMeshComp))
	{
		return;
	}

	UPhysicsAsset* PhysicsAsset = SkeletalMeshComp->GetPhysicsAsset();
	if (!IsValid(PhysicsAsset))
	{
		return;
	}

	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		if (!IsValid(BodySetup))
		{
			continue;
		}

		const int32 BoneIndex = SkeletalMeshComp->GetBoneIndex(BodySetup->BoneName);
		if (BoneIndex == INDEX_NONE || !ComponentSpaceTransforms.IsValidIndex(BoneIndex))
		{
			continue;
		}

		const FTransform BoneWorldTransform = ComponentSpaceTransforms[BoneIndex] * MeshTransform;

		for (const FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			FTransform CapsuleTransform(SphylElem.Rotation, SphylElem.Center);
			CapsuleTransform = CapsuleTransform * BoneWorldTransform;

			const float Radius = SphylElem.Radius * CapsuleTransform.GetScale3D().GetMax();
			const float HalfHeight = SphylElem.Length * 0.5f * CapsuleTransform.GetScale3D().Z;
			DrawDebugCapsule(GetWorld(), CapsuleTransform.GetLocation(), HalfHeight + Radius, Radius, CapsuleTransform.GetRotation(), Color, false, LifeTime, 0, 0.5f);
		}

		for (const FKSphereElem& SphereElem : BodySetup->AggGeom.SphereElems)
		{
			const FVector SphereCenter = BoneWorldTransform.TransformPosition(SphereElem.Center);
			const float Radius = SphereElem.Radius * BoneWorldTransform.GetScale3D().GetMax();
			DrawDebugSphere(GetWorld(), SphereCenter, Radius, 12, Color, false, LifeTime, 0, 0.5f);
		}

		for (const FKBoxElem& BoxElem : BodySetup->AggGeom.BoxElems)
		{
			FTransform BoxTransform(BoxElem.Rotation, BoxElem.Center);
			BoxTransform = BoxTransform * BoneWorldTransform;
			const FVector BoxExtent = FVector(BoxElem.X, BoxElem.Y, BoxElem.Z) * 0.5f * BoxTransform.GetScale3D();
			DrawDebugBox(GetWorld(), BoxTransform.GetLocation(), BoxExtent, BoxTransform.GetRotation(), Color, false, LifeTime, 0, 0.5f);
		}
	}
}
