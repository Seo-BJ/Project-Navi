// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "LyraTimeSyncComponent.generated.h"

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

	// 클라이언트와 서버 시간의 차이/
	// (추정된 서버 시간) - (클라이언트 로컬 시간)
	float ClientServerDelta = 0.f;

	// 클라이언트와 서버 간의 편도 시간(지연 시간)
	float SingleTripTime = 0.f;
	
	// 시간 동기화 요청을 보내는 주기 (초 단위)
	UPROPERTY(EditDefaultsOnly, Category = "TimeSync")
	float TimeSyncFrequency = 2.f;

	// 다음 동기화까지의 시간을 측정하는 타이머
	float TimeSyncRunningTime = 0.f;

	UPROPERTY()
	TObjectPtr<AController> OwnerController;
};
