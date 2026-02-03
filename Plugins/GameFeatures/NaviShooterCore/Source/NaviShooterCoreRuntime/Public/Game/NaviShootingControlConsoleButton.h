// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilitysystem/LyraActorWithAbilities.h"
#include "NaviShootingControlConsoleButton.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FShootingButtonHit, ANaviShootingControlConsoleButton* HitButton)

class UStaticMeshComponent;
struct FGameplayEffectSpec;


UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviShootingControlConsoleButton : public ALyraActorWithAbilities
{
	GENERATED_BODY()

public:
	ANaviShootingControlConsoleButton(const FObjectInitializer& ObjectInitializer);
	FShootingButtonHit OnShootingButtonHit;

	UFUNCTION(BlueprintCallable)
	void OnButtonHit();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|ConsoleButton")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;
	
};