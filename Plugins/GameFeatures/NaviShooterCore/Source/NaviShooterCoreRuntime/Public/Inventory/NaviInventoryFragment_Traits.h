// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryFragment_AbilityGranting.h"
#include "Weapons/NaviWeaponTraitDefinition.h"
#include "NaviInventoryFragment_Traits.generated.h"

/**
 * Fragment that holds a list of Weapon Traits (DataAssets).
 * Grants abilities defined in those traits.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviInventoryFragment_Traits : public ULyraInventoryFragment_AbilityGranting
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traits")
	TArray<TObjectPtr<UNaviWeaponTraitDefinition>> Traits;

	//~ Begin ULyraInventoryFragment_AbilityGranting Interface
	virtual void GetAbilitySetsToGrant(TArray<const ULyraAbilitySet*>& OutAbilitySets) const override;
	//~ End ULyraInventoryFragment_AbilityGranting Interface
};
