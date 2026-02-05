// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"
#include "LyraPatrolPath.h"
#include "LyraLagCompensationTestCharacter.generated.h"

class ALyraPatrolPath;

/**
 * ALyraLagCompensationTestCharacter
 * 
 * Character version of LyraLagCompensationTestActor to test LyraMeshLagCompensationComponent
 * with skeletal mesh assets and animations.
 */
UCLASS()
class LYRAGAME_API ALyraLagCompensationTestCharacter : public ALyraCharacter
{
	GENERATED_BODY()

public:
	ALyraLagCompensationTestCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	// PatrolPath에서 스폰 후 초기화하기 위한 함수
	void InitializeTestCharacter(ALyraPatrolPath* InPath, ELagCompensationTestMode InMode, float InSpeed);

protected:
	virtual void BeginPlay() override;

	// 테스트 모드 설정 (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Lag Compensation Test")
	ELagCompensationTestMode TestMode = ELagCompensationTestMode::Normal;

	// Patrol path actor (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	TObjectPtr<ALyraPatrolPath> PatrolPath;

	// Movement speed (PatrolPath에서 설정됨)
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float MovementSpeed = 300.f;

private:
	FVector CurrentTargetLocation;

	// 현재 왕복(Trip)에서 액션을 수행했는지 여부
	bool bHasPerformedTripAction = false;

	void UpdateTargetLocation();
	void ApplyModeActions();
};
