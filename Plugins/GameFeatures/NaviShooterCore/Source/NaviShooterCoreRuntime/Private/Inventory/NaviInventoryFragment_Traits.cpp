// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NaviInventoryFragment_Traits.h"
#include "Weapons/NaviWeaponTraitDefinition.h"

void UNaviInventoryFragment_Traits::GetAbilitySetsToGrant(TArray<const ULyraAbilitySet*>& OutAbilitySets) const
{
	for (const UNaviWeaponTraitDefinition* TraitDef : Traits)
	{
		if (TraitDef)
		{
			OutAbilitySets.Append(TraitDef->AbilitySetsToGrant);
		}
	}
}
