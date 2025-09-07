// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamSelectionPanel.h"
#include "Components/HorizontalBox.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "GameModes/LyraGameState.h"
#include "TeamSelectionIcon.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/AgentSelection/AgentSelectionInfo.h"
#include "Player/LyraPlayerState.h"
#include "Engine/StreamableManager.h"
#include "System/LyraAssetManager.h"
#include "Engine/DataTable.h"
#include "UI/Data/NaviUIDataTypes.h"

void UTeamSelectionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	PopulateTeamSelectionIcons();
    
}

void UTeamSelectionPanel::UpdateSelectionPanel(FAgentSelectionInfo Info)
{
    if (!AgentDescriptionData) return;

    APlayerController* LocalPC = GetOwningPlayer();
    if (!IsValid(LocalPC)) return; 

    ALyraPlayerState* LocalPlayerState = LocalPC->GetPlayerState<ALyraPlayerState>();
    if (!IsValid(LocalPlayerState)) return; 

    const int32 LocalPlayerTeamId = LocalPlayerState->GetTeamId();
    if (LocalPlayerTeamId == FGenericTeamId(INDEX_NONE)) return;

    if (LocalPlayerTeamId != Info.TeamID) return;
    
    
    TObjectPtr<UTeamSelectionIcon>* TeamSelectionIcon = TeamSelectionIconsMap.Find(Info.Username);
    if (TeamSelectionIcon == nullptr) return;
    
    const FName AgentTagName = Info.AgentTag.GetTagName();
    const FString AgentNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *AgentTagName.ToString(), *AgentDescriptionData->GetName(), *GetName());

    FNaviAgentDescriptionData* FoundAgentDescriptionRow = AgentDescriptionData->FindRow<FNaviAgentDescriptionData>(AgentTagName, AgentNameString);
    if (FoundAgentDescriptionRow)
    {
        TeamSelectionIcon->Get()->UpdateSelectionIcon(nullptr, FoundAgentDescriptionRow->AgentName); // 이름 먼저 설정, 아이콘은 로딩 전

        // 2. 아이콘 텍스처 비동기 로드 요청
        TSoftObjectPtr<UTexture2D> IconSoftPtr = FoundAgentDescriptionRow->Icon;

        if (IconSoftPtr.IsNull()) 
        {
            UE_LOG(LogTemp, Log, TEXT("UpdateSelectionPanel: Icon SoftPtr is null for agent %s."), *FoundAgentDescriptionRow->AgentName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UpdateSelectionPanel: Requesting async load for icon %s"), *IconSoftPtr.ToString());

            FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
            
            TWeakObjectPtr<UTeamSelectionIcon> WeakIconPtr = TeamSelectionIcon->Get();
            FText AgentNameText = FoundAgentDescriptionRow->AgentName; 

            StreamableManager.RequestAsyncLoad(IconSoftPtr.ToSoftObjectPath(),
                FStreamableDelegate::CreateLambda([WeakIconPtr, IconSoftPtr, AgentNameText]()
                {
                    UTeamSelectionIcon* StrongIcon = WeakIconPtr.Get(); 
                    if (StrongIcon)
                    {
                        UTexture2D* LoadedTexture = IconSoftPtr.Get(); 
                        if (IsValid(LoadedTexture))
                        {
                            StrongIcon->UpdateSelectionIcon(LoadedTexture, AgentNameText);
                            UE_LOG(LogTemp, Log, TEXT("UpdateSelectionPanel: Async load complete for %s, icon updated."), *AgentNameText.ToString());
                        }
                        else
                        {
                             UE_LOG(LogTemp, Warning, TEXT("UpdateSelectionPanel: Async load complete for %s, but failed to get Texture from %s."), *AgentNameText.ToString(), *IconSoftPtr.ToString());
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("UpdateSelectionPanel: Async load complete for %s, but Icon Widget was destroyed."), *AgentNameText.ToString());
                    }
                })
            );
        }
    }
}

void UTeamSelectionPanel::PopulateTeamSelectionIcons()
{
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
    TeamSelectionIconsMap.Add(LocalPlayerState->GetPlayerUserName(), Own_SelectionPanel);
    for (int32 i = 0; i < Teammates.Num(); ++i)
    {
        ALyraPlayerState* TeammatePS = Teammates[i];
        if (!IsValid(TeammatePS)) continue;
        switch (i)
        {
        case 0:
            First_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            TeamSelectionIconsMap.Add(TeammatePS->GetPlayerUserName(), First_SelectionPanel);
            break;
        case 1:
            Second_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            TeamSelectionIconsMap.Add(TeammatePS->GetPlayerUserName(), Second_SelectionPanel);
            break;
        case 2:
            Third_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            TeamSelectionIconsMap.Add(TeammatePS->GetPlayerUserName(), Third_SelectionPanel);
            break;
        case 3:
            Fourth_SelectionPanel->SerUserNameText(TeammatePS->GetPlayerUserName());
            TeamSelectionIconsMap.Add(TeammatePS->GetPlayerUserName(), Fourth_SelectionPanel);
            break;
        default:
            break;
        }
        
        
    }
}
