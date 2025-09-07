// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraTimeSyncComponent.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ULyraTimeSyncComponent::ULyraTimeSyncComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); 
}

void ULyraTimeSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	AController* Controller = GetOwner<AController>();
	if (IsValid(Controller))
	{
		OwnerController = Controller;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());

	}
}

void ULyraTimeSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckTimeSync(DeltaTime);
}

void ULyraTimeSyncComponent::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (OwnerController && OwnerController->IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		TimeSyncRunningTime = 0.f;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ULyraTimeSyncComponent::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ULyraTimeSyncComponent::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = FMath::Max(0.f, RoundTripTime * 0.5f); // 음수 방지

	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ULyraTimeSyncComponent::GetServerTime() const
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}