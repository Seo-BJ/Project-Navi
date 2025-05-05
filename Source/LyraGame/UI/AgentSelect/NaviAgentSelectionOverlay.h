// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "LyraGameplayTags.h"
#include "UI/Data/NaviUIDataTypes.h"

#include "NaviAgentSelectionOverlay.generated.h"

class UCommonTextBlock;
class UDataTable;

class UNaviAgentDescriptionPanel;
class UNaviAgentSelectionPanel;
class UTeamSelectionPanel;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviAgentSelectionOverlay : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:


protected:

    virtual void NativeConstruct() override;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
    TObjectPtr<UDataTable> AgentSkillUIData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentDescriptionData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentClassData;



	

private:

	FGameplayTag CurrentAgentTag;
	
	
};
