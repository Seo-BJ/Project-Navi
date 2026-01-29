// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NaviLeaf.h"

#include "NaviLeaf_ArmorDisplay.generated.h"

struct FNaviArmorStatDefinition;
class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf_ArmorDisplay : public UNaviLeaf
{
	GENERATED_BODY()

public:
	virtual void UpdateArmorStats(const FNaviArmorStatDefinition& WeaponData) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ArmorName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Armor;
	
	
};
