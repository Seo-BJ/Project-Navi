// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraWeaponActor.h"

#include "Character/LyraCharacter.h"


// Sets default values
ALyraWeaponActor::ALyraWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	MeshFpp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFpp"));
	MeshFpp->SetupAttachment(RootComponent);
	MeshFpp->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	MeshFpp->SetOnlyOwnerSee(true);
	MeshFpp->bCastDynamicShadow = false;
	MeshFpp->CastShadow = false;

	MeshFpp->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	MeshFpp->bUseAttachParentBound = true;

	MeshTpp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshTpp"));
	MeshTpp->SetupAttachment(RootComponent);
	MeshTpp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeshTpp->SetOwnerNoSee(true);}

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
	if (MeshFpp == nullptr) return FTransform::Identity;
	return MeshFpp->GetSocketTransform("aimsocket");
}


void ALyraWeaponActor::AttachMeshToPawn(FName AttachSocket, FTransform AttachTransform)
{
	if (ALyraCharacter* OwnerPawn = GetOwner<ALyraCharacter>())
	{
		if( OwnerPawn->IsLocallyControlled() == true )
		{
			USkeletalMeshComponent* CharacterMesh1p = OwnerPawn->GetFirstPersonMesh();
			USkeletalMeshComponent* CharacterPawnMesh3p = OwnerPawn->GetMesh();
			
			MeshFpp->SetHiddenInGame( false );
			// MeshFpp->SetRelativeTransform(AttachTransform);
			MeshFpp->AttachToComponent(CharacterMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
			
			MeshTpp->SetHiddenInGame( false );
			MeshTpp->SetRelativeTransform(AttachTransform);
			MeshTpp->AttachToComponent(CharacterPawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = MeshFpp;
			USkeletalMeshComponent* UsePawnMesh = OwnerPawn->GetFirstPersonMesh();
			MeshTpp->SetRelativeTransform(AttachTransform);
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
			UseWeaponMesh->SetHiddenInGame( false );
		}
	}
}

