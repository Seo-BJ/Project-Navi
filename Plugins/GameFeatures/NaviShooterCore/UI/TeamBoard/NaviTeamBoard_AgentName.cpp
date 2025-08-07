// Copyright 2024, NAVER Z Corporation
// All rights reserved.


#include "UI/TeamBoard/NaviTeamBoard_AgentName.h"
#include "Components/CommonTextBlock.h"

void UNaviTeamBoard_AgentName::SetAgentName(const FText& NewName)
{
	if (AgentName_TextBlock)
	{
		AgentName_TextBlock->SetText(NewName);
	}
}
