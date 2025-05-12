// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviAgentIcon.generated.h"

class UImage;
class UTexture2D;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviAgentIcon : public UUserWidget
{
	GENERATED_BODY()

public:

	void UpdateAgentIcomImage(TSoftObjectPtr<UTexture2D> AgentImage);
	
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AgentIcon_Image;
};
