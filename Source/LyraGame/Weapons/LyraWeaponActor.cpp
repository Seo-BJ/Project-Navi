// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraWeaponActor.h"

#include "Character/LyraCharacter.h"


// Sets default values
ALyraWeaponActor::ALyraWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	FppMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FppMesh"));
	FppMesh->SetupAttachment(RootComponent);
	FppMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	FppMesh->SetOnlyOwnerSee(true);
	FppMesh->bCastDynamicShadow = false;
	FppMesh->CastShadow = false;

	FppMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FppMesh->bUseAttachParentBound = true;

	TppMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TppMesh"));
	TppMesh->SetupAttachment(RootComponent);
	TppMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TppMesh->SetOwnerNoSee(true);}

void ALyraWeaponActor::OnEquipped(FName AttachSocket, FTransform AttachTransform)
{
	// AttachMeshToPawn(WeaponAttachSocket, WeaponAttachTransform);
}



void ALyraWeaponActor::OnUnequipped()
{

}

void ALyraWeaponActor::OnEquippedClient()
{
	if (ALyraCharacter* OwnerPawn = GetOwner<ALyraCharacter>())
	{
		AttachMeshToPawn(WeaponAttachSocket, WeaponAttachTransform);
	}
}

FTransform ALyraWeaponActor::GetAimPointTransform()
{
	if (FppMesh == nullptr) return FTransform::Identity;
	return FppMesh->GetSocketTransform("aimsocket");
}


void ALyraWeaponActor::AttachMeshToPawn(FName AttachSocket, FTransform AttachTransform)
{
	if (ALyraCharacter* OwnerPawn = GetOwner<ALyraCharacter>())
	{
		if( OwnerPawn->IsLocallyControlled() == true )
		{
			USkeletalMeshComponent* CharacterMesh1p = OwnerPawn->GetFirstPersonMesh();
			USkeletalMeshComponent* CharacterPawnMesh3p = OwnerPawn->GetMesh();
			
			FppMesh->SetHiddenInGame( false );
			// FppMesh->SetRelativeTransform(AttachTransform);
			FppMesh->AttachToComponent(CharacterMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
			
			// TppMesh->SetHiddenInGame( false );
			// TppMesh->SetRelativeTransform(AttachTransform);
			// TppMesh->AttachToComponent(CharacterPawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = TppMesh;
			USkeletalMeshComponent* UsePawnMesh = OwnerPawn->GetMesh();
			// TppMesh->SetRelativeTransform(AttachTransform);
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
			UseWeaponMesh->SetHiddenInGame( false );
		}
	}
}

