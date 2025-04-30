// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NaviCharacter.h"
#include "NaviCharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class UObject;

// ALyraCharacter typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
UCLASS(Blueprintable)
class NAVIGAME_API ANaviCharacterWithAbilities : public ANaviCharacter
{
	GENERATED_BODY()

public:
	ANaviCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Navi|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
	
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraHealthSet> HealthSet;
	// Combat attribute set used by this actor.
	
	UPROPERTY()
	TObjectPtr<const class ULyraCombatSet> CombatSet;
};
