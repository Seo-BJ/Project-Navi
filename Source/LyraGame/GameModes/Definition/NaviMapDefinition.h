// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "LyraGameplayTags.h" 
#include "NaviMapDefinition.generated.h"

class UTexture2D;
class UUserWidget;

UCLASS(BlueprintType) 
class LYRAGAME_API UNaviMapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	/** The specific map to load */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedTypes = "Map"))
    FPrimaryAssetId ActualMapToLoad;

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
	
    // UPrimaryDataAsset의 GetPrimaryAssetId는 기본적으로 에셋 자체의 ID를 반환합니다.
    // (예: NaviMapDefinition'/Game/Data/Maps/DA_Map_Ascent.DA_Map_Ascent')
    // 이 UNaviMapDefinition 에셋 자체가 고유한 Primary Asset이 되므로,
    // 이 함수를 특별히 오버라이드할 필요는 대부분 없습니다.
    // AssetType을 명시적으로 설정하고 싶다면 생성자에서 할 수 있습니다.
    // virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	void RequestAndSetLoadingScreenWidget(UObject* WorldContextObject) const;
	
};