// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LyraGameplayTags.h"
#include "AgentSelectionInfo.generated.h"

USTRUCT(BlueprintType)
struct FAgentSelectionInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAgentSelectionInfo() {}
	FAgentSelectionInfo(const FString& Name) : Username(Name) {}

	UPROPERTY(BlueprintReadWrite)
	FString Username{};

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	int32 TeamID = FGenericTeamId(INDEX_NONE);

	UPROPERTY(BlueprintReadWrite)
	int32 IndexInTeam = -1;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag AgentTag = LyraGameplayTags::Agent_Class;

	UPROPERTY(BlueprintReadWrite)
	bool bConfirmSelection = false;

	void PreReplicatedRemove(const FFastArraySerializer& Serializer);
	void PostReplicatedAdd(const FFastArraySerializer& Serializer);
	void PostReplicatedChange(const FFastArraySerializer& Serializer);
};

USTRUCT()
struct FAgentSelectionInfoArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAgentSelectionInfo> SelectedAgents;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FAgentSelectionInfo, FAgentSelectionInfoArray>(SelectedAgents, DeltaParams, *this);
	}

	void AddAgentSelectionInfo(const FAgentSelectionInfo& AgentSelectionInfo);
	void RemoveAgentSelectionInfo(const FString& UserName);
	void ChangeAgentSelectionInfo(const FAgentSelectionInfo& AgentSelectionInfo);
};

template<>
struct TStructOpsTypeTraits<FAgentSelectionInfoArray> : public TStructOpsTypeTraitsBase2<FAgentSelectionInfo>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


USTRUCT(BlueprintType)
struct FAgentSelection_MessageBase
{
	GENERATED_BODY()

	// 해당 정보와 연관된 PlayerState (Controller에서 얻어옴)
	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	TWeakObjectPtr<APlayerState> AssociatedPlayerState = nullptr;
};

// 에이전트 정보 '추가' 메시지
USTRUCT(BlueprintType)
struct FAgentSelection_AddedMessage : public FAgentSelection_MessageBase
{
	GENERATED_BODY()

	// 추가된 에이전트 정보의 복사본
	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	FAgentSelectionInfo AddedInfo;
};

// 에이전트 정보 '제거' 메시지
USTRUCT(BlueprintType)
struct FAgentSelection_RemovedMessage : public FAgentSelection_MessageBase
{
	GENERATED_BODY()

	// 제거된 플레이어의 Username
	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	FString RemovedUsername;

	// 제거되기 전의 다른 정보들 (필요에 따라 추가)
	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	FGameplayTag RemovedAgentTag;

	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	int32 RemovedTeamID = INDEX_NONE;
};

// 에이전트 정보 '변경' 메시지
USTRUCT(BlueprintType)
struct FAgentSelection_ChangedMessage : public FAgentSelection_MessageBase
{
	GENERATED_BODY()

	// 변경된 후의 에이전트 정보 복사본
	UPROPERTY(BlueprintReadWrite, Category = "Agent Selection Message")
	FAgentSelectionInfo ChangedInfo;
};





