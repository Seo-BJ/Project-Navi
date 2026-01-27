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
#include "Weapons/NaviWeaponStatDefinition.h"
#include "Equipment/Armor/NaviArmorStatDefinition.h"

#include "Engine/DataTable.h"
#include "NativeGameplayTags.h" 

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Inventory/ItemDropAndPickUp/LyraDropAndPickupable.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NaviShooterCoreGameplayTags.h"

UNaviCredsShopComponent::UNaviCredsShopComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UNaviCredsShopComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 서버/클라이언트 모두에서 UI 표시를 위해 캐싱 필요 (특히 로컬 플레이어용)
    // 데이터 테이블은 정적이므로 한 번만 계산하면 됩니다.
    CacheStatRanges();
}

void UNaviCredsShopComponent::CacheStatRanges()
{
	if (!WeaponStatTable) return;

	StatMinMaxCache.Empty();

	const FString ContextString(TEXT("CacheStatRanges"));
	TArray<FNaviWeaponStatDefinition*> AllRows;
	WeaponStatTable->GetAllRows<FNaviWeaponStatDefinition>(ContextString, AllRows);

	if (AllRows.Num() == 0) return;

	// 헬퍼 람다: Min/Max 갱신
	auto UpdateMinMax = [&](FGameplayTag Tag, float Value)
	{
		if (!StatMinMaxCache.Contains(Tag))
		{
			StatMinMaxCache.Add(Tag, FVector2D(Value, Value));
		}
		else
		{
			FVector2D& Range = StatMinMaxCache[Tag];
			Range.X = FMath::Min(Range.X, Value);
			Range.Y = FMath::Max(Range.Y, Value);
		}
	};

	using namespace NaviShooterCoreGameplayTags;

	for (const FNaviWeaponStatDefinition* Row : AllRows)
	{
		if (!Row) continue;

		UpdateMinMax(Weapon_Stat_FireRate, Row->FireRate);
		UpdateMinMax(Weapon_Stat_RunSpeed, Row->RunSpeed);
		UpdateMinMax(Weapon_Stat_EquipSpeed, Row->EquipSpeed);
		UpdateMinMax(Weapon_Stat_ReloadSpeed, Row->ReloadSpeed);
		UpdateMinMax(Weapon_Stat_MagazineSize, (float)Row->MagazineSize);
		UpdateMinMax(Weapon_Stat_MaxAmmo, (float)Row->MaxAmmo);
		UpdateMinMax(Weapon_Stat_FirstShotSpread, Row->FirstShotSpread_HipFire); // Use Hip spread for cache/bar
		UpdateMinMax(Weapon_Stat_CreditCost, (float)Row->CreditCost);

		if (Row->DamageFalloffs.Num() > 0)
		{
			const FDamageFalloff& BaseDamage = Row->DamageFalloffs[0];
			UpdateMinMax(Weapon_Stat_Damage_Head, BaseDamage.HeadShotDamage);
			UpdateMinMax(Weapon_Stat_Damage_Body, BaseDamage.BodyShotDamage);
			UpdateMinMax(Weapon_Stat_Damage_Leg, BaseDamage.LegShotDamage);
		}
	}
}

bool UNaviCredsShopComponent::GetStatRange(FGameplayTag Tag, float& OutMin, float& OutMax) const
{
	if (const FVector2D* Range = StatMinMaxCache.Find(Tag))
	{
		OutMin = Range->X;
		OutMax = Range->Y;
		return true;
	}
	return false;
}

bool UNaviCredsShopComponent::TryBuyEquipment(AController* RequestingPlayerController, FGameplayTag TargetEquipmentTag)
{
    if (!ensure(GetOwner()->HasAuthority()) || !RequestingPlayerController || !TargetEquipmentTag.IsValid())
    {
        return false;
    }
    
    ALyraPlayerState* PlayerState = RequestingPlayerController->GetPlayerState<ALyraPlayerState>();
    ULyraAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetLyraAbilitySystemComponent() : nullptr;
    if (!PlayerState || !ASC)
    {
        return false;
    }
    
    const int32 Cost = GetEquipmentCost(TargetEquipmentTag);
    if (Cost == -1)
    {
        return false;
    }
    
    // 크레드 속성 셋 가져오기
    const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>();
    if (!CredsSet)
    {
        return false;
    }
    
    if (TargetEquipmentTag.MatchesTag(NaviGameplayTags::Weapon))
    {
        UNaviQuickBarComponent* QuickBarComponent = RequestingPlayerController->FindComponentByClass<UNaviQuickBarComponent>();
        ULyraInventoryManagerComponent* LyraInventoryManagerComponent =RequestingPlayerController->FindComponentByClass<ULyraInventoryManagerComponent>();
        if (QuickBarComponent == nullptr || LyraInventoryManagerComponent == nullptr) return false;
        
        ULyraWeaponPickupDefinition* WeaponPickupDefinition = WeaponDefinitionMap[TargetEquipmentTag];
        TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition = WeaponPickupDefinition ? WeaponPickupDefinition->InventoryItemDefinition : nullptr;
        if (ItemDefinition == nullptr) return false;
        
        int32 EquipmentSlotIndex = QuickBarComponent->GetSlotIndexForItemTag(TargetEquipmentTag);
        ULyraInventoryItemInstance* ItemInstanceInSlot = QuickBarComponent->GetItemInSlot(EquipmentSlotIndex);
        ULyraInventoryItemDefinition* ItemDefinitionInSlot = ItemInstanceInSlot ? ItemInstanceInSlot->GetItemDef()->GetDefaultObject<ULyraInventoryItemDefinition>() : nullptr;

        UGameplayEffect* CostEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("CredsCostEffect"));
        CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
        FGameplayModifierInfo ModifierInfo;
        ModifierInfo.Attribute = UNaviCredsSet::GetCredsAttribute();
        ModifierInfo.ModifierOp = EGameplayModOp::Additive;
        
        if (ItemInstanceInSlot == nullptr) // 해당 Slot에 Item이 없는 경우
        {
            if (CredsSet->GetCreds() >= Cost)
            {
                ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Cost); 
                CostEffect->Modifiers.Add(ModifierInfo);
                ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, ASC->MakeEffectContext());
            }
        }
        else
        {
            if (ItemInstanceInSlot->HasStatTag(LyraGameplayTags::Lyra_Item_Dropped))
            {
                if (CredsSet->GetCreds() >= Cost)
                {
                    ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Cost); 
                    CostEffect->Modifiers.Add(ModifierInfo);
                    ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, ASC->MakeEffectContext());
                    
                    QuickBarComponent->SpawnAndDropEquipment(DroppedWeaponClass);
                }
            }
            else
            {
                // 무기 환불
                if (CredsSet->GetCreds() + GetEquipmentCost(ItemDefinitionInSlot->ItemTag) >= Cost)
                {
                    QuickBarComponent->RemoveItemFromSlot(EquipmentSlotIndex);
                    LyraInventoryManagerComponent->RemoveItemInstance(ItemInstanceInSlot);
                    
                    ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-(Cost - GetEquipmentCost(ItemDefinitionInSlot->ItemTag))); 
                    CostEffect->Modifiers.Add(ModifierInfo);
                    ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, ASC->MakeEffectContext());
                }
            }
        }
        GiveWeaponToPlayer(ItemDefinition, RequestingPlayerController);

    }
    else if (TargetEquipmentTag.MatchesTag(NaviGameplayTags::Armor))
    {
        
        // ArmorStatTable에서 EquipmentTag에 해당하는 방어구 스탯 정보를 찾습니다.
        // 데이터 테이블의 RowName이 태그의 TagName과 일치한다고 가정합니다.
        const FName RowName = TargetEquipmentTag.GetTagName();
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
    return true;
}

void UNaviCredsShopComponent::GiveWeaponToPlayer(TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass, AController* ReceivingController)
{
    if (WeaponItemClass == nullptr || ReceivingController == nullptr) return;

    UNaviQuickBarComponent* QuickBar = ReceivingController->GetComponentByClass<UNaviQuickBarComponent>();
    ULyraInventoryManagerComponent* LyraInventoryManagerComponent = ReceivingController->GetComponentByClass<ULyraInventoryManagerComponent>();
    if (QuickBar == nullptr|| LyraInventoryManagerComponent == nullptr) return;
    
    ULyraInventoryItemInstance* LyraInventoryItemDefinition = LyraInventoryManagerComponent->AddItemDefinition(WeaponItemClass);
    int AddedIndex = QuickBar->AddItemToSlot(LyraInventoryItemDefinition);
    QuickBar->SetActiveSlotIndex(QuickBar->GetActiveSlotIndexOnEquipmentBuy(AddedIndex));
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

const FNaviWeaponStatDefinition* UNaviCredsShopComponent::GetWeaponStatRow(FGameplayTag Tag) const
{
	if (!WeaponStatTable || !Tag.IsValid())
	{
		return nullptr;
	}

	const FName RowName = Tag.GetTagName();
	return WeaponStatTable->FindRow<FNaviWeaponStatDefinition>(RowName, TEXT("GetWeaponStatRow"));
}

const FNaviArmorStatDefinition* UNaviCredsShopComponent::GetArmorStatRow(FGameplayTag Tag) const
{
	// Assuming Armor uses FNaviArmorStatDefinition as seen in other functions
	if (!ArmorStatTable || !Tag.IsValid())
	{
		return nullptr;
	}

	const FName RowName = Tag.GetTagName();
	return ArmorStatTable->FindRow<FNaviArmorStatDefinition>(RowName, TEXT("GetArmorStatRow"));
}
