// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraTimeSyncComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
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
		if (!HasAuthority())
		{

		}
		else
		{
			ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		}
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
	SingleTripTime = FMath::Max(0.f, RoundTripTime * 0.5f);

	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

	float EngineServerTime = 0.f;
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		EngineServerTime = GS->GetServerWorldTimeSeconds();
	}
	float EngineSingleTrip = 0.f;
	if (IsValid(OwnerController) && OwnerController->PlayerState)
	{
		EngineSingleTrip = OwnerController->PlayerState->GetPingInMilliseconds() / 2000.f;
	}
	const float SelfServerTime = GetWorld()->GetTimeSeconds() + ClientServerDelta;
	// EngineServerTime은 리플 지연 때문에 이미 OneWay만큼 과거 → ServerNow로 비교하려면 EngineSingleTrip을 더해 origin을 맞춘다.
	const float EngineServerTimeCorrected = EngineServerTime + EngineSingleTrip;
	UE_LOG(LogTemp, Log,
		TEXT("[TimeSyncCompare/Sync] RTT=%.4f | ServerNow self=%.4f engineCorr=%.4f diff=%.4f | OneWay self=%.4f engine=%.4f diff=%.4f"),
		RoundTripTime,
		SelfServerTime, EngineServerTimeCorrected, SelfServerTime - EngineServerTimeCorrected,
		SingleTripTime, EngineSingleTrip, SingleTripTime - EngineSingleTrip);
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