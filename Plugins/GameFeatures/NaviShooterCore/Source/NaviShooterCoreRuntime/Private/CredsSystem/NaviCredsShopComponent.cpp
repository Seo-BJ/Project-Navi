// Copyright Epic Games, Inc. All Rights Reserved.

#include "CredsSystem/NaviCredsShopComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Player/LyraPlayerState.h" // ANaviPlayerState 헤더 필요
#include "AbilitySystem/LyraAbilitySystemComponent.h" // UNaviAbilitySystemComponent 헤더 필요
#include "NaviCredsSet.h" // UNaviCredsSet 헤더 필요
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Equipment/LyraPickupDefinition.h"
#include "Engine/DataTable.h"
#include "NativeGameplayTags.h" // FNativeGameplayTag 필요
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "Inventory/LyraInventoryItemDefinition.h"

// 데이터 테이블에서 사용할 행의 이름을 정의합니다.
// 이는 FNaviWeaponStatDefinition 구조체에 선언된 WeaponTag와 일치해야 합니다.
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Type_Pistol_Classic, "Weapon.Type.Pistol.Classic");
// ...다른 무기 태그들도 여기에 정의할 수 있습니다...

// FNaviWeaponStatDefinition 구조체 정의가 필요합니다.
// 이미 다른 파일에 정의되어 있다면 그 파일을 include 해야 합니다.
// 예시: #include "NaviWeaponStatDefinition.h"

UNaviCredsShopComponent::UNaviCredsShopComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // 이 컴포넌트는 서버에서만 모든 로직을 처리하므로 Tick이 필요 없습니다.
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UNaviCredsShopComponent::BeginPlay()
{
    Super::BeginPlay();
}

int32 UNaviCredsShopComponent::GetWeaponCost(const FGameplayTag& WeaponTag) const
{
    if (!WeaponStatTable)
    {
        return -1;
    }

    // 데이터 테이블에서 태그에 해당하는 행을 찾습니다.
    // 데이터 테이블의 RowName은 FName이므로, 태그를 FName으로 변환해야 합니다.
    const FName RowName = WeaponTag.GetTagName();
    const FNaviWeaponStatDefinition* Row = WeaponStatTable->FindRow<FNaviWeaponStatDefinition>(RowName, TEXT("FindWeaponCost"));

    return Row ? Row->CreditCost : -1;
}

void UNaviCredsShopComponent::PurchaseWeapon(AController* RequestingPlayer, FGameplayTag WeaponTag)
{
    // 서버 권한 확인
    if (!ensure(GetOwner()->HasAuthority()) || !RequestingPlayer || !WeaponTag.IsValid())
    {
        return;
    }

    // 플레이어 스테이트와 어빌리티 시스템 컴포넌트 가져오기
    ALyraPlayerState* PlayerState = RequestingPlayer->GetPlayerState<ALyraPlayerState>();
    ULyraAbilitySystemComponent* ASC = PlayerState ? PlayerState->GetLyraAbilitySystemComponent() : nullptr;
    if (!PlayerState || !ASC)
    {
        return;
    }

    // 구매하려는 무기의 가격 조회
    const int32 Cost = GetWeaponCost(WeaponTag);
    // 판매 목록에 있는 무기인지, 그리고 유효한 가격인지 확인
    if (Cost < 0 /* || !WeaponDefinitionMap.Contains(WeaponTag) */) // TODO: BeginPlay에서 Map 초기화 후 주석 해제
    {
        // 판매하지 않는 아이템
        return;
    }
    
    // 크레드 속성 셋 가져오기
    const UNaviCredsSet* CredsSet = ASC->GetSet<UNaviCredsSet>();
    if (!CredsSet)
    {
        return;
    }

    // 크레드가 충분한지 확인
    if (CredsSet->GetCreds() >= Cost)
    {
        // 크레드 차감 GameplayEffect 생성 및 적용
        UGameplayEffect* CostEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("CredsCostEffect"));
        CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
        
        // 크레드 속성을 차감하는 Modifier 추가
        FGameplayModifierInfo ModifierInfo;
        ModifierInfo.Attribute = UNaviCredsSet::GetCredsAttribute();
        ModifierInfo.ModifierOp = EGameplayModOp::Additive;
        ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(-Cost); // 음수 값으로 더해서 차감
        CostEffect->Modifiers.Add(ModifierInfo);

        ASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, ASC->MakeEffectContext());

        // 무기 지급
        // TODO: WeaponDefinitionMap에서 WeaponDef 조회 후 전달
        GiveWeaponToPlayer(RequestingPlayer, WeaponDefinitionMap[WeaponTag]);
        
        // TODO: 로그 또는 클라이언트에 성공 피드백 전송
    }
    else
    {
        // TODO: 크레드 부족 피드백 전송
    }
}

void UNaviCredsShopComponent::GiveWeaponToPlayer(AController* TargetController, ULyraWeaponPickupDefinition* WeaponDefinition)
{
    if (!TargetController || !TargetController->GetPawn() || !WeaponDefinition)
    {
        return;
    }

    // 플레이어 폰에서 인벤토리 매니저 컴포넌트 찾기
    ULyraInventoryManagerComponent* InventoryManager = TargetController->GetPawn()->FindComponentByClass<ULyraInventoryManagerComponent>();
    if (InventoryManager)
    {
        // 인벤토리에 아이템 정의 추가
        // TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass = WeaponDefinition->InventoryItemDefinition;
        // InventoryManager->AddItemDefinition(WeaponItemClass, 1); // 예시. 수량은 1
    }
}