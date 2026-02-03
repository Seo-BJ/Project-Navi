// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraActorWithAbilities.h"
#include "Game/NaviShootingTestTypes.h"
#include "NaviShootingTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNaviTargetEliminated, ANaviShootingTarget*, Target, bool, bWasKilled);

class UCapsuleComponent;
class UGameplayEffect;
struct FGameplayEffectSpec;

/**
 * ANaviShootingTarget
 * 
 * 사격 훈련용 봇(타겟)입니다.
 * 설정된 시간 동안 살아있으며, 피격 시 사망하거나 시간이 다 되면 사라집니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviShootingTarget : public ALyraActorWithAbilities
{
	GENERATED_BODY()
	
public:	
	ANaviShootingTarget(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 타겟 초기화 (스폰 직후 호출) */
	void InitTarget(float InLifespan, bool bInStrafe, bool bInArmor);

	/** 타겟이 제거되었을 때 (사망 or 시간초과) 알림 */
	FOnNaviTargetEliminated OnTargetEliminated;

	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComp; }

protected:
	/** 시각적 메시 (봇 외형) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Target")
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	/** 충돌체 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Target")
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	/** 아머 적용을 위한 GE 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Target")
	TSubclassOf<UGameplayEffect> ArmorEffectClass;

	/** 사망 시 재생할 몽타주 (선택 사항) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Target")
	TObjectPtr<UAnimMontage> DeathMontage;

	// --- 내부 상태 ---
	
	float CurrentLifespan = 0.0f;
	float TimeSinceSpawn = 0.0f;
	bool bShouldStrafe = false;
	bool bIsDead = false;

	// Strafe 관련 변수
	FVector SpawnLocation;
	float StrafeDirection = 1.0f;
	float StrafeSpeed = 150.0f;
	float StrafeRange = 100.0f;

	/** 체력이 0이 되었을 때 호출 */
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	
	/** 시간이 다 되어서 사라질 때 */
	void OnTimeExpired();
};
