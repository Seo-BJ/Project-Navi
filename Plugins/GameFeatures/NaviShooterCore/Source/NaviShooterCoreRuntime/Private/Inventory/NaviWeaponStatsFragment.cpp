// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NaviWeaponStatsFragment.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "NativeGameplayTags.h"

// Define tags for matching. 
// Ideally these should be in a central tag header, but defining locally for the adapter logic is acceptable.
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_FireRate, "Weapon.Stat.FireRate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_RunSpeed, "Weapon.Stat.RunSpeed");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_EquipSpeed, "Weapon.Stat.EquipSpeed");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_ReloadSpeed, "Weapon.Stat.ReloadSpeed");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_MagazineSize, "Weapon.Stat.MagazineSize");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_MaxAmmo, "Weapon.Stat.MaxAmmo");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_FirstShotSpread, "Weapon.Stat.FirstShotSpread");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_CreditCost, "Weapon.Stat.CreditCost");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_Damage_Head, "Weapon.Stat.Damage.Head");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_Damage_Body, "Weapon.Stat.Damage.Body");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_Damage_Leg, "Weapon.Stat.Damage.Leg");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Info_DisplayName, "Weapon.Info.DisplayName");

float UNaviWeaponStatsFragment::GetNumericStat_Implementation(FGameplayTag Tag, bool& bFound) const
{
	// 1. Try to find the row
	if (!WeaponStatRow.IsNull())
	{
		const FNaviWeaponStatDefinition* Row = WeaponStatRow.GetRow<FNaviWeaponStatDefinition>(TEXT("WeaponStatFragment"));
		if (Row)
		{
			bFound = true;
			if (Tag == TAG_Weapon_Stat_FireRate) return Row->FireRate;
			if (Tag == TAG_Weapon_Stat_RunSpeed) return Row->RunSpeed;
			if (Tag == TAG_Weapon_Stat_EquipSpeed) return Row->EquipSpeed;
			if (Tag == TAG_Weapon_Stat_ReloadSpeed) return Row->ReloadSpeed;
			if (Tag == TAG_Weapon_Stat_MagazineSize) return (float)Row->MagazineSize;
			if (Tag == TAG_Weapon_Stat_MaxAmmo) return (float)Row->MaxAmmo;
			if (Tag == TAG_Weapon_Stat_FirstShotSpread) return Row->FirstShotSpread;
			if (Tag == TAG_Weapon_Stat_CreditCost) return (float)Row->CreditCost;
			
			// For damage, we might need a specific range index or just return base damage (Range 0)
			if (Row->DamageFalloffs.Num() > 0)
			{
				const FDamageFalloff& BaseDamage = Row->DamageFalloffs[0];
				if (Tag == TAG_Weapon_Stat_Damage_Head) return BaseDamage.HeadShotDamage;
				if (Tag == TAG_Weapon_Stat_Damage_Body) return BaseDamage.BodyShotDamage;
				if (Tag == TAG_Weapon_Stat_Damage_Leg) return BaseDamage.LegShotDamage;
			}
		}
	}

	// 2. Fallback to the TMap in the base class (manual overrides)
	return Super::GetNumericStat_Implementation(Tag, bFound);
}

FText UNaviWeaponStatsFragment::GetTextData_Implementation(FGameplayTag Tag, bool& bFound) const
{
	if (!WeaponStatRow.IsNull())
	{
		const FNaviWeaponStatDefinition* Row = WeaponStatRow.GetRow<FNaviWeaponStatDefinition>(TEXT("WeaponStatFragment"));
		if (Row)
		{
			if (Tag == TAG_Weapon_Info_DisplayName)
			{
				bFound = true;
				return Row->DisplayName;
			}
		}
	}

	return Super::GetTextData_Implementation(Tag, bFound);
}
