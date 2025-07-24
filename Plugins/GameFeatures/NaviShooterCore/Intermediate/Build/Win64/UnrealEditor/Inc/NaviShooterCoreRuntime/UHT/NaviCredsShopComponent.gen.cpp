// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/CredsSystem/NaviCredsShopComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviCredsShopComponent() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AController_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UDataTable_NoRegister();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
LYRAGAME_API UClass* Z_Construct_UClass_ULyraWeaponPickupDefinition_NoRegister();
MODULARGAMEPLAY_API UClass* Z_Construct_UClass_UGameStateComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsShopComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsShopComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviCredsShopComponent Function BuyEquipment
struct Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics
{
	struct NaviCredsShopComponent_eventBuyEquipment_Parms
	{
		AController* RequestingPlayerController;
		FGameplayTag EquipmentTag;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|Shop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n     * \xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xea\xb0\x80 \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90 \xeb\xac\xb4\xea\xb8\xb0 \xea\xb5\xac\xeb\xa7\xa4\xeb\xa5\xbc \xec\x9a\x94\xec\xb2\xad\xed\x95\xa0 \xeb\x95\x8c \xed\x98\xb8\xec\xb6\x9c\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n     * @param RequestingPlayerController \xea\xb5\xac\xeb\xa7\xa4\xeb\xa5\xbc \xec\x9a\x94\xec\xb2\xad\xed\x95\x98\xeb\x8a\x94 \xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4\xec\x96\xb4\xec\x9d\x98 \xec\xbb\xa8\xed\x8a\xb8\xeb\xa1\xa4\xeb\x9f\xac\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n     * @param EquipmentTag \xea\xb5\xac\xeb\xa7\xa4\xed\x95\x98\xeb\xa0\xa4\xeb\x8a\x94 \xeb\xac\xb4\xea\xb8\xb0\xec\x9d\x98 \xea\xb3\xa0\xec\x9c\xa0 \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n     */" },
#endif
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x81\xb4\xeb\x9d\xbc\xec\x9d\xb4\xec\x96\xb8\xed\x8a\xb8\xea\xb0\x80 \xec\x84\x9c\xeb\xb2\x84\xec\x97\x90 \xeb\xac\xb4\xea\xb8\xb0 \xea\xb5\xac\xeb\xa7\xa4\xeb\xa5\xbc \xec\x9a\x94\xec\xb2\xad\xed\x95\xa0 \xeb\x95\x8c \xed\x98\xb8\xec\xb6\x9c\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n@param RequestingPlayerController \xea\xb5\xac\xeb\xa7\xa4\xeb\xa5\xbc \xec\x9a\x94\xec\xb2\xad\xed\x95\x98\xeb\x8a\x94 \xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4\xec\x96\xb4\xec\x9d\x98 \xec\xbb\xa8\xed\x8a\xb8\xeb\xa1\xa4\xeb\x9f\xac\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n@param EquipmentTag \xea\xb5\xac\xeb\xa7\xa4\xed\x95\x98\xeb\xa0\xa4\xeb\x8a\x94 \xeb\xac\xb4\xea\xb8\xb0\xec\x9d\x98 \xea\xb3\xa0\xec\x9c\xa0 \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_RequestingPlayerController;
	static const UECodeGen_Private::FStructPropertyParams NewProp_EquipmentTag;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NewProp_RequestingPlayerController = { "RequestingPlayerController", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviCredsShopComponent_eventBuyEquipment_Parms, RequestingPlayerController), Z_Construct_UClass_AController_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NewProp_EquipmentTag = { "EquipmentTag", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviCredsShopComponent_eventBuyEquipment_Parms, EquipmentTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(0, nullptr) }; // 1298103297
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NewProp_RequestingPlayerController,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NewProp_EquipmentTag,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviCredsShopComponent, nullptr, "BuyEquipment", nullptr, nullptr, Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NaviCredsShopComponent_eventBuyEquipment_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::NaviCredsShopComponent_eventBuyEquipment_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviCredsShopComponent::execBuyEquipment)
{
	P_GET_OBJECT(AController,Z_Param_RequestingPlayerController);
	P_GET_STRUCT(FGameplayTag,Z_Param_EquipmentTag);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->BuyEquipment(Z_Param_RequestingPlayerController,Z_Param_EquipmentTag);
	P_NATIVE_END;
}
// End Class UNaviCredsShopComponent Function BuyEquipment

// Begin Class UNaviCredsShopComponent
void UNaviCredsShopComponent::StaticRegisterNativesUNaviCredsShopComponent()
{
	UClass* Class = UNaviCredsShopComponent::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "BuyEquipment", &UNaviCredsShopComponent::execBuyEquipment },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviCredsShopComponent);
UClass* Z_Construct_UClass_UNaviCredsShopComponent_NoRegister()
{
	return UNaviCredsShopComponent::StaticClass();
}
struct Z_Construct_UClass_UNaviCredsShopComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * GameState\xec\x97\x90 \xec\xb6\x94\xea\xb0\x80\xeb\x90\x98\xec\x96\xb4 \xeb\xac\xb4\xea\xb8\xb0 \xec\x83\x81\xec\xa0\x90 \xec\x97\xad\xed\x95\xa0\xec\x9d\x84 \xed\x95\x98\xeb\x8a\x94 \xec\xbb\xb4\xed\x8f\xac\xeb\x84\x8c\xed\x8a\xb8.\n * \xeb\xaa\xa8\xeb\x93\xa0 \xeb\xac\xb4\xea\xb8\xb0 \xec\xa0\x95\xeb\xb3\xb4\xeb\xa5\xbc \xea\xb4\x80\xeb\xa6\xac\xed\x95\x98\xea\xb3\xa0 \xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4\xec\x96\xb4\xec\x9d\x98 \xea\xb5\xac\xeb\xa7\xa4 \xec\x9a\x94\xec\xb2\xad\xec\x9d\x84 \xec\xb2\x98\xeb\xa6\xac\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4.\n */" },
#endif
		{ "HideCategories", "Trigger PhysicsVolume" },
		{ "IncludePath", "CredsSystem/NaviCredsShopComponent.h" },
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopComponent.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "GameState\xec\x97\x90 \xec\xb6\x94\xea\xb0\x80\xeb\x90\x98\xec\x96\xb4 \xeb\xac\xb4\xea\xb8\xb0 \xec\x83\x81\xec\xa0\x90 \xec\x97\xad\xed\x95\xa0\xec\x9d\x84 \xed\x95\x98\xeb\x8a\x94 \xec\xbb\xb4\xed\x8f\xac\xeb\x84\x8c\xed\x8a\xb8.\n\xeb\xaa\xa8\xeb\x93\xa0 \xeb\xac\xb4\xea\xb8\xb0 \xec\xa0\x95\xeb\xb3\xb4\xeb\xa5\xbc \xea\xb4\x80\xeb\xa6\xac\xed\x95\x98\xea\xb3\xa0 \xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4\xec\x96\xb4\xec\x9d\x98 \xea\xb5\xac\xeb\xa7\xa4 \xec\x9a\x94\xec\xb2\xad\xec\x9d\x84 \xec\xb2\x98\xeb\xa6\xac\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WeaponStatTable_MetaData[] = {
		{ "Category", "Navi|Shop|Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n     * \xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x83\xaf \xeb\xb0\x8f \xea\xb0\x80\xea\xb2\xa9 \xec\xa0\x95\xeb\xb3\xb4\xea\xb0\x80 \xeb\x93\xa4\xec\x96\xb4\xec\x9e\x88\xeb\x8a\x94 \xeb\x8d\xb0\xec\x9d\xb4\xed\x84\xb0 \xed\x85\x8c\xec\x9d\xb4\xeb\xb8\x94\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n     */" },
#endif
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x83\xaf \xeb\xb0\x8f \xea\xb0\x80\xea\xb2\xa9 \xec\xa0\x95\xeb\xb3\xb4\xea\xb0\x80 \xeb\x93\xa4\xec\x96\xb4\xec\x9e\x88\xeb\x8a\x94 \xeb\x8d\xb0\xec\x9d\xb4\xed\x84\xb0 \xed\x85\x8c\xec\x9d\xb4\xeb\xb8\x94\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ArmorStatTable_MetaData[] = {
		{ "Category", "Navi|Shop|Data" },
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WeaponDefinitionMap_MetaData[] = {
		{ "Category", "Navi|Shop|Data" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n     * \xeb\xb9\xa0\xeb\xa5\xb8 \xec\xa1\xb0\xed\x9a\x8c\xeb\xa5\xbc \xec\x9c\x84\xed\x95\xb4 GameplayTag\xeb\xa5\xbc \xed\x82\xa4\xeb\xa1\x9c \xec\x82\xac\xec\x9a\xa9\xed\x95\x98\xeb\x8a\x94 \xeb\xac\xb4\xea\xb8\xb0 \xec\xa0\x95\xec\x9d\x98 \xeb\xa7\xb5\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4.\n     */" },
#endif
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xb9\xa0\xeb\xa5\xb8 \xec\xa1\xb0\xed\x9a\x8c\xeb\xa5\xbc \xec\x9c\x84\xed\x95\xb4 GameplayTag\xeb\xa5\xbc \xed\x82\xa4\xeb\xa1\x9c \xec\x82\xac\xec\x9a\xa9\xed\x95\x98\xeb\x8a\x94 \xeb\xac\xb4\xea\xb8\xb0 \xec\xa0\x95\xec\x9d\x98 \xeb\xa7\xb5\xec\x9e\x85\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_WeaponStatTable;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ArmorStatTable;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_WeaponDefinitionMap_ValueProp;
	static const UECodeGen_Private::FStructPropertyParams NewProp_WeaponDefinitionMap_Key_KeyProp;
	static const UECodeGen_Private::FMapPropertyParams NewProp_WeaponDefinitionMap;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviCredsShopComponent_BuyEquipment, "BuyEquipment" }, // 1115016026
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviCredsShopComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponStatTable = { "WeaponStatTable", nullptr, (EPropertyFlags)0x0124080000010001, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsShopComponent, WeaponStatTable), Z_Construct_UClass_UDataTable_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WeaponStatTable_MetaData), NewProp_WeaponStatTable_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_ArmorStatTable = { "ArmorStatTable", nullptr, (EPropertyFlags)0x0124080000010001, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsShopComponent, ArmorStatTable), Z_Construct_UClass_UDataTable_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ArmorStatTable_MetaData), NewProp_ArmorStatTable_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap_ValueProp = { "WeaponDefinitionMap", nullptr, (EPropertyFlags)0x0104000000000001, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 1, Z_Construct_UClass_ULyraWeaponPickupDefinition_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap_Key_KeyProp = { "WeaponDefinitionMap_Key", nullptr, (EPropertyFlags)0x0100000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(0, nullptr) }; // 1298103297
const UECodeGen_Private::FMapPropertyParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap = { "WeaponDefinitionMap", nullptr, (EPropertyFlags)0x0124080000010001, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsShopComponent, WeaponDefinitionMap), EMapPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WeaponDefinitionMap_MetaData), NewProp_WeaponDefinitionMap_MetaData) }; // 1298103297
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNaviCredsShopComponent_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponStatTable,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_ArmorStatTable,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap_ValueProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap_Key_KeyProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsShopComponent_Statics::NewProp_WeaponDefinitionMap,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopComponent_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNaviCredsShopComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UGameStateComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviCredsShopComponent_Statics::ClassParams = {
	&UNaviCredsShopComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UNaviCredsShopComponent_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopComponent_Statics::PropPointers),
	0,
	0x00A000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviCredsShopComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviCredsShopComponent()
{
	if (!Z_Registration_Info_UClass_UNaviCredsShopComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviCredsShopComponent.OuterSingleton, Z_Construct_UClass_UNaviCredsShopComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviCredsShopComponent.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviCredsShopComponent>()
{
	return UNaviCredsShopComponent::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviCredsShopComponent);
UNaviCredsShopComponent::~UNaviCredsShopComponent() {}
// End Class UNaviCredsShopComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviCredsShopComponent, UNaviCredsShopComponent::StaticClass, TEXT("UNaviCredsShopComponent"), &Z_Registration_Info_UClass_UNaviCredsShopComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviCredsShopComponent), 2889391271U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopComponent_h_2705754703(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
