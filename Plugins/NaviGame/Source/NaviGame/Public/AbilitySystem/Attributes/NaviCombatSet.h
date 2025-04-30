// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NaviAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NaviCombatSet.generated.h"

/**
 * 
 */
UCLASS()
class NAVIGAME_API UNaviCombatSet : public UNaviAttributeSet
{
	GENERATED_BODY()

public:

	UNaviCombatSet();

	ATTRIBUTE_ACCESSORS(UNaviCombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UNaviCombatSet, BaseHeal);

protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Lyra|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// The base amount of healing to apply in the heal execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "Lyra|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};
