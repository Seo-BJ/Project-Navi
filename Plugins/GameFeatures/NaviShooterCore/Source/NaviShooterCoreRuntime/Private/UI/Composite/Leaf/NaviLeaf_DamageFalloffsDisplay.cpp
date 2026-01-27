// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_DamageFalloffsDisplay.h"
#include "UI/Composite/NaviWidget_DamageColumn.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "Components/PanelWidget.h"
#include "NativeGameplayTags.h"
#include "NaviShooterCoreGameplayTags.h"

void UNaviLeaf_DamageFalloffsDisplay::SetFalloffData(const TArray<FDamageFalloff>& Falloffs)
{
	if (!ColumnContainer || !ColumnWidgetClass)
	{
		SetIsCollapsed(true);
		return;
	}

	// 1. 기존 위젯 모두 제거
	ColumnContainer->ClearChildren();

	if (Falloffs.Num() == 0)
	{
		SetIsCollapsed(true);
		return;
	}

	SetIsCollapsed(false);

	// 2. 데이터 순회하며 위젯 생성
	for (int32 i = 0; i < Falloffs.Num(); ++i)
	{
		const FDamageFalloff& Data = Falloffs[i];
		bool bIsLast = (i == Falloffs.Num() - 1);

		UNaviWidget_DamageColumn* ColumnWidget = CreateWidget<UNaviWidget_DamageColumn>(this, ColumnWidgetClass);
		if (ColumnWidget)
		{
			ColumnWidget->SetFalloffData(Data, bIsLast);
			ColumnContainer->AddChild(ColumnWidget);
		}
	}
}

void UNaviLeaf_DamageFalloffsDisplay::UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData)
{
	// 내 타겟 태그가 Falloffs인지 확인 (선택 사항이지만 안전을 위해)
	if (GetTargetTag().MatchesTag(NaviShooterCoreGameplayTags::Weapon_Stat_Damage_Falloffs))
	{
		SetFalloffData(WeaponData.DamageFalloffs);
	}
}