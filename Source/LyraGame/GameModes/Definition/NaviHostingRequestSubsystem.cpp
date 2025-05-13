// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviHostingRequestSubsystem.h"

#include "NaviExperienceDefinition.h"
#include "NaviMapDefinition.h"
#include "CommonSessionSubsystem.h"

#include "Replays/LyraReplaySubsystem.h"

UCommonSession_HostSessionRequest* UNaviHostingRequestSubsystem::CreateCombinedHostingRequest(
	const UObject* WorldContextObject, const UNaviExperienceDefinition* ExperienceDefinition,
	const UNaviMapDefinition* MapDefinition)
{
    // 입력 유효성 검사
    if (!WorldContextObject || !ExperienceDefinition || !MapDefinition)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateCombinedHostingRequest: 유효하지 않은 입력값입니다 (WorldContext, ExperienceDefinition 또는 MapDefinition이 null)."));
        return nullptr;
    }

	const FString ExperienceName = ExperienceDefinition->ExperienceID.PrimaryAssetName.ToString();
	const FString UserFacingExperienceName = GetPrimaryAssetId().PrimaryAssetName.ToString();

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UCommonSession_HostSessionRequest* Result = nullptr;
	
	if (UCommonSessionSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UCommonSessionSubsystem>() : nullptr)
	{
		Result = Subsystem->CreateOnlineHostSessionRequest();
	}

	if (!Result)
	{
		// Couldn't use the subsystem so create one
		Result = NewObject<UCommonSession_HostSessionRequest>();
		Result->OnlineMode = ECommonSessionOnlineMode::Online;
		Result->bUseLobbies = true;
		Result->bUseLobbiesVoiceChat = false;
		// We always enable presence on this session because it is the primary session used for matchmaking. For online systems that care about presence, only the primary session should have presence enabled
		Result->bUsePresence = !IsRunningDedicatedServer();
	}
	
    Result->MapID = MapDefinition->MapID; 
	
    Result->ModeNameForAdvertisement = UserFacingExperienceName;
    Result->MaxPlayerCount = ExperienceDefinition->MaxPlayerCount; 
	
    Result->ExtraArgs = ExperienceDefinition->ExtraArgs; 
    Result->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	
    if (ULyraReplaySubsystem::DoesPlatformSupportReplays())
    {
       if (ExperienceDefinition->bRecordReplay) // bRecordReplay 변수가 있다고 가정
       {
          Result->ExtraArgs.Add(TEXT("DemoRec"), FString());
       }
    }

    // 최종 결합된 Request 반환
    return Result;
	
}

FString UNaviHostingRequestSubsystem::GetTravelURL(UCommonSession_HostSessionRequest* Request)
{

	FString PendingTravelURL = Request->ConstructTravelURL();
	return PendingTravelURL; 
	// GetWorld()->ServerTravel(PendingTravelURL);
	
}

void UNaviHostingRequestSubsystem::ServerTravelWithSavedDefinitions(const UObject* WorldContextObject)
{
	UCommonSession_HostSessionRequest* Request = CreateCombinedHostingRequest(WorldContextObject, SavedExperienceDefinition, SavedMapDefinition);
	FString URL = GetTravelURL(Request);
	WorldContextObject->GetWorld()->ServerTravel(URL);
}




void UNaviHostingRequestSubsystem::SaveSelectedMapAndExperienceDefinition(UNaviExperienceDefinition* ExperienceDefinition, UNaviMapDefinition* MapDefinition)
{
	SavedExperienceDefinition = ExperienceDefinition;
	SavedMapDefinition = MapDefinition;
}
