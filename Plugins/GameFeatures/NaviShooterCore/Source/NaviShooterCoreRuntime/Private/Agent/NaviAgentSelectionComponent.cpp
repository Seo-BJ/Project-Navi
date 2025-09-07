// Fill out your copyright notice in the Description page of Project Settings.


#include "Agent/NaviAgentSelectionComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Agent/NaviAgentDefinition.h"
#include "Agent/NaviAgentInstance.h"
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


void UNaviAgentSelectionComponent::ServerRespawnWithNewAgent_Implementation(const FGameplayTag& AgentTag)
{
	APlayerController* OwningController = GetOwner<APlayerController>();
	if (!ensure(OwningController) || !OwningController->HasAuthority() || AgentTag == FGameplayTag::EmptyTag)
	{
		return;
	}
	
	TSubclassOf<UNaviAgentDefinition>* Definition = SelectableAgentMap.Find(AgentTag);
	if (Definition)
	{
		SelectedAgentDefinition = *Definition;
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


void UNaviAgentSelectionComponent::ServerApplyAgentSelection_Implementation(const FGameplayTag& AgentTag)
{
	AController* OwnerController = GetOwner<AController>();
	if (OwnerController && OwnerController->HasAuthority())
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			if (ULyraEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<ULyraEquipmentManagerComponent>())
			{
				TSubclassOf<UNaviAgentDefinition>* Definition = SelectableAgentMap.Find(AgentTag);
				if (Definition)
				{
					EquipmentManager->EquipItem(*Definition);
				}
			}
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
	
	if (GetOwner()->HasAuthority())
	{
		if (ULyraEquipmentManagerComponent* EquipmentManager = TargetPawn->FindComponentByClass<ULyraEquipmentManagerComponent>())
		{
			//EquipmentManager->UnequipItem(); 
			EquipmentManager->EquipItem(SelectedAgentDefinition);
		}
	}
}
