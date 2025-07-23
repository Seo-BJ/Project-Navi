// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviHealthAndArmorText.generated.h"

class APawn;
class UCommonNumericTextBlock;
class ULyraHealthComponent;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviHealthAndArmorText : public UUserWidget
{
	GENERATED_BODY()

public:

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> HealthAmount_TextBlock;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	TObjectPtr<UCommonNumericTextBlock> ArmorAmount_TextBlock;

private:
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnHealthChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void OnArmorChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

};
