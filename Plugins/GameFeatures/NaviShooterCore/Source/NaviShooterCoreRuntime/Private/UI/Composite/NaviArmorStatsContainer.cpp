// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviArmorStatsContainer.h"

void UNaviArmorStatsContainer::SetArmorData(const FNaviArmorStatDefinition* ArmorRow)
{

	if (!ArmorRow)
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
			Child->UpdateArmorStats(*ArmorRow);
		}
	}
}
