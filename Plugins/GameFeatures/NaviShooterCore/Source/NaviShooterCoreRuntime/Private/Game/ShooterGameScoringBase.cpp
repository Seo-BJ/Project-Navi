// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameScoringBase.h"

#include "Messages/LyraVerbMessage.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Player/LyraPlayerState.h"
#include "LyraGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Elimination_Message, "Lyra.Elimination.Message");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Assist_Message, "Lyra.Assist.Message");

UShooterGameScoringBase::UShooterGameScoringBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShooterGameScoringBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		
		MessageSubsystem.RegisterListener(TAG_Lyra_Elimination_Message,this,
			&ThisClass::OnEliminationMessageReceived);

		MessageSubsystem.RegisterListener(TAG_Lyra_Assist_Message,this,
			&ThisClass::OnAssistMessageReceived);
		
	}
}

void UShooterGameScoringBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*
	// Unregister the listener when this component is destroyed to prevent memory leaks adnd crashes.
	if (EliminationListenerHandle.IsValid())
	{
		if (UGameplayMessageSubsystem* MessageSubsystem = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(GetGameInstance()))
		{
			MessageSubsystem->UnregisterListener(EliminationListenerHandle);
		}
	}
	*/
	Super::EndPlay(EndPlayReason);
}

void UShooterGameScoringBase::OnEliminationMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message)
{
	if (!Message.Instigator || !Message.Target)
	{
		return;
	}

	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!TeamSubsystem)
	{
		return;
	}
	
	ELyraTeamComparison TeamComparison = TeamSubsystem->CompareTeams(Message.Instigator, Message.Target);
	if (TeamComparison == ELyraTeamComparison::DifferentTeams)
	{
		if (ALyraPlayerState* InstigatorPS = Cast<ALyraPlayerState>(Message.Instigator))
		{
			InstigatorPS->AddStatTagStack(NaviGameplayTags::Navi_Score_Eliminations, 1);
		}

		if (ALyraPlayerState* TargetPS = Cast<ALyraPlayerState>(Message.Target))
		{
			TargetPS->AddStatTagStack(NaviGameplayTags::Navi_Score_Eliminations, 1);
		}
	}
}

void UShooterGameScoringBase::OnAssistMessageReceived(FGameplayTag Tag, const FLyraVerbMessage& Message)
{
	if (!Message.Instigator)
	{
		return;
	}

	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	if (!TeamSubsystem)
	{
		return;
	}
	

	if (ALyraPlayerState* InstigatorPS = Cast<ALyraPlayerState>(Message.Instigator))
	{
		InstigatorPS->AddStatTagStack(NaviGameplayTags::Navi_Score_Assists, 1);
	}

}

void UShooterGameScoringBase::ResetAllActivePlayers()
{
}

void UShooterGameScoringBase::PostWarmUp()
{
}


