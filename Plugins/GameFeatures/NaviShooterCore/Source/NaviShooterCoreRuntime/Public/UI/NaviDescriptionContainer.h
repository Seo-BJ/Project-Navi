// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "NaviDescriptionContainer.generated.h"

class UNaviWeaponStatsContainer;
class UCommonVisibilitySwitcher;
class ULyraInventoryItemDefinition;

/**
 * 아이템 정의(ItemDef)를 받아 적절한 설명 위젯(무기/방어구/스킬)으로 스위칭하여 보여주는 컨테이너입니다.
 * 상점, 인벤토리 등에서 재사용할 수 있습니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviDescriptionContainer : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * 장비 태그를 받아 타입을 분석하고, ShopComponent에서 데이터를 조회하여 적절한 자식 위젯을 활성화합니다.
	 * @param EquipmentTag 표시할 장비의 태그 (예: Weapon.Rifle.AK47)
	 */
	UFUNCTION(BlueprintCallable, Category = "Navi|Description")
	void SetDataToDisplay(FGameplayTag EquipmentTag);

protected:
	virtual void NativeOnInitialized() override;
	
	// 헬퍼: 무기 스탯 정보로 UI 갱신
	void UpdateWeaponStats(const struct FNaviWeaponStatDefinition* WeaponRow);

protected:
	// 아이템 타입(무기, 방어구, 스킬)에 따라 표시할 위젯을 전환하는 스위처입니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> PreviewSwitcher;

	// 무기 스탯을 표시하는 컨테이너 위젯입니다. (Index 0)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviWeaponStatsContainer> WeaponStatsContainer;

	// 방어구 정보를 표시하는 위젯입니다. (Index 1)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ArmorStatsContainer;

	// 스킬 정보를 표시하는 위젯입니다. (Index 2)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> SkillStatsContainer;
};
