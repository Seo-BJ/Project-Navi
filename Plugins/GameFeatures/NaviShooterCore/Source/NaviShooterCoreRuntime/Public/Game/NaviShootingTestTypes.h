// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NaviShootingTestTypes.generated.h"

/** 
 * 난이도 (반응 속도)
 * 봇이 얼마나 빨리 사라지는지를 결정합니다.
 */
UENUM(BlueprintType)
enum class ENaviTestSpeed : uint8
{
	Easy		UMETA(DisplayName = "Easy (Slow)"),
	Medium		UMETA(DisplayName = "Medium"),
	Hard		UMETA(DisplayName = "Hard (Fast)")
};

/**
 * 목표 처치 수 (Streak)
 * 연속해서 몇 기의 봇을 제거할지 목표를 설정합니다.
 */
UENUM(BlueprintType)
enum class ENaviTestStreak : uint8
{
	Limit50		UMETA(DisplayName = "Eliminate 50"),
	Limit100	UMETA(DisplayName = "Eliminate 100"),
	Endless		UMETA(DisplayName = "Endless (Practice)")
};

/**
 * 봇 이동 모드
 * 봇이 제자리에 서있을지, 움직일지 결정합니다.
 */
UENUM(BlueprintType)
enum class ENaviBotMovement : uint8
{
	Static		UMETA(DisplayName = "Static"),
	Strafe		UMETA(DisplayName = "Strafe (Left/Right)")
};

/**
 * 사격 테스트의 전체 설정 정보를 담는 구조체
 */
USTRUCT(BlueprintType)
struct FNaviShootingTestSettings
{
	GENERATED_BODY()

	/** 난이도 (봇 생존 시간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest")
	ENaviTestSpeed Speed = ENaviTestSpeed::Easy;

	/** 목표 처치 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest")
	ENaviTestStreak Streak = ENaviTestStreak::Limit50;

	/** 봇 이동 모드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest")
	ENaviBotMovement BotMovement = ENaviBotMovement::Static;

	/** 봇 아머 적용 여부 (On: 경갑 적용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest")
	bool bBotArmor = false;

	/** 무한 탄약 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest")
	bool bInfiniteAmmo = false;

	/** 스폰 최소 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest|Range")
	float MinSpawnDistance = 1000.0f;

	/** 스폰 최대 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest|Range")
	float MaxSpawnDistance = 2500.0f;

	/** 스폰 각도 범위 (부채꼴 중심각, 도 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|ShootingTest|Range")
	float SpawnAngleFOV = 90.0f;
};
