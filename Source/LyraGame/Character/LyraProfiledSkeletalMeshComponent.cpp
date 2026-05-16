// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraProfiledSkeletalMeshComponent.h"

#include "Character/LyraCharacter.h"
#include "GameFramework/Character.h"
#include "LagCompensation/LyraLagCompProfiler.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraProfiledSkeletalMeshComponent)

void ULyraProfiledSkeletalMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	AActor* Owner = GetOwner();
	ACharacter* CharacterOwner = Cast<ACharacter>(Owner);
	UWorld* World = GetWorld();

	const bool bIsLyraCharacterMesh =
		IsValid(Owner) &&
		Owner->IsA<ALyraCharacter>() &&
		CharacterOwner &&
		CharacterOwner->GetMesh() == this;

	const bool bShouldProfile = bIsLyraCharacterMesh && FLyraLagCompProfiler::Get().IsActiveForWorld(World);
	const FString OwnerName = bShouldProfile ? GetNameSafe(Owner) : FString();
	const FString ComponentName = bShouldProfile ? GetNameSafe(this) : FString();
	const double StartSeconds = bShouldProfile ? FPlatformTime::Seconds() : 0.0;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldProfile)
	{
		const double DurationMicroseconds = (FPlatformTime::Seconds() - StartSeconds) * 1000000.0;
		FLyraLagCompProfiler::Get().RecordDuration(
			ELyraLagCompProfileMetric::LyraCharacterMeshTick,
			World,
			ELyraSnapShotMode::None,
			DurationMicroseconds,
			true,
			OwnerName,
			ComponentName);
	}
}
