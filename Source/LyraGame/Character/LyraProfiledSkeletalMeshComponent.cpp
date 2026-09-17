// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraProfiledSkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraProfiledSkeletalMeshComponent)

void ULyraProfiledSkeletalMeshComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	// Unreal Insights decides whether this event is recorded. Gameplay code does not
	// query or notify the profiling controller.
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraProfile_MeshTickGT);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
