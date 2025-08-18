// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameScoringBase.h"
#include "CompetitiveMatchScoring.generated.h"

class ULyraGamePhaseAbility;
class ULyraExperienceDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingTimeChangedDelegate, float, RemainTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingBuyingTimeChangedDelegate, float, NewBuyingTime);

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

	UPROPERTY(EditDefaultsOnly, Category = "Game Phase")	
	TSubclassOf<ULyraGamePhaseAbility> BuyingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game Phase")
	TSubclassOf<ULyraGamePhaseAbility> StartingPhaseAbilityClass;
	
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
	
	void HandlePostMatch();


	// Buying Phase
	void BuyingTimeCountDown();
	void EndBuyingPhase();
	
	FTimerHandle BuyingPhaseTimerHandle;
	
	// Playing Phase
	UFUNCTION()
	void OnPlayingPhaseStarted(const ULyraGamePhaseAbility* PhaseAbility);
	void CountDown();
	
	FTimerHandle RoundTimerHandle;
	
	// Post Game Phase
	UFUNCTION()
	void OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility);
	

};
