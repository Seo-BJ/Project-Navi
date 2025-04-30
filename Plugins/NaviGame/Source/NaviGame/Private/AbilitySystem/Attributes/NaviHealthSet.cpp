// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/NaviHealthSet.h"

#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "LyraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "Engine/World.h"
#include "Messages/LyraVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UNaviHealthSet::UNaviHealthSet() :
     Health(100.0f)
    , MaxHealth(100.0f)
    , Shield(0.0f)
    , MaxShield(0.0f)
    , Armor(0.0f)
    , MaxArmor(0.0f)
{
}

void UNaviHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, Shield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, MaxShield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, Armor, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNaviHealthSet, MaxArmor, COND_None, REPNOTIFY_Always);
}


bool UNaviHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }

    // Handle modifying incoming normal damage
    bool bDoesDataHasDamageAttribute = (Data.EvaluatedData.Attribute == GetIncomingDamageToHealthAttribute())
    || (Data.EvaluatedData.Attribute == GetIncomingDamageToArmorAttribute())
    || (Data.EvaluatedData.Attribute == GetIncomingDamageToShieldAttribute());
    
    if (bDoesDataHasDamageAttribute)
    {
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);

            if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }

#if !UE_BUILD_SHIPPING
            // Check GodMode cheat, unlimited health is checked below
            if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }
#endif // #if !UE_BUILD_SHIPPING
        }
    }

    // Save the current health
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();

    return true;
}


void UNaviHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

	const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
	float MinimumHealth = 0.0f;
	float MinimumArmor = 0.0f;
	float MinimumShield = 0.0f;
	
#if !UE_BUILD_SHIPPING
	// Godmode and unlimited health stop death unless it's a self destruct
	if (!bIsDamageFromSelfDestruct &&
		(Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_UnlimitedHealth) ))
	{
		MinimumHealth = 1.0f;
	}
#endif // #if !UE_BUILD_SHIPPING

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetIncomingDamageToHealthAttribute())
	{
		BroadcastDamageMessage(Data);
		// Convert into -Health and then clamp
		SetHealth(FMath::Clamp(GetHealth() - GetIncomingDamageToHealth(), MinimumHealth, GetMaxHealth()));
		SetIncomingDamageToHealth(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingDamageToArmorAttribute())
	{
		BroadcastDamageMessage(Data);
		SetArmor(FMath::Clamp(GetArmor() - GetIncomingDamageToArmor(), MinimumArmor, GetMaxArmor()));
		SetIncomingDamageToArmor(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingDamageToShieldAttribute())
	{
		BroadcastDamageMessage(Data);
		SetShield(FMath::Clamp(GetShield() - GetIncomingDamageToShield(), MinimumShield, GetMaxShield()));
		SetIncomingDamageToShield(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetInComingHealingAttribute())
	{
		// Convert into +Health and then clamp
		SetHealth(FMath::Clamp(GetHealth() + GetInComingHealing(), MinimumHealth, GetMaxHealth()));
		SetInComingHealing(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp and fall into out of health handling below
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// TODO clamp current health?

		// Notify on any requested max health changes
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	// If health has actually changed activate callbacks
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	// Check health again in case an event above changed it.
	bOutOfHealth = (GetHealth() <= 0.0f);

}


void UNaviHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}
void UNaviHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}
void UNaviHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetHealth() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}

	if (Attribute == GetMaxArmorAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetArmor() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetArmorAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfArmor && (GetArmor() > 0.0f))
	{
		bOutOfArmor = false;
	}

	if (Attribute == GetMaxShieldAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetShield() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetShieldAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfShield && (GetShield() > 0.0f))
	{
		bOutOfShield = false;
	}
}
void UNaviHealthSet::BroadcastDamageMessage(const FGameplayEffectModCallbackData& Data)
{
	// Send a standardized verb message that other systems can observe
	if (Data.EvaluatedData.Magnitude > 0.0f)
	{
		FLyraVerbMessage Message;
		Message.Verb = TAG_Lyra_Damage_Message;
		Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
		Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
		Message.Target = GetOwningActor();
		Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
		//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
		//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
		Message.Magnitude = Data.EvaluatedData.Magnitude;

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(Message.Verb, Message);
	}
}

void UNaviHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetArmorAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxArmor()); 
	}
	else if (Attribute == GetMaxArmorAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); 
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); 
	}
}

void UNaviHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, Health, OldValue);

    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

    if (!bOutOfHealth && CurrentHealth <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
    }

    bOutOfHealth = (CurrentHealth <= 0.0f);
    
}
void UNaviHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, MaxHealth, OldValue);
    
    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
}

void UNaviHealthSet::OnRep_Armor(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, Armor, OldValue);
    const float CurrentArmor = GetArmor();
    const float EstimatedMagnitude = CurrentArmor - OldValue.GetCurrentValue();
	
    OnArmorChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentArmor);

    if (!bOutOfArmor && CurrentArmor <= 0.0f)
    {
        OnOutOfArmor.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentArmor);
    }

    bOutOfArmor = (CurrentArmor <= 0.0f);
}
void UNaviHealthSet::OnRep_MaxArmor(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, MaxArmor, OldValue);
    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxArmor() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxArmor());
}

void UNaviHealthSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, Shield, OldValue);
    const float CurrentSheild = GetShield();
    const float EstimatedMagnitude = CurrentSheild - OldValue.GetCurrentValue();
	
    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentSheild);

    if (!bOutOfHealth && CurrentSheild <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentSheild);
    }

    bOutOfHealth = (CurrentSheild <= 0.0f);
}
void UNaviHealthSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviHealthSet, MaxShield, OldValue);
    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxShield() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxShield());
}

