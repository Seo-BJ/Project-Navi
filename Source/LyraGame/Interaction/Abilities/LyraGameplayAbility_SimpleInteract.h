// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "LyraGameplayAbility_SimpleInteract.generated.h"

/**
 * A universal ability that bridges GAS interaction to the ISimpleInteractable interface.
 * Use this ability in InteractionOptions for objects that implement ISimpleInteractable.
 */
UCLASS()
class LYRAGAME_API ULyraGameplayAbility_SimpleInteract : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	ULyraGameplayAbility_SimpleInteract(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
