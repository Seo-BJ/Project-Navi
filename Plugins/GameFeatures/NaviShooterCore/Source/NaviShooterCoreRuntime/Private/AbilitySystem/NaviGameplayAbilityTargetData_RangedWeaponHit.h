// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"

#include "NaviGameplayAbilityTargetData_RangedWeaponHit.generated.h"

class FArchive;
struct FGameplayEffectContextHandle;

/**
 * 
 */
USTRUCT()
struct NAVISHOOTERCORERUNTIME_API FNaviGameplayAbilityTargetData_RangedWeaponHit : public FLyraGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()
	FNaviGameplayAbilityTargetData_RangedWeaponHit() : ClientHitTime(-1.0f)
	{ }
	
	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

    // 클라이언트가 총을 발사한 과거 시점 (서버 시간 기준)
	UPROPERTY()
	float ClientHitTime;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FNaviGameplayAbilityTargetData_RangedWeaponHit::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FNaviGameplayAbilityTargetData_RangedWeaponHit> : public TStructOpsTypeTraitsBase2<FNaviGameplayAbilityTargetData_RangedWeaponHit>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

