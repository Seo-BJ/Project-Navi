// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviCredsSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "LyraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "Messages/LyraVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NaviCredsSet)

UNaviCredsSet::UNaviCredsSet()
	: Creds(0.0f)
	, MaxCreds(9000.0f)
{

}

void UNaviCredsSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCredsSet, Creds, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCredsSet, MaxCreds, COND_None, REPNOTIFY_Always);
}

void UNaviCredsSet::OnRep_Creds(const FGameplayAttributeData& OldValue)
{
}

void UNaviCredsSet::OnRep_MaxCreds(const FGameplayAttributeData& OldValue)
{
}

bool UNaviCredsSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}
	
	if (Data.EvaluatedData.Magnitude > 0.0f)
#if !UE_BUILD_SHIPPING
	// Check GodMode cheat, unlimited health is checked below
	if (Data.Target.HasMatchingGameplayTag(NaviGameplayTags::Navi_Cheat_InfiniteCreds))
	{
		Data.EvaluatedData.Magnitude = 0.0f;
	}
#endif // #if !UE_BUILD_SHIPPING
	return true;
}

void UNaviCredsSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	
	if (Data.EvaluatedData.Attribute == GetCredsAttribute())
	{
		// Clamp and fall into out of health handling below
		BroadcastCredsChangeMessage(Data);
		SetCreds(FMath::Clamp(GetCreds() - Data.EvaluatedData.Magnitude, 0, GetMaxCreds()));
	}
	

}

void UNaviCredsSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UNaviCredsSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UNaviCredsSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UNaviCredsSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCredsAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxCreds());
	}
}
void UNaviCredsSet::BroadcastCredsChangeMessage(const FGameplayEffectModCallbackData& Data)
{
	// Send a standardized verb message that other systems can observe
	if (Data.EvaluatedData.Magnitude > 0.0f)
	{
		FLyraVerbMessage Message;
		// Message.Verb = TAG_Lyra_Damage_Message;
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
	else if (Data.EvaluatedData.Magnitude > 0.0f)
	{
		
	}
}
