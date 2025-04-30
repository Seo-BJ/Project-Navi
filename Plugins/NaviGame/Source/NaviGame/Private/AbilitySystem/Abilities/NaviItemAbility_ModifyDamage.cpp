// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilitysystem/Abilities/NaviItemAbility_ModifyDamage.h"
#include "NaviGame/NaviGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UNaviItemAbility_ModifyDamage::UNaviItemAbility_ModifyDamage()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // Needed for event listening
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; // Damage mods usually server-side
    // This ability is typically passive, triggered by an event, not player input
}

void UNaviItemAbility_ModifyDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    // Listen for the BeforeDamage event
    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Bind to the Generic Event delegate
    // EventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(NaviGameplayTags::Event_Damage_BeforeDamage).AddUObject(this, &UNaviItemAbility_ModifyDamage::HandleBeforeDamageEvent);
}

void UNaviItemAbility_ModifyDamage::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    // Cleanup delegate handle
    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
     if (ASC)
     {
        ASC->GenericGameplayEventCallbacks.FindOrAdd(NaviGameplayTags::Event_Damage_BeforeDamage).Remove(EventHandle);
     }
    Super::OnRemoveAbility(ActorInfo, Spec);
}


void UNaviItemAbility_ModifyDamage::HandleBeforeDamageEvent(const FGameplayEventData* Payload)
{
    // Extract custom payload
    if (Payload->OptionalObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("UNaviItemAbility_ModifyDamage::HandleBeforeDamageEvent"));
    }
}