// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ControllerComponent.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "GameplayTagContainer.h"

#include "NaviQuickBarComponent.generated.h"

class AActor;
class ULyraEquipmentInstance;
class ULyraEquipmentManagerComponent;
class UObject;
class ALyraDropAndPickupable;
struct FFrame;

// 이 컴포넌트의 슬롯 정보가 변경될 때 브로드캐스트되는 메시지입니다.
USTRUCT(BlueprintType)
struct FNaviQuickBarSlotsChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TArray<TObjectPtr<ULyraInventoryItemInstance>> Slots;
};

// 활성화된 슬롯의 인덱스가 변경될 때 브로드캐스트되는 메시지입니다.
USTRUCT(BlueprintType)
struct FNaviQuickBarActiveIndexChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	int32 ActiveIndex = 0;
/*
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	FGameplayTag ActiveEquipmentTag;*/
};

/**
 * 발로란트의 슬롯 규칙을 따르는 QuickBarComponent
 * 4개의 고정된 슬롯으로, 특정 아이템(Primary, Sidearms, Melee, Spike) 타입만 허용
 * 이미 슬롯에 아이템이 있는 경우, 아이템을 Drop하거나 Shop에 아이템이 환불된다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()
	
public:
	UNaviQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ UActorComponent interface
	virtual void BeginPlay() override;
	//~ End of UActorComponent interface
	
	/** 다음 슬롯의 아이템을 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "Navi|QuickBar")
	void CycleActiveSlotForward();

	/** 이전 슬롯의 아이템을 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "Navi|QuickBar")
	void CycleActiveSlotBackward();

	UFUNCTION(BlueprintCallable, Category = "Navi|QuickBar")
	void SecActiveSlotIndexOnEquipmentPickUp();

	/** 지정된 인덱스의 슬롯을 활성화(장착)합니다. 서버에서만 호출해야 합니다. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Navi|QuickBar")
	void SetActiveSlotIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navi|QuickBar")
	int32 GetActiveSlotIndexOnEquipmentBuy(int32 SlotIndex);

	/** 모든 슬롯의 아이템 인스턴스 배열을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Navi|QuickBar")
	const TArray<ULyraInventoryItemInstance*>& GetSlots() const { return Slots; }

	/** 현재 활성화된 슬롯의 인덱스를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navi|QuickBar")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	/** 현재 활성화된 슬롯의 아이템 인스턴스를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navi|QuickBar")
	ULyraInventoryItemInstance* GetActiveSlotItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navi|QuickBar")
	ULyraInventoryItemInstance* GetItemInSlot(int32 SlotIndex) const;
	
	/**
	 * 아이템을 타입에 맞는 올바른 슬롯에 추가합니다.
	 * 서버 권한이 필요합니다.
	 * @param Item 추가할 아이템 인스턴스입니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Navi|QuickBar")
	int AddItemToSlot(ULyraInventoryItemInstance* Item);

	/**
	 * 지정된 슬롯에서 아이템을 제거합니다.
	 * 서버 권한이 필요합니다.
	 * @return 제거된 아이템 인스턴스를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Navi|QuickBar")
	ULyraInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Navi|QuickBar")
	void SpawnAndDropEquipment(TSubclassOf<ALyraDropAndPickupable> DroppedAndPickupableClass, ULyraInventoryItemInstance* ItemInstance);
	
	/**
	 * ItemTag에 대하여 지정된 Slot의 Index를 반환한다.
	 * @param ItemTag Item의 Tag
	 * @return 해당 슬롯의 Index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Navi|QuickBar")
	int GetSlotIndexForItemTag(const FGameplayTag& ItemTag) const;
	
private:
	/** 현재 활성화된 슬롯의 아이템을 장착 해제합니다. */
	void UnequipItemInSlot();
	
	/** 현재 활성화된 슬롯의 아이템을 장착합니다. */
	void EquipItemInSlot();

	/** Pawn에 부착된 EquipmentManagerComponent를 찾습니다. */
	ULyraEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	/** 퀵바의 총 슬롯 수. 발로란트 규칙에 따라 4개로 고정됩니다. */
	UPROPERTY()
	int32 NumSlots;

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex(int32 OldIndex);

private:
	/** 슬롯 아이템 목록. 서버에서 클라이언트로 복제됩니다. */
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<TObjectPtr<ULyraInventoryItemInstance>> Slots;

	/** 현재 활성화된 슬롯의 인덱스. 서버에서 클라이언트로 복제됩니다. */
	UPROPERTY()
	int32 LastActiveSlotIndex = -1;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	/** 현재 장착된 장비의 인스턴스입니다. */
	UPROPERTY()
	TObjectPtr<ULyraEquipmentInstance> EquippedItem; // ULyraEquipmentInstance -> UNaviEquipmentInstance로 변경 가능
};

/**
 * 칼 든 상태 
 * 보조무기 구입 -> 보조무기 슬롯으로 이동 
 * 주무기 구입 -> 주무기 슬롯으로 이동 
 *
 * 보조무기 든 상태
 * 보조무기 구입 -> 구매 적용 
 * 주무기 구입 -> 주무기 슬롯으로 이동 
 *
 * 주무기 든 상태
 * 보조무기 구입 -> 슬롯 이동 X
 * 주무기 구입 -> 구매 적용
 *
 *
 * 무기 드랍, 획득시
 * 무기 드랍시 최근 엑티브 슬롯
 * 무기 픽업시 슬롯 변화 X
 *
 *
 * @todo slot이 비워져잇을때 케이스 정리해서 로직 정리 및 리팩토링 필요
 */

