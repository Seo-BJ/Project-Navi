// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Deque.h"
#include "LyraSnapShotComponent.h"
#include "LyraSnapShotComponent_SkeletalMesh.generated.h"

class USkeletalMeshComponent;

/** 특정 서버 시간에 저장된 SkeletalMesh pose Snapshot frame. */
USTRUCT(BlueprintType)
struct FLyraMeshSnapShotFrame
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;

	UPROPERTY()
	TObjectPtr<AActor> HitActor = nullptr;

	/** 저장 시점의 mesh component transform. component-space bone transform을 world로 변환할 때 사용한다. */
	UPROPERTY()
	FTransform MeshTransform;

	/** 모든 bone의 component-space transform. PhysicsAsset 충돌 계산의 기준 pose다. */
	UPROPERTY()
	TArray<FTransform> BoneTransforms;
};

/**
 * SkeletalMesh 기반 Snapshot 저장 컴포넌트.
 * 서버 Tick마다 mesh component transform과 bone component-space transform을 기록한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraSnapShotComponent_SkeletalMesh : public ULyraSnapShotComponent
{
	GENERATED_BODY()

public:
	ULyraSnapShotComponent_SkeletalMesh();

	virtual ELyraSnapShotMode GetSnapShotMode() const override { return ELyraSnapShotMode::SkeletalMesh; }
	virtual bool HasAnySnapShot() const override { return SnapShotHistory.Num() > 0; }

	/** Actor에서 판정에 사용할 SkeletalMeshComponent를 찾는다. */
	USkeletalMeshComponent* GetSkeletalMesh(AActor* InActor) const;

	/** 현재 mesh pose를 Snapshot frame으로 캡처한다. */
	void CaptureCurrentSnapShot(AActor* HitActor, FLyraMeshSnapShotFrame& OutPackage) const;

	/** 요청한 서버 시간에 해당하는 mesh Snapshot을 찾고, 필요하면 보간한다. */
	bool TryGetSnapShotAtTime(float HitTime, FLyraMeshSnapShotFrame& OutPackage) const;

#if ENABLE_DRAW_DEBUG
	/** 저장된 mesh Snapshot pose를 PhysicsAsset primitive 기준으로 시각화한다. */
	void DrawDebugPose(const FLyraMeshSnapShotFrame& Package, FColor Color) const;
#endif

protected:
	/** 서버 Tick에서 최신 mesh Snapshot을 추가하고 MaxRecordTime 밖의 과거 frame을 제거한다. */
	virtual void UpdateSnapShotHistory() override;

private:
	FLyraMeshSnapShotFrame InterpolateFrame(const FLyraMeshSnapShotFrame& Older, const FLyraMeshSnapShotFrame& Younger, float HitTime) const;

	/** 최신 frame이 앞쪽에 오도록 저장되는 mesh Snapshot history. */
	TDeque<FLyraMeshSnapShotFrame> SnapShotHistory;
};
