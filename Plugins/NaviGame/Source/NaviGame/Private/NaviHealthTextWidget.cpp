// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviHealthTextWidget.h"
#include "Components/ProgressBar.h" // 예시: 체력 바 업데이트 시 필요
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"            // IAbilitySystemInterface
#include "Character/LyraHealthComponent.h"    // Lyra Health Component
#include "AbilitySystem/LyraAbilitySystemComponent.h" // Lyra ASC (필요하다면)


void UNaviHealthTextWidget::NativeConstruct()
{
    Super::NativeConstruct();
    APlayerController* OwningPlayerController=  GetOwningPlayer();
    if (IsValid(OwningPlayerController))
    {
        OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UNaviHealthTextWidget::OnPossessedPawnChanged);
    }
    OnPossessedPawnChanged(nullptr, OwningPlayerController->GetPawn());
}

void UNaviHealthTextWidget::NativeDestruct()
{
    // 위젯 소멸 시 반드시 바인딩 해제
    // ClearBindings();
    Super::NativeDestruct();
}

void UNaviHealthTextWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    ULyraHealthComponent* OldHealthComponent = ULyraHealthComponent::FindHealthComponent(OldPawn);
    if (IsValid(OldHealthComponent))
    {
        OldHealthComponent->OnHealthChanged.RemoveDynamic(this, &UNaviHealthTextWidget::HandleHealthChanged);
        OldHealthComponent->OnArmorChanged.RemoveDynamic(this, &UNaviHealthTextWidget::HandleArmorChanged);
        OldHealthComponent->OnShieldChanged.RemoveDynamic(this, &UNaviHealthTextWidget::HandleShieldChanged);
    }
    HandlePossessedPawnChanged(OldPawn, NewPawn);
    ULyraHealthComponent* NewHealthComponent = ULyraHealthComponent::FindHealthComponent(NewPawn);
    if (IsValid(NewHealthComponent))
    {
        NewHealthComponent->OnHealthChanged.AddDynamic(this, &UNaviHealthTextWidget::HandleHealthChanged);
        NewHealthComponent->OnArmorChanged.AddDynamic(this, &UNaviHealthTextWidget::HandleArmorChanged);
        NewHealthComponent->OnShieldChanged.AddDynamic(this, &UNaviHealthTextWidget::HandleShieldChanged);
    }
}
void UNaviHealthTextWidget::HandleHealthChanged_Implementation(ULyraHealthComponent* HealthComp, float OldValue, float NewValue, AActor* Instigator)
{
    
}
void UNaviHealthTextWidget::HandleArmorChanged_Implementation(ULyraHealthComponent* HealthComp, float OldValue, float NewValue,
    AActor* Instigator)
{
    
}

void UNaviHealthTextWidget::HandleShieldChanged_Implementation(ULyraHealthComponent* HealthComp, float OldValue, float NewValue,
    AActor* Instigator)
{

    
}

