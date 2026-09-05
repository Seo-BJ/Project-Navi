// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacterWithAbilities.h"
#include "GameplayAbilitySpecHandle.h"
#include "LyraPatrolPath.h"
#include "LyraLagCompensationTestCharacter.generated.h"

class ALyraPatrolPath;
class UAnimMontage;
class ULyraExperienceDefinition;
class ULyraGameplayAbility_LagCompensationTest;

/**
 * ALyraLagCompensationTestCharacter
 * 
 * Character version of LyraLagCompensationTestActor to test LyraMeshLagCompensationComponent
 * with skeletal mesh assets and animations.
 */
UCLASS(Blueprintable)
class LYRAGAME_API ALyraLagCompensationTestCharacter : public ALyraCharacterWithAbilities
{
	GENERATED_BODY()

public:
	ALyraLagCompensationTestCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	// PatrolPath에서 스폰 후 초기화하기 위한 함수
	void InitializeTestCharacter(
		ALyraPatrolPath* InPath,
		ELagCompensationTestMode InMode,
		ELagCompensationAbilityTestMode InAbilityTestMode,
		float InSpeed);

	UAnimMontage* GetPeriodicTestMontage() const { return PeriodicTestMontage; }
	float GetPeriodicMontagePlayRate() const { return PeriodicMontagePlayRate; }
	void RecordPeriodicTestMontageResult(bool bInterrupted);
	int32 GetPeriodicAbilityActivationCount() const { return PeriodicAbilityActivationCount; }
	int32 GetPeriodicMontageCompletedCount() const { return PeriodicMontageCompletedCount; }
	int32 GetPeriodicMontageInterruptedCount() const { return PeriodicMontageInterruptedCount; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Server-only ability that plays PeriodicTestMontage through GAS montage replication. */
	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation Test|Ability")
	TSubclassOf<ULyraGameplayAbility_LagCompensationTest> PeriodicTestAbilityClass;

	/** Montage whose changing skeletal pose should be captured by ServerSideRewind. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lag Compensation Test|Ability")
	TObjectPtr<UAnimMontage> PeriodicTestMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lag Compensation Test|Ability", meta = (ClampMin = "0.01"))
	float PeriodicMontagePlayRate = 1.0f;

	/** Time between activation attempts. An active ability is not restarted. */
	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation Test|Ability", meta = (ClampMin = "0.05", Units = "s"))
	float AbilityActivationInterval = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation Test|Ability")
	bool bActivateAbilityImmediately = true;

	// 테스트 모드 설정 (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Lag Compensation Test")
	ELagCompensationTestMode TestMode = ELagCompensationTestMode::Normal;

	// Idle이면 이동 테스트만 수행하고, ActivateAbility이면 몽타주 어빌리티를 주기적으로 실행
	UPROPERTY(VisibleAnywhere, Category = "Lag Compensation Test")
	ELagCompensationAbilityTestMode AbilityTestMode = ELagCompensationAbilityTestMode::Idle;

	// Patrol path actor (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	TObjectPtr<ALyraPatrolPath> PatrolPath;

	// Movement speed (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float MovementSpeed = 300.f;

private:
	FVector CurrentTargetLocation;
	FTimerHandle AbilityActivationTimerHandle;
	FGameplayAbilitySpecHandle PeriodicTestAbilityHandle;
	bool bExperienceReadyForAbilityTest = false;
	bool bAbilityTestInitialized = false;

	/** SSR Evaluate 전후 gameplay side effect가 없음을 확인하기 위한 누적 횟수다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lag Compensation Test|Ability", meta = (AllowPrivateAccess = "true"))
	int32 PeriodicAbilityActivationCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lag Compensation Test|Ability", meta = (AllowPrivateAccess = "true"))
	int32 PeriodicMontageCompletedCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lag Compensation Test|Ability", meta = (AllowPrivateAccess = "true"))
	int32 PeriodicMontageInterruptedCount = 0;

	// 현재 왕복(Trip)에서 액션을 수행했는지 여부
	bool bHasPerformedTripAction = false;

	void UpdateTargetLocation();
	void ApplyModeActions();
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	void InitializePeriodicAbilityTest();
	void ActivatePeriodicTestAbility();
};
