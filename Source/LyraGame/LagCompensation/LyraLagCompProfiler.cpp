// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/LyraLagCompProfiler.h"

#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraLagCompProfiler, Log, All);

namespace LyraLagCompProfilerCVars
{
#if UE_BUILD_SHIPPING
	static bool bAutoStart = false;
#else
	static bool bAutoStart = true;
#endif
	static FAutoConsoleVariableRef CVarAutoStart(
		TEXT("lyra.LagComp.Profile.AutoStart"),
		bAutoStart,
		TEXT("Automatically starts a dedicated-server lag compensation profiling session in Play As Client."),
		ECVF_Default);

	static float DurationSeconds = 120.0f;
	static FAutoConsoleVariableRef CVarDurationSeconds(
		TEXT("lyra.LagComp.Profile.DurationSeconds"),
		DurationSeconds,
		TEXT("Duration in seconds for automatic lag compensation profiling sessions."),
		ECVF_Default);
}

namespace
{
const TCHAR* ToMetricName(ELyraLagCompProfileMetric Metric)
{
	switch (Metric)
	{
	case ELyraLagCompProfileMetric::DedicatedServerWorldTick:
		return TEXT("DedicatedServerWorldTick");
	case ELyraLagCompProfileMetric::DedicatedServerWorldTickInterval:
		return TEXT("DedicatedServerWorldTickInterval");
	case ELyraLagCompProfileMetric::ServerSideRewind:
		return TEXT("ServerSideRewind");
	case ELyraLagCompProfileMetric::SnapshotUpdate:
		return TEXT("SnapshotUpdate");
	case ELyraLagCompProfileMetric::LyraCharacterMeshTick:
		return TEXT("LyraCharacterMeshTick");
	default:
		return TEXT("Unknown");
	}
}

const TCHAR* ToSnapShotModeName(ELyraSnapShotMode Mode)
{
	switch (Mode)
	{
	case ELyraSnapShotMode::BoxComponent:
		return TEXT("BoxComponent");
	case ELyraSnapShotMode::SkeletalMesh:
		return TEXT("SkelSnapshot");
	case ELyraSnapShotMode::AnimNode:
		return TEXT("AnimNode");
	case ELyraSnapShotMode::None:
	default:
		return TEXT("None");
	}
}

FString EscapeCsv(const FString& Value)
{
	if (!Value.Contains(TEXT(",")) && !Value.Contains(TEXT("\"")) && !Value.Contains(TEXT("\n")) && !Value.Contains(TEXT("\r")))
	{
		return Value;
	}

	FString Escaped = Value;
	Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));
	return FString::Printf(TEXT("\"%s\""), *Escaped);
}

FString SanitizeFilenameToken(FString Token)
{
	if (Token.IsEmpty())
	{
		return TEXT("Unknown");
	}

	const TCHAR* InvalidChars = TEXT("\\/:*?\"<>| ");
	for (const TCHAR* Char = InvalidChars; *Char != TEXT('\0'); ++Char)
	{
		Token.ReplaceCharInline(*Char, TEXT('_'));
	}
	return Token;
}

double GetTickRateFPSFromInterval(ELyraLagCompProfileMetric Metric, double DurationMicroseconds)
{
	if (Metric != ELyraLagCompProfileMetric::DedicatedServerWorldTickInterval || DurationMicroseconds <= 0.0)
	{
		return 0.0;
	}

	return 1000000.0 / DurationMicroseconds;
}

static constexpr ELyraLagCompProfileMetric AllMetrics[] =
{
	ELyraLagCompProfileMetric::DedicatedServerWorldTick,
	ELyraLagCompProfileMetric::DedicatedServerWorldTickInterval,
	ELyraLagCompProfileMetric::ServerSideRewind,
	ELyraLagCompProfileMetric::SnapshotUpdate,
	ELyraLagCompProfileMetric::LyraCharacterMeshTick
};
}

FLyraLagCompProfiler& FLyraLagCompProfiler::Get()
{
	static FLyraLagCompProfiler Instance;
	return Instance;
}

void FLyraLagCompProfiler::FMetricAccumulator::Add(double DurationMicroseconds, bool bSuccess)
{
	++Count;
	SuccessCount += bSuccess ? 1 : 0;
	TotalMicroseconds += DurationMicroseconds;
	MinMicroseconds = FMath::Min(MinMicroseconds, DurationMicroseconds);
	MaxMicroseconds = FMath::Max(MaxMicroseconds, DurationMicroseconds);
}

void FLyraLagCompProfiler::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	WorldTickStartHandle = FWorldDelegates::OnWorldTickStart.AddRaw(this, &FLyraLagCompProfiler::OnWorldTickStart);
	WorldTickEndHandle = FWorldDelegates::OnWorldTickEnd.AddRaw(this, &FLyraLagCompProfiler::OnWorldTickEnd);
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FLyraLagCompProfiler::OnWorldCleanup);
	bInitialized = true;
}

void FLyraLagCompProfiler::Shutdown()
{
	if (bActive)
	{
		StopSession(TEXT("ModuleShutdown"));
	}

	if (bInitialized)
	{
		FWorldDelegates::OnWorldTickStart.Remove(WorldTickStartHandle);
		FWorldDelegates::OnWorldTickEnd.Remove(WorldTickEndHandle);
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		bInitialized = false;
	}
}

bool FLyraLagCompProfiler::IsActiveForWorld(const UWorld* World) const
{
	return bActive && IsValid(World) && ActiveWorld.Get() == World;
}

void FLyraLagCompProfiler::RecordDuration(
	ELyraLagCompProfileMetric Metric,
	const UWorld* World,
	ELyraSnapShotMode SnapShotMode,
	double DurationMicroseconds,
	bool bSuccess,
	const FString& OwnerName,
	const FString& ComponentName,
	const FString& Extra)
{
	if (!IsActiveForWorld(World))
	{
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	const ELyraSnapShotMode SampleMode = SnapShotMode == ELyraSnapShotMode::None ? SessionSnapShotMode : SnapShotMode;

	FProfileSample& Sample = Samples.AddDefaulted_GetRef();
	Sample.SampleIndex = ++NextSampleIndex;
	Sample.SessionTimeSeconds = NowSeconds - SessionStartSeconds;
	Sample.WorldTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	Sample.Metric = Metric;
	Sample.SnapShotMode = SampleMode;
	Sample.DurationMicroseconds = FMath::Max(0.0, DurationMicroseconds);
	Sample.bSuccess = bSuccess;
	Sample.OwnerName = OwnerName;
	Sample.ComponentName = ComponentName;
	Sample.Extra = Extra;

	Accumulators.FindOrAdd(Metric).Add(Sample.DurationMicroseconds, bSuccess);
}

void FLyraLagCompProfiler::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (!IsValid(World) || !World->IsGameWorld() || World->GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	TryAutoStart(World);

	if (IsActiveForWorld(World))
	{
		const double TickStartSeconds = FPlatformTime::Seconds();

		if (bHasPreviousWorldTickStart)
		{
			const double TickIntervalMicroseconds = (TickStartSeconds - PreviousWorldTickStartSeconds) * 1000000.0;
			if (TickIntervalMicroseconds > 0.0)
			{
				RecordDuration(
					ELyraLagCompProfileMetric::DedicatedServerWorldTickInterval,
					World,
					SessionSnapShotMode,
					TickIntervalMicroseconds,
					true,
					TEXT("DedicatedServer"),
					TEXT("UWorld"),
					TEXT("StartToStart"));
			}
		}

		PreviousWorldTickStartSeconds = TickStartSeconds;
		bHasPreviousWorldTickStart = true;
		WorldTickStartSeconds = TickStartSeconds;
		bHasWorldTickStart = true;
	}
}

void FLyraLagCompProfiler::OnWorldTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (!IsActiveForWorld(World))
	{
		return;
	}

	if (bHasWorldTickStart)
	{
		const double DurationMicroseconds = (FPlatformTime::Seconds() - WorldTickStartSeconds) * 1000000.0;
		RecordDuration(
			ELyraLagCompProfileMetric::DedicatedServerWorldTick,
			World,
			SessionSnapShotMode,
			DurationMicroseconds,
			true,
			TEXT("DedicatedServer"),
			TEXT("UWorld"));
		bHasWorldTickStart = false;
	}

	if ((FPlatformTime::Seconds() - SessionStartSeconds) >= SessionDurationSeconds)
	{
		StopSession(TEXT("Completed"));
	}
}

void FLyraLagCompProfiler::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (IsActiveForWorld(World))
	{
		StopSession(TEXT("WorldCleanup"));
	}

	if (LastSessionWorld == World)
	{
		bAutoStartConsumed = false;
		LastSessionWorld = nullptr;
	}
}

bool FLyraLagCompProfiler::CanAutoStartForWorld(const UWorld* World) const
{
	return LyraLagCompProfilerCVars::bAutoStart &&
		!bActive &&
		!bAutoStartConsumed &&
		IsValid(World) &&
		World->IsGameWorld() &&
		World->GetNetMode() == NM_DedicatedServer &&
		LyraLagCompProfilerCVars::DurationSeconds > 0.0f;
}

void FLyraLagCompProfiler::TryAutoStart(UWorld* World)
{
	if (!CanAutoStartForWorld(World))
	{
		return;
	}

	const ELyraSnapShotMode DetectedMode = DetectSnapShotMode(World);
	if (DetectedMode == ELyraSnapShotMode::None)
	{
		return;
	}

	StartSession(World, DetectedMode, LyraLagCompProfilerCVars::DurationSeconds);
}

void FLyraLagCompProfiler::StartSession(UWorld* World, ELyraSnapShotMode SnapShotMode, float DurationSeconds)
{
	ResetSessionData();

	bActive = true;
	bAutoStartConsumed = true;
	ActiveWorld = World;
	LastSessionWorld = World;
	SessionSnapShotMode = SnapShotMode;
	SessionDurationSeconds = FMath::Max(0.001f, DurationSeconds);
	SessionStartSeconds = FPlatformTime::Seconds();
	SessionTimestamp = FDateTime::Now();

	UE_LOG(LogLyraLagCompProfiler, Display,
		TEXT("Started lag compensation profiling. Mode=%s Duration=%.2fs World=%s"),
		ToSnapShotModeName(SessionSnapShotMode),
		SessionDurationSeconds,
		*GetNameSafe(World));
}

void FLyraLagCompProfiler::StopSession(const TCHAR* Reason)
{
	if (!bActive)
	{
		return;
	}

	UE_LOG(LogLyraLagCompProfiler, Display,
		TEXT("Stopping lag compensation profiling. Reason=%s Samples=%d"),
		Reason,
		Samples.Num());

	WriteCsvFiles(Reason);

	bActive = false;
	ActiveWorld.Reset();
	bHasWorldTickStart = false;
	bHasPreviousWorldTickStart = false;
}

void FLyraLagCompProfiler::ResetSessionData()
{
	Samples.Reset();
	Accumulators.Reset();
	NextSampleIndex = 0;
	bHasWorldTickStart = false;
	bHasPreviousWorldTickStart = false;
	SessionSnapShotMode = ELyraSnapShotMode::None;
	SessionStartSeconds = 0.0;
	WorldTickStartSeconds = 0.0;
	PreviousWorldTickStartSeconds = 0.0;
}

void FLyraLagCompProfiler::WriteCsvFiles(const TCHAR* Reason) const
{
	const FString OutputDir = FPaths::ProjectSavedDir() / TEXT("Profiling") / TEXT("LagComp");
	IFileManager::Get().MakeDirectory(*OutputDir, true);

	const FString Timestamp = SessionTimestamp.ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString ModeToken = SanitizeFilenameToken(ToSnapShotModeName(SessionSnapShotMode));
	const FString BaseFileName = FString::Printf(TEXT("%s_%s"), *Timestamp, *ModeToken);
	const FString RawFilePath = OutputDir / FString::Printf(TEXT("%s_raw.csv"), *BaseFileName);
	const FString SummaryFilePath = OutputDir / FString::Printf(TEXT("%s_summary.csv"), *BaseFileName);

	FString RawCsv;
	RawCsv.Reserve(Samples.Num() * 160);
	RawCsv += TEXT("SampleIndex,SessionTimeSeconds,WorldTimeSeconds,Metric,SnapshotMode,DurationMicroseconds,DurationMilliseconds,TickRateFPS,bSuccess,OwnerName,ComponentName,Extra\n");
	for (const FProfileSample& Sample : Samples)
	{
		const double TickRateFPS = GetTickRateFPSFromInterval(Sample.Metric, Sample.DurationMicroseconds);
		RawCsv += FString::Printf(
			TEXT("%lld,%.6f,%.6f,%s,%s,%.3f,%.6f,%.6f,%d,%s,%s,%s\n"),
			Sample.SampleIndex,
			Sample.SessionTimeSeconds,
			Sample.WorldTimeSeconds,
			ToMetricName(Sample.Metric),
			ToSnapShotModeName(Sample.SnapShotMode),
			Sample.DurationMicroseconds,
			Sample.DurationMicroseconds / 1000.0,
			TickRateFPS,
			Sample.bSuccess ? 1 : 0,
			*EscapeCsv(Sample.OwnerName),
			*EscapeCsv(Sample.ComponentName),
			*EscapeCsv(Sample.Extra));
	}

	FString SummaryCsv;
	SummaryCsv += TEXT("SessionLabel,EndReason,Metric,SnapshotMode,SampleCount,SuccessCount,AverageMicroseconds,AverageMilliseconds,AverageTickRateFPS,MinMicroseconds,MaxMicroseconds,TotalMicroseconds\n");
	for (ELyraLagCompProfileMetric Metric : AllMetrics)
	{
		const FMetricAccumulator* Accumulator = Accumulators.Find(Metric);
		const int64 Count = Accumulator ? Accumulator->Count : 0;
		const int64 SuccessCount = Accumulator ? Accumulator->SuccessCount : 0;
		const double TotalMicroseconds = Accumulator ? Accumulator->TotalMicroseconds : 0.0;
		const double AverageMicroseconds = Count > 0 ? TotalMicroseconds / static_cast<double>(Count) : 0.0;
		const double AverageTickRateFPS = GetTickRateFPSFromInterval(Metric, AverageMicroseconds);
		const double MinMicroseconds = Count > 0 ? Accumulator->MinMicroseconds : 0.0;
		const double MaxMicroseconds = Count > 0 ? Accumulator->MaxMicroseconds : 0.0;

		SummaryCsv += FString::Printf(
			TEXT("%s,%s,%s,%s,%lld,%lld,%.3f,%.6f,%.6f,%.3f,%.3f,%.3f\n"),
			ToSnapShotModeName(SessionSnapShotMode),
			Reason,
			ToMetricName(Metric),
			ToSnapShotModeName(SessionSnapShotMode),
			Count,
			SuccessCount,
			AverageMicroseconds,
			AverageMicroseconds / 1000.0,
			AverageTickRateFPS,
			MinMicroseconds,
			MaxMicroseconds,
			TotalMicroseconds);
	}

	const bool bRawSaved = FFileHelper::SaveStringToFile(RawCsv, *RawFilePath);
	const bool bSummarySaved = FFileHelper::SaveStringToFile(SummaryCsv, *SummaryFilePath);
	UE_LOG(LogLyraLagCompProfiler, Display,
		TEXT("Lag compensation profile CSV saved. Raw=%s (%s) Summary=%s (%s)"),
		*RawFilePath,
		bRawSaved ? TEXT("ok") : TEXT("failed"),
		*SummaryFilePath,
		bSummarySaved ? TEXT("ok") : TEXT("failed"));
}

ELyraSnapShotMode FLyraLagCompProfiler::DetectSnapShotMode(const UWorld* World) const
{
	for (TObjectIterator<ULyraSnapShotComponent> It; It; ++It)
	{
		const ULyraSnapShotComponent* Component = *It;
		if (!IsValid(Component) || Component->GetWorld() != World)
		{
			continue;
		}

		const ELyraSnapShotMode Mode = Component->GetSnapShotMode();
		if (Mode == ELyraSnapShotMode::AnimNode || Mode == ELyraSnapShotMode::SkeletalMesh)
		{
			return Mode;
		}
	}

	return ELyraSnapShotMode::None;
}
