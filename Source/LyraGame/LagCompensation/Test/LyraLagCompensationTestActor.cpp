// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraLagCompensationTestActor.h"
#include "LyraPatrolPath.h"

ALyraLagCompensationTestActor::ALyraLagCompensationTestActor(const FObjectInitializer& ObjectInitializer)
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

void ALyraLagCompensationTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALyraLagCompensationTestActor::InitializeTestActor(ALyraPatrolPath* InPath, ELagCompensationTestMode InMode, float InSpeed)
{
	PatrolPath = InPath;
	TestMode = InMode;
	MovementSpeed = InSpeed;

	if (PatrolPath)
	{
		SetActorLocation(PatrolPath->GetPoint1WorldLocation());
		CurrentTargetLocation = PatrolPath->GetPoint2WorldLocation();
	}
}

void ALyraLagCompensationTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority() && PatrolPath)
	{
		const FVector CurrentLocation = GetActorLocation();
		FVector Direction = (CurrentTargetLocation - CurrentLocation);
		Direction.Z = 0.f;

		if (Direction.SizeSquared() > 25.f)
		{
			Direction.Normalize();

			// 위치 이동
			const FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
			SetActorLocation(NewLocation);

			// 회전 보간 (이동 방향으로 부드럽게 회전)
			const FRotator TargetRotation = Direction.Rotation();
			const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.f);
			SetActorRotation(NewRotation);
		}
		
		if (FVector::DistSquared(GetActorLocation(), CurrentTargetLocation) < FMath::Square(10.f))
		{
			UpdateTargetLocation();
		}
	}
}

const TMap<FName, TObjectPtr<UBoxComponent>>& ALyraLagCompensationTestActor::GetHitCollisionBoxes() const
{
	return HitCollisionBoxes;
}

void ALyraLagCompensationTestActor::UpdateTargetLocation()
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
