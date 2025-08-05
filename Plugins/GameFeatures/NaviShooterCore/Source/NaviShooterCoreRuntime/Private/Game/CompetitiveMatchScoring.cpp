// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CompetitiveMatchScoring.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h" 
#include "AbilitySystem/Phases/LyraGamePhaseAbility.h" 
#include "GameModes/LyraExperienceManagerComponent.h"




UCompetitiveMatchScoring::UCompetitiveMatchScoring(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
}


void UCompetitiveMatchScoring::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCompetitiveMatchScoring, TargetScore);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingTime);
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingBuyingTime); 

}


void UCompetitiveMatchScoring::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
		
	}
}


void UCompetitiveMatchScoring::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
	if (!HasAuthority())
	{
		return;
	}

	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(BuyingPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate BuyingPhaseEndDelegate;
		BuyingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPlayingPhaseStarted);
		PhaseSubsystem->StartPhase(BuyingPhaseAbilityClass, BuyingPhaseEndDelegate);

		RemainingBuyingTime = BuyingTime;
		GetWorld()->GetTimerManager().SetTimer(
			BuyingPhaseTimerHandle,
			this,
			&UCompetitiveMatchScoring::BuyingTimeCountDown,
			1.0f,
			true);
	}
}


void UCompetitiveMatchScoring::BuyingTimeCountDown()
{
	RemainingBuyingTime -= 1.0f;

	if (RemainingBuyingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(BuyingPhaseTimerHandle);
		EndBuyingPhase();
	}
}

void UCompetitiveMatchScoring::EndBuyingPhase()
{
	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(StartingPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate PlayingPhaseEndDelegate;
		PlayingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
		PhaseSubsystem->StartPhase(StartingPhaseAbilityClass, PlayingPhaseEndDelegate);
	}
}


void UCompetitiveMatchScoring::OnPlayingPhaseStarted(const ULyraGamePhaseAbility* PhaseAbility)
{
	RemainingTime = RoundInitialTime;

	GetWorld()->GetTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&UCompetitiveMatchScoring::CountDown,
		1.0f,
		true);
}

void UCompetitiveMatchScoring::CountDown()
{
	RemainingTime -= 1.0f;

	if (RemainingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
		HandlePostMatch();
	}
}

void UCompetitiveMatchScoring::OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility)
{
	
}


void UCompetitiveMatchScoring::HandlePostMatch()
{

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
}


void UCompetitiveMatchScoring::OnRep_TargetScore()
{

}

void UCompetitiveMatchScoring::OnRep_RemainingTime()
{
	OnRemainingTimeChanged.Broadcast(RemainingTime);
}

void UCompetitiveMatchScoring::OnRep_RemainingBuyingTime()
{
	OnRemainingBuyingTimeChanged.Broadcast(RemainingBuyingTime);
}
