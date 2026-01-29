// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/Leaf/NaviLeaf_ArmorDisplay.h"
#include "LyraGameplayTags.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Equipment/Armor/NaviArmorStatDefinition.h"
#include "Weapons/NaviWeaponWidgetDefinition.h"
#include "DataRegistryId.h"
#include "DataRegistrySubsystem.h"
#include "CredsSystem/NaviCredsShopComponent.h"

void UNaviLeaf_ArmorDisplay::UpdateArmorStats(const FNaviArmorStatDefinition& ArmorData)
{
	Super::UpdateArmorStats(ArmorData);
	
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GS = World->GetGameState())
		{
			if (UNaviCredsShopComponent* ShopComp = GS->GetComponentByClass<UNaviCredsShopComponent>())
			{
				const UDataTable* WeaponArmorDataTable = ShopComp->GetWeaponArmorWidgetTable();
				if (IsValid(WeaponArmorDataTable))
				{
					const FName RowName = ArmorData.ArmorTag.GetTagName();
					const FNaviWeaponWidgetDefinition* Row = WeaponArmorDataTable->FindRow<FNaviWeaponWidgetDefinition>(RowName, TEXT("FindWeaponCost"));
					if (Row)
					{

						if (Text_ArmorName)
						{
							Text_ArmorName->SetText(Row->DisplayName);
						}
						
						if (Image_Armor)
						{
							Image_Armor->SetBrushFromTexture(Row->WidgetIcon);
						}
					
					}
				}
			}
		}
	}
}



