// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyState.h"

#include "DedicatedServers/DedicatedServers.h"
#include "Net/UnrealNetwork.h"


ALobbyState::ALobbyState()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALobbyState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyState, PlayerInfoArray);
}

void ALobbyState::AddPlayerInfo(const FLobbyPlayerInfo& PlayerInfo)
{
	PlayerInfoArray.AddPlayer(PlayerInfo);
	UE_LOG(LogDedicatedServers, Warning, TEXT("PlayerInfoArray AddPlayer"));
}

void ALobbyState::RemovePlayerInfo(const FString& Username)
{
	PlayerInfoArray.RemovePlayer(Username);
	UE_LOG(LogDedicatedServers, Warning, TEXT("PlayerInfoArray RemovePlayer"));
}

TArray<FLobbyPlayerInfo> ALobbyState::GetPlayers() const
{
	return PlayerInfoArray.Players;
}

void ALobbyState::OnRep_LobbyPlayerInfo()
{
	FLobbyPlayerInfoDelta Delta = ComputePlayerInfoDelta(LastPlayerInfoArray.Players, PlayerInfoArray.Players);
	for (const auto& PlayerInfo : Delta.AddedPlayers)
	{
		OnPlayerInfoAdded.Broadcast(PlayerInfo);
		UE_LOG(LogDedicatedServers, Warning, TEXT("OnPlayerInfoAdded"));
	}
	for (const auto& PlayerInfo : Delta.RemovedPlayers)
	{
		OnPlayerInfoRemoved.Broadcast(PlayerInfo);
		UE_LOG(LogDedicatedServers, Warning, TEXT("OnPlayerInfoRemoved"));

		
	}

	LastPlayerInfoArray = PlayerInfoArray;
}

FLobbyPlayerInfoDelta ALobbyState::ComputePlayerInfoDelta(const TArray<FLobbyPlayerInfo>& OldArray, const TArray<FLobbyPlayerInfo>& NewArray)
{
	FLobbyPlayerInfoDelta Delta;

	TMap<FString, const FLobbyPlayerInfo*> OldMap;
	TMap<FString, const FLobbyPlayerInfo*> NewMap;

	for (const auto& PlayerInfo : OldArray)
	{
	}
	for (const auto& PlayerInfo : NewArray)
	{
		NewMap.Add(PlayerInfo.Username, &PlayerInfo);
	}
	for (const auto& OldPlayerInfo : OldArray)
	{
		if (!NewMap.Contains(OldPlayerInfo.Username))
		{
			Delta.RemovedPlayers.Add(OldPlayerInfo);
		}
	}
	for (const auto& NewPlayerInfo : NewArray)
	{
		if (!OldMap.Contains(NewPlayerInfo.Username))
		{
			Delta.AddedPlayers.Add(NewPlayerInfo);
		}
	}

	return Delta;
}