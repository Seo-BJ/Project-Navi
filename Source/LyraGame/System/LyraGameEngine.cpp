// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameEngine.h"

#include "LagCompensation/LyraLagCompensationProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameEngine)

class IEngineLoop;


ULyraGameEngine::ULyraGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float ULyraGameEngine::GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing) const
{
#if !UE_BUILD_SHIPPING
	static const bool bUncapDedicatedServerProfile =
		IsRunningDedicatedServer() && FLyraSSRProfileConfiguration::WasProfileArgumentProvided();
	if (bUncapDedicatedServerProfile)
	{
		// UGameEngine normally clamps dedicated servers to NetServerMaxTickRate.
		// Benchmark sessions intentionally bypass only that network cap. Fixed frame
		// rate or t.MaxFPS still surfaces as a validation failure in the profiler.
		return UEngine::GetMaxTickRate(DeltaTime, false);
	}
#endif
	return Super::GetMaxTickRate(DeltaTime, bAllowFrameRateSmoothing);
}

void ULyraGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

