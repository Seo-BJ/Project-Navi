// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "Engine/HitResult.h"
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

	/**
	 * 서버 판정 HitResult. bHitConfirmed일 때만 유효하다.
	 * 되감은 포즈 기준으로, 선분이 가장 먼저 닿은 바디(또는 히트 박스)의 Actor/Component, ImpactPoint/Normal, BoneName,
	 * PhysMaterial(부위 태그 포함)을 담는다. TraceStart/TraceEnd는 판정에 쓴 선분이다.
	 */
	UPROPERTY()
	FHitResult ConfirmedHit;

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

	/**
	 * 쏜 액터. Box 방식의 콜리전 트레이스에서 자기 자신과 부착 액터를 무시하는 데 쓴다.
	 * (사격 시작점이 쏜 사람의 몸 안쪽에 있어 자기 메시가 먼저 맞을 수 있다)
	 */
	UPROPERTY()
	TObjectPtr<AActor> InstigatorActor = nullptr;

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
