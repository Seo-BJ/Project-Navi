// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviLoadingScreenBase.generated.h"

class UTexture2D;
class UImage;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviLoadingScreenBase : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void UpdateLoadingScreenBackgroundImage(TSoftObjectPtr<UTexture2D> BackgroundTexture);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LoadingScreenBackgroundImage;
	
};
