// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/NaviComposite.h"
#include "NaviWeaponStatsContainer.generated.h"

class ULyraInventoryItemDefinition;
struct FNaviWeaponStatDefinition;

/**
 * A container widget specifically for displaying weapon statistics.
 * It expects to have child widgets (NaviLeaf_StatDisplay) arranged in the designer.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviWeaponStatsContainer : public UNaviComposite
{
	GENERATED_BODY()

public:
	/**
	 * Updates the stats using raw data from the data table.
	 * Bypasses the need for an ItemDefinition.
	 */
	void SetWeaponData(const FNaviWeaponStatDefinition* WeaponRow);

	
	

	/**
	 * Main entry point for updating the stats.
	 * Call this when the mouse hovers over a weapon button.
	 * 
	 * @param ItemDef The weapon item definition to extract stats from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Navi|WeaponStats")
	void SetWeaponToDisplay(const ULyraInventoryItemDefinition* ItemDef);
};
