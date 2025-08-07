// Copyright 2024, NAVER Z Corporation
// All rights reserved.


#include "UI/TeamBoard/NaviTeamBoard_Creds.h"
#include "Components/Image.h"
#include "CommonNumericTextBlock.h"

void UNaviTeamBoard_Creds::SetCreds(int32 NewCreds)
{
	if (CredsAmount_TextBlock)
	{
		CredsAmount_TextBlock->SetCurrentValue(NewCreds);
	}
}
