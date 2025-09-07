// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTestWeaponFire.h"
#include "LyraPatrolPath.h"

ALyraTestWeaponFire::ALyraTestWeaponFire(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	if (BoxComponent)
	{
		HitCollisionBoxes.Add(FName("head"), BoxComponent);
	}
}

void ALyraTestWeaponFire::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (PatrolPath)
		{
			SetActorLocation(PatrolPath->GetPoint1WorldLocation());
			CurrentTargetLocation = PatrolPath->GetPoint2WorldLocation();
		}
	}
}

void ALyraTestWeaponFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		if (PatrolPath)
		{
			const FVector CurrentLocation = GetActorLocation();
			FVector Direction = (CurrentTargetLocation - CurrentLocation);
			Direction.Normalize();

			const FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;
			SetActorLocation(NewLocation);
			
			if (FVector::DistSquared(NewLocation, CurrentTargetLocation) < FMath::Square(5.f))
			{
				UpdateTargetLocation();
			}
		}
	}
}

const TMap<FName, TObjectPtr<UBoxComponent>>& ALyraTestWeaponFire::GetHitCollisionBoxes() const
{
	return HitCollisionBoxes;
}

void ALyraTestWeaponFire::UpdateTargetLocation()
{
	if (PatrolPath)
	{
		if (CurrentTargetLocation.Equals(PatrolPath->GetPoint2WorldLocation()))
		{
			CurrentTargetLocation = PatrolPath->GetPoint1WorldLocation();
		}
		else
		{
			CurrentTargetLocation = PatrolPath->GetPoint2WorldLocation();
		}
	}
}
