// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_StatDisplay.h"
#include "Components/TextBlock.h"
#include "Inventory/NaviInventoryFragment_UIInfo.h"
#include "Inventory/LyraInventoryItemDefinition.h"

void UNaviLeaf_StatDisplay::UpdateView(const ULyraInventoryItemDefinition* ItemDef)
{
	Super::UpdateView(ItemDef);

	if (!ItemDef)
	{
		SetIsCollapsed(true);
		return;
	}

	// 1. Update Label (Static or Localized text)
	UpdateLabel();

	// 2. Fetch Data
	const UNaviInventoryFragment_UIInfo* UIInfo = Cast<UNaviInventoryFragment_UIInfo>(
		ItemDef->FindFragmentByClass(UNaviInventoryFragment_UIInfo::StaticClass())
	);

	bool bFound = false;
	float StatValue = 0.0f;

	if (UIInfo)
	{
		StatValue = UIInfo->GetNumericStat(TargetTag, bFound);
	}

	// 3. Update Value or Collapse
	if (bFound)
	{
		UpdateValue(StatValue);
		SetIsCollapsed(false);
	}
	else
	{
		if (bCollapseIfMissing)
		{
			SetIsCollapsed(true);
		}
		else
		{
			// Optional: Clear text or show default "0" or "-"
			if (Text_Value)
			{
				Text_Value->SetText(FText::FromString(TEXT("-")));
			}
			SetIsCollapsed(false);
		}
	}
}

void UNaviLeaf_StatDisplay::SetStatValue(float Value)
{
	//	UpdateLabel();
	UpdateValue(Value);
	SetIsCollapsed(false);
}

void UNaviLeaf_StatDisplay::UpdateLabel()
{
	if (Text_Label)
	{
		if (!LabelText.IsEmpty())
		{
			Text_Label->SetText(LabelText);
			Text_Label->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_Label->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UNaviLeaf_StatDisplay::UpdateValue(float Value)
{
	if (Text_Value)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = MinFractionalDigits;
		Options.MaximumFractionalDigits = MaxFractionalDigits;

		Text_Value->SetText(FText::AsNumber(Value, &Options));
	}
}

void UNaviLeaf_StatDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (!LabelText.IsEmpty())
	{
		Text_Label->SetText(LabelText);
		Text_Label->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	}
}
