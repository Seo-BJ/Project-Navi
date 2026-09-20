// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "LyraTimeSyncComponent.generated.h"

class UWorld;

/**
 * PlayerController에 부착되어 서버와 클라이언트 간의 시간을 동기화하는 컴포넌트
 * 서버 사이드 리와인드의 정확도를 위해 클라이언트가 서버 시간을 계산할수 있도록 함
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraTimeSyncComponent : public UControllerComponent
{
	GENERATED_BODY()

public:	
	ULyraTimeSyncComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief 현재 서버 시간을 추정하여 반환
	 * @return 서버의 현재 시간 (클라이언트에서는 추정치, 서버에서는 실제 시간)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeSync")
	float GetServerTime() const;

	/**
	 * @brief 클라이언트와 서버 간의 편도 시간(지연 시간)을 반환
	 * @return 편도 시간 (초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeSync")
	float GetSingleTripTime() const { return SingleTripTime; }

protected:
	virtual void BeginPlay() override;

private:
	/**
	 * @brief 주기적으로 서버에 시간 동기화를 요청합니다.
	 */
	void CheckTimeSync(float DeltaTime);

	/*
	* 클라이언트-서버 시간 동기화 RPC
	*/

	// 클라이언트가 서버에 현재 서버 시간을 요청 (클라이언트 -> 서버)
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// 서버가 클라이언트의 요청에 응답하여 시간 정보를 전달 (서버 -> 클라이언트)
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	/** 동기화 1회(요청 → 응답)에서 얻은 측정값 */
	struct FTimeSyncSample
	{
		// 왕복 시간 (초)
		float RoundTripTime = 0.f;

		// 이 샘플로 계산한 (추정 서버 시간) - (클라이언트 로컬 시간)
		float ClientServerDelta = 0.f;
	};

	/**
	 * 응답 하나를 샘플로 저장하고, 보관 중인 샘플 중 RTT가 가장 작은 것으로 ClientServerDelta와 SingleTripTime을 정한다.
	 * RTT가 가장 작은 샘플은 재전송·큐잉·서버 프레임 대기가 가장 적게 섞인 측정이므로
	 * "왕복이 대칭"이라는 가정(편도 = RTT/2)에 가장 가깝다 (NTP의 최소 지연 필터와 같은 방식).
	 */
	void AddTimeSyncSample(float RoundTripTime, float SampleClientServerDelta);

	// 클라이언트와 서버 시간의 차이/
	// (추정된 서버 시간) - (클라이언트 로컬 시간)
	float ClientServerDelta = 0.f;

	// 클라이언트와 서버 간의 편도 시간(지연 시간)
	float SingleTripTime = 0.f;

	// 최근 동기화 샘플 (오래된 것부터). 최대 MaxTimeSyncSamples개.
	TArray<FTimeSyncSample> TimeSyncSamples;

	// 샘플을 측정한 월드. 심리스 트래블로 월드가 바뀌면 월드 시간이 0부터 다시 시작하므로 이전 샘플을 버린다.
	TWeakObjectPtr<UWorld> TimeSyncSampleWorld;

	// 시간 동기화 요청을 보내는 주기 (초 단위)
	UPROPERTY(EditDefaultsOnly, Category = "TimeSync")
	float TimeSyncFrequency = 1.f;

	/**
	 * 최소 RTT를 고를 때 보관하는 최근 샘플 수.
	 * 클수록 튀는 값에 강하지만, 지연이 계속 늘어난 경우 반영이 늦어진다 (최대 MaxTimeSyncSamples * TimeSyncFrequency 초).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "TimeSync", meta = (ClampMin = 1))
	int32 MaxTimeSyncSamples = 8;

	// 다음 동기화까지의 시간을 측정하는 타이머
	float TimeSyncRunningTime = 0.f;

	UPROPERTY()
	TObjectPtr<AController> OwnerController;
};
