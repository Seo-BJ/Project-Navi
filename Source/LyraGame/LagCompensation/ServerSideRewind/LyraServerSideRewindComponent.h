// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "LagCompensation/LyraLagCompensationTypes.h"
#include "LyraServerSideRewindComponent.generated.h"

class ULyraSnapShotComponent;
class ULyraSnapShotComponent_AnimNode;
class ULyraSnapShotComponent_BoxComponent;
class ULyraSnapShotComponent_SkeletalMesh;
class UBoxComponent;
class USkeletalMeshComponent;

struct FAnimSSRRewindPose;
struct FLyraBoxSnapShotFrame;
struct FLyraMeshSnapShotFrame;

LYRAGAME_API ULyraSnapShotComponent* SelectUniqueActiveLyraSnapShotComponent(
	TConstArrayView<ULyraSnapShotComponent*> Components);

/**
 * 서버 사이드 리와인드 판정 전용 컴포넌트.
 * Snapshot 저장은 ULyraSnapShotComponent 계층이 담당하고, 이 컴포넌트는 저장된 Snapshot을 이용해 최종 hit 여부만 판단한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraServerSideRewindComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraServerSideRewindComponent();

	/** 서버 리와인드 판정 진입점. */
	FServerSideRewindResult ServerSideRewind(const FLyraServerSideRewindRequest& Request);

	/** 활성 플래그로 선택된 유일한 Snapshot 컴포넌트. 프로파일 검증에도 사용한다. */
	ULyraSnapShotComponent* ResolveSnapShotComponent(AActor* TargetActor) const;

private:
	FServerSideRewindResult ServerSideRewindBoxComponent(ULyraSnapShotComponent_BoxComponent* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const;
	FServerSideRewindResult ServerSideRewindSkeletalMesh(ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const;
	FServerSideRewindResult ServerSideRewindAnimNode(ULyraSnapShotComponent_AnimNode* SnapShotComponent, const FLyraServerSideRewindRequest& Request) const;

	FServerSideRewindResult ConfirmBoxHit(
		ULyraSnapShotComponent_BoxComponent* SnapShotComponent,
		const FLyraBoxSnapShotFrame& FrameToCheck,
		const FLyraServerSideRewindRequest& Request
		) const;

	FServerSideRewindResult ConfirmMeshHit(
		ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent,
		const FLyraMeshSnapShotFrame& FrameToCheck,
		const FLyraServerSideRewindRequest& Request
		) const;

	FServerSideRewindResult ConfirmAnimNodeHit(
		ULyraSnapShotComponent_AnimNode* SnapShotComponent,
		const FAnimSSRRewindPose& RewindPose,
		const FLyraServerSideRewindRequest& Request
		) const;

	/** Box Snapshot 방식에서 실제 collision trace를 수행한다. */
	bool PerformBoxHitCheck(const TArray<UBoxComponent*>& BoxesToCheck, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHit) const;

	/** Box Snapshot 방식 판정 중 기존 mesh/capsule collision을 끄거나 복구한다. */
	void SetMeshCollisionEnabledType(AActor* TargetActor, ECollisionEnabled::Type CollisionEnabled) const;

	/** SkeletalMesh Snapshot 방식에서 PhysicsAsset primitive와 trace segment의 교차 여부를 계산한다. */
	bool PerformPhysicsAssetCollision(
		ULyraSnapShotComponent_SkeletalMesh* SnapShotComponent,
		AActor* TargetActor,
		const FLyraMeshSnapShotFrame& Frame,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		const TSet<FName>& BoneFilter,
		FHitResult& OutHit
		) const;

	bool PerformPhysicsAssetCollision(
		USkeletalMeshComponent* SkeletalMeshComp,
		const FTransform& MeshTransform,
		const TArray<FTransform>& ComponentSpaceTransforms,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		const TSet<FName>& BoneFilter,
		FHitResult& OutHit
		) const;

#if ENABLE_DRAW_DEBUG
	void VisualizeBoxConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* TargetActor) const;
	void DrawDebugBoxHitResult(const FHitResult& HitResult, bool bConfirmHit) const;
	void VisualizeMeshConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, const AActor* TargetActor) const;
#endif

	/** Box 방식 SSR trace와 결과 hitbox를 그릴지 여부. */
	UPROPERTY(EditAnywhere, Category = "Server Side Rewind|Debug")
	bool bDrawBoxHitResult = false;

	/** Box 방식 debug line/box 유지 시간. */
	UPROPERTY(EditAnywhere, Category = "Server Side Rewind|Debug", meta = (ClampMin = "0.0"))
	float BoxHitDebugDrawTime = 4.0f;
};
