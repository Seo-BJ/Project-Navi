// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NaviShopListWidget.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "Blueprint/WidgetTree.h"

void UNaviShopListWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree)
	{
		// 위젯 트리를 순회하며 모든 하위 위젯을 확인합니다.
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			// LyraButtonBase 타입(또는 이를 상속받은 블루프린트)인 경우 이벤트를 바인딩합니다.
			if (ULyraButtonBase* Button = Cast<ULyraButtonBase>(Widget))
			{
				// 호버(마우스 올림) 이벤트 바인딩
				Button->OnHovered().AddWeakLambda(this, [this, Button]()
				{
					HandleButtonHovered(Button);
				});

				// 언호버(마우스 뗌) 이벤트 바인딩
				Button->OnUnhovered().AddWeakLambda(this, [this, Button]()
				{
					HandleButtonUnhovered(Button);
				});

				// 클릭 이벤트 바인딩
				Button->OnClicked().AddWeakLambda(this, [this, Button]()
				{
					HandleButtonClicked(Button);
				});
			}
		});
	}
}

void UNaviShopListWidget::HandleButtonHovered(ULyraButtonBase* Button)
{
	OnButtonHovered.Broadcast(Button);
}

void UNaviShopListWidget::HandleButtonUnhovered(ULyraButtonBase* Button)
{
	OnButtonUnhovered.Broadcast(Button);
}

void UNaviShopListWidget::HandleButtonClicked(ULyraButtonBase* Button)
{
	OnButtonClicked.Broadcast(Button);
}
