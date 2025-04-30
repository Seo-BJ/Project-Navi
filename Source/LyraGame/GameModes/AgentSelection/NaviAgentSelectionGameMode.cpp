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

		TryAcceptPlayerSession(PlayerSessionId, Username, ErrorMessage);
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
		RemovePlayerInfoFromLobbyState(Exiting);
	}
}


FString ANaviAgentSelectionGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	ALyraPlayerController* PlayerController = Cast<ALyraPlayerController>(NewPlayerController);
	if (IsValid(PlayerController))
	{
		ALyraPlayerState* PlayerState = PlayerController->GetPlayerState<ALyraPlayerState>();
		if (IsValid(PlayerState))
		{
			const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
			const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));
			
			PlayerState->SetPlayerSessionId(PlayerSessionId);
			PlayerState->SetPlayerUserName(Username);
		}
	}
	AddPlayerInfoToLobbyState(NewPlayerController);
	return InitializedString;
}
/*
void ANaviAgentSelectionGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	CheckAndStartLobbyCountdown();
	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		AddPlayerInfoToLobbyState(NewController);
	}
}
*/
void ANaviAgentSelectionGameMode::AddPlayerInfoToLobbyState(AController* Player) const
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADSGameState* DSGameState = GetGameState<ADSGameState>();
	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		FLobbyPlayerInfo PlayerInfo(DSPlayerController->Username);
		DSGameState->LobbyState->AddPlayerInfo(PlayerInfo);
	}
}
void ANaviAgentSelectionGameMode::RemovePlayerInfoFromLobbyState(AController* Player) const
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADSGameState* DSGameState = GetGameState<ADSGameState>();
	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		DSGameState->LobbyState->RemovePlayerInfo(DSPlayerController->Username);
	}
}

void ANaviAgentSelectionGameMode::TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage)
{
	if (PlayerSessionId.IsEmpty() || Username.IsEmpty())
	{
		OutErrorMessage = TEXT("PlayerSessionId and/or Username invalid.");
		return;
	}

#if WITH_GAMELIFT
	Aws::GameLift::Server::Model::DescribePlayerSessionsRequest DescribePlayerSessionsRequest;
	DescribePlayerSessionsRequest.SetPlayerSessionId(TCHAR_TO_ANSI(*PlayerSessionId));

	const auto& DescribePlayerSessionsOutcome = Aws::GameLift::Server::DescribePlayerSessions(DescribePlayerSessionsRequest);
	if (!DescribePlayerSessionsOutcome.IsSuccess())
	{
		OutErrorMessage = TEXT("DescribePlayerSessions failed.");
		return;
	}

	const auto& DescribePlayerSessionsResult = DescribePlayerSessionsOutcome.GetResult();
	int32 Count = 0;
	const Aws::GameLift::Server::Model::PlayerSession* PlayerSessions = DescribePlayerSessionsResult.GetPlayerSessions(Count);
	if (PlayerSessions == nullptr || Count == 0)
	{
		OutErrorMessage = TEXT("GetPlayerSessions failed.");
		return;
	}

	for (int32 i = 0; i < Count; i++)
	{
		const Aws::GameLift::Server::Model::PlayerSession& PlayerSession = PlayerSessions[i];
		if (!Username.Equals(PlayerSession.GetPlayerId())) continue;
		if (PlayerSession.GetStatus() != Aws::GameLift::Server::Model::PlayerSessionStatus::RESERVED)
		{
			OutErrorMessage = FString::Printf(TEXT("Session for %s not RESERVED; Fail PreLogin."), *Username);
			return;
		}

		const auto& AcceptPlayerSessionOutcome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));
		OutErrorMessage = AcceptPlayerSessionOutcome.IsSuccess() ? "" : FString::Printf(TEXT("Failed to accept player session for %s"), *Username);
	}
#endif
}


