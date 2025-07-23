// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "NaviDamageExecution.generated.h"

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UNaviDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
