// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NaviPlayerSpawningManagerComponent.h"

#include "Player/NaviPlayerStart.h"
#include "Teams/LyraTeamSubsystem.h"


UNaviPlayerSpawningManagerComponent::UNaviPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

AActor* UNaviPlayerSpawningManagerComponent::OnChoosePlayerStart(AController* Player,
	TArray<ALyraPlayerStart*>& PlayerStarts)
{
	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!ensure(TeamSubsystem))
	{
		return Super::OnChoosePlayerStart(Player, PlayerStarts);
	}

	const int32 PlayerTeamId = TeamSubsystem->FindTeamFromObject(Player);

	if (PlayerTeamId == INDEX_NONE)
	{
		return Super::OnChoosePlayerStart(Player, PlayerStarts);
	}

	TArray<ALyraPlayerStart*> TeamSpecificStartPoints;
	for (ALyraPlayerStart* StartPoint : PlayerStarts)
	{
		if (ANaviPlayerStart* TeamStartPoint = Cast<ANaviPlayerStart>(StartPoint))
		{
			if (TeamStartPoint->TeamId == PlayerTeamId)
			{
				TeamSpecificStartPoints.Add(StartPoint);
			}
		}
	}

	if (TeamSpecificStartPoints.Num() > 0)
	{
		return GetFirstRandomUnoccupiedPlayerStart(Player, TeamSpecificStartPoints);
	}
	
	return nullptr;
}
