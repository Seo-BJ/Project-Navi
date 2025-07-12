// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/NaviPlayerCredsComponent.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviPlayerCredsComponent() {}

// Begin Cross Module References
MODULARGAMEPLAY_API UClass* Z_Construct_UClass_UPlayerStateComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviPlayerCredsComponent();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviPlayerCredsComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviPlayerCredsComponent
void UNaviPlayerCredsComponent::StaticRegisterNativesUNaviPlayerCredsComponent()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviPlayerCredsComponent);
UClass* Z_Construct_UClass_UNaviPlayerCredsComponent_NoRegister()
{
	return UNaviPlayerCredsComponent::StaticClass();
}
struct Z_Construct_UClass_UNaviPlayerCredsComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
		{ "HideCategories", "Trigger PhysicsVolume" },
		{ "IncludePath", "NaviPlayerCredsComponent.h" },
		{ "ModuleRelativePath", "Public/NaviPlayerCredsComponent.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviPlayerCredsComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UPlayerStateComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::ClassParams = {
	&UNaviPlayerCredsComponent::StaticClass,
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
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviPlayerCredsComponent()
{
	if (!Z_Registration_Info_UClass_UNaviPlayerCredsComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviPlayerCredsComponent.OuterSingleton, Z_Construct_UClass_UNaviPlayerCredsComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviPlayerCredsComponent.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviPlayerCredsComponent>()
{
	return UNaviPlayerCredsComponent::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviPlayerCredsComponent);
UNaviPlayerCredsComponent::~UNaviPlayerCredsComponent() {}
// End Class UNaviPlayerCredsComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviPlayerCredsComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviPlayerCredsComponent, UNaviPlayerCredsComponent::StaticClass, TEXT("UNaviPlayerCredsComponent"), &Z_Registration_Info_UClass_UNaviPlayerCredsComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviPlayerCredsComponent), 1725404683U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviPlayerCredsComponent_h_1020438435(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviPlayerCredsComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviPlayerCredsComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
