// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "NaviSignUpPage.generated.h"

class ULyraButtonBase;
class UNaviTextBoxBase;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviSignUpPage : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> UserName_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> Password_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> ConfirmPassword_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> Email_TextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> SignUp_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> Back_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusMessage_TextBlock;

	UFUNCTION()
	void UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets);

	void ClearTextBoxes();

protected:
	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void UpdateSignUpButtonState(const FText& Text);

	bool IsValidEmail(const FString& Email);
	bool IsStrongPassword(const FString& Password, FString& StatusMessage);
};
