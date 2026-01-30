// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NaviInventoryFragment_SetStatsFromDataTable.h"

#include "Inventory/LyraInventoryItemInstance.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "LyraGameplayTags.h"
#include "Weapons/NaviWeaponStatDefinition.h" // 생성하신 데이터 구조체 헤더

void UNaviInventoryFragment_SetStatsFromDataTable::OnInstanceCreated(ULyraInventoryItemInstance* Instance) const
{
    Super::OnInstanceCreated(Instance);
    
    if (Instance == nullptr || WeaponStatRow.IsNull())
    {
        return;
    }
    
    // Instance를 통해 장착하는 경우 StatTags의 Count가 0보다 크면 Return. 기존의 StatTags를 그대로 사용.
    int32 TotalRemainAmmo = Instance->GetStatTagStackCount(LyraGameplayTags::Lyra_ShooterGame_Weapon_SpareAmmo)
    + Instance->GetStatTagStackCount(LyraGameplayTags::Lyra_ShooterGame_Weapon_MagazineAmmo);
    if (TotalRemainAmmo > 0)
    {
        return;
    }
    
    if (TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition = Instance->GetItemDef())
    {
        if (const FNaviWeaponStatDefinition* WeaponStats = WeaponStatRow.GetRow<FNaviWeaponStatDefinition>(TEXT("LyraRangedWeaponInstance_GetStats")))
        {
            Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_MagazineAmmo, WeaponStats->MagazineSize);
            Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_MagazineSize, WeaponStats->MagazineSize);
            Instance->AddStatTagStack(LyraGameplayTags::Lyra_ShooterGame_Weapon_SpareAmmo, WeaponStats->MaxAmmo);
        }
    }
}