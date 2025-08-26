// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NaviFppCharacter.h"


#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/LyraCameraComponent.h"


ANaviFppCharacter::ANaviFppCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 0.0f; 
	SpringArm->bUsePawnControlRotation = true; 

	ULyraCameraComponent* LyraCameraComponent = GetLyraCameraComponent();
	if (LyraCameraComponent)
	{
		LyraCameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	}
	
	USkeletalMeshComponent* FPMesh = GetFirstPersonMesh();
	
	if (FPMesh)
	{
		FPMesh->SetupAttachment(LyraCameraComponent);
	}
}

void ANaviFppCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


