// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "LyraGameplayTags.h" 
#include "NaviMapDefinition.generated.h"

class UTexture2D;
class UUserWidget;
class UCommonSession_HostSessionRequest;

UCLASS(BlueprintType) 
class LYRAGAME_API UNaviMapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	/** The specific map to load */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedTypes = "Map"))
    FPrimaryAssetId MapID;

	/** Map Name Text */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText MapName;

	/** Map Name Tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MapNameTag;
	
	/** Map Description Text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText MapDescription;
	
	/** Map Loading Screen Image */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> MapLoadingScreenImage;
	
    /**
     * @brief 이 맵으로 진입할 때 표시될 기본 로딩 스크린 위젯입니다.
     * 게임 모드나 다른 설정에 의해 오버라이드될 수 있습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<UUserWidget> LoadingScreenWidget;
	
	/** Create a request object about MapID that is used to actually start a session and Server Travel */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta = (WorldContext = "WorldContextObject"))
	UCommonSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;
	
};