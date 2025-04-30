// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviSignInPage.h"

#include "UI/Foundation/LyraButtonBase.h"
#include "Components/TextBlock.h"

void UNaviSignInPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	StatusMessage_TextBlock->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		SignIn_Button->SetIsEnabled(true);
	}
}
