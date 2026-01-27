// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "NaviCompositeBase.generated.h"

class ULyraInventoryItemDefinition;
struct FNaviWeaponStatDefinition;

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
	virtual void UpdateView(const ULyraInventoryItemDefinition* ItemDef);
	
	/**
	 * Raw Data 기반 업데이트를 위한 인터페이스입니다.
	 * 무기 데이터를 받아 자신의 UI를 갱신하거나 자식에게 전파합니다.
	 * 기본 구현은 아무것도 하지 않습니다.
	 */
	virtual void UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData) {}

	// Helper to collapse or expand the widget
	virtual void SetIsCollapsed(bool bCollapsed);
	FGameplayTag GetTargetTag() const { return TargetTag; }

protected:
	// The specific data tag this widget is interested in (e.g., "Weapon.Stat.Damage")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navi|Composite", meta = (Categories = "Weapon.Stat"))
	FGameplayTag TargetTag;
};
