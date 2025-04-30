// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "LyraGameplayTags.h"
#include "TeamSelectionPanel.generated.h"

class UHorizontalBox;
class ULyraButtonBase;
class UCommonTextBlock;
class UDataTable;
class UTeamSelectionIcon;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UTeamSelectionPanel : public ULyraActivatableWidget
{
	GENERATED_BODY()
	
public:


protected:

	virtual void NativeConstruct() override;
	
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TeamAgentIcons_HorizontalBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<ULyraButtonBase> ConfirmSelection_Button;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTeamSelectionIcon> Own_SelectionPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTeamSelectionIcon> First_SelectionPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTeamSelectionIcon> Second_SelectionPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTeamSelectionIcon> Third_SelectionPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTeamSelectionIcon> Fourth_SelectionPanel;
	
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTeamSelectionIcon> TeamSelectionIconClass;
	
	//void SetPlayer
	void PopulateTeamSelectionIcons();

private:

	FGameplayTag CurrentAgentTag;
};
