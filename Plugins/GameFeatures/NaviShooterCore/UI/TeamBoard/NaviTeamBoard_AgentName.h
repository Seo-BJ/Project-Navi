// Copyright 2024, NAVER Z Corporation
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamBoard_AgentName.generated.h"

class UCommonTextBlock;

/**
 *
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviTeamBoard_AgentName : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAgentName(const FText& NewName);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> AgentName_TextBlock;
};
