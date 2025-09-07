// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LyraPatrolPath.generated.h"

class USceneComponent;

UCLASS()
class LYRAGAME_API ALyraPatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	ALyraPatrolPath();

	FVector GetPoint1WorldLocation() const;
	FVector GetPoint2WorldLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Point1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Point2;
};
