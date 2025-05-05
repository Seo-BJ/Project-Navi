// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentSelectionGameMode.h"
#include "LyraLogChannels.h"
#include "Game/DSGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyPlayerInfo.h"
#include "Lobby/LobbyState.h"
#include "Player/DSPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Player/LyraPlayerController.h"

ANaviAgentSelectionGameMode::ANaviAgentSelectionGameMode()
{
	bUseSeamlessTravel = true;
	LobbyStatus = ELobbyStatus::WaitingForPlayers;
	MinPlayers = 1;
	//LobbyCountdownTimer.Type = ECountdownTimerType::LobbyCountdown;
}
void ANaviAgentSelectionGameMode::BeginPlay()
{
	Super::BeginPlay();

	// @Todo: 현재 LyraGameMode에서 InitGameLift()를 호출중임.
	// InitGameLift();
}

void ANaviAgentSelectionGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!GetWorld()->IsPlayInEditor()) // <--- PIE 확인 추가
	{
		const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
		const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

		// TryAcceptPlayerSession(PlayerSessionId, Username, ErrorMessage);
		UE_LOG(LogLyraDedicatedServer, Warning, TEXT("ANaviAgentSelectionGameMode::PreLogin- PlayerSessionId: %s, Username: %s"), *PlayerSessionId, *Username);
	}
	else
	{
		ErrorMessage = TEXT("");
		UE_LOG(LogLyraDedicatedServer, Warning, TEXT("ANaviAgentSelectionGameMode::PreLogin (PIE) - Skipping GameLift validation."));
	}
}
void ANaviAgentSelectionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// CheckAndStartLobbyCountdown();
}


void ANaviAgentSelectionGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	// CheckAndStopLobbyCountdown();
	RemovePlayerSession(Exiting);
	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		// RemovePlayerInfoFromLobbyState(Exiting);
	}
}
