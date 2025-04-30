// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "LyraGameplayTags.h"
#include "UI/Data/NaviUIDataTypes.h"

#include "NaviAgentDescriptionPanel.generated.h"

class ULyraButtonBase;
class UCommonTextBlock;
class UDataTable;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviAgentDescriptionPanel : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "UI Update")
	void UpdateAgentTextBlocks(const FGameplayTag& TargetAgentTag);

	UFUNCTION(BlueprintCallable, Category = "UI Update")
	void UpdateSkillTextBlocks(const FGameplayTag& TargetSkillTag);

	UFUNCTION(BlueprintCallable, Category = "UI Update")
	void UpdateClassDescriptionTextBlock();

protected:

	virtual void NativeConstruct() override;

  
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentSkillUIData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentDescriptionData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentClassData;



	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<ULyraButtonBase> AgentClass_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<ULyraButtonBase> CSkill_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<ULyraButtonBase> QSkill_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<ULyraButtonBase> ESkill_Button;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<ULyraButtonBase> XSkill_Button;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> AgentClass_TextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> AgentName_TextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> AgentDescription_TextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Name_TextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Description_TextBlock;


private:

	FGameplayTag CurrentAgentTag;
};
