#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NaviFlashbangProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UGameplayEffect;
class UStaticMeshComponent;

/**
 * ANaviFlashbangProjectile
 * 
 * Server-authoritative flashbang projectile that calculates blind intensity
 * based on distance, angle, and occlusion, then applies a GameplayEffect.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviFlashbangProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ANaviFlashbangProjectile();

protected:
	virtual void BeginPlay() override;

	/** Executes the explosion logic. Call this via timer or blueprint event. */
	UFUNCTION(BlueprintCallable, Category = "Navi|Flashbang")
	void Explode();

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Flashbang")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Flashbang")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Flashbang")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	// Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	float ExplosionRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	float FuseTime = 1.5f;

	/** The GameplayEffect to apply (e.g., GE_Flashbang) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	TSubclassOf<UGameplayEffect> FlashEffectClass;

	/** Tag used to pass the calculated intensity to the GE via SetByCaller */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	FGameplayTag FlashIntensityTag;

	/** If true, the flashbang will blind allies (including self). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	bool bCanBlindAllies = true;

	/** Radius for the occlusion check trace. Helps avoid small obstacles blocking the flash. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navi|Flashbang")
	float OcclusionTraceRadius = 10.0f;

private:
	/** Calculates the blind intensity for a specific victim */
	float CalculateFlashIntensity(AActor* Victim, const FVector& FlashLocation);
};
