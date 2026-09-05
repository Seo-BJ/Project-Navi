// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensation/LyraLagCompensationProfile.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
bool ParseVariantValue(FString& OutValue)
{
	return FParse::Value(FCommandLine::Get(), TEXT("LyraSSRVariant="), OutValue);
}

bool ParseProfileValue(FString& OutValue)
{
	return FParse::Value(FCommandLine::Get(), TEXT("LyraSSRProfile="), OutValue);
}

bool ParseAnimationModeValue(FString& OutValue)
{
	return FParse::Value(FCommandLine::Get(), TEXT("LyraSSRAnimationMode="), OutValue);
}

ELyraSSRProfileVariant ParseVariant()
{
#if UE_BUILD_SHIPPING
	return ELyraSSRProfileVariant::AnimNode;
#else
	FString Value;
	if (ParseVariantValue(Value))
	{
		if (Value.Equals(TEXT("SkeletalMesh"), ESearchCase::IgnoreCase))
		{
			return ELyraSSRProfileVariant::SkeletalMesh;
		}
		if (Value.Equals(TEXT("AnimNode"), ESearchCase::IgnoreCase))
		{
			return ELyraSSRProfileVariant::AnimNode;
		}
	}
	return ELyraSSRProfileVariant::AnimNode;
#endif
}

ELyraSSRProfileMode ParseMode()
{
#if UE_BUILD_SHIPPING
	return ELyraSSRProfileMode::None;
#else
	FString Value;
	if (!ParseProfileValue(Value))
	{
		return ELyraSSRProfileMode::None;
	}

	if (Value.Equals(TEXT("SteadyTrace"), ESearchCase::IgnoreCase)) return ELyraSSRProfileMode::SteadyTrace;
	if (Value.Equals(TEXT("Snapshot"), ESearchCase::IgnoreCase)) return ELyraSSRProfileMode::Snapshot;
	if (Value.Equals(TEXT("RewindMicro"), ESearchCase::IgnoreCase)) return ELyraSSRProfileMode::RewindMicro;
	if (Value.Equals(TEXT("RewindLoad"), ESearchCase::IgnoreCase)) return ELyraSSRProfileMode::RewindLoad;
	return ELyraSSRProfileMode::None;
#endif
}

ELyraSSRAnimationMode ParseAnimationMode()
{
#if UE_BUILD_SHIPPING
	return ELyraSSRAnimationMode::Auto;
#else
	FString Value;
	ELyraSSRAnimationMode Result = ELyraSSRAnimationMode::Auto;
	return ParseAnimationModeValue(Value) && TryParseLyraSSRAnimationMode(Value, Result)
		? Result
		: ELyraSSRAnimationMode::Auto;
#endif
}

bool IsKnownVariant(const FString& Value)
{
	return Value.Equals(TEXT("SkeletalMesh"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("AnimNode"), ESearchCase::IgnoreCase);
}

bool IsKnownProfile(const FString& Value)
{
	return Value.Equals(TEXT("SteadyTrace"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("Snapshot"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("RewindMicro"), ESearchCase::IgnoreCase)
		|| Value.Equals(TEXT("RewindLoad"), ESearchCase::IgnoreCase);
}
}

ELyraSSRProfileVariant FLyraSSRProfileConfiguration::GetVariant()
{
	static const ELyraSSRProfileVariant Variant = ParseVariant();
	return Variant;
}

ELyraSSRProfileMode FLyraSSRProfileConfiguration::GetMode()
{
	static const ELyraSSRProfileMode Mode = ParseMode();
	return Mode;
}

ELyraSSRAnimationMode FLyraSSRProfileConfiguration::GetAnimationMode()
{
	static const ELyraSSRAnimationMode AnimationMode = ParseAnimationMode();
	return AnimationMode;
}

bool FLyraSSRProfileConfiguration::WasProfileArgumentProvided()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	FString Value;
	return ParseProfileValue(Value);
#endif
}

bool FLyraSSRProfileConfiguration::AreArgumentsValid()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	FString Value;
	if (ParseVariantValue(Value) && !IsKnownVariant(Value))
	{
		return false;
	}
	if (ParseProfileValue(Value) && !IsKnownProfile(Value))
	{
		return false;
	}
	ELyraSSRAnimationMode ParsedAnimationMode = ELyraSSRAnimationMode::Auto;
	return !ParseAnimationModeValue(Value)
		|| (TryParseLyraSSRAnimationMode(Value, ParsedAnimationMode)
			&& ParsedAnimationMode != ELyraSSRAnimationMode::Synchronous);
#endif
}

const TCHAR* FLyraSSRProfileConfiguration::GetVariantName(ELyraSSRProfileVariant Variant)
{
	return Variant == ELyraSSRProfileVariant::SkeletalMesh ? TEXT("SkeletalMesh") : TEXT("AnimNode");
}

const TCHAR* FLyraSSRProfileConfiguration::GetModeName(ELyraSSRProfileMode Mode)
{
	switch (Mode)
	{
	case ELyraSSRProfileMode::SteadyTrace: return TEXT("SteadyTrace");
	case ELyraSSRProfileMode::Snapshot: return TEXT("Snapshot");
	case ELyraSSRProfileMode::RewindMicro: return TEXT("RewindMicro");
	case ELyraSSRProfileMode::RewindLoad: return TEXT("RewindLoad");
	default: return TEXT("None");
	}
}

const TCHAR* FLyraSSRProfileConfiguration::GetAnimationModeName(ELyraSSRAnimationMode AnimationMode)
{
	switch (AnimationMode)
	{
	case ELyraSSRAnimationMode::Parallel: return TEXT("Parallel");
	case ELyraSSRAnimationMode::Synchronous: return TEXT("Synchronous");
	default: return TEXT("Auto");
	}
}

bool TryParseLyraSSRAnimationMode(const FString& Value, ELyraSSRAnimationMode& OutAnimationMode)
{
	if (Value.Equals(TEXT("Auto"), ESearchCase::IgnoreCase))
	{
		OutAnimationMode = ELyraSSRAnimationMode::Auto;
		return true;
	}
	if (Value.Equals(TEXT("Parallel"), ESearchCase::IgnoreCase))
	{
		OutAnimationMode = ELyraSSRAnimationMode::Parallel;
		return true;
	}
	if (Value.Equals(TEXT("Synchronous"), ESearchCase::IgnoreCase))
	{
		OutAnimationMode = ELyraSSRAnimationMode::Synchronous;
		return true;
	}
	return false;
}

bool IsLyraSSRAnimationExecutionValid(
	ELyraSSRAnimationMode RequestedMode,
	int32 ParallelTicks,
	int32 NonParallelTicks)
{
	if (RequestedMode == ELyraSSRAnimationMode::Parallel)
	{
		return ParallelTicks > 0 && NonParallelTicks == 0;
	}
	if (RequestedMode == ELyraSSRAnimationMode::Synchronous)
	{
		return NonParallelTicks > 0 && ParallelTicks == 0;
	}
	return (ParallelTicks > 0) != (NonParallelTicks > 0);
}
