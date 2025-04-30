// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NaviAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Abilitysystem/LyraGameplayEffectContext.h"
#include "NaviGame/NaviGameplayTags.h"




void UNaviAbilitySystemLibrary::SetIsRadialDamageEffectParam(FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin)
{
	DamageEffectParams.bIsRadialDamage = bIsRadial;
	DamageEffectParams.RadialDamageInnerRadius = InnerRadius;
	DamageEffectParams.RadialDamageOuterRadius = OuterRadius;
	DamageEffectParams.RadialDamageOrigin = Origin;
}

void UNaviAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude)
{
	KnockbackDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
	}
	else
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * Magnitude;
	}
}

void UNaviAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude)
{
	ImpulseDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * DamageEffectParams.DeathImpulseMagnitude;
	}
	else
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * Magnitude;
	}
}

void UNaviAbilitySystemLibrary::SetTargetEffectParamsASC(FDamageEffectParams& DamageEffectParams,
	UAbilitySystemComponent* InASC)
{
	DamageEffectParams.TargetAbilitySystemComponent = InASC;
}


\
bool UNaviAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsBlockedHit();
	}
	return false;
}

bool UNaviAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float UNaviAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UNaviAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UNaviAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag UNaviAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (AuraEffectContext->GetDamageType().IsValid())
		{
			return *AuraEffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector UNaviAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector UNaviAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool UNaviAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

bool UNaviAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsRadialDamage();
	}
	return false;
}

float UNaviAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float UNaviAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector UNaviAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FLyraGameplayEffectContext* AuraEffectContext = static_cast<const FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void UNaviAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UNaviAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
		{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UNaviAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
		{
		AuraEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UNaviAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDamage(InDamage);
	}
}

void UNaviAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDuration(InDuration);
	}
}

void UNaviAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UNaviAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		AuraEffectContext->SetDamageType(DamageType);
	}
}

void UNaviAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UNaviAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InForce)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetKnockbackForce(InForce);
	}
}

void UNaviAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsRadialDamage)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void UNaviAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InInnerRadius)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void UNaviAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InOuterRadius)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void UNaviAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InOrigin)
{
	if (FLyraGameplayEffectContext* AuraEffectContext = static_cast<FLyraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetRadialDamageOrigin(InOrigin);
	}
}

void UNaviAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxTargets)
	{
		if (ActorsToCheck.Num() == 0) break;
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor;
		for (AActor* PotentialTarget : ActorsToCheck)
		{
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		++NumTargetsFound;
	}
}

bool UNaviAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return !bFriends;
}

FGameplayEffectContextHandle UNaviAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	
	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContexthandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContexthandle, DamageEffectParams.KnockbackForce);

	SetIsRadialDamage(EffectContexthandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContexthandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContexthandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContexthandle, DamageEffectParams.RadialDamageOrigin);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);
	
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContexthandle;
}

TArray<FRotator> UNaviAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> UNaviAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}



void UNaviAbilitySystemLibrary::ApplyDamageContainerFromTargetData(
    const FGameplayAbilityTargetDataHandle& TargetDataHandle,
    AActor* InstigatorActor,
    TSubclassOf<UGameplayEffect> DamageCalculationEffectClass,
    TSubclassOf<UGameplayEffect> DamageExecutionEffectClass,
    const UGameplayAbility* DamageAbility)
{
 
    if (!InstigatorActor || !InstigatorActor->HasAuthority())
    {
        return;
    }

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor);
    if (!SourceASC || !DamageCalculationEffectClass || !DamageExecutionEffectClass || !DamageAbility || TargetDataHandle.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyDamageContainerFromTargetData: Invalid inputs or empty TargetDataHandle."));
        return;
    }

	// --- 2. TargetData에서 HitResult 추출 (GC 위치 결정에 중요!) ---
	FHitResult FirstHitResult;
	// 예시: 첫 번째 유효한 TargetData에서 HitResult 가져오기 (TargetData 타입 확인 필요)
	if (TargetDataHandle.IsValid(0) && TargetDataHandle.Get(0) != nullptr) {
		const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(0);
		// FGameplayAbilityTargetData_SingleTargetHit 타입인지 확인
		if (TargetData->GetScriptStruct()->IsChildOf(FGameplayAbilityTargetData_SingleTargetHit::StaticStruct())) {
			FirstHitResult = static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(TargetData)->HitResult;
		}
		// 필요에 따라 다른 TargetData 타입 (예: FGameplayAbilityTargetData_LocationInfo) 처리 로직 추가
	}
	// 만약 HitResult를 못 찾으면 GC는 여전히 0,0,0 이나 액터 위치에 나올 수 있음

	// --- 3. HitResult 정보가 포함된 Context Handle 생성 (루프 전에 한 번!) ---
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext(); // 기본 컨텍스트 생성
	ContextHandle.AddInstigator(InstigatorActor, InstigatorActor); // 가해자 정보 추가
	// Optional: ContextHandle.SetAbility(DamageAbility); // 어빌리티 정보 추가
	ContextHandle.AddHitResult(FirstHitResult); // <<< 추출된 HitResult 추가!

	// TArray<TWeakObjectPtr<AActor>> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetDataHandle, 0);
	// ContextHandle.AddActors(TargetActors);
	
    FGameplayEffectContextHandle CalculationEffectContextHandle = SourceASC->MakeEffectContext();
    FGameplayEffectSpecHandle DamageCalculationSpecHandle = SourceASC->MakeOutgoingSpec(DamageCalculationEffectClass, DamageAbility->GetAbilityLevel(), CalculationEffectContextHandle);

	
    FGameplayEventData Payload;
    Payload.EventTag = NaviGameplayTags::Event_Damage_BeforeDamage; 
    Payload.Instigator = InstigatorActor;
    Payload.ContextHandle = CalculationEffectContextHandle; // 생성된 커스텀 컨텍스트 전달!
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InstigatorActor, Payload.EventTag, Payload);

   //  SourceASC->ApplyGameplayEffectSpecToSelf(*DamageCalculationSpecHandle.Data);

	// 1. TargetDataHandle에서 유효한 액터 목록 가져오기
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetDataHandle, 0);

	// 2. 각 타겟 액터에게 DamageExecutionSpec 적용
	for (const TWeakObjectPtr<AActor>& TargetActorPtr : TargetActors)
	{
		AActor* TargetActor = TargetActorPtr.Get();
		if (!TargetActor) continue; // 유효하지 않은 액터 건너뛰기

		// 3. 타겟 액터의 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC) continue; // ASC 없으면 건너뛰기
		
		FGameplayEffectSpecHandle DamageExecutionSpecHandle = SourceASC->MakeOutgoingSpec(DamageExecutionEffectClass, DamageAbility->GetAbilityLevel(), ContextHandle);
		
		// 4. 대상(Target) ASC에 DamageExecutionSpec 적용 (ApplyGameplayEffectSpecToSelf 사용)
		//    주의: Spec Handle 자체는 유효해야 함.
		if (DamageExecutionSpecHandle.IsValid())
		{
			// Spec 복사본을 만드는 것이 더 안전할 수 있습니다 (ApplyGameplayEffectSpecToSelf가 내부 수정 가능성)
			// FGameplayEffectSpec* SpecToApply = new FGameplayEffectSpec(*DamageExecutionSpecHandle.Data.Get());
			// TargetASC->ApplyGameplayEffectSpecToSelf(*SpecToApply);
			// delete SpecToApply; // 'new' 사용 시 메모리 관리 필요

			// 간단하게 직접 적용 (Spec 내부 데이터가 공유될 수 있음에 유의)
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageExecutionSpecHandle.Data.Get());
		}
	}
	
}

