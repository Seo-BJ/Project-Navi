// Fill out your copyright notice in the Description page of Project Settings.

#include "LyraSnapShotComponent_BoxComponent.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "LagCompensation/ILagCompensationTarget.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogLagCompensation);

DECLARE_STATS_GROUP(TEXT("Lyra Lag Compensation Box Snapshot"), STATGROUP_LyraLagBoxSnapshot, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("BoxSnapshot - UpdateHistory"), STAT_LyraLagBoxSnapshot_UpdateHistory, STATGROUP_LyraLagBoxSnapshot);
DECLARE_CYCLE_STAT(TEXT("BoxSnapshot - HistorySearch"), STAT_LyraLagBoxSnapshot_HistorySearch, STATGROUP_LyraLagBoxSnapshot);
DECLARE_CYCLE_STAT(TEXT("BoxSnapshot - Interpolate"), STAT_LyraLagBoxSnapshot_Interpolate, STATGROUP_LyraLagBoxSnapshot);
DECLARE_CYCLE_STAT(TEXT("BoxSnapshot - CaptureFrame"), STAT_LyraLagBoxSnapshot_CaptureFrame, STATGROUP_LyraLagBoxSnapshot);
DECLARE_DWORD_COUNTER_STAT(TEXT("BoxSnapshot - History Frames"), STAT_LyraLagBoxSnapshot_HistoryCount, STATGROUP_LyraLagBoxSnapshot);
DECLARE_MEMORY_STAT(TEXT("BoxSnapshot - History Memory"), STAT_LyraLagBoxSnapshot_HistoryMemory, STATGROUP_LyraLagBoxSnapshot);

ULyraSnapShotComponent_BoxComponent::ULyraSnapShotComponent_BoxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraSnapShotComponent_BoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	if (bDrawFrameHistory)
	{
		++TickCounter;
		if (TickCounter % DrawDebugFrequency == 0 && SnapShotHistory.Num() > 0)
		{
			DrawDebugFramePackage(SnapShotHistory.First());
		}
	}
#endif
}

void ULyraSnapShotComponent_BoxComponent::UpdateSnapShotHistory()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagBoxSnapshot_UpdateHistory);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBoxSnapshot_UpdateHistory);

	FLyraBoxSnapShotFrame ThisFrame;
	CaptureCurrentSnapShot(GetOwner(), ThisFrame);
	if (ThisFrame.HitBoxInfo.IsEmpty())
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

	INC_DWORD_STAT_BY(STAT_LyraLagBoxSnapshot_HistoryCount, SnapShotHistory.Num());
	SET_MEMORY_STAT(STAT_LyraLagBoxSnapshot_HistoryMemory, SnapShotHistory.GetAllocatedSize());
}

void ULyraSnapShotComponent_BoxComponent::CaptureCurrentSnapShot(AActor* HitActor, FLyraBoxSnapShotFrame& OutFramePackage) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagBoxSnapshot_CaptureFrame);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBoxSnapshot_CaptureFrame);

	OutFramePackage = FLyraBoxSnapShotFrame();
	if (!IsValid(HitActor))
	{
		return;
	}

	const ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	const UWorld* World = GetWorld();
	if (!Target || !IsValid(World))
	{
		return;
	}

	OutFramePackage.HitActor = HitActor;
	OutFramePackage.Time = World->GetTimeSeconds();

	for (const TPair<FName, TObjectPtr<UBoxComponent>>& BoxPair : Target->GetHitCollisionBoxes())
	{
		UBoxComponent* BoxComponent = BoxPair.Value.Get();
		if (!IsValid(BoxComponent))
		{
			continue;
		}

		OutFramePackage.HitBoxInfo.Add(BoxPair.Key, CaptureBoxInformation(*BoxComponent));
	}
}

FLyraBoxInformation ULyraSnapShotComponent_BoxComponent::CaptureBoxInformation(const UBoxComponent& BoxComponent)
{
	FLyraBoxInformation BoxInformation;
	BoxInformation.WorldTransform = BoxComponent.GetComponentTransform();
	BoxInformation.UnscaledBoxExtent = BoxComponent.GetUnscaledBoxExtent();
	return BoxInformation;
}

void ULyraSnapShotComponent_BoxComponent::ApplyBoxInformation(
	UBoxComponent& BoxComponent,
	const FLyraBoxInformation& BoxInformation)
{
	BoxComponent.SetWorldTransform(BoxInformation.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
	BoxComponent.SetBoxExtent(BoxInformation.UnscaledBoxExtent, false);
}

bool ULyraSnapShotComponent_BoxComponent::TryGetSnapShotAtTime(float HitTime, FLyraBoxSnapShotFrame& OutFramePackage) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagBoxSnapshot_HistorySearch);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBoxSnapshot_HistorySearch);

	OutFramePackage = FLyraBoxSnapShotFrame();
	if (SnapShotHistory.Num() == 0)
	{
		return false;
	}

	const float OldestHistoryTime = SnapShotHistory.Last().Time;
	const float NewestHistoryTime = SnapShotHistory.First().Time;

	if (OldestHistoryTime > HitTime)
	{
		return false;
	}
	if (FMath::IsNearlyEqual(OldestHistoryTime, HitTime))
	{
		OutFramePackage = SnapShotHistory.Last();
		return true;
	}
	if (FMath::IsNearlyEqual(NewestHistoryTime, HitTime))
	{
		OutFramePackage = SnapShotHistory.First();
		return true;
	}

	if (NewestHistoryTime < HitTime)
	{
		OutFramePackage = SnapShotHistory.Num() >= 2
			? ExtrapolateByTwoFrames(SnapShotHistory[1], SnapShotHistory[0], HitTime)
			: SnapShotHistory.First();
		return !OutFramePackage.HitBoxInfo.IsEmpty();
	}

	const int32 Count = SnapShotHistory.Num();
	int32 YoungerIdx = 0;
	int32 OlderIdx = 0;
	while (OlderIdx + 1 < Count && SnapShotHistory[OlderIdx].Time > HitTime)
	{
		++OlderIdx;
		if (SnapShotHistory[OlderIdx].Time > HitTime)
		{
			YoungerIdx = OlderIdx;
		}
	}

	OutFramePackage = FMath::IsNearlyEqual(SnapShotHistory[OlderIdx].Time, HitTime)
		? SnapShotHistory[OlderIdx]
		: InterpolateBetweenTwoFrames(SnapShotHistory[OlderIdx], SnapShotHistory[YoungerIdx], HitTime);

	return !OutFramePackage.HitBoxInfo.IsEmpty();
}

FLyraBoxSnapShotFrame ULyraSnapShotComponent_BoxComponent::InterpolateBetweenTwoFrames(
	const FLyraBoxSnapShotFrame& OlderFrame,
	const FLyraBoxSnapShotFrame& YoungerFrame,
	float HitTime
	) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBoxSnapshot_Interpolate);

	const float DeltaTime = YoungerFrame.Time - OlderFrame.Time;
	if (FMath::IsNearlyZero(DeltaTime))
	{
		return YoungerFrame;
	}

	const float InterpolateFraction = FMath::Clamp((HitTime - OlderFrame.Time) / DeltaTime, 0.0f, 1.0f);
	FLyraBoxSnapShotFrame InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	InterpFramePackage.HitActor = YoungerFrame.HitActor;

	for (const TPair<FName, FLyraBoxInformation>& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FLyraBoxInformation* OlderBoxPtr = OlderFrame.HitBoxInfo.Find(BoxInfoName);
		if (!OlderBoxPtr)
		{
			continue;
		}

		const FLyraBoxInformation& OlderBox = *OlderBoxPtr;
		const FLyraBoxInformation& YoungerBox = YoungerPair.Value;

		FLyraBoxInformation InterpBoxInfo;
		InterpBoxInfo.WorldTransform.Blend(OlderBox.WorldTransform, YoungerBox.WorldTransform, InterpolateFraction);
		InterpBoxInfo.UnscaledBoxExtent = FMath::Lerp(OlderBox.UnscaledBoxExtent, YoungerBox.UnscaledBoxExtent, InterpolateFraction);
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FLyraBoxSnapShotFrame ULyraSnapShotComponent_BoxComponent::ExtrapolateByTwoFrames(
	const FLyraBoxSnapShotFrame& SecondNewestFrame,
	const FLyraBoxSnapShotFrame& FirstNewestFrame,
	float HitTime
	) const
{
	FLyraBoxSnapShotFrame ExtrapolateFramePackage;
	ExtrapolateFramePackage.Time = HitTime;
	ExtrapolateFramePackage.HitActor = FirstNewestFrame.HitActor;

	const float ExtrapolationTime = HitTime - SecondNewestFrame.Time;
	const float DeltaTime = FirstNewestFrame.Time - SecondNewestFrame.Time;
	if (FMath::IsNearlyZero(DeltaTime))
	{
		return FirstNewestFrame;
	}

	for (const TPair<FName, FLyraBoxInformation>& FirstPair : FirstNewestFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = FirstPair.Key;
		const FLyraBoxInformation* SecondBoxPtr = SecondNewestFrame.HitBoxInfo.Find(BoxInfoName);
		if (!SecondBoxPtr)
		{
			continue;
		}

		const FLyraBoxInformation& SecondHitBox = *SecondBoxPtr;
		const FLyraBoxInformation& FirstHitBox = FirstPair.Value;
		const float Alpha = ExtrapolationTime / DeltaTime;

		FLyraBoxInformation ExtrapolatedBoxInfo;
		const FVector ExtrapolatedLocation = SecondHitBox.WorldTransform.GetLocation()
			+ Alpha * (FirstHitBox.WorldTransform.GetLocation() - SecondHitBox.WorldTransform.GetLocation());
		const FRotator SecondRotation = SecondHitBox.WorldTransform.Rotator();
		const FRotator FirstRotation = FirstHitBox.WorldTransform.Rotator();
		const FRotator ExtrapolatedRotation = SecondRotation + Alpha * (FirstRotation - SecondRotation).GetNormalized();
		ExtrapolatedBoxInfo.WorldTransform = FTransform(
			ExtrapolatedRotation,
			ExtrapolatedLocation,
			FirstHitBox.WorldTransform.GetScale3D());
		ExtrapolatedBoxInfo.UnscaledBoxExtent = FirstHitBox.UnscaledBoxExtent;

		ExtrapolateFramePackage.HitBoxInfo.Add(BoxInfoName, ExtrapolatedBoxInfo);
	}

	return ExtrapolateFramePackage;
}

void ULyraSnapShotComponent_BoxComponent::ApplySnapShot(AActor* HitActor, const FLyraBoxSnapShotFrame& Package) const
{
	if (!IsValid(HitActor))
	{
		return;
	}

	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	if (!Target)
	{
		return;
	}

	for (const TPair<FName, TObjectPtr<UBoxComponent>>& HitBoxPair : Target->GetHitCollisionBoxes())
	{
		UBoxComponent* BoxComponent = HitBoxPair.Value.Get();
		const FLyraBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
		if (IsValid(BoxComponent) && BoxValue)
		{
			ApplyBoxInformation(*BoxComponent, *BoxValue);
		}
	}
}

#if ENABLE_DRAW_DEBUG
void ULyraSnapShotComponent_BoxComponent::DrawDebugFramePackage(const FLyraBoxSnapShotFrame& FramePackage) const
{
	for (const TPair<FName, FLyraBoxInformation>& BoxInfo : FramePackage.HitBoxInfo)
	{
		const FTransform& WorldTransform = BoxInfo.Value.WorldTransform;
		const FVector ScaledBoxExtent = BoxInfo.Value.UnscaledBoxExtent * WorldTransform.GetScale3D().GetAbs();
		DrawDebugBox(GetWorld(), WorldTransform.GetLocation(), ScaledBoxExtent, WorldTransform.GetRotation(), FColor::Green, false, DrawDebugHitBoxTime);
	}
}
#endif
