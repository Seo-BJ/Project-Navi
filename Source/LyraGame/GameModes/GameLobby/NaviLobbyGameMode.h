// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "Types/DSTypes.h"
#include "NaviLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LYRAGAME_API ANaviLobbyGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:
	
	ANaviLobbyGameMode();
	// void CheckAndStartLobbyCountdown();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
protected:
	
	virtual void BeginPlay() override;

	// virtual void InitSeamlessTravelPlayer(AController* NewController) override;

	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = L"") override;
	
	void AddPlayerInfoToLobbyState(AController* Player) const;
	void RemovePlayerInfoFromLobbyState(AController* Player) const;
	
	UPROPERTY()
	ELobbyStatus LobbyStatus;

	UPROPERTY(EditDefaultsOnly)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DestinationMap;

	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;
	
private:
	
	void TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage);
};
