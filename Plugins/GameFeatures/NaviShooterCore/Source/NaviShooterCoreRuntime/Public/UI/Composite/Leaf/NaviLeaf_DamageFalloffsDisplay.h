// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/Leaf/NaviLeaf.h"
#include "NaviLeaf_DamageFalloffsDisplay.generated.h"

struct FDamageFalloff;
class UNaviWidget_DamageColumn;
class UPanelWidget;

/**
 * 무기의 거리별 데미지 감소 정보를 표시하는 리프 위젯입니다.
 * 내부적으로 서브 위젯(Column)들을 동적으로 생성하여 배치합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf_DamageFalloffsDisplay : public UNaviLeaf
{
	GENERATED_BODY()

public:
	// Raw Data(배열)를 받아 UI를 갱신합니다.
	void SetFalloffData(const TArray<FDamageFalloff>& Falloffs);

	//~ Begin UNaviCompositeBase Interface
	virtual void UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData) override;
	//~ End UNaviCompositeBase Interface

protected:
	// 서브 위젯들을 담을 컨테이너 패널 (HorizontalBox 등)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ColumnContainer;

	// 서브 위젯 클래스 (BP에서 지정)
	UPROPERTY(EditAnywhere, Category = "Navi|DamageDisplay")
	TSubclassOf<UNaviWidget_DamageColumn> ColumnWidgetClass;
};
