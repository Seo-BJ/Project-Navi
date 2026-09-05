// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LyraLagCompensationProfile.generated.h"

UENUM()
enum class ELyraSSRProfileVariant : uint8
{
	SkeletalMesh,
	AnimNode
};

UENUM()
enum class ELyraSSRProfileMode : uint8
{
	None,
	SteadyTrace,
	Snapshot,
	RewindMicro,
	RewindLoad
};

UENUM()
enum class ELyraSSRRewindScenario : uint8
{
	None,
	InterpolatedMiss
};

UENUM()
enum class ELyraSSRAnimationMode : uint8
{
	/** Use the native engine scheduling path; profiling still requires observed work to be parallel. */
	Auto,
	/** Enable the native parallel animation CVars and require every observed mesh tick to be parallel. */
	Parallel,
	/** Retained only so old command lines fail validation with a useful name. */
	Synchronous
};

/** Immutable process-wide profiling selection parsed from the server command line. */
struct LYRAGAME_API FLyraSSRProfileConfiguration
{
	static ELyraSSRProfileVariant GetVariant();
	static ELyraSSRProfileMode GetMode();
	static ELyraSSRAnimationMode GetAnimationMode();
	static bool WasProfileArgumentProvided();
	static bool AreArgumentsValid();
	static const TCHAR* GetVariantName(ELyraSSRProfileVariant Variant);
	static const TCHAR* GetModeName(ELyraSSRProfileMode Mode);
	static const TCHAR* GetAnimationModeName(ELyraSSRAnimationMode AnimationMode);
};
LYRAGAME_API bool TryParseLyraSSRAnimationMode(const FString& Value, ELyraSSRAnimationMode& OutAnimationMode);
LYRAGAME_API bool IsLyraSSRAnimationExecutionValid(
	ELyraSSRAnimationMode RequestedMode,
	int32 ParallelTicks,
	int32 NonParallelTicks);
