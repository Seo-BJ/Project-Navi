// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "UObject/Interface.h"
#include "ITakesLyraPlayerState.generated.h"

class ALyraPlayerState;

UINTERFACE(MinimalAPI, Blueprintable)
class UTakesLyraPlayerState : public UInterface
{
	GENERATED_BODY()
};

class LYRAGAME_API ITakesLyraPlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayerState(ALyraPlayerState* InPlayerState);
};


