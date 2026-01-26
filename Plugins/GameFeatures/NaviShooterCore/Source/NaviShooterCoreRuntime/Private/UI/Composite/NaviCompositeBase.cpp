// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviCompositeBase.h"

void UNaviCompositeBase::UpdateView(const ULyraInventoryItemDefinition* ItemDef)
{
	// Base implementation does nothing.
	// Subclasses (Leafs) will use ItemDef to find data matching TargetTag.
	// Subclasses (Composites) will propagate this call to children.
}

void UNaviCompositeBase::SetIsCollapsed(bool bCollapsed)
{
	SetVisibility(bCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}
