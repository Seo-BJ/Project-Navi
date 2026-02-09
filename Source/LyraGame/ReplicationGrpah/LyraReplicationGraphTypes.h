// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ReplicationGraphTypes.h"
#include "LyraReplicationGraphTypes.generated.h"

// 이 열거형은 액터를 올바른 레플리케이션 노드로 라우팅하는 데 사용되는 메인 열거형입니다. 각 클래스는 하나의 열거형 값에 매핑됩니다.
UENUM()
enum class EClassRepNodeMapping : uint32
{
	NotRouted,						// 어떤 노드에도 매핑되지 않습니다. 특수 케이스 노드(ULyraReplicationGraphNode_PlayerStateFrequencyLimiter 등)에서 처리되는 특수 액터에 사용됩니다.
	RelevantAllConnections,			// AlwaysRelevantNode 또는 AlwaysRelevantStreamingLevelNode 노드로 라우팅됩니다.

	// 아래는 공간화(Spatialized)된 열거형들입니다. ULyraReplicationGraph::IsSpatialized를 참조하세요.

	Spatialize_Static,				// GridNode로 라우팅: 움직이지 않으며 매 프레임 업데이트가 필요 없는 액터들입니다.
	Spatialize_Dynamic,				// GridNode로 라우팅: 자주 움직이며 프레임당 한 번 업데이트되는 액터들입니다.
	Spatialize_Dormancy,			// GridNode로 라우팅: 휴면(Dormant) 상태일 때는 정적(Static)으로 처리하고, 활성 상태일 때는 동적(Dynamic)으로 처리합니다. "활성 상태일 때 움직이는" 액터들을 위한 설정입니다.
};

// 클래스에 직접 할당할 수 있는 액터 클래스 설정입니다. FRepGraphActorTemplateSettings에 매핑될 수도 있습니다.
USTRUCT()
struct FRepGraphActorClassSettings
{
	GENERATED_BODY()

	FRepGraphActorClassSettings() = default;

	// 설정이 적용될 클래스의 이름입니다.
	UPROPERTY(EditAnywhere)
	FSoftClassPath ActorClass;

	// 이 클래스의 RepInfo를 ClassRepNodePolicies 맵에 추가할지 여부입니다.
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddClassRepInfoToMap  = true;

	// 클래스를 ClassRepNodePolicies 맵에 추가할 때 사용할 ClassNodeMapping 값입니다.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddClassRepInfoToMap"))
	EClassRepNodeMapping ClassNodeMapping = EClassRepNodeMapping::NotRouted;

	// RPC_Multicast_OpenChannelForClass 맵에 추가할지 여부입니다.
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddToRPC_Multicast_OpenChannelForClassMap = false;

	// RPC_Multicast_OpenChannelForClass 맵에 추가될 때 실제로 채널을 열지 여부입니다.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddToRPC_Multicast_OpenChannelForClassMap"))
	bool bRPC_Multicast_OpenChannelForClass = true;

	UClass* GetStaticActorClass() const
	{
		UClass* StaticActorClass = nullptr;
		const FString ActorClassNameString = ActorClass.ToString();

		if (FPackageName::IsScriptPackage(ActorClassNameString))
		{
			StaticActorClass = FindObject<UClass>(nullptr, *ActorClassNameString, true);

			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Find Static Class for %s"), *ActorClassNameString);
			}
		}
		else
		{
			// Allow blueprints to be used for custom class settings
			StaticActorClass = (UClass*)StaticLoadObject(UClass::StaticClass(), nullptr, *ActorClassNameString);
			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Load Static Class for %s"), *ActorClassNameString);
			}
		}

		return StaticActorClass;
	}
};