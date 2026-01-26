// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/NaviCompositeBase.h"
#include "NaviLeaf.generated.h"

class UNaviInventoryFragment_UIInfo;

/**
 * Leaf widget that displays specific data from the Inventory Item Definition.
 * Logic for extracting data and updating UI elements should be implemented in Blueprint
 * via the OnUpdateView event, or by subclassing this in C++.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf : public UNaviCompositeBase
{
	GENERATED_BODY()

public:
	virtual void UpdateView(const ULyraInventoryItemDefinition* ItemDef) override;

protected:
	// Event called when UpdateView is triggered.
	// Implement this in Blueprint to set Text, Images, etc.
	// Return true if data was found and displayed (Widget will be shown),
	// Return false if data was missing (Widget will be collapsed).
	UFUNCTION(BlueprintImplementableEvent, Category = "Navi|Composite")
	bool OnUpdateView(const ULyraInventoryItemDefinition* ItemDef, const UNaviInventoryFragment_UIInfo* UIInfoFragment);
};
