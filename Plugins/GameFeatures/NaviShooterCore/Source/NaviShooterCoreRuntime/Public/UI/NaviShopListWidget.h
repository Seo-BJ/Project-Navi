// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "NaviShopListWidget.generated.h"

class ULyraButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopButtonHovered, ULyraButtonBase*, Button);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopButtonClicked, ULyraButtonBase*, Button);

/**
 * 상점의 아이템 버튼 목록을 관리하는 위젯입니다.
 * 하위 위젯 트리를 스캔하여 모든 LyraButtonBase 버튼의 이벤트를 캡처하고 외부로 전파합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviShopListWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	// 버튼에 마우스가 올라갔을 때 발생하는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Navi|Shop")
	FOnShopButtonHovered OnButtonHovered;

	// 버튼에서 마우스가 떨어졌을 때 발생하는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Navi|Shop")
	FOnShopButtonHovered OnButtonUnhovered;

	// 버튼이 클릭되었을 때 발생하는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Navi|Shop")
	FOnShopButtonClicked OnButtonClicked;

protected:
	void HandleButtonHovered(ULyraButtonBase* Button);
	void HandleButtonUnhovered(ULyraButtonBase* Button);
	void HandleButtonClicked(ULyraButtonBase* Button);
};
