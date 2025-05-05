// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentSelectionManagerComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"

#include "NaviAgentSelectionManagerComponent.h" // 컴포넌트 헤더
#include "AgentSelectionInfo.h"              // FAgentSelectionInfo 구조체 헤더// 메시지 구조체 헤더 (이전에 정의)
#include "GameFramework/GameplayMessageSubsystem.h"

#include "LyraGameplayTags.h"              // Gameplay Tag 헤더 (실제 경로/이름으로 수정)
#include "GameFramework/PlayerState.h"       // APlayerState
#include "GameFramework/Controller.h"        // AController
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UNaviAgentSelectionManagerComponent::UNaviAgentSelectionManagerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
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
	AgentSelectionInfoArray.RegisterWithOwner(this);
}




void UNaviAgentSelectionManagerComponent::ServerHandleAgentSelection(const FAgentSelectionInfo& Info)
{
    if (HasAuthority())
    {
        AgentSelectionInfoArray.AddAgentSelectionInfo(Info);
    }
}


void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoAdded(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;
    UE_LOG(LogTemp, Log, TEXT("UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoAdded (Player: %s)"), *PlayerName);

    UWorld* World = GetWorld(); // 컴포넌트에서 월드 컨텍스트 가져오기
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoAdded: 유효하지 않은 월드 컨텍스트 (Player: %s)."), *PlayerName);
        return;
    }

	UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!MessageSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoAdded: GameplayMessageSubsystem 가져오기 실패 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 채널 태그 가져오기 (실제 프로젝트의 태그로 교체)
    FGameplayTag ChannelTag = LyraGameplayTags::Agent_Selection_Added;
    // FGameplayTag ChannelTag = FGameplayTag::RequestGameplayTag(FName("AgentSelection.Added")); // 임시 태그 사용 시

    if (!ChannelTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoAdded: AgentSelection.Added 채널 태그가 유효하지 않음 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 페이로드 준비
    FAgentSelection_AddedMessage MessagePayload;
    MessagePayload.AddedInfo = Info; // 전달받은 추가된 정보 복사

    // 연관된 PlayerState 설정 시도
    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }

    // 메시지 브로드캐스트
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);

    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoAdded: AgentSelection.Added 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoRemoved(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;
     UE_LOG(LogTemp, Log, TEXT("UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoRemoved (Player: %s)"), *PlayerName);

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoRemoved: 유효하지 않은 월드 컨텍스트 (Player: %s)."), *PlayerName);
        return;
    }

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!MessageSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoRemoved: GameplayMessageSubsystem 가져오기 실패 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 채널 태그 가져오기 (실제 프로젝트의 태그로 교체)
    FGameplayTag ChannelTag = LyraGameplayTags::Agent_Selection_Removed;
    // FGameplayTag ChannelTag = FGameplayTag::RequestGameplayTag(FName("AgentSelection.Removed")); // 임시 태그 사용 시

    if (!ChannelTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoRemoved: AgentSelection.Removed 채널 태그가 유효하지 않음 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 페이로드 준비 (제거되기 전의 정보 사용)
    FAgentSelection_RemovedMessage MessagePayload;
    MessagePayload.RemovedUsername = Info.Username;
    MessagePayload.RemovedAgentTag = Info.AgentTag;
    MessagePayload.RemovedTeamID = Info.TeamID;

    // 연관된 PlayerState 설정 시도
    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }

    // 메시지 브로드캐스트
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);

    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoRemoved: AgentSelection.Removed 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}

void UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoChanged(const FAgentSelectionInfo& Info)
{
    const FString PlayerName = Info.Username;
    UE_LOG(LogTemp, Log, TEXT("UNaviAgentSelectionManagerComponent::OnAgentSelectionInfoChanged (Player: %s)"), *PlayerName);

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoChanged: 유효하지 않은 월드 컨텍스트 (Player: %s)."), *PlayerName);
        return;
    }

    UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(this);
    if (!MessageSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoChanged: GameplayMessageSubsystem 가져오기 실패 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 채널 태그 가져오기 (실제 프로젝트의 태그로 교체)
    FGameplayTag ChannelTag = LyraGameplayTags::Agent_Selection_Changed;
    // FGameplayTag ChannelTag = FGameplayTag::RequestGameplayTag(FName("AgentSelection.Changed")); // 임시 태그 사용 시

    if (!ChannelTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("OnAgentSelectionInfoChanged: AgentSelection.Changed 채널 태그가 유효하지 않음 (Player: %s)."), *PlayerName);
        return;
    }

    // 메시지 페이로드 준비
    FAgentSelection_ChangedMessage MessagePayload;
    MessagePayload.ChangedInfo = Info; // 전달받은 변경된 정보 복사

    // 연관된 PlayerState 설정 시도
    if (Info.Controller)
    {
        MessagePayload.AssociatedPlayerState = Info.Controller->GetPlayerState<APlayerState>();
    }

    // 메시지 브로드캐스트
    MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);

    UE_LOG(LogTemp, Log, TEXT("OnAgentSelectionInfoChanged: AgentSelection.Changed 메시지 브로드캐스트 완료 (Player: %s)."), *PlayerName);
}
