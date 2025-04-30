// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "NaviTextBoxBase.generated.h"

class UEditableTextBox;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviTextBoxBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox;
	
};
