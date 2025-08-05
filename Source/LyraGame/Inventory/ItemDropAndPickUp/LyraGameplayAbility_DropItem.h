// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "LyraGameplayAbility_DropItem.generated.h"

class ALyraDropAndPickupable;

/**
 * 
 */
UCLASS()
class LYRAGAME_API ULyraGameplayAbility_DropItem : public ULyraGameplayAbility
{
	GENERATED_BODY()
public:
	
	ULyraGameplayAbility_DropItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	UFUNCTION(BlueprintCallable)
	void SpawnPickupableItemActor(TSubclassOf<ALyraDropAndPickupable> DroppedWeaponClass, ULyraInventoryItemInstance* ItemInstance);

	UPROPERTY(EditAnywhere)
	float LaunchSpeed = 450.0f;
};
