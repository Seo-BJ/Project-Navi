// Fill out your copyright notice in the Description page of Project Settings.

#include "LyraSnapShotComponent_SkeletalMesh.h"

#include "Components/SkeletalMeshComponent.h"
#include "Development/LyraDeveloperSettings.h"
#include "GameFramework/Character.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Lyra Lag Compensation Mesh Snapshot"), STATGROUP_LyraLagMeshSnapshot, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("MeshSnapshot - UpdateHistory"), STAT_LyraLagMeshSnapshot_UpdateHistory, STATGROUP_LyraLagMeshSnapshot);
DECLARE_CYCLE_STAT(TEXT("MeshSnapshot - HistorySearch"), STAT_LyraLagMeshSnapshot_HistorySearch, STATGROUP_LyraLagMeshSnapshot);
DECLARE_CYCLE_STAT(TEXT("MeshSnapshot - Interpolate"), STAT_LyraLagMeshSnapshot_Interpolate, STATGROUP_LyraLagMeshSnapshot);
DECLARE_CYCLE_STAT(TEXT("MeshSnapshot - CaptureFrame"), STAT_LyraLagMeshSnapshot_CaptureFrame, STATGROUP_LyraLagMeshSnapshot);
DECLARE_DWORD_COUNTER_STAT(TEXT("MeshSnapshot - History Frames"), STAT_LyraLagMeshSnapshot_HistoryCount, STATGROUP_LyraLagMeshSnapshot);
DECLARE_MEMORY_STAT(TEXT("MeshSnapshot - History Memory"), STAT_LyraLagMeshSnapshot_HistoryMemory, STATGROUP_LyraLagMeshSnapshot);

ULyraSnapShotComponent_SkeletalMesh::ULyraSnapShotComponent_SkeletalMesh()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

USkeletalMeshComponent* ULyraSnapShotComponent_SkeletalMesh::GetSkeletalMesh(AActor* InActor) const
{
	if (!IsValid(InActor))
	{
		return nullptr;
	}

	if (const ACharacter* Character = Cast<ACharacter>(InActor))
	{
		return Character->GetMesh();
	}

	return InActor->FindComponentByClass<USkeletalMeshComponent>();
}

void ULyraSnapShotComponent_SkeletalMesh::UpdateSnapShotHistory()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagMeshSnapshot_UpdateHistory);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagMeshSnapshot_UpdateHistory);

	FLyraMeshSnapShotFrame ThisFrame;
	CaptureCurrentSnapShot(GetOwner(), ThisFrame);
	if (ThisFrame.BoneTransforms.IsEmpty())
	{
		return;
	}

	SnapShotHistory.EmplaceFirst(MoveTemp(ThisFrame));

	if (SnapShotHistory.Num() >= 2)
	{
		float HistoryLength = SnapShotHistory.First().Time - SnapShotHistory.Last().Time;
		while (HistoryLength > MaxRecordTime && SnapShotHistory.Num() >= 2)
		{
			SnapShotHistory.PopLast();
			HistoryLength = SnapShotHistory.First().Time - SnapShotHistory.Last().Time;
		}
	}

	INC_DWORD_STAT_BY(STAT_LyraLagMeshSnapshot_HistoryCount, SnapShotHistory.Num());
	SET_MEMORY_STAT(STAT_LyraLagMeshSnapshot_HistoryMemory, SnapShotHistory.GetAllocatedSize());
}

void ULyraSnapShotComponent_SkeletalMesh::CaptureCurrentSnapShot(AActor* HitActor, FLyraMeshSnapShotFrame& OutPackage) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagMeshSnapshot_CaptureFrame);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagMeshSnapshot_CaptureFrame);

	OutPackage = FLyraMeshSnapShotFrame();
	if (!IsValid(HitActor))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = GetSkeletalMesh(HitActor);
	if (!IsValid(Mesh))
	{
		return;
	}

	OutPackage.Time = World->GetTimeSeconds();
	OutPackage.HitActor = HitActor;
	OutPackage.MeshTransform = Mesh->GetComponentTransform();
	OutPackage.BoneTransforms = Mesh->GetComponentSpaceTransforms();
}

bool ULyraSnapShotComponent_SkeletalMesh::TryGetSnapShotAtTime(float HitTime, FLyraMeshSnapShotFrame& OutPackage) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagMeshSnapshot_HistorySearch);

	OutPackage = FLyraMeshSnapShotFrame();
	if (SnapShotHistory.Num() == 0)
	{
		return false;
	}

	const float OldestTime = SnapShotHistory.Last().Time;
	const float NewestTime = SnapShotHistory.First().Time;

	if (OldestTime > HitTime)
	{
		return false;
	}
	if (NewestTime <= HitTime)
	{
		OutPackage = SnapShotHistory.First();
		return !OutPackage.BoneTransforms.IsEmpty();
	}

	const int32 Count = SnapShotHistory.Num();
	int32 YoungerIdx = 0;
	while (YoungerIdx + 1 < Count && SnapShotHistory[YoungerIdx + 1].Time > HitTime)
	{
		++YoungerIdx;
	}

	OutPackage = (YoungerIdx + 1 < Count)
		? InterpolateFrame(SnapShotHistory[YoungerIdx + 1], SnapShotHistory[YoungerIdx], HitTime)
		: SnapShotHistory.Last();

	return !OutPackage.BoneTransforms.IsEmpty();
}

FLyraMeshSnapShotFrame ULyraSnapShotComponent_SkeletalMesh::InterpolateFrame(const FLyraMeshSnapShotFrame& Older, const FLyraMeshSnapShotFrame& Younger, float HitTime) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagMeshSnapshot_Interpolate);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagMeshSnapshot_Interpolate);

	const float DeltaTime = Younger.Time - Older.Time;
	if (FMath::IsNearlyZero(DeltaTime) || Older.BoneTransforms.Num() != Younger.BoneTransforms.Num())
	{
		return Younger;
	}

	const float Alpha = FMath::Clamp((HitTime - Older.Time) / DeltaTime, 0.0f, 1.0f);

	FLyraMeshSnapShotFrame InterpPackage;
	InterpPackage.Time = HitTime;
	InterpPackage.HitActor = Younger.HitActor;
	InterpPackage.MeshTransform.Blend(Older.MeshTransform, Younger.MeshTransform, Alpha);
	InterpPackage.BoneTransforms.SetNum(Older.BoneTransforms.Num());

	for (int32 BoneIndex = 0; BoneIndex < Older.BoneTransforms.Num(); ++BoneIndex)
	{
		InterpPackage.BoneTransforms[BoneIndex].Blend(Older.BoneTransforms[BoneIndex], Younger.BoneTransforms[BoneIndex], Alpha);
	}

	return InterpPackage;
}

#if ENABLE_DRAW_DEBUG
void ULyraSnapShotComponent_SkeletalMesh::DrawDebugPose(const FLyraMeshSnapShotFrame& Package, FColor Color) const
{
	if (!IsValid(Package.HitActor))
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMeshComp = GetSkeletalMesh(Package.HitActor);
	const ULyraLagCompensationDeveloperSettings* Settings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	const float LifeTime = IsValid(Settings) ? Settings->DrawMeshLagCompensationDuration : 5.0f;

	DrawPhysicsAssetPose(
		SkeletalMeshComp,
		Package.MeshTransform,
		Package.BoneTransforms,
		Color,
		LifeTime);
}
#endif
