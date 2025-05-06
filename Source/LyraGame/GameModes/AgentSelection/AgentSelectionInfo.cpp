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
	const int32 FoundIndex = Items.IndexOfByPredicate([&UserName](const FAgentSelectionInfo& Info) {
		return Info.Username == UserName;
	});

	if (FoundIndex != INDEX_NONE)
	{
		Items.RemoveAtSwap(FoundIndex); // 순서 유지가 필요하면 RemoveAt, 속도가 중요하면 RemoveAtSwap
		MarkArrayDirty();
	}
	else
	{
		// UE_LOG(LogAgentSelection, Warning, TEXT("RemoveAgentSelectionInfo: 제거할 사용자 '%s'를 찾지 못했습니다."), *UserName);
	}
}


void FAgentSelectionInfoArray::ChangeAgentSelectionInfo(const FAgentSelectionInfo& AgentSelectionInfo)
{
	FAgentSelectionInfo* ExistingEntry = Items.FindByPredicate([&](const FAgentSelectionInfo& Info) {
		return Info.Username == AgentSelectionInfo.Username;
	}); 

	if (ExistingEntry)
	{
		ExistingEntry->Controller = AgentSelectionInfo.Controller;
		ExistingEntry->TeamID = AgentSelectionInfo.TeamID;
		ExistingEntry->IndexInTeam = AgentSelectionInfo.IndexInTeam;
		ExistingEntry->AgentTag = AgentSelectionInfo.AgentTag;
		ExistingEntry->bConfirmSelection = AgentSelectionInfo.bConfirmSelection;
		
		MarkItemDirty(*ExistingEntry);
	}
	else
	{
		// UE_LOG(LogAgentSelection, Warning, TEXT("ChangeAgentSelectionInfo: 업데이트할 사용자 '%s'를 찾지 못했습니다."), *AgentSelectionInfo.Username);
	}
}


void FAgentSelectionInfoArray::ConfirmAgentSelection(const FString& UserName)
{

	FAgentSelectionInfo* ExistingEntry = Items.FindByPredicate([&UserName](const FAgentSelectionInfo& Info) {
		return Info.Username == UserName;
	});
	
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
