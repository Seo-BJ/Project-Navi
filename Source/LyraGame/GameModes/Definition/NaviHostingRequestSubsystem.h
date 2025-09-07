// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NaviHostingRequestSubsystem.generated.h"

class UNaviExperienceDefinition;
class UNaviMapDefinition;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviHostingRequestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 제공된 Experience 및 Map Definition을 기반으로 결합된 호스팅 요청 객체를 생성합니다.
	 *
	 * @param WorldContextObject 현재 월드 컨텍스트를 제공하는 오브젝트 (GameInstance 등을 얻기 위함)
	 * @param ExperienceDefinition 사용할 Experience 설정
	 * @param MapDefinition 사용할 Map 설정
	 * @return 성공 시 생성되고 채워진 UCommonSession_HostSessionRequest 객체, 실패 시 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Navi|Hosting")
	UCommonSession_HostSessionRequest* CreateCombinedHostingRequest(
		const UObject* WorldContextObject,
		const UNaviExperienceDefinition* ExperienceDefinition,
		const UNaviMapDefinition* MapDefinition
	);
	
	UFUNCTION(BlueprintCallable, Category = "Navi|Hosting")
	FString GetTravelURL(UCommonSession_HostSessionRequest* Request);
	
	UFUNCTION(BlueprintCallable, Category = "Navi|Hosting")
	void ServerTravelWithSavedDefinitions(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Navi|Hosting")
	void SaveSelectedMapAndExperienceDefinition(UNaviExperienceDefinition* ExperienceDefinition,
	UNaviMapDefinition* MapDefinition
	);

private:

	TObjectPtr<UNaviExperienceDefinition> SavedExperienceDefinition;
	TObjectPtr<UNaviMapDefinition> SavedMapDefinition;

	
};
