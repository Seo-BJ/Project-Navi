// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviGameplayTags.h"

namespace NaviGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Damage_BeforeDamage, "Event.Damage.BeforeDamage", "Event fired on Instigator before damage is executed, allowing modification.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Damage_OnHit, "Event.Damage.OnHit", "Event fired on Instigator and Target after damage has been executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage_Amount, "Data.Damage.Amount", "SetByCaller tag for passing damage magnitude.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage_IsCritical, "Data.Damage.IsCritical", "SetByCaller tag for passing critical hit status (0.0f or 1.0f).");
}
