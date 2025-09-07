// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquipmentInterface.h"
#include "GameFramework/Actor.h"
#include "LyraWeaponActor.generated.h"

class USkeletalMeshComponent;
;

UCLASS()
class LYRAGAME_API ALyraWeaponActor : public AActor, public IEquipmentInterface
{
	GENERATED_BODY()

public:
	ALyraWeaponActor();

	FORCEINLINE USkeletalMeshComponent* GetFirstPersonPerspectiveMesh() { return FppMesh; }
	FORCEINLINE USkeletalMeshComponent* GetThirdPersonPerspectiveMesh() { return TppMesh; }

	void OnEquipped(FName AttachSocket, FTransform AttachTransform);

	void OnUnequipped();

	UPROPERTY(EditDefaultsOnly)
	FName WeaponAttachSocket;
	
	UPROPERTY(EditDefaultsOnly)
	FTransform WeaponAttachTransform;

	virtual void OnEquippedClient() override;

	FTransform GetAimPointTransform();


protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FppMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> TppMesh;

private:
	void AttachMeshToPawn(FName AttachSocket, FTransform AttachTransform);
	
};
