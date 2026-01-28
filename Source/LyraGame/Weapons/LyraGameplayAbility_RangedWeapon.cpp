// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayAbility_RangedWeapon.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "Physics/LyraCollisionChannels.h"
#include "LyraLogChannels.h"
#include "AIController.h"
#include "NativeGameplayTags.h"
#include "Weapons/LyraWeaponStateComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "DrawDebugHelpers.h"

#include "TimerManager.h"
#include "Character/LyraCharacter.h"
#include "LagCompensation/LyraLagCompensationComponent.h"
#include "LagCompensation/LyraTimeSyncComponent.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_RangedWeapon)

namespace LyraConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuraton(
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
		TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
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
		return LagCompensation_TraceChannel_HitBox;
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

	if (BulletsPerCartridge == -1)
	{
		BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();
	}
	
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

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;
		
		bool bProjectileWeapon = false;

#if WITH_SERVER_CODE
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority)
				{
					bool bOverallHitSuccess = false; // 이번 발사가 최소 1회 유효한 Hit가 발생했는지
					TArray<uint8> HitReplaces; // 대체된 Hit 배열

					if (bUseServerSideRewind)
					{
						ALyraCharacter* AttackerCharacter = GetLyraCharacterFromActorInfo();
						ULyraLagCompensationComponent* LagComp = (IsValid(AttackerCharacter)) ? AttackerCharacter->GetComponentByClass<ULyraLagCompensationComponent>() : nullptr;
						if (IsValid(LagComp))
						{
						for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
							{
								if (FLyraGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FLyraGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
								{
									const float HitTime = SingleTargetHit->HitTime;
									const FHitResult& ClientHitResult = SingleTargetHit->HitResult;
									
									// 서버 사이드 리와인드 실행
									FServerSideRewindResult RewindResult =
										LagComp->ServerSideRewind(ClientHitResult.GetActor(), ClientHitResult.TraceStart, ClientHitResult.ImpactPoint, HitTime);

									UE_LOG(LogTemp, Warning, TEXT("  Hit[%d]: Actor=%s, bConfirmed=%d, bHeadshot=%d"),
									       i,
									       ClientHitResult.GetActor() ? *ClientHitResult.GetActor()->GetName() : TEXT("NULL"),
									       RewindResult.bHitConfirmed,
									       RewindResult.bHeadShot);

									// @Todo: 서버 권위적인 로직을 통해 Hit Replaced 시스템을 사용
									if (SingleTargetHit->bHitReplaced)
									{
										HitReplaces.Add(i);
									}

									if (RewindResult.bHitConfirmed)
									{
										// 동일하다면, 이 피격은 유효하다고 간주
										bOverallHitSuccess = true;
									}
								}
							}
						}
					}
					else
					{
						// 클라이언트가 보낸 모든 피격 데이터를 하나씩 검증
						for (uint8 i = 0; i < LocalTargetDataHandle.Num(); ++i)
						{
							if (const FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
							{
								const FHitResult& ClientHitResult = SingleTargetHit->HitResult;
								AActor* ClientHitActor = ClientHitResult.GetActor();

								if (ClientHitActor)
								{
									// 서버가 '현재 시간' 기준으로 직접 트레이스를 수행
									FHitResult ServerHitResult;
									FCollisionQueryParams TraceParams;
									const FVector TraceEnd = ClientHitResult.TraceStart + (ClientHitResult.Location - ClientHitResult.TraceStart) * 1.25f; // 클라이언트가 쏜 위치보다 약간 더 길게 트레이스 설정
									TraceParams.AddIgnoredActor(GetAvatarActorFromActorInfo()); // 자기 자신 외에 피격 대상의 캡슐도 무시하고 히트박스만 보도록로 검사
									GetWorld()->LineTraceSingleByChannel(ServerHitResult, ClientHitResult.TraceStart, TraceEnd, LagCompensation_TraceChannel_HitBox, TraceParams);
									
									// 서버의 트레이스 결과와 클라이언트가 맞췄다고 주장하는 액터가 동일한지 확인
									if (ServerHitResult.bBlockingHit && (ServerHitResult.GetActor() == ClientHitActor))
									{
										// 동일하다면, 이 피격은 유효하다고 간주
										bOverallHitSuccess = true;
									}

									// @Todo: 서버 권위적인 로직을 통해 Hit Replaced 시스템을 사용
									if (SingleTargetHit->bHitReplaced)
									{
										HitReplaces.Add(i);
									}
								}
							}
						}
					}

					// 검증 결과를 바탕으로 클라이언트에 통보

					if (ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>())
					{
						WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bOverallHitSuccess, HitReplaces);
					}
					
					// 서버 사이드 리와인드 Hit 결과 확인. Ammo 확인. 
					if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
					{
						ULyraRangedWeaponInstance* WeaponData = GetWeaponInstance();
						check(WeaponData);
						WeaponData->AddSpread();
						WeaponData->StartRecoil();
						if (bOverallHitSuccess)
						{
							OnRangedWeaponTargetDataReady(LocalTargetDataHandle); 
						}
					}
					else
					{
						K2_EndAbility();
					}
				}
				
			}
		}
#endif //WITH_SERVER_CODE
		
		if (AController* Controller = GetControllerFromActorInfo(); IsValid(Controller))
		{
			if (Controller->GetLocalRole() != ROLE_Authority)
			{
				if ( CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
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

	UE_LOG(LogTemp, Warning, TEXT("[HitMarker Step 1] StartRangedWeaponTargeting - WeaponStateComponent=%s"),
	       WeaponStateComponent ? TEXT("VALID") : TEXT("NULL"));

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);

	UE_LOG(LogTemp, Warning, TEXT("[HitMarker Step 2] PerformLocalTargeting - FoundHits.Num()=%d"), FoundHits.Num());
	for (int i = 0; i < FoundHits.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Hit[%d]: Actor=%s, Location=%s"),
		       i,
		       FoundHits[i].GetActor() ? *FoundHits[i].GetActor()->GetName() : TEXT("NULL"),
		       *FoundHits[i].Location.ToString());
	}

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
			
			if (bUseServerSideRewind)
			{
				ALyraPlayerController* LyraPC = GetLyraPlayerControllerFromActorInfo();
				if (IsValid(LyraPC))
				{
					ULyraTimeSyncComponent* TimeSyncComponent = LyraPC->GetComponentByClass<ULyraTimeSyncComponent>();
					const float HitTime = (IsValid(TimeSyncComponent)) ? (TimeSyncComponent->GetServerTime() - TimeSyncComponent->GetSingleTripTime()) : GetWorld()->GetTimeSeconds();
					NewTargetData->HitTime = HitTime;
				}
			}

			TargetData.Add(NewTargetData);
		}
	}

	// Send hit marker information
	const bool bProjectileWeapon = false;
	if (!bProjectileWeapon && (WeaponStateComponent != nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker Step 3] AddUnconfirmedServerSideHitMarkers - UniqueId=%d"), TargetData.UniqueId);
		WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[HitMarker Step 3 FAILED] bProjectileWeapon=%d, WeaponStateComponent=%s"),
		       bProjectileWeapon, WeaponStateComponent ? TEXT("VALID") : TEXT("NULL"));
	}

	// Process the target data immediately
	UE_LOG(LogTemp, Warning, TEXT("[HitMarker Step 4] OnTargetDataReadyCallback (Client Prediction)"));
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}
