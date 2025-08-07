// Copyright 2024, NAVER Z Corporation
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamBoard_KDA.generated.h"

class UCommonNumericTextBlock;

/**
 *
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviTeamBoard_KDA : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetKDA(int32 Kills, int32 Deaths, int32 Assists);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Kills_TextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Deaths_TextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Assists_TextBlock;
};
