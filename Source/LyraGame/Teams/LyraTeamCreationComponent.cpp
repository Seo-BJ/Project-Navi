// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTeamCreationComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "LyraTeamPublicInfo.h"
#include "LyraTeamPrivateInfo.h"
#include "Player/LyraPlayerState.h"
#include "Engine/World.h"
#include "GameModes/LyraGameMode.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "LyraLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTeamCreationComponent)

ULyraTeamCreationComponent::ULyraTeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PublicTeamInfoClass = ALyraTeamPublicInfo::StaticClass();
	PrivateTeamInfoClass = ALyraTeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult ULyraTeamCreationComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	//@TODO: TEAMS: Validate that all display assets have the same properties set!

	return Result;
}
#endif

void ULyraTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void ULyraTeamCreationComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority() && bActivateOnExperienceLoaded)
	{
		ServerCreateTeams();
		ServerAssignPlayersToTeams();
	}
#endif
}

#if WITH_SERVER_CODE

void ULyraTeamCreationComponent::ServerCreateTeams()
{
	for (const auto& KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

void ULyraTeamCreationComponent::ServerAssignPlayersToTeams()
{
	// Assign players that already exist to teams
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
		{
			ServerChooseTeamForPlayer(LyraPS);
		}
	}

	// Listen for new players logging in
	ALyraGameMode* GameMode = Cast<ALyraGameMode>(GameState->AuthorityGameMode);
	check(GameMode);

	GameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
}

void ULyraTeamCreationComponent::OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);
	if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(NewPlayer->PlayerState))
	{
		ServerChooseTeamForPlayer(LyraPS);
	}
}

void ULyraTeamCreationComponent::ServerChooseTeamForPlayer(ALyraPlayerState* PS)
{
	if (!IsValid(PS)) return;
	if (PS->IsOnlyASpectator())
	{
		PS->SetGenericTeamId(FGenericTeamId::NoTeam);
		PS->SetIndexInTeam(-1);
	}
	else
	{
		const int32 BestTeamId = GetLeastPopulatedTeamID(); // 가장 적합한 팀 ID 찾기

		if (BestTeamId != INDEX_NONE) // 유효한 팀 ID를 찾았다면
		{
			// --- 팀 내 인덱스 계산 로직 시작 ---
			int32 CurrentPlayerCountInTeam = 0;
			AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
			for (APlayerState* ExistingPS : GameState->PlayerArray)
			{
				// 유효하고, 같은 팀이며, 비활성 상태가 아닌 플레이어 수 계산
				if (ExistingPS && ExistingPS != PS) // 자기 자신은 제외 (아직 팀 배정 전이므로)
				{
					if (ALyraPlayerState* ExistingLyraPS = Cast<ALyraPlayerState>(ExistingPS))
					{
						// GetTeamId()가 int32를 반환한다고 가정합니다. 만약 FGenericTeamId를 반환하면 .GetId() 등을 사용해야 할 수 있습니다.
						if (ExistingLyraPS->GetTeamId() == BestTeamId && !ExistingLyraPS->IsInactive())
						{
							CurrentPlayerCountInTeam++;
						}
					}
				}
			}
			const int32 NewIndexInTeam = CurrentPlayerCountInTeam; // 현재 인원수가 새 플레이어의 인덱스 (0부터 시작)
			// --- 팀 내 인덱스 계산 로직 끝 ---

			// 팀 ID 설정
			const FGenericTeamId TeamID = IntegerToGenericTeamId(BestTeamId); // FGenericTeamId로 변환 필요 시
			PS->SetGenericTeamId(TeamID);

			// 팀 내 인덱스 설정 (새로 추가된 핵심 부분)
			PS->SetIndexInTeam(NewIndexInTeam);

			UE_LOG(LogLyraTeams, Log, TEXT("ServerChooseTeamForPlayer: Assigned Player %s to Team %d, Index %d"), *PS->GetPlayerName(), BestTeamId, NewIndexInTeam);
		}
		else
		{
			UE_LOG(LogLyraTeams, Warning, TEXT("ServerChooseTeamForPlayer: Could not find a valid team for Player %s."), *PS->GetPlayerName());
			// 유효한 팀이 없는 경우 (예: 모든 팀이 꽉 찼거나 정의된 팀이 없을 때)
			PS->SetGenericTeamId(FGenericTeamId::NoTeam);
			PS->SetIndexInTeam(-1);
		}
		
		
	}
}


void ULyraTeamCreationComponent::ServerCreateTeam(int32 TeamId, ULyraTeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	//@TODO: ensure the team doesn't already exist

	UWorld* World = GetWorld();
	check(World);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALyraTeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<ALyraTeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
	checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
	NewTeamPublicInfo->SetTeamId(TeamId);
	NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

	ALyraTeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<ALyraTeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
	checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
	NewTeamPrivateInfo->SetTeamId(TeamId);
}

int32 ULyraTeamCreationComponent::GetLeastPopulatedTeamID() const
{
	const int32 NumTeams = TeamsToCreate.Num();
	if (NumTeams > 0)
	{
		TMap<int32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);

		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			TeamMemberCounts.Add(TeamId, 0);
		}

		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
			{
				const int32 PlayerTeamID = LyraPS->GetTeamId();

				if ((PlayerTeamID != INDEX_NONE) && !LyraPS->IsInactive())	// do not count unassigned or disconnected players
				{
					check(TeamMemberCounts.Contains(PlayerTeamID))
					TeamMemberCounts[PlayerTeamID] += 1;
				}
			}
		}

		// sort by lowest team population, then by team ID
		int32 BestTeamId = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 TestTeamId = KVP.Key;
			const uint32 TestTeamPlayerCount = KVP.Value;

			if (TestTeamPlayerCount < BestPlayerCount)
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
			else if (TestTeamPlayerCount == BestPlayerCount)
			{
				if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
				{
					BestTeamId = TestTeamId;
					BestPlayerCount = TestTeamPlayerCount;
				}
			}
		}

		return BestTeamId;
	}

	return INDEX_NONE;
}


// WITH_SERVER_CODE

void ULyraTeamCreationComponent::SetPlayerTeam(ALyraPlayerState* PlayerStateToSet, int32 NewTeamID)
{
	check(HasAuthority()); // 서버 권한 확인
    check(PlayerStateToSet); // 대상 플레이어 상태 유효성 확인

    // 목표 팀 ID 유효성 검사
    const bool bIsValidTeamId = (NewTeamID == FGenericTeamId::NoTeam) || TeamsToCreate.Contains(static_cast<uint8>(NewTeamID));
    if (!bIsValidTeamId)
    {
        UE_LOG(LogLyraTeams, Warning, TEXT("SetPlayerTeam: Attempted to set invalid Team ID %d for player %s."), NewTeamID, *PlayerStateToSet->GetPlayerName());
        return;
    }

    const FGenericTeamId CurrentTeamId = PlayerStateToSet->GetGenericTeamId();
    const int32 CurrentIndexInTeam = PlayerStateToSet->GetIndexInTeam(); // 현재 인덱스 가져오기
    const FGenericTeamId TargetTeamId = FGenericTeamId(NewTeamID);

    // 실제로 팀이 변경되는 경우에만 아래 로직 수행
    if (CurrentTeamId != TargetTeamId)
    {
        UE_LOG(LogLyraTeams, Log, TEXT("SetPlayerTeam: Requesting team change for %s from %d (Index %d) to %d."),
            *PlayerStateToSet->GetPlayerName(), CurrentTeamId.GetId(), CurrentIndexInTeam, TargetTeamId.GetId());

        AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>(); // GameState 가져오기

        // --- 1. 원래 팀에 남은 플레이어들의 인덱스 업데이트 ---
        // 플레이어가 유효한 팀에서 떠나는 경우에만 실행 (NoTeam에서 옮기는 경우는 제외)
        if (CurrentTeamId != FGenericTeamId::NoTeam && CurrentIndexInTeam != -1)
        {
            TArray<ALyraPlayerState*> PlayersInOriginalTeam; // 원래 팀 플레이어 임시 저장
            for (APlayerState* PS : GameState->PlayerArray)
            {
                if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
                {
                    // 자기 자신 제외, 원래 팀 소속, 유효한 인덱스, 비활성 아님
                    if (LyraPS != PlayerStateToSet &&
                        LyraPS->GetGenericTeamId() == CurrentTeamId &&
                        LyraPS->GetIndexInTeam() != -1 &&
                        !LyraPS->IsInactive())
                    {
                        PlayersInOriginalTeam.Add(LyraPS);
                    }
                }
            }

            // 인덱스 기준으로 정렬 (선택 사항이지만, 확실하게 하려면)
            PlayersInOriginalTeam.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
                return A.GetIndexInTeam() < B.GetIndexInTeam();
            });
        	
            // 떠난 플레이어보다 뒷 순서였던 플레이어들의 인덱스 감소
            int32 NewIndexCounter = 0;
            for(ALyraPlayerState* RemainingPlayer : PlayersInOriginalTeam)
            {
                 // 떠난 플레이어보다 앞 순서였던 플레이어는 인덱스 유지
                 if (RemainingPlayer->GetIndexInTeam() < CurrentIndexInTeam)
                 {
                     NewIndexCounter++; // 다음 인덱스 준비
                     continue;
                 }
                 // 떠난 플레이어보다 뒷 순서였던 플레이어는 인덱스 1 감소
                 if (RemainingPlayer->GetIndexInTeam() > CurrentIndexInTeam)
                 {
                     int32 DecrementedIndex = RemainingPlayer->GetIndexInTeam() - 1;
                     // 혹은 NewIndexCounter 사용 (더 안전)
                     // int32 DecrementedIndex = NewIndexCounter;
                     RemainingPlayer->SetIndexInTeam(DecrementedIndex);
                      UE_LOG(LogLyraTeams, Verbose, TEXT("SetPlayerTeam: Updated index for %s in original team %d to %d."),
                            *RemainingPlayer->GetPlayerName(), CurrentTeamId.GetId(), DecrementedIndex);
                      NewIndexCounter++;
                 }
                 // 만약 떠난 플레이어와 같은 인덱스였다면? (데이터 오류 가능성) -> 로그 남기거나 처리 필요
            }
        }

        // --- 2. 팀을 옮긴 플레이어의 새 인덱스 계산 및 설정 ---
        int32 NewIndexInTargetTeam = -1;
        if (TargetTeamId != FGenericTeamId::NoTeam) // 유효한 팀으로 옮기는 경우
        {
            int32 PlayerCountInTargetTeam = 0;
            for (APlayerState* PS : GameState->PlayerArray)
            {
                 // 자기 자신 제외, 목표 팀 소속, 비활성 아님
                 if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
                 {
                     if (LyraPS != PlayerStateToSet &&
                         LyraPS->GetGenericTeamId() == TargetTeamId &&
                         !LyraPS->IsInactive())
                     {
                         PlayerCountInTargetTeam++;
                     }
                 }
            }
            NewIndexInTargetTeam = PlayerCountInTargetTeam; // 새 팀의 마지막 인덱스
        }
        // (TargetTeamId가 NoTeam이면 NewIndexInTargetTeam은 -1 유지됨)

        // --- 3. 팀을 옮긴 플레이어의 상태 최종 업데이트 ---
        PlayerStateToSet->SetIndexInTeam(NewIndexInTargetTeam); // 새 인덱스 설정
        PlayerStateToSet->SetGenericTeamId(TargetTeamId);   // 새 팀 ID 설정

        UE_LOG(LogLyraTeams, Log, TEXT("SetPlayerTeam: Completed team change for %s to Team ID %d with Index %d."),
               *PlayerStateToSet->GetPlayerName(), TargetTeamId.GetId(), NewIndexInTargetTeam);

        // TODO: 팀 변경 시 추가적으로 필요한 로직이 있다면 여기에 구현
        // (예: 플레이어 스폰 재시작, UI 업데이트 트리거 등)
    }
    else // 팀 변경이 없는 경우 (같은 팀을 다시 누름)
    {
        UE_LOG(LogLyraTeams, Log, TEXT("SetPlayerTeam: Player %s is already on Team ID %d. No change needed."), *PlayerStateToSet->GetPlayerName(), NewTeamID);
        // 필요하다면 여기서도 인덱스 재정렬 로직 호출 고려 가능
    }
}
#endif

void ULyraTeamCreationComponent::ServerHandlePlayerLogout(AController* LeavingController)
{
 check(HasAuthority()); // 서버에서만 실행되어야 함

    if (!LeavingController)
    {
        UE_LOG(LogLyraTeams, Warning, TEXT("ServerHandlePlayerLogout: LeavingController is null."));
        return;
    }

    // 로그아웃하는 플레이어의 PlayerState 가져오기
    ALyraPlayerState* LeavingPlayerState = LeavingController->GetPlayerState<ALyraPlayerState>();
    if (!LeavingPlayerState)
    {
        // 로그아웃 시점에는 PlayerState가 아직 유효해야 하지만, 안전하게 체크
        UE_LOG(LogLyraTeams, Warning, TEXT("ServerHandlePlayerLogout: Could not get LyraPlayerState for leaving controller %s."), *LeavingController->GetName());
        return;
    }

    const FGenericTeamId LeavingTeamId = LeavingPlayerState->GetGenericTeamId();
    const int32 LeavingIndexInTeam = LeavingPlayerState->GetIndexInTeam();

    UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Player %s (Team: %d, Index: %d) is logging out."),
        *LeavingPlayerState->GetPlayerName(), LeavingTeamId.GetId(), LeavingIndexInTeam);

    // 플레이어가 유효한 팀에 속해 있었고 유효한 인덱스를 가졌는지 확인
    if (LeavingTeamId == FGenericTeamId::NoTeam || LeavingIndexInTeam == -1)
    {
        UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Leaving player was not in a valid team or had no valid index. No index update needed."));
        return; // 팀 없거나 인덱스 없으면 다른 플레이어 인덱스 변경 불필요
    }

    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    if (!GameState)
    {
        return; // GameState 없으면 처리 불가
    }

    // 떠난 플레이어와 같은 팀에 있던 다른 플레이어들을 찾습니다.
    TArray<ALyraPlayerState*> RemainingPlayersInTeam;
    for (APlayerState* PS : GameState->PlayerArray)
    {
        // 자기 자신(떠나는 플레이어) 제외
        if (PS && PS != LeavingPlayerState)
        {
            if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
            {
                // 같은 팀이고, 비활성 상태가 아닌 플레이어
                if (LyraPS->GetGenericTeamId() == LeavingTeamId && !LyraPS->IsInactive())
                {
                    RemainingPlayersInTeam.Add(LyraPS);
                }
            }
        }
    }

    // 남은 플레이어가 없다면 업데이트할 필요 없음
    if (RemainingPlayersInTeam.IsEmpty())
    {
        UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: No remaining players found in team %d."), LeavingTeamId.GetId());
        return;
    }

    UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Found %d remaining players in team %d. Re-assigning indices."), RemainingPlayersInTeam.Num(), LeavingTeamId.GetId());

	
	RemainingPlayersInTeam.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
		return A.GetIndexInTeam() < B.GetIndexInTeam();
	});

	
    // 정렬된 순서대로 0부터 인덱스 재할당
    for (int32 i = 0; i < RemainingPlayersInTeam.Num(); ++i)
    {
        ALyraPlayerState* RemainingPlayer = RemainingPlayersInTeam[i];
        if (RemainingPlayer)
        {
            // 현재 인덱스와 새로 부여될 인덱스가 다르면 업데이트
            if (RemainingPlayer->GetIndexInTeam() != i)
            {
                UE_LOG(LogLyraTeams, Verbose, TEXT("ServerHandlePlayerLogout: Updating index for %s from %d to %d in team %d."),
                    *RemainingPlayer->GetPlayerName(), RemainingPlayer->GetIndexInTeam(), i, LeavingTeamId.GetId());

                RemainingPlayer->SetIndexInTeam(i);
            }
            // 인덱스가 이미 순서대로 맞다면 로그만 남기거나 아무것도 안 함 (선택)
            // else {
            //    UE_LOG(LogLyraTeams, Verbose, TEXT("ServerHandlePlayerLogout: Index for %s is already correct (%d) in team %d."),
            //         *RemainingPlayer->GetPlayerName(), i, LeavingTeamId.GetId());
            // }
        }
    }
     UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Finished re-assigning indices for team %d."), LeavingTeamId.GetId());
}



/*
check(HasAuthority());
	check(PlayerStateToSet);
	
	const bool bIsValidTeamId = (NewTeamID == FGenericTeamId::NoTeam) || TeamsToCreate.Contains(static_cast<uint8>(NewTeamID));
	if (!bIsValidTeamId)
	{
		UE_LOG(LogLyraTeams, Warning, TEXT("SetPlayerTeam: Attempted to set invalid Team ID %d for player %s."), NewTeamID, *PlayerStateToSet->GetPlayerName());
		return;
	}
	
	const FGenericTeamId CurrentTeamId = PlayerStateToSet->GetGenericTeamId();
	const FGenericTeamId TargetTeamId = FGenericTeamId(NewTeamID); 

	if (CurrentTeamId != TargetTeamId)
	{
		PlayerStateToSet->SetGenericTeamId(TargetTeamId);
		UE_LOG(LogLyraTeams, Log, TEXT("SetPlayerTeam: Set player %s to Team ID %d."), *PlayerStateToSet->GetPlayerName(), NewTeamID);

		// TODO: 팀 변경 시 추가적으로 필요한 로직이 있다면 여기에 구현
		// (예: 플레이어 스폰 재시작, UI 업데이트 트리거 등)
	}
	else
	{
		UE_LOG(LogLyraTeams, Log, TEXT("SetPlayerTeam: Player %s is already on Team ID %d."), *PlayerStateToSet->GetPlayerName(), NewTeamID);
	}
 
 **/