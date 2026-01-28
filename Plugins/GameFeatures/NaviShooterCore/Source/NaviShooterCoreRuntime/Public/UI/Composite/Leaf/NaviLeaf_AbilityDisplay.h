// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/Leaf/NaviLeaf.h"
#include "NaviLeaf_AbilityDisplay.generated.h"

class ULyraGameplayAbility;

/**
 * Displays ability information for a weapon.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf_AbilityDisplay : public UNaviLeaf
{
	GENERATED_BODY()

public:
	virtual void UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Navi|Composite")
	void OnAbilityFound(TSubclassOf<ULyraGameplayAbility> AbilityClass);
};
