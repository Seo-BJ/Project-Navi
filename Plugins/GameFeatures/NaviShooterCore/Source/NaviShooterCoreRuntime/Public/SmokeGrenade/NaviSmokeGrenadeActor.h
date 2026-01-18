#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NaviSmokeGrenadeActor.generated.h"

class UStaticMeshComponent;

/**
 * ANaviSmokeGrenadeActor
 *
 * A spherical smoke grenade effect that blocks vision but allows movement.
 * Spawns with a duration and destroys itself afterwards.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviSmokeGrenadeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ANaviSmokeGrenadeActor();

protected:
	virtual void BeginPlay() override;

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Smoke")
	TObjectPtr<UStaticMeshComponent> SmokeMeshComponent;

	// Settings
	/** Duration of the smoke before it starts to dissipate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Smoke", meta = (ExposeOnSpawn = true))
	float SmokeDuration;

	/** Radius of the smoke sphere. Assumes the mesh is a 100-unit diameter sphere (default engine sphere). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Smoke", meta = (ExposeOnSpawn = true))
	float SmokeRadius;

protected:
	/** Dynamic Material Instance for runtime parameter changes (Color, Opacity, etc.) */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Navi|Smoke")
	TObjectPtr<UMaterialInstanceDynamic> SmokeMaterialInstance;

	/** Timer Handle for the smoke duration */
	FTimerHandle TimerHandle_SmokeDuration;

	/** Called when the duration expires. Implement visual fade-out here in BP. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Navi|Smoke")
	void OnDissipate();

	/** Call this from BP when the dissipation (fade-out) is finished to destroy the actor. */
	UFUNCTION(BlueprintCallable, Category = "Navi|Smoke")
	void FinishDissipation();

	/** Helper to set scalar parameter on the smoke material */
	UFUNCTION(BlueprintCallable, Category = "Navi|Smoke")
	void SetSmokeScalarParameter(FName ParamName, float Value);
};
