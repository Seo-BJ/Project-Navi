// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentSelectionComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Agent/NaviAgentDefinition.h"
#include "Agent/NaviAgentInstance.h"

#include "GameplayTags.h"
#include "LyraGameplayTags.h"
#include "Character/LyraHealthComponent.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Net/UnrealNetwork.h"

UNaviAgentSelectionComponent::UNaviAgentSelectionComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}

void UNaviAgentSelectionComponent::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* OwningController = GetOwner<APlayerController>();
	if (ensure(OwningController))
	{
		OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	}
	
}
void UNaviAgentSelectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNaviAgentSelectionComponent, SelectedAgentDefinition);
}


void UNaviAgentSelectionComponent::ServerRequestAgentSelectionByTag_Implementation(const FGameplayTag& AgentTag)
{
	APlayerController* OwningController = GetOwner<APlayerController>();
	if (!ensure(OwningController) || !OwningController->HasAuthority())
	{
		return;
	}
	
	
	UNaviAgentDefinition* TargetAgentDef = nullptr;
	for (TSubclassOf<UNaviAgentDefinition> Definition : SelectableAgents)
	{
		if (Definition && Definition.GetDefaultObject()->AgentTag == AgentTag)
		{
			SelectedAgentDefinition = Definition;
		}
	}
	
	APawn* OldPawn = OwningController->GetPawn();
	if (OldPawn)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OldPawn);
		if (ASC)
		{
			FGameplayEventData Payload;
			Payload.Instigator = OwningController;
			Payload.Target = OldPawn;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OldPawn, LyraGameplayTags::GameplayEvent_RequestReset, Payload);
		}
	}
	
}


void UNaviAgentSelectionComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	ApplyAgentToPawn(NewPawn);
}

void UNaviAgentSelectionComponent::ApplyAgentToPawn(APawn* TargetPawn)
{
	if (!TargetPawn || !SelectedAgentDefinition)
	{
		return;
	}

	// 실제 장비 장착은 서버에서만 수행
	if (GetOwner()->HasAuthority())
	{
		if (ULyraEquipmentManagerComponent* EquipmentManager = TargetPawn->FindComponentByClass<ULyraEquipmentManagerComponent>())
		{
			//EquipmentManager->UnequipItem(); 
			EquipmentManager->EquipItem(SelectedAgentDefinition);
		}
	}
}
