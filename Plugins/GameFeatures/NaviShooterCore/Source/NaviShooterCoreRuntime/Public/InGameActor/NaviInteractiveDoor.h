#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraActorWithAbilities.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Interaction/SimpleInteractable.h"
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
 * 버튼을 통해 열고 닫을 수 있는 상호작용 문입니다.
 * 체력을 가지고 있으며 파괴될 수 있습니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviInteractiveDoor : public ALyraActorWithAbilities, public IInteractableTarget, public ISimpleInteractable
{
	GENERATED_BODY()

public:
	ANaviInteractiveDoor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~IInteractableTarget 인터페이스
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) override;
	//~IInteractableTarget 인터페이스 끝

	//~ISimpleInteractable 인터페이스
	virtual void SimpleInteract_Implementation(AActor* InstigatorActor) override;
	//~ISimpleInteractable 인터페이스 끝

	virtual USceneComponent* GetInteractionSceneComponent() override { return InteractionPoint; }

	/** 문의 열림/닫힘 상태를 토글합니다. GameplayAbilities에서 호출할 수 있습니다. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Navi|Door")
	void ToggleDoor();

protected:
	/** 문의 프레임 (고정된 부분) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> DoorFrameMesh;

	/** 움직이는 문 본체 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> DoorBodyMesh;

	/** 상호작용할 버튼 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;
	
	/** 상호작용 UI가 표시될 위치 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Door")
	TObjectPtr<USceneComponent> InteractionPoint;

	/** 이동 방식 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	EDoorMoveType MoveType;

	/** 이동할 거리(평행 이동 시) 또는 각도(회전 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	float MoveAmount;

	/** 열림/닫힘 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	float MoveSpeed;

	/** 표시할 상호작용 옵션 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navi|Door")
	FInteractionOption InteractionOption;

	/** 문 본체의 초기 상대 트랜스폼 */
	UPROPERTY()
	FTransform InitialDoorTransform;

	/** 목표 상대 트랜스폼 (열림 상태에 따라 계산됨) */
	FTransform TargetDoorTransform;

private:
	/** 문이 열려있는지 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, SaveGame)
	bool bIsOpen;

	/** 문이 파괴되었는지 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_IsDestroyed, SaveGame)
	bool bIsDestroyed;

	UFUNCTION()
	void OnRep_IsOpen();

	UFUNCTION()
	void OnRep_IsDestroyed();

	/** 체력이 0에 도달했을 때 호출됨 */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

	/** 매 틱마다 문의 트랜스폼 이동을 업데이트합니다. */
	void UpdateDoorMovement(float DeltaTime);
};