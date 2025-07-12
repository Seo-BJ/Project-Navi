// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviPlayerCredsComponent.h"


// Sets default values for this component's properties
UNaviPlayerCredsComponent::UNaviPlayerCredsComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
}


// Called when the game starts
void UNaviPlayerCredsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNaviPlayerCredsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

