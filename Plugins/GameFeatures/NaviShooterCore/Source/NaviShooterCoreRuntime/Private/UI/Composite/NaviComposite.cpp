// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviComposite.h"
#include "Blueprint/WidgetTree.h"

void UNaviComposite::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Automatically find all direct or nested children that are of type UNaviCompositeBase
	// Note: You might want to restrict this to direct children depending on your UI structure preferences,
	// but searching the whole tree allows for flexible nesting.
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			// Ensure we don't add ourselves
			if (Widget == this)
			{
				return;
			}

			if (UNaviCompositeBase* ChildComposite = Cast<UNaviCompositeBase>(Widget))
			{
				Children.Add(ChildComposite);
				
				// Optional: Start collapsed by default? 
				// ChildComposite->SetIsCollapsed(true);
			}
		});
	}
}

void UNaviComposite::UpdateView(const ULyraInventoryItemDefinition* ItemDef)
{
	Super::UpdateView(ItemDef);

	for (UNaviCompositeBase* Child : Children)
	{
		if (Child)
		{
			Child->UpdateView(ItemDef);
		}
	}
}

void UNaviComposite::SetIsCollapsed(bool bCollapsed)
{
	// Collapsing the container effectively hides all children visually,
	// but we might want to explicitly collapse children logic if needed.
	Super::SetIsCollapsed(bCollapsed);
}
