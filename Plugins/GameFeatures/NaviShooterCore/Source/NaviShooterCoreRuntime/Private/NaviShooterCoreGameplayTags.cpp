// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviShooterCoreGameplayTags.h"

namespace NaviShooterCoreGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_FireRate, "Weapon.Stat.FireRate");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_RunSpeed, "Weapon.Stat.RunSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_EquipSpeed, "Weapon.Stat.EquipSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_ReloadSpeed, "Weapon.Stat.ReloadSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_MagazineSize, "Weapon.Stat.MagazineSize");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_MaxAmmo, "Weapon.Stat.MaxAmmo");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_FirstShotSpread, "Weapon.Stat.FirstShotSpread");

	UE_DEFINE_GAMEPLAY_TAG (Weapon_Stat_FirstShotSpread_HipFire, "Weapon.Stat.FirstShotSpread.HipFire");
	UE_DEFINE_GAMEPLAY_TAG (Weapon_Stat_FirstShotSpread_ADS,"Weapon.Stat.FirstShotSpread.ADS");
	
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_CreditCost, "Weapon.Stat.CreditCost");
	
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_Damage_Head, "Weapon.Stat.Damage.Head");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_Damage_Body, "Weapon.Stat.Damage.Body");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_Damage_Leg, "Weapon.Stat.Damage.Leg");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Stat_Damage_Falloffs, "Weapon.Stat.Damage.Falloffs");
}
