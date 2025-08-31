// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerStart.h"
#include "NaviPlayerStart.generated.h"

UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviPlayerStart : public ALyraPlayerStart
{
	GENERATED_BODY()

public:
	ANaviPlayerStart(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditInstanceOnly, Category = "Team", meta = (ClampMin = 0))
	int32 TeamId = INDEX_NONE;
};
