// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"

#include "NaviScoreBoard.generated.h"

class UNaviScoreBoard_AgentIcon;
class UNaviScoreBoard_Name;
class UNaviScoreBoard_Ultimate;
class UNaviScoreBoard_KDA;
class UNaviScoreBoard_LoadOut;
class UNaviScoreBoard_Creds;
class UNaviScoreBoard_Ping;
class ALyraPlayerState;
/**
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard : public ULyraActivatableWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_AgentIcon> NaviScoreBoard_AgentIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_Name> NaviScoreBoard_Name;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_Ultimate> NaviScoreBoard_Ultimate;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_KDA> NaviScoreBoard_KDA;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_LoadOut> NaviScoreBoard_LoadOut;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_Creds> NaviScoreBoard_Creds;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNaviScoreBoard_Ping> NaviScoreBoard_Ping;
};
