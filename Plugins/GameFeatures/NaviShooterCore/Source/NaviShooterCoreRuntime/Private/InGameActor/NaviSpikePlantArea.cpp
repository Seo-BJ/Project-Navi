// Fill out your copyright notice in the Description page of Project Settings.

#include "InGameActor/NaviSpikePlantArea.h"

#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include "LyraGameplayTags.h"

ANaviSpikePlantArea::ANaviSpikePlantArea()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
    SetRootComponent(OverlapBox);

    bIsPlantable = false;
}

void ANaviSpikePlantArea::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ANaviSpikePlantArea::OnOverlapBegin);
        OverlapBox->OnComponentEndOverlap.AddDynamic(this, &ANaviSpikePlantArea::OnOverlapEnd);
        // CheckIfGrounded();
    }
}

void ANaviSpikePlantArea::CheckIfGrounded()
{
/*
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    FHitResult HitResult;
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.0f);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
    {
        float GroundSlope = FMath::Acos(FVector::DotProduct(HitResult.ImpactNormal, FVector::UpVector));
        if (FMath::RadiansToDegrees(GroundSlope) < 30.0f)
        {
            bIsPlantable = true;
        }
    }
*/
}

void ANaviSpikePlantArea::OnOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }
    
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (ASC)
    {
  
        // @Todo: Replicated, not Replicated 둘 다 필요한지??
        ASC->AddLooseGameplayTag(NaviGameplayTags::Navi_Spike_InPlantArea);
        ASC->AddReplicatedLooseGameplayTag(NaviGameplayTags::Navi_Spike_InPlantArea);
        
    }
}

void ANaviSpikePlantArea::OnOverlapEnd_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (ASC)
    {
   
        ASC->RemoveLooseGameplayTag(NaviGameplayTags::Navi_Spike_InPlantArea);
        ASC->RemoveReplicatedLooseGameplayTag(NaviGameplayTags::Navi_Spike_InPlantArea);
        
    }
}