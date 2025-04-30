// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "NaviGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NAVIGAME_API ANaviGameMode : public ALyraGameMode
{
	GENERATED_BODY()
	
public:

	ANaviGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
