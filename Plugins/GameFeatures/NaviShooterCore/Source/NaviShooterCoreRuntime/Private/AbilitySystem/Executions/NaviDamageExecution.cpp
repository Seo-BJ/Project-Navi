// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Executions/NaviDamageExecution.h"

#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/LyraGameplayEffectContext.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"
#include "Engine/World.h"
#include "LyraLogChannels.h"
#include "AbilitySystem/Attributes/LyraCombatSet.h"

#include "Teams/LyraTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NaviDamageExecution)

struct FNaviDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;
	FGameplayEffectAttributeCaptureDefinition HealthDef;
	FGameplayEffectAttributeCaptureDefinition ArmorDef;
	
	FNaviDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(ULyraCombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		ArmorDef = FGameplayEffectAttributeCaptureDefinition(ULyraHealthSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target, true);
	}
};

static FNaviDamageStatics& NaviDamageStatics()
{
	static FNaviDamageStatics Statics;
	return Statics;
}


UNaviDamageExecution::UNaviDamageExecution()
{
	RelevantAttributesToCapture.Add(NaviDamageStatics().BaseDamageDef);
	RelevantAttributesToCapture.Add(NaviDamageStatics().ArmorDef);
}

void UNaviDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Spec.GetContext());
	check(TypedContext);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float SourceBaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(NaviDamageStatics().BaseDamageDef, EvaluateParameters, SourceBaseDamage);
	float TargetArmor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(NaviDamageStatics().ArmorDef, EvaluateParameters, TargetArmor);


	const AActor* EffectCauser = TypedContext->GetEffectCauser();
	const FHitResult* HitActorResult = TypedContext->GetHitResult();

	AActor* HitActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	// Calculation of hit actor, surface, zone, and distance all rely on whether the calculation has a hit result or not.
	// Effects just being added directly w/o having been targeted will always come in without a hit result, which must default
	// to some fallback information.
	if (HitActorResult)
	{
		const FHitResult& CurHitResult = *HitActorResult;
		HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor)
		{
			ImpactLocation = CurHitResult.ImpactPoint;
			ImpactNormal = CurHitResult.ImpactNormal;
			StartTrace = CurHitResult.TraceStart;
			EndTrace = CurHitResult.TraceEnd;
		}
	}

	// Handle case of no hit result or hit result not actually returning an actor
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!HitActor)
	{
		HitActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
		if (HitActor)
		{
			ImpactLocation = HitActor->GetActorLocation();
		}
	}

	// Apply rules for team damage/self damage/etc...
	float DamageInteractionAllowedMultiplier = 0.0f;
	if (HitActor)
	{
		ULyraTeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
		}
	}

	// Determine distance
	double Distance = WORLD_MAX;

	if (TypedContext->HasOrigin())
	{
		Distance = FVector::Dist(TypedContext->GetOrigin(), ImpactLocation);
	}
	else if (EffectCauser)
	{
		Distance = FVector::Dist(EffectCauser->GetActorLocation(), ImpactLocation);
	}
	else
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("Damage Calculation cannot deduce a source location for damage coming from %s; Falling back to WORLD_MAX dist!"), *GetPathNameSafe(Spec.Def));
	}

	// Apply ability source modifiers
	float PhysicalMaterialAttenuation = 1.0f;
	float DistanceAttenuation = 1.0f;
	if (const ILyraAbilitySourceInterface* AbilitySource = TypedContext->GetAbilitySource())
	{
		if (const UPhysicalMaterial* PhysMat = TypedContext->GetPhysicalMaterial())
		{
			PhysicalMaterialAttenuation = AbilitySource->GetPhysicalMaterialAttenuation(PhysMat, SourceTags, TargetTags);
		}

		DistanceAttenuation = AbilitySource->GetDistanceAttenuation(Distance, SourceTags, TargetTags);
	}
	DistanceAttenuation = FMath::Max(DistanceAttenuation, 0.0f);

	// Clamping is done when damage is converted to -health
	const float AdjustedDamage  = FMath::Max(SourceBaseDamage * DistanceAttenuation * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);
	float DamageToHealth = 0.0f;
	float DamageToArmor = 0.0f;

	if (AdjustedDamage > 0.0f)
	{
		if (TargetArmor > 0.0f)
		{
			const float ArmorDamageAbsorptionRatio = 0.66f; // 방어구 흡수율
			const float PotentialArmorAbsorption = AdjustedDamage * ArmorDamageAbsorptionRatio;
			const float PotentialDamageToHealth = AdjustedDamage * (1-ArmorDamageAbsorptionRatio);
			if (TargetArmor > PotentialArmorAbsorption)
			{
				DamageToArmor = PotentialArmorAbsorption;
				DamageToHealth = PotentialDamageToHealth;
			}
			else
			{
				DamageToArmor = TargetArmor;
				DamageToHealth = FMath::Max(0.0f, (PotentialDamageToHealth + PotentialArmorAbsorption - TargetArmor));
			}
		}
		else
		{
			DamageToHealth = AdjustedDamage;
		}
		
	}
	if (DamageToArmor > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraHealthSet::GetIncomingDamageToArmorAttribute(), EGameplayModOp::Additive, DamageToArmor));
	}
	if (DamageToHealth > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraHealthSet::GetIncomingDamageToHealthAttribute(), EGameplayModOp::Additive, DamageToHealth));
	}
#endif // #if WITH_SERVER_CODE
}