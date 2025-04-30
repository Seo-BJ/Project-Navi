// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NaviEnemyCharacter.h"

#include "AI/NaviAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
ANaviEnemyCharacter::ANaviEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ANaviEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANaviEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	AuraAIController = Cast<ANaviAIController>(NewController);
	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AuraAIController->RunBehaviorTree(BehaviorTree);
}