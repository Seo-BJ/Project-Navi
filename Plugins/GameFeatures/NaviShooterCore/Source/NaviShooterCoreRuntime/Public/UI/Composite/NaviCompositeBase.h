// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "NaviCompositeBase.generated.h"

class ULyraInventoryItemDefinition;

/**
 * Base class for the Composite UI pattern in Navi.
 * Allows recursive updates of UI based on Inventory Item Definitions.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviCompositeBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Update the view based on the provided Item Definition.
	// Can be overridden by subclasses to update specific UI elements or propagate to children.
	virtual void UpdateView(const ULyraInventoryItemDefinition* ItemDef);

	// Helper to collapse or expand the widget
	virtual void SetIsCollapsed(bool bCollapsed);

	FGameplayTag GetTargetTag() const { return TargetTag; }

protected:
	// The specific data tag this widget is interested in (e.g., "Weapon.Stat.Damage")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navi|Composite", meta = (Categories = "Weapon.Stat"))
	FGameplayTag TargetTag;
};
