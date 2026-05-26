// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraProfiledSkeletalMeshComponent.h"

#include "Character/LyraCharacter.h"
#include "GameFramework/Character.h"
#include "Stats/Stats.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraProfiledSkeletalMeshComponent)

DECLARE_STATS_GROUP(TEXT("Lyra Character Mesh"), STATGROUP_LyraCharacterMesh, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("LyraCharacterMeshTick"), STAT_LyraCharacterMeshTick, STATGROUP_LyraCharacterMesh);

void ULyraProfiledSkeletalMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	AActor* Owner = GetOwner();
	ACharacter* CharacterOwner = Cast<ACharacter>(Owner);

	const bool bIsLyraCharacterMesh =
		IsValid(Owner) &&
		Owner->IsA<ALyraCharacter>() &&
		CharacterOwner &&
		CharacterOwner->GetMesh() == this;

	if (bIsLyraCharacterMesh)
	{
		SCOPE_CYCLE_COUNTER(STAT_LyraCharacterMeshTick);
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
