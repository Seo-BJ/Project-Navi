// Fill out your copyright notice in the Description page of Project Settings.


// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPatrolPath.h"
#include "Components/SceneComponent.h"

ALyraPatrolPath::ALyraPatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	Point1 = CreateDefaultSubobject<USceneComponent>(TEXT("Point1"));
	Point1->SetupAttachment(DefaultSceneRoot);
	Point1->SetRelativeLocation(FVector(0.f, -100.f, 0.f));

	Point2 = CreateDefaultSubobject<USceneComponent>(TEXT("Point2"));
	Point2->SetupAttachment(DefaultSceneRoot);
	Point2->SetRelativeLocation(FVector(0.f, 100.f, 0.f));
}

FVector ALyraPatrolPath::GetPoint1WorldLocation() const
{
	return Point1->GetComponentLocation();
}

FVector ALyraPatrolPath::GetPoint2WorldLocation() const
{
	return Point2->GetComponentLocation();
}