// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "NaviSignInPage.generated.h"

class ULyraButtonBase;
class UNaviTextBoxBase;
class UTextBlock;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviSignInPage : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> UserName_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> Password_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> SignIn_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> SignUp_Button;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusMessage_TextBlock;

	UFUNCTION()
	void UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets);
};
