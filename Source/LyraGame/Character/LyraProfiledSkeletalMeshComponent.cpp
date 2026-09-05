// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraProfiledSkeletalMeshComponent.h"

#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "LagCompensation/LyraLagCompensationProfileSubsystem.h"
#include "LagCompensation/Test/LyraLagCompensationTestCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraProfiledSkeletalMeshComponent)

void ULyraProfiledSkeletalMeshComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	AActor* Owner = GetOwner();
	ACharacter* CharacterOwner = Cast<ACharacter>(Owner);
	const bool bIsProfileTarget =
		IsValid(Owner)
		&& IsInGameThread()
		&& Owner->HasAuthority()
		&& Owner->IsA<ALyraLagCompensationTestCharacter>()
		&& CharacterOwner
		&& CharacterOwner->GetMesh() == this
		&& !bEnableUpdateRateOptimizations;

	ULyraLagCompensationProfileSubsystem* Profiler = bIsProfileTarget && GetWorld()
		? GetWorld()->GetSubsystem<ULyraLagCompensationProfileSubsystem>()
		: nullptr;
	if (Profiler && Profiler->IsMeasurementActive())
	{
		// This is game-thread wall time for the native mesh tick. Worker evaluation is
		// recorded separately by the engine's FParallelAnimationEvaluationTask scope.
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_MeshTickGT);
			Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		}
		Profiler->RecordAnimationExecution(IsRunningParallelEvaluation());
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
