// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LyraGameplayTags.h" 
#include "NaviMapDefinition.generated.h"

class UTexture2D;
class UUserWidget;

UCLASS(BlueprintType, Const, meta = (DisplayName = "Navi Map Definition")) 
class LYRAGAME_API UNaviMapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

    /**
     * @brief 실제로 로드될 맵 월드 에셋의 Primary Asset ID입니다.
     * (예: World'/Game/Maps/YourMapFolder/MapName.MapName')
     * 이 ID는 에셋 매니저가 해당 맵을 식별하고 로드하는 데 사용됩니다.
     * 프로젝트 설정 > 에셋 관리자에서 해당 맵 월드(.umap) 에셋이 Primary Asset으로 스캔되도록 설정해야 합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedTypes = "Map"))
    FPrimaryAssetId ActualMapToLoad;

    /**
     * @brief UI에 표시될 맵의 이름입니다. (현지화 가능하도록 FText 사용)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    /**
     * @brief UI (맵 선택 화면, 로딩 화면 등)에 표시될 맵의 대표 이미지입니다.
     * 비동기 로딩을 위해 TSoftObjectPtr을 사용하는 것이 좋습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> MapPreviewImage;
	
    /**
     * @brief UI에 표시될 맵의 상세 설명입니다. (선택 사항)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
    FText MapDescription;

    /**
     * @brief 이 맵을 식별하거나 특정 카테고리로 분류하는 데 사용될 게임플레이 태그입니다.
     * (예: Map.Size.Small, Map.Environment.Desert, Map.Feature.OneWayTeleporter 등)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer MapTags;

    /**
     * @brief 이 맵에서 지원하는 게임 모드들을 나타내는 게임플레이 태그입니다.
     * 예를 들어, 이 맵이 "폭탄 설치" 모드(태그: Navi.GameMode.BombMission)와
     * "팀 데스매치" 모드(태그: Navi.GameMode.TeamDeathmatch)를 지원한다면 해당 태그들을 추가합니다.
     * UI에서 선택된 게임 모드에 따라 맵을 필터링하는 데 사용할 수 있습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer SupportedGameModes;

    /**
     * @brief 이 맵으로 진입할 때 표시될 기본 로딩 스크린 위젯입니다.
     * 게임 모드나 다른 설정에 의해 오버라이드될 수 있습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<UUserWidget> LoadingScreenWidget;

    /**
     * @brief 이 맵에서 허용되는 최대 플레이어 수입니다. (0 이하면 게임 모드의 기본값 사용)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
    int32 MaxPlayerCountOverride = 0;


    // UPrimaryDataAsset의 GetPrimaryAssetId는 기본적으로 에셋 자체의 ID를 반환합니다.
    // (예: NaviMapDefinition'/Game/Data/Maps/DA_Map_Ascent.DA_Map_Ascent')
    // 이 UNaviMapDefinition 에셋 자체가 고유한 Primary Asset이 되므로,
    // 이 함수를 특별히 오버라이드할 필요는 대부분 없습니다.
    // AssetType을 명시적으로 설정하고 싶다면 생성자에서 할 수 있습니다.
    // virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	void RequestAndSetLoadingScreenWidget(UObject* WorldContextObject) const;
	
};