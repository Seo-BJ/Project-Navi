// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraConnectionManager.h"
#include "ReplicationGraph.h"
#include "LyraReplicationGraphTypes.h"
#include "LyraReplicationGraph.generated.h"

class ULyraConnectionManager;
class AGameplayDebuggerCategoryReplicator;
class ULyraReplicationGraphNode_AlwaysRelevant_ForConnection;
class ULyraReplicationGraphNode_AlwaysRelevant_ForTeam;
class UReplicationGraphNode_AlwaysRelevant_WithPending;

DECLARE_LOG_CATEGORY_EXTERN(LogLyraRepGraph, Display, All);

DECLARE_STATS_GROUP(TEXT("Lyra Replication Graph"), STATGROUP_LyraRepGraph, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("RepGraph: Gather Non-Team Visibility"), STAT_LyraRepGraph_GatherNonTeamVisibility, STATGROUP_LyraRepGraph);
DECLARE_CYCLE_STAT(TEXT("RepGraph: Prepare PlayerStates"), STAT_LyraRepGraph_PreparePlayerStates, STATGROUP_LyraRepGraph);
DECLARE_CYCLE_STAT(TEXT("RepGraph: Handle Pending Requests"), STAT_LyraRepGraph_HandlePendingRequests, STATGROUP_LyraRepGraph);
DECLARE_CYCLE_STAT(TEXT("RepGraph: AlwaysRelevantNode Gather"), STAT_LyraRepGraph_AlwaysRelevantNode_Gather, STATGROUP_LyraRepGraph);
DECLARE_CYCLE_STAT(TEXT("RepGraph: Get Trace Targets"), STAT_LyraRepGraph_GetTraceTargets, STATGROUP_LyraRepGraph);
DECLARE_CYCLE_STAT(TEXT("RepGraph: Physical Visibility Trace"), STAT_LyraRepGraph_PhysicalTrace, STATGROUP_LyraRepGraph);

/** Lyra 레플리케이션 그래프 구현체입니다. 상세 노트는 LyraReplicationGraph.cpp를 참조하세요! */
UCLASS(transient, config=Engine)
class ULyraReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	ULyraReplicationGraph();
	
	UPROPERTY()
	TArray<TObjectPtr<UClass>>	AlwaysRelevantClasses; // 항상 모든 플레이어에게 보여야 하는 액터 클래스 목록입니다.
	
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode; // 거리 기반 레플리케이션을 처리하는 메인 2D 그리드 노드입니다.

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantGlobalNode; // 항상 연관된 액터들을 관리하는 전역 리스트 노드입니다.

	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors; // 스트리밍 레벨별 항상 연관된 액터 목록입니다.

	
	virtual void InitGlobalActorClassSettings() override; // 클래스별 레플리케이션 설정(Cull Distance, Frequency 등)을 초기화합니다.
	virtual void InitGlobalGraphNodes() override;      // 그리드 노드, 항상 연관 노드 등 전역 노드들을 생성합니다.
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override; // 새로운 연결(플레이어)이 들어올 때마다 해당 연결을 위한 전용 노드를 생성합니다.

	virtual void RemoveClientConnection(UNetConnection* NetConnection) override;
	virtual void ResetGameWorldState() override;

	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override; // 액터가 스폰되었을 때 적절한 노드에 배치합니다.
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override; // 액터가 소멸되었을 때 노드에서 제거합니다.
	
	void SetTeamForPlayerController(APlayerController* PlayerController, int32 Team);
	void HandlePendingActorsAndTeamRequests();

#if WITH_GAMEPLAY_DEBUGGER
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	void PrintRepNodePolicies();

private:
	void AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping);
	void RegisterClassRepNodeMapping(UClass* Class);
	EClassRepNodeMapping GetClassNodeMapping(UClass* Class) const;

	void RegisterClassReplicationInfo(UClass* Class);
	bool ConditionalInitClassReplicationInfo(UClass* Class, FClassReplicationInfo& ClassInfo);
	void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool Spatialize) const;

	EClassRepNodeMapping GetMappingPolicy(UClass* Class);

	bool IsSpatialized(EClassRepNodeMapping Mapping) const { return Mapping >= EClassRepNodeMapping::Spatialize_Static; }

	TClassMap<EClassRepNodeMapping> ClassRepNodePolicies;
	
	/** ULyraReplicationGraph::InitGlobalActorClassSettings 코드에서 레플리케이션 설정이 명시적으로 설정된 클래스들입니다. */
	TArray<UClass*> ExplicitlySetClasses;

	ULyraConnectionManager* GetLyraConnectionManagerFromActor(const AActor* Actor);

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_AlwaysRelevant_WithPending> AlwaysRelevantNode;
	
	// -----------------------------------------------
	// LyraTeamSubsystem 기반 Replication Graph 시스템
	// -----------------------------------------------

	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> PendingConnectionActors;
	TArray<TTuple<int32, APlayerController*>> PendingTeamRequests;
	
	friend ULyraReplicationGraphNode_AlwaysRelevant_ForTeam;
	FTeamConnectionListMap TeamConnectionListMap;

public:
	int32 GetTeamID(const UNetReplicationGraphConnection* RepGraphConnection) const;
	
};

