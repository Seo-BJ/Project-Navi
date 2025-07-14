// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/Weapons/NaviWeaponStatDefinition.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviWeaponStatDefinition() {}

// Begin Cross Module References
ENGINE_API UScriptStruct* Z_Construct_UScriptStruct_FTableRowBase();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FDamageFalloff();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FNaviWeaponStatDefinition();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin ScriptStruct FDamageFalloff
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_DamageFalloff;
class UScriptStruct* FDamageFalloff::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_DamageFalloff.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_DamageFalloff.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FDamageFalloff, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("DamageFalloff"));
	}
	return Z_Registration_Info_UScriptStruct_DamageFalloff.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FDamageFalloff>()
{
	return FDamageFalloff::StaticStruct();
}
struct Z_Construct_UScriptStruct_FDamageFalloff_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xea\xb1\xb0\xeb\xa6\xac\xeb\xb3\x84 \xeb\x8d\xb0\xeb\xaf\xb8\xec\xa7\x80 \xec\xa0\x95\xeb\xb3\xb4\xeb\xa5\xbc \xec\xa0\x95\xec\x9d\x98\xed\x95\x98\xeb\x8a\x94 \xea\xb5\xac\xec\xa1\xb0\xec\xb2\xb4\n */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xea\xb1\xb0\xeb\xa6\xac\xeb\xb3\x84 \xeb\x8d\xb0\xeb\xaf\xb8\xec\xa7\x80 \xec\xa0\x95\xeb\xb3\xb4\xeb\xa5\xbc \xec\xa0\x95\xec\x9d\x98\xed\x95\x98\xeb\x8a\x94 \xea\xb5\xac\xec\xa1\xb0\xec\xb2\xb4" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RangeStart_MetaData[] = {
		{ "Category", "Damage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80 \xea\xb0\x90\xec\x86\x8c\xea\xb0\x80 \xec\x8b\x9c\xec\x9e\x91\xeb\x90\x98\xeb\x8a\x94 \xea\xb1\xb0\xeb\xa6\xac (\xeb\x8b\xa8\xec\x9c\x84: \xeb\xaf\xb8\xed\x84\xb0)\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80 \xea\xb0\x90\xec\x86\x8c\xea\xb0\x80 \xec\x8b\x9c\xec\x9e\x91\xeb\x90\x98\xeb\x8a\x94 \xea\xb1\xb0\xeb\xa6\xac (\xeb\x8b\xa8\xec\x9c\x84: \xeb\xaf\xb8\xed\x84\xb0)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RangeEnd_MetaData[] = {
		{ "Category", "Damage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80 \xea\xb0\x90\xec\x86\x8c\xea\xb0\x80 \xeb\x81\x9d\xeb\x82\x98\xeb\x8a\x94 \xea\xb1\xb0\xeb\xa6\xac (\xeb\x8b\xa8\xec\x9c\x84: \xeb\xaf\xb8\xed\x84\xb0)\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80 \xea\xb0\x90\xec\x86\x8c\xea\xb0\x80 \xeb\x81\x9d\xeb\x82\x98\xeb\x8a\x94 \xea\xb1\xb0\xeb\xa6\xac (\xeb\x8b\xa8\xec\x9c\x84: \xeb\xaf\xb8\xed\x84\xb0)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeadShotDamage_MetaData[] = {
		{ "Category", "Damage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\xa8\xb8\xeb\xa6\xac \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\xa8\xb8\xeb\xa6\xac \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BodyShotDamage_MetaData[] = {
		{ "Category", "Damage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\xaa\xb8\xed\x86\xb5 \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\xaa\xb8\xed\x86\xb5 \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_LegShotDamage_MetaData[] = {
		{ "Category", "Damage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\x8b\xa4\xeb\xa6\xac \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x95\xb4\xeb\x8b\xb9 \xea\xb1\xb0\xeb\xa6\xac\xec\x97\x90\xec\x84\x9c\xec\x9d\x98 \xeb\x8b\xa4\xeb\xa6\xac \xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RangeStart;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RangeEnd;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeadShotDamage;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BodyShotDamage;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_LegShotDamage;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FDamageFalloff>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_RangeStart = { "RangeStart", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FDamageFalloff, RangeStart), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RangeStart_MetaData), NewProp_RangeStart_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_RangeEnd = { "RangeEnd", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FDamageFalloff, RangeEnd), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RangeEnd_MetaData), NewProp_RangeEnd_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_HeadShotDamage = { "HeadShotDamage", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FDamageFalloff, HeadShotDamage), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeadShotDamage_MetaData), NewProp_HeadShotDamage_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_BodyShotDamage = { "BodyShotDamage", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FDamageFalloff, BodyShotDamage), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BodyShotDamage_MetaData), NewProp_BodyShotDamage_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_LegShotDamage = { "LegShotDamage", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FDamageFalloff, LegShotDamage), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_LegShotDamage_MetaData), NewProp_LegShotDamage_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FDamageFalloff_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_RangeStart,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_RangeEnd,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_HeadShotDamage,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_BodyShotDamage,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewProp_LegShotDamage,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FDamageFalloff_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FDamageFalloff_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	nullptr,
	&NewStructOps,
	"DamageFalloff",
	Z_Construct_UScriptStruct_FDamageFalloff_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FDamageFalloff_Statics::PropPointers),
	sizeof(FDamageFalloff),
	alignof(FDamageFalloff),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FDamageFalloff_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FDamageFalloff_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FDamageFalloff()
{
	if (!Z_Registration_Info_UScriptStruct_DamageFalloff.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_DamageFalloff.InnerSingleton, Z_Construct_UScriptStruct_FDamageFalloff_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_DamageFalloff.InnerSingleton;
}
// End ScriptStruct FDamageFalloff

// Begin ScriptStruct FNaviWeaponStatDefinition
static_assert(std::is_polymorphic<FNaviWeaponStatDefinition>() == std::is_polymorphic<FTableRowBase>(), "USTRUCT FNaviWeaponStatDefinition cannot be polymorphic unless super FTableRowBase is polymorphic");
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition;
class UScriptStruct* FNaviWeaponStatDefinition::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FNaviWeaponStatDefinition, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("NaviWeaponStatDefinition"));
	}
	return Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FNaviWeaponStatDefinition>()
{
	return FNaviWeaponStatDefinition::StaticStruct();
}
struct Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x85\x9f \xeb\x8d\xb0\xec\x9d\xb4\xed\x84\xb0 \xed\x85\x8c\xec\x9d\xb4\xeb\xb8\x94 \xed\x96\x89 \xea\xb5\xac\xec\xa1\xb0\xec\xb2\xb4\n */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x85\x9f \xeb\x8d\xb0\xec\x9d\xb4\xed\x84\xb0 \xed\x85\x8c\xec\x9d\xb4\xeb\xb8\x94 \xed\x96\x89 \xea\xb5\xac\xec\xa1\xb0\xec\xb2\xb4" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DisplayName_MetaData[] = {
		{ "Category", "\xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// --- \xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4 ---\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "--- \xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4 ---" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WeaponTag_MetaData[] = {
		{ "Category", "\xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\xac\xb4\xea\xb8\xb0 \xec\x9d\xb4\xeb\xa6\x84\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0 \xec\x9d\xb4\xeb\xa6\x84" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CreditCost_MetaData[] = {
		{ "Category", "\xea\xb8\xb0\xeb\xb3\xb8 \xec\xa0\x95\xeb\xb3\xb4" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\xac\xb4\xea\xb8\xb0 \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0 \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DamageFalloffs_MetaData[] = {
		{ "Category", "\xeb\x8c\x80\xeb\xaf\xb8\xec\xa7\x80" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// --- \xeb\x8d\xb0\xeb\xaf\xb8\xec\xa7\x80 \xec\xa0\x95\xeb\xb3\xb4 ---\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "--- \xeb\x8d\xb0\xeb\xaf\xb8\xec\xa7\x80 \xec\xa0\x95\xeb\xb3\xb4 ---" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_FireRate_MetaData[] = {
		{ "Category", "\xeb\xb0\x9c\xec\x82\xac \xec\x8a\xa4\xed\x83\xaf" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// --- \xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x83\xaf ---\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "--- \xeb\xac\xb4\xea\xb8\xb0 \xec\x8a\xa4\xed\x83\xaf ---" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RunSpeed_MetaData[] = {
		{ "Category", "\xeb\xb0\x9c\xec\x82\xac \xec\x8a\xa4\xed\x83\xaf" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\x97\xb0\xec\x82\xac \xec\x86\x8d\xeb\x8f\x84 (\xec\xb4\x88\xeb\x8b\xb9 \xeb\xb0\x9c\xec\x82\xac \xec\x88\x98)\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x97\xb0\xec\x82\xac \xec\x86\x8d\xeb\x8f\x84 (\xec\xb4\x88\xeb\x8b\xb9 \xeb\xb0\x9c\xec\x82\xac \xec\x88\x98)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EquipSpeed_MetaData[] = {
		{ "Category", "\xeb\xb0\x9c\xec\x82\xac \xec\x8a\xa4\xed\x83\xaf" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8b\xac\xeb\xa6\xac\xea\xb8\xb0 \xec\x86\x8d\xeb\x8f\x84\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8b\xac\xeb\xa6\xac\xea\xb8\xb0 \xec\x86\x8d\xeb\x8f\x84" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ReloadSpeed_MetaData[] = {
		{ "Category", "\xeb\xb0\x9c\xec\x82\xac \xec\x8a\xa4\xed\x83\xaf" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\x9e\xa5\xec\xb0\xa9 \xec\x86\x8d\xeb\x8f\x84\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x9e\xa5\xec\xb0\xa9 \xec\x86\x8d\xeb\x8f\x84" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MagazineSize_MetaData[] = {
		{ "Category", "\xed\x83\x84\xec\x95\xbd" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// --- \xed\x83\x84\xec\xb0\xbd \xec\xa0\x95\xeb\xb3\xb4 ---\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "--- \xed\x83\x84\xec\xb0\xbd \xec\xa0\x95\xeb\xb3\xb4 ---" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MaxAmmo_MetaData[] = {
		{ "Category", "\xed\x83\x84\xec\x95\xbd" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xed\x83\x84\xec\xb0\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x83\x84\xec\xb0\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_FirstShotSpread_MetaData[] = {
		{ "Category", "\xec\xa0\x95\xed\x99\x95\xeb\x8f\x84" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// --- \xec\xa0\x95\xed\x99\x95\xeb\x8f\x84 ---\n" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponStatDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "--- \xec\xa0\x95\xed\x99\x95\xeb\x8f\x84 ---" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FTextPropertyParams NewProp_DisplayName;
	static const UECodeGen_Private::FStructPropertyParams NewProp_WeaponTag;
	static const UECodeGen_Private::FIntPropertyParams NewProp_CreditCost;
	static const UECodeGen_Private::FStructPropertyParams NewProp_DamageFalloffs_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_DamageFalloffs;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_FireRate;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RunSpeed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_EquipSpeed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_ReloadSpeed;
	static const UECodeGen_Private::FIntPropertyParams NewProp_MagazineSize;
	static const UECodeGen_Private::FIntPropertyParams NewProp_MaxAmmo;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_FirstShotSpread;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FNaviWeaponStatDefinition>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FTextPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DisplayName = { "DisplayName", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, DisplayName), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DisplayName_MetaData), NewProp_DisplayName_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_WeaponTag = { "WeaponTag", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, WeaponTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WeaponTag_MetaData), NewProp_WeaponTag_MetaData) }; // 1298103297
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_CreditCost = { "CreditCost", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, CreditCost), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CreditCost_MetaData), NewProp_CreditCost_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DamageFalloffs_Inner = { "DamageFalloffs", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UScriptStruct_FDamageFalloff, METADATA_PARAMS(0, nullptr) }; // 442083766
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DamageFalloffs = { "DamageFalloffs", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, DamageFalloffs), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DamageFalloffs_MetaData), NewProp_DamageFalloffs_MetaData) }; // 442083766
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_FireRate = { "FireRate", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, FireRate), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_FireRate_MetaData), NewProp_FireRate_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_RunSpeed = { "RunSpeed", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, RunSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RunSpeed_MetaData), NewProp_RunSpeed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_EquipSpeed = { "EquipSpeed", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, EquipSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EquipSpeed_MetaData), NewProp_EquipSpeed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_ReloadSpeed = { "ReloadSpeed", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, ReloadSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ReloadSpeed_MetaData), NewProp_ReloadSpeed_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_MagazineSize = { "MagazineSize", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, MagazineSize), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MagazineSize_MetaData), NewProp_MagazineSize_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_MaxAmmo = { "MaxAmmo", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, MaxAmmo), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MaxAmmo_MetaData), NewProp_MaxAmmo_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_FirstShotSpread = { "FirstShotSpread", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponStatDefinition, FirstShotSpread), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_FirstShotSpread_MetaData), NewProp_FirstShotSpread_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DisplayName,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_WeaponTag,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_CreditCost,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DamageFalloffs_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_DamageFalloffs,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_FireRate,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_RunSpeed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_EquipSpeed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_ReloadSpeed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_MagazineSize,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_MaxAmmo,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewProp_FirstShotSpread,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	Z_Construct_UScriptStruct_FTableRowBase,
	&NewStructOps,
	"NaviWeaponStatDefinition",
	Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::PropPointers),
	sizeof(FNaviWeaponStatDefinition),
	alignof(FNaviWeaponStatDefinition),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FNaviWeaponStatDefinition()
{
	if (!Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.InnerSingleton, Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition.InnerSingleton;
}
// End ScriptStruct FNaviWeaponStatDefinition

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponStatDefinition_h_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FDamageFalloff::StaticStruct, Z_Construct_UScriptStruct_FDamageFalloff_Statics::NewStructOps, TEXT("DamageFalloff"), &Z_Registration_Info_UScriptStruct_DamageFalloff, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FDamageFalloff), 442083766U) },
		{ FNaviWeaponStatDefinition::StaticStruct, Z_Construct_UScriptStruct_FNaviWeaponStatDefinition_Statics::NewStructOps, TEXT("NaviWeaponStatDefinition"), &Z_Registration_Info_UScriptStruct_NaviWeaponStatDefinition, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FNaviWeaponStatDefinition), 142074768U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponStatDefinition_h_1599090909(TEXT("/Script/NaviShooterCoreRuntime"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponStatDefinition_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponStatDefinition_h_Statics::ScriptStructInfo),
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
