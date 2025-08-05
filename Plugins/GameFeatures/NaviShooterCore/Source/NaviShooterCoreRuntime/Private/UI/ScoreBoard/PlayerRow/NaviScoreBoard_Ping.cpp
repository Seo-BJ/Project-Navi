// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_Ping.h"
#include "Player/LyraPlayerState.h"
#include "CommonNumericTextBlock.h"

void UNaviScoreBoard_Ping::SetPlayerState(ALyraPlayerState* InPlayerState)
{
	if (Ping_TextBlock && InPlayerState)
	{
		Ping_TextBlock->SetText(FText::AsNumber(FMath::RoundToInt(InPlayerState->GetPingInMilliseconds())));
	}
}
