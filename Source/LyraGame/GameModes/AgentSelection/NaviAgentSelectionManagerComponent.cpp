// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentSelectionManagerComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"

#include "AgentSelectionInfo.h"              
#include "GameFramework/GameplayMessageSubsystem.h"

#include "LyraGameplayTags.h"             
#include "GameFramework/PlayerState.h"       
#include "GameFramework/Controller.h"       
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerState.h"

UNaviAgentSelectionManagerComponent::UNaviAgentSelectionManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNaviAgentSelectionManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UNaviAgentSelectionManagerComponent, AgentSelectionInfoArray);
}

void UNaviAgentSelectionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	
}

void UNaviAgentSelectionManagerComponent::OnRep_AgentSelectionInfoArray()
{
}

void UNaviAgentSelectionManagerComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    int PlayerNum = GameState->PlayerArray.Num();
	AgentSelectionInfoArray.RegisterWithOwner(this, PlayerNum);
}




void UNaviAgentSelectionManagerComponent::HandleAgentSelectionRequest(const FAgentSelectionInfo& Info)
{
    if (HasAuthority())
    {
        AgentSelectionInfoArray.AddAgentSelectionInfo(Info);
        ALyraPlayerState* LyraPS = Info.Controller->GetPlayerState<ALyraPlayerState>();
        if (!IsValid(LyraPS)) return;
        LyraPS->SetSelectedAgentTag(Info.AgentTag);
    }
}

void UNaviAgentSelectionManagerComponent::HandleConfirmSelectionRequest(const FString& UserName)
{
    if (HasAuthority())
    {
        AgentSelectionInfoArray.ConfirmAgentSelection(UserName);
    }
}


void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoAdded(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;

    UWorld* World = GetWorld(); 
    if (!IsValid(World)) return;

	UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!IsValid(MessageSubsystem)) return;
    
    FGameplayTag ChannelTag = NaviGameplayTags::Agent_Selection_Added;
    FAgentSelection_AddedMessage MessagePayload;
    MessagePayload.AddedInfo = Info; 

    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }
    
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoRemoved: Agent_Selection_Added 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoRemoved(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;

    UWorld* World = GetWorld(); 
    if (!IsValid(World)) return;

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!IsValid(MessageSubsystem)) return;
    
    FGameplayTag ChannelTag = NaviGameplayTags::Agent_Selection_Removed;
    FAgentSelection_RemovedMessage MessagePayload;
    MessagePayload.RemovedUsername = Info.Username;
    MessagePayload.RemovedAgentTag = Info.AgentTag;
    MessagePayload.RemovedTeamID = Info.TeamID;

    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }
    
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoRemoved: Agent_Selection_Removed 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoChanged(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;

    UWorld* World = GetWorld(); 
    if (!IsValid(World)) return;

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!IsValid(MessageSubsystem)) return;
    
    FGameplayTag ChannelTag = NaviGameplayTags::Agent_Selection_Changed;
    FAgentSelection_ChangedMessage MessagePayload;
    MessagePayload.ChangedInfo = Info; 
    
    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }
    
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoChanged: Agent_Selection_Changed 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAgentSelectionConfirmed(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;

    UWorld* World = GetWorld(); 
    if (!IsValid(World)) return;

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!IsValid(MessageSubsystem)) return;

    FGameplayTag ChannelTag = NaviGameplayTags::Agent_Selection_Confirm;
    FAgentSelection_ChangedMessage MessagePayload;
    MessagePayload.ChangedInfo = Info; 
    
    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }
    
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoChanged: Agent_Selection_Confirm 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAllPlayerConfirmedAgentSelection(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;

    UWorld* World = GetWorld(); 
    if (!IsValid(World)) return;

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!IsValid(MessageSubsystem)) return;

    FGameplayTag ChannelTag = NaviGameplayTags::Agent_Selection_AllPlayerConfirm;
    FAgentSelection_ChangedMessage MessagePayload;
    MessagePayload.ChangedInfo = Info; 

    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }
    
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoChanged: Agent_Selection_AllPlayerConfirm 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

