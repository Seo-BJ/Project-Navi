// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_StatDisplay.h"
#include "Components/TextBlock.h"
#include "Inventory/NaviInventoryFragment_UIInfo.h"
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

void UNaviLeaf_StatDisplay::UpdateView(const ULyraInventoryItemDefinition* ItemDef)
{
	Super::UpdateView(ItemDef);

	if (!ItemDef)
	{
		SetIsCollapsed(true);
		return;
	}

	// 1. Update Label (Static or Localized text)
	UpdateLabel();

	// 2. Fetch Data
	const UNaviInventoryFragment_UIInfo* UIInfo = Cast<UNaviInventoryFragment_UIInfo>(
		ItemDef->FindFragmentByClass(UNaviInventoryFragment_UIInfo::StaticClass())
	);

	bool bFound = false;
	float StatValue = 0.0f;

	if (UIInfo)
	{
		StatValue = UIInfo->GetNumericStat(TargetTag, bFound);
	}

	// 3. Update Value or Collapse
	if (bFound)
	{
		UpdateValue(StatValue);
		SetIsCollapsed(false);
	}
	else
	{
		if (bCollapseIfMissing)
		{
			SetIsCollapsed(true);
		}
		else
		{
			// Optional: Clear text or show default "0" or "-"
			if (Text_Value)
			{
				Text_Value->SetText(FText::FromString(TEXT("-")));
			}
			SetIsCollapsed(false);
		}
	}
}

void UNaviLeaf_StatDisplay::UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponRow)
{
	float ValueToSet = 0.0f;
	bool bValueFound = false;

	if (TargetTag == TAG_Weapon_Stat_FireRate) { ValueToSet = WeaponRow.FireRate; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_RunSpeed) { ValueToSet = WeaponRow.RunSpeed; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_EquipSpeed) { ValueToSet = WeaponRow.EquipSpeed; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_ReloadSpeed) { ValueToSet = WeaponRow.ReloadSpeed; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_MagazineSize) { ValueToSet = (float)WeaponRow.MagazineSize; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_MaxAmmo) { ValueToSet = (float)WeaponRow.MaxAmmo; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_FirstShotSpread) { ValueToSet = WeaponRow.FirstShotSpread; bValueFound = true; }
	else if (TargetTag == TAG_Weapon_Stat_CreditCost) { ValueToSet = (float)WeaponRow.CreditCost; bValueFound = true; }
	
	// Damage logic (Base damage for simple display)
	if (WeaponRow.DamageFalloffs.Num() > 0)
	{
		const FDamageFalloff& BaseDamage = WeaponRow.DamageFalloffs[0];
		if (TargetTag == TAG_Weapon_Stat_Damage_Head) { ValueToSet = BaseDamage.HeadShotDamage; bValueFound = true; }
		else if (TargetTag == TAG_Weapon_Stat_Damage_Body) { ValueToSet = BaseDamage.BodyShotDamage; bValueFound = true; }
		else if (TargetTag == TAG_Weapon_Stat_Damage_Leg) { ValueToSet = BaseDamage.LegShotDamage; bValueFound = true; }
	}

	if (bValueFound)
	{
		SetStatValue(ValueToSet);
	}
	else
	{
		SetIsCollapsed(true);
	}
}

void UNaviLeaf_StatDisplay::SetStatValue(float Value)
{
	UpdateLabel();
	UpdateValue(Value);
	SetIsCollapsed(false);
}

void UNaviLeaf_StatDisplay::UpdateLabel()
{
	if (Text_Label)
	{
		if (!LabelText.IsEmpty())
		{
			Text_Label->SetText(LabelText);
			Text_Label->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Text_Label->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UNaviLeaf_StatDisplay::UpdateValue(float Value)
{
	if (Text_Value)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = MinFractionalDigits;
		Options.MaximumFractionalDigits = MaxFractionalDigits;

		Text_Value->SetText(FText::AsNumber(Value, &Options));
	}
}

void UNaviLeaf_StatDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();
}
