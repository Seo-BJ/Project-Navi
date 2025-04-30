// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviUserInfoRow.h"

#include "CommonTextBlock.h"


void UNaviUserInfoRow::SetUserNameText(FString UserName)
{
	UserName_Text->SetText(FText::FromString(UserName));
	InUserName = UserName;;
	
}

void UNaviUserInfoRow::SetUserPingText(int PingMs)
{
	UserName_Text->SetText(FText::FromString(FString::FromInt(PingMs)));
}
