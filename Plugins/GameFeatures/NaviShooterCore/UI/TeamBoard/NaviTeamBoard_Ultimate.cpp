// Copyright 2024, NAVER Z Corporation
// All rights reserved.


#include "UI/TeamBoard/NaviTeamBoard_Ultimate.h"
#include "CommonNumericTextBlock.h"

void UNaviTeamBoard_Ultimate::SetUltimatePoints(int32 CurrentPoints, int32 MaxPoints)
{
	if (UltimatePoint_TextBlock)
	{
		UltimatePoint_TextBlock->SetCurrentValue(CurrentPoints);
	}

	if (UltimateMaxPoint_TextBlock)
	{
		UltimateMaxPoint_TextBlock->SetCurrentValue(MaxPoints);
	}
}
