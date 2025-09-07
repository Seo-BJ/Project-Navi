// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Equipment/LyraEquipmentInstance.h"
#include "GameplayTags.h"
#include "Cosmetics/LyraCharacterPartTypes.h"

#include "NaviAgentInstance.generated.h"

class UGameplayEffect;
class ULyraInventoryItemDefinition;
class ULyraInventoryItemInstance;
class USkeletalMesh;
class UAnimInstance;
struct FActiveGameplayEffectHandle;


/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviAgentInstance : public ULyraEquipmentInstance
{
	GENERATED_BODY()
	
public:
	UNaviAgentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	// ~ULyraEquipmentInstance
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	// ~End of ULyraEquipmentInstance
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi Agent | Instance")
	TArray<FLyraCharacterPart> CharacterParts;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi Agent | Instance")
	TSubclassOf<UGameplayEffect> AdditionalAttributes;
;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi Agent | Instance")
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi Agent | Instance")
	TSoftClassPtr<UAnimInstance> AnimInstanceOverride;
    
private:
	// 적용된 기본 스탯 게임플레이 이펙트의 핸들입니다. 해제 시 사용됩니다.
	FActiveGameplayEffectHandle AppliedGEHandle;

	// 지급된 아이템 인스턴스 목록입니다. 해제 시 사용됩니다.
	TArray<TObjectPtr<ULyraInventoryItemInstance>> GrantedItemInstances;

	// 스폰된 캐릭터 파츠 액터 목록입니다. 해제 시 사용됩니다.
	TArray<TObjectPtr<AActor>> SpawnedCharacterParts;

};
