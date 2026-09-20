// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicalMaterialWithTags.h"

#include "LyraGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PhysicalMaterialWithTags)

UPhysicalMaterialWithTags::UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayTag UPhysicalMaterialWithTags::FindZoneTag(const UPhysicalMaterial* PhysicalMaterial)
{
	if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
	{
		for (const FGameplayTag& MaterialTag : PhysMatWithTags->Tags)
		{
			if (MaterialTag.MatchesTag(LyraGameplayTags::Gameplay_Zone))
			{
				return MaterialTag;
			}
		}
	}
	return FGameplayTag();
}

