// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "DataRegistrySubsystem.h"

#include "NaviInventoryFragment_SetStatsFromDataTable.generated.h"

class ULyraInventoryItemInstance;
class FDataRegistryItemAcquiredResult;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviInventoryFragment_SetStatsFromDataTable : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:

	virtual void OnInstanceCreated(ULyraInventoryItemInstance* Instance) const override;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataTable> WeaponStatsTable;
};