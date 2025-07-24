// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"

#include "NaviCredsSet.generated.h"

/**
 * 
 */
UCLASS()
class UNaviCredsSet : public ULyraAttributeSet
{
	GENERATED_BODY()

public:
	UNaviCredsSet();

	ATTRIBUTE_ACCESSORS(UNaviCredsSet, Creds);
	ATTRIBUTE_ACCESSORS(UNaviCredsSet, MaxCreds);
	ATTRIBUTE_ACCESSORS(UNaviCredsSet, InComingCredsGain);
	ATTRIBUTE_ACCESSORS(UNaviCredsSet, IncomingCredsCost);
	
	// 크레드 값이 변경될 때 호출되는 델리게이트입니다. (클라이언트 예측 정보가 없을 수 있음)
	mutable FLyraAttributeEvent OnCredsChanged;

	// 최대 크레드 값이 변경될 때 호출되는 델리게이트입니다.
	mutable FLyraAttributeEvent OnMaxCredsChanged;

protected:

	UFUNCTION()
	void OnRep_Creds(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxCreds(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Creds, Category =  "Navi|creds", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Creds;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxCreds, Category = "Navi|creds", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxCreds;

	float CredsBeforeAttributeChange;

	void BroadcastCredsChangeMessage(const FGameplayEffectModCallbackData& Data);

	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------
	
	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category = "Navi|creds", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData InComingCredsGain;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category = "Navi|creds", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData IncomingCredsCost;
};






