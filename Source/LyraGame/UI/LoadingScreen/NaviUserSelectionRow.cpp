// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviUserSelectionRow.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBoxSlot.h"
#include "NaviAgentIcon.h"

void UNaviUserSelectionRow::UpdateUserSelectionRow(TSoftObjectPtr<UTexture2D> AgentImage, const FText& AgentName,
	const FString& UserName)
{
	if (AgentImage.IsNull()) return;
	
	AgentIcon->UpdateAgentIcomImage(AgentImage);
	AgentName_Text->SetText(AgentName);
	UserName_Text->SetText(FText::FromString(UserName));
}

void UNaviUserSelectionRow::UpdateHorizontalAlignment(bool bOffensiveTeam)
{
	if (bOffensiveTeam)
	{
		UWidgetLayoutLibrary::SlotAsHorizontalBoxSlot(AgentIcon)->SetHorizontalAlignment(HAlign_Left);
	}
	else
	{
		UWidgetLayoutLibrary::SlotAsHorizontalBoxSlot(AgentIcon)->SetHorizontalAlignment(HAlign_Right);
	}
}
