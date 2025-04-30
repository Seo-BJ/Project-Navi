// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/NaviCombatSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

UNaviCombatSet::UNaviCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

void UNaviCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UNaviCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviCombatSet, BaseDamage, OldValue);
}

void UNaviCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviCombatSet, BaseHeal, OldValue);
}

