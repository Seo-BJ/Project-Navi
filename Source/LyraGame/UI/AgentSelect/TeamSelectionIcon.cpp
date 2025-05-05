// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamSelectionIcon.h"
#include "Components/Image.h"
#include "CommonTextBlock.h"


void UTeamSelectionIcon::NativeConstruct()
{
	Super::NativeConstruct();
}


void UTeamSelectionIcon::SerUserNameText(FString UserName)
{
	check(UserName_TextBlock);
	UserName_TextBlock->SetText(FText::FromString(UserName));
	InUserName = UserName;
}
