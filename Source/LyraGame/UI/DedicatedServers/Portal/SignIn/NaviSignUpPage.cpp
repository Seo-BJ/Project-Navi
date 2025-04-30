// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviSignUpPage.h"

#include "Components/EditableTextBox.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "UI/DedicatedServers/Portal/SignIn/NaviTextBoxBase.h"
#include "Components/TextBlock.h"


void UNaviSignUpPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	StatusMessage_TextBlock->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		SignUp_Button->SetIsEnabled(true);
	}
}

void UNaviSignUpPage::ClearTextBoxes()
{
	UserName_TextBox->TextBox->SetText(FText::GetEmpty());
	Password_TextBox->TextBox->SetText(FText::GetEmpty());
	ConfirmPassword_TextBox->TextBox->SetText(FText::GetEmpty());
	Email_TextBox->TextBox->SetText(FText::GetEmpty());
}

void UNaviSignUpPage::NativeConstruct()
{
	Super::NativeConstruct();

	UserName_TextBox->TextBox->OnTextChanged.AddDynamic(this, &UNaviSignUpPage::UpdateSignUpButtonState);
	Password_TextBox->TextBox->OnTextChanged.AddDynamic(this, &UNaviSignUpPage::UpdateSignUpButtonState);
	ConfirmPassword_TextBox->TextBox->OnTextChanged.AddDynamic(this, &UNaviSignUpPage::UpdateSignUpButtonState);
	Email_TextBox->TextBox->OnTextChanged.AddDynamic(this, &UNaviSignUpPage::UpdateSignUpButtonState);
	SignUp_Button->SetIsEnabled(false);
}

void UNaviSignUpPage::UpdateSignUpButtonState(const FText& Text)
{
	const bool bIsUsernameValid = !UserName_TextBox->TextBox->GetText().ToString().IsEmpty();
	const bool bArePasswordsEqual = Password_TextBox->TextBox->GetText().ToString() == ConfirmPassword_TextBox->TextBox->GetText().ToString();
	const bool bIsValiEmail = IsValidEmail(Email_TextBox->TextBox->GetText().ToString());
	const bool bIsPasswordLongEnough = Password_TextBox->TextBox->GetText().ToString().Len() >= 8;

	FString StatusMessage;
	const bool bIsStrongPassword = IsStrongPassword(Password_TextBox->TextBox->GetText().ToString(), StatusMessage);
	if (!bIsStrongPassword)
	{
		StatusMessage_TextBlock->SetText(FText::FromString(StatusMessage));
	}
	else if (!bIsUsernameValid)
	{
		StatusMessage_TextBlock->SetText(FText::FromString(TEXT("Please enter a valid Username.")));
	}
	else if (!bArePasswordsEqual)
	{
		StatusMessage_TextBlock->SetText(FText::FromString(TEXT("Please ensure that passwords match.")));
	}
	else if (!bIsValiEmail)
	{
		StatusMessage_TextBlock->SetText(FText::FromString(TEXT("Please enter a valid email.")));
	}
	else if (!bIsPasswordLongEnough)
	{
		StatusMessage_TextBlock->SetText(FText::FromString(TEXT("Passwords must be at least 8 characters.")));
	}
	else
	{
		StatusMessage_TextBlock->SetText(FText::GetEmpty());
	}
	SignUp_Button->SetIsEnabled(bIsUsernameValid && bArePasswordsEqual && bIsValiEmail && bIsStrongPassword);
}

bool UNaviSignUpPage::IsValidEmail(const FString& Email)
{
	const FRegexPattern EmailPattern(TEXT(R"((^[^\s@]+@[^\s@]+\.[^\s@]{2,}$))"));
	FRegexMatcher Matcher(EmailPattern, Email);
	return Matcher.FindNext();
}

bool UNaviSignUpPage::IsStrongPassword(const FString& Password, FString& StatusMessage)
{
	const FRegexPattern NumberPattern(TEXT(R"(\d)")); // contains at least one number
	const FRegexPattern SpecialCharPattern(TEXT(R"([^\w\s])")); // contains at least one special character
	const FRegexPattern UppercasePattern(TEXT(R"([A-Z])")); // contains at least one uppercase character
	const FRegexPattern LowercasePattern(TEXT(R"([a-z])")); // contains at least one lowercase character

	FRegexMatcher NumberMatcher(NumberPattern, Password);
	FRegexMatcher SpecialCharMatcher(SpecialCharPattern, Password);
	FRegexMatcher UppercaseMatcher(UppercasePattern, Password);
	FRegexMatcher LowercaseMatcher(LowercasePattern, Password);

	if (!NumberMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least 1 number.");
		return false;
	}
	if (!SpecialCharMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one special character.");
		return false;
	}
	if (!UppercaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one uppercase character.");
		return false;
	}
	if (!LowercaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least one lowercase character.");
		return false;
	}
	return true;
}








