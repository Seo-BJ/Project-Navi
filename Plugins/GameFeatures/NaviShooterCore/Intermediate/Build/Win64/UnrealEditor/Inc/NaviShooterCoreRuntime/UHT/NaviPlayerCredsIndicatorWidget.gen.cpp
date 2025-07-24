// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/UI/HUD/NaviPlayerCredsIndicatorWidget.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviPlayerCredsIndicatorWidget() {}

// Begin Cross Module References
COMMONUI_API UClass* Z_Construct_UClass_UCommonNumericTextBlock_NoRegister();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_NoRegister();
UMG_API UClass* Z_Construct_UClass_UImage_NoRegister();
UMG_API UClass* Z_Construct_UClass_UUserWidget();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviPlayerCredsIndicatorWidget Function SetCreds
struct Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics
{
	struct NaviPlayerCredsIndicatorWidget_eventSetCreds_Parms
	{
		float NewCreds;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Navi|Creds" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviPlayerCredsIndicatorWidget.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewCreds;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::NewProp_NewCreds = { "NewCreds", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviPlayerCredsIndicatorWidget_eventSetCreds_Parms, NewCreds), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::NewProp_NewCreds,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget, nullptr, "SetCreds", nullptr, nullptr, Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::NaviPlayerCredsIndicatorWidget_eventSetCreds_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::NaviPlayerCredsIndicatorWidget_eventSetCreds_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviPlayerCredsIndicatorWidget::execSetCreds)
{
	P_GET_PROPERTY(FFloatProperty,Z_Param_NewCreds);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetCreds(Z_Param_NewCreds);
	P_NATIVE_END;
}
// End Class UNaviPlayerCredsIndicatorWidget Function SetCreds

// Begin Class UNaviPlayerCredsIndicatorWidget
void UNaviPlayerCredsIndicatorWidget::StaticRegisterNativesUNaviPlayerCredsIndicatorWidget()
{
	UClass* Class = UNaviPlayerCredsIndicatorWidget::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "SetCreds", &UNaviPlayerCredsIndicatorWidget::execSetCreds },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviPlayerCredsIndicatorWidget);
UClass* Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_NoRegister()
{
	return UNaviPlayerCredsIndicatorWidget::StaticClass();
}
struct Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "UI/HUD/NaviPlayerCredsIndicatorWidget.h" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviPlayerCredsIndicatorWidget.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CredsIcon_Image_MetaData[] = {
		{ "BindWidget", "" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviPlayerCredsIndicatorWidget.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Creds_TextBlock_MetaData[] = {
		{ "BindWidget", "" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviPlayerCredsIndicatorWidget.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_CredsIcon_Image;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Creds_TextBlock;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviPlayerCredsIndicatorWidget_SetCreds, "SetCreds" }, // 1366824576
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviPlayerCredsIndicatorWidget>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::NewProp_CredsIcon_Image = { "CredsIcon_Image", nullptr, (EPropertyFlags)0x0144000000080008, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviPlayerCredsIndicatorWidget, CredsIcon_Image), Z_Construct_UClass_UImage_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CredsIcon_Image_MetaData), NewProp_CredsIcon_Image_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::NewProp_Creds_TextBlock = { "Creds_TextBlock", nullptr, (EPropertyFlags)0x0144000000080008, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviPlayerCredsIndicatorWidget, Creds_TextBlock), Z_Construct_UClass_UCommonNumericTextBlock_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Creds_TextBlock_MetaData), NewProp_Creds_TextBlock_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::NewProp_CredsIcon_Image,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::NewProp_Creds_TextBlock,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UUserWidget,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::ClassParams = {
	&UNaviPlayerCredsIndicatorWidget::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::PropPointers),
	0,
	0x00B010A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget()
{
	if (!Z_Registration_Info_UClass_UNaviPlayerCredsIndicatorWidget.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviPlayerCredsIndicatorWidget.OuterSingleton, Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviPlayerCredsIndicatorWidget.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviPlayerCredsIndicatorWidget>()
{
	return UNaviPlayerCredsIndicatorWidget::StaticClass();
}
UNaviPlayerCredsIndicatorWidget::UNaviPlayerCredsIndicatorWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviPlayerCredsIndicatorWidget);
UNaviPlayerCredsIndicatorWidget::~UNaviPlayerCredsIndicatorWidget() {}
// End Class UNaviPlayerCredsIndicatorWidget

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviPlayerCredsIndicatorWidget_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviPlayerCredsIndicatorWidget, UNaviPlayerCredsIndicatorWidget::StaticClass, TEXT("UNaviPlayerCredsIndicatorWidget"), &Z_Registration_Info_UClass_UNaviPlayerCredsIndicatorWidget, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviPlayerCredsIndicatorWidget), 1828972449U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviPlayerCredsIndicatorWidget_h_2595157265(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviPlayerCredsIndicatorWidget_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviPlayerCredsIndicatorWidget_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
