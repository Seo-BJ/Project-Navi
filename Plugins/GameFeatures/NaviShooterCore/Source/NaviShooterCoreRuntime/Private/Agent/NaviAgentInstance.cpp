// Fill out your copyright notice in the Description page of Project Settings.


#include "Agent/NaviAgentInstance.h"
#include "Character/LyraCharacter.h"
#include "Player/LyraPlayerController.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Inventory/LyraInventoryItemInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "Cosmetics/LyraControllerComponent_CharacterParts.h"

UNaviAgentInstance::UNaviAgentInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UNaviAgentInstance::OnEquipped()
{
    Super::OnEquipped();

    ALyraCharacter* OuterLyraCharacter = Cast<ALyraCharacter>(GetPawn());
    if (!OuterLyraCharacter)
    {
        return;
    }
    ALyraPlayerController* LyraPC = OuterLyraCharacter->GetLyraPlayerController();
    if (!LyraPC)
    {
        return;
    }
    ULyraControllerComponent_CharacterParts* CosmeticController = LyraPC->GetComponentByClass<ULyraControllerComponent_CharacterParts>();
    if (!CosmeticController)
    {
        return;
    }
    USkeletalMeshComponent* MeshComponent = OuterLyraCharacter->GetMesh();
    if (!MeshComponent)
    {
        return;
    }

    // 1. 외형 적용 (Skeletal Mesh 및 Character Parts)
    if (USkeletalMesh* TargetMesh = Cast<USkeletalMesh>(SkeletalMeshOverride.LoadSynchronous()))
    {
        MeshComponent->SetSkeletalMeshAsset(TargetMesh);
    }
    
    CosmeticController->RemoveAllCharacterParts();
    for (FLyraCharacterPart PartClass : CharacterParts)
    {
        CosmeticController->AddCharacterPart(PartClass);
    }

    // 2. 기본 스탯(Attribute) 적용
    if (ULyraAbilitySystemComponent* ASC = OuterLyraCharacter->GetLyraAbilitySystemComponent())
    {
        if (AdditionalAttributes)
        {
            FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
            EffectContext.AddSourceObject(this);
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AdditionalAttributes, 1.0f, EffectContext);
            if (SpecHandle.IsValid())
            {
                AppliedGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
}

void UNaviAgentInstance::OnUnequipped()
{
    Super::OnUnequipped();
    
    ALyraCharacter* OuterLyraCharacter = Cast<ALyraCharacter>(GetPawn());
    if (!OuterLyraCharacter)
    {
        return;
    }

    
    // 1. 스폰했던 캐릭터 파츠 제거
    for (AActor* Part : SpawnedCharacterParts)
    {
        if (Part)
        {
            Part->Destroy();
        }
    }
    SpawnedCharacterParts.Empty();

    // 2. 적용되었던 기본 스탯(Attribute) 제거
    if (ULyraAbilitySystemComponent* ASC = OuterLyraCharacter->GetLyraAbilitySystemComponent())
    {
        if (AppliedGEHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(AppliedGEHandle);
        }
    }

    // 3. 지급했던 인벤토리 아이템 제거
    if (ALyraPlayerController* PC = OuterLyraCharacter->GetController<ALyraPlayerController>())
    {
        if (ULyraInventoryManagerComponent* InventoryManager = PC->FindComponentByClass<ULyraInventoryManagerComponent>())
        {
            for (ULyraInventoryItemInstance* ItemInstance : GrantedItemInstances)
            {
                InventoryManager->RemoveItemInstance(ItemInstance);
            }
            GrantedItemInstances.Empty();
        }
    }
}