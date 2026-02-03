// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAbilityCost_ItemTagStack.h"

#include "AbilitySystemComponent.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraAbilityCost_ItemTagStack)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

ULyraAbilityCost_ItemTagStack::ULyraAbilityCost_ItemTagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_ABILITY_FAIL_COST;
}

bool ULyraAbilityCost_ItemTagStack::CheckCost(const ULyraGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const ULyraGameplayAbility_FromEquipment* EquipmentAbility = Cast<const ULyraGameplayAbility_FromEquipment>(Ability))
	{
		if (ULyraInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
			bool bCanApplyCost = ItemInstance->GetStatTagStackCount(Tag) >= NumStacks;

			// Inform other abilities why this cost cannot be applied
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}

			// Check ActorInfo has Conditional Free Tag
			UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
			if (IsValid(AbilitySystemComponent) && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ConditionalFreeTag))
			{
				bCanApplyCost = true;
			}
			return bCanApplyCost;
		}
	}
	return false;
}

void ULyraAbilityCost_ItemTagStack::ApplyCost(const ULyraGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const ULyraGameplayAbility_FromEquipment* EquipmentAbility = Cast<const ULyraGameplayAbility_FromEquipment>(Ability))
		{
			if (ULyraInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				// Check ActorInfo has Conditional Free Tag
				UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
				if (IsValid(AbilitySystemComponent) && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ConditionalFreeTag))
				{
					NumStacks = 0;
				}
				
				ItemInstance->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}

