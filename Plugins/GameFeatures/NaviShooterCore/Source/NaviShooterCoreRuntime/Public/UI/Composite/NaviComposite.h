// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/NaviCompositeBase.h"
#include "NaviComposite.generated.h"

/**
 * Composite container widget that propagates UpdateView calls to its children.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviComposite : public UNaviCompositeBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void UpdateView(const ULyraInventoryItemDefinition* ItemDef) override;
	virtual void SetIsCollapsed(bool bCollapsed) override;

protected:
	// Automatically populated list of child composite widgets
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNaviCompositeBase>> Children;
};
