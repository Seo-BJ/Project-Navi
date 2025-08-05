// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "ShooterGameScoringBase.generated.h"

struct FLyraVerbMessage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UShooterGameScoringBase : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UShooterGameScoringBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	FGameplayMessageListenerHandle EliminationListenerHandle;

	virtual void OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message);
	virtual void OnAssistMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message);

	void ResetAllActivePlayers();
	void PostWarmUp();
};
