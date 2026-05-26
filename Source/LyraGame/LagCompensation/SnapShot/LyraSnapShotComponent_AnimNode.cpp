// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSnapShotComponent_AnimNode.h"

#include "Animation/AnimClassInterface.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendProfile.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_StateMachine.h"
#include "AnimNodes/AnimNode_AimOffsetLookAt.h"
#include "AnimNodes/AnimNode_BlendListByBool.h"
#include "AnimNodes/AnimNode_BlendListByEnum.h"
#include "AnimNodes/AnimNode_BlendListByInt.h"
#include "AnimNodes/AnimNode_BlendSpaceEvaluator.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "AnimNodes/AnimNode_PoseHandler.h"
#include "AnimNodes/AnimNode_RotationOffsetBlendSpace.h"
#include "AnimNodes/AnimNode_SequenceEvaluator.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AttributesRuntime.h"
#include "BoneControllers/AnimNode_CopyBone.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "Misc/ScopeExit.h"
#include "ReferenceSkeleton.h"
#include "Stats/Stats.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraServerSideRewind, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogLyraServerSideRewindDebug, Log, All);

DECLARE_STATS_GROUP(TEXT("LyraSSR"), STATGROUP_LyraSSR, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("EvaluateFrameSnapshotPose"), STAT_LyraSSR_EvaluateFrameSnapshotPose, STATGROUP_LyraSSR);
DECLARE_CYCLE_STAT(TEXT("DrawSavedSnapshotDebug"), STAT_LyraSSR_DrawSavedSnapshotDebug, STATGROUP_LyraSSR);
DECLARE_CYCLE_STAT(TEXT("DrawServerCurrentMeshPoseDebug"), STAT_LyraSSR_DrawServerCurrentMeshPoseDebug, STATGROUP_LyraSSR);
DECLARE_CYCLE_STAT(TEXT("DrawClientCurrentMeshPoseDebug"), STAT_LyraSSR_DrawClientCurrentMeshPoseDebug, STATGROUP_LyraSSR);

namespace
{
int32 CountStateMachineTransitions(const FBakedAnimationStateMachine* Machine)
{
	if (!Machine)
	{
		return 0;
	}

	int32 TransitionCount = 0;
	for (const FBakedAnimationState& State : Machine->States)
	{
		TransitionCount += State.Transitions.Num();
	}
	return TransitionCount;
}

uint32 HashLayeredBoneBlendConfiguration(const FAnimNode_LayeredBoneBlend& Node)
{
	uint32 Hash = GetTypeHash(static_cast<uint8>(Node.BlendMode));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.BlendPoses.Num()));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.BlendWeights.Num()));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.LayerSetup.Num()));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.BlendMasks.Num()));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.LODThreshold));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.bMeshSpaceRotationBlend));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.bMeshSpaceScaleBlend));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<uint8>(Node.CurveBlendOption.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.bBlendRootMotionBasedOnRootBone));

	for (const FInputBlendPose& InputBlendPose : Node.LayerSetup)
	{
		Hash = HashCombineFast(Hash, GetTypeHash(InputBlendPose.BranchFilters.Num()));
		for (const FBranchFilter& BranchFilter : InputBlendPose.BranchFilters)
		{
			Hash = HashCombineFast(Hash, GetTypeHash(BranchFilter.BoneName));
			Hash = HashCombineFast(Hash, GetTypeHash(BranchFilter.BlendDepth));
		}
	}

	for (const TObjectPtr<UBlendProfile>& BlendMask : Node.BlendMasks)
	{
		Hash = HashCombineFast(Hash, GetTypeHash(GetPathNameSafe(BlendMask.Get())));
	}

	return Hash;
}

uint32 HashInputRange(const FInputRange& Range)
{
	uint32 Hash = GetTypeHash(Range.Min);
	Hash = HashCombineFast(Hash, GetTypeHash(Range.Max));
	return Hash;
}

uint32 HashInputScaleBias(const FInputScaleBias& ScaleBias)
{
	uint32 Hash = GetTypeHash(ScaleBias.Scale);
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBias.Bias));
	return Hash;
}

uint32 HashInputScaleBiasClamp(const FInputScaleBiasClamp& ScaleBiasClamp)
{
	uint32 Hash = GetTypeHash(ScaleBiasClamp.bMapRange);
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.bClampResult));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.bInterpResult));
	Hash = HashCombineFast(Hash, HashInputRange(ScaleBiasClamp.InRange));
	Hash = HashCombineFast(Hash, HashInputRange(ScaleBiasClamp.OutRange));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.Scale));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.Bias));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.ClampMin));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.ClampMax));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.InterpSpeedIncreasing));
	Hash = HashCombineFast(Hash, GetTypeHash(ScaleBiasClamp.InterpSpeedDecreasing));
	return Hash;
}

uint32 HashInputAlphaBoolBlend(const FInputAlphaBoolBlend& AlphaBoolBlend)
{
	uint32 Hash = GetTypeHash(AlphaBoolBlend.BlendInTime);
	Hash = HashCombineFast(Hash, GetTypeHash(AlphaBoolBlend.BlendOutTime));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<uint8>(AlphaBoolBlend.BlendOption)));
	Hash = HashCombineFast(Hash, GetTypeHash(GetPathNameSafe(AlphaBoolBlend.CustomCurve.Get())));
	return Hash;
}

uint32 HashRotationOffsetBlendSpaceConfiguration(const FAnimNode_RotationOffsetBlendSpace& Node)
{
	uint32 Hash = GetTypeHash(Node.LODThreshold);
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<uint8>(Node.AlphaInputType)));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.AlphaCurveName));
	Hash = HashCombineFast(Hash, HashInputScaleBias(Node.AlphaScaleBias));
	Hash = HashCombineFast(Hash, HashInputScaleBiasClamp(Node.AlphaScaleBiasClamp));
	Hash = HashCombineFast(Hash, HashInputAlphaBoolBlend(Node.AlphaBoolBlend));
	return Hash;
}

uint32 HashBlendListByBoolConfiguration(const FAnimNode_BlendListByBool& Node)
{
	uint32 Hash = GetTypeHash(Node.GetBlendPoseCount());
	const TArray<float>& BlendTimes = Node.GetBlendTimes();
	Hash = HashCombineFast(Hash, GetTypeHash(BlendTimes.Num()));
	for (float BlendTime : BlendTimes)
	{
		Hash = HashCombineFast(Hash, GetTypeHash(BlendTime));
	}

	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<uint8>(Node.GetTransitionType())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<uint8>(Node.GetBlendType())));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.GetResetChildOnActivation()));
	Hash = HashCombineFast(Hash, GetTypeHash(GetPathNameSafe(Node.GetCustomBlendCurve())));
	Hash = HashCombineFast(Hash, GetTypeHash(GetPathNameSafe(Node.GetBlendProfile())));
	return Hash;
}

uint32 HashCopyBoneConfiguration(const FAnimNode_CopyBone& Node)
{
	uint32 Hash = GetTypeHash(Node.SourceBone.BoneName);
	Hash = HashCombineFast(Hash, GetTypeHash(Node.TargetBone.BoneName));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.ControlSpace.GetValue())));
	return Hash;
}

uint32 HashTransformBoneConfiguration(const FAnimNode_ModifyBone& Node)
{
	uint32 Hash = GetTypeHash(Node.BoneToModify.BoneName);
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.TranslationMode.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.RotationMode.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.ScaleMode.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.TranslationSpace.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.RotationSpace.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.ScaleSpace.GetValue())));
	return Hash;
}

FName GetBoneSocketTargetSnapshotName(const FBoneSocketTarget& Target)
{
	return Target.bUseSocket ? Target.SocketReference.SocketName : Target.BoneReference.BoneName;
}

uint32 HashTwoBoneIKConfiguration(const FAnimNode_TwoBoneIK& Node)
{
	uint32 Hash = GetTypeHash(Node.IKBone.BoneName);
	Hash = HashCombineFast(Hash, GetTypeHash(Node.StartStretchRatio));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.MaxStretchScale));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.EffectorTarget.bUseSocket));
	Hash = HashCombineFast(Hash, GetTypeHash(GetBoneSocketTargetSnapshotName(Node.EffectorTarget)));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.JointTarget.bUseSocket));
	Hash = HashCombineFast(Hash, GetTypeHash(GetBoneSocketTargetSnapshotName(Node.JointTarget)));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.TwistAxis.Axis));
	Hash = HashCombineFast(Hash, GetTypeHash(Node.TwistAxis.bInLocalSpace));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.EffectorLocationSpace.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<int32>(Node.JointTargetLocationSpace.GetValue())));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<bool>(Node.bAllowStretching)));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<bool>(Node.bTakeRotationFromEffectorSpace)));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<bool>(Node.bMaintainEffectorRelRot)));
	Hash = HashCombineFast(Hash, GetTypeHash(static_cast<bool>(Node.bAllowTwist)));
	return Hash;
}

bool IsSequencePlayerNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_SequencePlayerBase::StaticStruct());
}

bool IsBlendSpacePlayerNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty &&
		NodeProperty->Struct &&
		(NodeProperty->Struct == FAnimNode_BlendSpacePlayer::StaticStruct() ||
			NodeProperty->Struct == FAnimNode_BlendSpacePlayer_Standalone::StaticStruct());
}

bool IsRotationOffsetBlendSpaceNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_RotationOffsetBlendSpace::StaticStruct());
}

bool IsBlendListByBoolNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_BlendListByBool::StaticStruct());
}

bool IsStateMachineNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_StateMachine::StaticStruct());
}

bool IsLayeredBoneBlendNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_LayeredBoneBlend::StaticStruct());
}

bool IsCopyBoneNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_CopyBone::StaticStruct());
}

bool IsTransformBoneNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_ModifyBone::StaticStruct());
}

bool IsTwoBoneIKNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_TwoBoneIK::StaticStruct());
}

bool IsUnsupportedSSRNodeProperty(const FStructProperty* NodeProperty)
{
	if (!NodeProperty || !NodeProperty->Struct)
	{
		return false;
	}

	return NodeProperty->Struct->IsChildOf(FAnimNode_AimOffsetLookAt::StaticStruct()) ||
		NodeProperty->Struct->IsChildOf(FAnimNode_BlendSpaceEvaluator::StaticStruct()) ||
		NodeProperty->Struct->IsChildOf(FAnimNode_SequenceEvaluatorBase::StaticStruct()) ||
		NodeProperty->Struct->IsChildOf(FAnimNode_PoseHandler::StaticStruct()) ||
		NodeProperty->Struct->IsChildOf(FAnimNode_BlendListByEnum::StaticStruct()) ||
		NodeProperty->Struct->IsChildOf(FAnimNode_BlendListByInt::StaticStruct());
}

FString DescribeAnimNodeProperty(const FStructProperty* NodeProperty)
{
	if (!NodeProperty)
	{
		return TEXT("<null property>");
	}

	return FString::Printf(
		TEXT("property=%s struct=%s"),
		*NodeProperty->GetName(),
		*GetNameSafe(NodeProperty->Struct));
}

FString DescribeSnapshotNodeIdentity(int32 NodeIndex, FName NodePropertyName, FName NodeStructName)
{
	return FString::Printf(
		TEXT("nodeIndex=%d property=%s struct=%s"),
		NodeIndex,
		*NodePropertyName.ToString(),
		*NodeStructName.ToString());
}

FString DescribeSnapshotNodeIdentity(const FAnimSSRNodeIdentity& Identity)
{
	return DescribeSnapshotNodeIdentity(Identity.NodeIndex, Identity.NodePropertyName, Identity.NodeStructName);
}

void CaptureSnapshotNodeIdentity(int32 NodeIndex, const FStructProperty* NodeProperty, FAnimSSRNodeIdentity& OutIdentity)
{
	OutIdentity.NodeIndex = NodeIndex;
	OutIdentity.NodePropertyName = NodeProperty ? NodeProperty->GetFName() : NAME_None;
	OutIdentity.NodeStructName = NodeProperty && NodeProperty->Struct ? NodeProperty->Struct->GetFName() : NAME_None;
}

const TArray<FStructProperty*>* GetSSRAnimNodeProperties(UAnimInstance* AnimInstance)
{
	if (!IsValid(AnimInstance))
	{
		return nullptr;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return nullptr;
	}

	return &AnimClass->GetAnimNodeProperties();
}

template <typename NodeType, typename PredicateType, typename CallbackType>
bool ForEachSSRAnimNode(UAnimInstance* AnimInstance, PredicateType Predicate, CallbackType Callback)
{
	const TArray<FStructProperty*>* NodeProperties = GetSSRAnimNodeProperties(AnimInstance);
	if (!NodeProperties)
	{
		return false;
	}

	for (int32 NodeIndex = 0; NodeIndex < NodeProperties->Num(); ++NodeIndex)
	{
		const FStructProperty* NodeProperty = (*NodeProperties)[NodeIndex];
		if (!Predicate(NodeProperty))
		{
			continue;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const NodeType* Node = static_cast<const NodeType*>(NodeMemory);
		if (!Node)
		{
			continue;
		}

		Callback(NodeIndex, NodeProperty, *Node);
	}

	return true;
}

void LogUnsupportedSSRNodeProperties(UAnimInstance* AnimInstance)
{
	if (!IsValid(AnimInstance))
	{
		return;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	for (int32 NodeIndex = 0; NodeIndex < NodeProperties.Num(); ++NodeIndex)
	{
		const FStructProperty* NodeProperty = NodeProperties[NodeIndex];
		if (!IsUnsupportedSSRNodeProperty(NodeProperty))
		{
			continue;
		}

		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("SSR unsupported animation node skipped. AnimClass=%s nodeIndex=%d %s"),
			*GetNameSafe(AnimInstance->GetClass()),
			NodeIndex,
			*DescribeAnimNodeProperty(NodeProperty));
	}
}

bool ShouldDrawDebugPose(bool bRequested, const FAnimSSRDebugPoseDrawSettings& DrawSettings, uint64& Counter)
{
	if (!bRequested)
	{
		return false;
	}

	++Counter;
	const uint64 DrawInterval = static_cast<uint64>(FMath::Max(1, DrawSettings.FrameInterval));
	return (Counter % DrawInterval) == 0;
}

void DrawPoseSkeleton(
	const UWorld* World,
	const USkeletalMesh* SkeletalMesh,
	const TArray<FTransform>& ComponentSpaceTransforms,
	const FTransform& MeshComponentTransform,
	const FAnimSSRDebugPoseDrawSettings& DrawSettings,
	const FVector& WorldOffset,
	const FString& Label)
{
	if (!IsValid(World) || !IsValid(SkeletalMesh))
	{
		return;
	}

	const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
	const int32 BoneCountFromPose = FMath::Min(RefSkeleton.GetNum(), ComponentSpaceTransforms.Num());
	const int32 BoneCountToDraw = DrawSettings.MaxDebugBones > 0
		? FMath::Min(BoneCountFromPose, DrawSettings.MaxDebugBones)
		: BoneCountFromPose;
	if (BoneCountToDraw <= 0)
	{
		return;
	}

	FVector LabelLocation = FVector::ZeroVector;
	for (int32 BoneIndex = 0; BoneIndex < BoneCountToDraw; ++BoneIndex)
	{
		const FVector BoneLocation = MeshComponentTransform.TransformPosition(ComponentSpaceTransforms[BoneIndex].GetLocation()) + WorldOffset;
		if (BoneIndex == 0)
		{
			LabelLocation = BoneLocation + FVector(0.0f, 0.0f, FMath::Max(20.0f, DrawSettings.JointRadius * 4.0f));
		}

		if (DrawSettings.bDrawJoints && DrawSettings.JointRadius > 0.0f)
		{
			DrawDebugSphere(World, BoneLocation, DrawSettings.JointRadius, 8, DrawSettings.Color, false, DrawSettings.Duration, 0, DrawSettings.Thickness);
		}

		const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
		if (ParentIndex != INDEX_NONE && ParentIndex < BoneCountToDraw)
		{
			const FVector ParentLocation = MeshComponentTransform.TransformPosition(ComponentSpaceTransforms[ParentIndex].GetLocation()) + WorldOffset;
			DrawDebugLine(World, ParentLocation, BoneLocation, DrawSettings.Color, false, DrawSettings.Duration, 0, DrawSettings.Thickness);
		}
	}

	if (!Label.IsEmpty())
	{
		DrawDebugString(World, LabelLocation, Label, nullptr, DrawSettings.Color, DrawSettings.Duration, false);
	}
}

template <typename PredicateType>
int32 CountMatchingAnimNodeProperties(const TArray<FStructProperty*>& NodeProperties, PredicateType Predicate)
{
	int32 Count = 0;
	for (const FStructProperty* NodeProperty : NodeProperties)
	{
		if (Predicate(NodeProperty))
		{
			++Count;
		}
	}
	return Count;
}

void CaptureSkeletalControlSnapshot(const FAnimNode_SkeletalControlBase& Node, FAnimSSRSkeletalControlSnapshot& OutSnapshot)
{
	OutSnapshot.ActualAlpha = Node.GetAlpha();
}

void RestoreSkeletalControlSnapshot(FAnimNode_SkeletalControlBase& Node, const FAnimSSRSkeletalControlSnapshot& Snapshot)
{
	Node.SetAlpha(Snapshot.ActualAlpha);
}

void DisableCapturedBoneControllerSnapshots(FAnimSSRFrameSnapshot& Snapshot)
{
	for (FAnimSSRCopyBoneSnapshot& CopyBoneSnapshot : Snapshot.NodeSnapshots.CopyBones)
	{
		CopyBoneSnapshot.SkeletalControlState.ActualAlpha = 0.0f;
	}

	for (FAnimSSRTransformBoneSnapshot& TransformBoneSnapshot : Snapshot.NodeSnapshots.TransformBones)
	{
		TransformBoneSnapshot.SkeletalControlState.ActualAlpha = 0.0f;
	}

	for (FAnimSSRTwoBoneIKSnapshot& TwoBoneIKSnapshot : Snapshot.NodeSnapshots.TwoBoneIKs)
	{
		TwoBoneIKSnapshot.TwoBoneIKState.ActualAlpha = 0.0f;
	}
}

FString MakeBoneControllerComparisonLabel(const TCHAR* StateLabel, const FAnimSSRFrameSnapshot& Snapshot)
{
	return FString::Printf(
		TEXT("SSR C/T/IK %s  C=%d T=%d IK=%d"),
		StateLabel,
		Snapshot.NodeSnapshots.CopyBones.Num(),
		Snapshot.NodeSnapshots.TransformBones.Num(),
		Snapshot.NodeSnapshots.TwoBoneIKs.Num());
}

const IAnimClassInterface* GetAnimClassForSSRValidation(const FString& OwnerName, const TCHAR* NodeTypeName, UAnimInstance* AnimInstance)
{
	if (!IsValid(AnimInstance))
	{
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: %s validation failed: AnimInstance is null."),
			*OwnerName,
			NodeTypeName);
		return nullptr;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: %s validation failed: AnimClassInterface is null. AnimClass=%s"),
			*OwnerName,
			NodeTypeName,
			*GetNameSafe(AnimInstance->GetClass()));
		return nullptr;
	}

	return AnimClass;
}

bool LogSequencePlayerSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRSequencePlayerSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("SequencePlayer"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsSequencePlayerNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRSequencePlayerSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsSequencePlayerNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_SequencePlayerBase* SequencePlayer = static_cast<const FAnimNode_SequencePlayerBase*>(NodeMemory);
		if (!SequencePlayer)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (SequencePlayer->GetSequence() != Snapshot.SequenceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: SequencePlayer snapshot[%d] sequence asset differs. node=%s snapshotAsset=%s currentAsset=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*GetPathNameSafe(Snapshot.SequenceAsset.Get()),
				*GetPathNameSafe(SequencePlayer->GetSequence()));
		}
	}

	return bMatches;
}

bool LogBlendSpacePlayerSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendSpacePlayerSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("BlendSpacePlayer"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsBlendSpacePlayerNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRBlendSpacePlayerSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsBlendSpacePlayerNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_BlendSpacePlayerBase* BlendSpacePlayer = static_cast<const FAnimNode_BlendSpacePlayerBase*>(NodeMemory);
		if (!BlendSpacePlayer)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (BlendSpacePlayer->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendSpacePlayer snapshot[%d] blend space asset differs. node=%s snapshotAsset=%s currentAsset=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*GetPathNameSafe(Snapshot.BlendSpaceAsset.Get()),
				*GetPathNameSafe(BlendSpacePlayer->GetBlendSpace()));
		}
	}

	return bMatches;
}

bool LogRotationOffsetBlendSpaceSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("RotationOffsetBlendSpace"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsRotationOffsetBlendSpaceNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRRotationOffsetBlendSpaceSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsRotationOffsetBlendSpaceNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_RotationOffsetBlendSpace* RotationOffsetBlendSpace = static_cast<const FAnimNode_RotationOffsetBlendSpace*>(NodeMemory);
		if (!RotationOffsetBlendSpace)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (RotationOffsetBlendSpace->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] blend space asset differs. node=%s snapshotAsset=%s currentAsset=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*GetPathNameSafe(Snapshot.BlendSpaceAsset.Get()),
				*GetPathNameSafe(RotationOffsetBlendSpace->GetBlendSpace()));
		}

		const uint32 CurrentHash = HashRotationOffsetBlendSpaceConfiguration(*RotationOffsetBlendSpace);
		if (CurrentHash != Snapshot.NodeConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: RotationOffsetBlendSpace snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.NodeConfigurationHash,
				CurrentHash);
		}
	}

	return bMatches;
}

bool LogBlendListByBoolSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendListByBoolSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("BlendListByBool"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsBlendListByBoolNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRBlendListByBoolSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsBlendListByBoolNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_BlendListByBool* BlendListByBool = static_cast<const FAnimNode_BlendListByBool*>(NodeMemory);
		if (!BlendListByBool)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		const int32 CurrentBlendPoseCount = BlendListByBool->GetBlendPoseCount();
		const int32 CurrentBlendTimeCount = BlendListByBool->GetBlendTimes().Num();
		const uint32 CurrentHash = HashBlendListByBoolConfiguration(*BlendListByBool);
		if (CurrentBlendPoseCount != Snapshot.BlendPoseCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] BlendPoseCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendPoseCount,
				CurrentBlendPoseCount);
		}
		if (CurrentBlendTimeCount != Snapshot.BlendTimeCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] BlendTimeCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendTimeCount,
				CurrentBlendTimeCount);
		}
		if (CurrentHash != Snapshot.NodeConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.NodeConfigurationHash,
				CurrentHash);
		}
		if (BlendListByBool->GetCustomBlendCurve() != Snapshot.CustomBlendCurve.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] custom curve differs. node=%s snapshotCurve=%s currentCurve=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*GetPathNameSafe(Snapshot.CustomBlendCurve.Get()),
				*GetPathNameSafe(BlendListByBool->GetCustomBlendCurve()));
		}
		if (BlendListByBool->GetBlendProfile() != Snapshot.BlendProfile.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] blend profile differs. node=%s snapshotProfile=%s currentProfile=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*GetPathNameSafe(Snapshot.BlendProfile.Get()),
				*GetPathNameSafe(BlendListByBool->GetBlendProfile()));
		}
		const int32 CapturedPerBlendDataCount = Snapshot.BlendListByBoolState.BlendListBaseState.PerBlendData.Num();
		if (CapturedPerBlendDataCount > 0 && CapturedPerBlendDataCount != Snapshot.BlendPoseCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] captured PerBlendData count is inconsistent. node=%s perBlendData=%d blendPoseCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				CapturedPerBlendDataCount,
				Snapshot.BlendPoseCount);
		}

		const int32 CapturedPerBoneSampleDataCount = Snapshot.BlendListByBoolState.BlendListBaseState.PerBoneSampleData.Num();
		if (Snapshot.BlendProfile.IsValid() && CapturedPerBoneSampleDataCount > 0 && CapturedPerBoneSampleDataCount != Snapshot.BlendPoseCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: BlendListByBool snapshot[%d] captured PerBoneSampleData count is inconsistent. node=%s perBoneSampleData=%d blendPoseCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				CapturedPerBoneSampleDataCount,
				Snapshot.BlendPoseCount);
		}
	}

	return bMatches;
}

bool LogStateMachineSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRStateMachineSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("StateMachine"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsStateMachineNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: StateMachine count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRStateMachineSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsStateMachineNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_StateMachine* StateMachine = static_cast<const FAnimNode_StateMachine*>(NodeMemory);
		if (!StateMachine)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (StateMachine->StateMachineIndexInClass != Snapshot.StateMachineIndexInClass)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] StateMachineIndexInClass differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.StateMachineIndexInClass,
				StateMachine->StateMachineIndexInClass);
		}

		const FBakedAnimationStateMachine* MachineDescription = StateMachine->GetMachineDescription();
		if (!MachineDescription)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] has no current machine description. node=%s snapshotMachine=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*Snapshot.MachineName.ToString());
			continue;
		}

		const int32 CurrentTransitionCount = CountStateMachineTransitions(MachineDescription);
		if (MachineDescription->MachineName != Snapshot.MachineName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] machine name differs. node=%s snapshot=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*Snapshot.MachineName.ToString(),
				*MachineDescription->MachineName.ToString());
		}
		if (MachineDescription->States.Num() != Snapshot.StateCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] state count differs. node=%s snapshot=%d current=%d machine=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.StateCount,
				MachineDescription->States.Num(),
				*MachineDescription->MachineName.ToString());
		}
		if (CurrentTransitionCount != Snapshot.TransitionCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: StateMachine snapshot[%d] transition count differs. node=%s snapshot=%d current=%d machine=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.TransitionCount,
				CurrentTransitionCount,
				*MachineDescription->MachineName.ToString());
		}
	}

	return bMatches;
}

bool LogLayeredBoneBlendSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRLayeredBoneBlendSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("LayeredBoneBlend"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsLayeredBoneBlendNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRLayeredBoneBlendSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsLayeredBoneBlendNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_LayeredBoneBlend* LayeredBoneBlend = static_cast<const FAnimNode_LayeredBoneBlend*>(NodeMemory);
		if (!LayeredBoneBlend)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		const uint32 CurrentHash = HashLayeredBoneBlendConfiguration(*LayeredBoneBlend);
		if (LayeredBoneBlend->BlendMode != Snapshot.BlendMode)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] blend mode differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				static_cast<int32>(Snapshot.BlendMode),
				static_cast<int32>(LayeredBoneBlend->BlendMode));
		}
		if (LayeredBoneBlend->BlendPoses.Num() != Snapshot.BlendPoseCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] BlendPoseCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendPoseCount,
				LayeredBoneBlend->BlendPoses.Num());
		}
		if (LayeredBoneBlend->BlendWeights.Num() != Snapshot.BlendWeightCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] BlendWeightCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendWeightCount,
				LayeredBoneBlend->BlendWeights.Num());
		}
		if (LayeredBoneBlend->LayerSetup.Num() != Snapshot.LayerSetupCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] LayerSetupCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.LayerSetupCount,
				LayeredBoneBlend->LayerSetup.Num());
		}
		if (LayeredBoneBlend->BlendMasks.Num() != Snapshot.BlendMaskCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] BlendMaskCount differs. node=%s snapshot=%d current=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendMaskCount,
				LayeredBoneBlend->BlendMasks.Num());
		}
		if (CurrentHash != Snapshot.LayerConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.LayerConfigurationHash,
				CurrentHash);
		}
		if (Snapshot.LayeredBoneBlendState.BlendWeights.Num() != Snapshot.BlendWeightCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] captured BlendWeights count is inconsistent. node=%s captured=%d expected=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.LayeredBoneBlendState.BlendWeights.Num(),
				Snapshot.BlendWeightCount);
		}
		if (Snapshot.BlendMasks.Num() != Snapshot.BlendMaskCount)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] captured BlendMasks count is inconsistent. node=%s captured=%d expected=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.BlendMasks.Num(),
				Snapshot.BlendMaskCount);
		}

		const int32 ComparableBlendMaskCount = FMath::Min(Snapshot.BlendMasks.Num(), LayeredBoneBlend->BlendMasks.Num());
		for (int32 BlendMaskIndex = 0; BlendMaskIndex < ComparableBlendMaskCount; ++BlendMaskIndex)
		{
			if (LayeredBoneBlend->BlendMasks[BlendMaskIndex].Get() != Snapshot.BlendMasks[BlendMaskIndex].Get())
			{
				bMatches = false;
				UE_LOG(LogLyraServerSideRewindDebug, Verbose,
					TEXT("[%s] SSR snapshot mismatch detail: LayeredBoneBlend snapshot[%d] blend mask[%d] differs. node=%s snapshotMask=%s currentMask=%s"),
					*OwnerName,
					SnapshotIndex,
					BlendMaskIndex,
					*SnapshotIdentity,
					*GetPathNameSafe(Snapshot.BlendMasks[BlendMaskIndex].Get()),
					*GetPathNameSafe(LayeredBoneBlend->BlendMasks[BlendMaskIndex].Get()));
			}
		}
	}

	return bMatches;
}

bool LogCopyBoneSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRCopyBoneSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("CopyBone"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsCopyBoneNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: CopyBone count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRCopyBoneSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: CopyBone snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsCopyBoneNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: CopyBone snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: CopyBone snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_CopyBone* CopyBone = static_cast<const FAnimNode_CopyBone*>(NodeMemory);
		if (!CopyBone)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: CopyBone snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		const uint32 CurrentHash = HashCopyBoneConfiguration(*CopyBone);
		if (CurrentHash != Snapshot.NodeConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: CopyBone snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u snapshotSource=%s currentSource=%s snapshotTarget=%s currentTarget=%s snapshotSpace=%d currentSpace=%d snapshotCopyTRS={%d,%d,%d} currentCopyTRS={%d,%d,%d}"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.NodeConfigurationHash,
				CurrentHash,
				*Snapshot.SourceBoneName.ToString(),
				*CopyBone->SourceBone.BoneName.ToString(),
				*Snapshot.TargetBoneName.ToString(),
				*CopyBone->TargetBone.BoneName.ToString(),
				static_cast<int32>(Snapshot.ControlSpace.GetValue()),
				static_cast<int32>(CopyBone->ControlSpace.GetValue()),
				Snapshot.bCopyTranslation ? 1 : 0,
				Snapshot.bCopyRotation ? 1 : 0,
				Snapshot.bCopyScale ? 1 : 0,
				CopyBone->bCopyTranslation ? 1 : 0,
				CopyBone->bCopyRotation ? 1 : 0,
				CopyBone->bCopyScale ? 1 : 0);
		}
	}

	return bMatches;
}

bool LogTransformBoneSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRTransformBoneSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("TransformBone"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsTransformBoneNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: TransformBone count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRTransformBoneSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TransformBone snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsTransformBoneNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TransformBone snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TransformBone snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_ModifyBone* TransformBone = static_cast<const FAnimNode_ModifyBone*>(NodeMemory);
		if (!TransformBone)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TransformBone snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		const uint32 CurrentHash = HashTransformBoneConfiguration(*TransformBone);
		if (CurrentHash != Snapshot.NodeConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TransformBone snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u snapshotBone=%s currentBone=%s snapshotModes={%d,%d,%d} currentModes={%d,%d,%d} snapshotSpaces={%d,%d,%d} currentSpaces={%d,%d,%d}"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.NodeConfigurationHash,
				CurrentHash,
				*Snapshot.BoneToModifyName.ToString(),
				*TransformBone->BoneToModify.BoneName.ToString(),
				static_cast<int32>(Snapshot.TranslationMode.GetValue()),
				static_cast<int32>(Snapshot.RotationMode.GetValue()),
				static_cast<int32>(Snapshot.ScaleMode.GetValue()),
				static_cast<int32>(TransformBone->TranslationMode.GetValue()),
				static_cast<int32>(TransformBone->RotationMode.GetValue()),
				static_cast<int32>(TransformBone->ScaleMode.GetValue()),
				static_cast<int32>(Snapshot.TranslationSpace.GetValue()),
				static_cast<int32>(Snapshot.RotationSpace.GetValue()),
				static_cast<int32>(Snapshot.ScaleSpace.GetValue()),
				static_cast<int32>(TransformBone->TranslationSpace.GetValue()),
				static_cast<int32>(TransformBone->RotationSpace.GetValue()),
				static_cast<int32>(TransformBone->ScaleSpace.GetValue()));
		}
	}

	return bMatches;
}

bool LogTwoBoneIKSnapshotValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const TArray<FAnimSSRTwoBoneIKSnapshot>& Snapshots)
{
	const IAnimClassInterface* AnimClass = GetAnimClassForSSRValidation(OwnerName, TEXT("TwoBoneIK"), AnimInstance);
	if (!AnimClass)
	{
		return false;
	}

	bool bMatches = true;
	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentCount = CountMatchingAnimNodeProperties(NodeProperties, IsTwoBoneIKNodeProperty);
	if (CurrentCount != Snapshots.Num())
	{
		bMatches = false;
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK count differs. snapshot=%d current=%d AnimClass=%s"),
			*OwnerName,
			Snapshots.Num(),
			CurrentCount,
			*GetNameSafe(AnimInstance->GetClass()));
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRTwoBoneIKSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FString SnapshotIdentity = DescribeSnapshotNodeIdentity(Snapshot.Identity);
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK snapshot[%d] has invalid node index. %s currentNodeCount=%d"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				NodeProperties.Num());
			continue;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsTwoBoneIKNodeProperty(NodeProperty))
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK snapshot[%d] points to unsupported node type. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
			continue;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK snapshot[%d] node identity differs. expected=%s current=%s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				*DescribeAnimNodeProperty(NodeProperty));
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_TwoBoneIK* TwoBoneIK = static_cast<const FAnimNode_TwoBoneIK*>(NodeMemory);
		if (!TwoBoneIK)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK snapshot[%d] node memory is null. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		const uint32 CurrentHash = HashTwoBoneIKConfiguration(*TwoBoneIK);
		if (CurrentHash != Snapshot.NodeConfigurationHash)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR snapshot mismatch detail: TwoBoneIK snapshot[%d] configuration hash differs. node=%s snapshotHash=%u currentHash=%u snapshotIKBone=%s currentIKBone=%s snapshotEffector={%s,%d} currentEffector={%s,%d} snapshotJoint={%s,%d} currentJoint={%s,%d} snapshotSpaces={%d,%d} currentSpaces={%d,%d} snapshotFlags={%d,%d,%d,%d} currentFlags={%d,%d,%d,%d}"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity,
				Snapshot.NodeConfigurationHash,
				CurrentHash,
				*Snapshot.IKBoneName.ToString(),
				*TwoBoneIK->IKBone.BoneName.ToString(),
				*Snapshot.EffectorTargetName.ToString(),
				Snapshot.bEffectorTargetUsesSocket ? 1 : 0,
				*GetBoneSocketTargetSnapshotName(TwoBoneIK->EffectorTarget).ToString(),
				TwoBoneIK->EffectorTarget.bUseSocket ? 1 : 0,
				*Snapshot.JointTargetName.ToString(),
				Snapshot.bJointTargetUsesSocket ? 1 : 0,
				*GetBoneSocketTargetSnapshotName(TwoBoneIK->JointTarget).ToString(),
				TwoBoneIK->JointTarget.bUseSocket ? 1 : 0,
				static_cast<int32>(Snapshot.EffectorLocationSpace.GetValue()),
				static_cast<int32>(Snapshot.JointTargetLocationSpace.GetValue()),
				static_cast<int32>(TwoBoneIK->EffectorLocationSpace.GetValue()),
				static_cast<int32>(TwoBoneIK->JointTargetLocationSpace.GetValue()),
				Snapshot.bAllowStretching ? 1 : 0,
				Snapshot.bTakeRotationFromEffectorSpace ? 1 : 0,
				Snapshot.bMaintainEffectorRelRot ? 1 : 0,
				Snapshot.bAllowTwist ? 1 : 0,
				TwoBoneIK->bAllowStretching ? 1 : 0,
				TwoBoneIK->bTakeRotationFromEffectorSpace ? 1 : 0,
				TwoBoneIK->bMaintainEffectorRelRot ? 1 : 0,
				TwoBoneIK->bAllowTwist ? 1 : 0);
		}
	}

	return bMatches;
}

class FScopedSSRAnimInstanceEvaluation
{
public:
	FScopedSSRAnimInstanceEvaluation(USkeletalMeshComponent* Mesh, UAnimInstance* MainAnimInstance)
	{
		AddAnimInstance(MainAnimInstance);

		if (Mesh)
		{
			const USkeletalMeshComponent* ConstMesh = Mesh;
			for (UAnimInstance* LinkedAnimInstance : ConstMesh->GetLinkedAnimInstances())
			{
				AddAnimInstance(LinkedAnimInstance);
			}

			if (Mesh->ShouldEvaluatePostProcessInstance())
			{
				AddAnimInstance(Mesh->GetPostProcessInstance());
			}
		}

		for (UAnimInstance* AnimInstance : AnimInstances)
		{
			AnimInstance->PreEvaluateAnimation();
		}
	}

	~FScopedSSRAnimInstanceEvaluation()
	{
		for (UAnimInstance* AnimInstance : AnimInstances)
		{
			if (AnimInstance)
			{
				AnimInstance->PostEvaluateAnimation();
			}
		}
	}

private:
	void AddAnimInstance(UAnimInstance* AnimInstance)
	{
		if (AnimInstance && !AnimInstances.Contains(AnimInstance))
		{
			AnimInstances.Add(AnimInstance);
		}
	}

	TArray<UAnimInstance*> AnimInstances;
};

}

ULyraSnapShotComponent_AnimNode::ULyraSnapShotComponent_AnimNode()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	MaxRecordTime = 0.5f;
}

void ULyraSnapShotComponent_AnimNode::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	EnsureTickAfterMesh(Mesh);
	UpdateExperienceReady();

	AActor* Owner = GetOwner();
	if (IsValid(Owner) && Owner->HasAuthority())
	{
		ConfigureMeshForSSR(Mesh);
	}
}

void ULyraSnapShotComponent_AnimNode::HandleExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
	(void)Experience;
	MarkExperienceLoadedForSSR();
}

void ULyraSnapShotComponent_AnimNode::MarkExperienceLoadedForSSR()
{
	if (!bExperienceLoadedForSSR)
	{
		ExperienceLoadedFrame = GFrameCounter;
	}

	bExperienceLoadedForSSR = true;
}

bool ULyraSnapShotComponent_AnimNode::UpdateExperienceReady()
{
	if (!bWaitForExperienceLoaded || bExperienceLoadedForSSR)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		MarkExperienceLoadedForSSR();
		return true;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!IsValid(GameState))
	{
		return false;
	}

	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	if (!IsValid(ExperienceComponent))
	{
		MarkExperienceLoadedForSSR();
		return true;
	}

	if (ExperienceComponent->IsExperienceLoaded())
	{
		MarkExperienceLoadedForSSR();
		return true;
	}

	if (!bRegisteredForExperienceLoaded)
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::HandleExperienceLoaded));
		bRegisteredForExperienceLoaded = true;
	}

	return bExperienceLoadedForSSR;
}

bool ULyraSnapShotComponent_AnimNode::IsReadyForSSREvaluation(USkeletalMeshComponent* Mesh, UAnimInstance* AnimInstance)
{
	(void)Mesh;
	(void)AnimInstance;

	if (!UpdateExperienceReady())
	{
		return false;
	}

	if (!bWaitForExperienceLoaded)
	{
		return true;
	}

	return GFrameCounter > ExperienceLoadedFrame;
}

void ULyraSnapShotComponent_AnimNode::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	EnsureTickAfterMesh(Mesh);

	if (!Owner->HasAuthority())
	{
		DrawClientCurrentMeshPose();
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	ConfigureMeshForSSR(Mesh);
	DrawServerCurrentMeshPose();
	if (Owner->GetNetMode() == NM_ListenServer)
	{
		// ListenServer는 authority world가 화면에 보이므로 client-current debug도 같은 tick에서 호출한다.
		DrawClientCurrentMeshPose();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

USkeletalMeshComponent* ULyraSnapShotComponent_AnimNode::GetSkeletalMesh(AActor* InActor) const
{
	if (!IsValid(InActor))
	{
		return nullptr;
	}

	if (InActor == GetOwner() && IsValid(TargetMesh.Get()))
	{
		return TargetMesh.Get();
	}

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(InActor))
	{
		return CharacterOwner->GetMesh();
	}

	return InActor->FindComponentByClass<USkeletalMeshComponent>();
}

void ULyraSnapShotComponent_AnimNode::CaptureCurrentSnapShot(AActor* HitActor, FAnimSSRFrameSnapshot& OutFrame)
{
	OutFrame = FAnimSSRFrameSnapshot();
	if (!IsValid(HitActor) || HitActor != GetOwner())
	{
		return;
	}

	if (!BuildFrameSnapshot(OutFrame))
	{
		OutFrame = FAnimSSRFrameSnapshot();
	}
}

bool ULyraSnapShotComponent_AnimNode::TryGetSnapShotAtTime(float HitTime, FAnimSSRFrameSnapshot& OutFrame) const
{
	OutFrame = FAnimSSRFrameSnapshot();
	FAnimSSRSnapshotRange Range;
	if (!TryGetSnapShotRangeAtTime(static_cast<double>(HitTime), Range))
	{
		return false;
	}

	const FAnimSSRFrameSnapshot* SelectedFrame = Range.YoungerFrame;
	if (!Range.bSingleFrame)
	{
		if (!Range.OlderFrame || !Range.YoungerFrame)
		{
			return false;
		}

		const double YoungerDistance = FMath::Abs(Range.YoungerFrame->ServerTime - Range.TargetServerTime);
		const double OlderDistance = FMath::Abs(Range.TargetServerTime - Range.OlderFrame->ServerTime);
		SelectedFrame = YoungerDistance <= OlderDistance ? Range.YoungerFrame : Range.OlderFrame;
	}

	if (!SelectedFrame)
	{
		return false;
	}

	OutFrame = *SelectedFrame;
	return !OutFrame.NodeSnapshots.IsEmpty();
}

bool ULyraSnapShotComponent_AnimNode::TryGetSnapShotRangeAtTime(double TargetServerTime, FAnimSSRSnapshotRange& OutRange) const
{
	OutRange = FAnimSSRSnapshotRange();
	OutRange.TargetServerTime = TargetServerTime;

	if (SnapShotHistory.Num() == 0)
	{
		return false;
	}

	const double OldestTime = SnapShotHistory.Last().ServerTime;
	const double NewestTime = SnapShotHistory.First().ServerTime;
	const bool bIsAtOldest = FMath::Abs(OldestTime - TargetServerTime) <= KINDA_SMALL_NUMBER;
	const bool bIsAtNewest = FMath::Abs(NewestTime - TargetServerTime) <= KINDA_SMALL_NUMBER;

	auto SetSingleFrame = [&OutRange](const FAnimSSRFrameSnapshot& Frame) -> bool
	{
		OutRange.OlderFrame = &Frame;
		OutRange.YoungerFrame = &Frame;
		OutRange.Alpha = 0.0f;
		OutRange.bSingleFrame = true;
		return !Frame.NodeSnapshots.IsEmpty();
	};

	if (OldestTime > TargetServerTime && !bIsAtOldest)
	{
		return false;
	}

	if (bIsAtOldest)
	{
		return SetSingleFrame(SnapShotHistory.Last());
	}

	if (NewestTime <= TargetServerTime || bIsAtNewest)
	{
		return SetSingleFrame(SnapShotHistory.First());
	}

	const int32 Count = SnapShotHistory.Num();
	int32 YoungerIdx = 0;
	while (YoungerIdx + 1 < Count && SnapShotHistory[YoungerIdx + 1].ServerTime > TargetServerTime)
	{
		++YoungerIdx;
	}

	if (YoungerIdx + 1 >= Count)
	{
		return SetSingleFrame(SnapShotHistory.Last());
	}

	const FAnimSSRFrameSnapshot& YoungerFrame = SnapShotHistory[YoungerIdx];
	const FAnimSSRFrameSnapshot& OlderFrame = SnapShotHistory[YoungerIdx + 1];
	const double DeltaTime = YoungerFrame.ServerTime - OlderFrame.ServerTime;
	if (FMath::IsNearlyZero(DeltaTime))
	{
		return SetSingleFrame(YoungerFrame);
	}

	const double Alpha = FMath::Clamp((TargetServerTime - OlderFrame.ServerTime) / DeltaTime, 0.0, 1.0);
	if (Alpha <= KINDA_SMALL_NUMBER)
	{
		return SetSingleFrame(OlderFrame);
	}
	if ((1.0 - Alpha) <= KINDA_SMALL_NUMBER)
	{
		return SetSingleFrame(YoungerFrame);
	}

	OutRange.OlderFrame = &OlderFrame;
	OutRange.YoungerFrame = &YoungerFrame;
	OutRange.Alpha = static_cast<float>(Alpha);
	OutRange.bSingleFrame = false;
	return !OlderFrame.NodeSnapshots.IsEmpty() && !YoungerFrame.NodeSnapshots.IsEmpty();
}

bool ULyraSnapShotComponent_AnimNode::EvaluateSnapShotPose(const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose)
{
	return EvaluateFrameSnapshotPose(Snapshot, OutPose);
}

bool ULyraSnapShotComponent_AnimNode::EvaluatePoseRewindSeconds(float RewindSeconds, FAnimSSRRewindPose& OutPose)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	return EvaluatePoseAtServerTime(World->GetTimeSeconds() - FMath::Max(0.0f, RewindSeconds), OutPose);
}

bool ULyraSnapShotComponent_AnimNode::EvaluatePoseAtServerTime(double TargetServerTime, FAnimSSRRewindPose& OutPose)
{
	OutPose = FAnimSSRRewindPose();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return false;
	}

	FAnimSSRSnapshotRange Range;
	if (!TryGetSnapShotRangeAtTime(TargetServerTime, Range))
	{
		return false;
	}

	if (Range.bSingleFrame)
	{
		return Range.YoungerFrame ? EvaluateSnapShotPose(*Range.YoungerFrame, OutPose) : false;
	}

	if (!Range.OlderFrame || !Range.YoungerFrame)
	{
		return false;
	}

	FAnimSSRRewindPose OlderPose;
	const bool bEvaluatedOlderPose = EvaluateFrameSnapshotPose(*Range.OlderFrame, OlderPose);

	FAnimSSRRewindPose YoungerPose;
	const bool bEvaluatedYoungerPose = EvaluateFrameSnapshotPose(*Range.YoungerFrame, YoungerPose);

	if (bEvaluatedOlderPose &&
		bEvaluatedYoungerPose &&
		InterpolateRewindPoses(OlderPose, YoungerPose, TargetServerTime, Range.Alpha, OutPose))
	{
		return true;
	}

	const double YoungerDistance = FMath::Abs(Range.YoungerFrame->ServerTime - TargetServerTime);
	const double OlderDistance = FMath::Abs(TargetServerTime - Range.OlderFrame->ServerTime);
	const FAnimSSRFrameSnapshot* NearestFrame = YoungerDistance <= OlderDistance ? Range.YoungerFrame : Range.OlderFrame;

	if (NearestFrame == Range.YoungerFrame && bEvaluatedYoungerPose)
	{
		OutPose = MoveTemp(YoungerPose);
		return true;
	}
	if (NearestFrame == Range.OlderFrame && bEvaluatedOlderPose)
	{
		OutPose = MoveTemp(OlderPose);
		return true;
	}

	const FAnimSSRFrameSnapshot* OtherFrame = NearestFrame == Range.YoungerFrame ? Range.OlderFrame : Range.YoungerFrame;
	if (OtherFrame == Range.YoungerFrame && bEvaluatedYoungerPose)
	{
		OutPose = MoveTemp(YoungerPose);
		return true;
	}
	if (OtherFrame == Range.OlderFrame && bEvaluatedOlderPose)
	{
		OutPose = MoveTemp(OlderPose);
		return true;
	}

	return false;
}

void ULyraSnapShotComponent_AnimNode::UpdateSnapShotHistory()
{
	FAnimSSRFrameSnapshot ThisFrame;
	CaptureCurrentSnapShot(GetOwner(), ThisFrame);
	if (ThisFrame.NodeSnapshots.IsEmpty())
	{
		return;
	}

	SaveSnapShotData(MoveTemp(ThisFrame));
}

void ULyraSnapShotComponent_AnimNode::SaveSnapShotData(FAnimSSRFrameSnapshot Snapshot)
{
	SnapShotHistory.EmplaceFirst(MoveTemp(Snapshot));

	const bool bWantsSavedSnapshotDraw = SavedSnapshotDrawSettings.bEnabled || BoneControllerComparisonDrawSettings.bEnabled;
	if (ShouldDrawDebugPose(bWantsSavedSnapshotDraw, SavedSnapshotDrawSettings, SavedSnapshotDebugFrameCounter))
	{
		// snapshot 저장 직후 그려서 저장 당시 pose를 바로 확인하고, interval로 DrawDebug 비용을 제한한다.
		DrawSavedSnapshotPose(SnapShotHistory.First());
	}

	if (SnapShotHistory.Num() >= 2)
	{
		double HistoryLength = SnapShotHistory.First().ServerTime - SnapShotHistory.Last().ServerTime;
		while (HistoryLength > MaxRecordTime && SnapShotHistory.Num() >= 2)
		{
			SnapShotHistory.PopLast();
			HistoryLength = SnapShotHistory.First().ServerTime - SnapShotHistory.Last().ServerTime;
		}
	}
}

USkeletalMeshComponent* ULyraSnapShotComponent_AnimNode::ResolveTargetMesh() const
{
	return GetSkeletalMesh(GetOwner());
}

void ULyraSnapShotComponent_AnimNode::EnsureTickAfterMesh(USkeletalMeshComponent* Mesh)
{
	if (!IsValid(Mesh) || CurrentPrerequisiteMesh.Get() == Mesh)
	{
		return;
	}

	if (USkeletalMeshComponent* PreviousMesh = CurrentPrerequisiteMesh.Get())
	{
		PrimaryComponentTick.RemovePrerequisite(PreviousMesh, PreviousMesh->PrimaryComponentTick);
	}

	PrimaryComponentTick.AddPrerequisite(Mesh, Mesh->PrimaryComponentTick);
	CurrentPrerequisiteMesh = Mesh;
}

void ULyraSnapShotComponent_AnimNode::ConfigureMeshForSSR(USkeletalMeshComponent* Mesh)
{
	if (!bConfigureMeshForSSR || !IsValid(Mesh))
	{
		return;
	}

	// 서버 비용을 줄이기 위해 Update는 유지하고 Evaluate/Refresh는 필요한 시점에 직접 수행한다.
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	Mesh->bEnableUpdateRateOptimizations = false;
}

bool ULyraSnapShotComponent_AnimNode::BuildFrameSnapshot(FAnimSSRFrameSnapshot& OutSnapshot)
{
	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!IsValid(Mesh) || !IsValid(AnimInstance) || !IsValid(Owner) || !IsValid(World))
	{
		return false;
	}

	if (!IsReadyForSSREvaluation(Mesh, AnimInstance))
	{
		return false;
	}

	// TickComponent 이후에도 Update-only 작업이 병렬로 남을 수 있으므로 snapshot 전에 동기화한다.
	Mesh->HandleExistingParallelEvaluationTask(/*bBlockOnTask=*/true, /*bPerformPostAnimEvaluation=*/true);
	if (AnimInstance->NeedsUpdate())
	{
		return false;
	}

	OutSnapshot = FAnimSSRFrameSnapshot();
	OutSnapshot.ServerTime = World->GetTimeSeconds();
	OutSnapshot.ActorTransform = Owner->GetActorTransform();
	OutSnapshot.MeshComponentTransform = Mesh->GetComponentTransform();
	OutSnapshot.Velocity = Owner->GetVelocity();

	if (const APawn* PawnOwner = Cast<APawn>(Owner))
	{
		OutSnapshot.ControlRotation = PawnOwner->GetControlRotation();
	}
	else
	{
		OutSnapshot.ControlRotation = Owner->GetActorRotation();
	}

	if (!CaptureNodeSnapshotSet(AnimInstance, OutSnapshot.NodeSnapshots))
	{
		return false;
	}

	if (OutSnapshot.NodeSnapshots.IsEmpty())
	{
		UE_LOG(LogLyraServerSideRewind, Warning, TEXT("[%s] No supported animation nodes found for SSR snapshots."), *GetNameSafe(Owner));
		return false;
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::CaptureNodeSnapshotSet(UAnimInstance* AnimInstance, FAnimSSRNodeSnapshotSet& OutSnapshots) const
{
	OutSnapshots = FAnimSSRNodeSnapshotSet();

	const bool bCapturedSequencePlayers = CaptureSequencePlayerSnapshots(AnimInstance, OutSnapshots.SequencePlayers);
	const bool bCapturedBlendSpacePlayers = CaptureBlendSpacePlayerSnapshots(AnimInstance, OutSnapshots.BlendSpacePlayers);
	const bool bCapturedRotationOffsetBlendSpaces = CaptureRotationOffsetBlendSpaceSnapshots(AnimInstance, OutSnapshots.RotationOffsetBlendSpaces);
	const bool bCapturedBlendListByBools = CaptureBlendListByBoolSnapshots(AnimInstance, OutSnapshots.BlendListByBools);
	const bool bCapturedStateMachines = CaptureStateMachineSnapshots(AnimInstance, OutSnapshots.StateMachines);
	const bool bCapturedLayeredBoneBlends = CaptureLayeredBoneBlendSnapshots(AnimInstance, OutSnapshots.LayeredBoneBlends);
	const bool bCapturedCopyBones = CaptureCopyBoneSnapshots(AnimInstance, OutSnapshots.CopyBones);
	const bool bCapturedTransformBones = CaptureTransformBoneSnapshots(AnimInstance, OutSnapshots.TransformBones);
	const bool bCapturedTwoBoneIKs = CaptureTwoBoneIKSnapshots(AnimInstance, OutSnapshots.TwoBoneIKs);

	LogUnsupportedSSRNodeProperties(AnimInstance);

	return bCapturedSequencePlayers &&
		bCapturedBlendSpacePlayers &&
		bCapturedRotationOffsetBlendSpaces &&
		bCapturedBlendListByBools &&
		bCapturedStateMachines &&
		bCapturedLayeredBoneBlends &&
		bCapturedCopyBones &&
		bCapturedTransformBones &&
		bCapturedTwoBoneIKs;
}

bool ULyraSnapShotComponent_AnimNode::ValidateNodeSnapshotSet(UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const
{
	return ValidateSequencePlayerSnapshots(AnimInstance, Snapshots.SequencePlayers) &&
		ValidateBlendSpacePlayerSnapshots(AnimInstance, Snapshots.BlendSpacePlayers) &&
		ValidateRotationOffsetBlendSpaceSnapshots(AnimInstance, Snapshots.RotationOffsetBlendSpaces) &&
		ValidateBlendListByBoolSnapshots(AnimInstance, Snapshots.BlendListByBools) &&
		ValidateStateMachineSnapshots(AnimInstance, Snapshots.StateMachines) &&
		ValidateLayeredBoneBlendSnapshots(AnimInstance, Snapshots.LayeredBoneBlends) &&
		ValidateCopyBoneSnapshots(AnimInstance, Snapshots.CopyBones) &&
		ValidateTransformBoneSnapshots(AnimInstance, Snapshots.TransformBones) &&
		ValidateTwoBoneIKSnapshots(AnimInstance, Snapshots.TwoBoneIKs);
}

void ULyraSnapShotComponent_AnimNode::LogNodeSnapshotSetValidationDetails(const FString& OwnerName, UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const
{
	LogSequencePlayerSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.SequencePlayers);
	LogBlendSpacePlayerSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.BlendSpacePlayers);
	LogRotationOffsetBlendSpaceSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.RotationOffsetBlendSpaces);
	LogBlendListByBoolSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.BlendListByBools);
	LogStateMachineSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.StateMachines);
	LogLayeredBoneBlendSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.LayeredBoneBlends);
	LogCopyBoneSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.CopyBones);
	LogTransformBoneSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.TransformBones);
	LogTwoBoneIKSnapshotValidationDetails(OwnerName, AnimInstance, Snapshots.TwoBoneIKs);
}

bool ULyraSnapShotComponent_AnimNode::RestoreNodeSnapshotSet(UAnimInstance* AnimInstance, const FAnimSSRNodeSnapshotSet& Snapshots) const
{
	return RestoreStateMachineSnapshots(AnimInstance, Snapshots.StateMachines) &&
		RestoreLayeredBoneBlendSnapshots(AnimInstance, Snapshots.LayeredBoneBlends) &&
		RestoreRotationOffsetBlendSpaceSnapshots(AnimInstance, Snapshots.RotationOffsetBlendSpaces) &&
		RestoreBlendListByBoolSnapshots(AnimInstance, Snapshots.BlendListByBools) &&
		RestoreSequencePlayerSnapshots(AnimInstance, Snapshots.SequencePlayers) &&
		RestoreBlendSpacePlayerSnapshots(AnimInstance, Snapshots.BlendSpacePlayers) &&
		RestoreCopyBoneSnapshots(AnimInstance, Snapshots.CopyBones) &&
		RestoreTransformBoneSnapshots(AnimInstance, Snapshots.TransformBones) &&
		RestoreTwoBoneIKSnapshots(AnimInstance, Snapshots.TwoBoneIKs);
}

bool ULyraSnapShotComponent_AnimNode::CaptureSequencePlayerSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRSequencePlayerSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_SequencePlayerBase>(
		AnimInstance,
		IsSequencePlayerNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_SequencePlayerBase& SequencePlayer)
		{
			FAnimSSRSequencePlayerSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.SequenceAsset = SequencePlayer.GetSequence();
			SequencePlayer.CaptureAssetPlayerState(Snapshot.AssetPlayerState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRBlendSpacePlayerSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_BlendSpacePlayerBase>(
		AnimInstance,
		IsBlendSpacePlayerNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_BlendSpacePlayerBase& BlendSpacePlayer)
		{
			FAnimSSRBlendSpacePlayerSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.BlendSpaceAsset = BlendSpacePlayer.GetBlendSpace();
			BlendSpacePlayer.CaptureBlendSpacePlayerState(Snapshot.BlendSpacePlayerState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_RotationOffsetBlendSpace>(
		AnimInstance,
		IsRotationOffsetBlendSpaceNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_RotationOffsetBlendSpace& RotationOffsetBlendSpace)
		{
			FAnimSSRRotationOffsetBlendSpaceSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.BlendSpaceAsset = RotationOffsetBlendSpace.GetBlendSpace();
			Snapshot.NodeConfigurationHash = HashRotationOffsetBlendSpaceConfiguration(RotationOffsetBlendSpace);
			RotationOffsetBlendSpace.CaptureRotationOffsetBlendSpaceState(Snapshot.RotationOffsetBlendSpaceState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureBlendListByBoolSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRBlendListByBoolSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_BlendListByBool>(
		AnimInstance,
		IsBlendListByBoolNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_BlendListByBool& BlendListByBool)
		{
			FAnimSSRBlendListByBoolSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.BlendPoseCount = BlendListByBool.GetBlendPoseCount();
			Snapshot.BlendTimeCount = BlendListByBool.GetBlendTimes().Num();
			Snapshot.NodeConfigurationHash = HashBlendListByBoolConfiguration(BlendListByBool);
			Snapshot.CustomBlendCurve = BlendListByBool.GetCustomBlendCurve();
			Snapshot.BlendProfile = BlendListByBool.GetBlendProfile();
			BlendListByBool.CaptureBlendListByBoolState(Snapshot.BlendListByBoolState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureStateMachineSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRStateMachineSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_StateMachine>(
		AnimInstance,
		IsStateMachineNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_StateMachine& StateMachine)
		{
			const FBakedAnimationStateMachine* MachineDescription = StateMachine.GetMachineDescription();

			FAnimSSRStateMachineSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.StateMachineIndexInClass = StateMachine.StateMachineIndexInClass;
			Snapshot.MachineName = MachineDescription ? MachineDescription->MachineName : NAME_None;
			Snapshot.StateCount = MachineDescription ? MachineDescription->States.Num() : 0;
			Snapshot.TransitionCount = CountStateMachineTransitions(MachineDescription);
			StateMachine.CaptureStateMachineState(Snapshot.StateMachineState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRLayeredBoneBlendSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_LayeredBoneBlend>(
		AnimInstance,
		IsLayeredBoneBlendNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_LayeredBoneBlend& LayeredBoneBlend)
		{
			FAnimSSRLayeredBoneBlendSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.BlendMode = LayeredBoneBlend.BlendMode;
			Snapshot.BlendPoseCount = LayeredBoneBlend.BlendPoses.Num();
			Snapshot.BlendWeightCount = LayeredBoneBlend.BlendWeights.Num();
			Snapshot.LayerSetupCount = LayeredBoneBlend.LayerSetup.Num();
			Snapshot.BlendMaskCount = LayeredBoneBlend.BlendMasks.Num();
			Snapshot.LayerConfigurationHash = HashLayeredBoneBlendConfiguration(LayeredBoneBlend);
			Snapshot.BlendMasks.Reserve(LayeredBoneBlend.BlendMasks.Num());
			for (const TObjectPtr<UBlendProfile>& BlendMask : LayeredBoneBlend.BlendMasks)
			{
				Snapshot.BlendMasks.Add(BlendMask.Get());
			}
			LayeredBoneBlend.CaptureLayeredBoneBlendState(Snapshot.LayeredBoneBlendState);
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureCopyBoneSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRCopyBoneSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_CopyBone>(
		AnimInstance,
		IsCopyBoneNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_CopyBone& CopyBone)
		{
			FAnimSSRCopyBoneSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.SourceBoneName = CopyBone.SourceBone.BoneName;
			Snapshot.TargetBoneName = CopyBone.TargetBone.BoneName;
			Snapshot.ControlSpace = CopyBone.ControlSpace;
			Snapshot.NodeConfigurationHash = HashCopyBoneConfiguration(CopyBone);
			CaptureSkeletalControlSnapshot(CopyBone, Snapshot.SkeletalControlState);
			Snapshot.bCopyTranslation = CopyBone.bCopyTranslation;
			Snapshot.bCopyRotation = CopyBone.bCopyRotation;
			Snapshot.bCopyScale = CopyBone.bCopyScale;
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureTransformBoneSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRTransformBoneSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_ModifyBone>(
		AnimInstance,
		IsTransformBoneNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_ModifyBone& TransformBone)
		{
			FAnimSSRTransformBoneSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.BoneToModifyName = TransformBone.BoneToModify.BoneName;
			Snapshot.TranslationMode = TransformBone.TranslationMode;
			Snapshot.RotationMode = TransformBone.RotationMode;
			Snapshot.ScaleMode = TransformBone.ScaleMode;
			Snapshot.TranslationSpace = TransformBone.TranslationSpace;
			Snapshot.RotationSpace = TransformBone.RotationSpace;
			Snapshot.ScaleSpace = TransformBone.ScaleSpace;
			Snapshot.NodeConfigurationHash = HashTransformBoneConfiguration(TransformBone);
			CaptureSkeletalControlSnapshot(TransformBone, Snapshot.SkeletalControlState);
			Snapshot.Translation = TransformBone.Translation;
			Snapshot.Rotation = TransformBone.Rotation;
			Snapshot.Scale = TransformBone.Scale;
		});
}

bool ULyraSnapShotComponent_AnimNode::CaptureTwoBoneIKSnapshots(UAnimInstance* AnimInstance, TArray<FAnimSSRTwoBoneIKSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_TwoBoneIK>(
		AnimInstance,
		IsTwoBoneIKNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_TwoBoneIK& TwoBoneIK)
		{
			FAnimSSRTwoBoneIKSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.IKBoneName = TwoBoneIK.IKBone.BoneName;
			Snapshot.EffectorTargetName = GetBoneSocketTargetSnapshotName(TwoBoneIK.EffectorTarget);
			Snapshot.JointTargetName = GetBoneSocketTargetSnapshotName(TwoBoneIK.JointTarget);
			Snapshot.bEffectorTargetUsesSocket = TwoBoneIK.EffectorTarget.bUseSocket;
			Snapshot.bJointTargetUsesSocket = TwoBoneIK.JointTarget.bUseSocket;
			Snapshot.EffectorLocationSpace = TwoBoneIK.EffectorLocationSpace;
			Snapshot.JointTargetLocationSpace = TwoBoneIK.JointTargetLocationSpace;
			Snapshot.bAllowStretching = TwoBoneIK.bAllowStretching;
			Snapshot.bTakeRotationFromEffectorSpace = TwoBoneIK.bTakeRotationFromEffectorSpace;
			Snapshot.bMaintainEffectorRelRot = TwoBoneIK.bMaintainEffectorRelRot;
			Snapshot.bAllowTwist = TwoBoneIK.bAllowTwist;
			Snapshot.NodeConfigurationHash = HashTwoBoneIKConfiguration(TwoBoneIK);
			TwoBoneIK.CaptureTwoBoneIKState(Snapshot.TwoBoneIKState);
		});
}

bool ULyraSnapShotComponent_AnimNode::ValidateSequencePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRSequencePlayerSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentSequencePlayerCount = CountMatchingAnimNodeProperties(NodeProperties, IsSequencePlayerNodeProperty);

	if (CurrentSequencePlayerCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRSequencePlayerSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsSequencePlayerNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_SequencePlayerBase* SequencePlayer = static_cast<const FAnimNode_SequencePlayerBase*>(NodeMemory);
		if (!SequencePlayer || SequencePlayer->GetSequence() != Snapshot.SequenceAsset.Get())
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendSpacePlayerSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentBlendSpacePlayerCount = CountMatchingAnimNodeProperties(NodeProperties, IsBlendSpacePlayerNodeProperty);

	if (CurrentBlendSpacePlayerCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRBlendSpacePlayerSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsBlendSpacePlayerNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_BlendSpacePlayerBase* BlendSpacePlayer = static_cast<const FAnimNode_BlendSpacePlayerBase*>(NodeMemory);
		if (!BlendSpacePlayer || BlendSpacePlayer->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get())
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentRotationOffsetBlendSpaceCount = CountMatchingAnimNodeProperties(NodeProperties, IsRotationOffsetBlendSpaceNodeProperty);

	if (CurrentRotationOffsetBlendSpaceCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRRotationOffsetBlendSpaceSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsRotationOffsetBlendSpaceNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_RotationOffsetBlendSpace* RotationOffsetBlendSpace = static_cast<const FAnimNode_RotationOffsetBlendSpace*>(NodeMemory);
		if (!RotationOffsetBlendSpace ||
			RotationOffsetBlendSpace->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get() ||
			HashRotationOffsetBlendSpaceConfiguration(*RotationOffsetBlendSpace) != Snapshot.NodeConfigurationHash)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateBlendListByBoolSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendListByBoolSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentBlendListByBoolCount = CountMatchingAnimNodeProperties(NodeProperties, IsBlendListByBoolNodeProperty);

	if (CurrentBlendListByBoolCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRBlendListByBoolSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsBlendListByBoolNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_BlendListByBool* BlendListByBool = static_cast<const FAnimNode_BlendListByBool*>(NodeMemory);
		if (!BlendListByBool)
		{
			return false;
		}

		if (BlendListByBool->GetBlendPoseCount() != Snapshot.BlendPoseCount ||
			BlendListByBool->GetBlendTimes().Num() != Snapshot.BlendTimeCount ||
			HashBlendListByBoolConfiguration(*BlendListByBool) != Snapshot.NodeConfigurationHash ||
			BlendListByBool->GetCustomBlendCurve() != Snapshot.CustomBlendCurve.Get() ||
			BlendListByBool->GetBlendProfile() != Snapshot.BlendProfile.Get())
		{
			return false;
		}

		const int32 CapturedPerBlendDataCount = Snapshot.BlendListByBoolState.BlendListBaseState.PerBlendData.Num();
		if (CapturedPerBlendDataCount > 0 && CapturedPerBlendDataCount != Snapshot.BlendPoseCount)
		{
			return false;
		}

		const int32 CapturedPerBoneSampleDataCount = Snapshot.BlendListByBoolState.BlendListBaseState.PerBoneSampleData.Num();
		if (Snapshot.BlendProfile.IsValid() && CapturedPerBoneSampleDataCount > 0 && CapturedPerBoneSampleDataCount != Snapshot.BlendPoseCount)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateStateMachineSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRStateMachineSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentStateMachineCount = CountMatchingAnimNodeProperties(NodeProperties, IsStateMachineNodeProperty);

	if (CurrentStateMachineCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRStateMachineSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsStateMachineNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_StateMachine* StateMachine = static_cast<const FAnimNode_StateMachine*>(NodeMemory);
		if (!StateMachine || StateMachine->StateMachineIndexInClass != Snapshot.StateMachineIndexInClass)
		{
			return false;
		}

		const FBakedAnimationStateMachine* MachineDescription = StateMachine->GetMachineDescription();
		if (!MachineDescription ||
			MachineDescription->MachineName != Snapshot.MachineName ||
			MachineDescription->States.Num() != Snapshot.StateCount ||
			CountStateMachineTransitions(MachineDescription) != Snapshot.TransitionCount)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRLayeredBoneBlendSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	const int32 CurrentLayeredBoneBlendCount = CountMatchingAnimNodeProperties(NodeProperties, IsLayeredBoneBlendNodeProperty);

	if (CurrentLayeredBoneBlendCount != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRLayeredBoneBlendSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsLayeredBoneBlendNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_LayeredBoneBlend* LayeredBoneBlend = static_cast<const FAnimNode_LayeredBoneBlend*>(NodeMemory);
		if (!LayeredBoneBlend)
		{
			return false;
		}

		if (LayeredBoneBlend->BlendMode != Snapshot.BlendMode ||
			LayeredBoneBlend->BlendPoses.Num() != Snapshot.BlendPoseCount ||
			LayeredBoneBlend->BlendWeights.Num() != Snapshot.BlendWeightCount ||
			LayeredBoneBlend->LayerSetup.Num() != Snapshot.LayerSetupCount ||
			LayeredBoneBlend->BlendMasks.Num() != Snapshot.BlendMaskCount ||
			HashLayeredBoneBlendConfiguration(*LayeredBoneBlend) != Snapshot.LayerConfigurationHash ||
			Snapshot.LayeredBoneBlendState.BlendWeights.Num() != Snapshot.BlendWeightCount ||
			Snapshot.BlendMasks.Num() != Snapshot.BlendMaskCount)
		{
			return false;
		}

		for (int32 BlendMaskIndex = 0; BlendMaskIndex < Snapshot.BlendMasks.Num(); ++BlendMaskIndex)
		{
			if (LayeredBoneBlend->BlendMasks[BlendMaskIndex] != Snapshot.BlendMasks[BlendMaskIndex].Get())
			{
				return false;
			}
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateCopyBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRCopyBoneSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	if (CountMatchingAnimNodeProperties(NodeProperties, IsCopyBoneNodeProperty) != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRCopyBoneSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsCopyBoneNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_CopyBone* CopyBone = static_cast<const FAnimNode_CopyBone*>(NodeMemory);
		if (!CopyBone ||
			CopyBone->SourceBone.BoneName != Snapshot.SourceBoneName ||
			CopyBone->TargetBone.BoneName != Snapshot.TargetBoneName ||
			CopyBone->ControlSpace != Snapshot.ControlSpace ||
			HashCopyBoneConfiguration(*CopyBone) != Snapshot.NodeConfigurationHash)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateTransformBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTransformBoneSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	if (CountMatchingAnimNodeProperties(NodeProperties, IsTransformBoneNodeProperty) != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRTransformBoneSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsTransformBoneNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_ModifyBone* TransformBone = static_cast<const FAnimNode_ModifyBone*>(NodeMemory);
		if (!TransformBone ||
			TransformBone->BoneToModify.BoneName != Snapshot.BoneToModifyName ||
			TransformBone->TranslationMode != Snapshot.TranslationMode ||
			TransformBone->RotationMode != Snapshot.RotationMode ||
			TransformBone->ScaleMode != Snapshot.ScaleMode ||
			TransformBone->TranslationSpace != Snapshot.TranslationSpace ||
			TransformBone->RotationSpace != Snapshot.RotationSpace ||
			TransformBone->ScaleSpace != Snapshot.ScaleSpace ||
			HashTransformBoneConfiguration(*TransformBone) != Snapshot.NodeConfigurationHash)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateTwoBoneIKSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTwoBoneIKSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();
	if (CountMatchingAnimNodeProperties(NodeProperties, IsTwoBoneIKNodeProperty) != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRTwoBoneIKSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties.IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		if (!IsTwoBoneIKNodeProperty(NodeProperty))
		{
			return false;
		}

		if (NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName || NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_TwoBoneIK* TwoBoneIK = static_cast<const FAnimNode_TwoBoneIK*>(NodeMemory);
		if (!TwoBoneIK ||
			TwoBoneIK->IKBone.BoneName != Snapshot.IKBoneName ||
			GetBoneSocketTargetSnapshotName(TwoBoneIK->EffectorTarget) != Snapshot.EffectorTargetName ||
			GetBoneSocketTargetSnapshotName(TwoBoneIK->JointTarget) != Snapshot.JointTargetName ||
			TwoBoneIK->EffectorTarget.bUseSocket != Snapshot.bEffectorTargetUsesSocket ||
			TwoBoneIK->JointTarget.bUseSocket != Snapshot.bJointTargetUsesSocket ||
			TwoBoneIK->EffectorLocationSpace != Snapshot.EffectorLocationSpace ||
			TwoBoneIK->JointTargetLocationSpace != Snapshot.JointTargetLocationSpace ||
			static_cast<bool>(TwoBoneIK->bAllowStretching) != Snapshot.bAllowStretching ||
			static_cast<bool>(TwoBoneIK->bTakeRotationFromEffectorSpace) != Snapshot.bTakeRotationFromEffectorSpace ||
			static_cast<bool>(TwoBoneIK->bMaintainEffectorRelRot) != Snapshot.bMaintainEffectorRelRot ||
			static_cast<bool>(TwoBoneIK->bAllowTwist) != Snapshot.bAllowTwist ||
			HashTwoBoneIKConfiguration(*TwoBoneIK) != Snapshot.NodeConfigurationHash)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreSequencePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRSequencePlayerSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRSequencePlayerSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_SequencePlayerBase* SequencePlayer = static_cast<FAnimNode_SequencePlayerBase*>(NodeMemory);
		SequencePlayer->RestoreAssetPlayerState(Snapshot.AssetPlayerState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreBlendSpacePlayerSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendSpacePlayerSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRBlendSpacePlayerSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_BlendSpacePlayerBase* BlendSpacePlayer = static_cast<FAnimNode_BlendSpacePlayerBase*>(NodeMemory);
		BlendSpacePlayer->RestoreBlendSpacePlayerState(Snapshot.BlendSpacePlayerState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreRotationOffsetBlendSpaceSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRRotationOffsetBlendSpaceSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRRotationOffsetBlendSpaceSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_RotationOffsetBlendSpace* RotationOffsetBlendSpace = static_cast<FAnimNode_RotationOffsetBlendSpace*>(NodeMemory);
		RotationOffsetBlendSpace->RestoreRotationOffsetBlendSpaceState(Snapshot.RotationOffsetBlendSpaceState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreBlendListByBoolSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRBlendListByBoolSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRBlendListByBoolSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_BlendListByBool* BlendListByBool = static_cast<FAnimNode_BlendListByBool*>(NodeMemory);
		BlendListByBool->RestoreBlendListByBoolState(Snapshot.BlendListByBoolState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreStateMachineSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRStateMachineSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRStateMachineSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_StateMachine* StateMachine = static_cast<FAnimNode_StateMachine*>(NodeMemory);
		StateMachine->RestoreStateMachineState(Snapshot.StateMachineState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreLayeredBoneBlendSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRLayeredBoneBlendSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRLayeredBoneBlendSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_LayeredBoneBlend* LayeredBoneBlend = static_cast<FAnimNode_LayeredBoneBlend*>(NodeMemory);
		LayeredBoneBlend->RestoreLayeredBoneBlendState(Snapshot.LayeredBoneBlendState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreCopyBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRCopyBoneSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRCopyBoneSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_CopyBone* CopyBone = static_cast<FAnimNode_CopyBone*>(NodeMemory);
		RestoreSkeletalControlSnapshot(*CopyBone, Snapshot.SkeletalControlState);
		CopyBone->bCopyTranslation = Snapshot.bCopyTranslation;
		CopyBone->bCopyRotation = Snapshot.bCopyRotation;
		CopyBone->bCopyScale = Snapshot.bCopyScale;
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreTransformBoneSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTransformBoneSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRTransformBoneSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_ModifyBone* TransformBone = static_cast<FAnimNode_ModifyBone*>(NodeMemory);
		RestoreSkeletalControlSnapshot(*TransformBone, Snapshot.SkeletalControlState);
		TransformBone->Translation = Snapshot.Translation;
		TransformBone->Rotation = Snapshot.Rotation;
		TransformBone->Scale = Snapshot.Scale;
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreTwoBoneIKSnapshots(UAnimInstance* AnimInstance, const TArray<FAnimSSRTwoBoneIKSnapshot>& Snapshots) const
{
	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		return false;
	}

	const TArray<FStructProperty*>& NodeProperties = AnimClass->GetAnimNodeProperties();

	for (const FAnimSSRTwoBoneIKSnapshot& Snapshot : Snapshots)
	{
		const FStructProperty* NodeProperty = NodeProperties[Snapshot.Identity.NodeIndex];
		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_TwoBoneIK* TwoBoneIK = static_cast<FAnimNode_TwoBoneIK*>(NodeMemory);
		TwoBoneIK->RestoreTwoBoneIKState(Snapshot.TwoBoneIKState);
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::EvaluateFrameSnapshotPose(const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose, bool bDisableCapturedBoneControllers)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_EvaluateFrameSnapshotPose);

	OutPose = FAnimSSRRewindPose();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (!IsValid(Mesh) || !IsValid(AnimInstance))
	{
		return false;
	}

	if (!IsReadyForSSREvaluation(Mesh, AnimInstance))
	{
		return false;
	}

	// SSR은 병렬 Update가 끝난 이후의 node 상태를 기준으로 저장/복원해야 한다.
	Mesh->HandleExistingParallelEvaluationTask(/*bBlockOnTask=*/true, /*bPerformPostAnimEvaluation=*/true);
	if (AnimInstance->NeedsUpdate())
	{
		UE_LOG(LogLyraServerSideRewind, Warning, TEXT("[%s] SSR evaluation skipped because AnimInstance still needs update."),
			*GetNameSafe(Owner));
		return false;
	}

	FAnimSSRNodeSnapshotSet CurrentNodeSnapshots;
	if (!CaptureNodeSnapshotSet(AnimInstance, CurrentNodeSnapshots))
	{
		return false;
	}

	// 과거 snapshot이 현재 AnimBP의 지원 node 집합과 같은지 먼저 확인한다.
	// 일부만 복원하면 partial state가 남아 이후 서버 animation tick까지 오염될 수 있다.
	if (!ValidateNodeSnapshotSet(AnimInstance, Snapshot.NodeSnapshots))
	{
		const FString OwnerName = GetNameSafe(Owner);
		UE_LOG(LogLyraServerSideRewind, Warning,
			TEXT("[%s] SSR snapshot does not match current supported animation graph. AnimClass=%s SnapshotTime=%.3f SnapshotCounts={Sequence=%d BlendSpace=%d RotationOffsetBlendSpace=%d BlendListByBool=%d StateMachine=%d LayeredBoneBlend=%d CopyBone=%d TransformBone=%d TwoBoneIK=%d}. Enable LogLyraServerSideRewindDebug Verbose for details."),
			*OwnerName,
			*GetNameSafe(AnimInstance->GetClass()),
			Snapshot.ServerTime,
			Snapshot.NodeSnapshots.SequencePlayers.Num(),
			Snapshot.NodeSnapshots.BlendSpacePlayers.Num(),
			Snapshot.NodeSnapshots.RotationOffsetBlendSpaces.Num(),
			Snapshot.NodeSnapshots.BlendListByBools.Num(),
			Snapshot.NodeSnapshots.StateMachines.Num(),
			Snapshot.NodeSnapshots.LayeredBoneBlends.Num(),
			Snapshot.NodeSnapshots.CopyBones.Num(),
			Snapshot.NodeSnapshots.TransformBones.Num(),
			Snapshot.NodeSnapshots.TwoBoneIKs.Num());

		LogNodeSnapshotSetValidationDetails(OwnerName, AnimInstance, Snapshot.NodeSnapshots);
		return false;
	}

	bool bShouldRestoreCurrentState = false;
	ON_SCOPE_EXIT
	{
		if (bShouldRestoreCurrentState)
		{
			// 과거 pose 평가가 AnimInstance 내부 상태를 임시로 바꾸므로 현재 상태 복구가 필요하다.
			const bool bRestoredNodeState = RestoreNodeSnapshotSet(AnimInstance, CurrentNodeSnapshots);
			ensureMsgf(bRestoredNodeState,
				TEXT("Failed to restore current supported animation node state after SSR evaluation."));
		}
	};

	FAnimSSRFrameSnapshot EvaluationSnapshot = Snapshot;
	if (bDisableCapturedBoneControllers)
	{
		DisableCapturedBoneControllerSnapshots(EvaluationSnapshot);
	}

	bShouldRestoreCurrentState = true;
	if (!RestoreNodeSnapshotSet(AnimInstance, EvaluationSnapshot.NodeSnapshots))
	{
		return false;
	}

	return EvaluateSnapshotPose(Mesh, AnimInstance, EvaluationSnapshot, OutPose);
}

bool ULyraSnapShotComponent_AnimNode::EvaluateSnapshotPose(USkeletalMeshComponent* Mesh, UAnimInstance* AnimInstance, const FAnimSSRFrameSnapshot& Snapshot, FAnimSSRRewindPose& OutPose) const
{
	if (!IsValid(Mesh) || !IsValid(AnimInstance) || !IsValid(Mesh->GetSkeletalMeshAsset()))
	{
		return false;
	}

	const USkeletalMesh* SkeletalMesh = Mesh->GetSkeletalMeshAsset();
	if (!AnimInstance->ParallelCanEvaluate(SkeletalMesh))
	{
		Mesh->RecalcRequiredBones(Mesh->GetPredictedLODLevel());
		if (!AnimInstance->ParallelCanEvaluate(SkeletalMesh))
		{
			return false;
		}
	}

	const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNum();
	if (NumBones <= 0)
	{
		return false;
	}

	TArray<FTransform> ComponentSpaceTransforms;
	TArray<FTransform> BoneSpaceTransforms;
	ComponentSpaceTransforms.SetNumUninitialized(NumBones);
	BoneSpaceTransforms.SetNumUninitialized(NumBones);

	FVector RootBoneTranslation = FVector::ZeroVector;
	FBlendedHeapCurve Curve;
	UE::Anim::FMeshAttributeContainer Attributes;

	// PerformAnimationProcessing 결과는 scratch 배열에만 받으므로 현재 mesh transform buffer를 직접 오염시키지 않는다.
	FScopedSSRAnimInstanceEvaluation ScopedAnimEvaluation(Mesh, AnimInstance);
	Mesh->PerformAnimationProcessing(
		SkeletalMesh,
		AnimInstance,
		/*bInDoEvaluation=*/true,
		/*bInForceRefPose=*/false,
		ComponentSpaceTransforms,
		BoneSpaceTransforms,
		RootBoneTranslation,
		Curve,
		Attributes);

	OutPose.SelectedServerTime = Snapshot.ServerTime;
	OutPose.ActorTransform = Snapshot.ActorTransform;
	OutPose.MeshComponentTransform = Snapshot.MeshComponentTransform;
	OutPose.ComponentSpaceTransforms = MoveTemp(ComponentSpaceTransforms);
	return OutPose.ComponentSpaceTransforms.Num() == NumBones;
}

bool ULyraSnapShotComponent_AnimNode::InterpolateRewindPoses(
	const FAnimSSRRewindPose& OlderPose,
	const FAnimSSRRewindPose& YoungerPose,
	double TargetServerTime,
	float Alpha,
	FAnimSSRRewindPose& OutPose) const
{
	OutPose = FAnimSSRRewindPose();

	if (OlderPose.ComponentSpaceTransforms.Num() == 0 ||
		OlderPose.ComponentSpaceTransforms.Num() != YoungerPose.ComponentSpaceTransforms.Num())
	{
		return false;
	}

	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	OutPose.SelectedServerTime = TargetServerTime;
	OutPose.ActorTransform.Blend(OlderPose.ActorTransform, YoungerPose.ActorTransform, ClampedAlpha);
	OutPose.MeshComponentTransform.Blend(OlderPose.MeshComponentTransform, YoungerPose.MeshComponentTransform, ClampedAlpha);
	OutPose.ComponentSpaceTransforms.SetNum(OlderPose.ComponentSpaceTransforms.Num());

	for (int32 BoneIndex = 0; BoneIndex < OlderPose.ComponentSpaceTransforms.Num(); ++BoneIndex)
	{
		OutPose.ComponentSpaceTransforms[BoneIndex].Blend(
			OlderPose.ComponentSpaceTransforms[BoneIndex],
			YoungerPose.ComponentSpaceTransforms[BoneIndex],
			ClampedAlpha);
	}

	return true;
}

#if ENABLE_DRAW_DEBUG
void ULyraSnapShotComponent_AnimNode::DrawDebugPose(const FAnimSSRRewindPose& RewindPose, FColor Color) const
{
	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float LifeTime = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;

	DrawPhysicsAssetPose(
		Mesh,
		RewindPose.MeshComponentTransform,
		RewindPose.ComponentSpaceTransforms,
		Color,
		LifeTime);
}

void ULyraSnapShotComponent_AnimNode::DrawCurrentDebugPose(FColor Color) const
{
	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}

	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float LifeTime = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;

	DrawPhysicsAssetPose(
		Mesh,
		Mesh->GetComponentTransform(),
		Mesh->GetComponentSpaceTransforms(),
		Color,
		LifeTime);
}
#endif

void ULyraSnapShotComponent_AnimNode::DrawSavedSnapshotPose(const FAnimSSRFrameSnapshot& Snapshot)
{
	if (!SavedSnapshotDrawSettings.bEnabled && !BoneControllerComparisonDrawSettings.bEnabled)
	{
		return;
	}

	const UWorld* World = GetWorld();
	USkeletalMeshComponent* Mesh = ResolveTargetMesh();
	const USkeletalMesh* SkeletalMesh = IsValid(Mesh) ? Mesh->GetSkeletalMeshAsset() : nullptr;
	if (!IsValid(World) || !IsValid(SkeletalMesh))
	{
		return;
	}

	if (SavedSnapshotDrawSettings.bEnabled || BoneControllerComparisonDrawSettings.bEnabled)
	{
		FAnimSSRRewindPose RewindPose;
		if (EvaluateFrameSnapshotPose(Snapshot, RewindPose, /*bDisableCapturedBoneControllers=*/false))
		{
			// This stat excludes snapshot restore/evaluate and measures only debug primitive submission.
			SCOPE_CYCLE_COUNTER(STAT_LyraSSR_DrawSavedSnapshotDebug);
			const FString Label = BoneControllerComparisonDrawSettings.bEnabled && BoneControllerComparisonDrawSettings.bDrawLabel
				? MakeBoneControllerComparisonLabel(TEXT("ON"), Snapshot)
				: FString();
			DrawRewindPose(
				SkeletalMesh,
				RewindPose,
				SavedSnapshotDrawSettings,
				FVector::ZeroVector,
				Label);
		}
	}

	if (BoneControllerComparisonDrawSettings.bEnabled)
	{
		FAnimSSRRewindPose DisabledPose;
		if (EvaluateFrameSnapshotPose(Snapshot, DisabledPose, /*bDisableCapturedBoneControllers=*/true))
		{
			SCOPE_CYCLE_COUNTER(STAT_LyraSSR_DrawSavedSnapshotDebug);
			FAnimSSRDebugPoseDrawSettings DisabledDrawSettings = SavedSnapshotDrawSettings;
			DisabledDrawSettings.Color = BoneControllerComparisonDrawSettings.DisabledPoseColor;
			const FString Label = BoneControllerComparisonDrawSettings.bDrawLabel
				? MakeBoneControllerComparisonLabel(TEXT("OFF"), Snapshot)
				: FString();
			DrawRewindPose(
				SkeletalMesh,
				DisabledPose,
				DisabledDrawSettings,
				BoneControllerComparisonDrawSettings.DisabledPoseWorldOffset,
				Label);
		}
	}

}

void ULyraSnapShotComponent_AnimNode::DrawServerCurrentMeshPose()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority() || !ShouldDrawDebugPose(ServerCurrentDrawSettings.bEnabled, ServerCurrentDrawSettings, ServerCurrentMeshPoseDebugFrameCounter))
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_DrawServerCurrentMeshPoseDebug);
	DrawCurrentMeshPose(ResolveTargetMesh(), ServerCurrentDrawSettings);
}

void ULyraSnapShotComponent_AnimNode::DrawClientCurrentMeshPose()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) ||
		(Owner->HasAuthority() && Owner->GetNetMode() != NM_ListenServer) ||
		!ShouldDrawDebugPose(ClientCurrentDrawSettings.bEnabled, ClientCurrentDrawSettings, ClientCurrentMeshPoseDebugFrameCounter))
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_LyraSSR_DrawClientCurrentMeshPoseDebug);
	DrawCurrentMeshPose(ResolveTargetMesh(), ClientCurrentDrawSettings);
}

void ULyraSnapShotComponent_AnimNode::DrawCurrentMeshPose(
	USkeletalMeshComponent* Mesh,
	const FAnimSSRDebugPoseDrawSettings& DrawSettings) const
{
	const UWorld* World = GetWorld();
	const USkeletalMesh* SkeletalMesh = IsValid(Mesh) ? Mesh->GetSkeletalMeshAsset() : nullptr;
	if (!IsValid(World) || !IsValid(Mesh) || !IsValid(SkeletalMesh))
	{
		return;
	}

	// 현재 visible pose 확인용이므로 여기서는 병렬 task flush나 PerformAnimationProcessing을 호출하지 않는다.
	// 서버 mesh가 Evaluate를 스킵했다면 component-space transform buffer에 남은 모습을 그대로 그린다.
	const TArray<FTransform>& ComponentSpaceTransforms = Mesh->GetComponentSpaceTransforms();
	DrawPoseSkeleton(World, SkeletalMesh, ComponentSpaceTransforms, Mesh->GetComponentTransform(), DrawSettings, FVector::ZeroVector, FString());
}

void ULyraSnapShotComponent_AnimNode::DrawRewindPose(
	const USkeletalMesh* SkeletalMesh,
	const FAnimSSRRewindPose& RewindPose,
	const FAnimSSRDebugPoseDrawSettings& DrawSettings,
	const FVector& WorldOffset,
	const FString& Label) const
{
	const UWorld* World = GetWorld();
	DrawPoseSkeleton(World, SkeletalMesh, RewindPose.ComponentSpaceTransforms, RewindPose.MeshComponentTransform, DrawSettings, WorldOffset, Label);
}
