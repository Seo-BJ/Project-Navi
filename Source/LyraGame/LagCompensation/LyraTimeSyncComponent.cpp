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
		if (Controller->IsLocalController() && !HasAuthority())
		{
			TimeSyncRunningTime = 0.f;
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
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const float ClientNow = World->GetTimeSeconds();
	const float RoundTripTime = ClientNow - TimeOfClientRequest;
	if (RoundTripTime < 0.f)
	{
		// 요청을 보낸 뒤 월드가 바뀌어(심리스 트래블) 시간이 다시 시작된 경우. 의미 없는 샘플이므로 버린다.
		return;
	}

	// 이번 측정만으로 계산한 오프셋. 바로 쓰지 않고 샘플로 저장한 뒤 최소 RTT 샘플을 고른다.
	const float SampleServerNow = TimeServerReceivedClientRequest + RoundTripTime * 0.5f;
	AddTimeSyncSample(RoundTripTime, SampleServerNow - ClientNow);

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
		TEXT("[TimeSyncCompare/Sync] RTT=%.4f (min of %d=%.4f) | ServerNow self=%.4f engineCorr=%.4f diff=%.4f | OneWay self=%.4f engine=%.4f diff=%.4f"),
		RoundTripTime, TimeSyncSamples.Num(), SingleTripTime * 2.f,
		SelfServerTime, EngineServerTimeCorrected, SelfServerTime - EngineServerTimeCorrected,
		SingleTripTime, EngineSingleTrip, SingleTripTime - EngineSingleTrip);
}

void ULyraTimeSyncComponent::AddTimeSyncSample(float RoundTripTime, float SampleClientServerDelta)
{
	UWorld* World = GetWorld();
	if (TimeSyncSampleWorld.Get() != World)
	{
		// 월드가 바뀌면 월드 시간이 다시 시작하므로 이전 월드에서 잰 오프셋은 쓸 수 없다.
		TimeSyncSamples.Reset();
		TimeSyncSampleWorld = World;
	}

	const int32 SampleLimit = FMath::Max(1, MaxTimeSyncSamples);
	while (TimeSyncSamples.Num() >= SampleLimit)
	{
		TimeSyncSamples.RemoveAt(0, EAllowShrinking::No);
	}
	TimeSyncSamples.Add({ RoundTripTime, SampleClientServerDelta });

	// 보관 중인 샘플 중 RTT가 가장 작은 것을 쓴다.
	// 오프셋(시계 차이)은 짧은 창 안에서는 거의 변하지 않으므로, 가장 덜 오염된 측정의 오프셋이 가장 정확하다.
	const FTimeSyncSample* BestSample = &TimeSyncSamples[0];
	for (const FTimeSyncSample& Sample : TimeSyncSamples)
	{
		if (Sample.RoundTripTime < BestSample->RoundTripTime)
		{
			BestSample = &Sample;
		}
	}

	SingleTripTime = FMath::Max(0.f, BestSample->RoundTripTime * 0.5f);
	ClientServerDelta = BestSample->ClientServerDelta;
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