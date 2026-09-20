// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "PhysicalMaterialWithTags.generated.h"

class UObject;

/**
 * ULyraWeaponInstance
 *
 * A piece of equipment representing a weapon spawned and applied to a pawn
 */
UCLASS()
class LYRAGAME_API UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// A container of gameplay tags that game code can use to reason about this physical material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;

	/**
	 * 물리 재질의 피격 부위 태그(Gameplay.Zone.*)를 찾는다. 없으면 빈 태그(바디샷).
	 * 서버 판정, 데미지 계산, 히트 마커가 모두 이 함수 하나로 부위를 정한다.
	 */
	static FGameplayTag FindZoneTag(const UPhysicalMaterial* PhysicalMaterial);
};
