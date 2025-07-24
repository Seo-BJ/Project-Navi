// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/NaviPlayerCredsIndicatorWidget.h"

#include "CommonNumericTextBlock.h"
#include "Components/Image.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "CredsSystem/NaviCredsSet.h"
#include "Player/LyraPlayerState.h"
#include "GameplayEffect.h"

void UNaviPlayerCredsIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Creds_TextBlock)
	{
		Creds_TextBlock->FormattingSpecification.MaximumFractionalDigits = 0;
	}
	if (ALyraPlayerState* PS = GetOwningPlayerState<ALyraPlayerState>())
	{
		if (ULyraAbilitySystemComponent* ASC = PS->GetLyraAbilitySystemComponent())
		{
			if (const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>())
			{
				if (Creds_TextBlock)
				{
					Creds_TextBlock->SetCurrentValue(CredsSet->GetCreds());
				}
				CredsChangedDelegateHandle = CredsSet->OnCredsChanged.AddUObject(this, &ThisClass::HandleCredsChanged);
			}
		}
	}
}
void UNaviPlayerCredsIndicatorWidget::NativeDestruct()
{
	if (ALyraPlayerState* PS = GetOwningPlayerState<ALyraPlayerState>())
	{
		ULyraAbilitySystemComponent* ASC = PS->GetLyraAbilitySystemComponent();
		if (ASC)
		{
			const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>();
			if (CredsSet)
			{
				CredsSet->OnCredsChanged.Remove(CredsChangedDelegateHandle);
				CredsChangedDelegateHandle.Reset();
			}
		}
	}
	Super::NativeDestruct();
	
}

void UNaviPlayerCredsIndicatorWidget::HandleCredsChanged(AActor* Instigator, AActor* Causer,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	SetCreds(NewValue);
}
void UNaviPlayerCredsIndicatorWidget::SetCreds(float NewCreds)
{
	if (Creds_TextBlock)
	{
		Creds_TextBlock->SetCurrentValue(NewCreds);
	}
}

