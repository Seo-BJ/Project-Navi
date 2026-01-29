// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreBoard/PlayerRow/NaviScoreBoard_LoadOut.h"

#include "LyraGameplayTags.h"
#include "LyraLogChannels.h"
#include "Equipment/NaviQuickBarComponent.h"
#include "Player/LyraPlayerState.h"
#include "Weapons/NaviWeaponWidgetDefinition.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"

#include "DataRegistryTypes.h"
#include "DataRegistrySubsystem.h"


void UNaviScoreBoard_LoadOut::SetPlayerState_Implementation(ALyraPlayerState* InPlayerState)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		UNaviQuickBarComponent* QuickBarComponent = PlayerController->GetComponentByClass<UNaviQuickBarComponent>();
		if (QuickBarComponent)
		{
			ULyraInventoryItemInstance* LowestWeaponInstance = nullptr;
			if (QuickBarComponent->GetItemInSlot(0))
			{
				LowestWeaponInstance = QuickBarComponent->GetItemInSlot(0);
			}
			else
			{
				if (QuickBarComponent->GetItemInSlot(1))
				{
					LowestWeaponInstance = QuickBarComponent->GetItemInSlot(1);
				}
				else
				{
					// @Todo : Melee Weapon
				}
			}

			if (LowestWeaponInstance)
			{
				FDataRegistryId ItemID = FDataRegistryId("WeaponWidgetElements",LowestWeaponInstance->GetItemTag().GetTagName());
				if (!UDataRegistrySubsystem::Get()->AcquireItem(ItemID, FDataRegistryItemAcquiredCallback::CreateUObject(this, &ThisClass::OnRegistryLoadCompleted)))
				{
					UE_LOG(LogLyra, Error, TEXT("Failed to find accolade registry for tag %s, accolades will not appear"), *LowestWeaponInstance->GetItemTag().GetTagName().ToString());
				}
			}
		}
	}
}

void UNaviScoreBoard_LoadOut::OnRegistryLoadCompleted(const FDataRegistryAcquireResult& AcquireResult)
{
	if (const FNaviWeaponWidgetDefinition* WidgetRow = AcquireResult.GetItem<FNaviWeaponWidgetDefinition>())
	{
		if (WidgetRow)
		{
			UTexture2D* Texture = WidgetRow->WidgetIcon;
			if (Texture)
			{
				if (WidgetRow->WeaponTag.MatchesTag(NaviGameplayTags::Weapon))
				{
					float Ratio = Texture->GetSizeX() / Texture->GetSizeY();
					WeaponIcon_Image->SetDesiredSizeOverride(FVector2d(HeightOverride * Ratio, HeightOverride));
					WeaponIcon_Image->SetBrushFromTexture(Texture, false);

				}
				else if (WidgetRow->WeaponTag.MatchesTag(NaviGameplayTags::Armor))
				{
					ArmorIcon_Image->SetDesiredSizeOverride(FVector2d(HeightOverride, HeightOverride));
					ArmorIcon_Image->SetBrushFromTexture(Texture);
				}
			}
		}
	}
}
