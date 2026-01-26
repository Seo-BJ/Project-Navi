// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NaviInventoryFragment_UIInfo.h"

float UNaviInventoryFragment_UIInfo::GetNumericStat_Implementation(FGameplayTag Tag, bool& bFound) const
{
	if (const float* Val = NumericStats.Find(Tag))
	{
		bFound = true;
		return *Val;
	}
	bFound = false;
	return 0.0f;
}

FText UNaviInventoryFragment_UIInfo::GetTextData_Implementation(FGameplayTag Tag, bool& bFound) const
{
	if (const FText* Val = TextData.Find(Tag))
	{
		bFound = true;
		return *Val;
	}
	bFound = false;
	return FText::GetEmpty();
}