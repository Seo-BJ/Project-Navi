// Fill out your copyright notice in the Description page of Project Settings.


#include "CredsSystem/NaviCredsSet.h"

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
	: Creds(9000.0f)
	, MaxCreds(9000.0f)
{
	CredsBeforeAttributeChange = 0.0f;
}

void UNaviCredsSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCredsSet, Creds, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNaviCredsSet, MaxCreds, COND_None, REPNOTIFY_Always);
}

void UNaviCredsSet::OnRep_Creds(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviCredsSet, Creds, OldValue);

	// 클라이언트에서 델리게이트를 호출하여 UI 등을 업데이트합니다.
	OnCredsChanged.Broadcast(nullptr, nullptr, nullptr, GetCreds() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetCreds());
}

void UNaviCredsSet::OnRep_MaxCreds(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UNaviCredsSet, MaxCreds, OldValue);

	// 클라이언트에서 델리게이트를 호출합니다.
	OnMaxCredsChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxCreds() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxCreds());
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

	/*
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	*/
	CredsBeforeAttributeChange = GetCreds();
    
	// 크레드를 직접 수정하는 대신, 메타 속성을 통해 처리합니다.
	if (Data.EvaluatedData.Attribute == GetInComingCredsGainAttribute())
	{
		// 획득한 크레드를 실제 크레드에 더하고, 메타 속성은 0으로 초기화합니다.
		SetCreds(FMath::Clamp(GetCreds() + GetInComingCredsGain(), 0.0f, GetMaxCreds()));
		SetInComingCredsGain(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingCredsCostAttribute())
	{
		// 소모한 크레드를 실제 크레드에서 차감하고, 메타 속성은 0으로 초기화합니다.
		SetCreds(FMath::Clamp(GetCreds() - GetIncomingCredsCost(), 0.0f, GetMaxCreds()));
		SetIncomingCredsCost(0.0f);
	}
	// Instant GE로 Creds 속성을 직접 변경하는 경우를 처리합니다.
	else if (Data.EvaluatedData.Attribute == GetCredsAttribute())
	{
		SetCreds(FMath::Clamp(GetCreds(), 0.0f, GetMaxCreds()));
	}

	// 크레드 값이 실제로 변경되었는지 확인하고 델리게이트를 호출합니다.
	if (GetCreds() != CredsBeforeAttributeChange)
	{
		const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
		AActor* Instigator = EffectContext.GetOriginalInstigator();
		AActor* Causer = EffectContext.GetEffectCauser();
        
		OnCredsChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, CredsBeforeAttributeChange, GetCreds());
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

	// 최대 크레드 값이 변경되면, 현재 크레드 값이 새로운 최대값을 넘지 않도록 조정합니다.
	if (Attribute == GetMaxCredsAttribute())
	{
		if (GetCreds() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);
			LyraASC->ApplyModToAttribute(GetCredsAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UNaviCredsSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCredsAttribute())
	{
		// 크레드 값은 0과 최대 크레드 값 사이로 제한합니다.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxCreds());
	}
	else if (Attribute == GetMaxCredsAttribute())
	{
		// 최대 크레드 값은 최소 1 이상이어야 합니다.
		NewValue = FMath::Max(NewValue, 1.0f);
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
