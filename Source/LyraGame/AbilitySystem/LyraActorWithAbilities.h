#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "LyraActorWithAbilities.generated.h"

class ULyraAbilitySystemComponent;
class ULyraHealthSet;
class ULyraCombatSet;
class UBoxComponent;
class UGameplayEffect;

UCLASS()
class LYRAGAME_API ALyraActorWithAbilities : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ALyraActorWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxComponent; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Ability System")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<const ULyraHealthSet> HealthSet;

	UPROPERTY(Transient)
	TObjectPtr<const ULyraCombatSet> CombatSet;
	
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability System")
	void InitializeHealthSet();
	
	UPROPERTY(EditAnywhere, Category = "Lyra|Ability System")
	TSubclassOf<UGameplayEffect> HealthInitEffect;
	
};