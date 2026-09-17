// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSnapShotComponent_AnimNode.h"
#include "LyraSnapShotComponent_AnimNode_Internal.h"

#include "Animation/AnimClassInterface.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendProfile.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimMontageEvaluationState.h"
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
#include "AnimNodes/AnimNode_Slot.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AttributesRuntime.h"
#include "BoneControllers/AnimNode_CopyBone.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Misc/ScopeExit.h"
#include "Stats/Stats.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraAnimNodeServerSideRewind, Log, All);

DECLARE_CYCLE_STAT(TEXT("EvaluateFrameSnapshotPose"), STAT_LyraSSR_EvaluateFrameSnapshotPose, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("History AnimInstance Snapshots"), STAT_LyraSSR_HistoryAnimInstanceSnapshots, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("History Slot Snapshots"), STAT_LyraSSR_HistorySlotSnapshots, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("History Montage States"), STAT_LyraSSR_HistoryMontageStates, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("Retained Montage Assets"), STAT_LyraSSR_RetainedMontageAssets, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("Topology Mismatches"), STAT_LyraSSR_TopologyMismatches, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("Expired Montage Assets"), STAT_LyraSSR_ExpiredMontageAssets, STATGROUP_LyraSSR);
DECLARE_DWORD_COUNTER_STAT(TEXT("Restore Failures"), STAT_LyraSSR_RestoreFailures, STATGROUP_LyraSSR);
DECLARE_MEMORY_STAT(TEXT("History Snapshot Memory"), STAT_LyraSSR_HistorySnapshotMemory, STATGROUP_LyraSSR);

using namespace LyraAnimNodeSnapshotPrivate;

namespace LyraAnimNodeSnapshotPrivate
{
void SetSnapshotLookupDiagnostics(
	FLyraSSRRewindDiagnostics* Diagnostics,
	bool bSucceeded,
	bool bUsedInterpolation)
{
	if (Diagnostics)
	{
		Diagnostics->bSnapshotLookupSucceeded = bSucceeded;
		Diagnostics->bUsedInterpolation = bUsedInterpolation;
	}
}

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

bool IsSlotNodeProperty(const FStructProperty* NodeProperty)
{
	return NodeProperty && NodeProperty->Struct && NodeProperty->Struct->IsChildOf(FAnimNode_Slot::StaticStruct());
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
	for (FAnimSSRAnimInstanceSnapshot& AnimInstanceSnapshot : Snapshot.AnimInstanceSnapshots)
	{
		for (FAnimSSRCopyBoneSnapshot& CopyBoneSnapshot : AnimInstanceSnapshot.NodeSnapshots.CopyBones)
		{
			CopyBoneSnapshot.SkeletalControlState.ActualAlpha = 0.0f;
		}

		for (FAnimSSRTransformBoneSnapshot& TransformBoneSnapshot : AnimInstanceSnapshot.NodeSnapshots.TransformBones)
		{
			TransformBoneSnapshot.SkeletalControlState.ActualAlpha = 0.0f;
		}

		for (FAnimSSRTwoBoneIKSnapshot& TwoBoneIKSnapshot : AnimInstanceSnapshot.NodeSnapshots.TwoBoneIKs)
		{
			TwoBoneIKSnapshot.TwoBoneIKState.ActualAlpha = 0.0f;
		}
	}
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
				// UAnimInstance::PostEvaluateAnimation은 Native/Blueprint callback을 호출하고
				// Proxy::PostEvaluate도 debug/log queue를 flush한다. SSR pass는 object reference cleanup만 수행한다.
				AnimInstance->ClearSnapshotEvaluationObjectReferences();
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

const FAnimSSRFrameSnapshot* SelectNearestSnapshotFrame(const FAnimSSRSnapshotRange& Range)
{
	if (Range.bSingleFrame)
	{
		return Range.YoungerFrame;
	}

	if (!Range.OlderFrame || !Range.YoungerFrame)
	{
		return nullptr;
	}

	const double YoungerDistance = FMath::Abs(Range.YoungerFrame->ServerTime - Range.TargetServerTime);
	const double OlderDistance = FMath::Abs(Range.TargetServerTime - Range.OlderFrame->ServerTime);
	// 거리가 같으면 기존 최근접 조회 정책대로 더 최신인 YoungerFrame을 선택한다.
	return YoungerDistance <= OlderDistance ? Range.YoungerFrame : Range.OlderFrame;
}

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
	if (!IsSSRActive())
	{
		return;
	}

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
	if (!IsSSRActive())
	{
		return;
	}

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

	const FAnimSSRFrameSnapshot* SelectedFrame = SelectNearestSnapshotFrame(Range);
	if (!SelectedFrame)
	{
		return false;
	}

	OutFrame = *SelectedFrame;
	return !OutFrame.IsEmpty();
}

bool ULyraSnapShotComponent_AnimNode::TryGetSnapShotRangeAtTime(
	double TargetServerTime,
	FAnimSSRSnapshotRange& OutRange,
	FLyraSSRRewindDiagnostics* Diagnostics) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_Rewind_SnapshotLookup);
	OutRange = FAnimSSRSnapshotRange();
	OutRange.TargetServerTime = TargetServerTime;

	if (SnapShotHistory.Num() == 0)
	{
		SetSnapshotLookupDiagnostics(Diagnostics, false, false);
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
		OutRange.bSingleFrame = true;
		return !Frame.IsEmpty();
	};

	if (OldestTime > TargetServerTime && !bIsAtOldest)
	{
		SetSnapshotLookupDiagnostics(Diagnostics, false, false);
		return false;
	}

	if (bIsAtOldest)
	{
		const bool bResult = SetSingleFrame(SnapShotHistory.Last());
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}

	if (NewestTime <= TargetServerTime || bIsAtNewest)
	{
		const bool bResult = SetSingleFrame(SnapShotHistory.First());
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}

	const int32 Count = SnapShotHistory.Num();
	int32 YoungerIdx = 0;
	while (YoungerIdx + 1 < Count && SnapShotHistory[YoungerIdx + 1].ServerTime > TargetServerTime)
	{
		++YoungerIdx;
	}

	if (YoungerIdx + 1 >= Count)
	{
		const bool bResult = SetSingleFrame(SnapShotHistory.Last());
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}

	const FAnimSSRFrameSnapshot& YoungerFrame = SnapShotHistory[YoungerIdx];
	const FAnimSSRFrameSnapshot& OlderFrame = SnapShotHistory[YoungerIdx + 1];
	const double DeltaTime = YoungerFrame.ServerTime - OlderFrame.ServerTime;
	if (FMath::IsNearlyZero(DeltaTime))
	{
		const bool bResult = SetSingleFrame(YoungerFrame);
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}

	const double Alpha = FMath::Clamp((TargetServerTime - OlderFrame.ServerTime) / DeltaTime, 0.0, 1.0);
	if (Alpha <= KINDA_SMALL_NUMBER)
	{
		const bool bResult = SetSingleFrame(OlderFrame);
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}
	if ((1.0 - Alpha) <= KINDA_SMALL_NUMBER)
	{
		const bool bResult = SetSingleFrame(YoungerFrame);
		SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
		return bResult;
	}

	OutRange.OlderFrame = &OlderFrame;
	OutRange.YoungerFrame = &YoungerFrame;
	OutRange.bSingleFrame = false;
	const bool bResult = !OlderFrame.IsEmpty() && !YoungerFrame.IsEmpty();
	SetSnapshotLookupDiagnostics(Diagnostics, bResult, false);
	return bResult;
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

bool ULyraSnapShotComponent_AnimNode::EvaluatePoseAtServerTime(
	double TargetServerTime,
	FAnimSSRRewindPose& OutPose,
	FLyraSSRRewindDiagnostics* Diagnostics)
{
	OutPose = FAnimSSRRewindPose();
	ON_SCOPE_EXIT
	{
		if (Diagnostics)
		{
			Diagnostics->bPoseReconstructionSucceeded = !OutPose.ComponentSpaceTransforms.IsEmpty();
		}
	};

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return false;
	}

	FAnimSSRSnapshotRange Range;
	if (!TryGetSnapShotRangeAtTime(TargetServerTime, Range, Diagnostics))
	{
		return false;
	}

	const FAnimSSRFrameSnapshot* NearestFrame = SelectNearestSnapshotFrame(Range);
	if (!NearestFrame || NearestFrame->IsEmpty())
	{
		SetSnapshotLookupDiagnostics(Diagnostics, false, false);
		return false;
	}

	SetSnapshotLookupDiagnostics(Diagnostics, true, false);
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_Rewind_PoseReconstruction);
	// 가장 가까운 snapshot 하나만 복원/평가하여 보간용 이중 pose 평가 비용을 제거한다.
	return EvaluateFrameSnapshotPose(*NearestFrame, OutPose, false, Diagnostics);
}

void ULyraSnapShotComponent_AnimNode::UpdateSnapShotHistory()
{
	if (const UWorld* World = GetWorld())
	{
		PruneSnapshotHistory(World->GetTimeSeconds());
	}

	FAnimSSRFrameSnapshot ThisFrame;
	CaptureCurrentSnapShot(GetOwner(), ThisFrame);
	if (ThisFrame.IsEmpty())
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

	PruneSnapshotHistory(SnapShotHistory.First().ServerTime);
	RefreshRetainedMontageAssets();
}

void ULyraSnapShotComponent_AnimNode::PruneSnapshotHistory(double CurrentServerTime)
{
	bool bRemovedFrame = false;
	while (!SnapShotHistory.IsEmpty() && CurrentServerTime - SnapShotHistory.Last().ServerTime > MaxRecordTime)
	{
		SnapShotHistory.PopLast();
		bRemovedFrame = true;
	}

	if (bRemovedFrame)
	{
		RefreshRetainedMontageAssets();
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
	TArray<FAnimSSRAnimInstanceIdentity> CurrentAnimInstanceIdentities;
	BuildAnimInstanceIdentities(Mesh, CurrentAnimInstanceIdentities);
	for (const FAnimSSRAnimInstanceIdentity& Identity : CurrentAnimInstanceIdentities)
	{
		if (!Identity.Instance.IsValid() || Identity.Instance->NeedsUpdate())
		{
			return false;
		}
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

	if (!CaptureAnimInstanceSnapshotSet(Mesh, OutSnapshot.AnimInstanceSnapshots))
	{
		return false;
	}

	if (OutSnapshot.IsEmpty())
	{
		UE_LOG(LogLyraAnimNodeServerSideRewind, Warning, TEXT("[%s] SSR snapshot으로 저장할 수 있는 지원 애니메이션/Slot/Montage 상태를 찾지 못했습니다."), *GetNameSafe(Owner));
		return false;
	}

	return true;
}

void ULyraSnapShotComponent_AnimNode::BuildAnimInstanceIdentities(
	USkeletalMeshComponent* Mesh,
	TArray<FAnimSSRAnimInstanceIdentity>& OutIdentities) const
{
	OutIdentities.Reset();
	if (!IsValid(Mesh))
	{
		return;
	}

	TSet<UAnimInstance*> AddedInstances;
	auto AddIdentity = [&OutIdentities, &AddedInstances](UAnimInstance* Instance, EAnimSSRAnimInstanceRole Role, int32 LinkedInstanceIndex)
	{
		if (!IsValid(Instance) || AddedInstances.Contains(Instance))
		{
			return;
		}

		FAnimSSRAnimInstanceIdentity& Identity = OutIdentities.AddDefaulted_GetRef();
		Identity.Role = Role;
		Identity.LinkedInstanceIndex = LinkedInstanceIndex;
		Identity.Instance = Instance;
		Identity.AnimClass = Instance->GetClass();
		AddedInstances.Add(Instance);
	};

	AddIdentity(Mesh->GetAnimInstance(), EAnimSSRAnimInstanceRole::Main, INDEX_NONE);

	const USkeletalMeshComponent* ConstMesh = Mesh;
	const TArray<UAnimInstance*>& LinkedInstances = ConstMesh->GetLinkedAnimInstances();
	for (int32 LinkedInstanceIndex = 0; LinkedInstanceIndex < LinkedInstances.Num(); ++LinkedInstanceIndex)
	{
		AddIdentity(LinkedInstances[LinkedInstanceIndex], EAnimSSRAnimInstanceRole::Linked, LinkedInstanceIndex);
	}

	if (Mesh->ShouldEvaluatePostProcessInstance())
	{
		AddIdentity(Mesh->GetPostProcessInstance(), EAnimSSRAnimInstanceRole::PostProcess, INDEX_NONE);
	}
}

bool ULyraSnapShotComponent_AnimNode::CaptureAnimInstanceSnapshotSet(
	USkeletalMeshComponent* Mesh,
	TArray<FAnimSSRAnimInstanceSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	TArray<FAnimSSRAnimInstanceIdentity> Identities;
	BuildAnimInstanceIdentities(Mesh, Identities);
	if (Identities.IsEmpty())
	{
		return false;
	}

	OutSnapshots.Reserve(Identities.Num());
	for (int32 SnapshotIndex = 0; SnapshotIndex < Identities.Num(); ++SnapshotIndex)
	{
		const FAnimSSRAnimInstanceIdentity& Identity = Identities[SnapshotIndex];
		UAnimInstance* Instance = Identity.Instance.Get();
		if (!IsValid(Instance))
		{
			return false;
		}

		FAnimSSRAnimInstanceSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
		Snapshot.Identity = Identity;
		if (!CaptureNodeSnapshotSet(Instance, Snapshot.NodeSnapshots) ||
			!CaptureSlotSnapshots(Instance, Snapshot.Slots))
		{
			return false;
		}

		Snapshot.bUsesMainInstanceMontageEvaluationData =
			Identity.Role != EAnimSSRAnimInstanceRole::Main && Instance->IsUsingMainInstanceMontageEvaluationData();
		Snapshot.MontageEvaluationDataOwnerSnapshotIndex =
			Snapshot.bUsesMainInstanceMontageEvaluationData ? 0 : SnapshotIndex;
		Snapshot.bHasOwnedMontageEvaluationData = !Snapshot.bUsesMainInstanceMontageEvaluationData;
		if (Snapshot.bHasOwnedMontageEvaluationData)
		{
			Instance->CaptureMontageEvaluationDataSnapshot(Snapshot.MontageEvaluationData);
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::ValidateAnimInstanceSnapshotSet(
	USkeletalMeshComponent* Mesh,
	const TArray<FAnimSSRAnimInstanceSnapshot>& Snapshots,
	FLyraSSRRewindDiagnostics* Diagnostics) const
{
	TArray<FAnimSSRAnimInstanceIdentity> CurrentIdentities;
	BuildAnimInstanceIdentities(Mesh, CurrentIdentities);
	if (CurrentIdentities.Num() != Snapshots.Num())
	{
		INC_DWORD_STAT(STAT_LyraSSR_TopologyMismatches);
		if (Diagnostics)
		{
			Diagnostics->bTopologyMatched = false;
		}
		UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
			TEXT("[%s] SSR AnimInstance topology mismatch: current=%d snapshot=%d."),
			*GetNameSafe(GetOwner()), CurrentIdentities.Num(), Snapshots.Num());
		return false;
	}

	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		const FAnimSSRAnimInstanceSnapshot& Snapshot = Snapshots[SnapshotIndex];
		const FAnimSSRAnimInstanceIdentity& CurrentIdentity = CurrentIdentities[SnapshotIndex];
		UAnimInstance* Instance = CurrentIdentity.Instance.Get();
		if (!IsValid(Instance) ||
			Snapshot.Identity.Role != CurrentIdentity.Role ||
			Snapshot.Identity.LinkedInstanceIndex != CurrentIdentity.LinkedInstanceIndex ||
			Snapshot.Identity.Instance.Get() != Instance ||
			Snapshot.Identity.AnimClass.Get() != Instance->GetClass())
		{
			INC_DWORD_STAT(STAT_LyraSSR_TopologyMismatches);
			if (Diagnostics)
			{
				Diagnostics->bTopologyMatched = false;
			}
			UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
				TEXT("[%s] SSR AnimInstance identity mismatch at index %d (role=%d linkedIndex=%d class=%s)."),
				*GetNameSafe(GetOwner()), SnapshotIndex, static_cast<int32>(Snapshot.Identity.Role),
				Snapshot.Identity.LinkedInstanceIndex, *GetNameSafe(Snapshot.Identity.AnimClass.Get()));
			return false;
		}

		const bool bCurrentlyUsesMainMontageData =
			CurrentIdentity.Role != EAnimSSRAnimInstanceRole::Main && Instance->IsUsingMainInstanceMontageEvaluationData();
		const int32 ExpectedMontageDataOwnerIndex = bCurrentlyUsesMainMontageData ? 0 : SnapshotIndex;
		if (bCurrentlyUsesMainMontageData != Snapshot.bUsesMainInstanceMontageEvaluationData ||
			Snapshot.MontageEvaluationDataOwnerSnapshotIndex != ExpectedMontageDataOwnerIndex ||
			Snapshot.bHasOwnedMontageEvaluationData == Snapshot.bUsesMainInstanceMontageEvaluationData)
		{
			INC_DWORD_STAT(STAT_LyraSSR_TopologyMismatches);
			if (Diagnostics)
			{
				Diagnostics->bTopologyMatched = false;
			}
			UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
				TEXT("[%s] SSR montage-data ownership mismatch for AnimInstance %s."),
				*GetNameSafe(GetOwner()), *GetNameSafe(Instance));
			return false;
		}

		if (!ValidateNodeSnapshotSet(Instance, Snapshot.NodeSnapshots) ||
			!ValidateSlotSnapshots(Instance, Snapshot.Slots))
		{
			if (Diagnostics)
			{
				Diagnostics->bTopologyMatched = false;
			}
			const FString OwnerName = GetNameSafe(GetOwner());
			UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
				TEXT("[%s] SSR AnimInstance node/Slot snapshot mismatch: instance=%s class=%s."),
				*OwnerName, *GetNameSafe(Instance), *GetNameSafe(Instance->GetClass()));
			LogNodeSnapshotSetValidationDetails(OwnerName, Instance, Snapshot.NodeSnapshots);
			return false;
		}

		if (Snapshot.bHasOwnedMontageEvaluationData)
		{
			for (const FMontageEvaluationState& MontageState : Snapshot.MontageEvaluationData.MontageEvaluationData)
			{
				if (!MontageState.Montage.IsValid() ||
					(MontageState.ActiveBlendProfile && !IsValid(MontageState.ActiveBlendProfile)))
				{
					INC_DWORD_STAT(STAT_LyraSSR_ExpiredMontageAssets);
					UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
						TEXT("[%s] SSR montage asset or blend profile expired: instance=%s snapshotIndex=%d."),
						*GetNameSafe(GetOwner()), *GetNameSafe(Instance), SnapshotIndex);
					return false;
				}
			}
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreAnimInstanceSnapshotSet(
	const TArray<FAnimSSRAnimInstanceSnapshot>& Snapshots) const
{
	// Main proxy를 먼저 복원한다. montage data 공유 linked instance는 별도 proxy 쓰기 없이 이를 읽는다.
	for (const FAnimSSRAnimInstanceSnapshot& Snapshot : Snapshots)
	{
		UAnimInstance* Instance = Snapshot.Identity.Instance.Get();
		if (!IsValid(Instance))
		{
			return false;
		}

		if (Snapshot.bHasOwnedMontageEvaluationData)
		{
			Instance->RestoreMontageEvaluationDataSnapshot(Snapshot.MontageEvaluationData);
		}
	}

	for (const FAnimSSRAnimInstanceSnapshot& Snapshot : Snapshots)
	{
		UAnimInstance* Instance = Snapshot.Identity.Instance.Get();
		if (!IsValid(Instance) ||
			!RestoreNodeSnapshotSet(Instance, Snapshot.NodeSnapshots) ||
			!RestoreSlotSnapshots(Instance, Snapshot.Slots))
		{
			return false;
		}
	}

	return true;
}

void ULyraSnapShotComponent_AnimNode::RefreshRetainedMontageAssets()
{
	RetainedMontageAssets.Reset();
	uint32 AnimInstanceSnapshotCount = 0;
	uint32 SlotSnapshotCount = 0;
	uint32 MontageStateCount = 0;
	SIZE_T HistorySnapshotMemory = SnapShotHistory.Num() * sizeof(FAnimSSRFrameSnapshot);

	for (const FAnimSSRFrameSnapshot& Frame : SnapShotHistory)
	{
		AnimInstanceSnapshotCount += Frame.AnimInstanceSnapshots.Num();
		HistorySnapshotMemory += Frame.AnimInstanceSnapshots.GetAllocatedSize();
		for (const FAnimSSRAnimInstanceSnapshot& AnimInstanceSnapshot : Frame.AnimInstanceSnapshots)
		{
			SlotSnapshotCount += AnimInstanceSnapshot.Slots.Num();
			MontageStateCount += AnimInstanceSnapshot.MontageEvaluationData.MontageEvaluationData.Num();
			HistorySnapshotMemory += AnimInstanceSnapshot.Slots.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.MontageEvaluationData.MontageEvaluationData.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.SequencePlayers.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.BlendSpacePlayers.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.RotationOffsetBlendSpaces.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.BlendListByBools.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.StateMachines.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.LayeredBoneBlends.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.CopyBones.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.TransformBones.GetAllocatedSize();
			HistorySnapshotMemory += AnimInstanceSnapshot.NodeSnapshots.TwoBoneIKs.GetAllocatedSize();
			for (const FMontageEvaluationState& MontageState : AnimInstanceSnapshot.MontageEvaluationData.MontageEvaluationData)
			{
				if (UAnimMontage* Montage = MontageState.Montage.Get())
				{
					RetainedMontageAssets.AddUnique(Montage);
				}
			}
		}
	}

	SET_DWORD_STAT(STAT_LyraSSR_HistoryAnimInstanceSnapshots, AnimInstanceSnapshotCount);
	SET_DWORD_STAT(STAT_LyraSSR_HistorySlotSnapshots, SlotSnapshotCount);
	SET_DWORD_STAT(STAT_LyraSSR_HistoryMontageStates, MontageStateCount);
	SET_DWORD_STAT(STAT_LyraSSR_RetainedMontageAssets, RetainedMontageAssets.Num());
	SET_MEMORY_STAT(STAT_LyraSSR_HistorySnapshotMemory, HistorySnapshotMemory + RetainedMontageAssets.GetAllocatedSize());

	UE_LOG(LogLyraAnimNodeServerSideRewind, VeryVerbose,
		TEXT("[%s] SSR history: frames=%d animInstances=%u slots=%u montageStates=%u retainedMontages=%d snapshotBytes=%llu"),
		*GetNameSafe(GetOwner()), SnapShotHistory.Num(), AnimInstanceSnapshotCount, SlotSnapshotCount,
		MontageStateCount, RetainedMontageAssets.Num(),
		static_cast<uint64>(HistorySnapshotMemory + RetainedMontageAssets.GetAllocatedSize()));
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

bool ULyraSnapShotComponent_AnimNode::CaptureSlotSnapshots(
	UAnimInstance* AnimInstance,
	TArray<FAnimSSRSlotSnapshot>& OutSnapshots) const
{
	OutSnapshots.Reset();

	return ForEachSSRAnimNode<FAnimNode_Slot>(
		AnimInstance,
		IsSlotNodeProperty,
		[&OutSnapshots](int32 NodeIndex, const FStructProperty* NodeProperty, const FAnimNode_Slot& Slot)
		{
			FAnimSSRSlotSnapshot& Snapshot = OutSnapshots.AddDefaulted_GetRef();
			CaptureSnapshotNodeIdentity(NodeIndex, NodeProperty, Snapshot.Identity);
			Snapshot.SlotName = Slot.SlotName;
			Snapshot.bAlwaysUpdateSourcePose = Slot.bAlwaysUpdateSourcePose;
			Slot.CaptureSlotState(Snapshot.SlotState);
		});
}

bool ULyraSnapShotComponent_AnimNode::ValidateSlotSnapshots(
	UAnimInstance* AnimInstance,
	const TArray<FAnimSSRSlotSnapshot>& Snapshots) const
{
	const TArray<FStructProperty*>* NodeProperties = GetSSRAnimNodeProperties(AnimInstance);
	if (!NodeProperties || CountMatchingAnimNodeProperties(*NodeProperties, IsSlotNodeProperty) != Snapshots.Num())
	{
		return false;
	}

	for (const FAnimSSRSlotSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties->IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = (*NodeProperties)[Snapshot.Identity.NodeIndex];
		if (!IsSlotNodeProperty(NodeProperty) ||
			NodeProperty->GetFName() != Snapshot.Identity.NodePropertyName ||
			NodeProperty->Struct->GetFName() != Snapshot.Identity.NodeStructName)
		{
			return false;
		}

		const void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		const FAnimNode_Slot* Slot = static_cast<const FAnimNode_Slot*>(NodeMemory);
		if (!Slot || Slot->SlotName != Snapshot.SlotName || Slot->bAlwaysUpdateSourcePose != Snapshot.bAlwaysUpdateSourcePose)
		{
			return false;
		}
	}

	return true;
}

bool ULyraSnapShotComponent_AnimNode::RestoreSlotSnapshots(
	UAnimInstance* AnimInstance,
	const TArray<FAnimSSRSlotSnapshot>& Snapshots) const
{
	const TArray<FStructProperty*>* NodeProperties = GetSSRAnimNodeProperties(AnimInstance);
	if (!NodeProperties)
	{
		return false;
	}

	for (const FAnimSSRSlotSnapshot& Snapshot : Snapshots)
	{
		if (!NodeProperties->IsValidIndex(Snapshot.Identity.NodeIndex))
		{
			return false;
		}

		const FStructProperty* NodeProperty = (*NodeProperties)[Snapshot.Identity.NodeIndex];
		if (!IsSlotNodeProperty(NodeProperty))
		{
			return false;
		}

		void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(AnimInstance);
		FAnimNode_Slot* Slot = static_cast<FAnimNode_Slot*>(NodeMemory);
		if (!Slot)
		{
			return false;
		}
		Slot->RestoreSlotState(Snapshot.SlotState);
	}

	return true;
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

bool ULyraSnapShotComponent_AnimNode::EvaluateFrameSnapshotPose(
	const FAnimSSRFrameSnapshot& Snapshot,
	FAnimSSRRewindPose& OutPose,
	bool bDisableCapturedBoneControllers,
	FLyraSSRRewindDiagnostics* Diagnostics)
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
	TArray<FAnimSSRAnimInstanceIdentity> CurrentAnimInstanceIdentities;
	BuildAnimInstanceIdentities(Mesh, CurrentAnimInstanceIdentities);
	for (const FAnimSSRAnimInstanceIdentity& Identity : CurrentAnimInstanceIdentities)
	{
		if (!Identity.Instance.IsValid() || Identity.Instance->NeedsUpdate())
		{
			UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
				TEXT("[%s] AnimInstance 업데이트가 아직 필요하여 SSR 평가를 건너뜁니다. Instance=%s Role=%d"),
				*GetNameSafe(Owner), *GetNameSafe(Identity.Instance.Get()), static_cast<int32>(Identity.Role));
			return false;
		}
	}

	TArray<FAnimSSRAnimInstanceSnapshot> CurrentAnimInstanceSnapshots;
	if (!CaptureAnimInstanceSnapshotSet(Mesh, CurrentAnimInstanceSnapshots))
	{
		return false;
	}

	// 과거 snapshot이 현재 main/linked/post-process topology 및 지원 node/Slot 집합과 같은지 먼저 확인한다.
	// 일부만 복원하면 partial state가 남아 이후 서버 animation tick까지 오염될 수 있다.
	if (!ValidateAnimInstanceSnapshotSet(Mesh, Snapshot.AnimInstanceSnapshots, Diagnostics))
	{
		UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
			TEXT("[%s] SSR snapshot validation failed. SnapshotTime=%.3f AnimInstances=%d."),
			*GetNameSafe(Owner), Snapshot.ServerTime, Snapshot.AnimInstanceSnapshots.Num());
		return false;
	}

	bool bShouldRestoreCurrentState = false;
	ON_SCOPE_EXIT
	{
		if (bShouldRestoreCurrentState)
		{
			// 과거 pose 평가는 proxy와 모든 graph instance의 상태를 임시로 바꾸므로 반환 경로와 무관하게 복구한다.
			const bool bRestoredCurrentState = RestoreAnimInstanceSnapshotSet(CurrentAnimInstanceSnapshots);
			if (!bRestoredCurrentState)
			{
				INC_DWORD_STAT(STAT_LyraSSR_RestoreFailures);
				if (Diagnostics)
				{
					Diagnostics->bStateRestoreSucceeded = false;
				}
			}
			ensureMsgf(bRestoredCurrentState,
				TEXT("Failed to restore current animation node/Slot/montage proxy state after SSR evaluation."));
		}
	};

	FAnimSSRFrameSnapshot EvaluationSnapshot = Snapshot;
	if (bDisableCapturedBoneControllers)
	{
		DisableCapturedBoneControllerSnapshots(EvaluationSnapshot);
	}

	bShouldRestoreCurrentState = true;
	if (!RestoreAnimInstanceSnapshotSet(EvaluationSnapshot.AnimInstanceSnapshots))
	{
		INC_DWORD_STAT(STAT_LyraSSR_RestoreFailures);
		if (Diagnostics)
		{
			Diagnostics->bStateRestoreSucceeded = false;
		}
		UE_LOG(LogLyraAnimNodeServerSideRewind, Warning,
			TEXT("[%s] Failed to restore historical AnimInstance/Slot/montage state for SSR evaluation."),
			*GetNameSafe(Owner));
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
