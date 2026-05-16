// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensation/LyraLagCompensationTypes.h"
#include "LyraSnapShotComponent.generated.h"

class ALyraCharacter;
class USkeletalMeshComponent;

/**
 * Snapshot 저장 컴포넌트의 공통 부모.
 * Tick마다 과거 상태를 기록하는 책임만 가진다. 실제 저장 데이터는 방식별 자식 클래스가 보유한다.
 */
UCLASS(Abstract, ClassGroup=(Custom))
class LYRAGAME_API ULyraSnapShotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraSnapShotComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 이 컴포넌트가 저장하는 Snapshot 방식. SSR 컴포넌트가 판정 dispatch에 사용한다. */
	virtual ELyraSnapShotMode GetSnapShotMode() const { return ELyraSnapShotMode::None; }

	/** 최소 하나 이상의 Snapshot이 저장되어 있는지 확인한다. */
	virtual bool HasAnySnapShot() const { return false; }

	/** 서버가 과거 Snapshot을 보관하는 시간(초). */
	float GetMaxRecordTime() const { return MaxRecordTime; }

	/** LyraCharacter의 현재 실제 mesh pose를 PhysicsAsset primitive 기준으로 시각화한다. */
	void DrawCurrentPhysicsAssetPose(const ALyraCharacter* TargetCharacter, FColor Color) const;

protected:
	/** 자식 컴포넌트가 방식별 Snapshot을 Deque에 추가하고 보관 시간을 정리한다. */
	virtual void UpdateSnapShotHistory() {}

	/** 서버가 과거 Snapshot 데이터를 보관하는 최대 시간(초). */
	UPROPERTY(EditAnywhere, Category = "Lag Compensation", meta = (ClampMin = "0.0"))
	float MaxRecordTime = 0.5f;

	void DrawPhysicsAssetPose(
		const USkeletalMeshComponent* SkeletalMeshComp,
		const FTransform& MeshTransform,
		const TArray<FTransform>& ComponentSpaceTransforms,
		FColor Color,
		float LifeTime) const;
};
