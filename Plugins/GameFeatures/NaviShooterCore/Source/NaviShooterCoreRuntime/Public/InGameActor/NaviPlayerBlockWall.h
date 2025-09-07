// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NaviPlayerBlockWall.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviPlayerBlockWall : public AActor
{
	GENERATED_BODY()

public:
	ANaviPlayerBlockWall();

protected:
	// AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~AActor interface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UFUNCTION()
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
private:

	UFUNCTION()
	void OnPlayingPhaseStarted();
	
	FDelegateHandle GamePhaseStartHandle;
};
