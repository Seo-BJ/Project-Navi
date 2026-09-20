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
#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h"
#include "GameplayEffect.h"

// 구매 페이즈 (Phase_CM_Buying). 하위 태그 Start/CountDown/End도 함께 매칭된다.
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Navi_Phase_Buying, "Navi.Phase.Buying");

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

namespace NaviCredsShop
{
	/** 크레드 변화량(음수 = 차감, 양수 = 환불)을 즉시 적용한다. 0이면 아무것도 하지 않는다. */
	void ApplyCredsDelta(ULyraAbilitySystemComponent* ASC, float CredsDelta)
	{
		if (ASC == nullptr || FMath::IsNearlyZero(CredsDelta))
		{
			return;
		}

		// 이름을 고정하면(예: "CredsCostEffect") 이전 객체가 GC되기 전에 같은 이름으로 덮어쓸 수 있으므로 이름을 지정하지 않는다.
		UGameplayEffect* CredsEffect = NewObject<UGameplayEffect>(GetTransientPackage());
		CredsEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UNaviCredsSet::GetCredsAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Additive;
		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(CredsDelta);
		CredsEffect->Modifiers.Add(ModifierInfo);

		ASC->ApplyGameplayEffectToSelf(CredsEffect, 1.0f, ASC->MakeEffectContext());
	}
}

bool UNaviCredsShopComponent::TryBuyEquipment(AController* RequestingPlayerController, FGameplayTag TargetEquipmentTag)
{
	// 구매는 "판정 → 적용" 두 단계로 처리한다.
	// 판정 단계에서는 크레드·슬롯·인벤토리를 전혀 바꾸지 않고, 모든 조건을 통과했을 때만 적용 단계에서 바꾼다.
	// 그래서 실패하면 아무것도 바뀌지 않는다.
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

	if (!CanPurchaseNow(ASC))
	{
		return false;
	}

	// 가격 테이블에 없는 장비(근접 무기, 스파이크 등)는 살 수 없다.
	const int32 Cost = GetEquipmentCost(TargetEquipmentTag);
	if (Cost < 0)
	{
		return false;
	}

	// 크레드 속성 셋 가져오기
	const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>();
	if (!CredsSet)
	{
		return false;
	}
	const float CurrentCreds = CredsSet->GetCreds();

	if (TargetEquipmentTag.MatchesTag(NaviGameplayTags::Weapon))
	{
		return TryBuyWeapon(RequestingPlayerController, ASC, TargetEquipmentTag, Cost, CurrentCreds);
	}
	if (TargetEquipmentTag.MatchesTag(NaviGameplayTags::Armor))
	{
		return TryBuyArmor(ASC, TargetEquipmentTag, Cost, CurrentCreds);
	}
	return false;
}

bool UNaviCredsShopComponent::CanPurchaseNow(const ULyraAbilitySystemComponent* ASC) const
{
	// 구매 페이즈에만 살 수 있다. (Phase_CM_Buying, 하위 태그 Start/CountDown/End 포함)
	const UWorld* World = GetWorld();
	const ULyraGamePhaseSubsystem* PhaseSubsystem = World ? World->GetSubsystem<ULyraGamePhaseSubsystem>() : nullptr;
	if (PhaseSubsystem == nullptr || !PhaseSubsystem->IsPhaseActive(TAG_Navi_Phase_Buying))
	{
		return false;
	}

	// 죽었거나 죽는 중이면 살 수 없다.
	if (ASC == nullptr || ASC->HasMatchingGameplayTag(LyraGameplayTags::Status_Death))
	{
		return false;
	}
	return true;
}

bool UNaviCredsShopComponent::TryBuyWeapon(AController* RequestingPlayerController, ULyraAbilitySystemComponent* ASC, const FGameplayTag& WeaponTag, int32 Cost, float CurrentCreds)
{
	// ---------------------------------------------------------------------
	// 판정 (아무것도 바꾸지 않는다)
	// ---------------------------------------------------------------------
	UNaviQuickBarComponent* QuickBarComponent = RequestingPlayerController->FindComponentByClass<UNaviQuickBarComponent>();
	ULyraInventoryManagerComponent* LyraInventoryManagerComponent = RequestingPlayerController->FindComponentByClass<ULyraInventoryManagerComponent>();
	if (QuickBarComponent == nullptr || LyraInventoryManagerComponent == nullptr)
	{
		return false;
	}

	// 무기 정의. 맵에 없는 태그면 실패한다. (TMap::operator[]는 키가 없으면 크래시하므로 Find로 조회)
	const ULyraWeaponPickupDefinition* WeaponPickupDefinition = GetWeaponPickupDefinition(WeaponTag);
	const TSubclassOf<ULyraInventoryItemDefinition> NewItemDefinition = WeaponPickupDefinition ? WeaponPickupDefinition->InventoryItemDefinition : nullptr;
	const ULyraInventoryItemDefinition* NewItemDefinitionCDO = NewItemDefinition ? NewItemDefinition->GetDefaultObject<ULyraInventoryItemDefinition>() : nullptr;
	if (NewItemDefinitionCDO == nullptr)
	{
		return false;
	}

	// 대상 슬롯. 슬롯이 없는 무기 태그면 실패한다.
	// 새 무기 정의의 태그가 가리키는 슬롯도 같아야 지급(AddItemToSlot)이 실패하지 않는다.
	const int32 EquipmentSlotIndex = QuickBarComponent->GetSlotIndexForItemTag(WeaponTag);
	if (EquipmentSlotIndex == INDEX_NONE || QuickBarComponent->GetSlotIndexForItemTag(NewItemDefinitionCDO->ItemTag) != EquipmentSlotIndex)
	{
		return false;
	}

	ULyraInventoryItemInstance* ItemInstanceInSlot = QuickBarComponent->GetItemInSlot(EquipmentSlotIndex);
	const TSubclassOf<ULyraInventoryItemDefinition> ItemDefinitionClassInSlot = ItemInstanceInSlot ? ItemInstanceInSlot->GetItemDef() : nullptr;
	const ULyraInventoryItemDefinition* ItemDefinitionInSlot = ItemDefinitionClassInSlot ? ItemDefinitionClassInSlot->GetDefaultObject<ULyraInventoryItemDefinition>() : nullptr;
	if (ItemInstanceInSlot != nullptr && ItemDefinitionInSlot == nullptr)
	{
		// 정의를 알 수 없는 아이템은 판매도 버리기도 할 수 없다.
		return false;
	}

	// 같은 무기를 다시 사면 아무것도 하지 않는다. (직접 산 무기든 주운 무기든)
	if (ItemDefinitionInSlot != nullptr && ItemDefinitionInSlot->ItemTag.MatchesTagExact(WeaponTag))
	{
		return false;
	}

	enum class EExistingWeaponAction : uint8
	{
		None,	// 슬롯이 비어 있음
		Drop,	// 주운 무기(Lyra.Item.Dropped): 바닥에 버리고 환불하지 않는다
		Sell	// 직접 산 무기: 슬롯·인벤토리에서 제거하고 가격만큼 돌려받는다 (자동 판매)
	};

	EExistingWeaponAction ExistingWeaponAction = EExistingWeaponAction::None;
	int32 RefundAmount = 0;
	if (ItemInstanceInSlot != nullptr)
	{
		if (ItemInstanceInSlot->HasStatTag(LyraGameplayTags::Lyra_Item_Dropped))
		{
			ExistingWeaponAction = EExistingWeaponAction::Drop;
			if (DroppedWeaponClass == nullptr)
			{
				return false;
			}
		}
		else
		{
			ExistingWeaponAction = EExistingWeaponAction::Sell;
			// 가격 테이블에 없는 무기면(-1) 환불액은 0
			RefundAmount = FMath::Max(0, GetEquipmentCost(ItemDefinitionInSlot->ItemTag));
		}
	}

	// 필요한 크레드 = 새 가격 - 환불액 (더 싼 무기로 바꾸면 음수 = 차액 환불)
	const int32 RequiredCreds = Cost - RefundAmount;
	if (CurrentCreds < RequiredCreds)
	{
		return false;
	}

	// ---------------------------------------------------------------------
	// 적용 (판정을 모두 통과했을 때만): 슬롯 비우기 → 새 무기 지급 → 크레드 정산
	// ---------------------------------------------------------------------
	if (ExistingWeaponAction == EExistingWeaponAction::Drop)
	{
		// 들고 있는 무기가 아니라 대상 슬롯의 주운 무기를 버린다. 픽업 생성에 실패하면 아무것도 바뀌지 않은 상태로 끝난다.
		if (!QuickBarComponent->DropItemInSlot(EquipmentSlotIndex, DroppedWeaponClass))
		{
			return false;
		}
	}
	else if (ExistingWeaponAction == EExistingWeaponAction::Sell)
	{
		QuickBarComponent->RemoveItemFromSlot(EquipmentSlotIndex);
		LyraInventoryManagerComponent->RemoveItemInstance(ItemInstanceInSlot);
	}

	// 슬롯을 비웠고 정의·슬롯을 미리 확인했으므로 여기서는 실패하지 않아야 한다.
	if (!ensureMsgf(GiveWeaponToPlayer(NewItemDefinition, RequestingPlayerController),
		TEXT("Failed to give weapon %s after validation. Slot %d may have changed."), *WeaponTag.ToString(), EquipmentSlotIndex))
	{
		return false;
	}

	NaviCredsShop::ApplyCredsDelta(ASC, static_cast<float>(-RequiredCreds));
	return true;
}

bool UNaviCredsShopComponent::TryBuyArmor(ULyraAbilitySystemComponent* ASC, const FGameplayTag& ArmorTag, int32 Cost, float CurrentCreds)
{
	// ArmorStatTable에서 EquipmentTag에 해당하는 방어구 스탯 정보를 찾습니다.
	// 데이터 테이블의 RowName이 태그의 TagName과 일치한다고 가정합니다.
	const FNaviArmorStatDefinition* ArmorStat = GetArmorStatRow(ArmorTag);
	if (ArmorStat == nullptr)
	{
		return false;
	}

	// 사도 방어구가 늘지 않는 구매는 거부한다.
	// - 같은 방어구를 가득 가진 경우 (예: 25 보유 + 25 구매)
	// - 더 좋은 방어구를 가진 상태에서 낮은 방어구를 사는 경우 (예: 50 보유 + 25 구매 → 50이 25로 줄어듦)
	// 깎인 방어구는 다시 사서 채울 수 있다 (예: 10 보유 + 25 구매).
	const float CurrentArmor = ASC->GetNumericAttribute(ULyraHealthSet::GetArmorAttribute());
	if (CurrentArmor >= static_cast<float>(ArmorStat->AbsorbAmount))
	{
		return false;
	}

	if (CurrentCreds < Cost)
	{
		return false;
	}

	// 크레드 차감
	NaviCredsShop::ApplyCredsDelta(ASC, static_cast<float>(-Cost));

	// Armor 속성을 변경할 GameplayEffect를 생성합니다. (이름을 지정하지 않아 이름 충돌을 피한다)
	UGameplayEffect* ArmorEffect = NewObject<UGameplayEffect>(GetTransientPackage());
	ArmorEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo ArmorModifierInfo;
	ArmorModifierInfo.Attribute = ULyraHealthSet::GetArmorAttribute();
	// AbsorbAmount 값으로 Armor 속성을 덮어씁니다.
	ArmorModifierInfo.ModifierOp = EGameplayModOp::Override;
	ArmorModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(static_cast<float>(ArmorStat->AbsorbAmount));
	ArmorEffect->Modifiers.Add(ArmorModifierInfo);

	// 플레이어의 ASC에 GameplayEffect를 적용하여 Armor 속성을 즉시 업데이트합니다.
	ASC->ApplyGameplayEffectToSelf(ArmorEffect, 1.0f, ASC->MakeEffectContext());
	return true;
}

bool UNaviCredsShopComponent::GiveWeaponToPlayer(TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass, AController* ReceivingController)
{
	if (WeaponItemClass == nullptr || ReceivingController == nullptr)
	{
		return false;
	}

	UNaviQuickBarComponent* QuickBar = ReceivingController->GetComponentByClass<UNaviQuickBarComponent>();
	ULyraInventoryManagerComponent* LyraInventoryManagerComponent = ReceivingController->GetComponentByClass<ULyraInventoryManagerComponent>();
	if (QuickBar == nullptr || LyraInventoryManagerComponent == nullptr)
	{
		return false;
	}

	ULyraInventoryItemInstance* NewItemInstance = LyraInventoryManagerComponent->AddItemDefinition(WeaponItemClass);
	if (NewItemInstance == nullptr)
	{
		return false;
	}

	const int32 AddedIndex = QuickBar->AddItemToSlot(NewItemInstance);
	if (AddedIndex == INDEX_NONE)
	{
		// 슬롯에 넣지 못했으면 인벤토리에 고아 아이템을 남기지 않는다.
		LyraInventoryManagerComponent->RemoveItemInstance(NewItemInstance);
		return false;
	}

	QuickBar->SetActiveSlotIndex(QuickBar->GetActiveSlotIndexOnEquipmentBuy(AddedIndex));
	return true;
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

const ULyraWeaponPickupDefinition* UNaviCredsShopComponent::GetWeaponPickupDefinition(FGameplayTag Tag) const
{
	if (const TObjectPtr<ULyraWeaponPickupDefinition>* FoundDef = WeaponDefinitionMap.Find(Tag))
	{
		return *FoundDef;
	}
	return nullptr;
}
