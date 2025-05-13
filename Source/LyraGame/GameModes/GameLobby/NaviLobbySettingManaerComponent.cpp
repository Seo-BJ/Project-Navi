// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviLobbySettingManaerComponent.h"

#include "OnlineSubsystemUtils.h"
#include "GameModes/LyraExperienceManagerComponent.h"

#include "GameModes/Definition/NaviExperienceDefinition.h"

#include "GameModes/Definition/NaviMapDefinition.h"

#include "GameModes/Definition/NaviHostingRequestSubsystem.h"

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
    
	DOREPLIFETIME(UNaviLobbySettingManaerComponent, MapDefinition);
	DOREPLIFETIME(UNaviLobbySettingManaerComponent, ExperienceDefinition);
}

void UNaviLobbySettingManaerComponent::HandleMapSelectionRequest(UNaviMapDefinition* SelectedMapDefinition)
{
	if (HasAuthority())
	{   
		MapDefinition = SelectedMapDefinition;
	}
}

void UNaviLobbySettingManaerComponent::HandleExperienceSelectionRequest(UNaviExperienceDefinition* SeletedExperienceDefinition)
{
	if (HasAuthority())
	{
		ExperienceDefinition = SeletedExperienceDefinition;
	}
}

void UNaviLobbySettingManaerComponent::SaveSelectedExperienceAndMapDefinition()
{
	UWorld* World = GetOwner()->GetWorld(); 
	if (!World) return;

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return;
	
	UNaviHostingRequestSubsystem* HostingSubsystem = GameInstance->GetSubsystem<UNaviHostingRequestSubsystem>();
	if (!HostingSubsystem) return;
	
	if (MapDefinition && ExperienceDefinition)
	{
		HostingSubsystem->SaveSelectedMapAndExperienceDefinition(ExperienceDefinition, MapDefinition);
	}
}

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

void UNaviLobbySettingManaerComponent::OnRep_MapDefinition()
{
	OnMapDefinitionChanged.Broadcast(MapDefinition);
}

void UNaviLobbySettingManaerComponent::OnRep_ExperienceDefinition()
{
	OnExperienceDefnitionChanged.Broadcast(ExperienceDefinition);
}

