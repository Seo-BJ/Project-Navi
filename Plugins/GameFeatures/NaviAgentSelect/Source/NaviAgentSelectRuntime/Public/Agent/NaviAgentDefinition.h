// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraEquipmentDefinition.h"
#include "GameplayTags.h"

#include "NaviAgentDefinition.generated.h"

class UNaviAgentInstance;
class UAnimInstance;



/**
 * 
 */
UCLASS()
class NAVIAGENTSELECTRUNTIME_API UNaviAgentDefinition : public ULyraEquipmentDefinition
{
	GENERATED_BODY()

public:
	UNaviAgentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	// @Todo: TSoftObjectPtr로 변경?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AgentTag;	
};
