// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/LyraActivatableWidget.h"
#include "NaviSuccessConfirmedPage.generated.h"

class ULyraButtonBase;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviSuccessConfirmedPage : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> Button_Ok;
};
