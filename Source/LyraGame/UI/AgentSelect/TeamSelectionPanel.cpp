// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamSelectionPanel.h"
#include "Components/HorizontalBox.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "GameModes/LyraGameState.h"
#include "TeamSelectionIcon.h"
#include "Player/LyraPlayerState.h"



void UTeamSelectionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	PopulateTeamSelectionIcons();
}

void UTeamSelectionPanel::PopulateTeamSelectionIcons()
{
	    // 0. 필요한 컴포넌트 및 클래스 유효성 검사
    if (!IsValid(TeamAgentIcons_HorizontalBox))
    {
        UE_LOG(LogTemp, Error, TEXT("UTeamSelectionPanel::PopulateTeamSelectionIcons: TeamAgentIcons_UniformGridPanel is not valid!"));
        return;
    }
     if (!TeamSelectionIconClass) // TeamSelectionIconClass가 설정되었는지 확인
    {
        UE_LOG(LogTemp, Error, TEXT("UTeamSelectionPanel::PopulateTeamSelectionIcons: TeamSelectionIconClass is not set in the Blueprint!"));
        return;
    }
    
    APlayerController* LocalPC = GetOwningPlayer();
    if (!IsValid(LocalPC)) return; 

    ALyraPlayerState* LocalPlayerState = LocalPC->GetPlayerState<ALyraPlayerState>();
    if (!IsValid(LocalPlayerState)) return; 

    const int32 LocalPlayerTeamId = LocalPlayerState->GetTeamId();
    if (LocalPlayerTeamId == FGenericTeamId(INDEX_NONE)) return;

    const int32 LocalIndexInTeam = LocalPlayerState->GetIndexInTeam();
    if (LocalIndexInTeam == -1) return; 
    
    AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
    if (!IsValid(GameState)) return; 

    // 3. 같은 팀 PlayerState 수집
    TArray<ALyraPlayerState*> Teammates;
    for (APlayerState* PS : GameState->PlayerArray)
    {
        ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS);
        if (IsValid(LyraPS) &&
            LyraPS->GetTeamId() == LocalPlayerTeamId &&
            LyraPS->GetIndexInTeam() != -1 &&
            LocalPlayerState != LyraPS &&
            LocalIndexInTeam != LyraPS->GetIndexInTeam())
        {
            Teammates.Add(LyraPS);
        }
    }
    
    Teammates.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
        return A.GetIndexInTeam() < B.GetIndexInTeam();
    });

    int TotalTeammates = 1 + Teammates.Num();
    switch (TotalTeammates)
    {
    case 1:
        First_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Second_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Third_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Fourth_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        break;
    case 2:
        Second_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Third_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Fourth_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        break;
    case 3:
        Third_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        Fourth_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        break;
    case 4:
        Fourth_SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
        break;
    case 5:
        break;
    default:
        break;
    }
    Own_SelectionPanel->SerUserNameText(LocalPlayerState->GetPlayerUserName());
    for (int32 i = 0; i < Teammates.Num(); ++i)
    {
        ALyraPlayerState* TeammatePS = Teammates[i];
        if (!IsValid(TeammatePS)) continue;
        switch (i)
        {
        case 0:
            First_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            break;
        case 1:
            Second_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            break;
        case 2:
            Third_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            break;
        case 3:
            Fourth_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            break;
        default:
            break;
        }
        
        
    }
}
