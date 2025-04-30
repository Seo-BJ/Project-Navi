// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/NaviGameplayAbility_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystem/LyraGameplayEffectContext.h"
#include "AbilitySystem/NaviAbilitySystemLibrary.h"
// #include "NaviGameplayEffectContext.h" // 커스텀 컨텍스트 include

UNaviGameplayAbility_Damage::UNaviGameplayAbility_Damage()
{
    // 기본값 설정 (예시)
    DamageMultiplier.SetValue(1.0f);
}



// 데미지 파이프라인을 시작하는 핵심 함수
void UNaviGameplayAbility_Damage::ApplyDamageContainerOnTarget(const FGameplayAbilityTargetDataHandle& TargetData)
{
    if (!HasAuthority(&CurrentActivationInfo)) // 서버에서만 실행되어야 함
    {
        return;
    }

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC)
    {
        return;
    }

    
    // Apply Damage 시작할때 Total Damage를 0으로 초기화 해도 될듯?
    // 결국에는 맨 처음 or 마지막에만 초기화 하면 되니깐..
    // 1. Total Damage 계산 (Event Trigger)
    // 2. 이벤트 종료시 Target에 Effect 적용
    // 3. Target과 Instigator 각각에 Event.hit 전달
    // 4. 데미지 리엑트
    // 5. 근데 피해 감소 어빌리티도 있다면 비슷한 구조로 고려를 해야할듯?
    
    // 타겟 데이터에서 유효한 대상들을 가져옵니다.
    const TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData, 0);

    for (const TWeakObjectPtr<AActor>& TargetActorPtr : TargetActors)
    {
        AActor* TargetActor = TargetActorPtr.Get();
        if (!TargetActor) continue;

        IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
        if (!TargetASI) continue;

        UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
        if (!TargetASC) continue;

        if (!DamageCalculationEffectClass || !DamageExecutionEffectClass)
        {
             UE_LOG(LogTemp, Warning, TEXT("Damage Effect Class is not set in %s"), *GetName());
             continue;
        }

      //  UNaviAbilitySystemLibrary::ApplyDamageContainer(TargetActor, DamageCalculationEffectClass, DamageExecutionEffectClass, this );
    } 
}