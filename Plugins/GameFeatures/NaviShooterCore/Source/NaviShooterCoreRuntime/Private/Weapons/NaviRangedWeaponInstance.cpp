// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NaviRangedWeaponInstance.h"
#include "NativeGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/LyraCameraComponent.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Weapons/LyraWeaponInstance.h"
#include "Weapons/NaviWeaponStatDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NaviRangedWeaponInstance)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Weapon_SteadyAimingCamera, "Lyra.Weapon.SteadyAimingCamera");
UNaviRangedWeaponInstance::UNaviRangedWeaponInstance(const FObjectInitializer& ObjectInitializer) 	
	: Super(ObjectInitializer)
{
	HeatToHeatPerShotCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	HeatToCoolDownPerSecondCurve.EditorCurveData.AddKey(0.0f, 2.0f);
}

void UNaviRangedWeaponInstance::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

#if WITH_EDITOR
void UNaviRangedWeaponInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateDebugVisualization();
}

void UNaviRangedWeaponInstance::UpdateDebugVisualization()
{
	ComputeHeatRange(/*out*/ Debug_MinHeat, /*out*/ Debug_MaxHeat);
	ComputeSpreadRange(/*out*/ Debug_MinSpreadAngle, /*out*/ Debug_MaxSpreadAngle);
	Debug_CurrentHeat = CurrentHeat;
	Debug_CurrentSpreadAngle = CurrentSpreadAngle;
	Debug_CurrentSpreadAngleMultiplier = CurrentSpreadAngleMultiplier;
}

#endif

const FNaviWeaponStatDefinition& UNaviRangedWeaponInstance::GetNaviWeaponStats()
{
	if (!CachedNaviWeaponStats && !WeaponStatRow.IsNull())
	{
		CachedNaviWeaponStats = WeaponStatRow.GetRow<FNaviWeaponStatDefinition>(TEXT("LyraRangedWeaponInstance_GetStats"));
	}
	return *CachedNaviWeaponStats;
}

void UNaviRangedWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	// Start heat in the middle
	float MinHeatRange;
	float MaxHeatRange;
	ComputeHeatRange(/*out*/ MinHeatRange, /*out*/ MaxHeatRange);
	CurrentHeat = (MinHeatRange + MaxHeatRange) * 0.5f;

	// Derive spread
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

	// Default the multipliers to 1x
	CurrentSpreadAngleMultiplier = 1.0f;
	StandingStillMultiplier = 1.0f;
	JumpFallMultiplier = 1.0f;
	CrouchingMultiplier = 1.0f;
}

void UNaviRangedWeaponInstance::OnUnequipped()
{
	Super::OnUnequipped();
}

void UNaviRangedWeaponInstance::Tick(float DeltaSeconds)
{
	APawn* Pawn = GetPawn();
	check(Pawn != nullptr);
	
	const bool bMinSpread = UpdateSpread(DeltaSeconds);
	const bool bMinMultipliers = UpdateMultipliers(DeltaSeconds);

	bHasFirstShotAccuracy = bAllowFirstShotAccuracy && bMinMultipliers && bMinSpread;

	UpdateRecoil(DeltaSeconds);

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

void UNaviRangedWeaponInstance::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
	float Min1;
	float Max1;
	HeatToHeatPerShotCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min1, /*out*/ Max1);

	float Min2;
	float Max2;
	HeatToCoolDownPerSecondCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min2, /*out*/ Max2);

	float Min3;
	float Max3;
	HeatToSpreadCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min3, /*out*/ Max3);

	MinHeat = FMath::Min(FMath::Min(Min1, Min2), Min3);
	MaxHeat = FMath::Max(FMath::Max(Max1, Max2), Max3);
}

void UNaviRangedWeaponInstance::ComputeSpreadRange(float& MinSpread, float& MaxSpread)
{
	HeatToSpreadCurve.GetRichCurveConst()->GetValueRange(/*out*/ MinSpread, /*out*/ MaxSpread);
}

void UNaviRangedWeaponInstance::AddSpread()
{
	// Sample the heat up curve
	const float HeatPerShot = HeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentHeat);
	CurrentHeat = ClampHeat(CurrentHeat + HeatPerShot);

	// Map the heat to the spread angle
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

float UNaviRangedWeaponInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	const FRichCurve* Curve = DistanceDamageFalloff.GetRichCurveConst();
	return Curve->HasAnyData() ? Curve->Eval(Distance) : 1.0f;
}

float UNaviRangedWeaponInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	float CombinedMultiplier = 1.0f;
	if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
	{
		for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
		{
			if (const float* pTagMultiplier = MaterialDamageMultiplier.Find(MaterialTag))
			{
				CombinedMultiplier *= *pTagMultiplier;
			}
		}
	}

	return CombinedMultiplier;
}

bool UNaviRangedWeaponInstance::UpdateSpread(float DeltaSeconds)
{
	// const float TimeSinceFired = GetWorld()->TimeSince(LastFireTime);
	const float TimeSinceFired = GetWorld()->TimeSince(TimeLastFired);
	
	if (TimeSinceFired > SpreadRecoveryCooldownDelay)
	{
		const float CooldownRate = HeatToCoolDownPerSecondCurve.GetRichCurveConst()->Eval(CurrentHeat);
		CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * DeltaSeconds));
		CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);
	}
	
	float MinSpread;
	float MaxSpread;
	ComputeSpreadRange(/*out*/ MinSpread, /*out*/ MaxSpread);

	return FMath::IsNearlyEqual(CurrentSpreadAngle, MinSpread, KINDA_SMALL_NUMBER);
}

bool UNaviRangedWeaponInstance::UpdateMultipliers(float DeltaSeconds)
{
	const float MultiplierNearlyEqualThreshold = 0.05f;

	APawn* Pawn = GetPawn();
	check(Pawn != nullptr);
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());

	// See if we are standing still, and if so, smoothly apply the bonus
	const float PawnSpeed = Pawn->GetVelocity().Size();
	const float MovementTargetValue = FMath::GetMappedRangeValueClamped(
		/*InputRange=*/ FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
		/*OutputRange=*/ FVector2D(SpreadAngleMultiplier_StandingStill, 1.0f),
		/*Alpha=*/ PawnSpeed);
	StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementTargetValue, DeltaSeconds, TransitionRate_StandingStill);
	const bool bStandingStillMultiplierAtMin = FMath::IsNearlyEqual(StandingStillMultiplier, SpreadAngleMultiplier_StandingStill, SpreadAngleMultiplier_StandingStill*0.1f);

	// See if we are crouching, and if so, smoothly apply the bonus
	const bool bIsCrouching = (CharMovementComp != nullptr) && CharMovementComp->IsCrouching();
	const float CrouchingTargetValue = bIsCrouching ? SpreadAngleMultiplier_Crouching : 1.0f;
	CrouchingMultiplier = FMath::FInterpTo(CrouchingMultiplier, CrouchingTargetValue, DeltaSeconds, TransitionRate_Crouching);
	const bool bCrouchingMultiplierAtTarget = FMath::IsNearlyEqual(CrouchingMultiplier, CrouchingTargetValue, MultiplierNearlyEqualThreshold);

	// See if we are in the air (jumping/falling), and if so, smoothly apply the penalty
	const bool bIsJumpingOrFalling = (CharMovementComp != nullptr) && CharMovementComp->IsFalling();
	const float JumpFallTargetValue = bIsJumpingOrFalling ? SpreadAngleMultiplier_JumpingOrFalling : 1.0f;
	JumpFallMultiplier = FMath::FInterpTo(JumpFallMultiplier, JumpFallTargetValue, DeltaSeconds, TransitionRate_JumpingOrFalling);
	const bool bJumpFallMultiplerIs1 = FMath::IsNearlyEqual(JumpFallMultiplier, 1.0f, MultiplierNearlyEqualThreshold);

	// Determine if we are aiming down sights, and apply the bonus based on how far into the camera transition we are
	float AimingAlpha = 0.0f;
	if (const ULyraCameraComponent* CameraComponent = ULyraCameraComponent::FindCameraComponent(Pawn))
	{
		float TopCameraWeight;
		FGameplayTag TopCameraTag;
		CameraComponent->GetBlendInfo(/*out*/ TopCameraWeight, /*out*/ TopCameraTag);

		AimingAlpha = (TopCameraTag == TAG_Lyra_Weapon_SteadyAimingCamera) ? TopCameraWeight : 0.0f;
	}
	const float AimingMultiplier = FMath::GetMappedRangeValueClamped(
		/*InputRange=*/ FVector2D(0.0f, 1.0f),
		/*OutputRange=*/ FVector2D(1.0f, SpreadAngleMultiplier_Aiming),
		/*Alpha=*/ AimingAlpha);
	const bool bAimingMultiplierAtTarget = FMath::IsNearlyEqual(AimingMultiplier, SpreadAngleMultiplier_Aiming, KINDA_SMALL_NUMBER);

	// Combine all the multipliers
	const float CombinedMultiplier = AimingMultiplier * StandingStillMultiplier * CrouchingMultiplier * JumpFallMultiplier;
	CurrentSpreadAngleMultiplier = CombinedMultiplier;

	// need to handle these spread multipliers indicating we are not at min spread
	return bStandingStillMultiplierAtMin && bCrouchingMultiplierAtTarget && bJumpFallMultiplerIs1 && bAimingMultiplierAtTarget;
}

void UNaviRangedWeaponInstance::StartRecoil()
{
	APawn* const Pawn = GetPawn();
	if (Pawn == nullptr) return;
	
	bIsFiring = true;
	
	TargetRecoil.X += FMath::FRandRange(HorizontalRecoilStep.X, HorizontalRecoilStep.Y);
	TargetRecoil.Y += FMath::FRandRange(VerticalRecoilStep.X, VerticalRecoilStep.Y);

	TargetRecoil.X = FMath::Clamp(TargetRecoil.X, -90.f, 90.f);
	TargetRecoil.Y = FMath::Clamp(TargetRecoil.Y, -90.f, 90.f);
}

void UNaviRangedWeaponInstance::StopRecoil()
{
	if (!bIsFiring) return;
    
	bIsFiring = false;
	ApplyInputCompensation();
}

void UNaviRangedWeaponInstance::UpdateRecoil(float DeltaTime)
{
	APawn* const Pawn = GetPawn();
	if (Pawn == nullptr) return;
	
	if (bIsFiring)
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FVector2D MouseDelta;
			PC->GetInputMouseDelta(MouseDelta.X, MouseDelta.Y);
			InputCompensation += MouseDelta;
		}
	}
	else
	{
		TargetRecoil = FMath::Vector2DInterpTo(TargetRecoil, FVector2D::ZeroVector, DeltaTime, Damping);
	}

	const FVector2D PreviousRecoil = CurrentRecoil;

	CurrentRecoil.X = FMath::FInterpTo(CurrentRecoil.X, TargetRecoil.X, DeltaTime, RecoilSmoothing.X);
	CurrentRecoil.Y = FMath::FInterpTo(CurrentRecoil.Y, TargetRecoil.Y, DeltaTime, RecoilSmoothing.Y);
    
	Pawn->AddControllerYawInput(CurrentRecoil.X - PreviousRecoil.X);
	Pawn->AddControllerPitchInput(CurrentRecoil.Y - PreviousRecoil.Y);
}

void UNaviRangedWeaponInstance::ApplyInputCompensation()
{
	const float CompWeight = Compensation;
	const float YawMultiplier = GetInputCompensationMultiplier(CurrentRecoil.X, InputCompensation.X * CompWeight);
	const float PitchMultiplier = GetInputCompensationMultiplier(CurrentRecoil.Y, -InputCompensation.Y * CompWeight);

	CurrentRecoil.X *= YawMultiplier;
	CurrentRecoil.Y *= PitchMultiplier;

	TargetRecoil = CurrentRecoil;
	InputCompensation = FVector2D::ZeroVector;
}

float UNaviRangedWeaponInstance::GetInputCompensationMultiplier(float RecoilValue, float CompensationValue) const
{
	const bool bIsOpposite = (RecoilValue * CompensationValue) <= 0.f;

	if (!FMath::IsNearlyZero(RecoilValue) && bIsOpposite)
	{
		const float Reduction = FMath::Clamp(FMath::Abs(CompensationValue / RecoilValue), 0.f, 1.f);
		return 1.f - Reduction;
	}
	return 1.f;
}