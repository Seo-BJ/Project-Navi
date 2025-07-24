// Copyright Epic Games, Inc. All Rights Reserved.

#include "CredsSystem/NaviCredsShopComponent.h"

#include "LyraGameplayTags.h"
#include "Player/LyraPlayerState.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Inventory/LyraInventoryItemDefinition.h"

#include "Equipment/LyraPickupDefinition.h"
#include "Equipment/NaviQuickBarComponent.h"
#include "Equipment/Armor/NaviArmorStatDefinition.h"

#include "CredsSystem/NaviCredsSet.h"
#include "Equipment/Weapons/NaviWeaponStatDefinition.h"
#include "Equipment/Armor/NaviArmorStatDefinition.h"

#include "Engine/DataTable.h"
#include "NativeGameplayTags.h" 

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"


UNaviCredsShopComponent::UNaviCredsShopComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UNaviCredsShopComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNaviCredsShopComponent::BuyEquipment(AController* RequestingPlayerController, FGameplayTag EquipmentTag)
{
    if (!ensure(GetOwner()->HasAuthority()) || !RequestingPlayerController || !EquipmentTag.IsValid())
    {
        return;
    }
    
    ALyraPlayerState* PlayerState = RequestingPlayerController->GetPlayerState<ALyraPlayerState>();
    ULyraAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetLyraAbilitySystemComponent() : nullptr;
    if (!PlayerState || !ASC)
    {
        return;
    }
    
    const int32 Cost = GetEquipmentCost(EquipmentTag);
    if (Cost == -1)
    {
        return;
    }
    
    // 크레드 속성 셋 가져오기
    const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>();
    if (!CredsSet)
    {
        return;
    }
    
    if (CredsSet->GetCreds() >= Cost)
    {
        UGameplayEffect* CostEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("CredsCostEffect"));
        CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
        
        FGameplayModifierInfo ModifierInfo;
        ModifierInfo.Attribute = UNaviCredsSet::GetCredsAttribute();
        ModifierInfo.ModifierOp = EGameplayModOp::Additive;
        ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Cost); 
        CostEffect->Modifiers.Add(ModifierInfo);

        ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, ASC->MakeEffectContext());

        if (EquipmentTag.MatchesTag(NaviGameplayTags::Weapon))
        {
            ULyraWeaponPickupDefinition* WeaponPickupDefinition = WeaponDefinitionMap[EquipmentTag];
            TSubclassOf<ULyraInventoryItemDefinition> WeaponItemDefinition = WeaponPickupDefinition ? WeaponPickupDefinition->InventoryItemDefinition : nullptr;
            if (WeaponItemDefinition != nullptr)
            {
                GiveWeaponToPlayer(WeaponItemDefinition, RequestingPlayerController);
            }
        }
        else if (EquipmentTag.MatchesTag(NaviGameplayTags::Armor))
        {
            
            // ArmorStatTable에서 EquipmentTag에 해당하는 방어구 스탯 정보를 찾습니다.
            // 데이터 테이블의 RowName이 태그의 TagName과 일치한다고 가정합니다.
            const FName RowName = EquipmentTag.GetTagName();
            const FNaviArmorStatDefinition* ArmorStat = ArmorStatTable ? ArmorStatTable->FindRow<FNaviArmorStatDefinition>(RowName, TEXT("Find Armor Stat from DataTable")) : nullptr;

            if (ArmorStat)
            {
                // Armor 속성을 변경할 GameplayEffect를 생성합니다.
                UGameplayEffect* ArmorEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("ArmorPurchaseEffect"));
                ArmorEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

                FGameplayModifierInfo ArmorModifierInfo;
                ArmorModifierInfo.Attribute = ULyraHealthSet::GetArmorAttribute();
                // AbsorbAmount 값으로 Armor 속성을 덮어씁니다.
                ArmorModifierInfo.ModifierOp = EGameplayModOp::Override;
                ArmorModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(ArmorStat->AbsorbAmount); 
                ArmorEffect->Modifiers.Add(ArmorModifierInfo);

                // 플레이어의 ASC에 GameplayEffect를 적용하여 Armor 속성을 즉시 업데이트합니다.
                ASC->ApplyGameplayEffectToSelf(ArmorEffect, 1.0f, ASC->MakeEffectContext());
            }
        }
    }
    else
    {
        
    }
}

void UNaviCredsShopComponent::GiveWeaponToPlayer(TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass, AController* ReceivingController)
{
    if (WeaponItemClass == nullptr || ReceivingController == nullptr) return;

    UNaviQuickBarComponent* NaviQuickBarComponent = ReceivingController->GetComponentByClass<UNaviQuickBarComponent>();
    ULyraInventoryManagerComponent* LyraInventoryManagerComponent = ReceivingController->GetComponentByClass<ULyraInventoryManagerComponent>();
    if (NaviQuickBarComponent == nullptr|| LyraInventoryManagerComponent == nullptr) return;
    
    const ULyraInventoryItemDefinition* ItemCDO = WeaponItemClass->GetDefaultObject<ULyraInventoryItemDefinition>();
    if (ItemCDO == nullptr) return;
    
    ULyraInventoryItemInstance* LyraInventoryItemDefinition = LyraInventoryManagerComponent->AddItemDefinition(WeaponItemClass);
    int AddedIndex = NaviQuickBarComponent->AddItemToSlot(ItemCDO->ItemTag, LyraInventoryItemDefinition);
    NaviQuickBarComponent->SetActiveSlotIndex(AddedIndex);
}

int32 UNaviCredsShopComponent::GetEquipmentCost(const FGameplayTag& EquipmentTag) const
{
    if (WeaponStatTable == nullptr || ArmorStatTable == nullptr) 
    {
        return -1;
    }

    if (EquipmentTag.MatchesTag(NaviGameplayTags::Weapon))
    {
        const FName RowName = EquipmentTag.GetTagName();
        const FNaviWeaponStatDefinition* Row = WeaponStatTable->FindRow<FNaviWeaponStatDefinition>(RowName, TEXT("FindWeaponCost"));
        return Row ? Row->CreditCost : -1;
    }
    if (EquipmentTag.MatchesTag(NaviGameplayTags::Armor))
    {
        const FName RowName = EquipmentTag.GetTagName();
        const FNaviArmorStatDefinition* Row = ArmorStatTable->FindRow<FNaviArmorStatDefinition>(RowName, TEXT("FindArmorCost"));
        return Row ? Row->CreditCost : -1;
    }
    
    return -1;
}
