// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviInventoryFragment_SetStatsFromDataTable.h"

#include "Inventory/LyraInventoryItemInstance.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "LyraGameplayTags.h"
#include "Equipment/Weapons/NaviWeaponStatDefinition.h" // 생성하신 데이터 구조체 헤더

void UNaviInventoryFragment_SetStatsFromDataTable::OnInstanceCreated(ULyraInventoryItemInstance* Instance) const
{
    Super::OnInstanceCreated(Instance);

    if (Instance == nullptr || WeaponStatsTable == nullptr)
    {
        return;
    }
    if (TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition = Instance->GetItemDef())
    {
        // Now you can access the ItemTag property
        const FGameplayTag ItemTag = ItemDefinition->GetDefaultObject<ULyraInventoryItemDefinition>()->ItemTag;
        const FName WeaponRowName = ItemTag.GetTagName();
        const FString ContextString(TEXT("Finding Weapon Stats"));
        const FNaviWeaponStatDefinition* WeaponStats = WeaponStatsTable->FindRow<FNaviWeaponStatDefinition>(WeaponRowName, ContextString);

        if (!WeaponStats)
        {
            UE_LOG(LogTemp, Warning, TEXT("WeaponStatsTable [%s]에서 WeaponTag [%s]에 해당하는 행을 찾을 수 없습니다."), *GetNameSafe(WeaponStatsTable), *WeaponRowName.ToString());
            return;
        }
    
        Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_MagazineAmmo, WeaponStats->MagazineSize);
        Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_MagazineSize, WeaponStats->MagazineSize);
        Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_SpareAmmo, WeaponStats->MaxAmmo);
    }
}