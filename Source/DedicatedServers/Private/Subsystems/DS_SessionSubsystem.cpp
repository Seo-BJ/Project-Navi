// Fill out your copyright notice in the Description page of Project Settings.


#include "DedicatedServers/Public/Subsystems/DS_SessionSubsystem.h"
// 필요한 기본 엔진 헤더들
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h" // GetUniquePlayerId에 필요했다면 유지
#include "Interfaces/IHttpResponse.h"
// 프로젝트 특정 헤더들
#include "Data/API/APIData.h" // APIData 클래스 정의 필요
#include "GameplayTags/DedicatedServersTags.h" // 태그 정의 필요
#include "Player/DSLocalPlayerSubsystem.h" // 로컬 플레이어 서브시스템 정의 필요
// #include "UI/HTTP/HTTPRequestTypes.h" // 헤더에 이미 포함

// --- FDSGameSession, FDSPlayerSession 구조체 정의가 포함된 헤더 ---
// 예: #include "Data/SessionData.h"
// ----------------------------------------------------------


// --- Initialize: 서브시스템 초기화 시 호출 ---
void UDS_SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 로드할 에셋의 경로를 지정합니다.
    const FString AssetPath = TEXT("/Game/DedicatedServers/Data/DA_GameSessionsAPIData");

    // LoadObject<T>를 사용하여 에셋을 로드합니다.
    // 첫 번째 인자는 Outer 객체이며, 보통 에셋 자체에는 nullptr을 사용합니다.
    // 두 번째 인자는 로드할 에셋의 경로입니다. FString을 TCHAR*로 변환하기 위해 * 연산자를 사용합니다.
    APIData = LoadObject<UAPIData>(nullptr, *AssetPath);

    // 로드가 성공했는지 확인하는 것이 중요합니다.
    if (!APIData)
    {
        // 로드 실패 시 에러 로그를 남깁니다. LogDSSessionSubsystem은 예시이며 실제 사용하는 로그 카테고리로 변경해야 합니다.
        UE_LOG(LogTemp, Error, TEXT("Failed to load APIData asset at path: %s"), *AssetPath);
        // 필요하다면 로드 실패 시 추가적인 에러 처리 로직을 구현할 수 있습니다.
    }
    else
    {
        // (선택 사항) 로드 성공 로그
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded APIData asset: %s"), *APIData->GetName());
    }
}

// --- Deinitialize: 서브시스템 해제 시 호출 ---
void UDS_SessionSubsystem::Deinitialize()
{
    // 활성 타이머 정리
    if (UWorld* World = GetGameInstance()->GetWorld())
    {
        World->GetTimerManager().ClearTimer(CreateSessionTimer);
    }
    else // 월드가 이미 없을 수도 있음 (종료 시점)
    {
         // GetWorldTimerManager()는 World가 필요하므로 여기선 추가 작업 어려움
    }

    // TODO: 진행 중인 HTTP 요청 취소 등 추가 정리 작업 필요시 구현

    UE_LOG(LogTemp, Log, TEXT("UGameSessionsSubsystem Deinitialized."));
    Super::Deinitialize();
}


void UDS_SessionSubsystem::JoinGameSession()
{
    BroadcastJoinGameSessionMessage.Broadcast(TEXT("Searching for Game Session..."), false);

    check(APIData);
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UDS_SessionSubsystem::FindOrCreateGameSession_Response);
    const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServersTags::GameSessionsAPI::FindOrCreateGameSession);
    Request->SetURL(APIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
    if (IsValid(LocalPlayerSubsystem))
    {
        Request->SetHeader(TEXT("Authorization"), LocalPlayerSubsystem->GetAuthResult().AccessToken);
    }
	
    Request->ProcessRequest();
}

void UDS_SessionSubsystem::FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        if (ContainsErrors(JsonObject))
        {
            BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
        }

        FDSGameSession GameSession;
        FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);

        const FString GameSessionId = GameSession.GameSessionId;
        const FString GameSessionStatus = GameSession.Status;
        HandleGameSessionStatus(GameSessionStatus, GameSessionId);
    }
}

// --- CreatePlayerSession_Response: 응답 처리 ---
void UDS_SessionSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful)
    {
        BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        if (ContainsErrors(JsonObject))
        {
            BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
        }

        FDSPlayerSession PlayerSession;
        FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &PlayerSession);
        PlayerSession.Dump();
		
        APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
        if (IsValid(LocalPlayerController))
        {
            FInputModeGameOnly InputModeData;
            LocalPlayerController->SetInputMode(InputModeData);
            LocalPlayerController->SetShowMouseCursor(false);
        }

        const FString Options = "?PlayerSessionId=" + PlayerSession.PlayerSessionId + "?Username=" + PlayerSession.PlayerId;
		
        const FString IpAndPort = PlayerSession.IpAddress + TEXT(":") + FString::FromInt(PlayerSession.Port);
        const FName Address(*IpAndPort);
        UGameplayStatics::OpenLevel(this, Address, true, Options);
    }
}

FString UDS_SessionSubsystem::GetUniquePlayerId() const
{
    APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
    if (IsValid(LocalPlayerController))
    {
        APlayerState* LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
        if (IsValid(LocalPlayerState) && LocalPlayerState->GetUniqueId().IsValid())
        {
            return TEXT("Player_") + FString::FromInt(LocalPlayerState->GetUniqueID());
        }
    }
    return FString();
}

void UDS_SessionSubsystem::HandleGameSessionStatus(const FString& Status, const FString& SessionId)
{
    if (Status.Equals(TEXT("ACTIVE")))
    {
        BroadcastJoinGameSessionMessage.Broadcast(TEXT("Found active Game Session. Creating a Player Session..."), false);

        if (UDSLocalPlayerSubsystem* DSLocalPlayerSubsystem = GetDSLocalPlayerSubsystem(); IsValid(DSLocalPlayerSubsystem))
        {
            TryCreatePlayerSession(DSLocalPlayerSubsystem->Username, SessionId);
        }
    }
    else if (Status.Equals(TEXT("ACTIVATING")))
    {
        FTimerDelegate CreateSessionDelegate;
        CreateSessionDelegate.BindUObject(this, &ThisClass::JoinGameSession);
        APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
        if (IsValid(LocalPlayerController))
        {
            LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CreateSessionDelegate, 0.5f, false);
        }
    }
    else
    {
        BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
    }
}

// --- TryCreatePlayerSession: 플레이어 세션 생성 요청 ---
void UDS_SessionSubsystem::TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
    check(APIData);
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UDS_SessionSubsystem::CreatePlayerSession_Response);
    const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServersTags::GameSessionsAPI::CreatePlayerSession);
    Request->SetURL(APIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TMap<FString, FString> Params = {
        { TEXT("playerId"), PlayerId },
        { TEXT("gameSessionId"), GameSessionId }
    };
    const FString Content = SerializeJsonContent(Params);

    Request->SetContentAsString(Content);
    Request->ProcessRequest();
}


