// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "NaviArmorStatDefinition.generated.h"

/**
 * 방어구 Stat DataTable Row Struct
 */
USTRUCT(BlueprintType)
struct FNaviArmorStatDefinition : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ArmorTag; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CreditCost = 0; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageAbsorptionRate = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AbsorbAmount = 0; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RegenerationPool = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAffectedByDecay = false;
	
};
