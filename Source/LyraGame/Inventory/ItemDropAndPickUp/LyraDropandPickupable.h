// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Inventory/IPickupable.h"

#include "LyraDropandPickupable.generated.h"

struct FInteractionQuery;
class UBoxComponent;
class UProjectileMovementComponent;

UCLASS(Abstract, Blueprintable)
class LYRAGAME_API ALyraDropandPickupable : public AActor, public IInteractableTarget, public IPickupable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALyraDropandPickupable();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;
	virtual FInventoryPickup GetPickupInventory() const override;
	
	UPROPERTY(EditAnywhere)
	FInteractionOption Option;

	UPROPERTY(EditAnywhere)
	FInventoryPickup StaticInventory;

protected:

	virtual void BeginPlay() override;
	
	// 발사체의 물리적 충돌을 담당할 컴포넌트. 땅에 닿으면 비활성화됨.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UBoxComponent> MovementCollision;
	
	// 포물선 운동을 담당할 발사체 이동 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	// 발사체가 어딘가에 부딪혔을 때 호출될 함수
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
public:
	

	// 외부에서 ProjectileMovementComponent를 가져갈 수 있도록 Getter 제공
	UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }
};
