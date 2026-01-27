// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "NaviWeaponStatDefinition.generated.h"


/**
 * 거리별 데미지 정보 Struct
 */
USTRUCT(BlueprintType)
struct FDamageFalloff
{
	GENERATED_BODY()

	// 대미지 감소가 시작되는 거리 (단위: 미터)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RangeStart = 0.0f;

	// 대미지 감소가 끝나는 거리 (단위: 미터)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RangeEnd = 50.0f;
    
	// 해당 거리에서의 머리 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HeadShotDamage = 0.0f;

	// 해당 거리에서의 몸통 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BodyShotDamage = 0.0f;

	// 해당 거리에서의 다리 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LegShotDamage = 0.0f;
};

UENUM(BlueprintType)
enum class ENaviWeaponPenetrationPower : uint8
{
	Low,
	Medium,
	High,
};


UENUM(BlueprintType)
enum class ENaviWeaponFireMode : uint8
{
	Automatic,
	SemiAutomatic,
};

/**
 * 얜용┛ Stat DataTable Row Struct
 */
USTRUCT(BlueprintType)
struct FNaviWeaponStatDefinition : public FTableRowBase
{
    GENERATED_BODY()
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag WeaponTag; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 CreditCost; 

    // 관통력 
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ENaviWeaponPenetrationPower PenetrationPower;

    // 발사 방식 
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ENaviWeaponFireMode FireMode = ENaviWeaponFireMode::Automatic;

	// 거리 별 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDamageFalloff> DamageFalloffs;

	// 연사 속도 (초당 발사 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FireRate = 600.0f;
	
	// 달리기 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RunSpeed = 600.0f;
	
	// 장착 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EquipSpeed = 1.0f;
	
	// 재장전 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ReloadSpeed = 2.5f; 

	// 탄창
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MagazineSize = 30;
	
	// 최대 탄창
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxAmmo = 90; 

	// 초탄 퍼짐
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FirstShotSpread = 0.25f; 
};
