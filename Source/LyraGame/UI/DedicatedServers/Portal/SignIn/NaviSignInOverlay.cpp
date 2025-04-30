// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviSignInOverlay.h"


#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

#include "UI/Portal/PortalManager.h"
#include "UI/API/GameSessions/JoinGame.h"

#include "NaviConfirmSignUpPage.h"
#include "NaviSignInPage.h"
#include "NaviSignUpPage.h"
#include "NaviSuccessConfirmedPage.h"
#include "NaviTextBoxBase.h"


#include "UI/Foundation/LyraButtonBase.h"

void UNaviSignInOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	check(PortalManagerClass);
	
	PortalManager = NewObject<UPortalManager>(this, PortalManagerClass);
	
	SignInPage->SignIn_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::SignInButtonClicked);
	SignInPage->SignUp_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::ShowSignUpPage);
	// SignInPage->SignIn_Button->OnClicked().AddUObject(PortalManager, &UPortalManager::QuitGame);
	PortalManager->SignInStatusMessageDelegate.AddDynamic(SignInPage, &UNaviSignInPage::UpdateStatusMessage);	
	
	SignUpPage->Back_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::UNaviSignInOverlay::ShowSignInPage);
	SignUpPage->SignUp_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::SignUpButtonClicked);
	PortalManager->SignUpStatusMessageDelegate.AddDynamic(SignUpPage, &UNaviSignUpPage::UpdateStatusMessage);
	PortalManager->OnSignUpSucceeded.AddDynamic(this, &UNaviSignInOverlay::OnSignUpSucceeded);
	
	ConfirmSignUpPage->Confirm_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::ConfirmButtonClicked);
	ConfirmSignUpPage->Back_Button->OnClicked().AddUObject(this, &UNaviSignInOverlay::ShowSignUpPage);
	PortalManager->OnConfirmSucceeded.AddDynamic(this, &UNaviSignInOverlay::OnConfirmSucceeded);
	PortalManager->ConfirmStatusMessageDelegate.AddDynamic(ConfirmSignUpPage, &UNaviConfirmSignUpPage::UpdateStatusMessage);
	
	SuccessConfirmedPage->Button_Ok->OnClicked().AddUObject(this, &UNaviSignInOverlay::ShowSignInPage);

	PortalManager->OnSignInSucceeded.AddDynamic(this, &UNaviSignInOverlay::OnSignInSucceeded);
}

void UNaviSignInOverlay::ShowSignInPage()
{
	WidgetSwitcher->SetActiveWidget(SignInPage);
}

void UNaviSignInOverlay::ShowSignUpPage()
{
	WidgetSwitcher->SetActiveWidget(SignUpPage);
}

void UNaviSignInOverlay::ShowConfirmSignUpPage()
{
	WidgetSwitcher->SetActiveWidget(ConfirmSignUpPage);
}

void UNaviSignInOverlay::ShowSuccessConfirmedPage()
{
	WidgetSwitcher->SetActiveWidget(SuccessConfirmedPage);
}

void UNaviSignInOverlay::SignInButtonClicked()
{
	const FString Username = SignInPage->UserName_TextBox->TextBox->GetText().ToString();
	const FString Password = SignInPage->Password_TextBox->TextBox->GetText().ToString();
	PortalManager->SignIn(Username, Password);
}

void UNaviSignInOverlay::SignUpButtonClicked()
{
	const FString Username = SignUpPage->UserName_TextBox->TextBox->GetText().ToString();
	const FString Password = SignUpPage->Password_TextBox->TextBox->GetText().ToString();
	const FString Email = SignUpPage->Email_TextBox->TextBox->GetText().ToString();
	PortalManager->SignUp(Username, Password, Email);
}

void UNaviSignInOverlay::ConfirmButtonClicked()
{
	const FString ConfirmationCode = ConfirmSignUpPage->TextBox_ConfirmationCode->TextBox->GetText().ToString();
	ConfirmSignUpPage->Confirm_Button->SetIsEnabled(false);
	PortalManager->Confirm(ConfirmationCode);
}

void UNaviSignInOverlay::OnSignUpSucceeded()
{
	SignUpPage->ClearTextBoxes();
	ConfirmSignUpPage->TextBlock_Destination->SetText(FText::FromString(PortalManager->LastSignUpResponse.CodeDeliveryDetails.Destination));
	ShowConfirmSignUpPage();
}

void UNaviSignInOverlay::OnConfirmSucceeded()
{
	ConfirmSignUpPage->ClearTextBoxes();
	ShowSuccessConfirmedPage();
}



