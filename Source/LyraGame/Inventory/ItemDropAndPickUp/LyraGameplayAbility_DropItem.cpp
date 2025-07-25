// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_DropItem.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/LyraPlayerController.h"
#include "Inventory/ItemDropAndPickUp/LyraDropandPickupable.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "Inventory/IPickupable.h"
#include "Kismet/GameplayStatics.h"
#include "Character/LyraCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h" // 필요할 수 있음

ULyraGameplayAbility_DropItem::ULyraGameplayAbility_DropItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}



void ULyraGameplayAbility_DropItem::SpawnPickupableItemActor(TSubclassOf<ALyraDropandPickupable> DroppedWeaponClass, ULyraInventoryItemInstance* ItemInstance)
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
    
    // 1. 위치 및 방향 정보 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    LyraPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const FRotator ControlRotation = LyraPlayerController->GetControlRotation();
    const FVector SpawnLocation = AvatarCharacter->GetActorLocation() + (ControlRotation.Vector() * 100.0f);
    const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
    
    // 2. 무기 액터 스폰 (SpawnActorDeferred 사용)
    ALyraDropandPickupable* SpawnedWeapon = GetWorld()->SpawnActorDeferred<ALyraDropandPickupable>(
        DroppedWeaponClass,
        SpawnTransform,
        GetActorInfo().OwnerActor.Get(),
        AvatarCharacter,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (SpawnedWeapon)
    {
        // 3. 스폰된 액터에 데이터 설정
        FPickupInstance PickupInstance;
        PickupInstance.Item = ItemInstance;
        SpawnedWeapon->StaticInventory.Instances.Add(PickupInstance);
   
        // 4. 발사체 컴포넌트에 초기 속도 설정
        const float LaunchSpeed = 450.0f;
        const FVector LaunchVelocity = ControlRotation.Vector() * LaunchSpeed;

        // ======================= [디버그 코드 추가] =======================
        UWorld* World = GetWorld();
        if (World)
        {
            // [디버그 1] 스폰 위치(SpawnLocation)를 녹색 구체로 5초간 표시합니다.
            DrawDebugSphere(
                World,
                SpawnLocation,
                10.0f,          // 구체 크기
                12,             // 구체 정밀도
                FColor::Green,
                false,          // 영구 표시 여부
                5.0f,           // 표시 시간 (초)
                0,              // 우선순위
                1.0f            // 두께
            );

            // [디버그 2] 발사 방향(LaunchVelocity)을 빨간색 화살표로 5초간 표시합니다.
            // 화살표 길이는 발사 방향을 나타내기 위해 150 유닛으로 정규화했습니다.
            DrawDebugDirectionalArrow(
                World,
                SpawnLocation,  // 시작점
                SpawnLocation + (LaunchVelocity.GetSafeNormal() * 150.0f), // 끝점
                10.0f,          // 화살표 머리 크기
                FColor::Red,
                false,
                5.0f,
                0,
                2.0f            // 화살표 선 두께
            );
        }
        // =================================================================

        if (UProjectileMovementComponent* ProjMovement = SpawnedWeapon->GetProjectileMovementComponent())
        {
            ProjMovement->Velocity = LaunchVelocity;
        }

        // 5. 액터 스폰 최종 완료
        UGameplayStatics::FinishSpawningActor(SpawnedWeapon, SpawnTransform);

        // 6. 스폰이 완료된 후 발사체 컴포넌트 활성화
        if (UProjectileMovementComponent* ProjMovement = SpawnedWeapon->GetProjectileMovementComponent())
        {
            ProjMovement->Activate();
        }
    }
}