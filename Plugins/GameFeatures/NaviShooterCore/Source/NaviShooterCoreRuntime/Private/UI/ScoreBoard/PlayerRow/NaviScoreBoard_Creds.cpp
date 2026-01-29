// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_Creds.h"
#include "CredsSystem/NaviCredsSet.h"
#include "Player/LyraPlayerState.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "CommonNumericTextBlock.h"

void UNaviScoreBoard_Creds::SetPlayerState_Implementation(ALyraPlayerState* InPlayerState)
{
	if (Creds_TextBlock && InPlayerState)
	{
		ULyraAbilitySystemComponent* LyraASC = InPlayerState->GetLyraAbilitySystemComponent();
		if (LyraASC)
		{
			const UNaviCredsSet* CredsSet = LyraASC->GetSet<UNaviCredsSet>();
			{
				float Creds = CredsSet->GetCreds();
				Creds_TextBlock->SetText(FText::AsNumber(FMath::FloorToInt(Creds)));
			}
		}
	}
}
