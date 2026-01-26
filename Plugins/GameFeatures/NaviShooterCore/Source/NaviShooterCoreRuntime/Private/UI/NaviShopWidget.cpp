// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NaviShopWidget.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "UI/NaviPurchaseButton.h"
#include "UI/NaviDescriptionContainer.h"
#include "UI/NaviShopListWidget.h"
#include "Inventory/LyraInventoryItemDefinition.h"

void UNaviShopWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// ShopListWidget의 이벤트를 구독합니다.
	if (ShopListWidget)
	{
		ShopListWidget->OnButtonHovered.AddDynamic(this, &ThisClass::HandleShopButtonHovered);
		ShopListWidget->OnButtonUnhovered.AddDynamic(this, &ThisClass::HandleShopButtonUnhovered);
		ShopListWidget->OnButtonClicked.AddDynamic(this, &ThisClass::HandleShopButtonClicked);
	}
}

void UNaviShopWidget::HandleShopButtonHovered(ULyraButtonBase* HoveredButton)
{
	if (!DescriptionContainer || !HoveredButton) return;

	// 버튼이 NaviPurchaseButton인지 확인하고, EquipmentTag를 가져옵니다.
	if (UNaviPurchaseButton* ShopButton = Cast<UNaviPurchaseButton>(HoveredButton))
	{
		if (const FGameplayTag& Tag = ShopButton->GetEquipmentTag(); Tag.IsValid())
		{
			// DescriptionContainer에 태그를 전달하여 데이터를 표시하게 합니다.
			DescriptionContainer->SetDataToDisplay(Tag);
			return;
		}
	}

	// NaviPurchaseButton이 아니거나 태그가 유효하지 않으면 숨김 처리
	DescriptionContainer->SetDataToDisplay(FGameplayTag::EmptyTag);
}

void UNaviShopWidget::HandleShopButtonUnhovered(ULyraButtonBase* UnhoveredButton)
{
	if (DescriptionContainer)
	{
		// 마우스를 떼면 설명창을 숨깁니다.
		// DescriptionContainer->SetDataToDisplay(FGameplayTag::EmptyTag);
	}
}

void UNaviShopWidget::HandleShopButtonClicked(ULyraButtonBase* ClickedButton)
{
	// 구매 로직 처리를 위해 블루프린트 이벤트를 호출합니다.
	BP_OnPurchaseAttempt(ClickedButton);
}
