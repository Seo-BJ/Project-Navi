// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviGameplayAbilityTargetData_RangedWeaponHit.h"

#include "AbilitySystem/LyraGameplayEffectContext.h"

struct FGameplayEffectContextHandle;

//////////////////////////////////////////////////////////////////////


void FNaviGameplayAbilityTargetData_RangedWeaponHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// Add game-specific data
	if (FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Context))
	{
		TypedContext->ClientHitTime = ClientHitTime;
	}
}

bool FNaviGameplayAbilityTargetData_RangedWeaponHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	Ar << ClientHitTime;

	return true;
}

