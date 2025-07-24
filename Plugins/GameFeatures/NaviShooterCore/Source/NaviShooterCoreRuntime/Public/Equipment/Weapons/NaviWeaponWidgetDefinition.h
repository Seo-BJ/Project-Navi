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
	/** 무기 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	/** 무기를 식별하기 위한 게임플레이 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponTag;

	/** 킬 피드에 표시될 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTexture2D> KillFeedIcon;

	/** 일반 위젯에 표시될 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTexture2D> WidgetIcon;
};
