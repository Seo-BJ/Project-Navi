// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_KDA.h"
#include "CommonNumericTextBlock.h"
#include "Player/LyraPlayerState.h"
#include "LyraGameplayTags.h"

void UNaviScoreBoard_KDA::SetPlayerState(ALyraPlayerState* InPlayerState)
{
	if (Kill_TextBlock && Death_TextBlock && Assist_TextBlock && InPlayerState)
	{
		Kill_TextBlock->SetText(FText::AsNumber(InPlayerState->GetStatTagStackCount(NaviGameplayTags::Navi_Score_Eliminations)));
		Death_TextBlock->SetText(FText::AsNumber(InPlayerState->GetStatTagStackCount(NaviGameplayTags::Navi_Score_Deaths)));
		Assist_TextBlock->SetText(FText::AsNumber(InPlayerState->GetStatTagStackCount(NaviGameplayTags::Navi_Score_Assists)));
	}
}
