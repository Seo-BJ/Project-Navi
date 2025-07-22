// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/Equipment/NaviQuickBarComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviQuickBarComponent() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
LYRAGAME_API UClass* Z_Construct_UClass_ULyraEquipmentInstance_NoRegister();
LYRAGAME_API UClass* Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister();
MODULARGAMEPLAY_API UClass* Z_Construct_UClass_UControllerComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviQuickBarComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviQuickBarComponent_NoRegister();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin ScriptStruct FNaviQuickBarSlotsChangedMessage
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage;
class UScriptStruct* FNaviQuickBarSlotsChangedMessage::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("NaviQuickBarSlotsChangedMessage"));
	}
	return Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FNaviQuickBarSlotsChangedMessage>()
{
	return FNaviQuickBarSlotsChangedMessage::StaticStruct();
}
struct Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\x9d\xb4 \xec\xbb\xb4\xed\x8f\xac\xeb\x84\x8c\xed\x8a\xb8\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf \xec\xa0\x95\xeb\xb3\xb4\xea\xb0\x80 \xeb\xb3\x80\xea\xb2\xbd\xeb\x90\xa0 \xeb\x95\x8c \xeb\xb8\x8c\xeb\xa1\x9c\xeb\x93\x9c\xec\xba\x90\xec\x8a\xa4\xed\x8a\xb8\xeb\x90\x98\xeb\x8a\x94 \xeb\xa9\x94\xec\x8b\x9c\xec\xa7\x80\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x9d\xb4 \xec\xbb\xb4\xed\x8f\xac\xeb\x84\x8c\xed\x8a\xb8\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf \xec\xa0\x95\xeb\xb3\xb4\xea\xb0\x80 \xeb\xb3\x80\xea\xb2\xbd\xeb\x90\xa0 \xeb\x95\x8c \xeb\xb8\x8c\xeb\xa1\x9c\xeb\x93\x9c\xec\xba\x90\xec\x8a\xa4\xed\x8a\xb8\xeb\x90\x98\xeb\x8a\x94 \xeb\xa9\x94\xec\x8b\x9c\xec\xa7\x80\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Owner_MetaData[] = {
		{ "Category", "Inventory" },
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Slots_MetaData[] = {
		{ "Category", "Inventory" },
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Owner;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Slots_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_Slots;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FNaviQuickBarSlotsChangedMessage>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Owner = { "Owner", nullptr, (EPropertyFlags)0x0114000000000014, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviQuickBarSlotsChangedMessage, Owner), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Owner_MetaData), NewProp_Owner_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Slots_Inner = { "Slots", nullptr, (EPropertyFlags)0x0104000000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Slots = { "Slots", nullptr, (EPropertyFlags)0x0114000000000014, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviQuickBarSlotsChangedMessage, Slots), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Slots_MetaData), NewProp_Slots_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Owner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Slots_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewProp_Slots,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	nullptr,
	&NewStructOps,
	"NaviQuickBarSlotsChangedMessage",
	Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::PropPointers),
	sizeof(FNaviQuickBarSlotsChangedMessage),
	alignof(FNaviQuickBarSlotsChangedMessage),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage()
{
	if (!Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.InnerSingleton, Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage.InnerSingleton;
}
// End ScriptStruct FNaviQuickBarSlotsChangedMessage

// Begin ScriptStruct FNaviQuickBarActiveIndexChangedMessage
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage;
class UScriptStruct* FNaviQuickBarActiveIndexChangedMessage::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("NaviQuickBarActiveIndexChangedMessage"));
	}
	return Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FNaviQuickBarActiveIndexChangedMessage>()
{
	return FNaviQuickBarActiveIndexChangedMessage::StaticStruct();
}
struct Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xea\xb0\x80 \xeb\xb3\x80\xea\xb2\xbd\xeb\x90\xa0 \xeb\x95\x8c \xeb\xb8\x8c\xeb\xa1\x9c\xeb\x93\x9c\xec\xba\x90\xec\x8a\xa4\xed\x8a\xb8\xeb\x90\x98\xeb\x8a\x94 \xeb\xa9\x94\xec\x8b\x9c\xec\xa7\x80\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xea\xb0\x80 \xeb\xb3\x80\xea\xb2\xbd\xeb\x90\xa0 \xeb\x95\x8c \xeb\xb8\x8c\xeb\xa1\x9c\xeb\x93\x9c\xec\xba\x90\xec\x8a\xa4\xed\x8a\xb8\xeb\x90\x98\xeb\x8a\x94 \xeb\xa9\x94\xec\x8b\x9c\xec\xa7\x80\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Owner_MetaData[] = {
		{ "Category", "Inventory" },
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ActiveIndex_MetaData[] = {
		{ "Category", "Inventory" },
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Owner;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ActiveIndex;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FNaviQuickBarActiveIndexChangedMessage>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::NewProp_Owner = { "Owner", nullptr, (EPropertyFlags)0x0114000000000014, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviQuickBarActiveIndexChangedMessage, Owner), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Owner_MetaData), NewProp_Owner_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::NewProp_ActiveIndex = { "ActiveIndex", nullptr, (EPropertyFlags)0x0010000000000014, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviQuickBarActiveIndexChangedMessage, ActiveIndex), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ActiveIndex_MetaData), NewProp_ActiveIndex_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::NewProp_Owner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::NewProp_ActiveIndex,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	nullptr,
	&NewStructOps,
	"NaviQuickBarActiveIndexChangedMessage",
	Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::PropPointers),
	sizeof(FNaviQuickBarActiveIndexChangedMessage),
	alignof(FNaviQuickBarActiveIndexChangedMessage),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage()
{
	if (!Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.InnerSingleton, Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage.InnerSingleton;
}
// End ScriptStruct FNaviQuickBarActiveIndexChangedMessage

// Begin Class UNaviQuickBarComponent Function AddItemToSlot
struct Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics
{
	struct NaviQuickBarComponent_eventAddItemToSlot_Parms
	{
		FGameplayTag ItemTag;
		ULyraInventoryItemInstance* Item;
		int32 ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xed\x83\x80\xec\x9e\x85\xec\x97\x90 \xeb\xa7\x9e\xeb\x8a\x94 \xec\x98\xac\xeb\xb0\x94\xeb\xa5\xb8 \xec\x8a\xac\xeb\xa1\xaf\xec\x97\x90 \xec\xb6\x94\xea\xb0\x80\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 * \xec\x84\x9c\xeb\xb2\x84 \xea\xb6\x8c\xed\x95\x9c\xec\x9d\xb4 \xed\x95\x84\xec\x9a\x94\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 * @param Item \xec\xb6\x94\xea\xb0\x80\xed\x95\xa0 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xed\x83\x80\xec\x9e\x85\xec\x97\x90 \xeb\xa7\x9e\xeb\x8a\x94 \xec\x98\xac\xeb\xb0\x94\xeb\xa5\xb8 \xec\x8a\xac\xeb\xa1\xaf\xec\x97\x90 \xec\xb6\x94\xea\xb0\x80\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\xec\x84\x9c\xeb\xb2\x84 \xea\xb6\x8c\xed\x95\x9c\xec\x9d\xb4 \xed\x95\x84\xec\x9a\x94\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n@param Item \xec\xb6\x94\xea\xb0\x80\xed\x95\xa0 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ItemTag_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_ItemTag;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Item;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_ItemTag = { "ItemTag", nullptr, (EPropertyFlags)0x0010000008000182, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventAddItemToSlot_Parms, ItemTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ItemTag_MetaData), NewProp_ItemTag_MetaData) }; // 1298103297
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_Item = { "Item", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventAddItemToSlot_Parms, Item), Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventAddItemToSlot_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_ItemTag,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_Item,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "AddItemToSlot", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NaviQuickBarComponent_eventAddItemToSlot_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420405, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::NaviQuickBarComponent_eventAddItemToSlot_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execAddItemToSlot)
{
	P_GET_STRUCT_REF(FGameplayTag,Z_Param_Out_ItemTag);
	P_GET_OBJECT(ULyraInventoryItemInstance,Z_Param_Item);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=P_THIS->AddItemToSlot(Z_Param_Out_ItemTag,Z_Param_Item);
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function AddItemToSlot

// Begin Class UNaviQuickBarComponent Function CycleActiveSlotBackward
struct Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xec\x9d\xb4\xec\xa0\x84 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\x9e\xa5\xec\xb0\xa9\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x9d\xb4\xec\xa0\x84 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\x9e\xa5\xec\xb0\xa9\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "CycleActiveSlotBackward", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execCycleActiveSlotBackward)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->CycleActiveSlotBackward();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function CycleActiveSlotBackward

// Begin Class UNaviQuickBarComponent Function CycleActiveSlotForward
struct Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xeb\x8b\xa4\xec\x9d\x8c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\x9e\xa5\xec\xb0\xa9\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8b\xa4\xec\x9d\x8c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\x9e\xa5\xec\xb0\xa9\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "CycleActiveSlotForward", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execCycleActiveSlotForward)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->CycleActiveSlotForward();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function CycleActiveSlotForward

// Begin Class UNaviQuickBarComponent Function GetActiveSlotIndex
struct Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics
{
	struct NaviQuickBarComponent_eventGetActiveSlotIndex_Parms
	{
		int32 ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventGetActiveSlotIndex_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "GetActiveSlotIndex", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::NaviQuickBarComponent_eventGetActiveSlotIndex_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x44020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::NaviQuickBarComponent_eventGetActiveSlotIndex_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execGetActiveSlotIndex)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=P_THIS->GetActiveSlotIndex();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function GetActiveSlotIndex

// Begin Class UNaviQuickBarComponent Function GetActiveSlotItem
struct Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics
{
	struct NaviQuickBarComponent_eventGetActiveSlotItem_Parms
	{
		ULyraInventoryItemInstance* ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventGetActiveSlotItem_Parms, ReturnValue), Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "GetActiveSlotItem", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::NaviQuickBarComponent_eventGetActiveSlotItem_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x44020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::NaviQuickBarComponent_eventGetActiveSlotItem_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execGetActiveSlotItem)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(ULyraInventoryItemInstance**)Z_Param__Result=P_THIS->GetActiveSlotItem();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function GetActiveSlotItem

// Begin Class UNaviQuickBarComponent Function GetSlots
struct Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics
{
	struct NaviQuickBarComponent_eventGetSlots_Parms
	{
		TArray<ULyraInventoryItemInstance*> ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xeb\xaa\xa8\xeb\x93\xa0 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4 \xeb\xb0\xb0\xec\x97\xb4\xec\x9d\x84 \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xaa\xa8\xeb\x93\xa0 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4 \xeb\xb0\xb0\xec\x97\xb4\xec\x9d\x84 \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ReturnValue_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ReturnValue_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NewProp_ReturnValue_Inner = { "ReturnValue", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000008000582, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventGetSlots_Parms, ReturnValue), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ReturnValue_MetaData), NewProp_ReturnValue_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NewProp_ReturnValue_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "GetSlots", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NaviQuickBarComponent_eventGetSlots_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x54020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::NaviQuickBarComponent_eventGetSlots_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execGetSlots)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<ULyraInventoryItemInstance*>*)Z_Param__Result=P_THIS->GetSlots();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function GetSlots

// Begin Class UNaviQuickBarComponent Function OnRep_ActiveSlotIndex
struct Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "OnRep_ActiveSlotIndex", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00080401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execOnRep_ActiveSlotIndex)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnRep_ActiveSlotIndex();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function OnRep_ActiveSlotIndex

// Begin Class UNaviQuickBarComponent Function OnRep_Slots
struct Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "OnRep_Slots", nullptr, nullptr, nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00080401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots_Statics::Function_MetaDataParams) };
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execOnRep_Slots)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnRep_Slots();
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function OnRep_Slots

// Begin Class UNaviQuickBarComponent Function RemoveItemFromSlot
struct Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics
{
	struct NaviQuickBarComponent_eventRemoveItemFromSlot_Parms
	{
		int32 SlotIndex;
		ULyraInventoryItemInstance* ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * \xec\xa7\x80\xec\xa0\x95\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x97\x90\xec\x84\x9c \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\xa0\x9c\xea\xb1\xb0\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 * \xec\x84\x9c\xeb\xb2\x84 \xea\xb6\x8c\xed\x95\x9c\xec\x9d\xb4 \xed\x95\x84\xec\x9a\x94\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 * @return \xec\xa0\x9c\xea\xb1\xb0\xeb\x90\x9c \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\xa7\x80\xec\xa0\x95\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x97\x90\xec\x84\x9c \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c\xec\x9d\x84 \xec\xa0\x9c\xea\xb1\xb0\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n\xec\x84\x9c\xeb\xb2\x84 \xea\xb6\x8c\xed\x95\x9c\xec\x9d\xb4 \xed\x95\x84\xec\x9a\x94\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n@return \xec\xa0\x9c\xea\xb1\xb0\xeb\x90\x9c \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xeb\xa5\xbc \xeb\xb0\x98\xed\x99\x98\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_SlotIndex;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NewProp_SlotIndex = { "SlotIndex", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventRemoveItemFromSlot_Parms, SlotIndex), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventRemoveItemFromSlot_Parms, ReturnValue), Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NewProp_SlotIndex,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "RemoveItemFromSlot", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NaviQuickBarComponent_eventRemoveItemFromSlot_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020405, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::NaviQuickBarComponent_eventRemoveItemFromSlot_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execRemoveItemFromSlot)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_SlotIndex);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(ULyraInventoryItemInstance**)Z_Param__Result=P_THIS->RemoveItemFromSlot(Z_Param_SlotIndex);
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function RemoveItemFromSlot

// Begin Class UNaviQuickBarComponent Function SetActiveSlotIndex
struct NaviQuickBarComponent_eventSetActiveSlotIndex_Parms
{
	int32 NewIndex;
};
static const FName NAME_UNaviQuickBarComponent_SetActiveSlotIndex = FName(TEXT("SetActiveSlotIndex"));
void UNaviQuickBarComponent::SetActiveSlotIndex(int32 NewIndex)
{
	NaviQuickBarComponent_eventSetActiveSlotIndex_Parms Parms;
	Parms.NewIndex=NewIndex;
	UFunction* Func = FindFunctionChecked(NAME_UNaviQuickBarComponent_SetActiveSlotIndex);
	ProcessEvent(Func,&Parms);
}
struct Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|QuickBar" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xec\xa7\x80\xec\xa0\x95\xeb\x90\x9c \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x84 \xed\x99\x9c\xec\x84\xb1\xed\x99\x94(\xec\x9e\xa5\xec\xb0\xa9)\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c\xeb\xa7\x8c \xed\x98\xb8\xec\xb6\x9c\xed\x95\xb4\xec\x95\xbc \xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\xa7\x80\xec\xa0\x95\xeb\x90\x9c \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x84 \xed\x99\x9c\xec\x84\xb1\xed\x99\x94(\xec\x9e\xa5\xec\xb0\xa9)\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c\xeb\xa7\x8c \xed\x98\xb8\xec\xb6\x9c\xed\x95\xb4\xec\x95\xbc \xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewIndex;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::NewProp_NewIndex = { "NewIndex", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviQuickBarComponent_eventSetActiveSlotIndex_Parms, NewIndex), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::NewProp_NewIndex,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviQuickBarComponent, nullptr, "SetActiveSlotIndex", nullptr, nullptr, Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::PropPointers), sizeof(NaviQuickBarComponent_eventSetActiveSlotIndex_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04220CC0, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::Function_MetaDataParams) };
static_assert(sizeof(NaviQuickBarComponent_eventSetActiveSlotIndex_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviQuickBarComponent::execSetActiveSlotIndex)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_NewIndex);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetActiveSlotIndex_Implementation(Z_Param_NewIndex);
	P_NATIVE_END;
}
// End Class UNaviQuickBarComponent Function SetActiveSlotIndex

// Begin Class UNaviQuickBarComponent
void UNaviQuickBarComponent::StaticRegisterNativesUNaviQuickBarComponent()
{
	UClass* Class = UNaviQuickBarComponent::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "AddItemToSlot", &UNaviQuickBarComponent::execAddItemToSlot },
		{ "CycleActiveSlotBackward", &UNaviQuickBarComponent::execCycleActiveSlotBackward },
		{ "CycleActiveSlotForward", &UNaviQuickBarComponent::execCycleActiveSlotForward },
		{ "GetActiveSlotIndex", &UNaviQuickBarComponent::execGetActiveSlotIndex },
		{ "GetActiveSlotItem", &UNaviQuickBarComponent::execGetActiveSlotItem },
		{ "GetSlots", &UNaviQuickBarComponent::execGetSlots },
		{ "OnRep_ActiveSlotIndex", &UNaviQuickBarComponent::execOnRep_ActiveSlotIndex },
		{ "OnRep_Slots", &UNaviQuickBarComponent::execOnRep_Slots },
		{ "RemoveItemFromSlot", &UNaviQuickBarComponent::execRemoveItemFromSlot },
		{ "SetActiveSlotIndex", &UNaviQuickBarComponent::execSetActiveSlotIndex },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviQuickBarComponent);
UClass* Z_Construct_UClass_UNaviQuickBarComponent_NoRegister()
{
	return UNaviQuickBarComponent::StaticClass();
}
struct Z_Construct_UClass_UNaviQuickBarComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xeb\xb0\x9c\xeb\xa1\x9c\xeb\x9e\x80\xed\x8a\xb8\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf \xea\xb7\x9c\xec\xb9\x99\xec\x9d\x84 \xeb\x94\xb0\xeb\xa5\xb4\xeb\x8a\x94 QuickBarComponent\n * 4\xea\xb0\x9c\xec\x9d\x98 \xea\xb3\xa0\xec\xa0\x95\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9c\xbc\xeb\xa1\x9c, \xed\x8a\xb9\xec\xa0\x95 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c(Primary, Sidearms, Melee, Spike) \xed\x83\x80\xec\x9e\x85\xeb\xa7\x8c \xed\x97\x88\xec\x9a\xa9\n */" },
#endif
		{ "HideCategories", "Trigger PhysicsVolume" },
		{ "IncludePath", "Equipment/NaviQuickBarComponent.h" },
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xb0\x9c\xeb\xa1\x9c\xeb\x9e\x80\xed\x8a\xb8\xec\x9d\x98 \xec\x8a\xac\xeb\xa1\xaf \xea\xb7\x9c\xec\xb9\x99\xec\x9d\x84 \xeb\x94\xb0\xeb\xa5\xb4\xeb\x8a\x94 QuickBarComponent\n4\xea\xb0\x9c\xec\x9d\x98 \xea\xb3\xa0\xec\xa0\x95\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9c\xbc\xeb\xa1\x9c, \xed\x8a\xb9\xec\xa0\x95 \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c(Primary, Sidearms, Melee, Spike) \xed\x83\x80\xec\x9e\x85\xeb\xa7\x8c \xed\x97\x88\xec\x9a\xa9" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NumSlots_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x80\xb5\xeb\xb0\x94\xec\x9d\x98 \xec\xb4\x9d \xec\x8a\xac\xeb\xa1\xaf \xec\x88\x98. \xeb\xb0\x9c\xeb\xa1\x9c\xeb\x9e\x80\xed\x8a\xb8 \xea\xb7\x9c\xec\xb9\x99\xec\x97\x90 \xeb\x94\xb0\xeb\x9d\xbc 4\xea\xb0\x9c\xeb\xa1\x9c \xea\xb3\xa0\xec\xa0\x95\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x80\xb5\xeb\xb0\x94\xec\x9d\x98 \xec\xb4\x9d \xec\x8a\xac\xeb\xa1\xaf \xec\x88\x98. \xeb\xb0\x9c\xeb\xa1\x9c\xeb\x9e\x80\xed\x8a\xb8 \xea\xb7\x9c\xec\xb9\x99\xec\x97\x90 \xeb\x94\xb0\xeb\x9d\xbc 4\xea\xb0\x9c\xeb\xa1\x9c \xea\xb3\xa0\xec\xa0\x95\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Slots_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xec\x8a\xac\xeb\xa1\xaf \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xeb\xaa\xa9\xeb\xa1\x9d. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c \xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xeb\xa1\x9c \xeb\xb3\xb5\xec\xa0\x9c\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x8a\xac\xeb\xa1\xaf \xec\x95\x84\xec\x9d\xb4\xed\x85\x9c \xeb\xaa\xa9\xeb\xa1\x9d. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c \xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xeb\xa1\x9c \xeb\xb3\xb5\xec\xa0\x9c\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ActiveSlotIndex_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c \xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xeb\xa1\x9c \xeb\xb3\xb5\xec\xa0\x9c\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x98\x84\xec\x9e\xac \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xeb\x90\x9c \xec\x8a\xac\xeb\xa1\xaf\xec\x9d\x98 \xec\x9d\xb8\xeb\x8d\xb1\xec\x8a\xa4. \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90\xec\x84\x9c \xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xeb\xa1\x9c \xeb\xb3\xb5\xec\xa0\x9c\xeb\x90\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EquippedItem_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x98\x84\xec\x9e\xac \xec\x9e\xa5\xec\xb0\xa9\xeb\x90\x9c \xec\x9e\xa5\xeb\xb9\x84\xec\x9d\x98 \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4. */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/NaviQuickBarComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x98\x84\xec\x9e\xac \xec\x9e\xa5\xec\xb0\xa9\xeb\x90\x9c \xec\x9e\xa5\xeb\xb9\x84\xec\x9d\x98 \xec\x9d\xb8\xec\x8a\xa4\xed\x84\xb4\xec\x8a\xa4\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NumSlots;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Slots_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_Slots;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ActiveSlotIndex;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_EquippedItem;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_AddItemToSlot, "AddItemToSlot" }, // 1439117953
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotBackward, "CycleActiveSlotBackward" }, // 454980944
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_CycleActiveSlotForward, "CycleActiveSlotForward" }, // 3199413929
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotIndex, "GetActiveSlotIndex" }, // 1455074160
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_GetActiveSlotItem, "GetActiveSlotItem" }, // 1863792902
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_GetSlots, "GetSlots" }, // 2126235276
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_ActiveSlotIndex, "OnRep_ActiveSlotIndex" }, // 3934661719
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_OnRep_Slots, "OnRep_Slots" }, // 2463863645
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_RemoveItemFromSlot, "RemoveItemFromSlot" }, // 2354203784
		{ &Z_Construct_UFunction_UNaviQuickBarComponent_SetActiveSlotIndex, "SetActiveSlotIndex" }, // 866115975
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviQuickBarComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_NumSlots = { "NumSlots", nullptr, (EPropertyFlags)0x0020080000000000, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviQuickBarComponent, NumSlots), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NumSlots_MetaData), NewProp_NumSlots_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_Slots_Inner = { "Slots", nullptr, (EPropertyFlags)0x0104000000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UClass_ULyraInventoryItemInstance_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_Slots = { "Slots", "OnRep_Slots", (EPropertyFlags)0x0144000100000020, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviQuickBarComponent, Slots), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Slots_MetaData), NewProp_Slots_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_ActiveSlotIndex = { "ActiveSlotIndex", "OnRep_ActiveSlotIndex", (EPropertyFlags)0x0040000100000020, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviQuickBarComponent, ActiveSlotIndex), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ActiveSlotIndex_MetaData), NewProp_ActiveSlotIndex_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_EquippedItem = { "EquippedItem", nullptr, (EPropertyFlags)0x0144000000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviQuickBarComponent, EquippedItem), Z_Construct_UClass_ULyraEquipmentInstance_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EquippedItem_MetaData), NewProp_EquippedItem_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNaviQuickBarComponent_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_NumSlots,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_Slots_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_Slots,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_ActiveSlotIndex,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviQuickBarComponent_Statics::NewProp_EquippedItem,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviQuickBarComponent_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNaviQuickBarComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UControllerComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviQuickBarComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviQuickBarComponent_Statics::ClassParams = {
	&UNaviQuickBarComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UNaviQuickBarComponent_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UNaviQuickBarComponent_Statics::PropPointers),
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviQuickBarComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviQuickBarComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviQuickBarComponent()
{
	if (!Z_Registration_Info_UClass_UNaviQuickBarComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviQuickBarComponent.OuterSingleton, Z_Construct_UClass_UNaviQuickBarComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviQuickBarComponent.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviQuickBarComponent>()
{
	return UNaviQuickBarComponent::StaticClass();
}
void UNaviQuickBarComponent::ValidateGeneratedRepEnums(const TArray<struct FRepRecord>& ClassReps) const
{
	static const FName Name_Slots(TEXT("Slots"));
	static const FName Name_ActiveSlotIndex(TEXT("ActiveSlotIndex"));
	const bool bIsValid = true
		&& Name_Slots == ClassReps[(int32)ENetFields_Private::Slots].Property->GetFName()
		&& Name_ActiveSlotIndex == ClassReps[(int32)ENetFields_Private::ActiveSlotIndex].Property->GetFName();
	checkf(bIsValid, TEXT("UHT Generated Rep Indices do not match runtime populated Rep Indices for properties in UNaviQuickBarComponent"));
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviQuickBarComponent);
UNaviQuickBarComponent::~UNaviQuickBarComponent() {}
// End Class UNaviQuickBarComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FNaviQuickBarSlotsChangedMessage::StaticStruct, Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics::NewStructOps, TEXT("NaviQuickBarSlotsChangedMessage"), &Z_Registration_Info_UScriptStruct_NaviQuickBarSlotsChangedMessage, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FNaviQuickBarSlotsChangedMessage), 1701665180U) },
		{ FNaviQuickBarActiveIndexChangedMessage::StaticStruct, Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics::NewStructOps, TEXT("NaviQuickBarActiveIndexChangedMessage"), &Z_Registration_Info_UScriptStruct_NaviQuickBarActiveIndexChangedMessage, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FNaviQuickBarActiveIndexChangedMessage), 2176818479U) },
	};
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviQuickBarComponent, UNaviQuickBarComponent::StaticClass, TEXT("UNaviQuickBarComponent"), &Z_Registration_Info_UClass_UNaviQuickBarComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviQuickBarComponent), 3231975952U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_2157381560(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_Statics::ClassInfo),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_Statics::ScriptStructInfo),
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
