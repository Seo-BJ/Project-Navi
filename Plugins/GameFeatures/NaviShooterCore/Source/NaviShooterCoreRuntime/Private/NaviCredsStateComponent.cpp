// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviCredsStateComponent.h"

#include "LyraGameplayTags.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/LyraVerbMessage.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(NaviCredsStateComponent)

void UNaviCredsStateComponent::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	//AddListenerHandle(MessageSubsystem.RegisterListener(NaviGameplayTags::Navi_Kill_Message, this, &ThisClass::OnKillMessage));

}
/*
void UNaviCredsStateComponent::OnKillMessage(FGameplayTag Channel, const FLyraVerbMessage& Payload)
{
	// Track elimination chains for the attacker (except for self-eliminations)
	if (Payload.Instigator != Payload.Target)
	{
		if (APlayerState* InstigatorPS = Cast<APlayerState>(Payload.Instigator))
		{
			FLyraVerbMessage EliminationMessage;
			EliminationMessage.Verb = NaviGameplayTags::Navi_Creds_Award_Kill;
			EliminationMessage.Instigator = InstigatorPS;
			EliminationMessage.InstigatorTags = Payload.InstigatorTags;
			EliminationMessage.ContextTags = Payload.ContextTags;
			if (int32* AmountToAward = CredsAwardData.Find(NaviGameplayTags::Navi_Creds_Award_Kill))
			{
				EliminationMessage.Magnitude = *AmountToAward;
			}
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
			MessageSubsystem.BroadcastMessage(EliminationMessage.Verb, EliminationMessage);
		}
	}
}

*/