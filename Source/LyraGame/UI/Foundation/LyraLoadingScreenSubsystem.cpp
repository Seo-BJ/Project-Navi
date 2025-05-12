// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Foundation/LyraLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLoadingScreenSubsystem)

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// ULyraLoadingScreenSubsystem

ULyraLoadingScreenSubsystem::ULyraLoadingScreenSubsystem()
{
}

void ULyraLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

void ULyraLoadingScreenSubsystem::SetLoadingScreenBackgroundTexture(TSoftObjectPtr<UTexture2D> NewBackgroundTexture)
{
	if (LoadingScrrenBackgroundImage != NewBackgroundTexture)
	{
		LoadingScrrenBackgroundImage = NewBackgroundTexture;

		OnLodingScreenBackgroundTextureChanged.Broadcast(LoadingScrrenBackgroundImage);
	}
}

TSubclassOf<UUserWidget> ULyraLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}

TSoftObjectPtr<UTexture2D> ULyraLoadingScreenSubsystem::GetLoadingScreenBackgroundTexture() const
{
	return LoadingScrrenBackgroundImage;
}

