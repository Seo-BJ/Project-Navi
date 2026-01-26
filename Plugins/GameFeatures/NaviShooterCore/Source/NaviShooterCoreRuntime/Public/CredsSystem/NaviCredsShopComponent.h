// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "System/GameplayTagStack.h"
#include "NaviCredsShopComponent.generated.h"

class ULyraInventoryItemDefinition;
class ULyraWeaponPickupDefinition;
class UDataTable;
class AController;
class ALyraDropAndPickupable;
struct FNaviWeaponStatDefinition;
struct FNaviArmorStatDefinition;

/**
 * GameState에 추가되어 무기 상점 역할을 하는 컴포넌트.
 * 모든 무기 정보를 관리하고 플레이어의 구매 요청을 처리합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviCredsShopComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    
    UNaviCredsShopComponent(const FObjectInitializer& ObjectInitializer);

    //~ UActorComponent interface
    virtual void BeginPlay() override;
    //~ End of UActorComponent interface

    /**
     * 클라이언트가 서버에 무기 구매를 요청할 때 호출합니다.
     * @param RequestingPlayerController 구매를 요청하는 플레이어의 컨트롤러입니다.
     * @param TargetEquipmentTag 구매하려는 무기의 고유 게임플레이 태그입니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Navi|Shop")
    bool TryBuyEquipment(AController* RequestingPlayerController, FGameplayTag TargetEquipmentTag);

public:
	// Find the weapon stat definition row for the given tag
	const FNaviWeaponStatDefinition* GetWeaponStatRow(FGameplayTag Tag) const;

	// Find the armor stat definition row for the given tag
	// (Assuming Armor uses the same definition or a similar one. Adjust return type if FNaviArmorStatDefinition exists)
	const FNaviArmorStatDefinition* GetArmorStatRow(FGameplayTag Tag) const; 

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data")
    TObjectPtr<UDataTable> WeaponStatTable;

    UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data")
    TObjectPtr<UDataTable> ArmorStatTable;
    
    /**
     * 빠른 조회를 위해 GameplayTag를 키로 사용하는 무기 정의 맵입니다.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data", meta = (Categories = "Weapon"))
    TMap<FGameplayTag, TObjectPtr<ULyraWeaponPickupDefinition>> WeaponDefinitionMap;

    /**
     * 태그에 해당하는 무기의 가격을 데이터 테이블에서 찾아 반환합니다.
     * @param EquipmentTag 가격을 조회할 무기의 태그입니다.
     * @return 무기의 가격. 찾지 못하면 -1을 반환합니다.
     */
    int32 GetEquipmentCost(const FGameplayTag& EquipmentTag) const;

    /**
     * 플레이어에게 무기 아이템을 지급합니다.
     * @param WeaponItemClass 지급 대상 Weapon의 Item Definition Class
     * @param ReceivingController 지급 대상 Controller
     */
    void GiveWeaponToPlayer(TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass, AController* ReceivingController);

    UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop")
    TSubclassOf<ALyraDropAndPickupable> DroppedWeaponClass;
    
};