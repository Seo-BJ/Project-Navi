// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviWeaponStatsContainer.h"
#include "UI/Composite/Leaf/NaviLeaf_StatDisplay.h"
#include "UI/Composite/Leaf/NaviLeaf_DamageFalloffsDisplay.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "NativeGameplayTags.h"

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
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Weapon_Stat_Damage_Falloffs, "Weapon.Stat.Damage.Falloffs");

void UNaviWeaponStatsContainer::SetWeaponData(const FNaviWeaponStatDefinition* WeaponRow)
{
	if (!WeaponRow)
	{
		SetIsCollapsed(true);
		return;
	}

	SetIsCollapsed(false);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	for (UNaviCompositeBase* Child : Children)
	{
		// 1. 일반 스탯 디스플레이 처리
		if (UNaviLeaf_StatDisplay* StatLeaf = Cast<UNaviLeaf_StatDisplay>(Child))
		{
			FGameplayTag LeafTargetTag = StatLeaf->GetTargetTag();
			float ValueToSet = 0.0f;
			bool bValueFound = false;

			if (LeafTargetTag == TAG_Weapon_Stat_FireRate) { ValueToSet = WeaponRow->FireRate; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_RunSpeed) { ValueToSet = WeaponRow->RunSpeed; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_EquipSpeed) { ValueToSet = WeaponRow->EquipSpeed; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_ReloadSpeed) { ValueToSet = WeaponRow->ReloadSpeed; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_MagazineSize) { ValueToSet = (float)WeaponRow->MagazineSize; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_MaxAmmo) { ValueToSet = (float)WeaponRow->MaxAmmo; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_FirstShotSpread) { ValueToSet = WeaponRow->FirstShotSpread; bValueFound = true; }
			else if (LeafTargetTag == TAG_Weapon_Stat_CreditCost) { ValueToSet = (float)WeaponRow->CreditCost; bValueFound = true; }
			
			// Damage logic (Base damage for simple display)
			if (WeaponRow->DamageFalloffs.Num() > 0)
			{
				const FDamageFalloff& BaseDamage = WeaponRow->DamageFalloffs[0];
				if (LeafTargetTag == TAG_Weapon_Stat_Damage_Head) { ValueToSet = BaseDamage.HeadShotDamage; bValueFound = true; }
				else if (LeafTargetTag == TAG_Weapon_Stat_Damage_Body) { ValueToSet = BaseDamage.BodyShotDamage; bValueFound = true; }
				else if (LeafTargetTag == TAG_Weapon_Stat_Damage_Leg) { ValueToSet = BaseDamage.LegShotDamage; bValueFound = true; }
			}

			if (bValueFound)
			{
				StatLeaf->SetStatValue(ValueToSet);
			}
			else
			{
				StatLeaf->SetIsCollapsed(true);
			}
		}
		// 2. 데미지 감소(Falloffs) 디스플레이 처리
		else if (Child->GetTargetTag().MatchesTag(TAG_Weapon_Stat_Damage_Falloffs))
		{
			if (UNaviLeaf_DamageFalloffsDisplay* FalloffLeaf = Cast<UNaviLeaf_DamageFalloffsDisplay>(Child))
			{
				FalloffLeaf->SetFalloffData(WeaponRow->DamageFalloffs);
			}
		}
	}
}

void UNaviWeaponStatsContainer::SetWeaponToDisplay(const ULyraInventoryItemDefinition* ItemDef)
{
	// Deprecated implementation or fallback
	Super::UpdateView(ItemDef);
}