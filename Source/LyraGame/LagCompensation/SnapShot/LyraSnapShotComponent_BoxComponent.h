// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraSnapShotComponent.h"
#include "Containers/Deque.h"

#include "LyraSnapShotComponent_BoxComponent.generated.h"

LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLagCompensation, Log, All);

class UBoxComponent;

/** BoxComponent Snapshot에 저장할 hitbox transform 정보. */
USTRUCT(BlueprintType)
struct FLyraBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector BoxExtent = FVector::ZeroVector;
};

/** 특정 서버 시간에 저장된 BoxComponent 기반 Snapshot frame. */
USTRUCT(BlueprintType)
struct FLyraBoxSnapShotFrame
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;

	UPROPERTY()
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY()
	TMap<FName, FLyraBoxInformation> HitBoxInfo;
};

/**
 * BoxComponent 기반 Snapshot 저장 컴포넌트.
 * 서버 Tick마다 ILagCompensationTarget의 hitbox transform을 기록한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraSnapShotComponent_BoxComponent : public ULyraSnapShotComponent
{
	GENERATED_BODY()

public:
	ULyraSnapShotComponent_BoxComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual ELyraSnapShotMode GetSnapShotMode() const override { return ELyraSnapShotMode::BoxComponent; }
	virtual bool HasAnySnapShot() const override { return SnapShotHistory.Num() > 0; }

	/** 현재 Actor의 hitbox 상태를 Snapshot frame으로 캡처한다. */
	void CaptureCurrentSnapShot(AActor* HitActor, FLyraBoxSnapShotFrame& OutFramePackage) const;

	/** 요청한 서버 시간에 해당하는 Snapshot을 찾고, 필요하면 보간/외삽한다. */
	bool TryGetSnapShotAtTime(float HitTime, FLyraBoxSnapShotFrame& OutFramePackage) const;

	/** hitbox 컴포넌트들을 Snapshot frame의 위치와 크기로 이동한다. */
	void ApplySnapShot(AActor* HitActor, const FLyraBoxSnapShotFrame& Package) const;

	/** hitbox 컴포넌트들을 저장된 현재 상태로 복구하고 trace collision을 끈다. */
	void RestoreSnapShot(AActor* HitActor, const FLyraBoxSnapShotFrame& Package) const;

	float GetDrawDebugHitBoxTime() const { return DrawDebugHitBoxTime; }

#if ENABLE_DRAW_DEBUG
	void DrawDebugFramePackage(const FLyraBoxSnapShotFrame& FramePackage) const;
#endif

protected:
	/** 서버 Tick에서 최신 Snapshot을 추가하고 MaxRecordTime 밖의 과거 frame을 제거한다. */
	virtual void UpdateSnapShotHistory() override;

private:
	FLyraBoxSnapShotFrame InterpolateBetweenTwoFrames(
		const FLyraBoxSnapShotFrame& OlderFrame,
		const FLyraBoxSnapShotFrame& YoungerFrame,
		float HitTime
		) const;

	FLyraBoxSnapShotFrame ExtrapolateByTwoFrames(
		const FLyraBoxSnapShotFrame& SecondNewestFrame,
		const FLyraBoxSnapShotFrame& FirstNewestFrame,
		float HitTime
		) const;

	/** 최신 frame이 앞쪽에 오도록 저장되는 Box Snapshot history. */
	TDeque<FLyraBoxSnapShotFrame> SnapShotHistory;

	UPROPERTY(EditAnywhere, Category = "Lag Compensation|Debug")
	bool bDrawFrameHistory = false;

	UPROPERTY(EditAnywhere, Category = "Lag Compensation|Debug", meta = (ClampMin = "1"))
	int32 DrawDebugFrequency = 2;

	UPROPERTY(EditAnywhere, Category = "Lag Compensation|Debug", meta = (ClampMin = "0.0"))
	float DrawDebugHitBoxTime = 4.0f;

	int32 TickCounter = 0;
};
