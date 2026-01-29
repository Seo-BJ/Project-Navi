// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ITakesLyraPlayerState.h"
#include "NaviScoreBoard_Name.generated.h"

class UCommonTextBlock;
class ALyraPlayerState;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard_Name : public UUserWidget,  public ITakesLyraPlayerState
{
	GENERATED_BODY()
	
public:
	//~ ITakesLyraPlayerState 
	virtual void SetPlayerState_Implementation(ALyraPlayerState* InPlayerState) override;
	//~ End of ITakesLyraPlayerState

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> PlayerName_TextBlock;
	
};
