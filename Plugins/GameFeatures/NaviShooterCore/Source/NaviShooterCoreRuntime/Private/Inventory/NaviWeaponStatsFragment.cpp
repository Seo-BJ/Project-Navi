// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NaviWeaponStatsFragment.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "NativeGameplayTags.h"
#include "NaviShooterCoreGameplayTags.h"

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
			if (Tag == NaviShooterCoreGameplayTags::Weapon_Stat_FireRate) return Row->FireRate;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_RunSpeed) return Row->RunSpeed;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_EquipSpeed) return Row->EquipSpeed;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_ReloadSpeed) return Row->ReloadSpeed;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_MagazineSize) return (float)Row->MagazineSize;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_MaxAmmo) return (float)Row->MaxAmmo;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_FirstShotSpread_HipFire) return Row->FirstShotSpread_HipFire;
			if (Tag ==  NaviShooterCoreGameplayTags::Weapon_Stat_FirstShotSpread_ADS) return Row->FirstShotSpread_ADS;
			if (Tag == NaviShooterCoreGameplayTags::Weapon_Stat_CreditCost) return (float)Row->CreditCost;
			
			// For damage, we might need a specific range index or just return base damage (Range 0)
			if (Row->DamageFalloffs.Num() > 0)
			{
				const FDamageFalloff& BaseDamage = Row->DamageFalloffs[0];
				if (Tag == NaviShooterCoreGameplayTags::Weapon_Stat_Damage_Head) return BaseDamage.HeadShotDamage;
				if (Tag == NaviShooterCoreGameplayTags::Weapon_Stat_Damage_Body) return BaseDamage.BodyShotDamage;
				if (Tag == NaviShooterCoreGameplayTags::Weapon_Stat_Damage_Leg) return BaseDamage.LegShotDamage;
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
