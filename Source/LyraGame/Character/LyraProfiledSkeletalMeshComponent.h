// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SkeletalMeshComponent.h"

#include "LyraProfiledSkeletalMeshComponent.generated.h"

UCLASS()
class LYRAGAME_API ULyraProfiledSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
