// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Deque.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Animation/AnimNode_StateMachine.h"
#include "AnimNodes/AnimNode_BlendListByBool.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "AnimNodes/AnimNode_RotationOffsetBlendSpace.h"
#include "BoneControllers/AnimNode_CopyBone.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "LyraSnapShotComponent.h"
#include "LyraSnapShotComponent_AnimNode.generated.h"

class FStructProperty;
class UAnimInstance;
class UAnimSequenceBase;
class UBlendProfile;
class UBlendSpace;
class UCurveFloat;
class USkeletalMesh;
class USkeletalMeshComponent;
class ULyraExperienceDefinition;

/** AnimClass의 node property 배열에서 동일 노드를 다시 찾기 위한 공통 식별자다. */
struct FAnimSSRNodeIdentity
{
	int32 NodeIndex = INDEX_NONE;
	FName NodePropertyName = NAME_None;
	FName NodeStructName = NAME_None;
};

/** 스켈레톤 pose DrawDebug 옵션을 한 곳에 묶어 saved/server/client 설정 반복을 줄인다. */
USTRUCT(BlueprintType)
struct FAnimSSRDebugPoseDrawSettings
{
	GENERATED_BODY()

	FAnimSSRDebugPoseDrawSettings() = default;

	FAnimSSRDebugPoseDrawSettings(
		bool bInEnabled,
		int32 InFrameInterval,
		float InDuration,
		const FColor& InColor,
		float InThickness,
		int32 InMaxDebugBones,
		bool bInDrawJoints,
		float InJointRadius)
		: bEnabled(bInEnabled)
		, FrameInterval(InFrameInterval)
		, Duration(InDuration)
		, Color(InColor)
		, Thickness(InThickness)
		, MaxDebugBones(InMaxDebugBones)
		, bDrawJoints(bInDrawJoints)
		, JointRadius(InJointRadius)
	{
	}

	/** 이 pose draw를 실행할지 결정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	bool bEnabled = false;

	/** 매 N번째 tick/frame마다 한 번만 그려 DrawDebug 비용을 제한한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug", meta = (ClampMin = "1"))
	int32 FrameInterval = 10;

	/** debug primitive가 화면에 남아 있는 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug", meta = (ClampMin = "0.0"))
	float Duration = 0.1f;

	/** bone hierarchy line과 joint sphere 색상이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	FColor Color = FColor::White;

	/** debug line/sphere 두께다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug", meta = (ClampMin = "0.0"))
	float Thickness = 1.0f;

	/** 0보다 크면 앞쪽 N개 bone만 그린다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug", meta = (ClampMin = "0"))
	int32 MaxDebugBones = 0;

	/** bone 위치에 joint sphere를 함께 그릴지 결정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	bool bDrawJoints = true;

	/** joint sphere 반지름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug", meta = (ClampMin = "0.0"))
	float JointRadius = 2.0f;
};

/** Bone Controller 적용/비적용 비교 DrawDebug 전용 설정이다. */
USTRUCT(BlueprintType)
struct FAnimSSRDebugComparisonDrawSettings
{
	GENERATED_BODY()

	/** 저장된 snapshot을 bone controller 적용/비적용 포즈로 나누어 비교할지 결정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	bool bEnabled = false;

	/** bone controller를 끈 비교 포즈의 색상이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	FColor DisabledPoseColor = FColor::Magenta;

	/** 두 pose가 겹치지 않도록 비교 포즈에 적용할 world offset이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	FVector DisabledPoseWorldOffset = FVector(0.0f, 75.0f, 0.0f);

	/** 비교 pose 위에 C/T/IK 개수 label을 표시할지 결정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw Debug")
	bool bDrawLabel = true;
};

/** Sequence Player 노드의 SSR 스냅샷이다. */
struct FAnimSSRSequencePlayerSnapshot
{
	FAnimSSRNodeIdentity Identity;
	TWeakObjectPtr<UAnimSequenceBase> SequenceAsset;
	FAnimNode_AssetPlayerStateSnapshot AssetPlayerState;
};

/** BlendSpace Player 노드의 SSR 스냅샷이다. */
struct FAnimSSRBlendSpacePlayerSnapshot
{
	FAnimSSRNodeIdentity Identity;
	TWeakObjectPtr<UBlendSpace> BlendSpaceAsset;
	FAnimNode_BlendSpacePlayerStateSnapshot BlendSpacePlayerState;
};

/** RotationOffset BlendSpace 노드의 SSR 스냅샷이다. */
struct FAnimSSRRotationOffsetBlendSpaceSnapshot
{
	FAnimSSRNodeIdentity Identity;
	TWeakObjectPtr<UBlendSpace> BlendSpaceAsset;
	uint32 NodeConfigurationHash = 0;
	FAnimNode_RotationOffsetBlendSpaceStateSnapshot RotationOffsetBlendSpaceState;
};

/** BlendListByBool 노드의 SSR 스냅샷이다. */
struct FAnimSSRBlendListByBoolSnapshot
{
	FAnimSSRNodeIdentity Identity;
	int32 BlendPoseCount = 0;
	int32 BlendTimeCount = 0;
	uint32 NodeConfigurationHash = 0;
	TWeakObjectPtr<UCurveFloat> CustomBlendCurve;
	TWeakObjectPtr<UBlendProfile> BlendProfile;
	FAnimNode_BlendListByBoolStateSnapshot BlendListByBoolState;
};

/** StateMachine 노드의 SSR 스냅샷이다. */
struct FAnimSSRStateMachineSnapshot
{
	FAnimSSRNodeIdentity Identity;
	int32 StateMachineIndexInClass = INDEX_NONE;
	FName MachineName = NAME_None;
	int32 StateCount = 0;
	int32 TransitionCount = 0;
	FAnimNode_StateMachineStateSnapshot StateMachineState;
};

/** LayeredBoneBlend 노드의 SSR 스냅샷이다. */
struct FAnimSSRLayeredBoneBlendSnapshot
{
	FAnimSSRNodeIdentity Identity;
	ELayeredBoneBlendMode BlendMode = ELayeredBoneBlendMode::BranchFilter;
	int32 BlendPoseCount = 0;
	int32 BlendWeightCount = 0;
	int32 LayerSetupCount = 0;
	int32 BlendMaskCount = 0;
	uint32 LayerConfigurationHash = 0;
	TArray<TWeakObjectPtr<UBlendProfile>> BlendMasks;
	FAnimNode_LayeredBoneBlendStateSnapshot LayeredBoneBlendState;
};

/** SkeletalControl 계열 노드가 공통으로 저장하는 업데이트 결과다. */
struct FAnimSSRSkeletalControlSnapshot
{
	float ActualAlpha = 0.0f;
};

/** CopyBone 노드의 SSR 스냅샷이다. */
struct FAnimSSRCopyBoneSnapshot
{
	FAnimSSRNodeIdentity Identity;
	FName SourceBoneName = NAME_None;
	FName TargetBoneName = NAME_None;
	TEnumAsByte<EBoneControlSpace> ControlSpace = BCS_ComponentSpace;
	uint32 NodeConfigurationHash = 0;
	FAnimSSRSkeletalControlSnapshot SkeletalControlState;
	bool bCopyTranslation = false;
	bool bCopyRotation = false;
	bool bCopyScale = false;
};

/** TransformBone 노드는 runtime에서 FAnimNode_ModifyBone으로 나타난다. */
struct FAnimSSRTransformBoneSnapshot
{
	FAnimSSRNodeIdentity Identity;
	FName BoneToModifyName = NAME_None;
	TEnumAsByte<EBoneModificationMode> TranslationMode = BMM_Ignore;
	TEnumAsByte<EBoneModificationMode> RotationMode = BMM_Ignore;
	TEnumAsByte<EBoneModificationMode> ScaleMode = BMM_Ignore;
	TEnumAsByte<EBoneControlSpace> TranslationSpace = BCS_ComponentSpace;
	TEnumAsByte<EBoneControlSpace> RotationSpace = BCS_ComponentSpace;
	TEnumAsByte<EBoneControlSpace> ScaleSpace = BCS_ComponentSpace;
	uint32 NodeConfigurationHash = 0;
	FAnimSSRSkeletalControlSnapshot SkeletalControlState;
	FVector Translation = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector Scale = FVector(1.0f);
};

/** TwoBoneIK 노드의 SSR 스냅샷이다. */
struct FAnimSSRTwoBoneIKSnapshot
{
	FAnimSSRNodeIdentity Identity;
	FName IKBoneName = NAME_None;
	FName EffectorTargetName = NAME_None;
	FName JointTargetName = NAME_None;
	bool bEffectorTargetUsesSocket = false;
	bool bJointTargetUsesSocket = false;
	TEnumAsByte<EBoneControlSpace> EffectorLocationSpace = BCS_ComponentSpace;
	TEnumAsByte<EBoneControlSpace> JointTargetLocationSpace = BCS_ComponentSpace;
	bool bAllowStretching = false;
	bool bTakeRotationFromEffectorSpace = false;
	bool bMaintainEffectorRelRot = false;
	bool bAllowTwist = true;
	uint32 NodeConfigurationHash = 0;
	FAnimNode_TwoBoneIKStateSnapshot TwoBoneIKState;
};

/** 한 AnimInstance에서 SSR이 지원하는 모든 node snapshot 묶음이다. */
struct FAnimSSRNodeSnapshotSet
{
	TArray<FAnimSSRSequencePlayerSnapshot> SequencePlayers;
	TArray<FAnimSSRBlendSpacePlayerSnapshot> BlendSpacePlayers;
	TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot> RotationOffsetBlendSpaces;
	TArray<FAnimSSRBlendListByBoolSnapshot> BlendListByBools;
	TArray<FAnimSSRStateMachineSnapshot> StateMachines;
	TArray<FAnimSSRLayeredBoneBlendSnapshot> LayeredBoneBlends;
	TArray<FAnimSSRCopyBoneSnapshot> CopyBones;
	TArray<FAnimSSRTransformBoneSnapshot> TransformBones;
	TArray<FAnimSSRTwoBoneIKSnapshot> TwoBoneIKs;

	/** 지원 node snapshot이 하나도 없는지 확인한다. */
	bool IsEmpty() const
	{
		return SequencePlayers.IsEmpty() &&
			BlendSpacePlayers.IsEmpty() &&
			RotationOffsetBlendSpaces.IsEmpty() &&
			BlendListByBools.IsEmpty() &&
			StateMachines.IsEmpty() &&
			LayeredBoneBlends.IsEmpty() &&
			CopyBones.IsEmpty() &&
			TransformBones.IsEmpty() &&
			TwoBoneIKs.IsEmpty();
	}
};

/** 특정 서버 frame에서 SSR pose 평가에 필요한 actor/mesh/node 상태다. */
struct FAnimSSRFrameSnapshot
{
	double ServerTime = 0.0;
	FTransform ActorTransform = FTransform::Identity;
	FTransform MeshComponentTransform = FTransform::Identity;
	FVector Velocity = FVector::ZeroVector;
	FRotator ControlRotation = FRotator::ZeroRotator;
	FAnimSSRNodeSnapshotSet NodeSnapshots;
};

/** SSR 평가로 생성된 과거 pose 결과다. */
struct FAnimSSRRewindPose
{
	double SelectedServerTime = 0.0;
	FTransform ActorTransform = FTransform::Identity;
	FTransform MeshComponentTransform = FTransform::Identity;
	TArray<FTransform> ComponentSpaceTransforms;
};

/** 보간된 SSR pose를 평가할 때 사용하는 저장 snapshot range다. */
struct FAnimSSRSnapshotRange
{
	const FAnimSSRFrameSnapshot* OlderFrame = nullptr;
	const FAnimSSRFrameSnapshot* YoungerFrame = nullptr;
	double TargetServerTime = 0.0;
	float Alpha = 0.0f;
	bool bSingleFrame = true;
};

/**
 * AnimNode 기반 Snapshot 저장 컴포넌트.
 * 현재는 future 방식으로 보존하며, 서버 hit 판정용 ULyraServerSideRewindComponent에는 연결하지 않는다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class LYRAGAME_API ULyraSnapShotComponent_AnimNode : public ULyraSnapShotComponent
{
	GENERATED_BODY()

public:
	ULyraSnapShotComponent_AnimNode();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual ELyraSnapShotMode GetSnapShotMode() const override { return ELyraSnapShotMode::AnimNode; }
	virtual bool HasAnySnapShot() const override { return SnapShotHistory.Num() > 0; }

	USkeletalMeshComponent* GetSkeletalMesh(AActor* InActor) const;
	void CaptureCurrentSnapShot(AActor* HitActor, FAnimSSRFrameSnapshot& OutFrame);

	/** 호환성/디버그 호출을 위해 저장된 AnimNode snapshot 중 가장 가까운 frame을 찾는다. */
	bool TryGetSnapShotAtTime(float HitTime, FAnimSSRFrameSnapshot& OutFrame) const;
	bool EvaluateSnapShotPose(const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose);

	/** 현재 서버 시간에서 RewindSeconds만큼 과거의 애님 노드 Snapshot pose를 평가한다. */
	bool EvaluatePoseRewindSeconds(float RewindSeconds, FAnimSSRRewindPose& OutPose);

	/** TargetServerTime의 AnimNode snapshot을 평가하고, 가능하면 저장 frame 사이의 bone transform을 보간한다. */
	bool EvaluatePoseAtServerTime(double TargetServerTime, FAnimSSRRewindPose& OutPose);

#if ENABLE_DRAW_DEBUG
	/** AnimNode rewind pose를 PhysicsAsset primitive 기준으로 시각화한다. */
	void DrawDebugPose(const FAnimSSRRewindPose& RewindPose, FColor Color) const;

	/** 현재 mesh pose를 PhysicsAsset primitive 기준으로 시각화한다. */
	void DrawCurrentDebugPose(FColor Color) const;
#endif

protected:
	virtual void UpdateSnapShotHistory() override;

	/** 애님 노드 Snapshot을 history 앞쪽에 저장하고 보관 시간을 넘은 frame을 제거한다. */
	void SaveSnapShotData(FAnimSSRFrameSnapshot Snapshot);

	/** 서버 mesh가 update-only로도 SSR pose 평가에 필요한 anim state를 유지하도록 기본 옵션을 조정한다. */
	UPROPERTY(EditAnywhere, Category = "Server Side Rewind")
	bool bConfigureMeshForSSR = true;

	UPROPERTY(EditAnywhere, Category = "Server Side Rewind|Initialization")
	bool bWaitForExperienceLoaded = true;

	/** 명시하지 않으면 Character owner의 Mesh 또는 첫 SkeletalMeshComponent를 사용한다. */
	UPROPERTY(EditAnywhere, Category = "Server Side Rewind")
	TObjectPtr<USkeletalMeshComponent> TargetMesh = nullptr;

	/** 저장된 snapshot pose를 DrawDebug로 그리는 설정이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Side Rewind|Debug")
	FAnimSSRDebugPoseDrawSettings SavedSnapshotDrawSettings = FAnimSSRDebugPoseDrawSettings(false, 10, 2.0f, FColor::Cyan, 1.0f, 0, true, 2.0f);

	/** 저장된 snapshot의 bone controller 적용/비적용 비교 설정이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Side Rewind|Debug|Bone Controllers")
	FAnimSSRDebugComparisonDrawSettings BoneControllerComparisonDrawSettings;

	/** 서버 현재 mesh pose를 DrawDebug로 그리는 설정이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Side Rewind|Debug|Server Current")
	FAnimSSRDebugPoseDrawSettings ServerCurrentDrawSettings = FAnimSSRDebugPoseDrawSettings(false, 10, 0.1f, FColor::Yellow, 1.0f, 0, true, 2.0f);

	/** 클라이언트 현재 mesh pose를 DrawDebug로 그리는 설정이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Side Rewind|Debug|Client Current")
	FAnimSSRDebugPoseDrawSettings ClientCurrentDrawSettings = FAnimSSRDebugPoseDrawSettings(false, 10, 0.1f, FColor::Green, 1.0f, 0, true, 2.0f);

	/** 최신 frame이 앞쪽에 오도록 저장되는 AnimNode Snapshot history. */
	TDeque<FAnimSSRFrameSnapshot> SnapShotHistory;

private:
	TWeakObjectPtr<USkeletalMeshComponent> CurrentPrerequisiteMesh;
	
	uint64 SavedSnapshotDebugFrameCounter = 0;
	uint64 ServerCurrentMeshPoseDebugFrameCounter = 0;
	uint64 ClientCurrentMeshPoseDebugFrameCounter = 0;
	bool bExperienceLoadedForSSR = false;
	bool bRegisteredForExperienceLoaded = false;
	uint64 ExperienceLoadedFrame = 0;

	void HandleExperienceLoaded(const ULyraExperienceDefinition* Experience);
	void MarkExperienceLoadedForSSR();
	bool UpdateExperienceReady();
	bool IsReadyForSSREvaluation(USkeletalMeshComponent* Mesh, UAnimInstance* AnimInstance);

	USkeletalMeshComponent* ResolveTargetMesh() const;
	void EnsureTickAfterMesh(USkeletalMeshComponent* Mesh);
	void ConfigureMeshForSSR(USkeletalMeshComponent* Mesh);
	bool BuildFrameSnapshot(FAnimSSRFrameSnapshot& OutSnapshot);

	bool CaptureNodeSnapshotSet(UAnimInstance* AnimInstance, FAnimSSRNodeSnapshotSet& OutSnapshots) const;
	bool ValidateNodeSnapshotSet(UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const;

	/** 저장된 node snapshot과 현재 AnimBP가 맞지 않을 때 상세 원인을 디버그 로그에 출력한다. */
	void LogNodeSnapshotSetValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const;

	/** 현재 AnimBP에서 SSR이 지원하지 않는 node를 디버그 로그에 출력한다. */
	void LogUnsupportedSSRNodeProperties(UAnimInstance* AnimInstance) const;
	bool RestoreNodeSnapshotSet(UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const;

	/**
	 * Capture 계열은 cpp 내부 공통 헬퍼로 AnimClass 조회와 node property 순회/캐스팅을 공유한다.
	 * 각 함수는 해당 AnimNode 타입에서 snapshot에 저장할 값만 담당한다.
	 */
	bool CaptureSequencePlayerSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRSequencePlayerSnapshot>& OutSnapshots) const;
	bool CaptureBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRBlendSpacePlayerSnapshot>& OutSnapshots) const;
	bool CaptureRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& OutSnapshots) const;
	bool CaptureBlendListByBoolSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRBlendListByBoolSnapshot>& OutSnapshots) const;
	bool CaptureStateMachineSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRStateMachineSnapshot>& OutSnapshots) const;
	bool CaptureLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRLayeredBoneBlendSnapshot>& OutSnapshots) const;
	bool CaptureCopyBoneSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRCopyBoneSnapshot>& OutSnapshots) const;
	bool CaptureTransformBoneSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRTransformBoneSnapshot>& OutSnapshots) const;
	bool CaptureTwoBoneIKSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRTwoBoneIKSnapshot>& OutSnapshots) const;

	bool ValidateSequencePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRSequencePlayerSnapshot>& Snapshots) const;
	bool ValidateBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendSpacePlayerSnapshot>& Snapshots) const;
	bool ValidateRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& Snapshots) const;
	bool ValidateBlendListByBoolSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendListByBoolSnapshot>& Snapshots) const;
	bool ValidateStateMachineSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRStateMachineSnapshot>& Snapshots) const;
	bool ValidateLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRLayeredBoneBlendSnapshot>& Snapshots) const;
	bool ValidateCopyBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRCopyBoneSnapshot>& Snapshots) const;
	bool ValidateTransformBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTransformBoneSnapshot>& Snapshots) const;
	bool ValidateTwoBoneIKSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTwoBoneIKSnapshot>& Snapshots) const;

	bool RestoreSequencePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRSequencePlayerSnapshot>& Snapshots) const;
	bool RestoreBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendSpacePlayerSnapshot>& Snapshots) const;
	bool RestoreRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& Snapshots) const;
	bool RestoreBlendListByBoolSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendListByBoolSnapshot>& Snapshots) const;
	bool RestoreStateMachineSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRStateMachineSnapshot>& Snapshots) const;
	bool RestoreLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRLayeredBoneBlendSnapshot>& Snapshots) const;
	bool RestoreCopyBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRCopyBoneSnapshot>& Snapshots) const;
	bool RestoreTransformBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTransformBoneSnapshot>& Snapshots) const;
	bool RestoreTwoBoneIKSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTwoBoneIKSnapshot>& Snapshots) const;

	bool TryGetSnapShotRangeAtTime(double TargetServerTime, FAnimSSRSnapshotRange& OutRange) const;
	bool EvaluateFrameSnapshotPose(const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose, bool bDisableCapturedBoneControllers = false);
	bool EvaluateSnapshotPose(USkeletalMeshComponent* Mesh, UAnimInstance* AnimInstance, const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose) const;
	bool InterpolateRewindPoses(const FAnimSSRRewindPose& OlderPose, const FAnimSSRRewindPose& YoungerPose, double TargetServerTime, float Alpha, FAnimSSRRewindPose& OutPose) const;

	/** 저장된 snapshot pose를 설정된 주기에 맞춰 DrawDebug로 출력한다. */
	void DrawSavedSnapshotPose(const FAnimSSRFrameSnapshot& Snapshot);

	/** 서버 권한 world의 현재 mesh pose를 DrawDebug로 출력한다. */
	void DrawServerCurrentMeshPose();

	/** 클라이언트 또는 listen server 화면의 현재 mesh pose를 DrawDebug로 출력한다. */
	void DrawClientCurrentMeshPose();

	/** 전달된 mesh의 현재 component-space pose를 DrawDebug로 출력한다. */
	void DrawCurrentMeshPose(USkeletalMeshComponent* Mesh, const FAnimSSRDebugPoseDrawSettings& DrawSettings) const;

	/** 평가된 rewind pose를 DrawDebug skeleton 형태로 출력한다. */
	void DrawRewindPose(
		const USkeletalMesh* SkeletalMesh,
		const FAnimSSRRewindPose& RewindPose,
		const FAnimSSRDebugPoseDrawSettings& DrawSettings,
		const FVector& WorldOffset,
		const FString& Label) const;
};
