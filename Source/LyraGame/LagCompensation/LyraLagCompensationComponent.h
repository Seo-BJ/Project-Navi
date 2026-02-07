// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LyraLagCompensationComponent.generated.h"

/**
 * Server-side rewind 이후 단일 Hit에 대한 판정 결과를 담는 구조체
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
 * 렉 보상(Lag Compensation) 기능을 제공하는 컴포넌트의 베이스 클래스
 */
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULyraLagCompensationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Server-Side Rewind 요청의 공통 인터페이스
	 */
	virtual FServerSideRewindResult ServerSideRewind(
		AActor* HitActor,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	) { return FServerSideRewindResult(); }

protected:
	/** 프레임 히스토리 업데이트 로직 (자식 클래스에서 구현) */
	virtual void UpdateFrameHistory() {}

	/** 서버가 프레임 데이터를 얼마나 오랫동안 보관하는지 (초 단위) */
	UPROPERTY(EditAnywhere, Category = "Lag Compensation")
	float MaxRecordTime = 2.0f;
};
