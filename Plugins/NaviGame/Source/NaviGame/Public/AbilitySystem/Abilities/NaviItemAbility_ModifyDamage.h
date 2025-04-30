// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "NaviItemAbility_ModifyDamage.generated.h"

/**
 * 
 */
UCLASS()
class NAVIGAME_API UNaviItemAbility_ModifyDamage : public ULyraGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UNaviItemAbility_ModifyDamage();

protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);

	void HandleBeforeDamageEvent(const FGameplayEventData* Payload);

	// Configurable multiplier for this item/passive
	UPROPERTY(EditDefaultsOnly, Category="Damage Mod")
	float DamageMultiplier = 1.2f; // e.g., Increase outgoing damage by 20%

	// The GE to apply temporarily for the modifier
	UPROPERTY(EditDefaultsOnly, Category="Damage Mod")
	TSubclassOf<UGameplayEffect> TemporaryModifierEffect; // Should grant a temp attribute like "OutgoingDamageMultiplier"

	FDelegateHandle EventHandle;
};