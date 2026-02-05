// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LyraLagCompensationComponent.h" // FServerSideRewindResult 재사용을 위해 포함
#include "LyraMeshLagCompensationComponent.generated.h"

// 로그 카테고리는 기존 LyraLagCompensationComponent의 것을 공유하거나 새로 정의할 수 있음
// 여기서는 공유한다고 가정하거나, cpp에서 새로 정의

class ALyraCharacter;
class ALyraPlayerController;
class ULyraGameplayAbility_RangedWeapon;

/**
 * 스켈레탈 메시의 전체 본 포즈(Bone Pose)를 저장하는 스냅샷 구조체
 */
USTRUCT(BlueprintType)
struct FMeshFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;
	
	UPROPERTY()
	TObjectPtr<AActor> HitActor = nullptr;

	// 메시의 월드 트랜스폼 (렉 보상 시 기준점으로 사용)
	UPROPERTY()
	FTransform MeshTransform;

	// 핵심: 모든 본의 Component Space Transform
	UPROPERTY()
	TArray<FTransform> BoneTransforms;
};

/**
 * Skeletal Mesh 기반의 정밀 렉 보상 컴포넌트
 * Physics Asset과 실제 애니메이션 포즈를 되감아 판정함
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraMeshLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

	friend ULyraGameplayAbility_RangedWeapon;
	
public:	
	ULyraMeshLagCompensationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Server-Side Rewind 요청 (Mesh 기반)
	 */
	FServerSideRewindResult ServerSideRewind(
		AActor* HitActor,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
		);

protected:
	// --- 핵심 로직 ---

	// 메시 컴포넌트 안전하게 가져오기
	USkeletalMeshComponent* GetSkeletalMesh(AActor* InActor) const;

	// 현재 메시의 포즈를 캡처하여 저장
	void CacheCurrentFrame(AActor* HitActor, FMeshFramePackage& OutPackage);

	// 프레임 히스토리 업데이트 (Tick에서 호출)
	void UpdateFrameHistory();

	// 특정 시간의 프레임 패키지 찾기 (보간 포함)
	FMeshFramePackage GetHitTimeFrame(AActor* HitActor, float HitTime);

	// 두 프레임 사이의 포즈 보간
	FMeshFramePackage InterpolateFrame(const FMeshFramePackage& Older, const FMeshFramePackage& Younger, float HitTime);

	// 되감기 실행 (메시 포즈 강제 적용 + 물리 업데이트)
	void RewindFrame(AActor* HitActor, const FMeshFramePackage& Package);

	// 되감기 복구 (원래 포즈로 복귀)
	// CacheCurrentFrame으로 저장해둔 '현재' 상태를 다시 RewindFrame 하는 것과 같음

	// 실제 판정 수행
	FServerSideRewindResult ConfirmHit(
		const FMeshFramePackage& FrameToCheck,
		AActor* HitActor,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
	);

private:

	bool PerformPhysicsAssetCollision(
	    const TObjectPtr<AActor> HitActor,
	    const FMeshFramePackage& Frame,
	    const FVector& TraceStart,
	    const FVector& TraceEnd,
	    FHitResult& OutHit);

	// --- 내부 헬퍼 ---
	
	// 판정 결과 시각화
	void VisualizeConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* HitActor);

	// 디버그용: 저장된 포즈를 시각화 (선택 사항)
	void DrawDebugPose(const FMeshFramePackage& Package, FColor Color) const;

	// --- 데이터 ---

	// 프레임 히스토리 (최신 -> 과거 순)
	TDoubleLinkedList<FMeshFramePackage> FrameHistory;

	// 설정 변수들
	UPROPERTY(EditAnywhere, Category = "Lag Compensation")
	float MaxRecordTime = 1.0f; // 1초 정도만 저장해도 충분 (메모리 절약)
};
