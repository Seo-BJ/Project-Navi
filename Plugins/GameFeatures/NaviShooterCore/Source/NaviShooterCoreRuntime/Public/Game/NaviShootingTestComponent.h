// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Components/GameStateComponent.h"
#include "Game/NaviShootingTestTypes.h"
#include "NaviShootingTestComponent.generated.h"

class ANaviShootingTarget;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNaviTestStateChanged, bool, bIsRunning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNaviTestScoreUpdate, int32, CurrentStreak, int32, TargetStreak);

/**
 * UNaviShootingTestComponent
 * 
 * 사격 훈련장 로직을 총괄하는 컴포넌트입니다. GameState에 부착되어 작동합니다.
 * 타겟 스폰, 점수 기록, 난이도 조절 등을 담당합니다.
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviShootingTestComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UNaviShootingTestComponent(const FObjectInitializer& ObjectInitializer);

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	/** 테스트 시작 */
	UFUNCTION(BlueprintCallable, Category = "Navi|ShootingTest")
	void StartTest(AActor* Instigator, const FNaviShootingTestSettings& InSettings);

	/** 테스트 강제 종료 */
	UFUNCTION(BlueprintCallable, Category = "Navi|ShootingTest")
	void EndTest();

	/** 현재 설정 변경 (테스트 중이 아닐 때만 가능) */
	UFUNCTION(BlueprintCallable, Category = "Navi|ShootingTest")
	void UpdateSettings(const FNaviShootingTestSettings& NewSettings);

	/** 현재 설정 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Navi|ShootingTest")
	const FNaviShootingTestSettings& GetSettings() const { return CurrentSettings; }

	// --- 이벤트 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "Navi|ShootingTest")
	FOnNaviTestStateChanged OnTestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Navi|ShootingTest")
	FOnNaviTestScoreUpdate OnScoreUpdated;

protected:
	/** 봇을 스폰할 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Navi|ShootingTest")
	TSubclassOf<ANaviShootingTarget> TargetClass;

	/** 무한 탄창 부여용 이펙트 */
	UPROPERTY(EditDefaultsOnly, Category = "Navi|ShootingTest")
	TSubclassOf<UGameplayEffect> InfiniteAmmoEffectClass;

	/** 기준이 될 스폰 포인트 (콘솔 등에서 주입) */
	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnPoint;

public:
	/** 스폰 포인트 설정 (외부 주입용) */
	void SetSpawnPoint(AActor* InSpawnPoint) { SpawnPoint = InSpawnPoint; }

private:
	UPROPERTY(Replicated)
	FNaviShootingTestSettings CurrentSettings;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsTestRunning)
	bool bIsTestRunning = false;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentKillCount)
	int32 CurrentKillCount = 0;
	
	UPROPERTY(Replicated)
	int32 TargetKillCount = 0; // 목표 킬 수 (Endless면 -1)

	UPROPERTY(Replicated)
	TObjectPtr<AActor> TestInstigator;

	UFUNCTION()
	void OnRep_IsTestRunning();

	UFUNCTION()
	void OnRep_CurrentKillCount();

	/** 현재 스폰된 타겟들 (보통 1개지만 확장성 고려) */
	UPROPERTY()
	TArray<TObjectPtr<ANaviShootingTarget>> ActiveTargets;

	/** 다음 스폰을 위한 타이머 핸들 */
	FTimerHandle SpawnTimerHandle;

	/** 무한 탄창 이펙트 핸들 (종료 시 제거용) */
	FActiveGameplayEffectHandle InfiniteAmmoHandle;

	// --- 내부 로직 ---
	
	void SpawnNextTarget();

	UFUNCTION()
	void OnTargetEliminated(ANaviShootingTarget* Target, bool bWasKilled);
	void CleanupTargets();
	
	/** 난이도에 따른 봇 수명 계산 */
	float GetTargetLifespan() const;

	/** 플레이어에게 무한 탄창 부여/제거 */
	void SetInfiniteAmmo(bool bEnable);

	/** 디버그용으로 스폰 범위 그리기 */
	void DrawDebugRange();
};
