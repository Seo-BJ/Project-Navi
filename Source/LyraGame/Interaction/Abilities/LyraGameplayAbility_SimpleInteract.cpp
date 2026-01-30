// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Abilities/LyraGameplayAbility_SimpleInteract.h"
#include "Interaction/SimpleInteractable.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_SimpleInteract)

ULyraGameplayAbility_SimpleInteract::ULyraGameplayAbility_SimpleInteract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// This ability is typically granted temporarily or triggered via event, 
	// but for Interaction options, it might be granted to the player.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void ULyraGameplayAbility_SimpleInteract::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// The Target is usually passed in the Event Data by the Lyra Interaction System
	// (See ULyraGameplayAbility_Interact::TriggerInteraction)
	const AActor* TargetActor = nullptr;

	if (TriggerEventData && TriggerEventData->Target)
	{
		TargetActor = const_cast<AActor*>(ToRawPtr(TriggerEventData->Target));
	}
	
	// If the target is valid and implements the SimpleInteractable interface
	if (TargetActor && TargetActor->Implements<USimpleInteractable>())
	{
		// Instigator is the avatar (player)
		AActor* InstigatorActor = GetAvatarActorFromActorInfo();
		
		// Execute the interface call
		ISimpleInteractable::Execute_SimpleInteract(const_cast<AActor*>(TargetActor), InstigatorActor);
	}
	else
	{
		// Fallback or Error logging could go here if needed
		// UE_LOG(LogLyra, Warning, TEXT("SimpleInteract Ability activated but Target is missing or does not implement ISimpleInteractable."));
	}

	// Simple interactions are instant, so we end the ability immediately.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
