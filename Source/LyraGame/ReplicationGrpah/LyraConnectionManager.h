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
	// UReplicationGraphNode_AlwaysRelevant_ForConnection is a node type provided by Epic for actors always relevant to a connection
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
};