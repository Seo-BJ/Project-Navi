// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NaviInventoryFragment_AltFire.h"

void UNaviInventoryFragment_AltFire::GetAbilitySetsToGrant(TArray<const ULyraAbilitySet*>& OutAbilitySets) const
{
	OutAbilitySets.Append(AbilitySetsToGrant);
}
