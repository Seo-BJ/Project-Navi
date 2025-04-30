// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NaviAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NaviHealthSet.generated.h"

/**
 * 
 */
UCLASS()
class NAVIGAME_API UNaviHealthSet : public UNaviAttributeSet
{
	GENERATED_BODY()

public:
	
	UNaviHealthSet();
	
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, Shield);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, MaxShield);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, Armor);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, MaxArmor);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, IncomingDamageToHealth);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, IncomingDamageToArmor);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, IncomingDamageToShield);
	ATTRIBUTE_ACCESSORS(UNaviHealthSet, InComingHealing);

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FLyraAttributeEvent OnHealthChanged;

	// Delegate when max health changes
	mutable FLyraAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FLyraAttributeEvent OnOutOfHealth;

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FLyraAttributeEvent OnArmorChanged;

	// Delegate when max health changes
	mutable FLyraAttributeEvent OnMaxArmorChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FLyraAttributeEvent OnOutOfArmor;

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FLyraAttributeEvent OnShieldChanged;

	// Delegate when max health changes
	mutable FLyraAttributeEvent OnMaxShieldChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FLyraAttributeEvent OnOutOfShield;

protected:
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Shield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxArmor(const FGameplayAttributeData& OldValue);
	
	void BroadcastDamageMessage(const FGameplayEffectModCallbackData& Data);
	
	// --- Replication ---
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	// Helper function to clamp values like Health, Shield
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	
private:
	
	// Current health
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_Health, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;
	
	// Max health
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_MaxHealth , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
	
	// Current shield
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_Shield , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Shield;
	
	// Max shield
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_MaxShield , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxShield;

	// Current shield
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_Shield , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Armor;
	
	// Max shield
	UPROPERTY(BlueprintReadOnly, Category = "Navi|Health", ReplicatedUsing = OnRep_MaxShield , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxArmor;


	// Used to track when the health reaches 0.
	bool bOutOfHealth;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;

	bool bOutOfArmor;

	// Store the health before any changes 
	float MaxArmorBeforeAttributeChange;
	float ArmorBeforeAttributeChange;


	bool bOutOfShield;

	// Store the health before any changes 
	float MaxShieldBeforeAttributeChange;
	float ShieldBeforeAttributeChange;



	
	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData InComingHealing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData IncomingDamageToHealth;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData IncomingDamageToArmor;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Lyra|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData IncomingDamageToShield;
};
