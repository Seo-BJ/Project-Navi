// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/NaviCredsStateComponent.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviCredsStateComponent() {}

// Begin Cross Module References
LYRAGAME_API UClass* Z_Construct_UClass_UGameplayMessageProcessor();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsStateComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviCredsStateComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviCredsStateComponent
void UNaviCredsStateComponent::StaticRegisterNativesUNaviCredsStateComponent()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviCredsStateComponent);
UClass* Z_Construct_UClass_UNaviCredsStateComponent_NoRegister()
{
	return UNaviCredsStateComponent::StaticClass();
}
struct Z_Construct_UClass_UNaviCredsStateComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
		{ "IncludePath", "NaviCredsStateComponent.h" },
		{ "ModuleRelativePath", "Public/NaviCredsStateComponent.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviCredsStateComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNaviCredsStateComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UGameplayMessageProcessor,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsStateComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviCredsStateComponent_Statics::ClassParams = {
	&UNaviCredsStateComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviCredsStateComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviCredsStateComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviCredsStateComponent()
{
	if (!Z_Registration_Info_UClass_UNaviCredsStateComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviCredsStateComponent.OuterSingleton, Z_Construct_UClass_UNaviCredsStateComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviCredsStateComponent.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviCredsStateComponent>()
{
	return UNaviCredsStateComponent::StaticClass();
}
UNaviCredsStateComponent::UNaviCredsStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviCredsStateComponent);
UNaviCredsStateComponent::~UNaviCredsStateComponent() {}
// End Class UNaviCredsStateComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsStateComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviCredsStateComponent, UNaviCredsStateComponent::StaticClass, TEXT("UNaviCredsStateComponent"), &Z_Registration_Info_UClass_UNaviCredsStateComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviCredsStateComponent), 4150524250U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsStateComponent_h_185876653(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsStateComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsStateComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
