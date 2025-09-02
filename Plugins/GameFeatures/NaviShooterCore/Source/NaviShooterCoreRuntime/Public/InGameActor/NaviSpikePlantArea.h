// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NaviSpikePlantArea.generated.h"

class UBoxComponent;

/**
 * 스파이크를 설치할 수 있는 사각형 영역을 정의하는 액터입니다.
 * 이 영역에 들어온 플레이어에게 GameplayTag를 부여합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviSpikePlantArea : public AActor
{
	GENERATED_BODY()

public:
	ANaviSpikePlantArea();

protected:
	// AActor interface
	virtual void BeginPlay() override;
	// ~AActor interface

	UFUNCTION(BlueprintNativeEvent, Category = "Spike Area")
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Spike Area")
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> OverlapBox;

	// 플레이어에게 부여/제거할 게임플레이 태그입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spike")
	FGameplayTag InPlantAreaTag;

private:
	void CheckIfGrounded();

	bool bIsPlantable;
};