// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTaggedWidget.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Player/LyraPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTaggedWidget)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_AbilitySystem_Message_TagChanged, "Lyra.AbilitySystem.Message.TagChanged");

//@TODO: The other TODOs in this file are all related to tag-based showing/hiding of widgets, see UE-142237

ULyraTaggedWidget::ULyraTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraTaggedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		MessageListenerHandle = UGameplayMessageSubsystem::Get(GetWorld()).RegisterListener(
			TAG_Lyra_AbilitySystem_Message_TagChanged,
			this,
			&ULyraTaggedWidget::OnTagChangedMessage
		);

		// 위젯 초기 설정시 태그 기반 설정
		OnWatchedTagsChanged();
	}
}

void ULyraTaggedWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (MessageListenerHandle.IsValid())
		{
			UGameplayMessageSubsystem::Get(GetWorld()).UnregisterListener(MessageListenerHandle);
		}
	}
	Super::NativeDestruct();
}

void ULyraTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		Super::SetVisibility(InVisibility);
		return;
	}
#endif

	// Remember what the caller requested; even if we're currently being
	// suppressed by a tag we should respect this call when we're done
	bWantsToBeVisible = ConvertSerializedVisibilityToRuntime(InVisibility).IsVisible();
	if (bWantsToBeVisible)
	{
		ShownVisibility = InVisibility;
	}
	else
	{
		HiddenVisibility = InVisibility;
	}
	OnWatchedTagsChanged();
}


void ULyraTaggedWidget::OnTagChangedMessage(FGameplayTag Channel, const FLyraTagChangedMessage& Message)
{   
	if (Message.OwnerActor != nullptr)
	{
		if (GetOwningPlayerState() == Cast<APlayerState>(Message.OwnerActor))
		{
			if (HiddenByTags.HasTag(Message.UpdatedTag))
			{
				OnWatchedTagsChanged();
			}
		}
	}
}

void ULyraTaggedWidget::OnWatchedTagsChanged()
{
	bool bHasHiddenTags = false;
	if (const ALyraPlayerState* MyPlayerState = GetOwningPlayerState<ALyraPlayerState>())
	{
		if (const ULyraAbilitySystemComponent* ASC = MyPlayerState->GetLyraAbilitySystemComponent())
		{
			bHasHiddenTags = ASC->HasAnyMatchingGameplayTags(HiddenByTags);
		}
	}

	// 최종 Visibility 계산
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}

