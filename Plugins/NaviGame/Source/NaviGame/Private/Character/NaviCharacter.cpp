// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NaviCharacter.h"


// Sets default values
ANaviCharacter::ANaviCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANaviCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANaviCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANaviCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

