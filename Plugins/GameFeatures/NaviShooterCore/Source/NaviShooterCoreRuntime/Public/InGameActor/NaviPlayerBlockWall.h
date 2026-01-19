// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
	// ~AActor interface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	// 페이즈 변경 감지 콜백 (신규 추가)
	void OnGamePhaseChanged(const FGameplayTag& PhaseTag);

	// 감지할 페이즈 태그 (에디터에서 설정 가능하게 하거나 하드코딩)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navi|Phase")
	FGameplayTag TargetPhaseTag;


	UFUNCTION()
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
	
private:
	
	FDelegateHandle GamePhaseStartHandle;
};
