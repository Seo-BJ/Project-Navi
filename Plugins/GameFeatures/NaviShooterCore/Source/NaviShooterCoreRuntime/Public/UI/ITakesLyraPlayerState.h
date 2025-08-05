// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "UObject/Interface.h"
#include "ITakesLyraPlayerState.generated.h"

class ALyraPlayerState;

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UTakesLyraPlayerState : public UInterface
{
	GENERATED_BODY()
};

class ITakesLyraPlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual void SetPlayerState(ALyraPlayerState* InPlayerState) = 0;
};


