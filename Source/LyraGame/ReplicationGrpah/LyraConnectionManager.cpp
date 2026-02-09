// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraConnectionManager.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

TArray<ULyraConnectionManager*>* FTeamConnectionListMap::GetConnectionArrayForTeam(int32 Team)
{
	return Find(Team);
}

TArray<ULyraConnectionManager*> FTeamConnectionListMap::GetVisibleConnectionArrayForNonTeam(const APawn* Pawn,
	int32 Team)
{
	TArray<ULyraConnectionManager*> NonTeamConnections;

	if (!IsValid(Pawn))
	{
		return NonTeamConnections;
	}

	// Setup query params and ignore all team members
	TArray<ULyraConnectionManager*>* TeamMembers = GetConnectionArrayForTeam(Team);
		
	FCollisionQueryParams TraceParams;
	if (TeamMembers)
	{
		for (const ULyraConnectionManager* ConnectionManager : *TeamMembers)
		{
			TraceParams.AddIgnoredActor(ConnectionManager->Pawn.Get());
		}
	}
	else
	{
		TraceParams.AddIgnoredActor(Pawn);
	}

	// Iterate over all teams that do not match the input team
	TArray<int32> Teams;
	GetKeys(Teams);

	const UWorld* World = Pawn->GetWorld();
	const FVector TraceOffset = FVector(0.0f, 0.0f, 180.0f);
	const FVector TraceStart = Pawn->GetActorLocation() + TraceOffset;
	for (int32 i = 0; i < Teams.Num(); i++)
	{
		const int32 TeamID = Teams[i];
		if (TeamID != Team)
		{
			const TArray<ULyraConnectionManager*>* OtherTeamMembers = GetConnectionArrayForTeam(TeamID);

			if (OtherTeamMembers)
			{
				for (ULyraConnectionManager* ConnectionManager : *OtherTeamMembers)
				{
					if (!ConnectionManager->Pawn.IsValid())
					{
						continue;
					}
					
					// Raycast between our pawn and the other. If we hit anything then we do not have line of sight
					FHitResult OutHit;
					const FVector TraceEnd = ConnectionManager->Pawn.Get()->GetActorLocation() + TraceOffset;
					if (!World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_GameTraceChannel1, TraceParams))
					{
						NonTeamConnections.Add(ConnectionManager);		
					}
				}
			}
		}
	}

	return NonTeamConnections;
}

void FTeamConnectionListMap::AddConnectionToTeam(int32 Team, ULyraConnectionManager* ConnManager)
{
	TArray<ULyraConnectionManager*>& TeamList = FindOrAdd(Team);
	TeamList.Add(ConnManager);
}

void FTeamConnectionListMap::RemoveConnectionFromTeam(int32 Team, ULyraConnectionManager* ConnManager)
{
	if (TArray<ULyraConnectionManager*>* TeamList = Find(Team))
	{
		TeamList->RemoveSwap(ConnManager);

		// Remove the team from the map if there are no more connections
		if (TeamList->Num() == 0)
		{
			Remove(Team);
		}
	}
}
