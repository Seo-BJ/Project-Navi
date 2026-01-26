// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NaviWeaponTraitDefinition.generated.h"

class ULyraAbilitySet;

/**
 * Defines a weapon trait (passive ability or feature) that can be attached to a weapon.
 */
UCLASS(BlueprintType, Const)
class NAVISHOOTERCORERUNTIME_API UNaviWeaponTraitDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// UI Name of the trait
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trait")
	FText DisplayName;

	// UI Description
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trait")
	FText Description;

	// UI Icon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trait")
	TObjectPtr<UTexture2D> Icon;

	// Ability sets to grant when this trait is active on an equipped weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trait")
	TArray<TObjectPtr<const ULyraAbilitySet>> AbilitySetsToGrant;

	// Tags associated with this trait (e.g., "Weapon.Trait.Silencer")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trait")
	FGameplayTagContainer TraitTags;
};
