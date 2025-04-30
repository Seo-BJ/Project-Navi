// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviLobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Lobby/LobbyState.h"

ANaviLobbyGameState::ANaviLobbyGameState()
{
	bReplicates = true;
}

void ANaviLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANaviLobbyGameState, LobbyState);
}

void ANaviLobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CreateLobbyState();
		OnLobbyStateInitialized.Broadcast(LobbyState);
	}
}

void ANaviLobbyGameState::CreateLobbyState()
{
	if (UWorld* World = GetWorld(); IsValid(World))
	{
		FActorSpawnParameters SpawnParams;
		LobbyState = World->SpawnActor<ALobbyState>(
			ALobbyState::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (IsValid(LobbyState))
		{
			LobbyState->SetOwner(this);
		}
	}
}

void ANaviLobbyGameState::OnRep_LobbyState()
{
	OnLobbyStateInitialized.Broadcast(LobbyState);
}