// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/Leaf/NaviLeaf.h"
#include "NaviLeaf_StatDisplay.generated.h"

class UTextBlock;

/**
 * A specialized Leaf widget to display a numeric stat.
 * It automatically fetches the value using the TargetTag from the UIInfo fragment.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf_StatDisplay : public UNaviLeaf
{
	GENERATED_BODY()

public:
	//~ Begin UNaviCompositeBase Interface
	virtual void UpdateView(const ULyraInventoryItemDefinition* ItemDef) override;
	//~ End UNaviCompositeBase Interface
	
	void SetLabelText(const FText& InText);
	void SetFractionalDigits(int32 Min, int32 Max);
	
	/** Manually set the displayed value, bypassing the ItemDef lookup. */
	void SetStatValue(float Value);

protected:	void UpdateLabel();
	void UpdateValue(float Value);

	void NativePreConstruct() override;

protected:
	// The text widget to display the stat value.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Value;

	// The text widget to display the label (optional).
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Label;

	// Text to display on the label. If empty, the label widget might be collapsed or hidden.
	UPROPERTY(EditAnywhere, Category = "Navi|StatDisplay")
	FText LabelText;

	// Minimum fractional digits to display (e.g., 0 for integers, 1 for 0.5).
	UPROPERTY(EditAnywhere, Category = "Navi|StatDisplay")
	int32 MinFractionalDigits{0};

	// Maximum fractional digits to display.
	UPROPERTY(EditAnywhere, Category = "Navi|StatDisplay")
	int32 MaxFractionalDigits{1};

	// If true, the widget will collapse if the stat is not found in the item definition.
	UPROPERTY(EditAnywhere, Category = "Navi|StatDisplay")
	bool bCollapseIfMissing{true};
};
