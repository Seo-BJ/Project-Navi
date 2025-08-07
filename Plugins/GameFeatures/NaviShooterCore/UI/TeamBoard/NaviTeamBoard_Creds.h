// Copyright 2024, NAVER Z Corporation
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamBoard_Creds.generated.h"

class UImage;
class UCommonNumericTextBlock;

/**
 *
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviTeamBoard_Creds : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetCreds(int32 NewCreds);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CredsIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> CredsAmount_TextBlock;
};
