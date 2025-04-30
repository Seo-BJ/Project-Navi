// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviUserInfoRow.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviUserInfoRow : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> UserName_Text;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> UserPing_text;

	UFUNCTION(BlueprintCallable)
	void SetUserNameText(FString UserName);

	UFUNCTION(BlueprintCallable)
	void SetUserPingText(int PingMs);

	UFUNCTION(BlueprintCallable)
	FString GetInUserName() {return InUserName;};

protected:
	
	UPROPERTY(BlueprintReadOnly)
	FString InUserName;
	
private:
	

};
