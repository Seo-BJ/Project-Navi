// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSnapShotComponent_AnimNode.h"
#include "LyraSnapShotComponent_AnimNode_Internal.h"

#include "Animation/AnimClassInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/BlendProfile.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimNode_StateMachine.h"
#include "AnimNodes/AnimNode_BlendListByBool.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "AnimNodes/AnimNode_RotationOffsetBlendSpace.h"
#include "BoneControllers/AnimNode_CopyBone.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "Curves/CurveFloat.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraServerSideRewindDebug, Log, All);

using namespace LyraAnimNodeSnapshotPrivate;

namespace
{
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

const IAnimClassInterface* GetAnimClassForSSRValidation(const FString& OwnerName, const TCHAR* NodeTypeName, UAnimInstance* AnimInstance)
{
	if (!IsValid(AnimInstance))
	{
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR 스냅샷 불일치 상세: %s 검증 실패: AnimInstance가 null입니다."),
			*OwnerName,
			NodeTypeName);
		return nullptr;
	}

	const IAnimClassInterface* AnimClass = IAnimClassInterface::GetFromClass(AnimInstance->GetClass());
	if (!AnimClass)
	{
		UE_LOG(LogLyraServerSideRewindDebug, Verbose,
			TEXT("[%s] SSR 스냅샷 불일치 상세: %s 검증 실패: AnimClassInterface가 null입니다. AnimClass=%s"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer snapshot[%d]의 노드 메모리가 null입니다. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (SequencePlayer->GetSequence() != Snapshot.SequenceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR 스냅샷 불일치 상세: SequencePlayer snapshot[%d]의 sequence asset이 다릅니다. node=%s snapshotAsset=%s currentAsset=%s"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer snapshot[%d]의 노드 메모리가 null입니다. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (BlendSpacePlayer->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendSpacePlayer snapshot[%d]의 blend space asset이 다릅니다. node=%s snapshotAsset=%s currentAsset=%s"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]의 노드 메모리가 null입니다. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (RotationOffsetBlendSpace->GetBlendSpace() != Snapshot.BlendSpaceAsset.Get())
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]의 blend space asset이 다릅니다. node=%s snapshotAsset=%s currentAsset=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: RotationOffsetBlendSpace snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 노드 메모리가 null입니다. %s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 BlendPoseCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 BlendTimeCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 custom curve가 다릅니다. node=%s snapshotCurve=%s currentCurve=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 blend profile이 다릅니다. node=%s snapshotProfile=%s currentProfile=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 캡처된 PerBlendData 개수가 일관되지 않습니다. node=%s perBlendData=%d blendPoseCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: BlendListByBool snapshot[%d]의 캡처된 PerBoneSampleData 개수가 일관되지 않습니다. node=%s perBoneSampleData=%d blendPoseCount=%d"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 노드 메모리가 null입니다. %s"),
				*OwnerName,
				SnapshotIndex,
				*SnapshotIdentity);
			continue;
		}

		if (StateMachine->StateMachineIndexInClass != Snapshot.StateMachineIndexInClass)
		{
			bMatches = false;
			UE_LOG(LogLyraServerSideRewindDebug, Verbose,
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 StateMachineIndexInClass가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 현재 machine description이 없습니다. node=%s snapshotMachine=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 machine 이름이 다릅니다. node=%s snapshot=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 state 개수가 다릅니다. node=%s snapshot=%d current=%d machine=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: StateMachine snapshot[%d]의 transition 개수가 다릅니다. node=%s snapshot=%d current=%d machine=%s"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 노드 메모리가 null입니다. %s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 blend mode가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 BlendPoseCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 BlendWeightCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 LayerSetupCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 BlendMaskCount가 다릅니다. node=%s snapshot=%d current=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 캡처된 BlendWeights 개수가 일관되지 않습니다. node=%s captured=%d expected=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 캡처된 BlendMasks 개수가 일관되지 않습니다. node=%s captured=%d expected=%d"),
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
					TEXT("[%s] SSR 스냅샷 불일치 상세: LayeredBoneBlend snapshot[%d]의 blend mask[%d]가 다릅니다. node=%s snapshotMask=%s currentMask=%s"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone snapshot[%d]의 노드 메모리가 null입니다. %s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: CopyBone snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u snapshotSource=%s currentSource=%s snapshotTarget=%s currentTarget=%s snapshotSpace=%d currentSpace=%d snapshotCopyTRS={%d,%d,%d} currentCopyTRS={%d,%d,%d}"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone snapshot[%d]의 노드 메모리가 null입니다. %s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TransformBone snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u snapshotBone=%s currentBone=%s snapshotModes={%d,%d,%d} currentModes={%d,%d,%d} snapshotSpaces={%d,%d,%d} currentSpaces={%d,%d,%d}"),
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
			TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK 개수가 다릅니다. snapshot=%d current=%d AnimClass=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK snapshot[%d]의 노드 인덱스가 유효하지 않습니다. %s currentNodeCount=%d"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK snapshot[%d]가 지원하지 않는 노드 타입을 가리킵니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK snapshot[%d]의 노드 식별자가 다릅니다. expected=%s current=%s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK snapshot[%d]의 노드 메모리가 null입니다. %s"),
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
				TEXT("[%s] SSR 스냅샷 불일치 상세: TwoBoneIK snapshot[%d]의 설정 해시가 다릅니다. node=%s snapshotHash=%u currentHash=%u snapshotIKBone=%s currentIKBone=%s snapshotEffector={%s,%d} currentEffector={%s,%d} snapshotJoint={%s,%d} currentJoint={%s,%d} snapshotSpaces={%d,%d} currentSpaces={%d,%d} snapshotFlags={%d,%d,%d,%d} currentFlags={%d,%d,%d,%d}"),
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

void ULyraSnapShotComponent_AnimNode::LogUnsupportedSSRNodeProperties(UAnimInstance* AnimInstance) const
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
			TEXT("SSR에서 지원하지 않는 애니메이션 노드를 건너뜁니다. AnimClass=%s nodeIndex=%d %s"),
			*GetNameSafe(AnimInstance->GetClass()),
			NodeIndex,
			*DescribeAnimNodeProperty(NodeProperty));
	}
}
