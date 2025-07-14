// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "NaviWeaponStatDefinition.generated.h"


/**
 * 거리별 데미지 정보를 정의하는 구조체
 */
USTRUCT(BlueprintType)
struct FDamageFalloff
{
	GENERATED_BODY()

	// 대미지 감소가 시작되는 거리 (단위: 미터)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float RangeStart = 0.0f;

	// 대미지 감소가 끝나는 거리 (단위: 미터)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float RangeEnd = 50.0f;
    
	// 해당 거리에서의 머리 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float HeadShotDamage = 0.0f;

	// 해당 거리에서의 몸통 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BodyShotDamage = 0.0f;

	// 해당 거리에서의 다리 대미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float LegShotDamage = 0.0f;
};


/**
 * 무기 스텟 데이터 테이블 행 구조체
 */
USTRUCT(BlueprintType)
struct FNaviWeaponStatDefinition : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// --- 기본 정보 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "기본 정보")
	FText DisplayName; // 무기 이름

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "기본 정보")
	FGameplayTag WeaponTag; // 무기 게임플레이 태그

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "기본 정보")
	int32 CreditCost; // 인게임 상점 가격

	// --- 데미지 정보 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "대미지")
	TArray<FDamageFalloff> DamageFalloffs;
	
	// --- 무기 스탯 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "발사 스탯")
	float FireRate = 600.0f; // 연사 속도 (초당 발사 수)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "발사 스탯")
	float RunSpeed = 600.0f; // 달리기 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "발사 스탯")
	float EquipSpeed = 1.0f; // 장착 속도
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "발사 스탯")
	float ReloadSpeed = 2.5f; // 재장전 속도

	// --- 탄창 정보 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "탄약")
	int32 MagazineSize = 30; // 탄창

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "탄약")
	int32 MaxAmmo = 90; // 최대 탄창

	// --- 정확도 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "정확도")
	float FirstShotSpread = 0.25f; // 초탄 퍼짐
};
