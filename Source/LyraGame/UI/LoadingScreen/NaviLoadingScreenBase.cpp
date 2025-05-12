// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviLoadingScreenBase.h"

#include "Engine/Texture2D.h"
#include "Components/Image.h"

void UNaviLoadingScreenBase::UpdateLoadingScreenBackgroundImage(TSoftObjectPtr<UTexture2D> BackgroundTexture)
{
	if (BackgroundTexture.IsNull()) return;
	LoadingScreenBackgroundImage->SetBrushFromSoftTexture(BackgroundTexture, false);
}
