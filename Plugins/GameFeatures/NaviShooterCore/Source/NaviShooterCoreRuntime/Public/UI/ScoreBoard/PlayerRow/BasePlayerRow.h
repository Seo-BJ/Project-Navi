// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ITakesLyraPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "BasePlayerRow.generated.h"

class ALyraPlayerState;
class UWidget;

/**
 * 
 */
UCLASS(Abstract)
class NAVISHOOTERCORERUNTIME_API UBasePlayerRow : public UUserWidget, public ITakesLyraPlayerState
{
	GENERATED_BODY()
	
public:
	//~ ITakesLyraPlayerState 
	virtual void SetPlayerState(ALyraPlayerState* InPlayerState) override;
	//~ ITakesLyraPlayerState 

protected:
	TWeakObjectPtr<ALyraPlayerState> AssociatedPlayerState;
	
	virtual TArray<UWidget*> GetPlayerStateAwareChildren() const;
};
