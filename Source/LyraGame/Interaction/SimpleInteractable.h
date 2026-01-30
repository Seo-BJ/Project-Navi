// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SimpleInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class USimpleInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that support simple, direct interaction without complex GAS logic.
 * (e.g., Doors, Lights, Simple Switches)
 */
class LYRAGAME_API ISimpleInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Called when a "Simple Interact" ability triggers on this actor.
	 * 
	 * @param InstigatorActor The actor who initiated the interaction (usually the Player Character).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SimpleInteract(AActor* InstigatorActor);
};
