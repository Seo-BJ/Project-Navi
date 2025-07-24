// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/CredsSystem/NaviCredsShopControllerComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviCredsShopControllerComponent() {}

// Begin Cross Module References
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
MODULARGAMEPLAY_API UClass* Z_Construct_UClass_UControllerComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsShopControllerComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsShopControllerComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviCredsShopControllerComponent Function ServerRequesBuyEquipment
struct NaviCredsShopControllerComponent_eventServerRequesBuyEquipment_Parms
{
	FGameplayTag EquipmentTag;
};
static const FName NAME_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment = FName(TEXT("ServerRequesBuyEquipment"));
void UNaviCredsShopControllerComponent::ServerRequesBuyEquipment(FGameplayTag EquipmentTag)
{
	NaviCredsShopControllerComponent_eventServerRequesBuyEquipment_Parms Parms;
	Parms.EquipmentTag=EquipmentTag;
	UFunction* Func = FindFunctionChecked(NAME_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment);
	ProcessEvent(Func,&Parms);
}
struct Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|Shop" },
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopControllerComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_EquipmentTag;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::NewProp_EquipmentTag = { "EquipmentTag", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviCredsShopControllerComponent_eventServerRequesBuyEquipment_Parms, EquipmentTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(0, nullptr) }; // 1298103297
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::NewProp_EquipmentTag,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviCredsShopControllerComponent, nullptr, "ServerRequesBuyEquipment", nullptr, nullptr, Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::PropPointers), sizeof(NaviCredsShopControllerComponent_eventServerRequesBuyEquipment_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04280CC0, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::Function_MetaDataParams) };
static_assert(sizeof(NaviCredsShopControllerComponent_eventServerRequesBuyEquipment_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviCredsShopControllerComponent::execServerRequesBuyEquipment)
{
	P_GET_STRUCT(FGameplayTag,Z_Param_EquipmentTag);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->ServerRequesBuyEquipment_Implementation(Z_Param_EquipmentTag);
	P_NATIVE_END;
}
// End Class UNaviCredsShopControllerComponent Function ServerRequesBuyEquipment

// Begin Class UNaviCredsShopControllerComponent
void UNaviCredsShopControllerComponent::StaticRegisterNativesUNaviCredsShopControllerComponent()
{
	UClass* Class = UNaviCredsShopControllerComponent::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "ServerRequesBuyEquipment", &UNaviCredsShopControllerComponent::execServerRequesBuyEquipment },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviCredsShopControllerComponent);
UClass* Z_Construct_UClass_UNaviCredsShopControllerComponent_NoRegister()
{
	return UNaviCredsShopControllerComponent::StaticClass();
}
struct Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
		{ "HideCategories", "Trigger PhysicsVolume" },
		{ "IncludePath", "CredsSystem/NaviCredsShopControllerComponent.h" },
		{ "ModuleRelativePath", "Public/CredsSystem/NaviCredsShopControllerComponent.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviCredsShopControllerComponent_ServerRequesBuyEquipment, "ServerRequesBuyEquipment" }, // 3299144412
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviCredsShopControllerComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UControllerComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::ClassParams = {
	&UNaviCredsShopControllerComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	0,
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviCredsShopControllerComponent()
{
	if (!Z_Registration_Info_UClass_UNaviCredsShopControllerComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviCredsShopControllerComponent.OuterSingleton, Z_Construct_UClass_UNaviCredsShopControllerComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviCredsShopControllerComponent.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviCredsShopControllerComponent>()
{
	return UNaviCredsShopControllerComponent::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviCredsShopControllerComponent);
UNaviCredsShopControllerComponent::~UNaviCredsShopControllerComponent() {}
// End Class UNaviCredsShopControllerComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopControllerComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviCredsShopControllerComponent, UNaviCredsShopControllerComponent::StaticClass, TEXT("UNaviCredsShopControllerComponent"), &Z_Registration_Info_UClass_UNaviCredsShopControllerComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviCredsShopControllerComponent), 4076952997U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopControllerComponent_h_1728126770(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopControllerComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_CredsSystem_NaviCredsShopControllerComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
