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
#include "Kismet/GameplayStatics.h"
#include "Character/LyraCharacter.h"
#include "Character/LyraHealthComponent.h"
#include "Player/LyraPlayerStart.h"
#include "GameModes/LyraGameMode.h"
#include "GameModes/LyraGameState.h"
#include "Messages/LyraVerbMessage.h"


UCompetitiveMatchScoring::UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
}

void UCompetitiveMatchScoring::HandleRoundResult(ERoundWinReason RoundWinReason)
{
	if (!HasAuthority()) return;
	
	ALyraGameState* LyraGS = GetOwner<ALyraGameState>();
	if (IsValid(LyraGS))
	{
		FRoundWinResultMessage RoundResultMessage;
		RoundResultMessage.WinReason = RoundWinReason;
		
		switch (RoundWinReason) {
		case ERoundWinReason::AttackerTeamEliminated:
			RoundResultMessage.WinTeamTag = NaviGameplayTags::TAG_Navi_Team_Role_Defender;
			HandleTeamEliminated( NaviGameplayTags::TAG_Navi_Team_Role_Attacker);
			break;
		case ERoundWinReason::DefenderTeamEliminated:
			RoundResultMessage.WinTeamTag = NaviGameplayTags::TAG_Navi_Team_Role_Attacker;
			HandleTeamEliminated( NaviGameplayTags::TAG_Navi_Team_Role_Defender);
			break;
		case ERoundWinReason::Timeout:
			RoundResultMessage.WinTeamTag = NaviGameplayTags::TAG_Navi_Team_Role_Defender;
			HandleRoundTimeout();
			break;
		case ERoundWinReason::SpikeDetonated:
			RoundResultMessage.WinTeamTag = NaviGameplayTags::TAG_Navi_Team_Role_Attacker;
			HandleSpikeDetonated();
			break;
		case ERoundWinReason::SpikeDefused:
			RoundResultMessage.WinTeamTag = NaviGameplayTags::TAG_Navi_Team_Role_Defender;
			HandleSpikeDefused();
			break;
		}
		
		RoundResultMessage.WinTeamID = GetTeamIdWithRole(RoundResultMessage.WinTeamTag);
		MulticastRoundWinResultMessageToClients(RoundResultMessage);
	}
	
}


void UCompetitiveMatchScoring::MulticastRoundWinResultMessageToClients_Implementation(const FRoundWinResultMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(FGameplayTag::RequestGameplayTag("Navi.Round.Result"), Message);
	}
}


void UCompetitiveMatchScoring::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCompetitiveMatchScoring, VictoryScore);
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
	if (!HasAuthority()) return;
	// @Todo: 모든 플레이어가 준비됐는지 확인
}

void UCompetitiveMatchScoring::HandleTeamEliminated(FGameplayTag EliminatedTeamTag)
{
	int32 DefenderTeamId = -1;
	if (EliminatedTeamTag == NaviGameplayTags::TAG_Navi_Team_Role_Attacker)
	{
		DefenderTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Defender);
		AwardRoundWin(DefenderTeamId, ERoundWinReason::AttackerTeamEliminated);
	}
	else if (EliminatedTeamTag == NaviGameplayTags::TAG_Navi_Team_Role_Defender)
	{
		DefenderTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Attacker);
		AwardRoundWin(DefenderTeamId, ERoundWinReason::DefenderTeamEliminated);
	}
}


void UCompetitiveMatchScoring::HandleSpikeDetonated()
{
	// 이미 승패가 결정됨 (해체 등)
	if (bIsRoundDecided)
	{
		return;
	}

	// 스파이크 폭발 시 공격팀 승리
	const int32 AttackerTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Attacker);
	if (AttackerTeamId != INDEX_NONE)
	{
		AwardRoundWin(AttackerTeamId, ERoundWinReason::SpikeDetonated);
	}
	else
	{
		HandlePostRoundPhaseEnd(nullptr);
	}
}


void UCompetitiveMatchScoring::HandleSpikeDefused()
{
	if (!HasAuthority() || bIsRoundDecided)
	{
		return;
	}
	
	// 스파이크 해체 시 수비팀 승리
	const int32 DefenderTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Defender);
	if (DefenderTeamId != INDEX_NONE)
	{
		AwardRoundWin(DefenderTeamId, ERoundWinReason::SpikeDefused);
	}
}


void UCompetitiveMatchScoring::HandleRoundTimeout()
{
	// 이미 승패가 결정되었거나 스파이크가 설치된 경우 무시
	if (bSpikePlanted || bIsRoundDecided)
	{
		return;
	}

	// 시간 초과 시 수비팀 승리
	const int32 DefenderTeamId = GetTeamIdWithRole(NaviGameplayTags::TAG_Navi_Team_Role_Defender);
	if (DefenderTeamId != INDEX_NONE)
	{
		AwardRoundWin(DefenderTeamId, ERoundWinReason::Timeout);
	}
	else
	{
		// 예외 상황: 팀 ID를 찾을 수 없음
		HandlePostRoundPhaseEnd(nullptr); 
	}
}




void UCompetitiveMatchScoring::HandlePostRoundPhaseEnd(const ULyraGamePhaseAbility* Phase)
{
	// 라운드 상태 초기화 (다음 라운드 준비)
	ResetRound();
	
	bSpikePlanted = false;
	bIsRoundDecided = false;
}

void UCompetitiveMatchScoring::ResetRound()
{
	if (!HasAuthority())
	{
		return;
	}

	CleanupMapActors();
	RespawnOrTeleportPlayers();
}

void UCompetitiveMatchScoring::CleanupMapActors()
{
	if (!GetWorld()) return;

	// 태그 기반으로 정리할 액터들 찾기 (예: 드랍된 무기, 스파이크 등)
	// @TODO: 실제 드랍된 무기나 스킬 오브젝트에 해당 태그를 붙여야 함 "Navi.State.RoundCleanup"
	// 현재는 예시로 작성
	
	TArray<AActor*> ActorsToCleanup;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Navi.State.RoundCleanup"), ActorsToCleanup);
	
	for (AActor* Actor : ActorsToCleanup)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
}

void UCompetitiveMatchScoring::RespawnOrTeleportPlayers()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GameState = World->GetGameState();
	ALyraGameMode* GameMode = Cast<ALyraGameMode>(World->GetAuthGameMode());
	if (!GameState || !GameMode) return;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS);
		if (!LyraPS) continue;

		ULyraAbilitySystemComponent* LyraASC = LyraPS->GetLyraAbilitySystemComponent();
		if (!LyraASC) continue;

		AController* Controller = LyraPS->GetOwningController();
		if (!Controller) continue;

		APawn* Pawn = Controller->GetPawn();
		ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(Pawn);

		// 1. 사망자 처리 (RestartPlayer)
		if (LyraASC->HasMatchingGameplayTag(LyraGameplayTags::Status_Death_Dead))
		{
			GameMode->RestartPlayer(Controller);
		}
		else 		// 2. 생존자 처리 (Teleport & Heal)
		{
			// 팀에 맞는 PlayerStart 찾기
			AActor* StartSpot = GameMode->ChoosePlayerStart(Controller);
			if (StartSpot)
			{
				// 텔레포트
				LyraCharacter->TeleportTo(StartSpot->GetActorLocation(), StartSpot->GetActorRotation());

				/*
				// 체력 회복
				if (ULyraHealthComponent* HealthComponent = LyraPS->GetHealthComponent())
				{
					// @TODO: 아머는 유지하고 체력만 채우는 로직이 필요할 수 있음.
					// 일단은 최대 체력으로 설정
					// HealthComponent->DamageSelf(..., -1000.0f, ...) 식의 치유나, 직접 값 설정 필요.
					// LyraHealthComponent에는 SetHealth가 protected일 수 있으므로, 
					// GameplayEffect를 적용하여 치유하는 것이 정석이나, 편의상 함수가 있다면 사용.
					
					// LyraHealthComponent 소스 확인 필요. public SetHealth가 없다면 GameplayEffect 사용 권장.
					// 여기서는 임시로직으로 넘어감. (필요 시 Heal GameplayEffect 적용 로직 추가)
				}
				
				// 속도/상태 초기화
				Controller->StopMovement(); */
			}
		}
	}
}

void UCompetitiveMatchScoring::StartNewRound_Implementation()
{
	if (!HasAuthority()) return;
	
	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!IsValid(TeamSubsystem) || !IsValid(GameState))
	{
		return;
	}

	/*
	TeamDeathMap.Empty();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
		{
			const int32 TeamId = LyraPS->GetTeamId();
			if (TeamId != INDEX_NONE)
			{
				TeamDeathMap.FindOrAdd(TeamId).Add(LyraPS);
			}
		}
	}*/
}


void UCompetitiveMatchScoring::OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message)
{
	Super::OnEliminationMessageReceived(Tag, Message);
	if (HasAuthority())
	{
		// 짧은 딜레이 후 체크 (동시 사망 등 엣지 케이스 처리)
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, Message]()
		{
			CheckTeamElimination(Message);
		});
	}
	
}

void UCompetitiveMatchScoring::CheckTeamElimination(const FLyraVerbMessage& Message)
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
	
	// Message.Target should be the PlayerState of the eliminated player
	if (ALyraPlayerState* EliminatedPS = Cast<ALyraPlayerState>(Message.Target))
	{
		const int32 TeamId = EliminatedPS->GetTeamId();
		if (TeamId != INDEX_NONE)
		{
			TeamDeathMap.FindOrAdd(TeamId).AddUnique(EliminatedPS);
		}
	}

	// Calculate total players per team

	TMap<int32, int32> TotalPlayerCounts;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
		{
			const int32 TeamId = LyraPS->GetTeamId();
			if (TeamId != INDEX_NONE)
			{
				TotalPlayerCounts.FindOrAdd(TeamId)++;
			}
		}
	}

	int32 EliminatedTeamId = INDEX_NONE;
	// Check if any team is fully eliminated
	for (const auto& Pair : TotalPlayerCounts)
	{
		const int32 TeamId = Pair.Key;
		const int32 TotalCount = Pair.Value;
		
		if (TeamDeathMap.Contains(TeamId))
		{
			if (TeamDeathMap[TeamId].Num() >= TotalCount)
			{
				EliminatedTeamId = TeamId;
				break;
			}
		}
	}

	if(EliminatedTeamId != INDEX_NONE)
	{
		if (TeamSubsystem->TeamHasTag(EliminatedTeamId, NaviGameplayTags::TAG_Navi_Team_Role_Attacker))
		{
			HandleRoundResult(ERoundWinReason::AttackerTeamEliminated);
		}
		else if (TeamSubsystem->TeamHasTag(EliminatedTeamId, NaviGameplayTags::TAG_Navi_Team_Role_Defender))
		{
			HandleRoundResult(ERoundWinReason::DefenderTeamEliminated);
		}
	}
}

void UCompetitiveMatchScoring::AwardRoundWin(int32 WinningTeamId, ERoundWinReason WinReason)
{
    if (!HasAuthority() || bIsRoundDecided)
    {
        return;
    }
    bIsRoundDecided = true;
    
    // UE_LOG(LogTemp, Log, TEXT("Round decided! Winner: Team %d, Reason: %d"), WinningTeamId, (int32)WinReason);

    AddScoreToTeam(WinningTeamId);
}

void UCompetitiveMatchScoring::AddScoreToTeam(int32 TeamId, int32 ScoreToAdd)
{
	if (!HasAuthority()) return;

	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!TeamSubsystem) return;
	
	TArray<int32> TeamIds = TeamSubsystem->GetTeamIDs();

	// @ Todo: 2팀 체크를 하드코딩 대신 다른 코드 사용?
	if (TeamIds.Num() == 2)
	{
		TeamSubsystem->AddTeamTagStack(TeamId, NaviGameplayTags::Navi_CompetitiveMatch_TeamScore, ScoreToAdd);
		const int32 Team1Score = GetTeamScore(TeamIds[0]);

		const int32 Team2Score = GetTeamScore(TeamIds[1]);
		if (Team1Score >= VictoryScore || Team2Score >= VictoryScore)
		{
			const int32 WinningTeamId = (Team1Score > Team2Score) ? TeamIds[0] : TeamIds[1];
			// HandleVictory(WinningTeamId);
		}
	}
}

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
