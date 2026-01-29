// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/NaviComposite.h"
#include "NaviArmorStatsContainer.generated.h"

class ULyraInventoryItemDefinition;
struct FNaviArmorStatDefinition;

/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviArmorStatsContainer : public UNaviComposite
{
	GENERATED_BODY()

public:
	/**
	 * Updates the stats using raw data from the data table.
	 * Bypasses the need for an ItemDefinition.
	 */
	void SetArmorData(const FNaviArmorStatDefinition* ArmorRow);


};
