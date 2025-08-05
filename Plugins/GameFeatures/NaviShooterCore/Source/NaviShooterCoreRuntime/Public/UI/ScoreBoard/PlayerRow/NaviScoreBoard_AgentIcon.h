// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/ITakesLyraPlayerState.h"

#include "NaviScoreBoard_AgentIcon.generated.h"

class UImage;
class ALyraPlayerState;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard_AgentIcon : public UUserWidget, public ITakesLyraPlayerState
{
	GENERATED_BODY()
	
public:
	//~ ITakesLyraPlayerState 
	virtual void SetPlayerState(ALyraPlayerState* InPlayerState) override;
	//~ End of ITakesLyraPlayerState
	
protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> AgentIcon_Image;

	/*
	void OnScoreboardMessageReceived(FGameplayTag Channel, const FNaviScoreboardMessage& Message);

private:
	// Gameplay Message 리스너 핸들입니다. 리스너를 등록/해제할 때 사용합니다.
	FGameplayMessageListenerHandle MessageListenerHandle;

	// 수신할 메시지의 채널을 나타내는 태그입니다.
	// 에디터에서 설정할 수 있도록 UPROPERTY로 노출하는 것이 좋습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Message")
	FGameplayTag ScoreboardMessageChannel;*/
};
