// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_StatDisplay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Inventory/NaviInventoryFragment_UIInfo.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "NativeGameplayTags.h"
#include "CredsSystem/NaviCredsShopComponent.h"
#include "GameFramework/GameStateBase.h"
#include "NaviShooterCoreGameplayTags.h"

void UNaviLeaf_StatDisplay::NativePreConstruct()
{
	Super::NativePreConstruct();
}

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
		SetStatValue(StatValue);
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
	FText UnitText = FText::GetEmpty();

	using namespace NaviShooterCoreGameplayTags;

	if (TargetTag == Weapon_Stat_FireRate) 
	{ 
		ValueToSet = WeaponRow.FireRate; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_RoundsPerSec", "round/second");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_RunSpeed) 
	{ 
		ValueToSet = WeaponRow.RunSpeed; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_MetersPerSec", "m/s");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_EquipSpeed) 
	{ 
		ValueToSet = WeaponRow.EquipSpeed; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_Seconds", "second");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_ReloadSpeed) 
	{ 
		ValueToSet = WeaponRow.ReloadSpeed; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_Seconds", "second");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_MagazineSize) 
	{ 
		ValueToSet = (float)WeaponRow.MagazineSize; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_Rounds", "round");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_MaxAmmo) 
	{ 
		ValueToSet = (float)WeaponRow.MaxAmmo; 
		UnitText = NSLOCTEXT("ST_NaviShooterCore", "Unit_Rounds", "round");
		bValueFound = true; 
	}
	else if (TargetTag == Weapon_Stat_FirstShotSpread) 
	{ 
		// Special case for Spread: Show Hip / ADS
		ValueToSet = WeaponRow.FirstShotSpread_HipFire; // Bar shows Hip spread
		
		FNumberFormattingOptions Opts;
		Opts.MinimumFractionalDigits = 1;
		Opts.MaximumFractionalDigits = 2;
		
		FText HipText = FText::AsNumber(WeaponRow.FirstShotSpread_HipFire, &Opts);
		FText ADSText = FText::AsNumber(WeaponRow.FirstShotSpread_ADS, &Opts);
		
		// Format: "1.0 / 0.5"
		FText SpreadText = FText::Format(NSLOCTEXT("Navi", "Format_Spread", "{0} / {1}"), HipText, ADSText);
		
		SetStatValueComplex(ValueToSet, SpreadText);
		return;
	}
	else if (TargetTag == Weapon_Stat_CreditCost) 
	{ 
		ValueToSet = (float)WeaponRow.CreditCost; 
		bValueFound = true; 
	}
	
	// Damage logic (Base damage for simple display)
	if (WeaponRow.DamageFalloffs.Num() > 0)
	{
		const FDamageFalloff& BaseDamage = WeaponRow.DamageFalloffs[0];
		if (TargetTag == Weapon_Stat_Damage_Head) { ValueToSet = BaseDamage.HeadShotDamage; bValueFound = true; }
		else if (TargetTag == Weapon_Stat_Damage_Body) { ValueToSet = BaseDamage.BodyShotDamage; bValueFound = true; }
		else if (TargetTag == Weapon_Stat_Damage_Leg) { ValueToSet = BaseDamage.LegShotDamage; bValueFound = true; }
	}

	if (bValueFound)
	{
		SetStatValue(ValueToSet, UnitText);
	}
	else
	{
		SetIsCollapsed(true);
	}
}

void UNaviLeaf_StatDisplay::SetStatValue(float Value, const FText& UnitText)
{
	UpdateLabel();
	UpdateValue(Value);
	
	// Append Unit if exists
	if (!UnitText.IsEmpty() && Text_Value)
	{
		FText CurrentText = Text_Value->GetText();
		Text_Value->SetText(FText::Format(NSLOCTEXT("Navi", "Format_StatValueUnit", "{0} {1}"), CurrentText, UnitText));
	}
	
	SetIsCollapsed(false);
}

void UNaviLeaf_StatDisplay::SetStatValueComplex(float ValueForBar, const FText& FullDisplayText)
{
	UpdateLabel();
	
	// Update Bar
	if (ProgressBar_Value)
	{
		UpdateProgressBar(ValueForBar);
	}
	
	// Set Custom Text
	if (Text_Value)
	{
		Text_Value->SetText(FullDisplayText);
	}
	
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

	if (ProgressBar_Value)
	{
		UpdateProgressBar(Value);
	}
}

void UNaviLeaf_StatDisplay::UpdateProgressBar(float Value)
{
	float Percent = 0.0f;
	
	// Find Shop Component to get Min/Max
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GS = World->GetGameState())
		{
			if (UNaviCredsShopComponent* ShopComp = GS->GetComponentByClass<UNaviCredsShopComponent>())
			{
				float MinStat = 0.f;
				float MaxStat = 1.f;
				
				// Get Cached Range
				if (ShopComp->GetStatRange(TargetTag, MinStat, MaxStat))
				{
					// Normalize Value (0..1)
					if (FMath::IsNearlyEqual(MinStat, MaxStat))
					{
						Percent = 1.0f;
					}
					else
					{
						Percent = (Value - MinStat) / (MaxStat - MinStat);
					}
				}
			}
		}
	}
	
	ProgressBar_Value->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
}