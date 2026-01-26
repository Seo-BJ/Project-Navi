// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Composite/NaviWidget_DamageColumn.h"
#include "Components/TextBlock.h"

void UNaviWidget_DamageColumn::SetFalloffData(const FDamageFalloff& Data, bool bIsLastRange)
{
	if (Text_Range)
	{
		// 거리 포맷: "0 ~ 10m" 또는 마지막 구간 "30m ~"
		FString RangeStr;
		// 소수점 제거 (정수로 표시)
		int32 Start = FMath::RoundToInt(Data.RangeStart);
		int32 End = FMath::RoundToInt(Data.RangeEnd);

		if (bIsLastRange)
		{
			// 마지막 구간 표기법 (예: 50m +)
			RangeStr = FString::Printf(TEXT("%dm +"), Start);
		}
		else
		{
			RangeStr = FString::Printf(TEXT("%d ~ %dm"), Start, End);
		}
		Text_Range->SetText(FText::FromString(RangeStr));
	}

	// 데미지 수치 설정 (소수점 1자리까지 표시하거나 정수로 표시)
	if (Text_HeadDamage)
	{
		Text_HeadDamage->SetText(FText::AsNumber(Data.HeadShotDamage));
	}
	if (Text_BodyDamage)
	{
		Text_BodyDamage->SetText(FText::AsNumber(Data.BodyShotDamage));
	}
	if (Text_LegDamage)
	{
		Text_LegDamage->SetText(FText::AsNumber(Data.LegShotDamage));
	}
}
