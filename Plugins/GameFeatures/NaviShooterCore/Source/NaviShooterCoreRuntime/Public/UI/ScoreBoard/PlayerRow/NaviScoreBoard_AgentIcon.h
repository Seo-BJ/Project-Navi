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
	virtual void SetPlayerState_Implementation(ALyraPlayerState* InPlayerState) override;
	//~ End of ITakesLyraPlayerState
	
protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> AgentIcon_Image;

};
