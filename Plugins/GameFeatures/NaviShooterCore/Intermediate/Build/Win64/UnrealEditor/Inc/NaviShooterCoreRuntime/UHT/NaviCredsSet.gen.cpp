// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/NaviCredsSet.h"
#include "GameplayAbilities/Public/AttributeSet.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviCredsSet() {}

// Begin Cross Module References
GAMEPLAYABILITIES_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayAttributeData();
LYRAGAME_API UClass* Z_Construct_UClass_ULyraAttributeSet();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsSet();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsSet_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviCredsSet Function OnRep_Creds
struct Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics
{
	struct NaviCredsSet_eventOnRep_Creds_Parms
	{
		FGameplayAttributeData OldValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OldValue_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_OldValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::NewProp_OldValue = { "OldValue", nullptr, (EPropertyFlags)0x0010000008000182, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviCredsSet_eventOnRep_Creds_Parms, OldValue), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OldValue_MetaData), NewProp_OldValue_MetaData) }; // 675369593
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::NewProp_OldValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviCredsSet, nullptr, "OnRep_Creds", nullptr, nullptr, Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::NaviCredsSet_eventOnRep_Creds_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00480401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::NaviCredsSet_eventOnRep_Creds_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviCredsSet::execOnRep_Creds)
{
	P_GET_STRUCT_REF(FGameplayAttributeData,Z_Param_Out_OldValue);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnRep_Creds(Z_Param_Out_OldValue);
	P_NATIVE_END;
}
// End Class UNaviCredsSet Function OnRep_Creds

// Begin Class UNaviCredsSet Function OnRep_MaxCreds
struct Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics
{
	struct NaviCredsSet_eventOnRep_MaxCreds_Parms
	{
		FGameplayAttributeData OldValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OldValue_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_OldValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::NewProp_OldValue = { "OldValue", nullptr, (EPropertyFlags)0x0010000008000182, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviCredsSet_eventOnRep_MaxCreds_Parms, OldValue), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OldValue_MetaData), NewProp_OldValue_MetaData) }; // 675369593
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::NewProp_OldValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviCredsSet, nullptr, "OnRep_MaxCreds", nullptr, nullptr, Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::NaviCredsSet_eventOnRep_MaxCreds_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00480401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::NaviCredsSet_eventOnRep_MaxCreds_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviCredsSet::execOnRep_MaxCreds)
{
	P_GET_STRUCT_REF(FGameplayAttributeData,Z_Param_Out_OldValue);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnRep_MaxCreds(Z_Param_Out_OldValue);
	P_NATIVE_END;
}
// End Class UNaviCredsSet Function OnRep_MaxCreds

// Begin Class UNaviCredsSet
void UNaviCredsSet::StaticRegisterNativesUNaviCredsSet()
{
	UClass* Class = UNaviCredsSet::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "OnRep_Creds", &UNaviCredsSet::execOnRep_Creds },
		{ "OnRep_MaxCreds", &UNaviCredsSet::execOnRep_MaxCreds },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviCredsSet);
UClass* Z_Construct_UClass_UNaviCredsSet_NoRegister()
{
	return UNaviCredsSet::StaticClass();
}
struct Z_Construct_UClass_UNaviCredsSet_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "NaviCredsSet.h" },
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Creds_MetaData[] = {
		{ "AllowPrivateAccess", "TRUE" },
		{ "Category", "Navi|creds" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.\n" },
#endif
		{ "HideFromModifiers", "" },
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MaxCreds_MetaData[] = {
		{ "AllowPrivateAccess", "TRUE" },
		{ "Category", "Navi|creds" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.\n" },
#endif
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The current max health attribute.  Max health is an attribute since gameplay effects can modify it." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_InComingCredsGain_MetaData[] = {
		{ "AllowPrivateAccess", "TRUE" },
		{ "Category", "Navi|creds" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Incoming healing. This is mapped directly to +Health\n" },
#endif
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Incoming healing. This is mapped directly to +Health" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IncomingCredsCost_MetaData[] = {
		{ "AllowPrivateAccess", "TRUE" },
		{ "Category", "Navi|creds" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Incoming damage. This is mapped directly to -Health\n" },
#endif
		{ "HideFromModifiers", "" },
		{ "ModuleRelativePath", "Public/NaviCredsSet.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Incoming damage. This is mapped directly to -Health" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_Creds;
	static const UECodeGen_Private::FStructPropertyParams NewProp_MaxCreds;
	static const UECodeGen_Private::FStructPropertyParams NewProp_InComingCredsGain;
	static const UECodeGen_Private::FStructPropertyParams NewProp_IncomingCredsCost;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviCredsSet_OnRep_Creds, "OnRep_Creds" }, // 2542114922
		{ &Z_Construct_UFunction_UNaviCredsSet_OnRep_MaxCreds, "OnRep_MaxCreds" }, // 3681229098
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviCredsSet>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_Creds = { "Creds", "OnRep_Creds", (EPropertyFlags)0x0040000100000034, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsSet, Creds), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Creds_MetaData), NewProp_Creds_MetaData) }; // 675369593
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_MaxCreds = { "MaxCreds", "OnRep_MaxCreds", (EPropertyFlags)0x0040000100000034, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsSet, MaxCreds), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MaxCreds_MetaData), NewProp_MaxCreds_MetaData) }; // 675369593
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_InComingCredsGain = { "InComingCredsGain", nullptr, (EPropertyFlags)0x0040000000000014, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsSet, InComingCredsGain), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_InComingCredsGain_MetaData), NewProp_InComingCredsGain_MetaData) }; // 675369593
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_IncomingCredsCost = { "IncomingCredsCost", nullptr, (EPropertyFlags)0x0040000000000014, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviCredsSet, IncomingCredsCost), Z_Construct_UScriptStruct_FGameplayAttributeData, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IncomingCredsCost_MetaData), NewProp_IncomingCredsCost_MetaData) }; // 675369593
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNaviCredsSet_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_Creds,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_MaxCreds,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_InComingCredsGain,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviCredsSet_Statics::NewProp_IncomingCredsCost,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsSet_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNaviCredsSet_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_ULyraAttributeSet,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsSet_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviCredsSet_Statics::ClassParams = {
	&UNaviCredsSet::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UNaviCredsSet_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsSet_Statics::PropPointers),
	0,
	0x003000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsSet_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviCredsSet_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviCredsSet()
{
	if (!Z_Registration_Info_UClass_UNaviCredsSet.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviCredsSet.OuterSingleton, Z_Construct_UClass_UNaviCredsSet_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviCredsSet.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviCredsSet>()
{
	return UNaviCredsSet::StaticClass();
}
void UNaviCredsSet::ValidateGeneratedRepEnums(const TArray<struct FRepRecord>& ClassReps) const
{
	static const FName Name_Creds(TEXT("Creds"));
	static const FName Name_MaxCreds(TEXT("MaxCreds"));
	const bool bIsValid = true
		&& Name_Creds == ClassReps[(int32)ENetFields_Private::Creds].Property->GetFName()
		&& Name_MaxCreds == ClassReps[(int32)ENetFields_Private::MaxCreds].Property->GetFName();
	checkf(bIsValid, TEXT("UHT Generated Rep Indices do not match runtime populated Rep Indices for properties in UNaviCredsSet"));
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviCredsSet);
UNaviCredsSet::~UNaviCredsSet() {}
// End Class UNaviCredsSet

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviCredsSet, UNaviCredsSet::StaticClass, TEXT("UNaviCredsSet"), &Z_Registration_Info_UClass_UNaviCredsSet, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviCredsSet), 2664893272U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_402894675(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
