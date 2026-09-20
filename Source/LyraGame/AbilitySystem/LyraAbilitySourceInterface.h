// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"

#include "LyraAbilitySourceInterface.generated.h"

class UObject;
class UPhysicalMaterial;
struct FGameplayTag;
struct FGameplayTagContainer;

/** Base interface for anything acting as a ability calculation source */
UINTERFACE()
class ULyraAbilitySourceInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ILyraAbilitySourceInterface
{
	GENERATED_IINTERFACE_BODY()

	/**
	 * Compute the multiplier for effect falloff with distance
	 * 
	 * @param Distance			Distance from source to target for ability calculations (distance bullet traveled for a gun, etc...)
	 * @param SourceTags		Aggregated Tags from the source
	 * @param TargetTags		Aggregated Tags currently on the target
	 * 
	 * @return Multiplier to apply to the base attribute value due to distance
	 */
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;

	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const = 0;

	/**
	 * 거리 구간과 피격 부위로 데미지 값을 직접 정하는 소스(부위별 데미지 테이블을 가진 무기)용.
	 *
	 * @param Distance		Origin에서 명중점까지 거리 (cm)
	 * @param HitZone		피격 부위 태그 (Gameplay.Zone.*). 비어 있으면 바디샷
	 *
	 * @return 데미지 값. 지원하지 않으면 음수를 반환하고, 호출자는 GetDistanceAttenuation × GetPhysicalMaterialAttenuation으로 계산한다.
	 */
	virtual float GetDamageForHitZone(float Distance, const FGameplayTag& HitZone, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const
	{
		return -1.0f;
	}
};
