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

USTRUCT(BlueprintType)
struct FSpikeDefusedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> SpikeDefuseInstigator = nullptr; 
};

UENUM(BlueprintType)
enum class ECompetitiveMatchRoundPhase : uint8
{
	CRP_Buying UMETA(DisplayName = "Buying Phase"),
	CRP_Round UMETA(DisplayName = "Round Phase"),
	CRP_Spike UMETA(DisplayName = "Spike Planted Phase"),
	CRP_PostRound UMETA(DisplayName = "Post Round Phase")
};

class ULyraGamePhaseAbility;
class ULyraExperienceDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRemainingSpikeTimeChangedDelegate, ECompetitiveMatchRoundPhase, Phase, float, NewSpikeTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayingPhaseStartedDelegate);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamScoreAddedDelegate, int32, TeamId, int32, OldScore, int32, NewScore);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UCompetitiveMatchScoring : public UShooterGameScoringBase
{
	GENERATED_BODY()

public:
	UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRemainingSpikeTimeChangedDelegate OnPhaseTimeChanged;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void HandleSpikePlanted(APawn* SpikePlanter);

	UPROPERTY(BlueprintAssignable)
	FOnPlayingPhaseStartedDelegate PlayingPhaseStartDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnTeamScoreAddedDelegate OnTeamScoreAddedDelegate;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Score")
	int32 VictoryScore = 13;
	
	UPROPERTY(EditDefaultsOnly, Category = "GamePhase|Time")
	float BuyingTime = 30.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingBuyingTime)
	float RemainingBuyingTime;
	
	UPROPERTY(EditDefaultsOnly, Category = "GamePhase|Time")
	float RoundTime = 45.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingRoundTime)
	float RemainingRoundTime;
	
	UPROPERTY(EditDefaultsOnly, Category = "GamePhase|Time")
	float SpikeDetonationTime = 45.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingSpikeTime)
	float RemainingSpikeTime;

	UPROPERTY(EditDefaultsOnly, Category = "GamePhase|Time")
	float PostGameTime = 7.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingPostRound)
	float RemainingPostRoundTime;

	UPROPERTY(EditDefaultsOnly, Category = "GamePhase")	
	TSubclassOf<ULyraGamePhaseAbility> BuyingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> RoundPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> SpikePlantedPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> PostRoundPhaseAbilityClass;

	UFUNCTION(BlueprintNativeEvent)
	void StartRound();
	


private:
	
	UFUNCTION()
	void OnRep_RemainingBuyingTime();

	UFUNCTION()
	void OnRep_RemainingRoundTime();
	
	UFUNCTION()
	void OnRep_RemainingSpikeTime();
	
	UFUNCTION()
	void OnRep_RemainingPostRound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastSpikePlantedMessage(APawn* SpikePlanter);
	
	void HandlePostRound();


	// Buying Phase
	void BuyingTimeCountDown();
	void EndBuyingPhase();
	
	FTimerHandle BuyingPhaseTimerHandle;
	
	// Playing Phase
	void RoundTimeCountDown();
	FTimerHandle RoundTimerHandle;
	
	// --- Spike Planted Phase ---

	void SpikeTimeCountDown();
	FTimerHandle SpikePhaseTimerHandle;
	
	// Post Game Phase
	void PostRoundTimeCountDown();
	FTimerHandle PostRoundPhaseTimerHandle;
	
	UFUNCTION()
	void OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility);


	//~Competitive Match Scoring
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	int32 GetTeamScore(int32 TeamId) const;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddScoreToTeam(int32 TeamId, int32 ScoreToAdd = 1);

protected:
	/** 승리 조건을 처리합니다. */
	UFUNCTION(BlueprintNativeEvent)
	void HandleVictory(int32 WinningTeamId);

	UPROPERTY(EditDefaultsOnly, Category = "Scoring")
	int32 OvertimeScoreDifference = 2; // 연장전 승리 점수 차이

	UPROPERTY(ReplicatedUsing = OnRep_IsInOvertime)
	bool bIsInOvertime = false;
	
	void StartOvertime();

private:
	// ... 기존 함수 선언 ...
	UFUNCTION()
	void OnRep_IsInOvertime();

	//~End of Competitive Match Scoring

	//~Round Win & Defeat Management

protected:
	bool bIsRoundDecided = false;

	void AwardRoundWin(int32 WinningTeamId, const FString& RoundWinReason);

	/** 역할 태그를 기반으로 팀 ID를 찾습니다. */
	int32 GetTeamIdWithRole(const FGameplayTag& RoleTag) const;

	/** 모든 플레이어의 생존 상태를 확인하여 라운드 승패를 결정합니다. */
	void CheckTeamElimination();

	/** 스파이크 해체 메시지를 처리합니다. */
	virtual void OnSpikeDefusedMessageReceived(FGameplayTag Tag, const FSpikeDefusedMessage& Message);
	

	// ShooterGameScoringBase의 함수를 오버라이드합니다.
	virtual void OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message) override;
	
};
