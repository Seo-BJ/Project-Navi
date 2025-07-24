// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/Equipment/Armor/NaviArmorStatDefinition.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviArmorStatDefinition() {}

// Begin Cross Module References
ENGINE_API UScriptStruct* Z_Construct_UScriptStruct_FTableRowBase();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FNaviArmorStatDefinition();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin ScriptStruct FNaviArmorStatDefinition
static_assert(std::is_polymorphic<FNaviArmorStatDefinition>() == std::is_polymorphic<FTableRowBase>(), "USTRUCT FNaviArmorStatDefinition cannot be polymorphic unless super FTableRowBase is polymorphic");
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition;
class UScriptStruct* FNaviArmorStatDefinition::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FNaviArmorStatDefinition, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("NaviArmorStatDefinition"));
	}
	return Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FNaviArmorStatDefinition>()
{
	return FNaviArmorStatDefinition::StaticStruct();
}
struct Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xeb\xb0\xa9\xec\x96\xb4\xea\xb5\xac Stat DataTable Row Struct\n */" },
#endif
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xb0\xa9\xec\x96\xb4\xea\xb5\xac Stat DataTable Row Struct" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ArmorTag_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CreditCost_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DamageAbsorptionRate_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AbsorbAmount_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RegenerationPool_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bAffectedByDecay_MetaData[] = {
		{ "Category", "NaviArmorStatDefinition" },
		{ "ModuleRelativePath", "Public/Equipment/Armor/NaviArmorStatDefinition.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_ArmorTag;
	static const UECodeGen_Private::FIntPropertyParams NewProp_CreditCost;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_DamageAbsorptionRate;
	static const UECodeGen_Private::FIntPropertyParams NewProp_AbsorbAmount;
	static const UECodeGen_Private::FIntPropertyParams NewProp_RegenerationPool;
	static void NewProp_bAffectedByDecay_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bAffectedByDecay;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FNaviArmorStatDefinition>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_ArmorTag = { "ArmorTag", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviArmorStatDefinition, ArmorTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ArmorTag_MetaData), NewProp_ArmorTag_MetaData) }; // 1298103297
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_CreditCost = { "CreditCost", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviArmorStatDefinition, CreditCost), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CreditCost_MetaData), NewProp_CreditCost_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_DamageAbsorptionRate = { "DamageAbsorptionRate", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviArmorStatDefinition, DamageAbsorptionRate), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DamageAbsorptionRate_MetaData), NewProp_DamageAbsorptionRate_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_AbsorbAmount = { "AbsorbAmount", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviArmorStatDefinition, AbsorbAmount), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AbsorbAmount_MetaData), NewProp_AbsorbAmount_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_RegenerationPool = { "RegenerationPool", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviArmorStatDefinition, RegenerationPool), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RegenerationPool_MetaData), NewProp_RegenerationPool_MetaData) };
void Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_bAffectedByDecay_SetBit(void* Obj)
{
	((FNaviArmorStatDefinition*)Obj)->bAffectedByDecay = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_bAffectedByDecay = { "bAffectedByDecay", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FNaviArmorStatDefinition), &Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_bAffectedByDecay_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bAffectedByDecay_MetaData), NewProp_bAffectedByDecay_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_ArmorTag,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_CreditCost,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_DamageAbsorptionRate,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_AbsorbAmount,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_RegenerationPool,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewProp_bAffectedByDecay,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	Z_Construct_UScriptStruct_FTableRowBase,
	&NewStructOps,
	"NaviArmorStatDefinition",
	Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::PropPointers),
	sizeof(FNaviArmorStatDefinition),
	alignof(FNaviArmorStatDefinition),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FNaviArmorStatDefinition()
{
	if (!Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.InnerSingleton, Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition.InnerSingleton;
}
// End ScriptStruct FNaviArmorStatDefinition

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_Armor_NaviArmorStatDefinition_h_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FNaviArmorStatDefinition::StaticStruct, Z_Construct_UScriptStruct_FNaviArmorStatDefinition_Statics::NewStructOps, TEXT("NaviArmorStatDefinition"), &Z_Registration_Info_UScriptStruct_NaviArmorStatDefinition, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FNaviArmorStatDefinition), 4068417168U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_Armor_NaviArmorStatDefinition_h_450412399(TEXT("/Script/NaviShooterCoreRuntime"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_Armor_NaviArmorStatDefinition_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_Armor_NaviArmorStatDefinition_h_Statics::ScriptStructInfo),
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
