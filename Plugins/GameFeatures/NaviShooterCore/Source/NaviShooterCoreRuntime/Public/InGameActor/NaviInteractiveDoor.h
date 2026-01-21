#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraActorWithAbilities.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "NaviInteractiveDoor.generated.h"

class UStaticMeshComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class EDoorMoveType : uint8
{
	TranslateVertical,
	TranslateHorizontal,
	Rotate
};

/**
 * ANaviInteractiveDoor
 * 
 * An interactive door that can be opened/closed via a button.
 * It has health and can be destroyed.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviInteractiveDoor : public ALyraActorWithAbilities, public IInteractableTarget
{
	GENERATED_BODY()

public:
	ANaviInteractiveDoor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~IInteractableTarget interface
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;
	//~End IInteractableTarget interface

	/** Toggle the door open/closed state. Can be called by GameplayAbilities. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Navi|Door")
	void ToggleDoor();

protected:
	/** Frame of the door (Static) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> DoorFrameMesh;

	/** Moving part of the door */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> DoorBodyMesh;

	/** Button to interact with */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;

	/** Type of movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	EDoorMoveType MoveType;

	/** Distance (for translation) or Angle (for rotation) to move */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	float MoveAmount;

	/** Speed of opening/closing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	float MoveSpeed;

	/** Interaction Option to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	FInteractionOption InteractionOption;

	/** Initial Relative Transform of the Door Body */
	UPROPERTY()
	FTransform InitialDoorTransform;

	/** Target Relative Transform (calculated based on Open state) */
	FTransform TargetDoorTransform;

private:
	/** True if the door is open */
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, SaveGame)
	bool bIsOpen;

	/** True if the door is destroyed */
	UPROPERTY(ReplicatedUsing = OnRep_IsDestroyed, SaveGame)
	bool bIsDestroyed;

	UFUNCTION()
	void OnRep_IsOpen();

	UFUNCTION()
	void OnRep_IsDestroyed();

	/** Called when health reaches zero */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

	/** Updates the door transform tick */
	void UpdateDoorMovement(float DeltaTime);
};
