// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "NaviSignInOverlay.generated.h"

class UJoinGame;
class UPortalManager;
class UWidgetSwitcher;

class UNaviConfirmSignUpPage;
class UNaviSuccessConfirmedPage;
class UButton;

class UNaviSignInPage;
class UNaviSignUpPage;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviSignInOverlay : public ULyraActivatableWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPortalManager> PortalManagerClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnSignInSucceeded();

private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviSignInPage> SignInPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviSignUpPage> SignUpPage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviConfirmSignUpPage> ConfirmSignUpPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviSuccessConfirmedPage> SuccessConfirmedPage;

	UPROPERTY()
	TObjectPtr<UPortalManager> PortalManager;

	UFUNCTION()
	void ShowSignInPage();

	UFUNCTION()
	void ShowSignUpPage();

	UFUNCTION()
	void ShowConfirmSignUpPage();

	UFUNCTION()
	void ShowSuccessConfirmedPage();

	UFUNCTION()
	void SignInButtonClicked();

	UFUNCTION()
	void SignUpButtonClicked();

	UFUNCTION()
	void ConfirmButtonClicked();

	UFUNCTION()
	void OnSignUpSucceeded();

	UFUNCTION()
	void OnConfirmSucceeded();
};
