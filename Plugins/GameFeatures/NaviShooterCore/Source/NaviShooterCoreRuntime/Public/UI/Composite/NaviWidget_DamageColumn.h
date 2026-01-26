// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapons/NaviWeaponStatDefinition.h" // FDamageFalloff 정의 필요
#include "NaviWidget_DamageColumn.generated.h"

class UTextBlock;

/**
 * 거리별 데미지 정보의 한 열(Column)을 표시하는 위젯입니다.
 * 거리 범위(Range), 헤드/바디/레그 데미지를 표시합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviWidget_DamageColumn : public UUserWidget
{
	GENERATED_BODY()

public:
	// 데이터를 받아 UI를 갱신합니다.
	void SetFalloffData(const FDamageFalloff& Data, bool bIsLastRange = false);

protected:
	// 거리 범위 텍스트 (예: "0 ~ 10m")
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Range;

	// 헤드샷 데미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_HeadDamage;

	// 바디샷 데미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BodyDamage;

	// 다리샷 데미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LegDamage;
};
