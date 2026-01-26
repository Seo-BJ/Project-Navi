// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/NaviInventoryFragment_UIInfo.h"

void UNaviLeaf::UpdateView(const ULyraInventoryItemDefinition* ItemDef)
{
	Super::UpdateView(ItemDef);

	if (!ItemDef)
	{
		SetIsCollapsed(true);
		return;
	}

	// Try to find the UI Info fragment
	const UNaviInventoryFragment_UIInfo* UIInfo = Cast<UNaviInventoryFragment_UIInfo>(
		ItemDef->FindFragmentByClass(UNaviInventoryFragment_UIInfo::StaticClass())
	);

	// Even if UIInfo is null, we pass it to BP. BP logic decides if it can show something generic
	// or if it strictly needs the fragment.
	// However, usually we rely on the fragment for the specific 'TargetTag' data.
	
	bool bShouldShow = OnUpdateView(ItemDef, UIInfo);
	SetIsCollapsed(!bShouldShow);
}
