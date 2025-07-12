// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Messages/GameplayMessageProcessor.h"

#include "NaviCredsStateComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVISHOOTERCORERUNTIME_API UNaviCredsStateComponent : public UGameplayMessageProcessor
{
	GENERATED_BODY()

public:

	// UNaviCredsStateComponent();
	
	virtual void StartListening() override;

private:
	
	// void OnKillMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload);

	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Creds System", meta=(AllowPrivateAccess))
	// TMap<FGameplayTag, int32> CredsAwardData;
};
