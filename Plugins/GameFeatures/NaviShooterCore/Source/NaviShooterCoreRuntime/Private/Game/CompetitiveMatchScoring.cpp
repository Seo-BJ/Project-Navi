// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CompetitiveMatchScoring.h"

#include "LyraGameplayTags.h"
#include "Net/UnrealNetwork.h"
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
	DOREPLIFETIME(UCompetitiveMatchScoring, RemainingSpikeTime);
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
		// BuyingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPlayingPhaseStarted);
		
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

// Buying PHase /////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		// PlayingPhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
		PhaseSubsystem->StartPhase(StartingPhaseAbilityClass, PlayingPhaseEndDelegate);
		
		PlayingPhaseStartDelegate.Broadcast();

		RemainingTime = RoundInitialTime;
		GetWorld()->GetTimerManager().SetTimer(
			RoundTimerHandle,
			this,
			&UCompetitiveMatchScoring::PlayingTimeCountDown,
			1.0f,
			true);
	}
}
// End of Buying PHase /////////////////////////////////////////////////////////////////////////////////////////////////

void UCompetitiveMatchScoring::PlayingTimeCountDown()
{
	RemainingTime -= 1.0f;

	if (RemainingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
		HandlePostMatch();
	}
}

void UCompetitiveMatchScoring::HandleSpikePlanted(APawn* SpikePlanter)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	Multicast_BroadcastSpikePlantedMessage(SpikePlanter);

	RemainingSpikeTime = SpikeDetonationTime;
	GetWorld()->GetTimerManager().SetTimer(
		SpikePhaseTimerHandle,
		this,
		&UCompetitiveMatchScoring::SpikeTimeCountDown,
		1.0f,
		true);

	ULyraGamePhaseSubsystem* PhaseSubsystem = GetWorld()->GetSubsystem<ULyraGamePhaseSubsystem>();
	if (ensure(PhaseSubsystem) && ensure(SpikePlantedPhaseAbilityClass))
	{
		FLyraGamePhaseDelegate SpikePhaseEndDelegate;
		// 스파이크 페이즈가 끝나면(폭발하면) PostGame 페이즈로 갑니다.
		// SpikePhaseEndDelegate.BindUObject(this, &UCompetitiveMatchScoring::OnPostGamePhaseStarted);
		PhaseSubsystem->StartPhase(SpikePlantedPhaseAbilityClass, SpikePhaseEndDelegate);
	}
}
void UCompetitiveMatchScoring::Multicast_BroadcastSpikePlantedMessage_Implementation(APawn* SpikePlanter)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	FSpikePlantedMessage Message;
	Message.SpikePlantInstigator = SpikePlanter;

	MessageSubsystem.BroadcastMessage(NaviGameplayTags::Navi_Spike_Plant_Finish, Message);
}


void UCompetitiveMatchScoring::SpikeTimeCountDown()
{
	RemainingSpikeTime -= 1.0f;
	if (RemainingSpikeTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpikePhaseTimerHandle);
		HandlePostMatch();
	}
}



void UCompetitiveMatchScoring::OnPostGamePhaseStarted(const ULyraGamePhaseAbility* PhaseAbility)
{
	
}


void UCompetitiveMatchScoring::HandlePostMatch()
{

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(SpikePhaseTimerHandle);
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

void UCompetitiveMatchScoring::OnRep_RemainingSpikeTime()
{
	OnRemainingSpikeTimeChanged.Broadcast(RemainingSpikeTime);
}
