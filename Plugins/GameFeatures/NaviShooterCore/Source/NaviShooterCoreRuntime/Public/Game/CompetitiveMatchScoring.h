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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamScoreAddedDelegate, int32, TeamId, int32, OldScore, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLyraGamePhaseDynamicMulticastDelegate, const ULyraGamePhaseAbility*, Phase);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UCompetitiveMatchScoring : public UShooterGameScoringBase
{
	GENERATED_BODY()

public:
	UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
	void HandleSpikePlanted(APawn* SpikePlanter);
	
	UFUNCTION(BlueprintCallable)
	virtual void HandleSpikeDefused(FGameplayTag Tag, const FSpikeDefusedMessage& Message);

	UPROPERTY(BlueprintCallable)
	FLyraGamePhaseDynamicMulticastDelegate BuyingPhaseEndDynamicMulticastDelegate;

	UPROPERTY(BlueprintCallable)
	FLyraGamePhaseDynamicMulticastDelegate PlayingPhaseEndDynamicMulticastDelegate;

	UPROPERTY(BlueprintCallable)
	FLyraGamePhaseDynamicMulticastDelegate SpikePantedPhaseDynamicMulticastDelegate;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Score")
	int32 VictoryScore = 13;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")	
	TSubclassOf<ULyraGamePhaseAbility> BuyingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> PlayingPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> SpikePlantedPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	TSubclassOf<ULyraGamePhaseAbility> PostRoundPhaseAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	float BuyingPhaseTime = 45.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	float PlayingPhaseTime = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")
	float SpikePlantedPhaseTime = 45.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GamePhase")	
	float PostRoundPhaseTime = 7.0f;
	
	FLyraGamePhaseDelegate BuyingPhaseEndDelegate;
	FLyraGamePhaseDelegate PlayingPhaseEndDelegate;
	FLyraGamePhaseDelegate SpikePlantedPhaseEndDelegate;
	
	UFUNCTION(BlueprintNativeEvent)
	void StartRound();
	
private:

	void OnBuyingPhaseEnded(const ULyraGamePhaseAbility* Phase);
	void OnPlayingPhaseEnded(const ULyraGamePhaseAbility* Phase);
	void OnSpikePhaseEnded(const ULyraGamePhaseAbility* Phase);
	void OnPostRoundPhaseEnded(const ULyraGamePhaseAbility* Phase);
	
	bool bSpikePlanted = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastSpikePlantedMessage(APawn* SpikePlanter);
	
	void StartNextRound();


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


	// ShooterGameScoringBase의 함수를 오버라이드합니다.
	virtual void OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message) override;
	
};
