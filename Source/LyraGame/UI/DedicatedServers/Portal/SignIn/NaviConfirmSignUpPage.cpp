// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviConfirmSignUpPage.h"

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "UI/DedicatedServers/Portal/SignIn/NaviTextBoxBase.h"


void UNaviConfirmSignUpPage::ClearTextBoxes()
{
	TextBox_ConfirmationCode->TextBox->SetText(FText::GetEmpty());
	TextBlock_StatusMessage->SetText(FText::GetEmpty());
	TextBlock_Destination->SetText(FText::GetEmpty());
}

void UNaviConfirmSignUpPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		Confirm_Button->SetIsEnabled(true);
	}
}

void UNaviConfirmSignUpPage::NativeConstruct()
{
	Super::NativeConstruct();

	TextBox_ConfirmationCode->TextBox->OnTextChanged.AddDynamic(this, &UNaviConfirmSignUpPage::UpdateConfirmButtonState);
	Confirm_Button->SetIsEnabled(false);
}

void UNaviConfirmSignUpPage::UpdateConfirmButtonState(const FText& Text)
{
	const FRegexPattern SixDigitsPattern(TEXT(R"(^\d{6}$)"));
	FRegexMatcher Matcher(SixDigitsPattern, Text.ToString());

	const bool bValidConfirmationCode = Matcher.FindNext();

	Confirm_Button->SetIsEnabled(bValidConfirmationCode);
	if (bValidConfirmationCode)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(""));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Please enter six numerical digits."));
	}
}




