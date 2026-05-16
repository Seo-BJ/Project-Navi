// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor", "DedicatedServers" });
		
		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}
		
        LyraGameTarget.ApplySharedLyraTargetSettings(this);
	}
}
