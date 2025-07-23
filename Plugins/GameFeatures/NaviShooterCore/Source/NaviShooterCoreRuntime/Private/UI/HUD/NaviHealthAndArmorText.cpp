// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/NaviHealthAndArmorText.h"

#include "Character/LyraHealthComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "CommonNumericTextBlock.h"

void UNaviHealthAndArmorText::NativeConstruct()
{
	Super::NativeConstruct();

	GetOwningPlayer()->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::OnPossessedPawnChanged);
	OnPossessedPawnChanged(nullptr, GetOwningPlayer()->GetPawn());
}

void UNaviHealthAndArmorText::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	ULyraHealthComponent* OldPawnHealthComp = ULyraHealthComponent::FindHealthComponent(OldPawn);
	if (OldPawnHealthComp != nullptr)
	{
		OldPawnHealthComp->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealthChanged);
		OldPawnHealthComp->OnArmorChanged.RemoveDynamic(this, &ThisClass::OnArmorChanged);
	}
	
	ULyraHealthComponent* NewPawnHealthComp = ULyraHealthComponent::FindHealthComponent(NewPawn);
	if (NewPawnHealthComp != nullptr)
	{
		NewPawnHealthComp->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::OnHealthChanged);
		NewPawnHealthComp->OnArmorChanged.AddUniqueDynamic(this, &ThisClass::OnArmorChanged);
	}
	
}

void UNaviHealthAndArmorText::OnHealthChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue,
	AActor* Instigator)
{
	HealthAmount_TextBlock->SetCurrentValue(NewValue);
}

void UNaviHealthAndArmorText::OnArmorChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue,
	AActor* Instigator)
{
	ArmorAmount_TextBlock->SetCurrentValue(NewValue);
}
