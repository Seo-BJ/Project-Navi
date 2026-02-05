// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LagCompensation/ILagCompensationTarget.h"
#include "Abilitysystem/LyraActorWithAbilities.h"
#include "LyraPatrolPath.h"
#include "LyraLagCompensationTestActor.generated.h"

class ALyraPatrolPath;

UCLASS()
class LYRAGAME_API ALyraLagCompensationTestActor : public ALyraActorWithAbilities
{
	GENERATED_BODY()

public:
	ALyraLagCompensationTestActor(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	// PatrolPath에서 스폰 후 초기화하기 위한 함수
	void InitializeTestActor(ALyraPatrolPath* InPath, ELagCompensationTestMode InMode, float InSpeed);

	//~ ILagCompensationTarget interface
	virtual const TMap<FName, TObjectPtr<UBoxComponent>>& GetHitCollisionBoxes() const override;
	//~ End ILagCompensationTarget interface
	
protected:
	virtual void BeginPlay() override;
	
	// 테스트 모드 설정 (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Lag Compensation Test")
	ELagCompensationTestMode TestMode = ELagCompensationTestMode::Normal;

	// 순찰 경로 Actor (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	TObjectPtr<ALyraPatrolPath> PatrolPath;

	// 이동 속도 (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float MovementSpeed = 300.f;

private:
	FVector CurrentTargetLocation;

	TMap<FName, TObjectPtr<UBoxComponent>> HitCollisionBoxes;

	void UpdateTargetLocation();
};