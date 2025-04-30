// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "TeamSelectionIcon.generated.h"

class UCommonTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UTeamSelectionIcon : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

	void SerUserNameText(FString UserName);

protected:
	
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> UserName_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> AgentIcon_Image;
	
};
