// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_DropItem.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/LyraPlayerController.h"
#include "Inventory/ItemDropAndPickUp/LyraDropAndPickupable.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "Inventory/IPickupable.h"
#include "Kismet/GameplayStatics.h"
#include "Character/LyraCharacter.h"
#include "DrawDebugHelpers.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Item_Dropped, "Lyra.Item.Dropped")

ULyraGameplayAbility_DropItem::ULyraGameplayAbility_DropItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}



void ULyraGameplayAbility_DropItem::SpawnPickupableItemActor(TSubclassOf<ALyraDropAndPickupable> DroppedWeaponClass, ULyraInventoryItemInstance* ItemInstance)
{
    if (DroppedWeaponClass == nullptr || ItemInstance == nullptr)
    {
        return;
    }

    ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();
    ACharacter* AvatarCharacter = GetLyraCharacterFromActorInfo();
    if (LyraPlayerController == nullptr || AvatarCharacter == nullptr)
    {
        return;
    }
 
    FVector CameraLocation;
    FRotator CameraRotation;
    LyraPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const FRotator ControlRotation = LyraPlayerController->GetControlRotation();
    const FVector SpawnLocation = AvatarCharacter->GetActorLocation() + (ControlRotation.Vector() * 100.0f);
    const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
    
    ALyraDropAndPickupable* SpawnedWeapon = GetWorld()->SpawnActorDeferred<ALyraDropAndPickupable>(
        DroppedWeaponClass,
        SpawnTransform,
        GetActorInfo().OwnerActor.Get(),
        AvatarCharacter,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (SpawnedWeapon)
    {
        FPickupInstance PickupInstance;
        PickupInstance.Item = ItemInstance;
        // Drop된 Item임을 표시
        PickupInstance.Item->AddStatTagStack(TAG_Lyra_Item_Dropped, 1);
        SpawnedWeapon->StaticInventory.Instances.Add(PickupInstance);
        
        const FVector LaunchVelocity = ControlRotation.Vector() * LaunchSpeed;

        if (UProjectileMovementComponent* ProjMovement = SpawnedWeapon->GetProjectileMovementComponent())
        {
            ProjMovement->Velocity = LaunchVelocity;
        }

        UGameplayStatics::FinishSpawningActor(SpawnedWeapon, SpawnTransform);

        if (UProjectileMovementComponent* ProjMovement = SpawnedWeapon->GetProjectileMovementComponent())
        {
            ProjMovement->Activate();
        }
    }
}