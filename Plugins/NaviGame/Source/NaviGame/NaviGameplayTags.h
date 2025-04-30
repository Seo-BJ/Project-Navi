// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "NativeGameplayTags.h"

namespace NaviGameplayTags
{
	NAVIGAME_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);


	// Event Tags
	NAVIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damage_BeforeDamage); // Fired by Calculation GE on Instigator
	NAVIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damage_OnHit);        // Fired by Execution GE on Instigator & Target

	// SetByCaller Tags (for passing data between GEs)
	NAVIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Amount);        // Used to pass damage value
	NAVIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_IsCritical);    // Used to pass crit status (as float 0 or 1)


	
}