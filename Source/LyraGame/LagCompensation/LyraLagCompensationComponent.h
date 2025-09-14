// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LyraLagCompensationComponent.generated.h"

LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLagCompensation, Log, All);

class ALyraCharacter;
class ALyraPlayerController;

// Hitbox의 위치, 회전, 크기 정보를 담는 구조체
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY()
	FVector BoxExtent = FVector::ZeroVector;
};

// 특정 시간의 Actor와 HitBox를 정보를 담는 구조체
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;
	
	UPROPERTY()
	TObjectPtr<AActor> HitActor = nullptr;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

// Server-side rewind 이후 단일 Hit에 대한 판정 결과를 담는 구조체
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
 * Server-side rewind를 통해 랙 보상(Lag Compensation)을 처리하는 컴포넌트
 * 이 컴포넌트의 Owner Actor의 Box Collision들의 위치들을 서버에서 저장
 * LyraGameplayAbility_RangedWeapon을 통해 사격시 사격 시간으로 서버 시간을 되감아 피격 판정을 수행
 * 
 * HitActor는 ILagCompensationTarget를 구현해야함
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class ABlasterCharacter;

	ULyraLagCompensationComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Server-Side Rewind를 요청하는 메인 함수
	 * @param HitActor 피격당한 Actor
	 * @param TraceStart 발사 시작 위치입니다.
	 * @param HitLocation 클라이언트가 보고한 피격 위치입니다
	 * @param HitTime 피격이 발생한 시점의 서버 시간
	 * @return FServerSideRewindResult 되감기 후의 최종 Hit 판정 결과
	 */
	FServerSideRewindResult ServerSideRewind(
		AActor* HitActor,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
		);

private:

	// FrameHistory를 갱신
	void UpdateFrameHistory();

	// Actor의 히트박스 들을 저장
	void SaveFramePackage(FFramePackage& Package);
	
	// HitTime을 기준으로 Hit 판정에 사용할 프레임 반환
	FFramePackage GetHitTimeFrame(AActor* HitActor, float HitTime);
	/**
	 * 되감기가 완료된 상태에서 실제 Hit 판정을 위한 Trace를 수행
	 * @param FrameToCheck 되감기 목표 시점의 프레임
	 */
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& FrameToCheck,
		AActor* HitActor,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
		);
	
	//두 프레임 사이 특정 시간(HitTime)에 위치 할 프레임을 선형 보간하여 계산
	FFramePackage InterpolateBetweenTwoFrames(
		const FFramePackage& OlderFrame,
		const FFramePackage& YoungerFrame,
		float HitTime
		);

	FFramePackage ExtrapolateByTwoFrames(
		const FFramePackage& SecondNewestFrame,
		const FFramePackage& FirstNewestFrame,
		float HitTime
	);
	
	// 현재 Actor의 모든 히트박스 정보를 OutFramePackage에 저장
	void CacheCurrentFrame(AActor* HitActor, FFramePackage& OutFramePackage);
	// Actor의 히트박스들을 특정 FramePackage의 위치로 되감음
	void RewindFrame(AActor* HitActor, const FFramePackage& Package);
	// Actor의 히트박스들을 저장된 현재 위치로 복원
	void ResetHitBoxes(AActor* HitActor, const FFramePackage& Package);

	// 되감기 중 Actor의 메인 Mesh Collision을 활성화 상태 설정
	void SetMeshCollisionEnabledType(AActor* HitActor, ECollisionEnabled::Type CollsionEnabled);
	
	// Draw Debug 함수
	void DrawDebugFramePackage(const FFramePackage& FramePackage);
	void DrawDebugConfirmHitResult(FHitResult ConfirmHitResult);

	// 서버가 관리하는 엑터의 과거 프레임 데이터
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	bool bDrawFrameHistory = false;

	UPROPERTY(EditAnywhere)
	bool bDrawHitResult = true;

	// 서버가 프레임 데이터를 얼마나 오랫동안 보관하는지 (초 단위)
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 2.0f;

	// 디버그 드로잉 빈도 (N틱에 1번)
	UPROPERTY(EditAnywhere)
	int32 DrawDebugFrequency = 2;

	// 디버그 히트박스를 유지할 시간
	UPROPERTY(EditAnywhere)
	float DrawDebugHitBoxTime = 4.0f;
	
	int32 TickCounter = 0;
};
