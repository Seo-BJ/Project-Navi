// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"

#include "NaviAgentSelectionComponent.generated.h"


class UNaviAgentDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviAgentSelectionComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UNaviAgentSelectionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UNaviAgentDefinition>> SelectableAgents;

	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerRequestAgentSelectionByTag(const FGameplayTag& AgentTag);

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

private:
	void ApplyAgentToPawn(APawn* TargetPawn);
	
	UPROPERTY(Replicated)
	TSubclassOf<UNaviAgentDefinition> SelectedAgentDefinition;
	

	
};
