// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviAgentSelectRuntime/Public/Agent/NaviAgentInstance.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviAgentInstance() {}

// Begin Cross Module References
LYRAGAME_API UClass* Z_Construct_UClass_ULyraEquipmentInstance();
NAVIAGENTSELECTRUNTIME_API UClass* Z_Construct_UClass_UNaviAgentInstance();
NAVIAGENTSELECTRUNTIME_API UClass* Z_Construct_UClass_UNaviAgentInstance_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviAgentSelectRuntime();
// End Cross Module References

// Begin Class UNaviAgentInstance
void UNaviAgentInstance::StaticRegisterNativesUNaviAgentInstance()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviAgentInstance);
UClass* Z_Construct_UClass_UNaviAgentInstance_NoRegister()
{
	return UNaviAgentInstance::StaticClass();
}
struct Z_Construct_UClass_UNaviAgentInstance_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "Agent/NaviAgentInstance.h" },
		{ "ModuleRelativePath", "Public/Agent/NaviAgentInstance.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviAgentInstance>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNaviAgentInstance_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_ULyraEquipmentInstance,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviAgentSelectRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviAgentInstance_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviAgentInstance_Statics::ClassParams = {
	&UNaviAgentInstance::StaticClass,
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
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviAgentInstance_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviAgentInstance_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviAgentInstance()
{
	if (!Z_Registration_Info_UClass_UNaviAgentInstance.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviAgentInstance.OuterSingleton, Z_Construct_UClass_UNaviAgentInstance_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviAgentInstance.OuterSingleton;
}
template<> NAVIAGENTSELECTRUNTIME_API UClass* StaticClass<UNaviAgentInstance>()
{
	return UNaviAgentInstance::StaticClass();
}
UNaviAgentInstance::UNaviAgentInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviAgentInstance);
UNaviAgentInstance::~UNaviAgentInstance() {}
// End Class UNaviAgentInstance

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentInstance_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviAgentInstance, UNaviAgentInstance::StaticClass, TEXT("UNaviAgentInstance"), &Z_Registration_Info_UClass_UNaviAgentInstance, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviAgentInstance), 1258702522U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentInstance_h_376551434(TEXT("/Script/NaviAgentSelectRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentInstance_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviAgentSelect_Source_NaviAgentSelectRuntime_Public_Agent_NaviAgentInstance_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
