// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "GameplayTagContainer.h"
#include "NaviInventoryFragment_UIInfo.generated.h"

/**
 * Base Fragment to hold arbitrary UI data.
 * Can be used directly with Maps, or subclassed to fetch data from external sources (like DataTables).
 */
UCLASS(BlueprintType, Blueprintable)
class LYRAGAME_API UNaviInventoryFragment_UIInfo : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	// Generic map for numeric stats (Damage, FireRate, Range, etc.)
	// Used if not overridden or if key not found in override.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	TMap<FGameplayTag, float> NumericStats;

	// Generic map for text descriptions or labels
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Info")
	TMap<FGameplayTag, FText> TextData;

	// Helper to get a float stat safely. Virtual to allow fetching from DataTables etc.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI Info")
	float GetNumericStat(FGameplayTag Tag, bool& bFound) const;
	virtual float GetNumericStat_Implementation(FGameplayTag Tag, bool& bFound) const;

	// Helper to get text data safely. Virtual to allow fetching from DataTables etc.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI Info")
	FText GetTextData(FGameplayTag Tag, bool& bFound) const;
	virtual FText GetTextData_Implementation(FGameplayTag Tag, bool& bFound) const;
};