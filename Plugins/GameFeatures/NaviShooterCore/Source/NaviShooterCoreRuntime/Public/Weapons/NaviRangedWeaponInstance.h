// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/LyraWeaponInstance.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"
#include "Engine/DataTable.h"

#include "NaviRangedWeaponInstance.generated.h"

class UPhysicalMaterial;
struct FNaviWeaponStatDefinition;


/**
 *  UNaviRangedWeaponInstance
 *  A piece of equipment representing a ranged weapon spawned and applied to a pawn
 *  Data-Driven 방식으로 설계되어 Navi Shooter Core에서 사용합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviRangedWeaponInstance : public ULyraWeaponInstance , public ILyraAbilitySourceInterface
{
	GENERATED_BODY()
	
public:
	UNaviRangedWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	void UpdateDebugVisualization();
#endif

	int32 GetBulletsPerCartridge() const
	{
		return BulletsPerCartridge;
	}
	
	/** 현재 탄퍼짐 각도를 반환합니다 (도 단위, 지름 기준) */
	float GetCalculatedSpreadAngle() const
	{
		return CurrentSpreadAngle;
	}

	float GetCalculatedSpreadAngleMultiplier() const
	{
		return bHasFirstShotAccuracy ? 0.0f : CurrentSpreadAngleMultiplier;
	}

	bool HasFirstShotAccuracy() const
	{
		return bHasFirstShotAccuracy;
	}

	float GetSpreadExponent() const
	{
		return SpreadExponent;
	}

	float GetMaxDamageRange() const
	{
		return MaxDamageRange;
	}

	float GetBulletTraceSweepRadius() const
	{
		return BulletTraceSweepRadius;
	}

	// Helper to get the stats struct
	UFUNCTION(BlueprintCallable)
	const FNaviWeaponStatDefinition& GetNaviWeaponStats();

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Spread|Fire Params")
	float Debug_MinHeat = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Spread|Fire Params")
	float Debug_MaxHeat = 0.0f;

	UPROPERTY(VisibleAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=deg))
	float Debug_MinSpreadAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=deg))
	float Debug_MaxSpreadAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, Category="Spread Debugging")
	float Debug_CurrentHeat = 0.0f;

	UPROPERTY(VisibleAnywhere, Category="Spread Debugging", meta = (ForceUnits=deg))
	float Debug_CurrentSpreadAngle = 0.0f;

	// 현재 *결합된* 탄퍼짐 각도 승수
	UPROPERTY(VisibleAnywhere, Category = "Spread Debugging", meta=(ForceUnits=x))
	float Debug_CurrentSpreadAngleMultiplier = 1.0f;

#endif

	// 탄퍼짐 지수(Spread exponent)입니다. 무기에 탄퍼짐이 있을 때(정확도가 완벽하지 않을 때)
	// 탄환이 중심선 주변에 얼마나 밀집될지를 결정합니다.
	// 값이 높을수록 탄환이 중앙에 더 가깝게 모입니다 (기본값은 1.0이며, 이는 탄퍼짐 범위 내에서 균일하게 분포함을 의미합니다).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1), Category="Spread|Fire Params")
	float SpreadExponent = 1.0f;

	// 열(Heat) 수치를 탄퍼짐 각도로 매핑하는 커브입니다.
	// 이 커브의 X축 범위는 일반적으로 무기의 최소/최대 열 범위를 설정합니다.
	// 이 커브의 Y축 범위는 최소 및 최대 탄퍼짐 각도를 정의하는 데 사용됩니다.
	UPROPERTY(EditAnywhere, Category = "Spread|Fire Params")
	FRuntimeFloatCurve HeatToSpreadCurve;

	// 현재 열 수치를 단발 사격 시 추가로 '가열(Heat up)'되는 양으로 매핑하는 커브입니다.
	// 일반적으로 단발 사격이 열을 얼마나 추가하는지를 나타내는 단일 데이터 포인트를 가진 평탄한 커브이지만,
	// 과열 시 열을 점진적으로 더 많이 추가하여 불이익을 주는 등의 목적으로 다른 형태를 가질 수 있습니다.
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToHeatPerShotCurve;
	
	// 현재 열 수치를 초당 열 냉각 속도로 매핑하는 커브입니다.
	// 일반적으로 열이 얼마나 빨리 식는지를 나타내는 단일 데이터 포인트를 가진 평탄한 커브이지만,
	// 높은 열 상태에서 회복 속도를 늦추어 과열에 대한 불이익을 주는 등의 목적으로 다른 형태를 가질 수 있습니다.
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToCoolDownPerSecondCurve;

	// 탄퍼짐 냉각(회복)이 시작되기 전, 사격 후 대기 시간(초 단위)입니다.
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=s))
	float SpreadRecoveryCooldownDelay = 0.0f;

	// 플레이어와 무기의 탄퍼짐이 모두 최소값일 때 무기가 완벽한 정확도를 가져야 하는지 여부입니다.
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	bool bAllowFirstShotAccuracy = false;

	// 조준(Aiming) 카메라 모드일 때의 탄퍼짐 승수입니다.
	UPROPERTY(EditAnywhere, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Aiming = 1.0f;

	// 가만히 서 있거나 매우 천천히 움직일 때의 탄퍼짐 승수입니다.
	// (StandingStillSpeedThreshold에서 사라지기 시작하여 StandingStillSpeedThreshold + StandingStillToMovingSpeedRange에서 완전히 사라집니다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_StandingStill = 1.0f;

	// 서 있는 상태의 정확도로(또는 그 반대로) 전환되는 속도입니다 (값이 높을수록 빠르며, 0은 즉시 전환됩니다; @see FInterpTo).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_StandingStill = 5.0f;

	// 이 속도 이하이면 서 있는 것으로 간주합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillSpeedThreshold = 80.0f;

	// StandingStillSpeedThreshold보다 이 값만큼 더 높은 속도까지 서 있는 상태 보너스를 점진적으로 줄여 1.0으로 되돌립니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillToMovingSpeedRange = 20.0f;

	// 웅크린 상태(Crouching)일 때의 탄퍼짐 승수이며, TransitionRate_Crouching에 따라 부드럽게 블렌딩됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Crouching = 1.0f;

	// 웅크린 상태의 정확도로(또는 그 반대로) 전환되는 속도입니다 (값이 높을수록 빠르며, 0은 즉시 전환됩니다; @see FInterpTo).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_Crouching = 5.0f;
	
	// 점프/낙하(Jumping/Falling) 중일 때의 탄퍼짐 승수이며, TransitionRate_JumpingOrFalling에 따라 부드럽게 블렌딩됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_JumpingOrFalling = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_JumpingOrFalling = 5.0f;
	
	// 수평 반동의 (최소, 최대) 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	FVector2D HorizontalRecoilStep{ FVector2D::ZeroVector };
    
	// 한 발 당 수직 반동 값의 (최소, 최대) 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	FVector2D VerticalRecoilStep = FVector2D::ZeroVector;

	// 목표 반동 값으로의 보간 속도 (높을수록 빠르게 도달)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	FVector2D RecoilSmoothing = FVector2D(15.0f, 15.0f);

	// 사격 중지 시 반동 회복 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin="0.0"))
	float Damping = 8.0f;

	// 플레이어의 반동 제어(마우스 움직임)가 실제 반동에 영향을 미치는 정도 (0.0 ~ 1.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin="0.0", ClampMax="1.0"))
	float Compensation = 0.6f;
	
	// 단일 카트리지(약실)에서 발사되는 총알의 수입니다 (일반적으로 1이지만 샷건의 경우 더 많을 수 있음).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	int32 BulletsPerCartridge = 1;

	// 이 무기가 피해를 줄 수 있는 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float MaxDamageRange = 25000.0f;

	// 총알 트레이스 스윕 구체의 반지름입니다 (0.0이면 라인 트레이스가 됨).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float BulletTraceSweepRadius = 0.0f;

	// 거리(cm)를 관련 게임플레이 이펙트의 기본 데미지에 대한 승수로 매핑하는 커브입니다.
	// 이 커브에 데이터가 없으면 무기는 거리에 따른 데미지 감소(Falloff)가 없는 것으로 간주됩니다.
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	FRuntimeFloatCurve DistanceDamageFalloff;

	// 데미지 처리에 영향을 미치는 특수 태그 목록입니다.
	// 이 태그들은 피격된 물체의 물리 재질(Physical Material)에 있는 태그와 비교됩니다.
	// 두 개 이상의 태그가 존재하는 경우, 승수(Multiplier)들은 곱셈으로 결합됩니다.
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	TMap<FGameplayTag, float> MaterialDamageMultiplier;

	// Navi Weapon Stats Table Row
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Config", meta = (RowType = "/Script/NaviShooterCoreRuntime.NaviWeaponStatDefinition"))
	FDataTableRowHandle WeaponStatRow;


private:
	// 이 무기가 마지막으로 발사된 시간 (월드 시간 기준)
	double LastFireTime = 0.0;

	// 현재 열(Heat)
	float CurrentHeat = 0.0f;

	// 현재 탄퍼짐 각도 (도 단위, 지름 기준)
	float CurrentSpreadAngle = 0.0f;

	// 현재 초탄 정확도가 적용되는지 여부
	bool bHasFirstShotAccuracy = false;

	// 현재 *결합된* 탄퍼짐 각도 승수
	float CurrentSpreadAngleMultiplier = 1.0f;

	// 현재 서 있는 상태(Standing Still) 승수
	float StandingStillMultiplier = 1.0f;

	// 현재 점프/낙하(Jumping/Falling) 승수
	float JumpFallMultiplier = 1.0f;

	// 현재 웅크리기(Crouching) 승수
	float CrouchingMultiplier = 1.0f;

public:
	void Tick(float DeltaSeconds);

	//~ULyraEquipmentInstance interface
	virtual void OnEquipped();
	virtual void OnUnequipped();
	//~End of ULyraEquipmentInstance interface

	void AddSpread();

	//~ILyraAbilitySourceInterface interface
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	//~End of ILyraAbilitySourceInterface interface

private:
	void ComputeSpreadRange(float& MinSpread, float& MaxSpread);
	void ComputeHeatRange(float& MinHeat, float& MaxHeat);

	inline float ClampHeat(float NewHeat)
	{
		float MinHeat;
		float MaxHeat;
		ComputeHeatRange(/*out*/ MinHeat, /*out*/ MaxHeat);

		return FMath::Clamp(NewHeat, MinHeat, MaxHeat);
	}

	// Updates the spread and returns true if the spread is at minimum
	bool UpdateSpread(float DeltaSeconds);

	// Updates the multipliers and returns true if they are at minimum
	bool UpdateMultipliers(float DeltaSeconds);
	
public:
	void StartRecoil();
	void StopRecoil();

private:
	void UpdateRecoil(float DeltaTime);

	void ApplyInputCompensation();

	float GetInputCompensationMultiplier(float RecoilValue, float CompensationValue) const;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	FVector2D TargetRecoil;
    
	FVector2D CurrentRecoil;
    
	FVector2D InputCompensation;
    
	bool bIsFiring;

	// Cached pointer to the weapon stats
	const FNaviWeaponStatDefinition* CachedNaviWeaponStats = nullptr;

};
