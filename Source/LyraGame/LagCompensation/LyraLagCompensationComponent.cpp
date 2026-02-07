// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensation/LyraLagCompensationComponent.h"

ULyraLagCompensationComponent::ULyraLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->HasAuthority())
	{
		UpdateFrameHistory();
	}
}
