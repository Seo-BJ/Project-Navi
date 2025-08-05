// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraInventoryItemDefinition.h"
#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"

#include "LyraInventoryItemInstance.generated.h"

class FLifetimeProperty;

class ULyraInventoryItemDefinition;
class ULyraInventoryItemFragment;
struct FFrame;
struct FGameplayTag;

/**
 * ULyraInventoryItemInstance
 */
UCLASS(BlueprintType)
class LYRAGAME_API ULyraInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	ULyraInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category=Inventory)
	TSubclassOf<ULyraInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	UFUNCTION(BlueprintCallable, Category=Inventory)
	FGameplayTag GetItemTag() const
	{
		if (ItemDef)
		{
			return ItemDef->GetDefaultObject<ULyraInventoryItemDefinition>()->ItemTag;
		}
		return FGameplayTag::EmptyTag;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const ULyraInventoryItemFragment* FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

private:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	void SetItemDef(TSubclassOf<ULyraInventoryItemDefinition> InDef);

	friend struct FLyraInventoryList;

protected:
	UFUNCTION()
	void OnRep_StatTags();

private:
    UPROPERTY(ReplicatedUsing = OnRep_StatTags)
    FGameplayTagStackContainer StatTags;

	// The item definition
	UPROPERTY(Replicated)
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;
	
};


// StatTags 변경 메시지를 위한 구조체 정의
USTRUCT(BlueprintType)
struct FLyraInventory_Message_StatTagsChanged
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	FGameplayTagStackContainer StatTags;
};