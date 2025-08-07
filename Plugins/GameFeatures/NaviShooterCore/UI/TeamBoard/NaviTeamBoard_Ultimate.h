// Copyright 2024, NAVER Z Corporation
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamBoard_Ultimate.generated.h"

class UCommonNumericTextBlock;

/**
 *
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviTeamBoard_Ultimate : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetUltimatePoints(int32 CurrentPoints, int32 MaxPoints);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> UltimatePoint_TextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> UltimateMaxPoint_TextBlock;
};
