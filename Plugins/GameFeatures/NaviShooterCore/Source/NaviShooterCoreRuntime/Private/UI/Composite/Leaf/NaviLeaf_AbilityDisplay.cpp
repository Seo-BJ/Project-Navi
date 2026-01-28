// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_AbilityDisplay.h"
#include "CredsSystem/NaviCredsShopComponent.h"
#include "Equipment/LyraPickupDefinition.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "Equipment/LyraEquipmentDefinition.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "GameFramework/GameStateBase.h"

void UNaviLeaf_AbilityDisplay::UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData)
{
    Super::UpdateWeaponStats(WeaponData);

    UWorld* World = GetWorld();
    if (!World) return;

    AGameStateBase* GS = World->GetGameState();
    if (!GS) return;

    UNaviCredsShopComponent* ShopComp = GS->GetComponentByClass<UNaviCredsShopComponent>();
    if (!ShopComp) return;

    const ULyraWeaponPickupDefinition* PickupDef = ShopComp->GetWeaponPickupDefinition(WeaponData.WeaponTag);
    if (!PickupDef) return;

    TSubclassOf<ULyraInventoryItemDefinition> ItemDefClass = PickupDef->InventoryItemDefinition;
    if (!ItemDefClass) return;

    const ULyraInventoryItemDefinition* ItemDefCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefClass);
    if (!ItemDefCDO) return;

    const UInventoryFragment_EquippableItem* EquipFragment = Cast<UInventoryFragment_EquippableItem>(ItemDefCDO->FindFragmentByClass(UInventoryFragment_EquippableItem::StaticClass()));
    if (!EquipFragment) return;

    TSubclassOf<ULyraEquipmentDefinition> EquipDefClass = EquipFragment->EquipmentDefinition;
    if (!EquipDefClass) return;

    const ULyraEquipmentDefinition* EquipDefCDO = GetDefault<ULyraEquipmentDefinition>(EquipDefClass);
    if (!EquipDefCDO) return;

    FGameplayTag AltFireTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Weapon.AltFire")); 

    for (const TObjectPtr<const ULyraAbilitySet>& AbilitySet : EquipDefCDO->AbilitySetsToGrant)
    {
        if (!AbilitySet) continue;

        for (const FLyraAbilitySet_GameplayAbility& AbilityInfo : AbilitySet->GetGrantedGameplayAbilities())
        {
            if (AbilityInfo.InputTag == AltFireTag)
            {
                 OnAbilityFound(AbilityInfo.Ability);
                 return; // Found it
            }
        }
    }
}
