// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "LagCompensation/LyraLagCompensationTypes.h"

class UWorld;

enum class ELyraLagCompProfileMetric : uint8
{
	DedicatedServerWorldTick,
	DedicatedServerWorldTickInterval,
	ServerSideRewind,
	SnapshotUpdate,
	LyraCharacterMeshTick
};

FORCEINLINE uint32 GetTypeHash(ELyraLagCompProfileMetric Metric)
{
	return ::GetTypeHash(static_cast<uint8>(Metric));
}

class FLyraLagCompProfiler
{
public:
	static FLyraLagCompProfiler& Get();

	void Initialize();
	void Shutdown();

	bool IsActive() const { return bActive; }
	bool IsActiveForWorld(const UWorld* World) const;

	void RecordDuration(
		ELyraLagCompProfileMetric Metric,
		const UWorld* World,
		ELyraSnapShotMode SnapShotMode,
		double DurationMicroseconds,
		bool bSuccess = true,
		const FString& OwnerName = FString(),
		const FString& ComponentName = FString(),
		const FString& Extra = FString());

private:
	struct FProfileSample
	{
		int64 SampleIndex = 0;
		double SessionTimeSeconds = 0.0;
		double WorldTimeSeconds = 0.0;
		ELyraLagCompProfileMetric Metric = ELyraLagCompProfileMetric::DedicatedServerWorldTick;
		ELyraSnapShotMode SnapShotMode = ELyraSnapShotMode::None;
		double DurationMicroseconds = 0.0;
		bool bSuccess = true;
		FString OwnerName;
		FString ComponentName;
		FString Extra;
	};

	struct FMetricAccumulator
	{
		int64 Count = 0;
		int64 SuccessCount = 0;
		double TotalMicroseconds = 0.0;
		double MinMicroseconds = TNumericLimits<double>::Max();
		double MaxMicroseconds = 0.0;

		void Add(double DurationMicroseconds, bool bSuccess);
	};

	FLyraLagCompProfiler() = default;

	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void OnWorldTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	bool CanAutoStartForWorld(const UWorld* World) const;
	void TryAutoStart(UWorld* World);
	void StartSession(UWorld* World, ELyraSnapShotMode SnapShotMode, float DurationSeconds);
	void StopSession(const TCHAR* Reason);
	void ResetSessionData();
	void WriteCsvFiles(const TCHAR* Reason) const;

	ELyraSnapShotMode DetectSnapShotMode(const UWorld* World) const;

	bool bInitialized = false;
	bool bActive = false;
	bool bAutoStartConsumed = false;
	bool bHasWorldTickStart = false;
	bool bHasPreviousWorldTickStart = false;

	TWeakObjectPtr<UWorld> ActiveWorld;
	const UWorld* LastSessionWorld = nullptr;
	ELyraSnapShotMode SessionSnapShotMode = ELyraSnapShotMode::None;
	float SessionDurationSeconds = 60.0f;
	double SessionStartSeconds = 0.0;
	double WorldTickStartSeconds = 0.0;
	double PreviousWorldTickStartSeconds = 0.0;
	int64 NextSampleIndex = 0;
	FDateTime SessionTimestamp;

	TArray<FProfileSample> Samples;
	TMap<ELyraLagCompProfileMetric, FMetricAccumulator> Accumulators;

	FDelegateHandle WorldTickStartHandle;
	FDelegateHandle WorldTickEndHandle;
	FDelegateHandle WorldCleanupHandle;
};
