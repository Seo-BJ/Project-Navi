// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "GameSessionScreen.generated.h"

class UGameSessionsManager;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UGameSessionScreen : public ULyraActivatableWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameSessionsManager> GameSessionsManagerClass;

protected:
	
	virtual void NativeOnActivated() override;

	UFUNCTION(BlueprintCallable)
	void JoinGameSession();
	
private:
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UGameSessionsManager> GameSessionsManager;
};
