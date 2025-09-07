// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Phases/LyraGamePhaseAbility.h"
#include "NaviPostRoundPhaseAbility.generated.h"

/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviPostRoundPhaseAbility : public ULyraGamePhaseAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
