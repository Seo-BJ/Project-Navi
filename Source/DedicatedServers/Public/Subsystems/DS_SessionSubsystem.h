// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/DS_HttpRequestSubsystem.h"
#include "Interfaces/IHttpRequest.h"
// #include "UI/HTTP/HTTPRequestManager.h" // 이전 부모 클래스 헤더 제거 (필요시 내부 로직 가져오기)
#include "DS_HttpRequestSubsystem.h"
#include "UI/HTTP/HTTPRequestTypes.h" // FAPIStatusMessage 때문에 필요할 수 있음

#include "DS_SessionSubsystem.generated.h"



// Forward Declarations
class UAPIData;
class UDSLocalPlayerSubsystem;
struct FDSGameSession; // .cpp 파일에서만 사용되면 Forward Declaration 불필요할 수 있음
struct FDSPlayerSession;

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UDS_SessionSubsystem : public UDS_HttpRequestSubsystem
{
	GENERATED_BODY()
	
public:
	// UGameInstanceSubsystem 기본 함수 오버라이드
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 기존 Public 멤버들
	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage BroadcastJoinGameSessionMessage;

	/** 게임 세션 찾기/생성 및 참여 프로세스를 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "Game Sessions")
	void JoinGameSession();

private:
	// 기존 Private 함수들
	void FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	// GetUniquePlayerId는 PlayerState 기반이라 클라이언트에서만 의미있을 수 있음. 필요시 유지.
	FString GetUniquePlayerId() const;
	void HandleGameSessionStatus(const FString& Status, const FString& SessionId);
	void TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	
	// Private 멤버 변수들
	FTimerHandle CreateSessionTimer;
};
