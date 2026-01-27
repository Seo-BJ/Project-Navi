// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviWeaponStatsContainer.h"
#include "UI/Composite/NaviCompositeBase.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Weapons/NaviWeaponStatDefinition.h"

void UNaviWeaponStatsContainer::SetWeaponData(const FNaviWeaponStatDefinition* WeaponRow)
{
	if (!WeaponRow)
	{
		SetIsCollapsed(true);
		return;
	}

	SetIsCollapsed(false);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// Composite Pattern: 단순히 자식들에게 데이터를 전파합니다.
	// 자식들이 알아서 자신의 Tag에 맞는 데이터를 꺼내 쓸 것입니다.
	for (UNaviCompositeBase* Child : Children)
	{
		if (Child)
		{
			Child->UpdateWeaponStats(*WeaponRow);
		}
	}
}

void UNaviWeaponStatsContainer::SetWeaponToDisplay(const ULyraInventoryItemDefinition* ItemDef)
{
	// Deprecated implementation or fallback
	Super::UpdateView(ItemDef);
}
