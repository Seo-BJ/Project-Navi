// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayAbility_RangedWeapon.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "Physics/LyraCollisionChannels.h"
#include "LyraLogChannels.h"
#include "Interaction/LyraWeaponInteractable.h"
#include "AIController.h"
#include "NativeGameplayTags.h"
#include "Weapons/LyraWeaponStateComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "DrawDebugHelpers.h"
#include "GameplayCueFunctionLibrary.h"
#include "GameplayPrediction.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "LyraWeaponDebugSettings.h"

#include "TimerManager.h"
#include "Character/LyraCharacter.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent.h"
#include "LagCompensation/ServerSideRewind/LyraServerSideRewindSubsystem.h"
#include "LagCompensation/LyraTimeSyncComponent.h"
#include "Player/LyraPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_RangedWeapon)

namespace LyraConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuration(
		TEXT("lyra.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHits(
		TEXT("lyra.Weapon.DrawBulletHitDuration"),
		DrawBulletHitDuration,
		TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitRadius = 3.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
		TEXT("lyra.Weapon.DrawBulletHitRadius"),
		DrawBulletHitRadius,
		TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in cm)"),
		ECVF_Default);

}

// Weapon fire will be blocked/canceled if the player has this tag
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WeaponFireBlocked, "Ability.Weapon.NoFiring");

//////////////////////////////////////////////////////////////////////

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
		// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}


ULyraGameplayAbility_RangedWeapon::ULyraGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SourceBlockedTags.AddTag(TAG_WeaponFireBlocked);
}

ULyraRangedWeaponInstance* ULyraGameplayAbility_RangedWeapon::GetWeaponInstance() const
{
	return Cast<ULyraRangedWeaponInstance>(GetAssociatedEquipment());
}

bool ULyraGameplayAbility_RangedWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (bResult)
	{
		if (GetWeaponInstance() == nullptr)
		{
			UE_LOG(LogLyraAbilitySystem, Error, TEXT("Weapon ability %s cannot be activated because there is no associated ranged weapon (equipment instance=%s but needs to be derived from %s)"),
				*GetPathName(),
				*GetPathNameSafe(GetAssociatedEquipment()),
				*ULyraRangedWeaponInstance::StaticClass()->GetName());
			bResult = false;
		}
	}

	return bResult;
}

int32 ULyraGameplayAbility_RangedWeapon::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// If we hit a pawn, we're good
			return Idx;
		}
		else
		{
			AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
			{
				// If we hit something attached to a pawn, we're good
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

void ULyraGameplayAbility_RangedWeapon::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

ECollisionChannel ULyraGameplayAbility_RangedWeapon::DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated, bool bTraceForServerSideRewind) const
{
	if (bTraceForServerSideRewind)
	{
		return Lyra_TraceChannel_LagCompensation_HitBox;
	}
	else
	{
		return Lyra_TraceChannel_Weapon;
	}
}

FHitResult ULyraGameplayAbility_RangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;

	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated, false);

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// Filter the output list to prevent multiple hits on the same actor;
		// this is to prevent a single bullet dealing damage multiple times to
		// a single actor if using an overlap trace
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

FVector ULyraGameplayAbility_RangedWeapon::GetWeaponTargetingSourceLocation() const
{
	// Use Pawn's location as a base
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	check(AvatarPawn);

	const FVector SourceLoc = AvatarPawn->GetActorLocation();
	const FQuat SourceRot = AvatarPawn->GetActorQuat();

	FVector TargetingSourceLocation = SourceLoc;

	//@TODO: Add an offset from the weapon instance and adjust based on pawn crouch/aiming/etc...

	return TargetingSourceLocation;
}

FTransform ULyraGameplayAbility_RangedWeapon::GetTargetingTransform(APawn* SourcePawn, ELyraAbilityTargetingSource Source) const
{
	check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController(); 
	ULyraWeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr) ? SourcePawnController->FindComponentByClass<ULyraWeaponStateComponent>() : nullptr;

	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != ELyraAbilityTargetingSource::Custom);

	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;


	if ((Controller != nullptr) && ((Source == ELyraAbilityTargetingSource::CameraTowardsFocus) || (Source == ELyraAbilityTargetingSource::PawnTowardsFocus) || (Source == ELyraAbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Get camera position for later
		bFoundFocus = true;

		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
		}
		else
		{
			SourceLoc = GetWeaponTargetingSourceLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		// Determine initial focal point to 
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);

		// Move the start and focal point up in front of pawn
		if (PC)
		{
			const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
			CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
			FocalLoc = CamLoc + (AimDir * FocalDistance);
		}
		//Move the start to be the HeadPosition of the AI
		else if (AAIController* AIController = Cast<AAIController>(Controller))
		{
			CamLoc = SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
		}

		if (Source == ELyraAbilityTargetingSource::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == ELyraAbilityTargetingSource::WeaponForward) || (Source == ELyraAbilityTargetingSource::WeaponTowardsFocus))
	{
		SourceLoc = GetWeaponTargetingSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == ELyraAbilityTargetingSource::PawnTowardsFocus) || (Source == ELyraAbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

FHitResult ULyraGameplayAbility_RangedWeapon::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
	if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
	}
#endif // ENABLE_DRAW_DEBUG

	FHitResult Impact;

	// Trace and process instant hit if something was hit
	// First trace without using sweep radius
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// If we had a blocking hit in our line trace that occurs in SweepHits before our
				// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

void ULyraGameplayAbility_RangedWeapon::PerformLocalTargeting(OUT TArray<FHitResult>& OutHits)
{
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponData)
	{
		FRangedWeaponFiringInput InputData;
		InputData.WeaponData = WeaponData;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		//@TODO: Should do more complicated logic here when the player is close to a wall, etc...
		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, ELyraAbilityTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
		if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
		{
			static float DebugThickness = 2.0f;
			DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f), FColor::Yellow, false, LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif

		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

void ULyraGameplayAbility_RangedWeapon::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits)
{
	ULyraRangedWeaponInstance* WeaponData = InputData.WeaponData;
	check(WeaponData);
	
	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), /*bIsSimulated=*/ false, /*out*/ AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
#if ENABLE_DRAW_DEBUG
			if (LyraConsoleVariables::DrawBulletHitDuration > 0.0f)
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, LyraConsoleVariables::DrawBulletHitRadius, FColor::Red, false, LyraConsoleVariables::DrawBulletHitRadius);
			}
#endif

			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}

			HitLocation = Impact.ImpactPoint;
		}

		// Make sure there's always an entry in OutHits so the direction can be used for tracers, etc...
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				// Locate the fake 'impact' at the end of the trace
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
}

void ULyraGameplayAbility_RangedWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Bind target data callback
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	// Update the last firing time
	ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void ULyraGameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
		check(WeaponData);
		WeaponData->StopRecoil();
		
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void ULyraGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		if (bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,CurrentActivationInfo.GetActivationPredictionKey(),LocalTargetDataHandle,ApplicationTag,MyAbilityComponent->ScopedPredictionKey);
		}

#if WITH_SERVER_CODE

		if (AController* Controller = GetControllerFromActorInfo(); IsValid(Controller) && Controller->GetLocalRole() == ROLE_Authority)
		{
			ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>();

			// 탄약 등 비용을 먼저 확정한다.
			// 실패하면 히트 검증(SSR) 비용을 쓰지 않고, 클라 히트 마커도 확정하지 않는다.
			if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
			{
				if (IsValid(WeaponStateComponent))
				{
					WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, TArray<FLyraConfirmedHit>());
				}

				K2_EndAbility();
			}
			else
			{
				ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
				check(WeaponData);
				WeaponData->AddSpread();
				WeaponData->StartRecoil();

				ULyraServerSideRewindSubsystem* RewindEvaluator = nullptr;
				if (GetDefault<ULyraLagCompensationSettings>()->bUseLagCompensation)
				{
					UWorld* World = GetWorld();
					RewindEvaluator = IsValid(World) ? World->GetSubsystem<ULyraServerSideRewindSubsystem>() : nullptr;
					ensureMsgf(IsValid(RewindEvaluator), TEXT("ServerSideRewindSubsystem is required in an authority World."));
				}

				// 클라가 보낸 핸들은 '주장'일 뿐이다. 항목마다 검증해서 확인된 것만 서버가 만든 핸들에 담는다.
				FGameplayAbilityTargetDataHandle ConfirmedTargetDataHandle;
				ConfirmedTargetDataHandle.UniqueId = LocalTargetDataHandle.UniqueId;
				TArray<FLyraConfirmedHit> ConfirmedHits;
				const FVector ServerShotOrigin = GetServerShotOrigin();

				// 한 발에서 나올 수 있는 히트 수를 넘는 항목은 검증하지 않는다 (SSR 비용 공격, 같은 히트 반복 방지).
				// 핸들 직렬화가 항목 수를 uint8로 보내므로 인덱스도 uint8 범위로 제한한다.
				const int32 MaxEntries = FMath::Min3(LocalTargetDataHandle.Num(),FMath::Max(1, BulletsPerCartridge) * FMath::Max(1, MaxHitsPerBullet),static_cast<int32>(MAX_uint8));
				UE_CLOG(LocalTargetDataHandle.Num() > MaxEntries, LogLyraAbilitySystem, Warning,
					TEXT("%s: client sent %d target data entries, validating only the first %d."),
					*GetName(), LocalTargetDataHandle.Num(), MaxEntries);

				{
					TRACE_CPUPROFILER_EVENT_SCOPE(ServerRewind_HitValidationLoop);

					TArray<const FHitResult*, TInlineAllocator<16>> SeenHitResults;
					for (int32 Index = 0; Index < MaxEntries; ++Index)
					{
						// 핸들 직렬화는 항목마다 ScriptStruct를 함께 보내므로, 클라가 다른 타입을 넣었을 수 있다.
						const FGameplayAbilityTargetData* TargetData = LocalTargetDataHandle.Get(Index);
						if (TargetData == nullptr || TargetData->GetScriptStruct() != FLyraGameplayAbilityTargetData_SingleTargetHit::StaticStruct())
						{
							UE_LOG(LogLyraAbilitySystem, Warning, TEXT("%s: rejected target data [%d] with unexpected type %s."),
								*GetName(), Index, TargetData ? *GetNameSafe(TargetData->GetScriptStruct()) : TEXT("null"));
							continue;
						}

						const FLyraGameplayAbilityTargetData_SingleTargetHit& ClientHit = static_cast<const FLyraGameplayAbilityTargetData_SingleTargetHit&>(*TargetData);
						const FHitResult& ClientHitResult = ClientHit.HitResult;

						// 같은 히트(대상, 시작점, 명중점)를 반복해 넣은 항목은 한 번만 인정한다.
						// 산탄은 같은 대상을 여러 번 맞히는 게 정상이므로 대상만으로는 거르지 않는다.
						const bool bDuplicate = SeenHitResults.ContainsByPredicate([&ClientHitResult](const FHitResult* Seen)
						{
							return Seen->HitObjectHandle == ClientHitResult.HitObjectHandle
								&& Seen->TraceStart.Equals(ClientHitResult.TraceStart)
								&& Seen->ImpactPoint.Equals(ClientHitResult.ImpactPoint);
						});
						if (bDuplicate)
						{
							UE_LOG(LogLyraAbilitySystem, Verbose, TEXT("%s: rejected duplicate target data [%d]."), *GetName(), Index);
							continue;
						}
						SeenHitResults.Add(&ClientHitResult);

						const FServerHitValidationResult Validation = ValidateClientHitOnServer(ClientHit, RewindEvaluator);
						UE_LOG(LogLyraAbilitySystem, Verbose, TEXT("%s: target data [%d] actor=%s confirmed=%d zone(client=%s, server=%s) bone(client=%s, server=%s)"),
							*GetName(), Index, *GetNameSafe(ClientHitResult.GetActor()), Validation.bConfirmed ? 1 : 0,
							*UPhysicalMaterialWithTags::FindZoneTag(ClientHitResult.PhysMaterial.Get()).ToString(),
							*UPhysicalMaterialWithTags::FindZoneTag(Validation.ServerHit.PhysMaterial.Get()).ToString(),
							*ClientHitResult.BoneName.ToString(), *Validation.ServerHit.BoneName.ToString());
						if (!Validation.bConfirmed)
						{
							continue;
						}

						// 판정은 클라의 주장(대상, 사격 선분, 시각)으로 하고, 이후 처리(데미지 GE, Cue, 히트 마커)는 서버가 만든 HitResult만 쓴다.
						// 클라 항목은 건드리지 않고 복사본의 HitResult를 서버 판정 결과로 교체한다.
						FHitResult ServerHitResult = Validation.ServerHit;
						// 데미지 거리 계산의 Origin(Context.AddHitResult가 TraceStart로 채움)이 서버 위치가 되도록 한다.
						ServerHitResult.TraceStart = ServerShotOrigin;
						ServerHitResult.TraceEnd = ServerHitResult.ImpactPoint;
						ServerHitResult.Distance = FVector::Dist(ServerShotOrigin, ServerHitResult.ImpactPoint);

						FLyraGameplayAbilityTargetData_SingleTargetHit* ServerTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit(ClientHit);
						ServerTargetData->HitResult = ServerHitResult;
						ConfirmedTargetDataHandle.Add(ServerTargetData);

						FLyraConfirmedHit& ConfirmedHit = ConfirmedHits.AddDefaulted_GetRef();
						ConfirmedHit.TargetDataIndex = static_cast<uint8>(Index);
						ConfirmedHit.HitZone = UPhysicalMaterialWithTags::FindZoneTag(ServerHitResult.PhysMaterial.Get());
					}
				}

				// 확인된 항목의 인덱스와 서버 판정 부위를 클라에 알려 해당 히트 마커만 확정한다.
				if (IsValid(WeaponStateComponent))
				{
					WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, ConfirmedHits);
				}

				// 피격 VFX는 클라가 예측하지 않고, 서버 판정 결과로 모든 클라(쏜 사람 포함)에 재생한다.
				ExecuteServerImpactCues(ConfirmedTargetDataHandle);

				// Handle interaction with ILyraWeaponInteractable (확인된 항목만)
				for (int32 Index = 0; Index < ConfirmedTargetDataHandle.Num(); ++Index)
				{
					if (const FLyraGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<const FLyraGameplayAbilityTargetData_SingleTargetHit*>(ConfirmedTargetDataHandle.Get(Index)))
					{
						if (AActor* HitActor = SingleTargetHit->HitResult.GetActor())
						{
							if (ILyraWeaponInteractable* Interactable = Cast<ILyraWeaponInteractable>(HitActor))
							{
								// 순수 가상 함수를 직접 호출합니다.
								Interactable->OnWeaponHit(SingleTargetHit->HitResult, GetAvatarActorFromActorInfo());
							}
						}
					}
				}

				// 확인된 항목만 담긴 핸들로 데미지 GE와 서버 연출을 처리한다.
				// 빈 핸들은 넘기지 않는다: BP가 항목 수를 확인하지 않고 0번 HitResult를 읽으면 원점 기준 Cue가 나갈 수 있다.
				// (기존에도 확인된 히트가 없으면 서버 BP를 호출하지 않았다)
				if (ConfirmedTargetDataHandle.Num() > 0)
				{
					OnRangedWeaponTargetDataReady(ConfirmedTargetDataHandle);
				}
			}
		}
#endif //WITH_SERVER_CODE
		
		if (AController* Controller = GetControllerFromActorInfo(); IsValid(Controller))
		{
			if (Controller->GetLocalRole() != ROLE_Authority)
			{
				if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
				{
					ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
					check(WeaponData);
					WeaponData->AddSpread();
					WeaponData->StartRecoil();
						
					// Let the blueprint do stuff like apply effects to the targets
					OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
				}
				else
				{
					K2_EndAbility();
				}
			}
		}
	}
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

ULyraGameplayAbility_RangedWeapon::FServerHitValidationResult ULyraGameplayAbility_RangedWeapon::ValidateClientHitOnServer(const FLyraGameplayAbilityTargetData_SingleTargetHit& ClientHit, ULyraServerSideRewindSubsystem* RewindEvaluator)
{
	const FHitResult& ClientHitResult = ClientHit.HitResult;
	AActor* TargetActor = ClientHitResult.GetActor();
	if (!IsValid(TargetActor))
	{
		// 빗나간 사격의 트레이서용 항목 (맞은 액터 없음)
		return FServerHitValidationResult();
	}

	// 움직이는 대상(스냅샷 기록 중인 캐릭터)은 클라가 본 시각으로 되감아 판정한다.
	if (IsValid(RewindEvaluator) && IsValid(RewindEvaluator->ResolveSnapShotComponent(TargetActor)))
	{
		FLyraServerSideRewindRequest RewindRequest;
		RewindRequest.TargetActor = TargetActor;
		RewindRequest.InstigatorActor = GetAvatarActorFromActorInfo();
		RewindRequest.TraceStart = ClientHitResult.TraceStart;
		RewindRequest.HitLocation = ClientHitResult.ImpactPoint;
		RewindRequest.HitTime = ClientHit.HitTime;
		const FServerSideRewindResult RewindResult = RewindEvaluator->ServerSideRewind(RewindRequest);

		FServerHitValidationResult Result;
		Result.bConfirmed = RewindResult.bHitConfirmed;
		if (Result.bConfirmed)
		{
			// 되감은 포즈 기준의 서버 HitResult (가장 가까운 바디의 명중점, 법선, 본, 물리 재질)
			Result.ServerHit = RewindResult.ConfirmedHit;
		}
		return Result;
	}

	// 스냅샷이 없는 대상(벽, 상호작용 액터 등)이나 SSR이 꺼진 경우는 현재 시각 기준으로 판정한다.
	return ValidateHitAtCurrentTime(ClientHitResult);
}

ULyraGameplayAbility_RangedWeapon::FServerHitValidationResult ULyraGameplayAbility_RangedWeapon::ValidateHitAtCurrentTime(const FHitResult& ClientHitResult) const
{
	FServerHitValidationResult Result;

	const AActor* ClientHitActor = ClientHitResult.GetActor();
	UWorld* World = GetWorld();
	if (!IsValid(ClientHitActor) || !IsValid(World))
	{
		return Result;
	}

	// 클라 트레이스(WeaponTrace)와 같은 조건: 복잡 충돌, 자기 자신과 부착 액터 무시, 같은 무기 채널, 물리 재질 반환(부위 판정용)
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(LyraServerHitValidation), /*bTraceComplex=*/ true, GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, /*bIsSimulated=*/ false, /*bTraceForServerSideRewind=*/ false);

	// 클라 명중점보다 조금 더 길게 트레이스한다.
	const FVector TraceStart = ClientHitResult.TraceStart;
	const FVector TraceEnd = TraceStart + (FVector(ClientHitResult.Location) - TraceStart) * 1.25f;

	FHitResult ServerHitResult;
	bool bConfirmed = World->LineTraceSingleByChannel(ServerHitResult, TraceStart, TraceEnd, TraceChannel, TraceParams)
		&& ServerHitResult.GetActor() == ClientHitActor;

	// 클라는 라인으로 못 맞히면 Sweep으로 다시 찾으므로(DoSingleBulletTrace), 같은 반지름으로 한 번 더 확인한다.
	if (!bConfirmed)
	{
		const ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
		const float SweepRadius = WeaponData ? WeaponData->GetBulletTraceSweepRadius() : 0.0f;
		if (SweepRadius > 0.0f)
		{
			ServerHitResult = FHitResult();
			bConfirmed = World->SweepSingleByChannel(ServerHitResult, TraceStart, TraceEnd, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams)
				&& ServerHitResult.GetActor() == ClientHitActor;
		}
	}

	if (bConfirmed)
	{
		// 서버 트레이스 결과를 그대로 서버 HitResult로 쓴다. (부위는 맞은 바디의 물리 재질 태그)
		Result.bConfirmed = true;
		Result.ServerHit = ServerHitResult;
	}
	return Result;
}

FVector ULyraGameplayAbility_RangedWeapon::GetServerShotOrigin() const
{
	// 서버 권위 위치인 폰 시점(액터 위치 + BaseEyeHeight)을 쓴다.
	// 카메라 위치(GetPlayerViewPoint)는 원격 플레이어의 경우 클라가 보내 준 값이라 쓰지 않는다.
	if (const APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		return AvatarPawn->GetPawnViewLocation();
	}
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	return IsValid(AvatarActor) ? AvatarActor->GetActorLocation() : FVector::ZeroVector;
}

void ULyraGameplayAbility_RangedWeapon::ExecuteServerImpactCues(const FGameplayAbilityTargetDataHandle& ConfirmedTargetDataHandle)
{
	if (!ImpactCueTag.IsValid() || ConfirmedTargetDataHandle.Num() == 0)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	// 이 함수는 서버가 타깃 데이터를 처리하는 동안, 즉 쏜 사람 클라의 예측 키가 걸린 범위 안에서 불린다.
	// 그 키를 단 채 Cue를 실행하면 GAS가 "예측한 클라는 이미 재생했다"고 보고 쏜 사람에게는 재생하지 않는다
	// (UAbilitySystemComponent::NetMulticast_InvokeGameplayCueExecuted_WithParams의 IsLocalClientKey 검사).
	// 클라는 Impact Cue를 예측하지 않으므로, 예측 키를 비워 쏜 사람을 포함한 모든 클라가 재생하게 한다.
	FScopedPredictionWindow NoPredictionWindow(AbilitySystemComponent, FPredictionKey(), /*InSetReplicatedPredictionKey=*/ false);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	for (int32 Index = 0; Index < ConfirmedTargetDataHandle.Num(); ++Index)
	{
		if (const FLyraGameplayAbilityTargetData_SingleTargetHit* ServerTargetData = static_cast<const FLyraGameplayAbilityTargetData_SingleTargetHit*>(ConfirmedTargetDataHandle.Get(Index)))
		{
			// 위치, 법선, 물리 재질(Surface Type)은 서버 HitResult(되감은 포즈 기준 월드 좌표)에서 온다.
			FGameplayCueParameters CueParameters = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(ServerTargetData->HitResult);
			CueParameters.Instigator = AvatarActor;
			CueParameters.EffectCauser = AvatarActor;
			CueParameters.SourceObject = GetWeaponInstance();
			CueParameters.AbilityLevel = GetAbilityLevel();
			AbilitySystemComponent->ExecuteGameplayCue(ImpactCueTag, CueParameters);
		}
	}
}

void ULyraGameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
	ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>();
	
	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);
	
	// Fill out the target data from the hit results
	FGameplayAbilityTargetDataHandle TargetData;
	TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

	if (FoundHits.Num() > 0)
	{
		const int32 CartridgeID = FMath::Rand();
		
		for (const FHitResult& FoundHit : FoundHits)
		{
			FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;
			NewTargetData->CartridgeID = CartridgeID;
			
			if (GetDefault<ULyraLagCompensationSettings>()->bUseLagCompensation)
			{
				ALyraPlayerController* LyraPC = GetLyraPlayerControllerFromActorInfo();
				if (IsValid(LyraPC))
				{
					ULyraTimeSyncComponent* TimeSyncComponent = LyraPC->GetComponentByClass<ULyraTimeSyncComponent>();
					if (IsValid(TimeSyncComponent))
					{
						const float CurrentServerTime = TimeSyncComponent->GetServerTime();
						const float SingleTripTime = TimeSyncComponent->GetSingleTripTime();
						const float HitTime = CurrentServerTime - SingleTripTime - InterpDelay;
						NewTargetData->HitTime = HitTime;
					}
					else
					{
						const float HitTime = GetWorld()->GetTimeSeconds();
						NewTargetData->HitTime = HitTime;
					}
#if ENABLE_DRAW_DEBUG
					if (GetDefault<ULyraLagCompensationDeveloperSettings>()->bDrawMeshLagCompensation)
					{
						AActor* TargetActor = FoundHit.GetActor();
						ALyraCharacter* TargetCharacter = Cast<ALyraCharacter>(TargetActor);
						ULyraSnapShotComponent* SnapShotComponent = IsValid(TargetCharacter) ? TargetCharacter->GetComponentByClass<ULyraSnapShotComponent>() : nullptr;
						if (IsValid(SnapShotComponent))
						{
							SnapShotComponent->DrawCurrentPhysicsAssetPose(TargetCharacter, FColor::White);
						}
					}
#endif
				}
			}

			TargetData.Add(NewTargetData);
		}
	}
	
	// Send hit marker information
	if (IsValid(WeaponStateComponent))
	{
		WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
	}
	
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}
