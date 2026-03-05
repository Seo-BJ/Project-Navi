// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraRangedWeaponInstance.h"
#include "NativeGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/LyraCameraComponent.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Weapons/LyraWeaponInstance.h"
#include "Weapons/NaviWeaponStatDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraRangedWeaponInstance)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Weapon_SteadyAimingCamera, "Lyra.Weapon.SteadyAimingCamera");

ULyraRangedWeaponInstance::ULyraRangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 기본값: 열 0에서 발사 시 열 1 추가, 초당 열 2 냉각
	HeatToHeatPerShotCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	HeatToCoolDownPerSecondCurve.EditorCurveData.AddKey(0.0f, 2.0f);
}

void ULyraRangedWeaponInstance::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

#if WITH_EDITOR
void ULyraRangedWeaponInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateDebugVisualization();
}

void ULyraRangedWeaponInstance::UpdateDebugVisualization()
{
	ComputeHeatRange(/*out*/ Debug_MinHeat, /*out*/ Debug_MaxHeat);
	ComputeSpreadRange(/*out*/ Debug_MinSpreadAngle, /*out*/ Debug_MaxSpreadAngle);
	Debug_CurrentHeat = CurrentHeat;
	Debug_CurrentSpreadAngle = CurrentSpreadAngle;
	Debug_CurrentSpreadAngleMultiplier = CurrentSpreadAngleMultiplier;
}

#endif

FNaviWeaponStatDefinition ULyraRangedWeaponInstance::GetNaviWeaponStats()
{
	if (!CachedNaviWeaponStats && !WeaponStatRow.IsNull())
	{
		CachedNaviWeaponStats = WeaponStatRow.GetRow<FNaviWeaponStatDefinition>(TEXT("LyraRangedWeaponInstance_GetStats"));
	}
	return *CachedNaviWeaponStats;
}

void ULyraRangedWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	// 장착 시 열(Heat)을 최소~최대 범위의 중간값으로 초기화
	// (장착 즉시 최소 탄퍼짐이 아닌 자연스러운 초기 상태를 만들기 위함)
	float MinHeatRange;
	float MaxHeatRange;
	ComputeHeatRange(/*out*/ MinHeatRange, /*out*/ MaxHeatRange);
	CurrentHeat = (MinHeatRange + MaxHeatRange) * 0.5f;

	// 초기 열 값으로부터 탄퍼짐 각도를 계산
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

	// 모든 탄퍼짐 승수를 기본값 1.0으로 초기화
	CurrentSpreadAngleMultiplier = 1.0f;
	StandingStillMultiplier = 1.0f;
	JumpFallMultiplier = 1.0f;
	CrouchingMultiplier = 1.0f;

	// 무기 스탯 데이터를 기반으로 캐릭터의 이동 속도를 무기별 달리기 속도로 설정
	APawn* Pawn = GetPawn();
	check(Pawn != nullptr);
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	check(CharMovementComp != nullptr);
	const FNaviWeaponStatDefinition StatDefinition = GetNaviWeaponStats();

	DefaultMaxWalkSpeed = CharMovementComp->MaxWalkSpeed;
	CharMovementComp->MaxWalkSpeed = StatDefinition.RunSpeed * 100.0f; // cm/s 단위 변환 (m/s → cm/s)
}

void ULyraRangedWeaponInstance::OnUnequipped()
{
	Super::OnUnequipped();
}

void ULyraRangedWeaponInstance::Tick(float DeltaSeconds)
{
	APawn* Pawn = GetPawn();
	check(Pawn != nullptr);

	// 탄퍼짐(Heat) 및 승수 업데이트 후, 두 조건이 모두 최솟값이면 초탄 정확도 활성화
	const bool bMinSpread = UpdateSpread(DeltaSeconds);
	const bool bMinMultipliers = UpdateMultipliers(DeltaSeconds);

	// bAllowFirstShotAccuracy가 허용되고, 탄퍼짐과 승수가 모두 최솟값일 때만 초탄 정확도 적용
	bHasFirstShotAccuracy = bAllowFirstShotAccuracy && bMinMultipliers && bMinSpread;

	// 반동 보간 및 캐릭터 컨트롤러 회전 적용
	UpdateRecoil(DeltaSeconds);

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

void ULyraRangedWeaponInstance::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
	// 세 커브(발사 시 열 증가, 냉각 속도, 탄퍼짐)의 시간 축(X축) 범위를 각각 구한 뒤,
	// 세 커브를 아우르는 전체 최솟값/최댓값을 열 범위로 사용합니다.
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

void ULyraRangedWeaponInstance::ComputeSpreadRange(float& MinSpread, float& MaxSpread)
{
	HeatToSpreadCurve.GetRichCurveConst()->GetValueRange(/*out*/ MinSpread, /*out*/ MaxSpread);
}

void ULyraRangedWeaponInstance::AddSpread()
{
	// 현재 열(Heat)에 따라 한 발 사격 시 추가되는 열량을 커브에서 샘플링
	const float HeatPerShot = HeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentHeat);
	// 열을 증가시키되 허용 범위 내로 클램핑
	CurrentHeat = ClampHeat(CurrentHeat + HeatPerShot);

	// 갱신된 열 값으로 현재 탄퍼짐 각도를 재계산
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

#if WITH_EDITOR
	UpdateDebugVisualization();
#endif
}

float ULyraRangedWeaponInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	// 언리얼 엔진은 거리를 cm 단위로 전달하므로 m 단위로 변환
	float LocalDistance = Distance / 100.0f;
	if (CachedNaviWeaponStats)
	{
		// DamageFalloffs 배열을 순회하여 현재 거리가 속하는 구간의 데미지 반환
		for (const FDamageFalloff& DamageFalloff : CachedNaviWeaponStats->DamageFalloffs)
		{
			float Start = DamageFalloff.RangeStart;
			float End = DamageFalloff.RangeEnd;
			if (Start <= LocalDistance && LocalDistance < End)
			{
				UE_LOG(LogTemp, Warning, TEXT("LocalDistance : %f, Damage : %f"), LocalDistance,  DamageFalloff.BodyShotDamage);
				return DamageFalloff.BodyShotDamage;
			}
		}
		// 모든 구간을 벗어난 경우 마지막 구간의 데미지(최대 사거리 이후 감쇠값) 반환
		UE_LOG(LogTemp, Warning, TEXT("50+ LocalDistance : %f, Damage : %f"), LocalDistance, CachedNaviWeaponStats->DamageFalloffs.Last().BodyShotDamage);
		return CachedNaviWeaponStats->DamageFalloffs.Last().BodyShotDamage;
	}
	return -1.0f;
}

float ULyraRangedWeaponInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	float CombinedMultiplier = 1.0f;
	if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
	{
		// 피격 물리 재질의 태그들을 순회하여, 무기에 등록된 MaterialDamageMultiplier와 일치하는 태그가 있으면
		// 해당 배율을 누적 곱셈 (태그가 여럿일 경우 배율이 중첩 적용됨)
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

bool ULyraRangedWeaponInstance::UpdateSpread(float DeltaSeconds)
{
	// 마지막 발사 이후 경과 시간이 쿨다운 지연(SpreadRecoveryCooldownDelay)을 초과한 경우에만 냉각 시작
	const float TimeSinceFired = GetWorld()->TimeSince(TimeLastFired);

	if (TimeSinceFired > SpreadRecoveryCooldownDelay)
	{
		// 현재 열에 대응하는 초당 냉각량을 커브에서 샘플링하여 열 감소 적용
		const float CooldownRate = HeatToCoolDownPerSecondCurve.GetRichCurveConst()->Eval(CurrentHeat);
		CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * DeltaSeconds));
		CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);
	}

	float MinSpread;
	float MaxSpread;
	ComputeSpreadRange(/*out*/ MinSpread, /*out*/ MaxSpread);

	// 탄퍼짐이 최솟값에 근접하면 true 반환 (초탄 정확도 조건 판단에 사용)
	return FMath::IsNearlyEqual(CurrentSpreadAngle, MinSpread, KINDA_SMALL_NUMBER);
}

bool ULyraRangedWeaponInstance::UpdateMultipliers(float DeltaSeconds)
{
	// 승수 비교 시 사용하는 허용 오차 (0.05 이내면 목표에 도달한 것으로 간주)
	const float MultiplierNearlyEqualThreshold = 0.05f;

	APawn* Pawn = GetPawn();
	check(Pawn != nullptr);
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	check(CharMovementComp != nullptr);

	// [정지 상태 승수]
	// 폰의 속도를 [StandingStillSpeedThreshold, +StandingStillToMovingSpeedRange] 구간에 매핑하여
	// 정지 시 SpreadAngleMultiplier_StandingStill, 이동 시 1.0 사이를 보간한 목표값 계산
	const float PawnSpeed = Pawn->GetVelocity().Size();
	const float MovementTargetValue = FMath::GetMappedRangeValueClamped(
		/*InputRange=*/ FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
		/*OutputRange=*/ FVector2D(SpreadAngleMultiplier_StandingStill, 1.0f),
		/*Alpha=*/ PawnSpeed);
	StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementTargetValue, DeltaSeconds, TransitionRate_StandingStill);
	// 현재 승수가 목표(정지 상태 승수) 10% 오차 범위 내에 있으면 최솟값 도달로 판단
	const bool bStandingStillMultiplierAtMin = FMath::IsNearlyEqual(StandingStillMultiplier, SpreadAngleMultiplier_StandingStill, SpreadAngleMultiplier_StandingStill*0.1f);

	// [웅크리기 승수]
	// 웅크린 상태이면 SpreadAngleMultiplier_Crouching, 아니면 1.0을 목표로 부드럽게 보간
	const bool bIsCrouching = (CharMovementComp != nullptr) && CharMovementComp->IsCrouching();
	const float CrouchingTargetValue = bIsCrouching ? SpreadAngleMultiplier_Crouching : 1.0f;
	CrouchingMultiplier = FMath::FInterpTo(CrouchingMultiplier, CrouchingTargetValue, DeltaSeconds, TransitionRate_Crouching);
	const bool bCrouchingMultiplierAtTarget = FMath::IsNearlyEqual(CrouchingMultiplier, CrouchingTargetValue, MultiplierNearlyEqualThreshold);

	// [점프/낙하 승수]
	// 공중에 있으면 SpreadAngleMultiplier_JumpingOrFalling(패널티), 착지 시 1.0으로 복귀
	const bool bIsJumpingOrFalling = (CharMovementComp != nullptr) && CharMovementComp->IsFalling();
	const float JumpFallTargetValue = bIsJumpingOrFalling ? SpreadAngleMultiplier_JumpingOrFalling : 1.0f;
	JumpFallMultiplier = FMath::FInterpTo(JumpFallMultiplier, JumpFallTargetValue, DeltaSeconds, TransitionRate_JumpingOrFalling);
	// 점프/낙하가 없을 때 1.0에 근접하면 정상 상태로 판단
	const bool bJumpFallMultiplerIs1 = FMath::IsNearlyEqual(JumpFallMultiplier, 1.0f, MultiplierNearlyEqualThreshold);

	// [조준(ADS) 승수]
	// SteadyAimingCamera 블렌드 가중치를 기반으로 1.0 ~ SpreadAngleMultiplier_Aiming 사이를 매핑
	// (카메라 전환이 완료될수록 조준 보너스가 완전히 적용됨)
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

	// 네 가지 승수를 곱하여 최종 탄퍼짐 승수 산출
	const float CombinedMultiplier = AimingMultiplier * StandingStillMultiplier * CrouchingMultiplier * JumpFallMultiplier;
	CurrentSpreadAngleMultiplier = CombinedMultiplier;

	// 모든 승수가 목표(최솟값)에 도달했을 때만 true 반환 → 초탄 정확도 조건에 사용
	return bStandingStillMultiplierAtMin && bCrouchingMultiplierAtTarget && bJumpFallMultiplerIs1 && bAimingMultiplierAtTarget;
}

void ULyraRangedWeaponInstance::StartRecoil()
{
	APawn* const Pawn = GetPawn();
	if (Pawn == nullptr) return;

	bIsFiring = true;

	// 발사 시마다 수평/수직 반동 범위에서 무작위 값을 목표 반동에 누적
	TargetRecoil.X += FMath::FRandRange(HorizontalRecoilStep.X, HorizontalRecoilStep.Y);
	TargetRecoil.Y += FMath::FRandRange(VerticalRecoilStep.X, VerticalRecoilStep.Y);

	// 반동이 ±90도를 초과하지 않도록 제한
	TargetRecoil.X = FMath::Clamp(TargetRecoil.X, -90.f, 90.f);
	TargetRecoil.Y = FMath::Clamp(TargetRecoil.Y, -90.f, 90.f);
}

void ULyraRangedWeaponInstance::StopRecoil()
{
	if (!bIsFiring) return;

	bIsFiring = false;
	// 사격 종료 시 누적된 마우스 입력을 기반으로 반동 보정 적용
	ApplyInputCompensation();
}

void ULyraRangedWeaponInstance::UpdateRecoil(float DeltaTime)
{
	APawn* const Pawn = GetPawn();
	if (Pawn == nullptr) return;

	if (bIsFiring)
	{
		// 사격 중에는 마우스 입력 델타를 누적하여 나중에 반동 보정(ApplyInputCompensation)에 활용
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FVector2D MouseDelta;
			PC->GetInputMouseDelta(MouseDelta.X, MouseDelta.Y);
			InputCompensation += MouseDelta;
		}
	}
	else
	{
		// 사격 중지 시 목표 반동을 Damping 속도로 0(무반동)으로 복귀
		TargetRecoil = FMath::Vector2DInterpTo(TargetRecoil, FVector2D::ZeroVector, DeltaTime, Damping);
	}

	// 이전 프레임과 현재 프레임의 반동 차이만큼 컨트롤러 회전에 누적 적용
	// (매 프레임 델타를 더함으로써 급격한 회전 대신 부드러운 반동 연출)
	const FVector2D PreviousRecoil = CurrentRecoil;

	CurrentRecoil.X = FMath::FInterpTo(CurrentRecoil.X, TargetRecoil.X, DeltaTime, RecoilSmoothing.X);
	CurrentRecoil.Y = FMath::FInterpTo(CurrentRecoil.Y, TargetRecoil.Y, DeltaTime, RecoilSmoothing.Y);

	Pawn->AddControllerYawInput(CurrentRecoil.X - PreviousRecoil.X);
	Pawn->AddControllerPitchInput(CurrentRecoil.Y - PreviousRecoil.Y);
}

void ULyraRangedWeaponInstance::ApplyInputCompensation()
{
	// 마우스 입력에 Compensation 가중치를 적용하여 반동 상쇄 비율 계산
	// Pitch는 마우스 Y축이 반전되므로 부호를 반전(-InputCompensation.Y)하여 방향을 맞춤
	const float CompWeight = Compensation;
	const float YawMultiplier = GetInputCompensationMultiplier(CurrentRecoil.X, InputCompensation.X * CompWeight);
	const float PitchMultiplier = GetInputCompensationMultiplier(CurrentRecoil.Y, -InputCompensation.Y * CompWeight);

	// 계산된 배율로 현재 반동을 줄이고, 목표 반동도 동기화하여 부드러운 복귀 유지
	CurrentRecoil.X *= YawMultiplier;
	CurrentRecoil.Y *= PitchMultiplier;

	TargetRecoil = CurrentRecoil;
	// 보정에 사용된 입력 누적값 초기화
	InputCompensation = FVector2D::ZeroVector;
}

float ULyraRangedWeaponInstance::GetInputCompensationMultiplier(float RecoilValue, float CompensationValue) const
{
	// 반동과 마우스 입력이 반대 방향일 때만 보정 적용
	// (같은 방향이면 마우스가 반동을 더 키우는 것이므로 보정하지 않음)
	const bool bIsOpposite = (RecoilValue * CompensationValue) <= 0.f;

	if (!FMath::IsNearlyZero(RecoilValue) && bIsOpposite)
	{
		// 반동 대비 마우스 보정량 비율을 [0, 1]로 클램핑하여 반동 감소 배율 계산
		// 예: 반동 10, 보정 6 → Reduction = 0.6 → 배율 = 0.4 (40%만 남음)
		const float Reduction = FMath::Clamp(FMath::Abs(CompensationValue / RecoilValue), 0.f, 1.f);
		return 1.f - Reduction;
	}
	return 1.f;
}