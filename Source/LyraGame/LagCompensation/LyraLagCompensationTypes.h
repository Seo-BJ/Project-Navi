// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "LyraLagCompensationTypes.generated.h"

/**
 * 서버 리와인드 판정 결과.
 * 무기 어빌리티는 이 값만 보고 최종 타격 성공과 헤드샷 여부를 처리한다.
 */
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	bool bHeadShot = false;
};

/**
 * 서버 리와인드 요청 데이터.
 * 호출 지점에서 필요한 입력값을 한 덩어리로 묶어 판정 컴포넌트 내부 흐름을 단순화한다.
 */
USTRUCT(BlueprintType)
struct FLyraServerSideRewindRequest
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY()
	FVector_NetQuantize TraceStart = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize HitLocation = FVector::ZeroVector;

	UPROPERTY()
	float HitTime = 0.0f;
};

/** Snapshot 저장 방식. 각 자식 컴포넌트의 Deque 데이터 타입이 서로 다르므로 모드만 공통으로 노출한다. */
UENUM(BlueprintType)
enum class ELyraSnapShotMode : uint8
{
	None,
	BoxComponent,
	SkeletalMesh,
	AnimNode
};
