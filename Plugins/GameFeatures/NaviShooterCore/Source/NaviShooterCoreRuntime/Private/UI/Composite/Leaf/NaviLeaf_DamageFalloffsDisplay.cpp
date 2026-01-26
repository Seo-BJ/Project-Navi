// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_DamageFalloffsDisplay.h"
#include "UI/Composite/NaviWidget_DamageColumn.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "Components/PanelWidget.h"

void UNaviLeaf_DamageFalloffsDisplay::SetFalloffData(const TArray<FDamageFalloff>& Falloffs)
{
	if (!IsValid(ColumnContainer) || !IsValid(ColumnWidgetClass))
	{
		return;
	}

	if (Falloffs.Num() == 0)
	{
		return;
	}
	
	// 1. 기존 위젯 모두 제거
	ColumnContainer->ClearChildren();
	
	// 2. 데이터 순회하며 위젯 생성
	for (int32 i = 0; i < Falloffs.Num(); ++i)
	{
		const FDamageFalloff& Data = Falloffs[i];
		bool bIsLast = (i == Falloffs.Num() - 1);

		if (UNaviWidget_DamageColumn* ColumnWidget = CreateWidget<UNaviWidget_DamageColumn>(this, ColumnWidgetClass); IsValid(ColumnWidget))
		{
			ColumnWidget->SetFalloffData(Data, bIsLast);
			ColumnContainer->AddChild(ColumnWidget);
		}
	}
}
