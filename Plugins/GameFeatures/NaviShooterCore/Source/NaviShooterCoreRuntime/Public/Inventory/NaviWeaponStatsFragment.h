// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/NaviInventoryFragment_UIInfo.h"
#include "Engine/DataTable.h"
#include "NaviWeaponStatsFragment.generated.h"

/**
 * Inventory Fragment that links to a Weapon Stat Data Table Row.
 * Automatically provides UI Info (Stats, Name, etc.) from the table row.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviWeaponStatsFragment : public UNaviInventoryFragment_UIInfo
{
	GENERATED_BODY()

public:
	// Link to the Weapon Stat Table Row (e.g., "Rifle_AK" in DT_WeaponStats)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|WeaponStats", meta=(RowType="/Script/NaviShooterCoreRuntime.NaviWeaponStatDefinition"))
	FDataTableRowHandle WeaponStatRow;

	//~ Begin UNaviInventoryFragment_UIInfo Interface
	virtual float GetNumericStat_Implementation(FGameplayTag Tag, bool& bFound) const override;
	virtual FText GetTextData_Implementation(FGameplayTag Tag, bool& bFound) const override;
	//~ End UNaviInventoryFragment_UIInfo Interface
};
