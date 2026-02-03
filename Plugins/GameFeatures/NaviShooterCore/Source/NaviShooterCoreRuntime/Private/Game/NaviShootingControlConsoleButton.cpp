// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NaviShootingControlConsoleButton.h"

#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "Components/StaticMeshComponent.h"


ANaviShootingControlConsoleButton::ANaviShootingControlConsoleButton(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	RootComponent = ButtonMesh;
}

// Called when the game starts or when spawned
void ANaviShootingControlConsoleButton::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ANaviShootingControlConsoleButton::OnButtonHit()
{
	OnShootingButtonHit.Broadcast(this);
}

