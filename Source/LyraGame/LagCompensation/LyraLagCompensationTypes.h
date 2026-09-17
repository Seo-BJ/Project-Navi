// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "LyraLagCompensationTypes.generated.h"

/** 한 번의 서버 리와인드 요청에서 수집한 진단 정보. */
struct FLyraSSRRewindDiagnostics
{
	bool bSnapshotLookupSucceeded = false;
	bool bUsedInterpolation = false;
	bool bPoseReconstructionSucceeded = true;
	bool bStateRestoreSucceeded = true;
	bool bTopologyMatched = true;
};

/**
 * 서버 리와인드 판정 결과.
 * 판정값과 요청 단위 진단값을 함께 반환하므로 SnapshotComponent에 이전 요청 상태를 남기지 않는다.
 */
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	bool bHeadShot = false;

	/** C++ 진단/프로파일링용 요청 단위 부가 정보. Blueprint 판정 계약에는 포함하지 않는다. */
	FLyraSSRRewindDiagnostics Diagnostics;
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
