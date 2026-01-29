// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_Ultimate.h"
#include "CommonNumericTextBlock.h"
#include "LyraGameplayTags.h"
#include "Player/LyraPlayerState.h"

void UNaviScoreBoard_Ultimate::SetPlayerState_Implementation(ALyraPlayerState* InPlayerState)
{
	if (CurrentUltimatePoint_TextBlock && RequiredUltimatePoint_TextBlock && InPlayerState)
	{
		CurrentUltimatePoint_TextBlock->SetText(FText::AsNumber(InPlayerState->GetStatTagStackCount
		(NaviGameplayTags::Agent_Ultimate_CurrentPoint)));
		RequiredUltimatePoint_TextBlock->SetText(FText::AsNumber(InPlayerState->GetStatTagStackCount
		(NaviGameplayTags::Agent_Ultimate_RequiredPoint)));
	}
}
