// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameActor/NaviPlayerBlockWall.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h"
#include "LyraGameplayTags.h"
#include "Engine/World.h"
#include "Game/CompetitiveMatchScoring.h"
#include "GameModes/LyraExperienceManagerComponent.h"

ANaviPlayerBlockWall::ANaviPlayerBlockWall()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    SetRootComponent(CollisionBox);
    CollisionBox->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName); 
    CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); 

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(CollisionBox);

}

void ANaviPlayerBlockWall::BeginPlay()
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

void ANaviPlayerBlockWall::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        if (!IsValid(World)) return;
        
        AGameStateBase* GameState = World->GetGameState();
        if (!IsValid(GameState)) return;
        
        UCompetitiveMatchScoring* MatchScoring = GameState->GetComponentByClass<UCompetitiveMatchScoring>();
        if (IsValid(MatchScoring))
        {
            MatchScoring->PlayingPhaseStartDelegate.AddDynamic(this, &ANaviPlayerBlockWall::OnPlayingPhaseStarted);
        }
    }
}

void ANaviPlayerBlockWall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        if (!IsValid(World)) return;
    
        AGameStateBase* GameState = World->GetGameState();
        if (!IsValid(GameState)) return;
    
        UCompetitiveMatchScoring* MatchScoring = GameState->GetComponentByClass<UCompetitiveMatchScoring>();
        if (IsValid(MatchScoring))
        {
            MatchScoring->PlayingPhaseStartDelegate.RemoveDynamic(this, &ANaviPlayerBlockWall::OnPlayingPhaseStarted);
        }
    }
}


void ANaviPlayerBlockWall::OnPlayingPhaseStarted()
{
    if (HasAuthority())
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Destroy();
    }
}
