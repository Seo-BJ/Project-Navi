// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "GameplayTagContainer.h"
#include "NaviPurchaseButton.generated.h"

/**
 * 상점에서 장비 구매를 위해 사용되는 버튼 클래스입니다.
 * 리플렉션 대신 명시적인 GameplayTag를 사용하여 아이템을 식별합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviPurchaseButton : public ULyraButtonBase
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetEquipmentTag() const {return EquipmentTag;}
	
protected:
	// 이 버튼이 나타내는 장비의 태그입니다. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navi|Shop")
	FGameplayTag EquipmentTag;
};
