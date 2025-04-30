// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTotalUserBoard.generated.h"

class UNaviTeamUserBoard;
class UNaviUserInfoRow;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviTotalUserBoard : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UNaviTeamUserBoard> First_TeamUserBoard;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UNaviTeamUserBoard> Second_TeamUserBoard;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UNaviTeamUserBoard> Neutrality_TeamUserBoard;

	UFUNCTION(BlueprintCallable, Category = "Lobby|Teams")
	void AddUserInfoRow(int32 TeamID, const FString& UserName); 

	UFUNCTION(BlueprintCallable, Category = "Lobby|Teams")
	UNaviUserInfoRow* FindUserInfoRow(const FString& UserName, bool& OutbSuccess, int32& OutTeamID);
	
	UFUNCTION(BlueprintCallable, Category = "Lobby|Teams")
	void RemoveUserInfoRow(const FString& UserName);
	
	UFUNCTION(BlueprintCallable, Category = "Lobby|Teams")
	void MoveUserInfoRow(int32 NewTeamID, const FString& UserName);

	void ClearAllTeamUserBoard() const;

protected:
	
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnLobbyStateInitialized(ALobbyState* LobbyState);

	void AddExistingPlayers();
	
	UFUNCTION()
	void OnPlayerRemoved(const FLobbyPlayerInfo& PlayerInfo);

	
};

