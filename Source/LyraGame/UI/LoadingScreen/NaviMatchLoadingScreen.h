// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviMatchLoadingScreen.generated.h"

class UNaviTeamColumn;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviMatchLoadingScreen : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UNaviTeamColumn> Offensive_TeamColumn;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UNaviTeamColumn> Dffensive_TeamColumn;
	
	void PopulateTeamColumns();

	
};
