// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/Test/LyraLagCompensationTestCharacter.h"
#include "LagCompensation/Test/LyraPatrolPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ALyraLagCompensationTestCharacter::ALyraLagCompensationTestCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// AI 컨트롤러 자동 생성 및 빙의 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MovementSpeed;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 360.f, 0.f);
		
		MoveComp->GravityScale = 1.0f;
		MoveComp->DefaultLandMovementMode = MOVE_Walking;

		// 앉기 기능 활성화
		MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	}

	bUseControllerRotationYaw = false;
}

void ALyraLagCompensationTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}

void ALyraLagCompensationTestCharacter::InitializeTestCharacter(ALyraPatrolPath* InPath, ELagCompensationTestMode InMode, float InSpeed)
{
	PatrolPath = InPath;
	TestMode = InMode;
	MovementSpeed = InSpeed;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MovementSpeed;
	}

	if (PatrolPath)
	{
		SetActorLocation(PatrolPath->GetPoint1WorldLocation());
		CurrentTargetLocation = PatrolPath->GetPoint2WorldLocation();
	}

	// 초기 모드 설정 적용
	if (TestMode == ELagCompensationTestMode::CrouchEachTrip)
	{
		Crouch();
	}
}

void ALyraLagCompensationTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && PatrolPath)
	{
		ApplyModeActions();

		const FVector CurrentLocation = GetActorLocation();
		FVector Direction = (CurrentTargetLocation - CurrentLocation);
		Direction.Z = 0.f;
		
		const float DistanceToTarget = Direction.Size();
		
		if (DistanceToTarget > 50.f)
		{
			Direction.Normalize();
			AddMovementInput(Direction, 1.0f);
		}
		else
		{
			UpdateTargetLocation();
		}
	}
}

void ALyraLagCompensationTestCharacter::ApplyModeActions()
{
	if (bHasPerformedTripAction) return;

	switch (TestMode)
	{
	case ELagCompensationTestMode::JumpEachTrip:
		{
			// 목표 지점 방향 계산
			const FVector TargetDir = (CurrentTargetLocation - GetActorLocation()).GetSafeNormal2D();
			const FVector ForwardDir = GetActorForwardVector().GetSafeNormal2D();

			// 현재 바라보는 방향과 목표 방향이 약 10도 이내로 일치할 때만 점프 수행
			// Dot Product 결과가 0.98 이상이면 거의 같은 방향 (cos 10도 ≒ 0.984)
			if (FVector::DotProduct(TargetDir, ForwardDir) > 0.98f)
			{
				Jump();
				bHasPerformedTripAction = true;
			}
		}
		break;

	case ELagCompensationTestMode::ToggleCrouchPerTrip:
		if (bIsCrouched)
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
		bHasPerformedTripAction = true;
		break;

	default:
		break;
	}
}

void ALyraLagCompensationTestCharacter::UpdateTargetLocation()
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

		// 다음 목적지로 출발할 때 액션 플래그 리셋
		bHasPerformedTripAction = false;
	}
}
