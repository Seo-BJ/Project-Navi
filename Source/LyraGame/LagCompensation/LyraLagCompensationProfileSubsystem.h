// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LagCompensation/LyraLagCompensationProfile.h"
#include "Subsystems/WorldSubsystem.h"
#include "LyraLagCompensationProfileSubsystem.generated.h"

class ALyraLagCompensationTestCharacter;

/**
 * Dedicated-server-only controller for Lyra SSR Unreal Insights captures.
 *
 * This subsystem controls warmup, workload injection, trace regions, and validity
 * counters. Timing samples are intentionally not buffered or written at runtime;
 * the .utrace session is the source of truth.
 */
UCLASS()
class LYRAGAME_API ULyraLagCompensationProfileSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	bool IsMeasurementActive() const;
	bool ShouldObserveAnimationExecution() const;
	void RecordSnapshotExecution(uint64 FrameNumber);
	void RecordAnimationExecution(bool bDispatchedParallelWork);

private:
	enum class ESessionState : uint8
	{
		WaitingForWorld,
		Warmup,
		MicroWarmup,
		Measuring,
		Finished
	};

	static constexpr int32 ExpectedCharacterCount = 10;

	void TryStartSession(double NowSeconds);
	void BeginMeasurement(double NowSeconds);
	void TickRewindMicro();
	void TickRewindLoad(double NowSeconds);
	bool ExecuteInterpolatedMiss();
	void FinishSession(double NowSeconds);
	void RecordServerFrame(double NowSeconds);
	void ValidateCompletedSnapshotFrame(uint64 FrameNumber, int32 CallsInFrame);
	void PublishTraceCounters() const;
	void RestoreAnimationCVars();
	bool ValidateAnimationCVars() const;
	bool InspectAnimationExecutionConfiguration();
	bool ApplyAnimationModeCVars();
	void Invalidate(uint32 ReasonBit);

	ELyraSSRProfileVariant Variant = ELyraSSRProfileVariant::AnimNode;
	ELyraSSRProfileMode Mode = ELyraSSRProfileMode::None;
	ELyraSSRAnimationMode AnimationMode = ELyraSSRAnimationMode::Parallel;
	ELyraSSRRewindScenario RewindScenario = ELyraSSRRewindScenario::None;
	ESessionState State = ESessionState::WaitingForWorld;

	TArray<TWeakObjectPtr<ALyraLagCompensationTestCharacter>> Targets;

	int32 TargetIndex = 0;
	int32 MicroWarmupCount = 0;
	int32 SnapshotCallCount = 0;
	int32 SnapshotMeasuredFrameCount = 0;
	int32 SnapshotCallsInCurrentFrame = 0;
	int32 ServerFrameCount = 0;
	int32 RewindRequestCount = 0;
	int32 RewindHitCount = 0;
	int32 RewindMissCount = 0;
	int32 RewindInvalidCount = 0;
	int32 MissedInjectionCount = 0;
	int32 ParallelAnimationTickCount = 0;
	int32 NonParallelAnimationTickCount = 0;
	int32 ParallelCapableAnimInstanceCount = 0;
	int32 GameThreadOnlyAnimInstanceCount = 0;
	uint32 InvalidReasonMask = 0;
	uint64 SessionStartFrame = 0;
	uint64 MeasurementStartFrame = 0;
	uint64 MeasurementEndFrame = 0;
	uint64 CurrentSnapshotFrame = MAX_uint64;
	double SessionDiscoveryStartSeconds = 0.0;
	double WarmupStartSeconds = 0.0;
	double MeasurementStartSeconds = 0.0;
	double LastServerFrameSeconds = 0.0;
	double NextLoadRequestSeconds = 0.0;

	bool bTraceRegionOpen = false;
	bool bSavedAnimationCVars = false;
	int32 SavedParallelEvaluation = 0;
	int32 SavedParallelUpdate = 0;
	int32 SavedParallelInterpolation = 0;
	int32 SavedForceParallelUpdate = 0;
	float SavedMaxFPS = 0.0f;
};
