// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NaviTeamUserBoard.generated.h"

class UVerticalBox;
class UNaviUserInfoRow;
/**
 * 
 */
UCLASS()
class LYRAGAME_API UNaviTeamUserBoard : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
	TObjectPtr<UVerticalBox> UserInformations_VerticalBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNaviUserInfoRow> UserInfoRowClass;

	
	UFUNCTION(BlueprintCallable)
	void CreateAndAddUserInfoRow(const FString& UserName);
	
	UFUNCTION(BlueprintCallable)
	UNaviUserInfoRow* FindUserInfoRow(const FString& UserName);
	
	UFUNCTION(BlueprintCallable)
	void RemoveUserInfoRow(const FString& UserName);
	
};
