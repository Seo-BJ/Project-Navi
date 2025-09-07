// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CompetitiveMatchScoring.h"

#include "LyraGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/Phases/LyraGamePhaseAbility.h" 
#include "GameFramework/GameState.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Player/LyraPlayerState.h"
#include "Teams/LyraTeamSubsystem.h"


UCompetitiveMatchScoring::UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
}


void UCompetitiveMatchScoring::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCompetitiveMatchScoring, VictoryScore);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingBuyingTime);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingRoundTime);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingSpikeTime);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingPostRoundTime);
	DOREPLIFETIME(UCompetitiveMatchScoring, bIsInOvertime); 
}

void UCompetitiveMatchScoring::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}


void UCompetitiveMatchScoring::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
	if (!HasAuthority()) 	return;

	// @Todo: 모든 플레이어가 준비됐는지 확인
	
	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(BuyingPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate BuyingPhaseEndDelegate;
		// BuyingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPlayingPhaseStarted);
		
		PhaseSubsystem->StartPhase(BuyingPhaseAbilityClass, BuyingPhaseEndDelegate);

		RemainingBuyingTime = BuyingTime;
		GetWorld()->GetTimerManager().SetTimer(
			BuyingPhaseTimerHandle,
			this,
			&UCompetitiveMatchScoring::BuyingTimeCountDown,
			1.0f,
			true);
	}
}

void UCompetitiveMatchScoring::BuyingTimeCountDown()
{
	RemainingBuyingTime -= 1.0f;
	if (RemainingBuyingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(BuyingPhaseTimerHandle);
		EndBuyingPhase();
	}
}

void UCompetitiveMatchScoring::EndBuyingPhase()
{
	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(RoundPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate PlayingPhaseEndDelegate;
		// PlayingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
		PhaseSubsystem->StartPhase(RoundPhaseAbilityClass, PlayingPhaseEndDelegate);
		
		PlayingPhaseStartDelegate.Broadcast();

		RemainingRoundTime = RoundTime;
		GetWorld()->GetTimerManager().SetTimer(
			RoundTimerHandle,
			this,
			&UCompetitiveMatchScoring::RoundTimeCountDown,
			1.0f,
			true);
	}
}
void UCompetitiveMatchScoring::HandleSpikePlanted(APawn* SpikePlanter)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	Multicast_BroadcastSpikePlantedMessage(SpikePlanter);

	RemainingSpikeTime = SpikeDetonationTime;
	GetWorld()->GetTimerManager().SetTimer(
		SpikePhaseTimerHandle,
		this,
		&UCompetitiveMatchScoring::SpikeTimeCountDown,
		1.0f,
		true);

	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(SpikePlantedPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate SpikePhaseEndDelegate;
		// 스파이크 페이즈가 끝나면 PostGame 페이즈로 이동.
		// SpikePhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
		PhaseSubsystem->StartPhase(SpikePlantedPhaseAbilityClass, SpikePhaseEndDelegate);
	}
}
void UCompetitiveMatchScoring::Multicast_BroadcastSpikePlantedMessage_Implementation(APawn* SpikePlanter)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	FSpikePlantedMessage Message;
	Message.SpikePlantInstigator = SpikePlanter;

	MessageSubsystem.BroadcastMessage(NaviGameplayTags::Navi_Spike_Plant_Finish, Message);
}



/*
 *
 *
*void UCompetitiveMatchScoring::RoundTimeCountDown()
{
	RemainingTime -= 1.0f;

	if (RemainingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
		HandlePostRound();
	}
}

 **/
void UCompetitiveMatchScoring::RoundTimeCountDown()
{
	RemainingRoundTime -= 1.0f;

	if (RemainingRoundTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
		// 시간 초과 시 수비팀 승리
		const int32 DefenderTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Defender);
		if (DefenderTeamId != INDEX_NONE)
		{
			AwardRoundWin(DefenderTeamId, TEXT("Timeout"));
		}
		else
		{
			HandlePostRound(); // 예외 처리
		}
	}
}

void UCompetitiveMatchScoring::SpikeTimeCountDown()
{
	RemainingSpikeTime -= 1.0f;
	if (RemainingSpikeTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpikePhaseTimerHandle);
		// 스파이크 폭발 시 공격팀 승리
		const int32 AttackerTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Attacker);
		if (AttackerTeamId != INDEX_NONE)
		{
			AwardRoundWin(AttackerTeamId, TEXT("Spike Detonated"));
		}
		else
		{
			HandlePostRound(); // 예외 처리
		}
	}
}


/*
*void UCompetitiveMatchScoring::SpikeTimeCountDown()
{
	RemainingSpikeTime -= 1.0f;
	if (RemainingSpikeTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpikePhaseTimerHandle);
		HandlePostRound();
	}
}

 *
 * 
 */

void UCompetitiveMatchScoring::OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility)
{
}


void UCompetitiveMatchScoring::OnRep_RemainingBuyingTime()
{
	OnPhaseTimeChanged.Broadcast(ECompetitiveMatchRoundPhase::CRP_Buying, RemainingBuyingTime);
}

void UCompetitiveMatchScoring::OnRep_RemainingRoundTime()
{
	OnPhaseTimeChanged.Broadcast(ECompetitiveMatchRoundPhase::CRP_Round, RemainingRoundTime);
}

void UCompetitiveMatchScoring::OnRep_RemainingSpikeTime()
{
	OnPhaseTimeChanged.Broadcast(ECompetitiveMatchRoundPhase::CRP_Spike, RemainingSpikeTime);
}

void UCompetitiveMatchScoring::OnRep_RemainingPostRound()
{
	OnPhaseTimeChanged.Broadcast(ECompetitiveMatchRoundPhase::CRP_PostRound, RemainingPostRoundTime);
}

// Competitive Match Scoring ///////////////////////////////////////////////////////////////////////////////////////////

int32 UCompetitiveMatchScoring::GetTeamScore(int32 TeamId) const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ULyraTeamSubsystem* TeamSubsystem = World->GetSubsystem<ULyraTeamSubsystem>())
		{
			return TeamSubsystem->GetTeamTagStackCount(TeamId, NaviGameplayTags::Navi_CompetitiveMatch_TeamScore);
		}
	}
	return 0;
}

void UCompetitiveMatchScoring::AddScoreToTeam(int32 TeamId, int32 ScoreToAdd)
{
	if (!HasAuthority()) return;

	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!TeamSubsystem) return;
	
	TArray<int32> TeamIds = TeamSubsystem->GetTeamIDs();

	// @ Todo: 2팀 체크를 하드코딩 대신 다른 코드 사용?
	if (TeamIds.Num() != 2)
	{
		return;
	}

	TeamSubsystem->AddTeamTagStack(TeamId, NaviGameplayTags::Navi_CompetitiveMatch_TeamScore, ScoreToAdd);
	
	const int32 Team1Score = GetTeamScore(TeamIds[0]);
	const int32 Team2Score = GetTeamScore(TeamIds[1]);
	
	if (bIsInOvertime)
	{
		// 연장전일 때, OvertimeScoreDifference 만큼 스코어 차이가 나야 매치 종료
		if (FMath::Abs(Team1Score - Team2Score) >= OvertimeScoreDifference)
		{
			const int32 WinningTeamId = (Team1Score > Team2Score) ? TeamIds[0] : TeamIds[1];
			HandleVictory(WinningTeamId);
		}
	}
	else
	{
		if (Team1Score == (VictoryScore - 1) && Team2Score == (VictoryScore - 1))
		{
			// 동점인 경우 연장전 시작
			StartOvertime();
		}
		else if (Team1Score >= VictoryScore || Team2Score >= VictoryScore)
		{
			const int32 WinningTeamId = (Team1Score > Team2Score) ? TeamIds[0] : TeamIds[1];
			HandleVictory(WinningTeamId);
		}
	}
}

void UCompetitiveMatchScoring::StartOvertime()
{
	if (!HasAuthority())
	{
		return;
	}
    
	bIsInOvertime = true;
	OnRep_IsInOvertime(); 
}


void UCompetitiveMatchScoring::HandleVictory_Implementation(int32 WinningTeamId)
{
	HandlePostRound();
}

void UCompetitiveMatchScoring::OnRep_IsInOvertime()
{
	if (bIsInOvertime)
	{
		// OnOvertimeStarted.Broadcast();
	}
}

void UCompetitiveMatchScoring::OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message)
{
    Super::OnEliminationMessageReceived(Tag, Message);

    if (HasAuthority())
    {
        // 짧은 딜레이 후 체크 (동시 사망 등 엣지 케이스 처리)
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::CheckTeamElimination);
    }
}

void UCompetitiveMatchScoring::CheckTeamElimination()
{
    if (bIsRoundDecided || !GetWorld())
    {
        return;
    }

    ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!TeamSubsystem || !GameState)
    {
        return;
    }
    
    TArray<int32> TeamIds = TeamSubsystem->GetTeamIDs();
    if (TeamIds.Num() < 2) return;

    TMap<int32, int32> AlivePlayerCounts;
    for (int32 TeamId : TeamIds)
    {
        AlivePlayerCounts.Add(TeamId, 0);
    }
	
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
        {
        	if (ULyraAbilitySystemComponent* LyraASC = LyraPS->GetLyraAbilitySystemComponent())
        	{
        		// @ Death처리는 HealthComponent 확인
        		if (!LyraASC->HasMatchingGameplayTag(LyraGameplayTags::Status_Death_Dying))
        		{
        			const int32 TeamId = LyraPS->GetTeamId();
        			if (AlivePlayerCounts.Contains(TeamId))
        			{
        				AlivePlayerCounts[TeamId]++;
        			}
        		}
        	}
        }
    }

    int32 EliminatedTeamId = INDEX_NONE;
    int32 WinningTeamId = INDEX_NONE;
	
    for (const auto& Pair : AlivePlayerCounts)
    { 	// 패배 팀 결정
        if (Pair.Value == 0)
        {
            EliminatedTeamId = Pair.Key;
            break;
        }
    }
	
    if(EliminatedTeamId != INDEX_NONE)
    { 	// 승리팀 결정
        for (const int32 TeamId : TeamIds)
        {
            if (TeamId != EliminatedTeamId)
            {
                WinningTeamId = TeamId;
                break;
            }
        }
        
        if (WinningTeamId != INDEX_NONE)
        {
            AwardRoundWin(WinningTeamId, TEXT("Team Elimination"));
        }
    }
}


void UCompetitiveMatchScoring::OnSpikeDefusedMessageReceived(FGameplayTag Tag, const FSpikeDefusedMessage& Message)
{
    if (HasAuthority())
    {
        // 스파이크 해체 시 수비팀 승리
        const int32 DefenderTeamId = GetTeamIdWithRole(FGameplayTag::RequestGameplayTag(TEXT("Navi.Team.Role.Defender")));
        if (DefenderTeamId != INDEX_NONE)
        {
            AwardRoundWin(DefenderTeamId, TEXT("Spike Defused"));
        }
    }
}

void UCompetitiveMatchScoring::AwardRoundWin(int32 WinningTeamId, const FString& RoundWinReason)
{
    if (!HasAuthority() || bIsRoundDecided)
    {
        return;
    }
    bIsRoundDecided = true;
    
    // UE_LOG(LogTemp, Log, TEXT("Round decided! Winner: Team %d, Reason: %s"), WinningTeamId, *RoundWinReason);

    AddScoreToTeam(WinningTeamId);
    HandlePostRound();
}

int32 UCompetitiveMatchScoring::GetTeamIdWithRole(const FGameplayTag& RoleTag) const
{
    ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
    if (!TeamSubsystem)
    {
        return INDEX_NONE;
    }

    TArray<int32> TeamIds = TeamSubsystem->GetTeamIDs();
;
    for (const int32 TeamId : TeamIds)
    {
        if (TeamSubsystem->TeamHasTag(TeamId, RoleTag))
        {
            return TeamId;
        }
    }

    return INDEX_NONE;
}

// ~Post Match /////////////////////////////////////////////////////////////////////////////////////////////////////////
void UCompetitiveMatchScoring::HandlePostRound()
{
	bIsRoundDecided = false;
	GetWorld()->GetTimerManager().ClearTimer(BuyingPhaseTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(SpikePhaseTimerHandle);
	
	RemainingPostRoundTime = PostGameTime;
	GetWorld()->GetTimerManager().SetTimer(
		PostRoundPhaseTimerHandle,
		this,
		&UCompetitiveMatchScoring::PostRoundTimeCountDown,
		1.0f,
		true);
	
    
    // @TODO: 라운드 종료 후 다음 라운드 준비(Buying Phase)로 넘어가는 GamePhase 전환 
}

void UCompetitiveMatchScoring::PostRoundTimeCountDown()
{
	RemainingPostRoundTime -= 1.0f;
	
	if (RemainingPostRoundTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PostRoundPhaseTimerHandle);
		ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
		if (ensure(PhaseSubsystem) && ensure(RoundPhaseAbilityClass))
		{
			FLyraGamePhaseDelegate PostRoundEndDelegate;
			// PlayingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
			PhaseSubsystem->StartPhase(PostRoundPhaseAbilityClass, PostRoundEndDelegate);
		}
	}
}

void UCompetitiveMatchScoring::StartRound_Implementation()
{
	
}
