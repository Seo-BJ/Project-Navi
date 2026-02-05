// Fill out your copyright notice in the Description page of Project Settings.


// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPatrolPath.h"
#include "Components/StaticMeshComponent.h"
#include "LyraLagCompensationTestCharacter.h"
#include "LyraLagCompensationTestActor.h"

ALyraPatrolPath::ALyraPatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	// 충돌을 완전히 끔 (루트 컴포넌트의 충돌 프로필을 NoCollision으로 설정)
	DefaultSceneRoot->SetCanEverAffectNavigation(false);
	
	// 데미지 및 기타 설정
	SetCanBeDamaged(false);

	Point1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Point1"));
	Point1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Point1->SetupAttachment(DefaultSceneRoot);
	Point1->SetRelativeLocation(FVector(0.f, -100.f, 0.f));

	Point2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Point2"));
	Point2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Point2->SetupAttachment(DefaultSceneRoot);
	Point2->SetRelativeLocation(FVector(0.f, 100.f, 0.f));
}

void ALyraPatrolPath::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 시각적 요소 숨김
	SetActorHiddenInGame(true);

	if (HasAuthority() && ActorClassToSpawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;

		FVector SpawnLocation = GetPoint1WorldLocation();
		FRotator SpawnRotation = (GetPoint2WorldLocation() - SpawnLocation).Rotation();

		SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (SpawnedActor)
		{
			if (ALyraLagCompensationTestCharacter* TestChar = Cast<ALyraLagCompensationTestCharacter>(SpawnedActor))
			{
				TestChar->InitializeTestCharacter(this, TestMode, MovementSpeed);
			}
			else if (ALyraLagCompensationTestActor* TestActor = Cast<ALyraLagCompensationTestActor>(SpawnedActor))
			{
				TestActor->InitializeTestActor(this, TestMode, MovementSpeed);
			}
		}
	}
}

FVector ALyraPatrolPath::GetPoint1WorldLocation() const
{
	return Point1->GetComponentLocation();
}

FVector ALyraPatrolPath::GetPoint2WorldLocation() const
{
	return Point2->GetComponentLocation();
}