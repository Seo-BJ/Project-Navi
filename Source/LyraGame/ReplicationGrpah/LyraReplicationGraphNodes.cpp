// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraReplicationGraphNodes.h"
#include "LyraReplicationGraph.h"
#include "LyraConnectionManager.h"
#include "Character/LyraCharacter.h"
#include "Player/LyraPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Teams/LyraTeamSubsystem.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraReplicationGraphNodes)

// ---------------------------------------------------------------------------------------------------------------------
// ULyraReplicationGraphNode_AlwaysRelevant_ForConnection
// ---------------------------------------------------------------------------------------------------------------------

void ULyraReplicationGraphNode_AlwaysRelevant_ForConnection::ResetGameWorldState()
{
	ReplicationActorList.Reset();
	AlwaysRelevantStreamingLevelsNeedingReplication.Empty();
}

void ULyraReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraRepGraph_AlwaysRelevantNode_Gather);
	ULyraReplicationGraph* LyraGraph = CastChecked<ULyraReplicationGraph>(GetOuter());

	ReplicationActorList.Reset();

	for (const FNetViewer& CurViewer : Params.Viewers)
	{
		ReplicationActorList.ConditionalAdd(CurViewer.InViewer);
		ReplicationActorList.ConditionalAdd(CurViewer.ViewTarget);

		if (ALyraPlayerController* PC = Cast<ALyraPlayerController>(CurViewer.InViewer))
		{
			// PlayerState의 50% 스로틀링.
			const bool bReplicatePS = (Params.ConnectionManager.ConnectionOrderNum % 2) == (Params.ReplicationFrameNum % 2);
			if (bReplicatePS)
			{
				if (APlayerState* PS = PC->PlayerState)
				{
					if (!bInitializedPlayerState)
					{
						bInitializedPlayerState = true;
						FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(PS);
						ConnectionActorInfo.ReplicationPeriodFrame = 1;
					}

					ReplicationActorList.ConditionalAdd(PS);
				}
			}

			FCachedAlwaysRelevantActorInfo& LastData = PastRelevantActorMap.FindOrAdd(CurViewer.Connection);

			if (ALyraCharacter* Pawn = Cast<ALyraCharacter>(PC->GetPawn()))
			{
				UpdateCachedRelevantActor(Params, Pawn, LastData.LastViewer);

				if (Pawn != CurViewer.ViewTarget)
				{
					ReplicationActorList.ConditionalAdd(Pawn);
				}
			}

			if (ALyraCharacter* ViewTargetPawn = Cast<ALyraCharacter>(CurViewer.ViewTarget))
			{
				UpdateCachedRelevantActor(Params, ViewTargetPawn, LastData.LastViewTarget);
			}
		}
	}

	CleanupCachedRelevantActors(PastRelevantActorMap);

	// 항상 연관된 스트리밍 레벨 액터들.
	FPerConnectionActorInfoMap& ConnectionActorInfoMap = Params.ConnectionManager.ActorInfoMap;
	TMap<FName, FActorRepListRefView>& AlwaysRelevantStreamingLevelActors = LyraGraph->AlwaysRelevantStreamingLevelActors;

	for (int32 Idx=AlwaysRelevantStreamingLevelsNeedingReplication.Num()-1; Idx >= 0; --Idx)
	{
		const FName& StreamingLevel = AlwaysRelevantStreamingLevelsNeedingReplication[Idx];
		FActorRepListRefView* Ptr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel);
		
		if (Ptr == nullptr)
		{
			AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
			continue;
		}

		FActorRepListRefView& RepList = *Ptr;
		if (RepList.Num() > 0)
		{
			bool bAllDormant = true;
			for (FActorRepListType Actor : RepList)
			{
				FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);
				if (ConnectionActorInfo.bDormantOnConnection == false)
				{
					bAllDormant = false;
					break;
				}
			}

			if (bAllDormant)
			{
				AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
			}
			else
			{
				Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
			}
		}
	}

#if WITH_GAMEPLAY_DEBUGGER
	if (GameplayDebugger)
	{
		ReplicationActorList.ConditionalAdd(GameplayDebugger);
	}
#endif

	Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorList);
}

void ULyraReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld)
{
	AlwaysRelevantStreamingLevelsNeedingReplication.Add(LevelName);
}

void ULyraReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove(FName LevelName)
{
	AlwaysRelevantStreamingLevelsNeedingReplication.Remove(LevelName);
}

void ULyraReplicationGraphNode_AlwaysRelevant_ForConnection::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
{
	DebugInfo.Log(NodeName);
	DebugInfo.PushIndent();
	LogActorRepList(DebugInfo, NodeName, ReplicationActorList);

	for (const FName& LevelName : AlwaysRelevantStreamingLevelsNeedingReplication)
	{
		ULyraReplicationGraph* LyraGraph = CastChecked<ULyraReplicationGraph>(GetOuter());
		if (FActorRepListRefView* RepList = LyraGraph->AlwaysRelevantStreamingLevelActors.Find(LevelName))
		{
			LogActorRepList(DebugInfo, FString::Printf(TEXT("AlwaysRelevant StreamingLevel List: %s"), *LevelName.ToString()), *RepList);
		}
	}

	DebugInfo.PopIndent();
}

// ---------------------------------------------------------------------------------------------------------------------
// ULyraReplicationGraphNode_PlayerStateFrequencyLimiter
// ---------------------------------------------------------------------------------------------------------------------

ULyraReplicationGraphNode_PlayerStateFrequencyLimiter::ULyraReplicationGraphNode_PlayerStateFrequencyLimiter()
{
	bRequiresPrepareForReplicationCall = true;
}

void ULyraReplicationGraphNode_PlayerStateFrequencyLimiter::PrepareForReplication()
{
	SCOPE_CYCLE_COUNTER(STAT_LyraRepGraph_PreparePlayerStates);
	ReplicationActorLists.Reset();
	ForceNetUpdateReplicationActorList.Reset();

	ReplicationActorLists.AddDefaulted();
	FActorRepListRefView* CurrentList = &ReplicationActorLists[0];

	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
	{
		APlayerState* PS = *It;
		if (IsActorValidForReplicationGather(PS) == false)
		{
			continue;
		}

		if (CurrentList->Num() >= TargetActorsPerFrame)
		{
			ReplicationActorLists.AddDefaulted();
			CurrentList = &ReplicationActorLists.Last(); 
		}
		
		CurrentList->Add(PS);
	}
}

void ULyraReplicationGraphNode_PlayerStateFrequencyLimiter::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	if (ReplicationActorLists.Num() > 0)
	{
		const int32 ListIdx = Params.ReplicationFrameNum % ReplicationActorLists.Num();
		Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorLists[ListIdx]);
	}

	if (ForceNetUpdateReplicationActorList.Num() > 0)
	{
		Params.OutGatheredReplicationLists.AddReplicationActorList(ForceNetUpdateReplicationActorList);
	}	
}

void ULyraReplicationGraphNode_PlayerStateFrequencyLimiter::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
{
	DebugInfo.Log(NodeName);
	DebugInfo.PushIndent();	

	int32 i=0;
	for (const FActorRepListRefView& List : ReplicationActorLists)
	{
		LogActorRepList(DebugInfo, FString::Printf(TEXT("Bucket[%d]"), i++), List);
	}

	DebugInfo.PopIndent();
}

// ---------------------------------------------------------------------------------------------------------------------
// UReplicationGraphNode_AlwaysRelevant_WithPending
// ---------------------------------------------------------------------------------------------------------------------

UReplicationGraphNode_AlwaysRelevant_WithPending::UReplicationGraphNode_AlwaysRelevant_WithPending()
{
	bRequiresPrepareForReplicationCall = true;
}

void UReplicationGraphNode_AlwaysRelevant_WithPending::PrepareForReplication()
{
	ULyraReplicationGraph* ReplicationGraph = Cast<ULyraReplicationGraph>(GetOuter());
	if (ReplicationGraph)
	{
		ReplicationGraph->HandlePendingActorsAndTeamRequests();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// ULyraReplicationGraphNode_AlwaysRelevant_ForTeam
// ---------------------------------------------------------------------------------------------------------------------

void ULyraReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	ULyraReplicationGraph* ReplicationGraph = CastChecked<ULyraReplicationGraph>(GetOuter());
	const ULyraConnectionManager* ConnectionManager = Cast<ULyraConnectionManager>(&Params.ConnectionManager);

	if (ReplicationGraph && ConnectionManager && ConnectionManager->Team != -1)
	{
		if (TArray<ULyraConnectionManager*>* TeamConnections = ReplicationGraph->TeamConnectionListMap.GetConnectionArrayForTeam(ConnectionManager->Team))
		{
			for (const ULyraConnectionManager* TeamMember : *TeamConnections)
			{
				if (TeamMember->TeamConnectionNode)
				{
					TeamMember->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
				}
			}
		}

		// 가시성이 확보된 적군들을 리스트에 추가
		const TArray<ULyraConnectionManager*>& NonTeamConnections = ReplicationGraph->TeamConnectionListMap.GetVisibleConnectionArrayForNonTeam(ConnectionManager->Pawn.Get(), ConnectionManager->Team);

		for (const ULyraConnectionManager* NonTeamMember : NonTeamConnections)
		{
			if (NonTeamMember->TeamConnectionNode)
			{
				NonTeamMember->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
			}
		}
	}
	else
	{
		Super::GatherActorListsForConnection(Params);
	}
}

void ULyraReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnectionDefault(const FConnectionGatherActorListParameters& Params)
{
	Super::GatherActorListsForConnection(Params);
}
