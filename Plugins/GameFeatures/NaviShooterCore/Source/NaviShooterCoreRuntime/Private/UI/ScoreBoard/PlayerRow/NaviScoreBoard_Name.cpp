// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_Name.h"
#include "Player/LyraPlayerState.h"
#include "CommonTextBlock.h"

void UNaviScoreBoard_Name::SetPlayerState(ALyraPlayerState* InPlayerState)
{
	if (PlayerName_TextBlock && InPlayerState)
	{
		FString PlayerName = InPlayerState->GetPlayerName();
		PlayerName_TextBlock->SetText(FText::FromString(PlayerName));
	}
}
