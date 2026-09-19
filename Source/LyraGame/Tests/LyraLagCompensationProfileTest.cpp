// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LagCompensation/LyraLagCompensationProfile.h"
#include "LagCompensation/ServerSideRewind/LyraServerSideRewindSubsystem.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_BoxComponent.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_AnimNode.h"
#include "LagCompensation/SnapShot/LyraSnapShotComponent_SkeletalMesh.h"
#include "Misc/ScopeExit.h"
#include "Physics/LyraCollisionChannels.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLyraBoxSnapshotGeometryRoundTripTest,
	"Lyra.LagCompensation.BoxSnapshot.GeometryRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FLyraBoxSnapshotGeometryRoundTripTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UBoxComponent* Box = NewObject<UBoxComponent>();
	const FTransform ExpectedTransform(
		FRotator(15.0, 35.0, 5.0),
		FVector(125.0, -80.0, 240.0),
		FVector(2.0, 0.5, 3.0));
	const FVector ExpectedUnscaledExtent(11.0, 23.0, 37.0);
	Box->SetWorldTransform(ExpectedTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Box->SetBoxExtent(ExpectedUnscaledExtent, false);

	for (int32 Iteration = 0; Iteration < 3; ++Iteration)
	{
		const FLyraBoxInformation Snapshot = ULyraSnapShotComponent_BoxComponent::CaptureBoxInformation(*Box);
		Box->SetWorldTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
		Box->SetBoxExtent(FVector(1.0), false);
		ULyraSnapShotComponent_BoxComponent::ApplyBoxInformation(*Box, Snapshot);

		TestTrue(TEXT("World transform round-trips with non-uniform scale"), Box->GetComponentTransform().Equals(ExpectedTransform));
		TestTrue(TEXT("Unscaled extent round-trips without scale accumulation"), Box->GetUnscaledBoxExtent().Equals(ExpectedUnscaledExtent));
		TestTrue(TEXT("Scaled extent remains stable across repeated round-trips"),
			Box->GetScaledBoxExtent().Equals(ExpectedUnscaledExtent * ExpectedTransform.GetScale3D().GetAbs()));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLyraBoxSnapshotExactCollisionRestorationTest,
	"Lyra.LagCompensation.BoxSnapshot.ExactCollisionRestoration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FLyraBoxSnapshotExactCollisionRestorationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UBoxComponent* HeadBox = NewObject<UBoxComponent>();
	UBoxComponent* BodyBox = NewObject<UBoxComponent>();
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>();
	UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>();

	const FTransform HeadTransform(FRotator(0.0, 25.0, 0.0), FVector(10.0, 20.0, 30.0), FVector(1.5, 0.75, 2.0));
	const FVector HeadExtent(8.0, 9.0, 10.0);
	HeadBox->SetWorldTransform(HeadTransform, false, nullptr, ETeleportType::TeleportPhysics);
	HeadBox->SetBoxExtent(HeadExtent, false);
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Overlap);
	BodyBox->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	BodyBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	TArray<UBoxComponent*> HitBoxes{ HeadBox, BodyBox };
	TArray<UPrimitiveComponent*> NormalCollisionComponents{ Mesh, Capsule };
	bool bRestoreSucceeded = false;
	{
		const ULyraServerSideRewindSubsystem::FBoxRewindState RewindState =
			ULyraServerSideRewindSubsystem::CaptureBoxRewindState(HitBoxes, NormalCollisionComponents);
		ULyraServerSideRewindSubsystem::SuspendBoxRewindCollision(RewindState);
		HeadBox->SetWorldTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
		HeadBox->SetBoxExtent(FVector(1.0), false);
		TArray<UBoxComponent*> HeadOnly{ HeadBox };
		ULyraServerSideRewindSubsystem::EnableOnlyBoxesForTrace(RewindState, HeadOnly);
		bRestoreSucceeded = ULyraServerSideRewindSubsystem::RestoreBoxRewindState(RewindState);
	}

	TestTrue(TEXT("Scoped state reports successful restoration"), bRestoreSucceeded);
	TestTrue(TEXT("Head transform is restored"), HeadBox->GetComponentTransform().Equals(HeadTransform));
	TestTrue(TEXT("Head extent is restored"), HeadBox->GetUnscaledBoxExtent().Equals(HeadExtent));
	TestTrue(TEXT("Head collision enabled is restored"), HeadBox->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
	TestTrue(TEXT("Head LagComp response is restored"),
		HeadBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Overlap);
	TestTrue(TEXT("Body collision enabled is restored"), BodyBox->GetCollisionEnabled() == ECollisionEnabled::PhysicsOnly);
	TestTrue(TEXT("Body LagComp response is restored"),
		BodyBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Ignore);
	TestTrue(TEXT("Mesh collision enabled is restored"), Mesh->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
	TestTrue(TEXT("Capsule collision enabled is restored"), Capsule->GetCollisionEnabled() == ECollisionEnabled::PhysicsOnly);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLyraBoxSnapshotTraceSelectionTest,
	"Lyra.LagCompensation.BoxSnapshot.TraceSelection",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FLyraBoxSnapshotTraceSelectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UBoxComponent* HeadBox = NewObject<UBoxComponent>();
	UBoxComponent* BodyBox = NewObject<UBoxComponent>();
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyBox->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	HeadBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Overlap);
	BodyBox->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Overlap);

	TArray<UBoxComponent*> HitBoxes{ HeadBox, BodyBox };
	TArray<UPrimitiveComponent*> NoNormalCollisionComponents;
	bool bRestoreSucceeded = false;
	{
		const ULyraServerSideRewindSubsystem::FBoxRewindState RewindState =
			ULyraServerSideRewindSubsystem::CaptureBoxRewindState(HitBoxes, NoNormalCollisionComponents);
		ULyraServerSideRewindSubsystem::SuspendBoxRewindCollision(RewindState);

		TArray<UBoxComponent*> HeadOnly{ HeadBox };
		ULyraServerSideRewindSubsystem::EnableOnlyBoxesForTrace(RewindState, HeadOnly);
		TestTrue(TEXT("Head is query-enabled for head trace"), HeadBox->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
		TestTrue(TEXT("Head blocks LagComp trace"),
			HeadBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Block);
		TestTrue(TEXT("Body remains disabled for head trace"), BodyBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
		TestTrue(TEXT("Body ignores LagComp head trace"),
			BodyBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Ignore);

		ULyraServerSideRewindSubsystem::EnableOnlyBoxesForTrace(RewindState, HitBoxes);
		TestTrue(TEXT("Head is enabled for full trace"), HeadBox->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
		TestTrue(TEXT("Body is enabled for full trace"), BodyBox->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
		TestTrue(TEXT("Body blocks LagComp full trace"),
			BodyBox->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Block);
		bRestoreSucceeded = ULyraServerSideRewindSubsystem::RestoreBoxRewindState(RewindState);
	}

	TestTrue(TEXT("Trace selection scope restores successfully"), bRestoreSucceeded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLyraBoxSnapshotEarlyExitRestorationTest,
	"Lyra.LagCompensation.BoxSnapshot.EarlyExitRestoration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FLyraBoxSnapshotEarlyExitRestorationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UBoxComponent* Box = NewObject<UBoxComponent>();
	const FTransform OriginalTransform(FRotator(5.0, 10.0, 15.0), FVector(40.0, 50.0, 60.0), FVector(0.5, 2.0, 1.25));
	const FVector OriginalExtent(7.0, 8.0, 9.0);
	Box->SetWorldTransform(OriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Box->SetBoxExtent(OriginalExtent, false);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Box->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Overlap);

	TArray<UBoxComponent*> HitBoxes{ Box };
	TArray<UPrimitiveComponent*> NoNormalCollisionComponents;
	bool bRestoreSucceeded = false;
	auto LeaveScopeEarly = [&]()
	{
		const ULyraServerSideRewindSubsystem::FBoxRewindState RewindState =
			ULyraServerSideRewindSubsystem::CaptureBoxRewindState(HitBoxes, NoNormalCollisionComponents);
		ON_SCOPE_EXIT
		{
			bRestoreSucceeded = ULyraServerSideRewindSubsystem::RestoreBoxRewindState(RewindState);
		};
		ULyraServerSideRewindSubsystem::SuspendBoxRewindCollision(RewindState);
		Box->SetWorldTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
		Box->SetBoxExtent(FVector(1.0), false);
		return;
	};
	LeaveScopeEarly();

	TestTrue(TEXT("Early exit restores successfully"), bRestoreSucceeded);
	TestTrue(TEXT("Early exit restores transform"), Box->GetComponentTransform().Equals(OriginalTransform));
	TestTrue(TEXT("Early exit restores extent"), Box->GetUnscaledBoxExtent().Equals(OriginalExtent));
	TestTrue(TEXT("Early exit restores collision enabled"), Box->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
	TestTrue(TEXT("Early exit restores LagComp response"),
		Box->GetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox) == ECollisionResponse::ECR_Overlap);
	return true;
}

#endif
