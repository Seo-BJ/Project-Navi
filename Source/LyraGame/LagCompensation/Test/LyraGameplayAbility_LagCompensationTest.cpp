// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/Test/LyraGameplayAbility_LagCompensationTest.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "LagCompensation/Test/LyraLagCompensationTestCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_LagCompensationTest)

ULyraGameplayAbility_LagCompensationTest::ULyraGameplayAbility_LagCompensationTest(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void ULyraGameplayAbility_LagCompensationTest::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const ALyraLagCompensationTestCharacter* TestCharacter =
		ActorInfo ? Cast<ALyraLagCompensationTestCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UAnimMontage* Montage = TestCharacter ? TestCharacter->GetPeriodicTestMontage() : nullptr;
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		TestCharacter->GetPeriodicMontagePlayRate());

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelledOrInterrupted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelledOrInterrupted);
	MontageTask->ReadyForActivation();
}

void ULyraGameplayAbility_LagCompensationTest::OnMontageCompleted()
{
	if (ALyraLagCompensationTestCharacter* TestCharacter = Cast<ALyraLagCompensationTestCharacter>(GetAvatarActorFromActorInfo()))
	{
		TestCharacter->RecordPeriodicTestMontageResult(/*bInterrupted=*/false);
	}
	K2_EndAbility();
}

void ULyraGameplayAbility_LagCompensationTest::OnMontageCancelledOrInterrupted()
{
	if (ALyraLagCompensationTestCharacter* TestCharacter = Cast<ALyraLagCompensationTestCharacter>(GetAvatarActorFromActorInfo()))
	{
		TestCharacter->RecordPeriodicTestMontageResult(/*bInterrupted=*/true);
	}
	K2_CancelAbility();
}
