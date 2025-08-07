// Copyright 2024, NAVER Z Corporation
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamBoard_LoadOut.generated.h"

class UImage;

/**
 *
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviTeamBoard_LoadOut : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PrimaryWeaponIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SecondaryWeaponIcon;
};
