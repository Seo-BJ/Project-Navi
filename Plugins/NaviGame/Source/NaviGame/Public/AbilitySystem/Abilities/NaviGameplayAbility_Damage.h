// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "NaviGameplayAbility_Damage.generated.h"

/**
 * 
 */
UCLASS()
class NAVIGAME_API UNaviGameplayAbility_Damage : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UNaviGameplayAbility_Damage();

	// 블루프린트에서 설정 가능한 변수들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageMultiplier; // 데미지 배율

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageCalculationEffectClass; // 데미지 계산 GE

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageExecutionEffectClass; // 데미지 적용 GE

protected:
	// 데미지 파이프라인 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyDamageContainerOnTarget(const FGameplayAbilityTargetDataHandle& TargetData);
	
	
};
