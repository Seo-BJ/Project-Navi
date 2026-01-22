// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameScoringBase.h"
#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h"
#include "CompetitiveMatchScoring.generated.h"

class ALyraPlayerState;

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

UENUM(BlueprintType)
enum class ERoundWinReason : uint8
{
	AttackerTeamEliminated UMETA(DisplayName = "AttackerTeamEliminated"),
	DefenderTeamEliminated UMETA(DisplayName = "DefenderTeamEliminated"),
	Timeout UMETA(DisplayName = "Timeout"),
	SpikeDetonated UMETA(DisplayName = "Spike Detonated"),
	SpikeDefused UMETA(DisplayName = "Spike Defused")
};


USTRUCT(BlueprintType)
struct FRoundWinResultMessage
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	ERoundWinReason WinReason;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag WinTeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(BlueprintReadWrite)
	int32 WinTeamID = -1;
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
	
	//UFUNCTION(BlueprintCallable)
	//void HandleSpikePlanted(APawn* SpikePlanter);
	
	UFUNCTION(BlueprintCallable)
	void HandleRoundResult(ERoundWinReason RoundWinReason);
	

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartNewRound();
	
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

private:

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Lyra|GameState")
	void MulticastRoundWinResultMessageToClients(const FRoundWinResultMessage Message);

	
	UFUNCTION(BlueprintCallable)
	void HandlePostRoundPhaseEnd(const ULyraGamePhaseAbility* Phase);
	
	bool bSpikePlanted = false;

	void HandleTeamEliminated(FGameplayTag EliminatedTeamTag);
	void HandleSpikeDefused();
	void HandleRoundTimeout();
	void HandleSpikeDetonated();

	TMap<int32, TArray<TWeakObjectPtr<ALyraPlayerState>>> TeamDeathMap;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	int32 GetTeamScore(int32 TeamId) const;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void AddScoreToTeam(int32 TeamId, int32 ScoreToAdd = 1);

protected:
	
	//~Round Reset Management
	
	void ResetRound();
	void CleanupMapActors();
	void RespawnOrTeleportPlayers();

private:


protected:
	bool bIsRoundDecided = false;

	UFUNCTION(BlueprintCallable)
	void AwardRoundWin(int32 WinningTeamId, ERoundWinReason WinReason);

	/** 역할 태그를 기반으로 팀 ID를 찾습니다. */
	int32 GetTeamIdWithRole(const FGameplayTag& RoleTag) const;

	/** 모든 플레이어의 생존 상태를 확인하여 라운드 승패를 결정합니다. */
	void CheckTeamElimination(const FLyraVerbMessage& Message);


	// ShooterGameScoringBase의 함수를 오버라이드합니다.
	virtual void OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message) override;
	
};
