// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "LyraGameplayAbility_LagCompensationTest.generated.h"

/**
 * Server-initiated test ability that plays the montage configured on
 * ALyraLagCompensationTestCharacter. GAS replicates the montage to clients.
 */
UCLASS()
class LYRAGAME_API ULyraGameplayAbility_LagCompensationTest : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	ULyraGameplayAbility_LagCompensationTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelledOrInterrupted();
};
