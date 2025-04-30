// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviHealthTextWidget.generated.h"

class ULyraHealthComponent;   
class UAbilitySystemComponent; 
struct FGameplayEffectSpec;   
/**
 * 
 */
UCLASS()
class NAVIGAME_API UNaviHealthTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override; 
	virtual void NativeDestruct() override;  
	//~ End UUserWidget Interface

	// Todo: BlueprintNativeFunction은 Delegate 등록이 안되는지?
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION(BlueprintImplementableEvent)
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION(BlueprintNativeEvent) 
	void HandleHealthChanged(ULyraHealthComponent* HealthComp, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent) 
	void HandleArmorChanged(ULyraHealthComponent* HealthComp, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent) 
	void HandleShieldChanged(ULyraHealthComponent* HealthComp, float OldValue, float NewValue, AActor* Instigator);
	
};
