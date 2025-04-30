// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviLobbyGameMode.h"
#include "NaviLobbyGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyPlayerInfo.h"

#include "Player/DSPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Player/LyraPlayerController.h"

#include "Lobby/LobbyState.h"

#include "LyraLogChannels.h"

#include "Teams/LyraTeamCreationComponent.h"

ANaviLobbyGameMode::ANaviLobbyGameMode()
{
	bUseSeamlessTravel = true;
	LobbyStatus = ELobbyStatus::WaitingForPlayers;
	MinPlayers = 1;
	GameStateClass = ANaviLobbyGameState::StaticClass();
	//LobbyCountdownTimer.Type = ECountdownTimerType::LobbyCountdown;
}
void ANaviLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// @Todo: 현재 LyraGameMode에서 InitGameLift()를 호출중임.
	// InitGameLift();
}

void ANaviLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!GetWorld()->IsPlayInEditor()) // <--- PIE 확인 추가
	{
		const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
		const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

		TryAcceptPlayerSession(PlayerSessionId, Username, ErrorMessage);
		UE_LOG(LogLyraDedicatedServer, Warning, TEXT("ANaviLobbyGameMode::PreLogin- PlayerSessionId: %s, Username: %s"), *PlayerSessionId, *Username);
	}
	else
	{
		ErrorMessage = TEXT("");
		UE_LOG(LogLyraDedicatedServer, Warning, TEXT("ANaviLobbyGameMode::PreLogin (PIE) - Skipping GameLift validation."));
	}
}
void ANaviLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// CheckAndStartLobbyCountdown();
}


void ANaviLobbyGameMode::Logout(AController* Exiting)
{
	if (Exiting) // 유효한 컨트롤러인지 확인
	{
		// GameState를 가져옵니다. 프로젝트에 맞는 GameState 클래스로 캐스팅하세요.
		// 예: ALyraGameState* GS = GetGameState<ALyraGameState>();
		ANaviLobbyGameState* GS = GetGameState<ANaviLobbyGameState>();

		if (GS)
		{
			// GameState에서 ULyraTeamCreationComponent 찾기
			ULyraTeamCreationComponent* TeamCreationComp = GS->FindComponentByClass<ULyraTeamCreationComponent>();
			if (TeamCreationComp)
			{
				// 컴포넌트의 로그아웃 처리 함수 호출
				TeamCreationComp->ServerHandlePlayerLogout(Exiting);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ANaviLobbyGameMode::Logout: Could not find ULyraTeamCreationComponent on GameState."));
			}
		}
	}


	Super::Logout(Exiting);
	// CheckAndStopLobbyCountdown();
	RemovePlayerSession(Exiting);
	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		RemovePlayerInfoFromLobbyState(Exiting);
	}
}


FString ANaviLobbyGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	ALyraPlayerController* LyraPlayerController = Cast<ALyraPlayerController>(NewPlayerController);
	if (IsValid(LyraPlayerController))
	{
		ALyraPlayerState* LyraPlayerState = LyraPlayerController->GetPlayerState<ALyraPlayerState>();
		if (IsValid(LyraPlayerState))
		{
			const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
			FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

			if (Username.IsEmpty() && GetWorld()->IsPlayInEditor())
			{
				int32 PlayerId = LyraPlayerState->GetPlayerId(); 
				Username = FString::Printf(TEXT("PIE_User_%d"), PlayerId);
				UE_LOG(LogTemp, Log, TEXT("InitNewPlayer: Empty Username in PIE, generated temporary name: %s"), *Username);
			}
			
			LyraPlayerState->SetPlayerSessionId(PlayerSessionId);
			LyraPlayerState->SetPlayerUserName(Username);
		}
	}
	
	AddPlayerInfoToLobbyState(NewPlayerController);
	return InitializedString;
}
/*
void ANaviLobbyGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	CheckAndStartLobbyCountdown();
	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		AddPlayerInfoToLobbyState(NewController);
	}
}
*/
void ANaviLobbyGameMode::AddPlayerInfoToLobbyState(AController* Player) const
{
	ALyraPlayerController* LyraPlayerController = Cast<ALyraPlayerController>(Player);
	check(LyraPlayerController);
	ALyraPlayerState* LyraPlayerState = LyraPlayerController->GetPlayerState<ALyraPlayerState>();
	check(LyraPlayerState);
	
	ANaviLobbyGameState* NaviLobbyGameState = GetGameState<ANaviLobbyGameState>();
	if (IsValid(NaviLobbyGameState) && IsValid(NaviLobbyGameState->LobbyState) && IsValid(LyraPlayerState))
	{
		FLobbyPlayerInfo PlayerInfo(LyraPlayerState->GetPlayerUserName());
		NaviLobbyGameState->LobbyState->AddPlayerInfo(PlayerInfo);
	}
}
void ANaviLobbyGameMode::RemovePlayerInfoFromLobbyState(AController* Player) const
{
	ALyraPlayerController* LyraPlayerController = Cast<ALyraPlayerController>(Player);
	check(LyraPlayerController);
	ALyraPlayerState* LyraPlayerState = LyraPlayerController->GetPlayerState<ALyraPlayerState>();
	check(LyraPlayerState);
	
	ANaviLobbyGameState* NaviLobbyGameState = GetGameState<ANaviLobbyGameState>();
	if (IsValid(NaviLobbyGameState) && IsValid(NaviLobbyGameState->LobbyState) && IsValid(LyraPlayerState))
	{
		NaviLobbyGameState->LobbyState->RemovePlayerInfo(LyraPlayerState->GetPlayerUserName());
	}
}

void ANaviLobbyGameMode::TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage)
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
