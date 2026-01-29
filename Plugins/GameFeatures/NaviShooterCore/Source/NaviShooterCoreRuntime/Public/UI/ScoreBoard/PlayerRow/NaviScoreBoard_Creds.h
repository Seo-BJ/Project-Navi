// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ITakesLyraPlayerState.h"
#include "NaviScoreBoard_Creds.generated.h"

class UCommonNumericTextBlock;
class ALyraPlayerState;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard_Creds : public UUserWidget, public ITakesLyraPlayerState
{
	GENERATED_BODY()
	
public:
	//~ ITakesLyraPlayerState 
	virtual void SetPlayerState_Implementation(ALyraPlayerState* InPlayerState) override;
	//~ End of ITakesLyraPlayerState

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Creds_TextBlock;
};
