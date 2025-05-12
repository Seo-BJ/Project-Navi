// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentIcon.h"

#include "Engine/Texture2D.h"
#include "Components/Image.h"

void UNaviAgentIcon::UpdateAgentIcomImage(TSoftObjectPtr<UTexture2D> AgentImage)
{
	if (AgentImage.IsNull()) return;
	AgentIcon_Image->SetBrushFromSoftTexture(AgentImage, false);
	
}
