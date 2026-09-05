// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/LyraLagCompensationProfileSubsystem.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "LagCompensation/LyraLagCompensationSettings.h"
#include "LagCompensation/ServerSideRewind/LyraServerSideRewindComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_AnimNode.h"
#include "LagCompensation/Test/LyraLagCompensationTestCharacter.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "ProfilingDebugging/MiscTrace.h"

DEFINE_LOG_CATEGORY_STATIC(LogLyraLagCompensationProfile, Log, All);

// Unchecked counters intentionally emit repeated values. FrameMilliseconds needs
// one sample per server frame, and zero-valued validation counters must still be
// exported so a missing trace channel cannot be mistaken for a valid zero.
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_SessionActive, TEXT("LyraSSR/SessionActive"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_SessionValid, TEXT("LyraSSR/SessionValid"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_InvalidReasonMask, TEXT("LyraSSR/InvalidReasonMask"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_Variant, TEXT("LyraSSR/Variant"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_Profile, TEXT("LyraSSR/Profile"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_RewindScenario, TEXT("LyraSSR/RewindScenario"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_DedicatedServer, TEXT("LyraSSR/DedicatedServer"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_CharacterCount, TEXT("LyraSSR/CharacterCount"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_ConfiguredNetServerMaxTickRate, TEXT("LyraSSR/ConfiguredNetServerMaxTickRate"));
TRACE_DECLARE_UNCHECKED_FLOAT_COUNTER(LyraSSR_EffectiveEngineMaxTickRate, TEXT("LyraSSR/EffectiveEngineMaxTickRate"));
TRACE_DECLARE_UNCHECKED_FLOAT_COUNTER(LyraSSR_WarmupSeconds, TEXT("LyraSSR/WarmupSeconds"));
TRACE_DECLARE_UNCHECKED_FLOAT_COUNTER(LyraSSR_RequestedMeasurementSeconds, TEXT("LyraSSR/RequestedMeasurementSeconds"));
TRACE_DECLARE_UNCHECKED_FLOAT_COUNTER(LyraSSR_ActualMeasurementSeconds, TEXT("LyraSSR/ActualMeasurementSeconds"));
TRACE_DECLARE_UNCHECKED_FLOAT_COUNTER(LyraSSR_FrameMilliseconds, TEXT("LyraSSR/FrameMilliseconds"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_ServerFrames, TEXT("LyraSSR/ServerFrames"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_SnapshotCalls, TEXT("LyraSSR/SnapshotCalls"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_SnapshotFrames, TEXT("LyraSSR/SnapshotFrames"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_RewindRequests, TEXT("LyraSSR/RewindRequests"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_RewindHits, TEXT("LyraSSR/RewindHits"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_RewindMisses, TEXT("LyraSSR/RewindMisses"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_RewindInvalidResults, TEXT("LyraSSR/RewindInvalidResults"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_MissedInjections, TEXT("LyraSSR/MissedInjections"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_ParallelAnimationMeshTicks, TEXT("LyraSSR/ParallelAnimationMeshTicks"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_NonParallelAnimationMeshTicks, TEXT("LyraSSR/NonParallelAnimationMeshTicks"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_ParallelCapableAnimInstances, TEXT("LyraSSR/ParallelCapableAnimInstances"));
TRACE_DECLARE_UNCHECKED_INT_COUNTER(LyraSSR_GameThreadOnlyAnimInstances, TEXT("LyraSSR/GameThreadOnlyAnimInstances"));

namespace
{
constexpr double WarmupDurationSeconds = 5.0;
constexpr double MeasurementDurationSeconds = 60.0;
constexpr double RewindSeconds = 6.5 / 60.0;
constexpr double RewindLoadIntervalSeconds = 1.0 / 30.0;
constexpr int32 RewindMicroWarmupRequests = 50;
constexpr int32 RewindMicroMeasuredRequests = 1000;
constexpr TCHAR MeasurementRegionName[] = TEXT("LyraSSR_Measurement");

enum EInvalidReason : uint32
{
	Invalid_None = 0,
	Invalid_NotDedicatedServer = 1 << 0,
	Invalid_CharacterCount = 1 << 1,
	Invalid_ActiveSnapshotSelection = 1 << 2,
	Invalid_ParallelCVars = 1 << 3,
	Invalid_SnapshotCallsPerFrame = 1 << 4,
	Invalid_UnexpectedRewindHit = 1 << 5,
	Invalid_SnapshotRange = 1 << 6,
	Invalid_SingleFrameFallback = 1 << 7,
	Invalid_PoseReconstruction = 1 << 8,
	Invalid_StateRestore = 1 << 9,
	Invalid_TopologyMismatch = 1 << 10,
	Invalid_MissedInjection = 1 << 11,
	Invalid_AnimationStateMutation = 1 << 12,
	Invalid_Arguments = 1 << 13,
	Invalid_Aborted = 1 << 14,
	Invalid_MeshSettings = 1 << 15,
	Invalid_RewindRequestCount = 1 << 16,
	Invalid_DebugDraw = 1 << 17,
	Invalid_SnapshotReadiness = 1 << 18,
	Invalid_AnimationExecution = 1 << 19,
	Invalid_MaxTickRate = 1 << 20,
	Invalid_NoSnapshotSamples = 1 << 21,
	Invalid_NoServerFrames = 1 << 22
};

IConsoleVariable* FindCVar(const TCHAR* Name)
{
	return IConsoleManager::Get().FindConsoleVariable(Name);
}

int32 ReadCVar(const TCHAR* Name, int32 MissingValue = MIN_int32)
{
	const IConsoleVariable* Variable = FindCVar(Name);
	return Variable ? Variable->GetInt() : MissingValue;
}

bool SetCVar(const TCHAR* Name, int32 Value)
{
	IConsoleVariable* Variable = FindCVar(Name);
	if (!Variable)
	{
		return false;
	}
	Variable->Set(Value, ECVF_SetByCode);
	return Variable->GetInt() == Value;
}

bool SetFloatCVar(const TCHAR* Name, float Value)
{
	IConsoleVariable* Variable = FindCVar(Name);
	if (!Variable)
	{
		return false;
	}
	Variable->Set(Value, ECVF_SetByCode);
	return FMath::IsNearlyEqual(Variable->GetFloat(), Value);
}

const TCHAR* GetVisibilityOptionName(EVisibilityBasedAnimTickOption Option)
{
	switch (Option)
	{
	case EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones: return TEXT("AlwaysTickPoseAndRefreshBones");
	case EVisibilityBasedAnimTickOption::AlwaysTickPose: return TEXT("AlwaysTickPose");
	case EVisibilityBasedAnimTickOption::OnlyTickMontagesAndRefreshBonesWhenPlayingMontages: return TEXT("OnlyTickMontagesAndRefreshBonesWhenPlayingMontages");
	case EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered: return TEXT("OnlyTickMontagesWhenNotRendered");
	case EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered: return TEXT("OnlyTickPoseWhenRendered");
	default: return TEXT("Unknown");
	}
}
}

bool ULyraLagCompensationProfileSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UWorld* World = Cast<UWorld>(Outer);
	return Super::ShouldCreateSubsystem(Outer)
		&& IsValid(World)
		&& (World->GetNetMode() == NM_DedicatedServer || IsRunningDedicatedServer())
		&& FLyraSSRProfileConfiguration::WasProfileArgumentProvided();
#endif
}

void ULyraLagCompensationProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Variant = FLyraSSRProfileConfiguration::GetVariant();
	Mode = FLyraSSRProfileConfiguration::GetMode();
	// Every benchmark uses the engine's native parallel animation path. The old
	// profiling-only worker implementation has deliberately been removed.
	AnimationMode = ELyraSSRAnimationMode::Parallel;
	RewindScenario = (Mode == ELyraSSRProfileMode::RewindMicro || Mode == ELyraSSRProfileMode::RewindLoad)
		? ELyraSSRRewindScenario::InterpolatedMiss
		: ELyraSSRRewindScenario::None;

	Targets.Reserve(ExpectedCharacterCount);
	SessionDiscoveryStartSeconds = FPlatformTime::Seconds();
}

void ULyraLagCompensationProfileSubsystem::Deinitialize()
{
	if (State != ESessionState::Finished && SessionStartFrame != 0)
	{
		Invalidate(Invalid_Aborted);
		FinishSession(FPlatformTime::Seconds());
	}
	else if (bTraceRegionOpen)
	{
		TRACE_END_REGION(MeasurementRegionName);
		bTraceRegionOpen = false;
	}
	RestoreAnimationCVars();
	Super::Deinitialize();
}

TStatId ULyraLagCompensationProfileSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULyraLagCompensationProfileSubsystem, STATGROUP_Tickables);
}

void ULyraLagCompensationProfileSubsystem::Tick(float DeltaTime)
{
	(void)DeltaTime;
	if (State == ESessionState::Finished)
	{
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	if (State == ESessionState::WaitingForWorld)
	{
		TryStartSession(NowSeconds);
		return;
	}

	if (State == ESessionState::Warmup)
	{
		if (Mode == ELyraSSRProfileMode::RewindLoad && NowSeconds - WarmupStartSeconds >= 1.0)
		{
			TickRewindLoad(NowSeconds);
		}
		if (NowSeconds - WarmupStartSeconds >= WarmupDurationSeconds)
		{
			if (Mode == ELyraSSRProfileMode::RewindMicro)
			{
				State = ESessionState::MicroWarmup;
			}
			else
			{
				BeginMeasurement(NowSeconds);
			}
		}
		return;
	}

	if (State == ESessionState::MicroWarmup)
	{
		if (ExecuteInterpolatedMiss())
		{
			++MicroWarmupCount;
		}
		if (MicroWarmupCount >= RewindMicroWarmupRequests)
		{
			BeginMeasurement(NowSeconds);
		}
		else if (RewindRequestCount >= 300)
		{
			FinishSession(NowSeconds);
		}
		return;
	}

	if (State == ESessionState::Measuring)
	{
		RecordServerFrame(NowSeconds);
		if (Mode == ELyraSSRProfileMode::RewindMicro)
		{
			TickRewindMicro();
			return;
		}
		if (NowSeconds - MeasurementStartSeconds >= MeasurementDurationSeconds)
		{
			FinishSession(NowSeconds);
			return;
		}
		if (Mode == ELyraSSRProfileMode::RewindLoad)
		{
			TickRewindLoad(NowSeconds);
		}
	}
}

void ULyraLagCompensationProfileSubsystem::TryStartSession(double NowSeconds)
{
	if (!FLyraSSRProfileConfiguration::AreArgumentsValid())
	{
		Invalidate(Invalid_Arguments);
		FinishSession(NowSeconds);
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}
	if (World->GetNetMode() != NM_DedicatedServer)
	{
		if (NowSeconds - SessionDiscoveryStartSeconds >= 30.0)
		{
			Invalidate(Invalid_NotDedicatedServer);
			FinishSession(NowSeconds);
		}
		return;
	}

	Targets.Reset(ExpectedCharacterCount);
	for (TActorIterator<ALyraLagCompensationTestCharacter> It(World); It; ++It)
	{
		Targets.Add(*It);
	}
	if (Targets.Num() != ExpectedCharacterCount)
	{
		if (NowSeconds - SessionDiscoveryStartSeconds >= 30.0)
		{
			Invalidate(Invalid_CharacterCount);
			FinishSession(NowSeconds);
		}
		return;
	}

	for (const TWeakObjectPtr<ALyraLagCompensationTestCharacter>& TargetPtr : Targets)
	{
		ALyraLagCompensationTestCharacter* Target = TargetPtr.Get();
		ULyraSnapShotComponent* ActiveSnapshot = Target ? Target->GetActiveSnapShotComponent() : nullptr;
		ULyraServerSideRewindComponent* Rewind = Target ? Target->GetServerSideRewindComponent() : nullptr;
		if (!ActiveSnapshot || !Rewind || Rewind->ResolveSnapShotComponent(Target) != ActiveSnapshot)
		{
			Invalidate(Invalid_ActiveSnapshotSelection);
		}

		const USkeletalMeshComponent* Mesh = Target ? Target->GetMesh() : nullptr;
		const EVisibilityBasedAnimTickOption ExpectedVisibility = Variant == ELyraSSRProfileVariant::SkeletalMesh
			? EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones
			: EVisibilityBasedAnimTickOption::AlwaysTickPose;
		if (!Mesh || Mesh->VisibilityBasedAnimTickOption != ExpectedVisibility || Mesh->bEnableUpdateRateOptimizations)
		{
			Invalidate(Invalid_MeshSettings);
		}
		if (const ULyraSnapShotComponent_AnimNode* AnimNode = Cast<ULyraSnapShotComponent_AnimNode>(ActiveSnapshot);
			AnimNode && AnimNode->IsProfileDebugDrawEnabled())
		{
			Invalidate(Invalid_DebugDraw);
		}
	}

	const ULyraLagCompensationDeveloperSettings* DeveloperSettings = GetDefault<ULyraLagCompensationDeveloperSettings>();
	if (DeveloperSettings && DeveloperSettings->bDrawMeshLagCompensation)
	{
		Invalidate(Invalid_DebugDraw);
	}
	if (!ApplyAnimationModeCVars())
	{
		Invalidate(Invalid_ParallelCVars);
	}
	if (GEngine && GEngine->GetMaxTickRate(0.0f, false) > 0.0f)
	{
		Invalidate(Invalid_MaxTickRate);
	}

	SessionStartFrame = GFrameCounter;
	WarmupStartSeconds = NowSeconds;
	NextLoadRequestSeconds = NowSeconds;
	State = ESessionState::Warmup;
	UE_LOG(LogLyraLagCompensationProfile, Display,
		TEXT("Lag compensation Insights profile armed. Variant=%s Profile=%s Animation=NativeParallel Warmup=5s"),
		FLyraSSRProfileConfiguration::GetVariantName(Variant),
		FLyraSSRProfileConfiguration::GetModeName(Mode));
}

void ULyraLagCompensationProfileSubsystem::BeginMeasurement(double NowSeconds)
{
	if (!InspectAnimationExecutionConfiguration())
	{
		Invalidate(Invalid_AnimationExecution);
	}
	for (const TWeakObjectPtr<ALyraLagCompensationTestCharacter>& TargetPtr : Targets)
	{
		const ALyraLagCompensationTestCharacter* Target = TargetPtr.Get();
		const ULyraSnapShotComponent* ActiveSnapshot = Target ? Target->GetActiveSnapShotComponent() : nullptr;
		if (!ActiveSnapshot || !ActiveSnapshot->HasAnySnapShot())
		{
			Invalidate(Invalid_SnapshotReadiness);
		}
	}

	MeasurementStartSeconds = NowSeconds;
	LastServerFrameSeconds = NowSeconds;
	MeasurementStartFrame = GFrameCounter;
	NextLoadRequestSeconds = NowSeconds;
	SnapshotCallCount = 0;
	SnapshotMeasuredFrameCount = 0;
	SnapshotCallsInCurrentFrame = 0;
	ServerFrameCount = 0;
	RewindRequestCount = 0;
	RewindHitCount = 0;
	RewindMissCount = 0;
	RewindInvalidCount = 0;
	MissedInjectionCount = 0;
	ParallelAnimationTickCount = 0;
	NonParallelAnimationTickCount = 0;
	CurrentSnapshotFrame = MAX_uint64;
	TargetIndex = 0;
	State = ESessionState::Measuring;

	TRACE_BEGIN_REGION(MeasurementRegionName);
	bTraceRegionOpen = true;
	PublishTraceCounters();
	TRACE_BOOKMARK(TEXT("LyraSSRProfileStart Variant=%s Profile=%s AnimationMode=NativeParallel Region=%s"),
		FLyraSSRProfileConfiguration::GetVariantName(Variant),
		FLyraSSRProfileConfiguration::GetModeName(Mode),
		MeasurementRegionName);
}

bool ULyraLagCompensationProfileSubsystem::IsMeasurementActive() const
{
	return State == ESessionState::Measuring;
}

bool ULyraLagCompensationProfileSubsystem::ShouldObserveAnimationExecution() const
{
	return IsMeasurementActive();
}

void ULyraLagCompensationProfileSubsystem::RecordSnapshotExecution(uint64 FrameNumber)
{
	if (!IsMeasurementActive())
	{
		return;
	}

	if (CurrentSnapshotFrame != FrameNumber)
	{
		if (CurrentSnapshotFrame != MAX_uint64)
		{
			ValidateCompletedSnapshotFrame(CurrentSnapshotFrame, SnapshotCallsInCurrentFrame);
			if (FrameNumber != CurrentSnapshotFrame + 1)
			{
				Invalidate(Invalid_SnapshotCallsPerFrame);
			}
		}
		CurrentSnapshotFrame = FrameNumber;
		SnapshotCallsInCurrentFrame = 0;
		++SnapshotMeasuredFrameCount;
	}

	++SnapshotCallsInCurrentFrame;
	++SnapshotCallCount;
}

void ULyraLagCompensationProfileSubsystem::RecordAnimationExecution(bool bDispatchedParallelWork)
{
	if (!IsMeasurementActive())
	{
		return;
	}

	if (bDispatchedParallelWork)
	{
		++ParallelAnimationTickCount;
	}
	else
	{
		++NonParallelAnimationTickCount;
	}
}

void ULyraLagCompensationProfileSubsystem::TickRewindMicro()
{
	ExecuteInterpolatedMiss();
	if (RewindRequestCount >= RewindMicroMeasuredRequests)
	{
		FinishSession(FPlatformTime::Seconds());
	}
}

void ULyraLagCompensationProfileSubsystem::TickRewindLoad(double NowSeconds)
{
	if (NowSeconds < NextLoadRequestSeconds)
	{
		return;
	}

	ExecuteInterpolatedMiss();
	NextLoadRequestSeconds += RewindLoadIntervalSeconds;
	while (NowSeconds >= NextLoadRequestSeconds)
	{
		++MissedInjectionCount;
		NextLoadRequestSeconds += RewindLoadIntervalSeconds;
	}
}

bool ULyraLagCompensationProfileSubsystem::ExecuteInterpolatedMiss()
{
	if (Targets.Num() != ExpectedCharacterCount)
	{
		Invalidate(Invalid_CharacterCount);
		return false;
	}

	ALyraLagCompensationTestCharacter* Target = Targets[TargetIndex].Get();
	TargetIndex = (TargetIndex + 1) % Targets.Num();
	ALyraLagCompensationTestCharacter* RequestOwner = Targets[0].Get();
	ULyraServerSideRewindComponent* Rewind = RequestOwner ? RequestOwner->GetServerSideRewindComponent() : nullptr;
	ULyraSnapShotComponent* Snapshot = Target ? Target->GetActiveSnapShotComponent() : nullptr;
	UWorld* World = GetWorld();
	if (!Target || !Rewind || !Snapshot || !World)
	{
		Invalidate(Invalid_ActiveSnapshotSelection);
		return false;
	}

	const int32 AbilityActivationsBefore = Target->GetPeriodicAbilityActivationCount();
	const int32 MontageCompletedBefore = Target->GetPeriodicMontageCompletedCount();
	const int32 MontageInterruptedBefore = Target->GetPeriodicMontageInterruptedCount();
	UAnimInstance* AnimInstance = Target->GetMesh() ? Target->GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* ActiveMontageBefore = AnimInstance ? AnimInstance->GetCurrentActiveMontage() : nullptr;
	const float MontagePositionBefore = ActiveMontageBefore ? AnimInstance->Montage_GetPosition(ActiveMontageBefore) : 0.0f;

	const FVector TraceStart = Target->GetActorLocation() + FVector(0.0, 0.0, 10000.0);
	const FVector HitLocation = TraceStart + FVector(1000.0, 0.0, 0.0);
	FLyraServerSideRewindRequest Request;
	Request.TargetActor = Target;
	Request.TraceStart = TraceStart;
	Request.HitLocation = HitLocation;
	Request.HitTime = static_cast<float>(World->GetTimeSeconds() - RewindSeconds);

	Snapshot->ResetRewindDiagnostics();
	const FServerSideRewindResult Result = Rewind->ServerSideRewind(Request);
	++RewindRequestCount;
	if (Result.bHitConfirmed)
	{
		++RewindHitCount;
		Invalidate(Invalid_UnexpectedRewindHit);
	}
	else
	{
		++RewindMissCount;
	}

	const FLyraSSRRewindDiagnostics& Diagnostics = Snapshot->GetLastRewindDiagnostics();
	bool bValidResult = true;
	if (!Diagnostics.bSnapshotLookupSucceeded) { Invalidate(Invalid_SnapshotRange); bValidResult = false; }
	if (!Diagnostics.bUsedInterpolation) { Invalidate(Invalid_SingleFrameFallback); bValidResult = false; }
	if (!Diagnostics.bPoseReconstructionSucceeded) { Invalidate(Invalid_PoseReconstruction); bValidResult = false; }
	if (!Diagnostics.bStateRestoreSucceeded) { Invalidate(Invalid_StateRestore); bValidResult = false; }
	if (!Diagnostics.bTopologyMatched) { Invalidate(Invalid_TopologyMismatch); bValidResult = false; }
	if (!bValidResult)
	{
		++RewindInvalidCount;
	}

	UAnimMontage* ActiveMontageAfter = AnimInstance ? AnimInstance->GetCurrentActiveMontage() : nullptr;
	const float MontagePositionAfter = ActiveMontageAfter ? AnimInstance->Montage_GetPosition(ActiveMontageAfter) : 0.0f;
	if (AbilityActivationsBefore != Target->GetPeriodicAbilityActivationCount()
		|| MontageCompletedBefore != Target->GetPeriodicMontageCompletedCount()
		|| MontageInterruptedBefore != Target->GetPeriodicMontageInterruptedCount()
		|| ActiveMontageBefore != ActiveMontageAfter
		|| !FMath::IsNearlyEqual(MontagePositionBefore, MontagePositionAfter, KINDA_SMALL_NUMBER))
	{
		Invalidate(Invalid_AnimationStateMutation);
		bValidResult = false;
	}

	return bValidResult;
}

void ULyraLagCompensationProfileSubsystem::RecordServerFrame(double NowSeconds)
{
	const double FrameMilliseconds = FMath::Max(0.0, NowSeconds - LastServerFrameSeconds) * 1000.0;
	LastServerFrameSeconds = NowSeconds;
	++ServerFrameCount;
	TRACE_COUNTER_SET(LyraSSR_FrameMilliseconds, FrameMilliseconds);
}

void ULyraLagCompensationProfileSubsystem::ValidateCompletedSnapshotFrame(uint64 FrameNumber, int32 CallsInFrame)
{
	// BeginMeasurement may occur after some component ticks in its first frame.
	if (FrameNumber != MeasurementStartFrame && CallsInFrame != ExpectedCharacterCount)
	{
		Invalidate(Invalid_SnapshotCallsPerFrame);
	}
}

void ULyraLagCompensationProfileSubsystem::FinishSession(double NowSeconds)
{
	if (State == ESessionState::Finished)
	{
		return;
	}

	MeasurementEndFrame = GFrameCounter;
	if (CurrentSnapshotFrame != MAX_uint64)
	{
		ValidateCompletedSnapshotFrame(CurrentSnapshotFrame, SnapshotCallsInCurrentFrame);
	}
	if (MeasurementStartSeconds > 0.0 && SnapshotCallCount == 0)
	{
		Invalidate(Invalid_NoSnapshotSamples);
	}
	if (MeasurementStartSeconds > 0.0 && ServerFrameCount == 0)
	{
		Invalidate(Invalid_NoServerFrames);
	}
	if (Mode == ELyraSSRProfileMode::RewindLoad && MissedInjectionCount > 0)
	{
		Invalidate(Invalid_MissedInjection);
	}
	if (Mode == ELyraSSRProfileMode::RewindLoad && RewindRequestCount != 1800)
	{
		Invalidate(Invalid_RewindRequestCount);
	}
	if (Mode == ELyraSSRProfileMode::RewindMicro && MeasurementStartSeconds > 0.0
		&& RewindRequestCount != RewindMicroMeasuredRequests)
	{
		Invalidate(Invalid_RewindRequestCount);
	}
	if (MeasurementStartSeconds > 0.0 && !IsLyraSSRAnimationExecutionValid(
		ELyraSSRAnimationMode::Parallel, ParallelAnimationTickCount, NonParallelAnimationTickCount))
	{
		Invalidate(Invalid_AnimationExecution);
	}

	const double ActualMeasurementSeconds = MeasurementStartSeconds > 0.0
		? FMath::Max(0.0, NowSeconds - MeasurementStartSeconds)
		: 0.0;
	TRACE_COUNTER_SET(LyraSSR_ActualMeasurementSeconds, ActualMeasurementSeconds);
	PublishTraceCounters();
	TRACE_COUNTER_SET(LyraSSR_SessionActive, 0);
	TRACE_BOOKMARK(TEXT("LyraSSRProfileEnd Variant=%s Profile=%s Valid=%s InvalidReasonMask=%u"),
		FLyraSSRProfileConfiguration::GetVariantName(Variant),
		FLyraSSRProfileConfiguration::GetModeName(Mode),
		InvalidReasonMask == 0 ? TEXT("true") : TEXT("false"),
		InvalidReasonMask);
	if (bTraceRegionOpen)
	{
		TRACE_END_REGION(MeasurementRegionName);
		bTraceRegionOpen = false;
	}

	State = ESessionState::Finished;
	RestoreAnimationCVars();
	UE_LOG(LogLyraLagCompensationProfile, Display,
		TEXT("Lag compensation Insights profile finished. Valid=%s InvalidReasonMask=%u Frames=%d SnapshotCalls=%d RewindRequests=%d Hits=%d Misses=%d ParallelTicks=%d NonParallelTicks=%d Duration=%.3fs. Stop PIE and analyze the server .utrace."),
		InvalidReasonMask == 0 ? TEXT("true") : TEXT("false"), InvalidReasonMask,
		ServerFrameCount, SnapshotCallCount, RewindRequestCount, RewindHitCount, RewindMissCount,
		ParallelAnimationTickCount, NonParallelAnimationTickCount, ActualMeasurementSeconds);
}

void ULyraLagCompensationProfileSubsystem::PublishTraceCounters() const
{
	const UWorld* World = GetWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	const float EffectiveMaxTickRate = GEngine ? GEngine->GetMaxTickRate(0.0f, false) : 0.0f;

	TRACE_COUNTER_SET(LyraSSR_SessionActive, State == ESessionState::Measuring ? 1 : 0);
	TRACE_COUNTER_SET(LyraSSR_SessionValid, InvalidReasonMask == 0 ? 1 : 0);
	TRACE_COUNTER_SET(LyraSSR_InvalidReasonMask, InvalidReasonMask);
	TRACE_COUNTER_SET(LyraSSR_Variant, static_cast<int64>(Variant));
	TRACE_COUNTER_SET(LyraSSR_Profile, static_cast<int64>(Mode));
	TRACE_COUNTER_SET(LyraSSR_RewindScenario, static_cast<int64>(RewindScenario));
	TRACE_COUNTER_SET(LyraSSR_DedicatedServer, World && World->GetNetMode() == NM_DedicatedServer ? 1 : 0);
	TRACE_COUNTER_SET(LyraSSR_CharacterCount, Targets.Num());
	TRACE_COUNTER_SET(LyraSSR_ConfiguredNetServerMaxTickRate, NetDriver ? NetDriver->GetNetServerMaxTickRate() : 0);
	TRACE_COUNTER_SET(LyraSSR_EffectiveEngineMaxTickRate, EffectiveMaxTickRate);
	TRACE_COUNTER_SET(LyraSSR_WarmupSeconds, WarmupDurationSeconds);
	TRACE_COUNTER_SET(LyraSSR_RequestedMeasurementSeconds,
		Mode == ELyraSSRProfileMode::RewindMicro ? 0.0 : MeasurementDurationSeconds);
	TRACE_COUNTER_SET(LyraSSR_ServerFrames, ServerFrameCount);
	TRACE_COUNTER_SET(LyraSSR_SnapshotCalls, SnapshotCallCount);
	TRACE_COUNTER_SET(LyraSSR_SnapshotFrames, SnapshotMeasuredFrameCount);
	TRACE_COUNTER_SET(LyraSSR_RewindRequests, RewindRequestCount);
	TRACE_COUNTER_SET(LyraSSR_RewindHits, RewindHitCount);
	TRACE_COUNTER_SET(LyraSSR_RewindMisses, RewindMissCount);
	TRACE_COUNTER_SET(LyraSSR_RewindInvalidResults, RewindInvalidCount);
	TRACE_COUNTER_SET(LyraSSR_MissedInjections, MissedInjectionCount);
	TRACE_COUNTER_SET(LyraSSR_ParallelAnimationMeshTicks, ParallelAnimationTickCount);
	TRACE_COUNTER_SET(LyraSSR_NonParallelAnimationMeshTicks, NonParallelAnimationTickCount);
	TRACE_COUNTER_SET(LyraSSR_ParallelCapableAnimInstances, ParallelCapableAnimInstanceCount);
	TRACE_COUNTER_SET(LyraSSR_GameThreadOnlyAnimInstances, GameThreadOnlyAnimInstanceCount);
}

bool ULyraLagCompensationProfileSubsystem::ValidateAnimationCVars() const
{
	return ReadCVar(TEXT("a.ParallelAnimEvaluation")) == 1
		&& ReadCVar(TEXT("a.ParallelAnimUpdate")) == 1
		&& ReadCVar(TEXT("a.ParallelAnimInterpolation")) == 1
		&& ReadCVar(TEXT("a.ForceParallelAnimUpdate")) == 0;
}

bool ULyraLagCompensationProfileSubsystem::InspectAnimationExecutionConfiguration()
{
	ParallelCapableAnimInstanceCount = 0;
	GameThreadOnlyAnimInstanceCount = 0;
	const auto RecordAnimInstanceCapability = [this](const UAnimInstance* AnimInstance, bool bRequiresMultiThreadedUpdateFlag)
	{
		const bool bParallelCapable = IsValid(AnimInstance)
			&& AnimInstance->CanRunParallelWork()
			&& (!bRequiresMultiThreadedUpdateFlag || AnimInstance->bUseMultiThreadedAnimationUpdate);
		if (bParallelCapable)
		{
			++ParallelCapableAnimInstanceCount;
		}
		else
		{
			++GameThreadOnlyAnimInstanceCount;
		}
	};

	for (const TWeakObjectPtr<ALyraLagCompensationTestCharacter>& TargetPtr : Targets)
	{
		const ALyraLagCompensationTestCharacter* Target = TargetPtr.Get();
		const USkeletalMeshComponent* Mesh = Target ? Target->GetMesh() : nullptr;
		if (!Mesh)
		{
			++GameThreadOnlyAnimInstanceCount;
			continue;
		}
		if (const UAnimInstance* MainInstance = Mesh->GetAnimInstance())
		{
			RecordAnimInstanceCapability(MainInstance, true);
		}
		else
		{
			++GameThreadOnlyAnimInstanceCount;
		}
		for (const UAnimInstance* LinkedInstance : Mesh->GetLinkedAnimInstances())
		{
			RecordAnimInstanceCapability(LinkedInstance, false);
		}
		if (const UAnimInstance* PostProcessInstance = Mesh->GetPostProcessInstance())
		{
			RecordAnimInstanceCapability(PostProcessInstance, true);
		}
	}

	return GetDefault<UEngine>()->bAllowMultiThreadedAnimationUpdate
		&& ParallelCapableAnimInstanceCount > 0
		&& GameThreadOnlyAnimInstanceCount == 0;
}

bool ULyraLagCompensationProfileSubsystem::ApplyAnimationModeCVars()
{
	SavedParallelEvaluation = ReadCVar(TEXT("a.ParallelAnimEvaluation"));
	SavedParallelUpdate = ReadCVar(TEXT("a.ParallelAnimUpdate"));
	SavedParallelInterpolation = ReadCVar(TEXT("a.ParallelAnimInterpolation"));
	SavedForceParallelUpdate = ReadCVar(TEXT("a.ForceParallelAnimUpdate"));
	IConsoleVariable* MaxFPSVariable = FindCVar(TEXT("t.MaxFPS"));
	SavedMaxFPS = MaxFPSVariable ? MaxFPSVariable->GetFloat() : 0.0f;
	bSavedAnimationCVars = SavedParallelEvaluation != MIN_int32
		&& SavedParallelUpdate != MIN_int32
		&& SavedParallelInterpolation != MIN_int32
		&& SavedForceParallelUpdate != MIN_int32
		&& MaxFPSVariable != nullptr;
	return bSavedAnimationCVars
		&& SetCVar(TEXT("a.ParallelAnimEvaluation"), 1)
		&& SetCVar(TEXT("a.ParallelAnimUpdate"), 1)
		&& SetCVar(TEXT("a.ParallelAnimInterpolation"), 1)
		&& SetCVar(TEXT("a.ForceParallelAnimUpdate"), 0)
		&& SetFloatCVar(TEXT("t.MaxFPS"), 0.0f)
		&& ValidateAnimationCVars();
}

void ULyraLagCompensationProfileSubsystem::RestoreAnimationCVars()
{
	if (!bSavedAnimationCVars)
	{
		return;
	}
	SetCVar(TEXT("a.ParallelAnimEvaluation"), SavedParallelEvaluation);
	SetCVar(TEXT("a.ParallelAnimUpdate"), SavedParallelUpdate);
	SetCVar(TEXT("a.ParallelAnimInterpolation"), SavedParallelInterpolation);
	SetCVar(TEXT("a.ForceParallelAnimUpdate"), SavedForceParallelUpdate);
	SetFloatCVar(TEXT("t.MaxFPS"), SavedMaxFPS);
	bSavedAnimationCVars = false;
}

void ULyraLagCompensationProfileSubsystem::Invalidate(uint32 ReasonBit)
{
	InvalidReasonMask |= ReasonBit;
	if (bTraceRegionOpen)
	{
		TRACE_COUNTER_SET(LyraSSR_SessionValid, 0);
		TRACE_COUNTER_SET(LyraSSR_InvalidReasonMask, InvalidReasonMask);
	}
}
