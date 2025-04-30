// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DedicatedServers/GameSessionScreen.h"

#include "DedicatedServers/Public/UI/GameSessions/GameSessionsManager.h"

void UGameSessionScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	GameSessionsManager = NewObject<UGameSessionsManager>(this, GameSessionsManagerClass);
	
}

void UGameSessionScreen::JoinGameSession()
{
	if (GameSessionsManager)
	{
		GameSessionsManager->JoinGameSession();
	}
}
