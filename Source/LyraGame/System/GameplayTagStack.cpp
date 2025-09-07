// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTagStack.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Teams/LyraTeamSubsystem.h"
#include "UObject/Stack.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagStack)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_GameplayTagStack_Message_StackChanged, "Lyra.GameplayTagStack.Message.StackChanged");


//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

FString FGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
}

//////////////////////////////////////////////////////////////////////
// FGameplayTagStackContainer

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (StackCount > 0)
	{
		for (FGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				const int32 NewCount = Stack.StackCount + StackCount;
				Stack.StackCount = NewCount;
				TagToCountMap[Tag] = NewCount;
				MarkItemDirty(Stack);
				return;
			}
		}

		FGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		MarkItemDirty(NewStack);
		TagToCountMap.Add(Tag, StackCount);
	}
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	//@TODO: Should we error if you try to remove a stack that doesn't exist or has a smaller count?
	if (StackCount > 0)
	{
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				if (Stack.StackCount <= StackCount)
				{
					It.RemoveCurrent();
					TagToCountMap.Remove(Tag);
					MarkArrayDirty();
				}
				else
				{
					const int32 NewCount = Stack.StackCount - StackCount;
					Stack.StackCount = NewCount;
					TagToCountMap[Tag] = NewCount;
					MarkItemDirty(Stack);
				}
				return;
			}
		}
	}
}

void FGameplayTagStackContainer::RegisterWithOwner(UObject* InOwner)
{
	Owner = InOwner;
}

void FGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FGameplayTag Tag = Stacks[Index].Tag;
		int32 OldCount = Stacks[Index].StackCount;
		int32 NewCount = TagToCountMap.Remove(Tag);
		BroadcastChangeMessage(Tag, OldCount, NewCount);
	}
}

void FGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		int32 OldCount = Stacks[Index].StackCount;
		int32 NewCount = TagToCountMap.Add(Stack.Tag, Stack.StackCount);
		BroadcastChangeMessage(Stack.Tag, OldCount, NewCount);
	}
}

void FGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		int32 OldCount = Stacks[Index].StackCount;
		const FGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap[Stack.Tag] = Stack.StackCount;
		int32 NewCount = Stack.StackCount;
		BroadcastChangeMessage(Stack.Tag, OldCount, NewCount);
	}
}

void FGameplayTagStackContainer::BroadcastChangeMessage(const FGameplayTag& TagStack, int32 OldCount, int32 NewCount) const
{
	if (!IsValid(Owner))
	{
		return;
	}
	
	FLyraGameplayTagStackChangeMessage Message;
	Message.StackOwner = Owner;
	Message.GameplayTag = TagStack;
	Message.OldStackCount = OldCount;
	Message.NewStackCount = NewCount;
	
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_GameplayTagStack_Message_StackChanged, Message);
}
