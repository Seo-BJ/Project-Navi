// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"

#include "NaviFppCharacter.generated.h"

class USpringArmComponent;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviFppCharacter : public ALyraCharacter
{
	GENERATED_BODY()

public:
	ANaviFppCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;
};
