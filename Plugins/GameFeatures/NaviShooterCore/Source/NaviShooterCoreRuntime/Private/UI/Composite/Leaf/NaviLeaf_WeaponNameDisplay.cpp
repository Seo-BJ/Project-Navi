// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_WeaponNameDisplay.h"

#include "LyraGameplayTags.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UNaviLeaf_WeaponNameDisplay::UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData)
{
	if (Text_WeaponName)
	{
		Text_WeaponName->SetText(WeaponData.DisplayName);
	}

	if (Text_WeaponType)
	{
		Text_WeaponType->SetText(GetWeaponTypeText(WeaponData.WeaponTag));	
	}

	if (Text_Penetration)
	{
		Text_Penetration->SetText(GetFireModeText(WeaponData.PenetrationPower));
	}

	if (Text_FireMode)
	{
		Text_FireMode->SetText(GetFireModeText(WeaponData.FireMode));
	}
	
	// Image_FireMode 로직은 아이콘 리소스가 필요하므로 생략하거나, 추후 Brush 설정 로직 추가 가능
}

FText UNaviLeaf_WeaponNameDisplay::GetFireModeText(ENaviWeaponPenetrationPower Power) const
{
	switch (Power)
	{
	case ENaviWeaponPenetrationPower::Low:
		return NSLOCTEXT("ST_NaviShooterCore", "PenetrationPower_Low", "Low");
	case ENaviWeaponPenetrationPower::Medium:
		return NSLOCTEXT("ST_NaviShooterCore", "PenetrationPower_Medium", "Medium");
	case ENaviWeaponPenetrationPower::High:
		return NSLOCTEXT("ST_NaviShooterCore", "PenetrationPower_High", "High");
	default:
		return FText::GetEmpty();
	}
}


FText UNaviLeaf_WeaponNameDisplay::GetFireModeText(ENaviWeaponFireMode Mode) const
{
	switch (Mode)
	{
	case ENaviWeaponFireMode::Automatic:
		return NSLOCTEXT("ST_NaviShooterCore", "FireMode_Auto", "Automatic");
	case ENaviWeaponFireMode::SemiAutomatic:
		return NSLOCTEXT("ST_NaviShooterCore", "FireMode_Semi", "Semi-Auto");
	default:
		return FText::GetEmpty();
	}
}

FText UNaviLeaf_WeaponNameDisplay::GetWeaponTypeText(FGameplayTag WeaponTag) const
{
	switch (WeaponTag)
	{
	case WeaponTag.MatchesTag(NaviGameplayTags::Weapon_Primary):
		return NSLOCTEXT("ST_NaviShooterCore", "WeaponType_Primary", "Primary Weapon");
	case WeaponTag.MatchesTag(NaviGameplayTags::Weapon_Sidearms):
		return NSLOCTEXT("ST_NaviShooterCore", "WeaponType_Sidearms", "Sidearms");
	default:
		return FText::GetEmpty();
	}
}
