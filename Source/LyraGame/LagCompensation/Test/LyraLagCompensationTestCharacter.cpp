// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/Test/LyraLagCompensationTestCharacter.h"
#include "LagCompensation/Test/LyraGameplayAbility_LagCompensationTest.h"
#include "LagCompensation/Test/LyraPatrolPath.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLagCompensationTestCharacter)

DEFINE_LOG_CATEGORY_STATIC(LogLyraLagCompensationTestCharacter, Log, All);

ALyraLagCompensationTestCharacter::ALyraLagCompensationTestCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	PeriodicTestAbilityClass = ULyraGameplayAbility_LagCompensationTest::StaticClass();

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

		AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
		ULyraExperienceManagerComponent* ExperienceComponent =
			GameState ? GameState->FindComponentByClass<ULyraExperienceManagerComponent>() : nullptr;
		if (ExperienceComponent)
		{
			ExperienceComponent->CallOrRegister_OnExperienceLoaded(
				FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
		}
		else
		{
			bExperienceReadyForAbilityTest = true;
			InitializePeriodicAbilityTest();
		}
	}
}

void ALyraLagCompensationTestCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AbilityActivationTimerHandle);
	}

	if (HasAuthority() && PeriodicTestAbilityHandle.IsValid())
	{
		if (ULyraAbilitySystemComponent* AbilitySystem = GetLyraAbilitySystemComponent())
		{
			AbilitySystem->ClearAbility(PeriodicTestAbilityHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALyraLagCompensationTestCharacter::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
	(void)Experience;
	bExperienceReadyForAbilityTest = true;
	InitializePeriodicAbilityTest();
}

void ALyraLagCompensationTestCharacter::InitializePeriodicAbilityTest()
{
	if (!HasAuthority()
		|| !bExperienceReadyForAbilityTest
		|| bAbilityTestInitialized
		|| AbilityTestMode != ELagCompensationAbilityTestMode::ActivateAbility)
	{
		return;
	}

	bAbilityTestInitialized = true;

	ULyraAbilitySystemComponent* AbilitySystem = GetLyraAbilitySystemComponent();
	if (!AbilitySystem || !PeriodicTestAbilityClass || !PeriodicTestMontage)
	{
		UE_LOG(
			LogLyraLagCompensationTestCharacter,
			Warning,
			TEXT("[%s] Periodic montage test disabled. ASC=%s AbilityClass=%s Montage=%s"),
			*GetNameSafe(this),
			*GetNameSafe(AbilitySystem),
			*GetNameSafe(PeriodicTestAbilityClass.Get()),
			*GetNameSafe(PeriodicTestMontage));
		return;
	}

	FGameplayAbilitySpec AbilitySpec(PeriodicTestAbilityClass, 1, INDEX_NONE, this);
	PeriodicTestAbilityHandle = AbilitySystem->GiveAbility(AbilitySpec);

	if (bActivateAbilityImmediately)
	{
		ActivatePeriodicTestAbility();
	}

	const float SafeInterval = FMath::Max(AbilityActivationInterval, 0.05f);
	GetWorldTimerManager().SetTimer(
		AbilityActivationTimerHandle,
		this,
		&ThisClass::ActivatePeriodicTestAbility,
		SafeInterval,
		true);
}

void ALyraLagCompensationTestCharacter::ActivatePeriodicTestAbility()
{
	if (!HasAuthority()
		|| AbilityTestMode != ELagCompensationAbilityTestMode::ActivateAbility
		|| !PeriodicTestAbilityHandle.IsValid())
	{
		return;
	}

	if (ULyraAbilitySystemComponent* AbilitySystem = GetLyraAbilitySystemComponent())
	{
		if (AbilitySystem->TryActivateAbility(PeriodicTestAbilityHandle))
		{
			++PeriodicAbilityActivationCount;
		}
	}
}

void ALyraLagCompensationTestCharacter::RecordPeriodicTestMontageResult(bool bInterrupted)
{
	if (bInterrupted)
	{
		++PeriodicMontageInterruptedCount;
	}
	else
	{
		++PeriodicMontageCompletedCount;
	}
}

void ALyraLagCompensationTestCharacter::InitializeTestCharacter(
	ALyraPatrolPath* InPath,
	ELagCompensationTestMode InMode,
	ELagCompensationAbilityTestMode InAbilityTestMode,
	float InSpeed)
{
	PatrolPath = InPath;
	TestMode = InMode;
	AbilityTestMode = InAbilityTestMode;
	MovementSpeed = InSpeed;

	// SpawnActor's BeginPlay can run before ALyraPatrolPath passes this configuration.
	InitializePeriodicAbilityTest();

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
