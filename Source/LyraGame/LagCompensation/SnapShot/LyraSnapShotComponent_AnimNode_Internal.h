// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraSnapShotComponent_AnimNode.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("LyraSSR"), STATGROUP_LyraSSR, STATCAT_Advanced);

class FStructProperty;
class UAnimInstance;

namespace LyraAnimNodeSnapshotPrivate
{
int32 CountStateMachineTransitions(const FBakedAnimationStateMachine* Machine);

uint32 HashLayeredBoneBlendConfiguration(const FAnimNode_LayeredBoneBlend& Node);
uint32 HashRotationOffsetBlendSpaceConfiguration(const FAnimNode_RotationOffsetBlendSpace& Node);
uint32 HashBlendListByBoolConfiguration(const FAnimNode_BlendListByBool& Node);
uint32 HashCopyBoneConfiguration(const FAnimNode_CopyBone& Node);
uint32 HashTransformBoneConfiguration(const FAnimNode_ModifyBone& Node);
uint32 HashTwoBoneIKConfiguration(const FAnimNode_TwoBoneIK& Node);

FName GetBoneSocketTargetSnapshotName(const FBoneSocketTarget& Target);

bool IsSequencePlayerNodeProperty(const FStructProperty* NodeProperty);
bool IsBlendSpacePlayerNodeProperty(const FStructProperty* NodeProperty);
bool IsRotationOffsetBlendSpaceNodeProperty(const FStructProperty* NodeProperty);
bool IsBlendListByBoolNodeProperty(const FStructProperty* NodeProperty);
bool IsStateMachineNodeProperty(const FStructProperty* NodeProperty);
bool IsLayeredBoneBlendNodeProperty(const FStructProperty* NodeProperty);
bool IsCopyBoneNodeProperty(const FStructProperty* NodeProperty);
bool IsTransformBoneNodeProperty(const FStructProperty* NodeProperty);
bool IsTwoBoneIKNodeProperty(const FStructProperty* NodeProperty);
bool IsSlotNodeProperty(const FStructProperty* NodeProperty);
bool IsUnsupportedSSRNodeProperty(const FStructProperty* NodeProperty);

FString DescribeAnimNodeProperty(const FStructProperty* NodeProperty);
FString DescribeSnapshotNodeIdentity(const FAnimSSRNodeIdentity& Identity);
void CaptureSnapshotNodeIdentity(int32 NodeIndex, const FStructProperty* NodeProperty, FAnimSSRNodeIdentity& OutIdentity);
const TArray<FStructProperty*>* GetSSRAnimNodeProperties(UAnimInstance* AnimInstance);

bool ShouldDrawDebugPose(bool bRequested, const FAnimSSRDebugPoseDrawSettings& DrawSettings, uint64& Counter);
}
