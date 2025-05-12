// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LyraGameplayTags.h"
#include "NaviTeamColumn.generated.h"


class UNaviUserSelectionRow;
class UVerticalBox;
class ALyraPlayerState;
struct FNaviAgentDescriptionData;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviTeamColumn : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bOffensiveTeam;
	
protected:

	virtual void NativeConstruct() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNaviUserSelectionRow> UserSelectionRowClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> UserSelections_VerticalBox;

	void PopulateUserSelectionRow();

	void CreateUserSelectionRows(TArray<ALyraPlayerState*> Teammates);
	FNaviAgentDescriptionData* GetAgentDescriptionData(const FGameplayTag& TargetAgentTag);

		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data") 
	TObjectPtr<UDataTable> AgentDescriptionData;
};
