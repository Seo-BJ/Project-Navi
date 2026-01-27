// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Composite/NaviCompositeBase.h"
#include "Weapons/NaviWeaponStatDefinition.h"
#include "NaviLeaf_WeaponNameDisplay.generated.h"

class UTextBlock;
class UImage;

/**
 * 무기의 이름, 발사 방식(자동/반자동), 관통력을 표시하는 리프 위젯입니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API UNaviLeaf_WeaponNameDisplay : public UNaviCompositeBase
{
	GENERATED_BODY()

public:
	//~ Begin UNaviCompositeBase Interface
	virtual void UpdateWeaponStats(const FNaviWeaponStatDefinition& WeaponData) override;
	FText GetFireModeText(ENaviWeaponPenetrationPower Power) const;
	FText GetFireModeText(ENaviWeaponFireMode Mode) const;
	FText GetWeaponTypeText(FGameplayTag WeaponTag) const;
	//~ End UNaviCompositeBase Interface

protected:
	// 무기 이름
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WeaponName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WeaponType;

	// 관통력 (예: "High")
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> Text_Penetration;

	// 발사 방식 (예: "Automatic")
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> Text_FireMode;
	
	// 발사 방식 아이콘 (Optional)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> Image_FireMode;
	

};
