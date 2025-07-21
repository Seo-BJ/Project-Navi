// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"
#include "NaviCredsShopControllerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviCredsShopControllerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNaviCredsShopControllerComponent(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Navi|Shop")
	void ServerRequestPurchaseWeapon(FGameplayTag WeaponTag);
};
