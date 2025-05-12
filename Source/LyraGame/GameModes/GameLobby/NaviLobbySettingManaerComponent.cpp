// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviLobbySettingManaerComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"

#include "Gamemodes/LyraGameState.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UNaviLobbySettingManaerComponent::UNaviLobbySettingManaerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNaviLobbySettingManaerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(UNaviLobbySettingManaerComponent, MapTag);
	DOREPLIFETIME(UNaviLobbySettingManaerComponent, ModTag);
}

FGameplayTag UNaviLobbySettingManaerComponent::GetMapTag()
{
	return MapTag;
}

FGameplayTag UNaviLobbySettingManaerComponent::GetModTag()
{
	return ModTag;
}

void UNaviLobbySettingManaerComponent::HandleMapSelectionRequest(const FGameplayTag& SelectedMapTag)
{
	if (HasAuthority())
	{
		MapTag = SelectedMapTag;
	}
	
}

void UNaviLobbySettingManaerComponent::HandleModSelectionRequest(const FGameplayTag& SelectedModTag)
{
	if (HasAuthority())
	{
		ModTag = SelectedModTag;
	}
}

// Called when the game starts
void UNaviLobbySettingManaerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &UNaviLobbySettingManaerComponent::OnExperienceLoaded));
	
	
}


void UNaviLobbySettingManaerComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{

}

void UNaviLobbySettingManaerComponent::OnRep_MapTag(FGameplayTag OldTag)
{
	OnMapSettingTagChanged.Broadcast(OldTag, MapTag);
}

void UNaviLobbySettingManaerComponent::OnRep_ModTag(FGameplayTag OldTag)
{
	OnModSettingTagChanged.Broadcast(OldTag, ModTag);
}

