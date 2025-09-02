// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameScoringBase.h"
#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h"
#include "CompetitiveMatchScoring.generated.h"

USTRUCT(BlueprintType)
struct FSpikePlantedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> SpikePlantInstigator = nullptr; 
};


class ULyraGamePhaseAbility;
class ULyraExperienceDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingSpikeTimeChangedDelegate, float, NewSpikeTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChangedDelegate, float, RemainTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingBuyingTimeChangedDelegate, float, NewBuyingTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayingPhaseStartedDelegate);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UCompetitiveMatchScoring : public UShooterGameScoringBase
{
	GENERATED_BODY()

public:
	UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRemainingTimeChangedDelegate OnRemainingTimeChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRemainingBuyingTimeChangedDelegate OnRemainingBuyingTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRemainingSpikeTimeChangedDelegate OnRemainingSpikeTimeChanged;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void HandleSpikePlanted(APawn* SpikePlanter);

	UPROPERTY(BlueprintAssignable)
	FOnPlayingPhaseStartedDelegate PlayingPhaseStartDelegate;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_TargetScore)
	int32 TargetScore = 25;
	
	UPROPERTY(EditDefaultsOnly, Category = "Scoring")
	float RoundInitialTime = 100.0f;
	
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	float RemainingTime;

	UPROPERTY(EditDefaultsOnly, Category = "Scoring")
	float BuyingTime = 30.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingBuyingTime)
	float RemainingBuyingTime;

	UPROPERTY(EditDefaultsOnly, Category = "Scoring|Spike", meta = (AllowPrivateAccess = "true"))
	float SpikeDetonationTime = 45.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingSpikeTime)
	float RemainingSpikeTime;

	UPROPERTY(EditDefaultsOnly, Category = "Game Phase")	
	TSubclassOf<ULyraGamePhaseAbility> BuyingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game Phase")
	TSubclassOf<ULyraGamePhaseAbility> StartingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game Phase")
	TSubclassOf<ULyraGamePhaseAbility> SpikePlantedPhaseAbilityClass;
	
	UFUNCTION()
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);


public:
private:

	UFUNCTION()
	void OnRep_TargetScore();
    
	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_RemainingBuyingTime();

	UFUNCTION()
	void OnRep_RemainingSpikeTime();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastSpikePlantedMessage(APawn* SpikePlanter);
	
	void HandlePostMatch();


	// Buying Phase
	void BuyingTimeCountDown();
	void EndBuyingPhase();
	
	FTimerHandle BuyingPhaseTimerHandle;
	
	// Playing Phase
	void PlayingTimeCountDown();
	FTimerHandle RoundTimerHandle;
	
	// --- Spike Planted Phase ---

	void SpikeTimeCountDown();
	FTimerHandle SpikePhaseTimerHandle;
	

	
	// Post Game Phase
	UFUNCTION()
	void OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility);
	
};
