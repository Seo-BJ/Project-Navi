// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NaviAbilitySystemGlobals.h"

#include "Abilitysystem/LyraGameplayEffectContext.h"

struct FGameplayEffectContext;

UNaviAbilitySystemGlobals::UNaviAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UNaviAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FLyraGameplayEffectContext();
}

