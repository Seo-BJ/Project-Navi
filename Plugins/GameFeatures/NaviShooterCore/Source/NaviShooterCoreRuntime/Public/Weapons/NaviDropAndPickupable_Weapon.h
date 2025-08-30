// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDropAndPickUp/LyraDropAndPickupable.h"
#include "NaviDropAndPickupable_Weapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;

UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviDropAndPickupable_Weapon : public ALyraDropAndPickupable
{
	GENERATED_BODY()

public:

	ANaviDropAndPickupable_Weapon();

	void SetSkeletalMesh(USkeletalMesh* SkeletalMesh);

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> OverlapCollision;

	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnDropFinished() override;

private:
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponMesh)
	TObjectPtr<USkeletalMesh> ReplicatedWeaponMesh;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	
	UFUNCTION()
	void OnRep_WeaponMesh();
	
};

