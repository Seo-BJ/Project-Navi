// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Inventory/IPickupable.h"

#include "LyraDropAndPickupable.generated.h"

struct FInteractionQuery;
class UBoxComponent;
class UProjectileMovementComponent;

UCLASS(Abstract, Blueprintable)
class LYRAGAME_API ALyraDropAndPickupable : public AActor, public IInteractableTarget, public IPickupable
{
	GENERATED_BODY()

public:
	
	ALyraDropAndPickupable();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;
	virtual FInventoryPickup GetPickupInventory() const override;
	
	UPROPERTY(EditAnywhere)
	FInteractionOption Option;

	UPROPERTY(EditAnywhere)
	FInventoryPickup StaticInventory;

protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> MovementCollision;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	
	UFUNCTION()
	void OnProjectileComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, const FHitResult& Hit);

	bool bIsOnGround = false;

	virtual void OnDropFinished();
	
public:
	
	UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }
};
