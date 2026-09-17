// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSnapShotComponent_AnimNode.h"
#include "LyraSnapShotComponent_AnimNode_Internal.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "ReferenceSkeleton.h"
#include "Stats/Stats.h"

DECLARE_CYCLE_STAT(TEXT("DrawSavedSnapshotDebug"), STAT_LyraSSR_DrawSavedSnapshotDebug, STATGROUP_LyraSSR);
DECLARE_CYCLE_STAT(TEXT("DrawServerCurrentMeshPoseDebug"), STAT_LyraSSR_DrawServerCurrentMeshPoseDebug, STATGROUP_LyraSSR);
DECLARE_CYCLE_STAT(TEXT("DrawClientCurrentMeshPoseDebug"), STAT_LyraSSR_DrawClientCurrentMeshPoseDebug, STATGROUP_LyraSSR);

using namespace LyraAnimNodeSnapshotPrivate;

namespace
{
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

FString MakeBoneControllerComparisonLabel(const TCHAR* StateLabel, const FAnimSSRFrameSnapshot& Snapshot)
{
	int32 CopyBoneCount = 0;
	int32 TransformBoneCount = 0;
	int32 TwoBoneIKCount = 0;
	for (const FAnimSSRAnimInstanceSnapshot& AnimInstanceSnapshot : Snapshot.AnimInstanceSnapshots)
	{
		CopyBoneCount += AnimInstanceSnapshot.NodeSnapshots.CopyBones.Num();
		TransformBoneCount += AnimInstanceSnapshot.NodeSnapshots.TransformBones.Num();
		TwoBoneIKCount += AnimInstanceSnapshot.NodeSnapshots.TwoBoneIKs.Num();
	}

	return FString::Printf(
		TEXT("SSR C/T/IK %s  C=%d T=%d IK=%d"),
		StateLabel,
		CopyBoneCount,
		TransformBoneCount,
		TwoBoneIKCount);
}
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
			// 이 stat은 snapshot 복원/평가 비용을 제외하고 debug primitive 제출 비용만 측정한다.
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
