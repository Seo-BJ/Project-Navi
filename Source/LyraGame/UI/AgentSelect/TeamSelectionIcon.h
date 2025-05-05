// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "TeamSelectionIcon.generated.h"

class UCommonTextBlock;
class UTexture2D;
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

	// @Todo: C++로 이동
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSelectionIcon(UTexture2D* AgentIconTexture, const FText& AgentName) const;

protected:
	
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> UserName_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SelectedAgentName_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> SelectedAgentIcon_Image;

private:
	
	FString InUserName;
};
