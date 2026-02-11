// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "LyraConnectionManager.generated.h"

class ULyraReplicationGraphNode_AlwaysRelevant_ForConnection;
class ULyraReplicationGraphNode_AlwaysRelevant_ForTeam;



/**
 * 
 */
UCLASS()
class LYRAGAME_API ULyraConnectionManager : public UNetReplicationGraphConnection
{
	GENERATED_BODY()

public:
	// UReplicationGraphNode_AlwaysRelevant_ForConnection는 연결에 항상 관련이 있는 액터들을 위해 에픽에서 제공하는 노드 타입입니다.
	//UPROPERTY()
	//ULyraReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantForConnectionNode;
	
	UPROPERTY()
	TObjectPtr<ULyraReplicationGraphNode_AlwaysRelevant_ForTeam> TeamConnectionNode;

	int32 Team = -1;

	UPROPERTY()
	TWeakObjectPtr<APawn> Pawn = nullptr;
};

struct FTeamConnectionListMap : TMap<int32, TArray<ULyraConnectionManager*>>
{
	TArray<ULyraConnectionManager*>* GetConnectionArrayForTeam(int32 Team);
	TArray<ULyraConnectionManager*> GetVisibleConnectionArrayForNonTeam(const APawn* Pawn, int32 Team);
	
	void AddConnectionToTeam(int32 Team, ULyraConnectionManager* ConnManager);
	void RemoveConnectionFromTeam(int32 Team, ULyraConnectionManager* ConnManager);

protected:
	/** 가시성 트레이스의 시작 위치를 가져옵니다 (예: 카메라 위치) */
	FVector GetTraceStartLocation(const APawn* Pawn) const;

	/** 가시성 트레이스를 위한 모든 잠재적 목표 위치들을 수집합니다 (예: 바운딩 박스 모서리들) */
	void GetTraceTargetLocations(const APawn* SourcePawn, const APawn* TargetPawn, TArray<FVector>& OutTargets) const;

	/** 시작점과 여러 목표 지점들 사이의 실제 트레이스를 수행합니다 */
	bool IsTargetVisible(const UWorld* World, const FVector& Start, const TArray<FVector>& Targets, ECollisionChannel TraceChannel, const FCollisionQueryParams& TraceParams) const;
};