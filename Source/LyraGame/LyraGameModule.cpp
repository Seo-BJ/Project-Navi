// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "LagCompensation/LyraLagCompProfiler.h"


/**
 * FLyraGameModule
 */
class FLyraGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		FLyraLagCompProfiler::Get().Initialize();
	}

	virtual void ShutdownModule() override
	{
		FLyraLagCompProfiler::Get().Shutdown();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");
