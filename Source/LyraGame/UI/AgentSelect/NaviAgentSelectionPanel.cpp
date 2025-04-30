// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentSelectionPanel.h"
#include "Components/ScrollBox.h"
#include "Components/GridPanel.h"

void UNaviAgentSelectionPanel::NativePreConstruct()
{
	Super::NativePreConstruct();

	FillGridPanel();
	
}

void UNaviAgentSelectionPanel::NativeConstruct()
{
	Super::NativeConstruct();
}






/*
void UNaviAgentSelectionPanel::FillGridPanel_Implem()
{
	/*
	if (!AgentDescriptionData  || !IsValid(AgentIcons_ScrollBox) || !IsValid(AgentIcons_GridPanel)) return;

    TArray<FName> RowNames = AgentDescriptionData->GetRowNames();
    int CurrentColumn = 0;
    int CurrentRow = 0;
    for (int i = 0; i < RowNames.Num(); i++)
    {
        // Get Data Table Row
        if (CurrentColumn == MaxColumn)
        {
            CurrentRow += 1;
            CurrentColumn = 0;
            
        }
        UUserWidget* AgentIconButton = CreateWidget<UUserWidget>(this, AgentIconButtonClass);
        AgentIcons_GridPanel->AddChildToGrid(AgentIconButton, CurrentRow, CurrentColumn);
        CurrentRow += 1;
    }
    
}
*/