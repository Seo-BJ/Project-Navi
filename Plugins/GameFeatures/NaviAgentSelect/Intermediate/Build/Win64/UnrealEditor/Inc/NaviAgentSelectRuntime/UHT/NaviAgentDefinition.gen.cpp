// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviAgentSelectRuntime/Public/Agent/NaviAgentDefinition.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviAgentDefinition() {}

// Begin Cross Module References
LYRAGAME_API UClass* Z_Construct_UClass_ULyraEquipmentDefinition();
NAVIAGENTSELECTRUNTIME_API UClass* Z_Construct_UClass_UNaviAgentDefinition();
NAVIAGENTSELECTRUNTIME_API UClass* Z_Construct_UClass_UNaviAgentDefinition_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviAgentSelectRuntime();
// End Cross Module References

// Begin Class UNaviAgentDefinition
void UNaviAgentDefinition::StaticRegisterNativesUNaviAgentDefinition()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviAgentDefinition);
UClass* Z_Construct_UClass_UNaviAgentDefinition_NoRegister()
{
	return UNaviAgentDefinition::StaticClass();
}
struct Z_Construct_UClass_UNaviAgentDefinition_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "Agent/NaviAgentDefinition.h" },
		{ "ModuleRelativePath", "Public/Agent/NaviAgentDefinition.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviAgentDefinition>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNaviAgentDefinition_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_ULyraEquipmentDefinition,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviAgentSelectRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviAgentDefinition_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviAgentDefinition_Statics::ClassParams = {
	&UNaviAgentDefinition::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x001100A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviAgentDefinition_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviAgentDefinition_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviAgentDefinition()
{
	if (!Z_Registration_Info_UClass_UNaviAgentDefinition.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviAgentDefinition.OuterSingleton, Z_Construct_UClass_UNaviAgentDefinition_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviAgentDefinition.OuterSingleton;
}
template<> NAVIAGENTSELECTRUNTIME_API UClass* StaticClass<UNaviAgentDefinition>()
{
	return UNaviAgentDefinition::StaticClass();
}
UNaviAgentDefinition::UNaviAgentDefinition(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviAgentDefinition);
UNaviAgentDefinition::~UNaviAgentDefinition() {}
// End Class UNaviAgentDefinition

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentDefinition_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviAgentDefinition, UNaviAgentDefinition::StaticClass, TEXT("UNaviAgentDefinition"), &Z_Registration_Info_UClass_UNaviAgentDefinition, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviAgentDefinition), 3157964714U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentDefinition_h_2993430641(TEXT("/Script/NaviAgentSelectRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentDefinition_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentDefinition_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
