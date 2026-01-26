// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "LyraInventoryFragment_AbilityGranting.generated.h"

class ULyraAbilitySet;

/**
 * Base fragment for any inventory item fragment that needs to grant abilities when equipped.
 */
UCLASS(Abstract)
class LYRAGAME_API ULyraInventoryFragment_AbilityGranting : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	// Returns the list of ability sets to grant.
	virtual void GetAbilitySetsToGrant(TArray<const ULyraAbilitySet*>& OutAbilitySets) const
	{
		// Default implementation does nothing.
	}
};
