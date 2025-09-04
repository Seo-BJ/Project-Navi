// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Phases/NaviPostRoundPhaseAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "LyraGameplayTags.h"
#include "GameFramework/GameModeBase.h"
#include "Player/LyraPlayerController.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "GameModes/LyraGameMode.h"

void UNaviPostRoundPhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                 const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		ALyraGameMode* GameMode = GetWorld()->GetAuthGameMode<ALyraGameMode>();
		if (!GameMode) return;

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ALyraPlayerController* LyraPC = Cast<ALyraPlayerController>(It->Get());
			if (!IsValid(LyraPC))
			{
				continue;
			}
			ULyraAbilitySystemComponent* LyraASC = LyraPC->GetLyraAbilitySystemComponent();
			if (!IsValid(LyraASC))
			{
				continue;
			}
			if (LyraASC->HasMatchingGameplayTag(LyraGameplayTags::Status_Death_Dying))
			{
				const FGameplayTag DeathAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Type.StatusChange.Death");

				TArray<FGameplayAbilitySpecHandle> OutAbilityHandles;
				LyraASC->FindAllAbilitiesWithTags(OutAbilityHandles, FGameplayTagContainer(DeathAbilityTag));

				for (const FGameplayAbilitySpecHandle& SpecHandle : OutAbilityHandles)
				{
					LyraASC->CancelAbilityHandle(SpecHandle);
					GameMode->RequestPlayerRestartNextFrame((LyraPC), true);
				}
			}
			else
			{
				GameMode->RequestPlayerRestartNextFrame((LyraPC), false);
			}
		}
	}
}
