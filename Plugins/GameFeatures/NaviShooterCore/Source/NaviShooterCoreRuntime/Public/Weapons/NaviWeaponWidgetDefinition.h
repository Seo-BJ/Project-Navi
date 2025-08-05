// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "NaviWeaponWidgetDefinition.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FNaviWeaponWidgetDefinition : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 무기 표시 이름 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	// 무기 식별 게임플레이 태그 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponTag;

	// KillFeed에서 사용하는 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTexture2D> KillFeedIcon;

	// 무기 아이콘 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTexture2D> WidgetIcon;
};
