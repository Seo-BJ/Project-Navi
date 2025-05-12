// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviUserSelectionRow.generated.h"

class UImage;
class UCommonTextBlock;
class UNaviAgentIcon;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviUserSelectionRow : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void UpdateUserSelectionRow(TSoftObjectPtr<UTexture2D>  AgentImage, const FText& AgentName, const FString& UserName);

	UFUNCTION(BlueprintCallable)
	void UpdateHorizontalAlignment(bool bOffensiveTeam);
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviAgentIcon> AgentIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> UserName_Text;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> AgentName_Text;

};

