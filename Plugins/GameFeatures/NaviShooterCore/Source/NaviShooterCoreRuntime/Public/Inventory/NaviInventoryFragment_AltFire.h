// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryFragment_AbilityGranting.h"
#include "NaviInventoryFragment_AltFire.generated.h"

/**
 * Fragment that defines the Alt-Fire (Secondary Attack) mode of a weapon.
 * Grants abilities and provides UI info.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviInventoryFragment_AltFire : public ULyraInventoryFragment_AbilityGranting
{
	GENERATED_BODY()

public:
	// Abilities to grant for Alt-Fire (e.g., GA_ADS, GA_GrenadeLauncher)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AltFire")
	TArray<TObjectPtr<const ULyraAbilitySet>> AbilitySetsToGrant;

	// UI: Name of the action (e.g., "Zoom")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AltFire")
	FText DisplayName;

	// UI: Description
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AltFire")
	FText Description;

	// UI: Icon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AltFire")
	TObjectPtr<UTexture2D> Icon;

	//~ Begin ULyraInventoryFragment_AbilityGranting Interface
	virtual void GetAbilitySetsToGrant(TArray<const ULyraAbilitySet*>& OutAbilitySets) const override;
	//~ End ULyraInventoryFragment_AbilityGranting Interface
};
