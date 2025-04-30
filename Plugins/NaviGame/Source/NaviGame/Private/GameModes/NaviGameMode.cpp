// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NaviGameMode.h"

#include "Character/LyraCharacter.h"
#include "GameModes/LyraGameState.h"
#include "Player/LyraPlayerState.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraGameSession.h"
#include "UI/LyraHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NaviGameMode)

ANaviGameMode::ANaviGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GameStateClass = ALyraGameState::StaticClass();
	PlayerControllerClass = ALyraPlayerController::StaticClass();
	PlayerStateClass = ALyraPlayerState::StaticClass();
	DefaultPawnClass = ALyraCharacter::StaticClass();
}
