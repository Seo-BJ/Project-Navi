// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NaviDescriptionContainer.h"
#include "UI/Composite/NaviWeaponStatsContainer.h"
#include "CredsSystem/NaviCredsShopComponent.h"
#include "CommonUI/Public/CommonVisibilitySwitcher.h"
#include "GameFramework/GameStateBase.h"
#include "NativeGameplayTags.h" 

// 태그 정의 (필요시 헤더로 이동)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_Weapon, "Weapon");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_Armor, "Armor");
// UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_Skill, "Skill");

void UNaviDescriptionContainer::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNaviDescriptionContainer::SetDataToDisplay(FGameplayTag EquipmentTag)
{
	if (!EquipmentTag.IsValid())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 1. ShopComponent 찾기 (GameState에 있다고 가정)
	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GS = World->GetGameState();
	if (!GS) return;

	UNaviCredsShopComponent* ShopComp = GS->GetComponentByClass<UNaviCredsShopComponent>();
	if (!ShopComp) return;


	// 2. 태그 타입에 따라 데이터 조회 및 스위칭
	if (EquipmentTag.MatchesTag(TAG_Navi_Weapon))
	{
		const FNaviWeaponStatDefinition* WeaponRow = ShopComp->GetWeaponStatRow(EquipmentTag);
		if (WeaponRow)
		{
			if (PreviewSwitcher)
			{
				PreviewSwitcher->SetActiveWidgetIndex(0); // Weapon Index
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			UpdateWeaponStats(WeaponRow);
		}
	}
	else if (EquipmentTag.MatchesTag(TAG_Navi_Armor))
	{
		// TODO: ArmorStatDefinition을 사용하는 로직 추가
		/*
		const FNaviArmorStatDefinition* ArmorRow = ShopComp->GetArmorStatRow(EquipmentTag);
		if (ArmorRow)
		{
			if (PreviewSwitcher)
			{
				PreviewSwitcher->SetActiveWidgetIndex(1); // Armor Index
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			// UpdateArmorStats(ArmorRow);
		}
		*/
		
		// 임시: 방어구라고 치고 스위처만 전환 (데이터가 없어서 내용은 못 채움)
		if (PreviewSwitcher)
		{
			PreviewSwitcher->SetActiveWidgetIndex(1);
			SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		// 그 외(스킬 등)
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNaviDescriptionContainer::UpdateWeaponStats(const FNaviWeaponStatDefinition* WeaponRow)
{
	if (WeaponStatsContainer)
	{
		// NaviWeaponStatsContainer가 ItemDef 대신 Raw Data를 받을 수 있도록 함수 호출
		// (이 함수는 다음 단계에서 NaviWeaponStatsContainer에 추가해야 함)
		WeaponStatsContainer->SetWeaponData(WeaponRow);
	}
}