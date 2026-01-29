// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerRow.h"
#include "NaviScoreBoard_PlayerRow.generated.h"

class UNaviScoreBoard_AgentIcon;
class UNaviScoreBoard_Name;
class UNaviScoreBoard_Ultimate;
class UNaviScoreBoard_KDA;
class UNaviScoreBoard_LoadOut;
class UNaviScoreBoard_Creds;
class UNaviScoreBoard_Ping;
class UHorizontalBox;
class UWidget;
class ALyraPlayerState;
/**
 *
 * 
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviScoreBoard_PlayerRow : public UBasePlayerRow
{
	GENERATED_BODY()
	
protected:
	
	//~ UBasePlayerRow 
	virtual TArray<UWidget*> GetPlayerStateAwareChildren() const override;
	virtual void SetPlayerState_Implementation(ALyraPlayerState* InPlayerState) override;
	//~ End of UBasePlayerRow
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox;
	
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


