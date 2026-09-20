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
class ULyraAbilitySystemComponent;
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
     * 구매 조건을 모두 판정한 뒤에만 크레드·슬롯·인벤토리를 바꿉니다. 실패하면 아무것도 바뀌지 않고 false를 반환합니다.
     * @param RequestingPlayerController 구매를 요청하는 플레이어의 컨트롤러입니다.
     * @param TargetEquipmentTag 구매하려는 무기의 고유 게임플레이 태그입니다.
     * @return 구매에 성공했으면 true
     */
    UFUNCTION(BlueprintCallable, Category = "Navi|Shop")
    bool TryBuyEquipment(AController* RequestingPlayerController, FGameplayTag TargetEquipmentTag);

public:
	// Find the weapon stat definition row for the given tag
	const FNaviWeaponStatDefinition* GetWeaponStatRow(FGameplayTag Tag) const;

	// Find the armor stat definition row for the given tag
	const FNaviArmorStatDefinition* GetArmorStatRow(FGameplayTag Tag) const; 

	// Find the weapon pickup definition for the given tag
	UFUNCTION(BlueprintCallable, Category = "Navi|Shop")
	const ULyraWeaponPickupDefinition* GetWeaponPickupDefinition(FGameplayTag Tag) const;

	/**
	 * 해당 스탯 태그(예: FireRate)의 전체 무기 중 최솟값(Min)과 최댓값(Max)을 반환합니다.
	 * @param Tag 조회할 스탯 태그
	 * @param OutMin 최솟값 출력
	 * @param OutMax 최댓값 출력
	 * @return 성공 여부 (데이터가 없으면 false)
	 */
	bool GetStatRange(FGameplayTag Tag, float& OutMin, float& OutMax) const;

	const UDataTable* GetWeaponArmorWidgetTable() {return WeaponArmorWidgetTable;}
	
protected:
	// 데이터 테이블을 순회하여 스탯별 Min/Max 값을 계산하고 캐싱합니다.
	void CacheStatRanges();

	// Key: StatTag, Value: X=Min, Y=Max
	UPROPERTY()
	TMap<FGameplayTag, FVector2D> StatMinMaxCache;
	
	UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data")
	TObjectPtr<UDataTable> WeaponStatTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data")
	TObjectPtr<UDataTable> ArmorStatTable;

	UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop|Data")
	TObjectPtr<UDataTable> WeaponArmorWidgetTable;
    
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
     * 슬롯에 넣지 못하면 인벤토리에 추가한 아이템을 되돌리고 false를 반환합니다.
     * @param WeaponItemClass 지급 대상 Weapon의 Item Definition Class
     * @param ReceivingController 지급 대상 Controller
     */
    bool GiveWeaponToPlayer(TSubclassOf<ULyraInventoryItemDefinition> WeaponItemClass, AController* ReceivingController);

    /** 지금 구매할 수 있는 상태인지 (구매 페이즈 + 살아 있음). */
    bool CanPurchaseNow(const ULyraAbilitySystemComponent* ASC) const;

    /** 무기 구매: 판정(빈 슬롯 / 주운 무기 버리기 / 산 무기 자동 판매) 후 적용. */
    bool TryBuyWeapon(AController* RequestingPlayerController, ULyraAbilitySystemComponent* ASC, const FGameplayTag& WeaponTag, int32 Cost, float CurrentCreds);

    /** 방어구 구매: 사도 방어구가 늘지 않는 구매는 거부. */
    bool TryBuyArmor(ULyraAbilitySystemComponent* ASC, const FGameplayTag& ArmorTag, int32 Cost, float CurrentCreds);

    UPROPERTY(EditDefaultsOnly, Category = "Navi|Shop")
    TSubclassOf<ALyraDropAndPickupable> DroppedWeaponClass;
    
};