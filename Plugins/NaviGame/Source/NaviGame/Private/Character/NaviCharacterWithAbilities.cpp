// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NaviCharacterWithAbilities.h"

#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraCombatSet.h"
#include "Async/TaskGraphInterfaces.h"

// Sets default values
ANaviCharacterWithAbilities::ANaviCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<ULyraAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<ULyraHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<ULyraCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void ANaviCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ANaviCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

