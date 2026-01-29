// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/BasePlayerRow.h"
#include "Player/LyraPlayerState.h"

void UBasePlayerRow::SetPlayerState_Implementation(ALyraPlayerState* InPlayerState)
{
	AssociatedPlayerState = InPlayerState;

	const TArray<UWidget*> PlayerStateAwareChildren = GetPlayerStateAwareChildren();

	for (UWidget* ChildWidget : PlayerStateAwareChildren)
	{
		if (ChildWidget && ChildWidget->Implements<UTakesLyraPlayerState>())
		{
			Cast<ITakesLyraPlayerState>(ChildWidget)->SetPlayerState(InPlayerState);
		}
	}
}

TArray<UWidget*> UBasePlayerRow::GetPlayerStateAwareChildren() const
{
	TArray<UWidget*> PlayerStateAwareChildren;
	return PlayerStateAwareChildren;
}
