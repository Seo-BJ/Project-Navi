// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgentSelectionInfo.h"
#include "Components/GameStateComponent.h"
#include "NaviAgentSelectionManagerComponent.generated.h"

class ULyraExperienceDefinition;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UNaviAgentSelectionManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	
	UNaviAgentSelectionManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void HandleAgentSelectionRequest(const FAgentSelectionInfo& Info);

	UFUNCTION(BlueprintCallable)
	void HandleConfirmSelectionRequest(const FString& UserName);

	
	void OnAgentSelectionInfoChanged(const FAgentSelectionInfo& Info);
	void OnAgentSelectionInfoAdded(const FAgentSelectionInfo& Info);
	void OnAgentSelectionInfoRemoved(const FAgentSelectionInfo& Info);
	void OnAgentSelectionConfirmed(const FAgentSelectionInfo& Info);
	void OnAllPlayerConfirmedAgentSelection(const FAgentSelectionInfo& Info);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	

	UFUNCTION()
	void OnRep_AgentSelectionInfoArray();
	
private:
	
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	UPROPERTY(ReplicatedUsing=OnRep_AgentSelectionInfoArray)
	FAgentSelectionInfoArray AgentSelectionInfoArray;
	
};


