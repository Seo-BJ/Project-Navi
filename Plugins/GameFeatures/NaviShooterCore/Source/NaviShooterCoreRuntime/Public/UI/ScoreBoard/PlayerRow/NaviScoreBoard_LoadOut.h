// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ITakesLyraPlayerState.h"
#include "NaviScoreBoard_LoadOut.generated.h"

class UImage;
struct FDataRegistryAcquireResult;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard_LoadOut : public UUserWidget, public ITakesLyraPlayerState
{
	GENERATED_BODY()
	
public:
	//~ ITakesLyraPlayerState 
	virtual void SetPlayerState_Implementation(ALyraPlayerState* InPlayerState) override;
	//~ End of ITakesLyraPlayerState
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponIcon_Image;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ArmorIcon_Image;

	UPROPERTY(EditDefaultsOnly)
	float HeightOverride = 0.f;
	
private:
	void OnRegistryLoadCompleted(const FDataRegistryAcquireResult& AcquireResult);


	
};
