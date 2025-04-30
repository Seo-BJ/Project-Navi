// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LyraGameplayTags.h"
#include "NaviAgentSelectionPanel.generated.h"

class ULyraButtonBase;
class UScrollBox;
class UGridPanel;
class UCommonTextBlock;
class UDataTable;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviAgentSelectionPanel : public UUserWidget
{
	GENERATED_BODY()

public:



protected:

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int MaxColumn = 4;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> AgentIconButtonClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentDescriptionData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> AllAgentClass_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> InitiatorClass_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> DuelistClass_Button;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> SentinelClass_Button;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> ControllerClass_Button;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> AgentIcons_ScrollBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGridPanel> AgentIcons_GridPanel;

	UFUNCTION(BlueprintImplementableEvent)
	void FillGridPanel();
	
private:

	FGameplayTag CurrentAgentTag;

	
};
