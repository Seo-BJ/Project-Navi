// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameActor/NaviPlayerBlockWall.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "AbilitySystem/Phases/LyraGamePhaseSubsystem.h"
#include "LyraGameplayTags.h"
#include "Engine/World.h"
#include "Game/CompetitiveMatchScoring.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "AbilitySystem/Phases/LyraGamePhaseAbility.h"


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
        if (ULyraGamePhaseSubsystem* PhaseSubsystem = World->GetSubsystem<ULyraGamePhaseSubsystem>())
        {
            if (TargetPhaseTag.IsValid())
            {
                // 1. 델리게이트 생성
                FLyraGamePhaseTagDelegate PhaseDelegate = FLyraGamePhaseTagDelegate::CreateUObject(this, &ThisClass::OnGamePhaseChanged);

                // 2. 서브시스템에 등록 (시작되었거나 이미 진행 중이면 호출)
                PhaseSubsystem->WhenPhaseStartsOrIsActive(TargetPhaseTag, EPhaseTagMatchType::ExactMatch, PhaseDelegate);
            }
        }
    }
}

void ANaviPlayerBlockWall::OnGamePhaseChanged(const FGameplayTag& PhaseTag)
{
    if (HasAuthority())
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Destroy();
    }
}