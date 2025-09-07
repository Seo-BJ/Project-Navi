// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ILagCompensationTarget.h"
#include "Abilitysystem/LyraActorWithAbilities.h"
#include "LyraTestWeaponFire.generated.h"

class ALyraPatrolPath;

UCLASS()
class LYRAGAME_API ALyraTestWeaponFire : public ALyraActorWithAbilities, public ILagCompensationTarget
{
	GENERATED_BODY()

public:
	ALyraTestWeaponFire(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	//~ ILagCompensationTarget interface
	virtual const TMap<FName, TObjectPtr<UBoxComponent>>& GetHitCollisionBoxes() const override;
	//~ End ILagCompensationTarget interface
	
protected:
	virtual void BeginPlay() override;
	
	// 순찰 경로 Actor
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	TObjectPtr<ALyraPatrolPath> PatrolPath;

	// 이동 속도
	UPROPERTY(EditInstanceOnly, Category = "Movement")
	float MovementSpeed = 300.f;

private:
	FVector CurrentTargetLocation;

	TMap<FName, TObjectPtr<UBoxComponent>> HitCollisionBoxes;

	void UpdateTargetLocation();
};