// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "NaviShopWidget.generated.h"

class UNaviDescriptionContainer;
class UNaviShopListWidget;
class ULyraButtonBase;

/**
 * 게임 내 상점을 위한 메인 위젯 클래스입니다.
 * 아이템 목록(ShopListWidget)과 설명창(DescriptionContainer)을 조립하고 상호작용을 중개합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviShopWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

protected:
	// ShopListWidget에서 호버 이벤트가 발생했을 때 호출됩니다.
	UFUNCTION()
	void HandleShopButtonHovered(ULyraButtonBase* HoveredButton);

	// ShopListWidget에서 언호버 이벤트가 발생했을 때 호출됩니다.
	UFUNCTION()
	void HandleShopButtonUnhovered(ULyraButtonBase* UnhoveredButton);

	// ShopListWidget에서 클릭 이벤트가 발생했을 때 호출됩니다.
	UFUNCTION()
	void HandleShopButtonClicked(ULyraButtonBase* ClickedButton);

	// 버튼 클릭 시 트리거되는 블루프린트 이벤트입니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Navi|Shop")
	void BP_OnPurchaseAttempt(ULyraButtonBase* ClickedButton);

protected:
	// 아이템 설명 및 스탯을 표시하는 컨테이너 위젯입니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviDescriptionContainer> DescriptionContainer;

	// 상점 아이템 버튼들을 포함하고 관리하는 위젯입니다.
	// 위젯 블루프린트에서 이름을 "ShopListWidget"으로 설정해야 바인딩됩니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviShopListWidget> ShopListWidget;
};