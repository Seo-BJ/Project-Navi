// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviTeamColumn.h"

#include "Player/LyraPlayerState.h"

#include "NaviUserSelectionRow.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "UI/Data/NaviUIDataTypes.h"

void UNaviTeamColumn::NativeConstruct()
{
	Super::NativeConstruct();
	PopulateUserSelectionRow();
}

void UNaviTeamColumn::PopulateUserSelectionRow()
{
	if (!UserSelectionRowClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UNaviTeamColumn::PopulatePlayerEntries: UserSelectionRowClass가 설정되지 않았습니다!"));
        return;
    }
    UserSelections_VerticalBox->ClearChildren();

    APlayerController* LocalPC = GetOwningPlayer();
    if (!IsValid(LocalPC)) return; 

    ALyraPlayerState* LocalPlayerState = LocalPC->GetPlayerState<ALyraPlayerState>();
    if (!IsValid(LocalPlayerState)) return; 

    const int32 LocalPlayerTeamId = LocalPlayerState->GetTeamId();
    if (LocalPlayerTeamId == FGenericTeamId(INDEX_NONE)) return;

    const int32 LocalIndexInTeam = LocalPlayerState->GetIndexInTeam();
    if (LocalIndexInTeam == -1) return;

    // @ToDo: 1 또는 2 하드 코딩 대신..
    UNaviUserSelectionRow* LocalPlayerRow = CreateWidget<UNaviUserSelectionRow>(this, UserSelectionRowClass);
    if (IsValid(LocalPlayerRow))
    {
        AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
        if (!IsValid(GameState)) return;
        
        if (LocalPlayerTeamId == FGenericTeamId(1) && bOffensiveTeam == true || LocalPlayerTeamId == FGenericTeamId(2) && bOffensiveTeam == false)
        {
            FNaviAgentDescriptionData* FoundData = GetAgentDescriptionData(LocalPlayerState->GetSelectedAgentTag());
            if (FoundData == nullptr) return;
            LocalPlayerRow->UpdateUserSelectionRow(FoundData->Icon, FoundData->AgentName, LocalPlayerState->GetPlayerName());
            LocalPlayerRow->UpdateHorizontalAlignment(bOffensiveTeam);
            UserSelections_VerticalBox->AddChildToVerticalBox(LocalPlayerRow);
            
            TArray<ALyraPlayerState*> Teammates;
            for (APlayerState* PS : GameState->PlayerArray)
            {
                ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS);
    
                if (IsValid(LyraPS) &&
                    LyraPS->GetTeamId() == LocalPlayerTeamId &&
                    LyraPS != LocalPlayerState && 
                    LyraPS->GetIndexInTeam() != -1) 
                {
                    Teammates.Add(LyraPS);
                }
            }
            
            Teammates.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
                return A.GetIndexInTeam() < B.GetIndexInTeam();
            });
            
            CreateUserSelectionRows(Teammates);
        }
        else
        {
            TArray<ALyraPlayerState*> OtherTeammates;
            for (APlayerState* PS : GameState->PlayerArray)
            {
                ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS);
    
                if (IsValid(LyraPS) &&
                    LyraPS->GetTeamId() != LocalPlayerTeamId &&
                    LyraPS != LocalPlayerState && 
                    LyraPS->GetIndexInTeam() != -1) 
                {
                    OtherTeammates.Add(LyraPS);
                }
            }

            OtherTeammates.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
                return A.GetIndexInTeam() < B.GetIndexInTeam();
            });
            
            CreateUserSelectionRows(OtherTeammates);
        }
    }
}


void UNaviTeamColumn::CreateUserSelectionRows(TArray<ALyraPlayerState*> Teammates)
{
    for (ALyraPlayerState* TeammatePS : Teammates)
    {
        if (!IsValid(TeammatePS)) continue;

        UNaviUserSelectionRow* TeammateRow = CreateWidget<UNaviUserSelectionRow>(this, UserSelectionRowClass);
        if (IsValid(TeammateRow))
        {
            FNaviAgentDescriptionData* FoundData = GetAgentDescriptionData(TeammatePS->GetSelectedAgentTag());
            if (FoundData == nullptr) return;
            TeammateRow->UpdateUserSelectionRow(FoundData->Icon, FoundData->AgentName, TeammatePS->GetPlayerName());
            TeammateRow->UpdateHorizontalAlignment(bOffensiveTeam);
            UserSelections_VerticalBox->AddChildToVerticalBox(TeammateRow);
            
        }
    }
}

FNaviAgentDescriptionData* UNaviTeamColumn::GetAgentDescriptionData(const FGameplayTag& TargetAgentTag)
{
    if (!AgentDescriptionData) return nullptr;
    
    const FName AgentTagName = TargetAgentTag.GetTagName();
    const FName ClassTagName = TargetAgentTag.RequestDirectParent().GetTagName();

    const FString AgentNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *AgentTagName.ToString(), *AgentDescriptionData->GetName(), *GetName());

    FNaviAgentDescriptionData* FoundAgentDescriptionRow = AgentDescriptionData->FindRow<FNaviAgentDescriptionData>(AgentTagName, AgentNameString);
    if (FoundAgentDescriptionRow)
    {
        return FoundAgentDescriptionRow;
    }
    return nullptr;
}

