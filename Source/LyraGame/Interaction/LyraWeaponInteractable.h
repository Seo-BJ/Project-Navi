// Copyright (c) 2024.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LyraWeaponInteractable.generated.h"

UINTERFACE(MinimalAPI)
class ULyraWeaponInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can be interacted with via weapon hits (e.g. shooting buttons).
 */
class LYRAGAME_API ILyraWeaponInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Called when a weapon hits this actor.
	 * @param HitResult The hit result from the weapon trace.
	 * @param Shooter The actor who shot the weapon.
	 */
	virtual void OnWeaponHit(const FHitResult& HitResult, AActor* Shooter) = 0;
};

