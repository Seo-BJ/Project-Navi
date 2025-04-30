// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/LyraActivatableWidget.h"
#include "NaviConfirmSignUpPage.generated.h"

class ULyraButtonBase;
class UNaviTextBoxBase;
class UTextBlock;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviConfirmSignUpPage : public ULyraActivatableWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviTextBoxBase> TextBox_ConfirmationCode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> Confirm_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> Back_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Destination;

	void ClearTextBoxes();

	UFUNCTION()
	void UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets);
protected:
	virtual void NativeConstruct() override;
private:

	UFUNCTION()
	void UpdateConfirmButtonState(const FText& Text);
};
