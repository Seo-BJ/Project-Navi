// Copyright 2024, NAVER Z Corporation
// All rights reserved.


#include "UI/TeamBoard/NaviTeamBoard_KDA.h"
#include "CommonNumericTextBlock.h"

void UNaviTeamBoard_KDA::SetKDA(int32 Kills, int32 Deaths, int32 Assists)
{
	if (Kills_TextBlock)
	{
		Kills_TextBlock->SetCurrentValue(Kills);
	}

	if (Deaths_TextBlock)
	{
		Deaths_TextBlock->SetCurrentValue(Deaths);
	}

	if (Assists_TextBlock)
	{
		Assists_TextBlock->SetCurrentValue(Assists);
	}
}
