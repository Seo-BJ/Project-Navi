// Fill out your copyright notice in the Description page of Project Settings.

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/NaviQuickBarComponent.h"

#include "Inventory/InventoryFragment_EquippableItem.h"

#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Equipment/LyraEquipmentDefinition.h"
#include "Equipment/LyraEquipmentInstance.h"
#include "LyraGameplayTags.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"



// GameplayMessageSystem에서 사용할 고유 태그를 정의합니다.
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_QuickBar_Message_SlotsChanged, "Navi.QuickBar.Message.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_QuickBar_Message_ActiveIndexChanged, "Navi.QuickBar.Message.ActiveIndexChanged");


UNaviQuickBarComponent::UNaviQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	NumSlots = 4; // 슬롯 개수를 4로 고정
	ActiveSlotIndex = -1;
}

void UNaviQuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void UNaviQuickBarComponent::BeginPlay()
{
	// 슬롯 배열의 크기를 NumSlots에 맞게 초기화합니다.
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}
	
	Super::BeginPlay();
}

void UNaviQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (GetActiveSlotIndex() < 0 ? Slots.Num() - 1 : GetActiveSlotIndex());
	int32 NewIndex = OldIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

void UNaviQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (GetActiveSlotIndex() < 0 ? Slots.Num() - 1 : GetActiveSlotIndex());
	int32 NewIndex = OldIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

void UNaviQuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex) && (Slots[NewIndex] != nullptr))
	{
		UnequipItemInSlot();
		ActiveSlotIndex = NewIndex;
		EquipItemInSlot();

		// 클라이언트에서도 RepNotify가 즉시 호출되도록 합니다.
		OnRep_ActiveSlotIndex();
	}
}

int UNaviQuickBarComponent::AddItemToSlot(ULyraInventoryItemInstance* Item)
{
	if (GetOwner()->HasAuthority() && Item != nullptr)
	{
		if (ULyraInventoryItemDefinition* ItemDef = Item->GetItemDef()->GetDefaultObject<ULyraInventoryItemDefinition>())
		{
			int SlotIndex = GetSlotIndexForItemTag(ItemDef->ItemTag);
			if (Slots[SlotIndex] == nullptr)
			{
				Slots[SlotIndex] = Item;
				OnRep_Slots();
				return SlotIndex;
			}
		}
	}
	return INDEX_NONE;
}


ULyraInventoryItemInstance* UNaviQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	ULyraInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
		OnRep_ActiveSlotIndex();
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];
		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}

int UNaviQuickBarComponent::GetSlotIndexForItemTag(const FGameplayTag& ItemTag) const
{
	if (ItemTag.MatchesTag(NaviGameplayTags::Weapon_Primary))
	{
		return 0;
	}
	if (ItemTag.MatchesTag(NaviGameplayTags::Weapon_Sidearms))
	{
		return 1;
	}
	if (ItemTag.MatchesTag(NaviGameplayTags::Weapon_Melee))
	{
		return 2;
	}
	if (ItemTag.MatchesTag(NaviGameplayTags::Item_Spike))
	{
		return 3;
	}
	return INDEX_NONE;
}


ULyraInventoryItemInstance* UNaviQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}


void UNaviQuickBarComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (ULyraInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<ULyraEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (ULyraEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}
	}
}

void UNaviQuickBarComponent::UnequipItemInSlot()
{
	if (ULyraEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem != nullptr)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

ULyraEquipmentManagerComponent* UNaviQuickBarComponent::FindEquipmentManager() const
{
	if (AController* OwnerController = GetOwner<AController>())
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<ULyraEquipmentManagerComponent>();
		}
	}
	return nullptr;
}

void UNaviQuickBarComponent::OnRep_Slots()
{
	FNaviQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Navi_QuickBar_Message_SlotsChanged, Message);
}

void UNaviQuickBarComponent::OnRep_ActiveSlotIndex()
{
	FNaviQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlotIndex;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Navi_QuickBar_Message_ActiveIndexChanged, Message);
}