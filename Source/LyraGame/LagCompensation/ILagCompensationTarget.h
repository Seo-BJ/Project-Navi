// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILagCompensationTarget.generated.h"

class UBoxComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class ULagCompensationTarget : public UInterface
{
	GENERATED_BODY()
};


class LYRAGAME_API ILagCompensationTarget
{
	GENERATED_BODY()

public:
	// Lag Compensation에서 사용 할 Hit Box들의 맵을 반환.
	virtual const TMap<FName, TObjectPtr<UBoxComponent>>& GetHitCollisionBoxes() const = 0;
};