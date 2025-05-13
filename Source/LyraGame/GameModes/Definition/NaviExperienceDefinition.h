// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "LyraGameplayTags.h"
#include "NaviExperienceDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType) 
class LYRAGAME_API UNaviExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Information")
	FText ExperienceNameText;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Information")
	FGameplayTag ExperienceTag;
	
	/** The gameplay experience to load */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience, meta=(AllowedTypes="LyraExperienceDefinition"))
	FPrimaryAssetId ExperienceID;

	/** Extra arguments passed as URL options to the game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	TMap<FString, FString> ExtraArgs;

	/** If true, a replay will be recorded of the game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	bool bRecordReplay = false;
	
	/** Max number of players for this session */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	int32 MaxPlayerCount = 16;

	/** Create a request object about Exprience that is used to actually start a session and Server Travel */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta = (WorldContext = "WorldContextObject"))
	UCommonSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;
};
