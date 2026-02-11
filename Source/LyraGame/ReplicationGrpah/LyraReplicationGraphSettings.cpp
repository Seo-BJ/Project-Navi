// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraReplicationGraphSettings.h"
#include "LyraReplicationGraph.h"
#include "Misc/App.h"


ULyraReplicationGraphSettings::ULyraReplicationGraphSettings()
{
	CategoryName = TEXT("Game");
	DefaultReplicationGraphClass = ULyraReplicationGraph::StaticClass();
	VisibleConnectionTraceChannel = ECC_GameTraceChannel1;
}