// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_PlayerRow.h"
#include "Player/LyraPlayerState.h"
#include "Components/HorizontalBox.h"

TArray<UWidget*> UNaviScoreBoard_PlayerRow::GetPlayerStateAwareChildren() const
{
	TArray<UWidget*> PlayerStateAwareChildren;
	if (HorizontalBox)
	{
		PlayerStateAwareChildren = HorizontalBox->GetAllChildren();;
		if (PlayerStateAwareChildren.Num() > 0)
		{
			for (UWidget* ChildWidget : PlayerStateAwareChildren)
			{
				if (ChildWidget && ChildWidget->Implements<UTakesLyraPlayerState>())
				{
					PlayerStateAwareChildren.Add(ChildWidget);
				}
			}
		}
	}
	return PlayerStateAwareChildren;
}

void UNaviScoreBoard_PlayerRow::SetPlayerState(ALyraPlayerState* InPlayerState)
{
	Super::SetPlayerState(InPlayerState);
}
