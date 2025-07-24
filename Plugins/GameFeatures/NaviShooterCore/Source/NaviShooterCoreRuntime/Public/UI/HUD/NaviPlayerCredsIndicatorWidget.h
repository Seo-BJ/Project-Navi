// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "NaviPlayerCredsIndicatorWidget.generated.h"

class UCommonNumericTextBlock;
class UImage;
struct FGameplayEffectSpec;

/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviPlayerCredsIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Navi|Creds")
	void SetCreds(float NewCreds);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void HandleCredsChanged(AActor* Instigator, AActor* Causer,
		const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	
private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CredsIcon_Image;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> Creds_TextBlock;
	
	FDelegateHandle CredsChangedDelegateHandle;
};
