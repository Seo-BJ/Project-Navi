// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"

#include "UObject/WeakObjectPtr.h"
#include "LyraLoadingScreenSubsystem.generated.h"

class UObject;
class UUserWidget;
struct FFrame;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingScreenWidgetChangedDelegate, TSubclassOf<UUserWidget>, NewWidgetClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingScreenBackgroundTextureChangedDelegate, TSoftObjectPtr<UTexture2D>, NewTexture2D);
/**
 * Tracks/stores the current loading screen configuration in a place
 * that persists across map transitions
 */
UCLASS()
class LYRAGAME_API ULyraLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULyraLoadingScreenSubsystem();

	// Sets the loading screen widget class to display inside of the loading screen widget host
	UFUNCTION(BlueprintCallable)
	void SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	UFUNCTION(BlueprintCallable)
	void SetLoadingScreenBackgroundTexture(TSoftObjectPtr<UTexture2D> NewBackgroundTexture);
	
	// Returns the last set loading screen widget class to display inside of the loading screen widget host
	UFUNCTION(BlueprintPure)
	TSubclassOf<UUserWidget> GetLoadingScreenContentWidget() const;

	UFUNCTION(BlueprintPure)
	TSoftObjectPtr<UTexture2D> GetLoadingScreenBackgroundTexture() const;

private:
	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;
	
	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FLoadingScreenBackgroundTextureChangedDelegate OnLodingScreenBackgroundTextureChanged;
	
	UPROPERTY()
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	
	UPROPERTY()
	TSoftObjectPtr<UTexture2D> LoadingScrrenBackgroundImage;
};
