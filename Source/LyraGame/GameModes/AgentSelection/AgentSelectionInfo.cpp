// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentSelectionInfo.h"
#include "NaviAgentSelectionManagerComponent.h"
#include "LyraLogChannels.h"
#include "NiagaraValidationRule.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void FAgentSelectionInfo::PreReplicatedRemove(const FAgentSelectionInfoArray& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAgentSelectionInfoRemoved(*this);
	}
}

void FAgentSelectionInfo::PostReplicatedAdd(const FAgentSelectionInfoArray& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnAgentSelectionInfoAdded(*this);
	}
}

void FAgentSelectionInfo::PostReplicatedChange(const FAgentSelectionInfoArray& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		if (this->bConfirmSelection == true)
		{
			InArraySerializer.Owner->OnAgentSelectionConfirmed(*this);
			if (InArraySerializer.HasAllPlayerConfirmedAgentSelection())
			{
				InArraySerializer.Owner->OnAllPlayerConfirmedAgentSelection(*this);
			}
		}
		else
		{
			InArraySerializer.Owner->OnAgentSelectionInfoChanged(*this);
		}
	}
}





void FAgentSelectionInfoArray::RegisterWithOwner(UNaviAgentSelectionManagerComponent* InOwner, int InTotalPlayerNum)
{
	Owner = InOwner;
	TotalPlayerNum = InTotalPlayerNum;
}

void FAgentSelectionInfoArray::AddAgentSelectionInfo(const FAgentSelectionInfo& AgentSelectionInfo)
{
	FAgentSelectionInfo* ExistingEntryByName = Items.FindByPredicate([&](const FAgentSelectionInfo& Info) {
		return Info.Username == AgentSelectionInfo.Username;
	});

	if (ExistingEntryByName)
	{
		ChangeAgentSelectionInfo(AgentSelectionInfo);
		// UE_LOG(LogAgentSelection, Log, TEXT("AddAgentSelectionInfo: 사용자 '%s'가 이미 존재합니다. ChangeAgentSelectionInfo를 호출합니다."), *AgentSelectionInfo.Username);
		return; 
	}

	const FAgentSelectionInfo* ConflictingEntry = Items.FindByPredicate([&](const FAgentSelectionInfo& OtherInfo) {
	   return (OtherInfo.AgentTag == AgentSelectionInfo.AgentTag && OtherInfo.bConfirmSelection);
   }); 
	
	if (ConflictingEntry)
	{
		return; 
	}
	Items.Add(AgentSelectionInfo);
	MarkArrayDirty();
}
void FAgentSelectionInfoArray::RemoveAgentSelectionInfo(const FString& UserName)
{

	// 1. 제거할 사용자의 인덱스를 찾습니다. (TArray::IndexOfByPredicate는 그대로 사용)
	const int32 FoundIndex = Items.IndexOfByPredicate([&UserName](const FAgentSelectionInfo& Info) {
		return Info.Username == UserName;
	});

	// 2. 사용자를 찾았다면 해당 인덱스의 항목을 제거합니다.
	if (FoundIndex != INDEX_NONE)
	{
		// UE_LOG(LogAgentSelection, Log, TEXT("RemoveAgentSelectionInfo: 사용자 '%s' (인덱스: %d)를 제거합니다."), *UserName, FoundIndex);
		Items.RemoveAt(FoundIndex); // 순서 유지가 필요하면 RemoveAt, 속도가 중요하면 RemoveAtSwap

		// 3. 배열 구조가 변경되었으므로 Serializer에게 알립니다.
		MarkArrayDirty();
	}
	else
	{
		// UE_LOG(LogAgentSelection, Warning, TEXT("RemoveAgentSelectionInfo: 제거할 사용자 '%s'를 찾지 못했습니다."), *UserName);
	}
}


void FAgentSelectionInfoArray::ChangeAgentSelectionInfo(const FAgentSelectionInfo& AgentSelectionInfo)
{
	// 1. 변경할 사용자의 기존 항목에 대한 포인터를 찾습니다. (TArray::FindByPredicate 사용)
	FAgentSelectionInfo* ExistingEntry = Items.FindByPredicate([&](const FAgentSelectionInfo& Info) {
		return Info.Username == AgentSelectionInfo.Username;
	}); // Algo:: 대신 SelectedAgents. 으로 변경

	// 2. 기존 항목을 찾았다면 내용을 업데이트합니다.
	if (ExistingEntry)
	{
		// UE_LOG(LogAgentSelection, Log, TEXT("ChangeAgentSelectionInfo: 사용자 '%s'의 정보를 업데이트합니다."), *AgentSelectionInfo.Username);

		// 식별자인 Username을 제외한 다른 속성들을 업데이트합니다.
		ExistingEntry->Controller = AgentSelectionInfo.Controller;
		ExistingEntry->TeamID = AgentSelectionInfo.TeamID;
		ExistingEntry->IndexInTeam = AgentSelectionInfo.IndexInTeam;
		ExistingEntry->AgentTag = AgentSelectionInfo.AgentTag;
		ExistingEntry->bConfirmSelection = AgentSelectionInfo.bConfirmSelection;
		// FAgentSelectionInfo에 다른 업데이트 필요한 속성이 있다면 여기에 추가

		// 3. 특정 항목의 내용이 변경되었으므로 Serializer에게 알립니다.
		MarkItemDirty(*ExistingEntry); // 변경된 항목 자체를 전달
	}
	else
	{
		// UE_LOG(LogAgentSelection, Warning, TEXT("ChangeAgentSelectionInfo: 업데이트할 사용자 '%s'를 찾지 못했습니다."), *AgentSelectionInfo.Username);
	}
}


void FAgentSelectionInfoArray::ConfirmAgentSelection(const FString& UserName)
{
	// 1. 변경할 사용자의 기존 항목에 대한 포인터를 찾습니다. (TArray::FindByPredicate 사용)
	FAgentSelectionInfo* ExistingEntry = Items.FindByPredicate([&UserName](const FAgentSelectionInfo& Info) {
		return Info.Username == UserName;
	});

	// 2. 기존 항목을 찾았다면 내용을 업데이트합니다.
	if (ExistingEntry)
	{
		ExistingEntry->bConfirmSelection = true;
		MarkItemDirty(*ExistingEntry); // 변경된 항목 자체를 전달
	}
	else
	{
		// UE_LOG(LogAgentSelection, Warning, TEXT("ChangeAgentSelectionInfo: 업데이트할 사용자 '%s'를 찾지 못했습니다."), *AgentSelectionInfo.Username);
	}
	if (HasAllPlayerConfirmedAgentSelection())
	{
		UGameplayMessageSubsystem* MessageSubsystem = &UGameplayMessageSubsystem::Get(Owner);
		if (!IsValid(MessageSubsystem)) return;

		FGameplayTag ChannelTag = LyraGameplayTags::Agent_Selection_AllPlayerConfirm;
		FAgentSelection_ChangedMessage MessagePayload;
		
		MessageSubsystem->BroadcastMessage(ChannelTag, MessagePayload);
	}
}

bool FAgentSelectionInfoArray::HasAllPlayerConfirmedAgentSelection() const
{
	if (Items.Num() != TotalPlayerNum) return false;
	for (FAgentSelectionInfo Info : Items)
	{
		if (Info.bConfirmSelection == false)
		{
			return false;
		}
	}
	return true;
}
