// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "Types/DSTypes.h"
#include "LyraGameplayTags.h"

#include "NaviAgentSelectionGameMode.generated.h"

class AController;

USTRUCT(BlueprintType)
struct FAgentSelection
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	int32 TeamID = -1;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller= nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag AgentTag;
	
	UPROPERTY(BlueprintReadWrite)
	bool bConfirmSelection = false;
};
/**
 * 
 */
UCLASS()
class LYRAGAME_API ANaviAgentSelectionGameMode : public ALyraGameMode
{
	GENERATED_BODY()
	
public:
	
	ANaviAgentSelectionGameMode();
	// void CheckAndStartLobbyCountdown();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
protected:
	
	virtual void BeginPlay() override;

	// virtual void InitSeamlessTravelPlayer(AController* NewController) override;

	virtual void Logout(AController* Exiting) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	// virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = L"") override;
	
	
	UPROPERTY()
	ELobbyStatus LobbyStatus;

	UPROPERTY(EditDefaultsOnly)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DestinationMap;

	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;
	
private:
	
};
