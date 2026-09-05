// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "LagCompensation/LyraLagCompensationProfile.h"
#include "LagCompensation/ServerSideRewind/LyraServerSideRewindComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_AnimNode.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_SkeletalMesh.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLyraSSRActiveSnapshotSelectionTest,
	"Lyra.LagCompensation.Profile.ActiveSnapshotSelection",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FLyraSSRActiveSnapshotSelectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	ULyraSnapShotComponent_SkeletalMesh* Skeletal = NewObject<ULyraSnapShotComponent_SkeletalMesh>();
	ULyraSnapShotComponent_AnimNode* AnimNode = NewObject<ULyraSnapShotComponent_AnimNode>();
	TArray<ULyraSnapShotComponent*> Components{ Skeletal, AnimNode };

	Skeletal->SetSSRActive(true);
	AnimNode->SetSSRActive(false);
	TestEqual(TEXT("SkeletalMesh is selected when it is the only active component"),
		SelectUniqueActiveLyraSnapShotComponent(Components), static_cast<ULyraSnapShotComponent*>(Skeletal));

	Skeletal->SetSSRActive(false);
	AnimNode->SetSSRActive(true);
	TestEqual(TEXT("AnimNode is selected when it is the only active component"),
		SelectUniqueActiveLyraSnapShotComponent(Components), static_cast<ULyraSnapShotComponent*>(AnimNode));

	Skeletal->SetSSRActive(true);
	TestNull(TEXT("Ambiguous active components are rejected"), SelectUniqueActiveLyraSnapShotComponent(Components));
	AnimNode->SetSSRActive(false);
	Skeletal->SetSSRActive(false);
	TestNull(TEXT("Missing active component is rejected"), SelectUniqueActiveLyraSnapShotComponent(Components));
	return true;
}

#endif
