// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/Weapons/NaviWeaponWidgetDefinition.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviWeaponWidgetDefinition() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_UTexture2D_NoRegister();
ENGINE_API UScriptStruct* Z_Construct_UScriptStruct_FTableRowBase();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
NAVISHOOTERCORERUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin ScriptStruct FNaviWeaponWidgetDefinition
static_assert(std::is_polymorphic<FNaviWeaponWidgetDefinition>() == std::is_polymorphic<FTableRowBase>(), "USTRUCT FNaviWeaponWidgetDefinition cannot be polymorphic unless super FTableRowBase is polymorphic");
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition;
class UScriptStruct* FNaviWeaponWidgetDefinition::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition, (UObject*)Z_Construct_UPackage__Script_NaviShooterCoreRuntime(), TEXT("NaviWeaponWidgetDefinition"));
	}
	return Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<FNaviWeaponWidgetDefinition>()
{
	return FNaviWeaponWidgetDefinition::StaticStruct();
}
struct Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponWidgetDefinition.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DisplayName_MetaData[] = {
		{ "Category", "Weapon" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xeb\xac\xb4\xea\xb8\xb0 \xed\x91\x9c\xec\x8b\x9c \xec\x9d\xb4\xeb\xa6\x84 */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponWidgetDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0 \xed\x91\x9c\xec\x8b\x9c \xec\x9d\xb4\xeb\xa6\x84" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WeaponTag_MetaData[] = {
		{ "Category", "Weapon" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xeb\xac\xb4\xea\xb8\xb0\xeb\xa5\xbc \xec\x8b\x9d\xeb\xb3\x84\xed\x95\x98\xea\xb8\xb0 \xec\x9c\x84\xed\x95\x9c \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8 */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponWidgetDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xac\xb4\xea\xb8\xb0\xeb\xa5\xbc \xec\x8b\x9d\xeb\xb3\x84\xed\x95\x98\xea\xb8\xb0 \xec\x9c\x84\xed\x95\x9c \xea\xb2\x8c\xec\x9e\x84\xed\x94\x8c\xeb\xa0\x88\xec\x9d\xb4 \xed\x83\x9c\xea\xb7\xb8" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_KillFeedIcon_MetaData[] = {
		{ "Category", "Weapon" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xed\x82\xac \xed\x94\xbc\xeb\x93\x9c\xec\x97\x90 \xed\x91\x9c\xec\x8b\x9c\xeb\x90\xa0 \xec\x95\x84\xec\x9d\xb4\xec\xbd\x98 */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponWidgetDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xed\x82\xac \xed\x94\xbc\xeb\x93\x9c\xec\x97\x90 \xed\x91\x9c\xec\x8b\x9c\xeb\x90\xa0 \xec\x95\x84\xec\x9d\xb4\xec\xbd\x98" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WidgetIcon_MetaData[] = {
		{ "Category", "Weapon" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \xec\x9d\xbc\xeb\xb0\x98 \xec\x9c\x84\xec\xa0\xaf\xec\x97\x90 \xed\x91\x9c\xec\x8b\x9c\xeb\x90\xa0 \xec\x95\x84\xec\x9d\xb4\xec\xbd\x98 */" },
#endif
		{ "ModuleRelativePath", "Public/Weapons/NaviWeaponWidgetDefinition.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x9d\xbc\xeb\xb0\x98 \xec\x9c\x84\xec\xa0\xaf\xec\x97\x90 \xed\x91\x9c\xec\x8b\x9c\xeb\x90\xa0 \xec\x95\x84\xec\x9d\xb4\xec\xbd\x98" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FTextPropertyParams NewProp_DisplayName;
	static const UECodeGen_Private::FStructPropertyParams NewProp_WeaponTag;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_KillFeedIcon;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_WidgetIcon;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FNaviWeaponWidgetDefinition>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FTextPropertyParams Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_DisplayName = { "DisplayName", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponWidgetDefinition, DisplayName), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DisplayName_MetaData), NewProp_DisplayName_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_WeaponTag = { "WeaponTag", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponWidgetDefinition, WeaponTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WeaponTag_MetaData), NewProp_WeaponTag_MetaData) }; // 1298103297
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_KillFeedIcon = { "KillFeedIcon", nullptr, (EPropertyFlags)0x0114000000000015, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponWidgetDefinition, KillFeedIcon), Z_Construct_UClass_UTexture2D_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_KillFeedIcon_MetaData), NewProp_KillFeedIcon_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_WidgetIcon = { "WidgetIcon", nullptr, (EPropertyFlags)0x0114000000000015, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FNaviWeaponWidgetDefinition, WidgetIcon), Z_Construct_UClass_UTexture2D_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WidgetIcon_MetaData), NewProp_WidgetIcon_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_DisplayName,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_WeaponTag,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_KillFeedIcon,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewProp_WidgetIcon,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
	Z_Construct_UScriptStruct_FTableRowBase,
	&NewStructOps,
	"NaviWeaponWidgetDefinition",
	Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::PropPointers),
	sizeof(FNaviWeaponWidgetDefinition),
	alignof(FNaviWeaponWidgetDefinition),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition()
{
	if (!Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.InnerSingleton, Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition.InnerSingleton;
}
// End ScriptStruct FNaviWeaponWidgetDefinition

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponWidgetDefinition_h_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FNaviWeaponWidgetDefinition::StaticStruct, Z_Construct_UScriptStruct_FNaviWeaponWidgetDefinition_Statics::NewStructOps, TEXT("NaviWeaponWidgetDefinition"), &Z_Registration_Info_UScriptStruct_NaviWeaponWidgetDefinition, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FNaviWeaponWidgetDefinition), 4184529676U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponWidgetDefinition_h_3280259653(TEXT("/Script/NaviShooterCoreRuntime"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponWidgetDefinition_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Weapons_NaviWeaponWidgetDefinition_h_Statics::ScriptStructInfo),
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
