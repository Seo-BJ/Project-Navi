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
	if (HasAuthority())
	{
		if (bActivateOnExperienceLoaded)
		{
			ServerCreateTeams();
			ServerAssignPlayersToTeams();
		}
		else
		{
			ServerCreateTeams();
		}
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
		const int32 BestTeamId = GetLeastPopulatedTeamID(); 

		if (BestTeamId != INDEX_NONE) 
		{
			int32 CurrentPlayerCountInTeam = 0;
			AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
			for (APlayerState* ExistingPS : GameState->PlayerArray)
			{
				if (ExistingPS && ExistingPS != PS) 
				{
					if (ALyraPlayerState* ExistingLyraPS = Cast<ALyraPlayerState>(ExistingPS))
					{
						if (ExistingLyraPS->GetTeamId() == BestTeamId && !ExistingLyraPS->IsInactive())
						{
							CurrentPlayerCountInTeam++;
						}
					}
				}
			}
			const int32 NewIndexInTeam = CurrentPlayerCountInTeam; 
			
			const FGenericTeamId TeamID = IntegerToGenericTeamId(BestTeamId); 
			PS->SetGenericTeamId(TeamID);
			
			PS->SetIndexInTeam(NewIndexInTeam);
		}
		else
		{
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

void ULyraTeamCreationComponent::ServerSetPlayerTeam(ALyraPlayerState* PlayerStateToSet, int32 NewTeamID)
{
	check(HasAuthority());
    check(PlayerStateToSet); 

    const bool bIsValidTeamId = (NewTeamID == FGenericTeamId::NoTeam) || TeamsToCreate.Contains(static_cast<uint8>(NewTeamID));
    if (!bIsValidTeamId)
    {
        UE_LOG(LogLyraTeams, Warning, TEXT("ServerSetPlayerTeam: Attempted to set invalid Team ID %d for player %s."), NewTeamID, *PlayerStateToSet->GetPlayerName());
        return;
    }

    const FGenericTeamId CurrentTeamId = PlayerStateToSet->GetGenericTeamId();
    const int32 CurrentIndexInTeam = PlayerStateToSet->GetIndexInTeam(); 
    const FGenericTeamId TargetTeamId = FGenericTeamId(NewTeamID);

    if (CurrentTeamId != TargetTeamId)
    {
        UE_LOG(LogLyraTeams, Log, TEXT("ServerSetPlayerTeam: Requesting team change for %s from %d (Index %d) to %d."),
            *PlayerStateToSet->GetPlayerName(), CurrentTeamId.GetId(), CurrentIndexInTeam, TargetTeamId.GetId());

        AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>(); 
        if (CurrentTeamId != FGenericTeamId::NoTeam && CurrentIndexInTeam != -1)
        {
            TArray<ALyraPlayerState*> PlayersInOriginalTeam; 
            for (APlayerState* PS : GameState->PlayerArray)
            {
                if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
                {
                    if (LyraPS != PlayerStateToSet &&
                        LyraPS->GetGenericTeamId() == CurrentTeamId &&
                        LyraPS->GetIndexInTeam() != -1 &&
                        !LyraPS->IsInactive())
                    {
                        PlayersInOriginalTeam.Add(LyraPS);
                    }
                }
            }

            PlayersInOriginalTeam.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
                return A.GetIndexInTeam() < B.GetIndexInTeam();
            });
        	
            int32 NewIndexCounter = 0;
            for(ALyraPlayerState* RemainingPlayer : PlayersInOriginalTeam)
            {
                 if (RemainingPlayer->GetIndexInTeam() < CurrentIndexInTeam)
                 {
                     NewIndexCounter++; 
                     continue;
                 }
                 if (RemainingPlayer->GetIndexInTeam() > CurrentIndexInTeam)
                 {
                     int32 DecrementedIndex = RemainingPlayer->GetIndexInTeam() - 1;
                     RemainingPlayer->SetIndexInTeam(DecrementedIndex);
                      // UE_LOG(LogLyraTeams, Verbose, TEXT("ServerSetPlayerTeam: Updated index for %s in original team %d to %d."),*RemainingPlayer->GetPlayerName(), CurrentTeamId.GetId(), DecrementedIndex);
                      NewIndexCounter++;
                 }
            }
        }

        int32 NewIndexInTargetTeam = -1;
        if (TargetTeamId != FGenericTeamId::NoTeam) 
        {
            int32 PlayerCountInTargetTeam = 0;
            for (APlayerState* PS : GameState->PlayerArray)
            {
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
            NewIndexInTargetTeam = PlayerCountInTargetTeam; 
        }

        PlayerStateToSet->SetIndexInTeam(NewIndexInTargetTeam); 
        PlayerStateToSet->SetGenericTeamId(TargetTeamId);   

        UE_LOG(LogLyraTeams, Log, TEXT("ServerSetPlayerTeam: Completed team change for %s to Team ID %d with Index %d."),
               *PlayerStateToSet->GetPlayerName(), TargetTeamId.GetId(), NewIndexInTargetTeam);

        // TODO: 팀 변경 시 추가적으로 필요한 로직 추가 가능
    }
}
#endif

void ULyraTeamCreationComponent::ServerHandlePlayerLogout(AController* LeavingController)
{
	check(HasAuthority()); 

    if (!LeavingController)
    {
        UE_LOG(LogLyraTeams, Warning, TEXT("ServerHandlePlayerLogout: LeavingController is null."));
        return;
    }

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

    if (LeavingTeamId == FGenericTeamId::NoTeam || LeavingIndexInTeam == -1)
    {
        UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Leaving player was not in a valid team or had no valid index. No index update needed."));
        return; 
    }

    AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
    if (!GameState)
    {
        return; 
    }

    TArray<ALyraPlayerState*> RemainingPlayersInTeam;
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (PS && PS != LeavingPlayerState)
        {
            if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
            {
                if (LyraPS->GetGenericTeamId() == LeavingTeamId && !LyraPS->IsInactive())
                {
                    RemainingPlayersInTeam.Add(LyraPS);
                }
            }
        }
    }

    if (RemainingPlayersInTeam.IsEmpty())
    {
        UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: No remaining players found in team %d."), LeavingTeamId.GetId());
        return;
    }

    UE_LOG(LogLyraTeams, Log, TEXT("ServerHandlePlayerLogout: Found %d remaining players in team %d. Re-assigning indices."), RemainingPlayersInTeam.Num(), LeavingTeamId.GetId());

	
	RemainingPlayersInTeam.Sort([](const ALyraPlayerState& A, const ALyraPlayerState& B) {
		return A.GetIndexInTeam() < B.GetIndexInTeam();
	});

	for (int32 i = 0; i < RemainingPlayersInTeam.Num(); ++i)
    {
        ALyraPlayerState* RemainingPlayer = RemainingPlayersInTeam[i];
        if (RemainingPlayer)
        {
            if (RemainingPlayer->GetIndexInTeam() != i)
            {
                UE_LOG(LogLyraTeams, Verbose, TEXT("ServerHandlePlayerLogout: Updating index for %s from %d to %d in team %d."),
                    *RemainingPlayer->GetPlayerName(), RemainingPlayer->GetIndexInTeam(), i, LeavingTeamId.GetId());

                RemainingPlayer->SetIndexInTeam(i);
            }
        }
    }
}

