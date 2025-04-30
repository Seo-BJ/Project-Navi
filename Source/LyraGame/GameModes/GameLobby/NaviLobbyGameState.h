// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameState.h"
#include "NaviLobbyGameState.generated.h"

class ALobbyState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNaviLobbyStateInitialized, ALobbyState*, LobbyState);
/**
 * 
 */
UCLASS()
class LYRAGAME_API ANaviLobbyGameState : public ALyraGameState
{
	GENERATED_BODY()

public:
	
	ANaviLobbyGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_LobbyState)
	TObjectPtr<ALobbyState> LobbyState;

	UPROPERTY(BlueprintAssignable)
	FOnNaviLobbyStateInitialized OnLobbyStateInitialized;

protected:
	
	virtual void BeginPlay() override;
	void CreateLobbyState();
	
private:

	UFUNCTION()
	void OnRep_LobbyState();
};
