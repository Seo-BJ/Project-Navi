// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "NaviShooterCoreRuntime/Public/UI/HUD/NaviHealthAndArmorText.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeNaviHealthAndArmorText() {}

// Begin Cross Module References
COMMONUI_API UClass* Z_Construct_UClass_UCommonNumericTextBlock_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_APawn_NoRegister();
LYRAGAME_API UClass* Z_Construct_UClass_ULyraHealthComponent_NoRegister();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviHealthAndArmorText();
NAVISHOOTERCORERUNTIME_API UClass* Z_Construct_UClass_UNaviHealthAndArmorText_NoRegister();
UMG_API UClass* Z_Construct_UClass_UUserWidget();
UPackage* Z_Construct_UPackage__Script_NaviShooterCoreRuntime();
// End Cross Module References

// Begin Class UNaviHealthAndArmorText Function OnArmorChanged
struct Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics
{
	struct NaviHealthAndArmorText_eventOnArmorChanged_Parms
	{
		ULyraHealthComponent* HealthComponent;
		float OldValue;
		float NewValue;
		AActor* Instigator;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HealthComponent_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_HealthComponent;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OldValue;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewValue;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Instigator;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_HealthComponent = { "HealthComponent", nullptr, (EPropertyFlags)0x0010000000080080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnArmorChanged_Parms, HealthComponent), Z_Construct_UClass_ULyraHealthComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HealthComponent_MetaData), NewProp_HealthComponent_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_OldValue = { "OldValue", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnArmorChanged_Parms, OldValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_NewValue = { "NewValue", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnArmorChanged_Parms, NewValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_Instigator = { "Instigator", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnArmorChanged_Parms, Instigator), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_HealthComponent,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_OldValue,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_NewValue,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NewProp_Instigator,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviHealthAndArmorText, nullptr, "OnArmorChanged", nullptr, nullptr, Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NaviHealthAndArmorText_eventOnArmorChanged_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::NaviHealthAndArmorText_eventOnArmorChanged_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviHealthAndArmorText::execOnArmorChanged)
{
	P_GET_OBJECT(ULyraHealthComponent,Z_Param_HealthComponent);
	P_GET_PROPERTY(FFloatProperty,Z_Param_OldValue);
	P_GET_PROPERTY(FFloatProperty,Z_Param_NewValue);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnArmorChanged(Z_Param_HealthComponent,Z_Param_OldValue,Z_Param_NewValue,Z_Param_Instigator);
	P_NATIVE_END;
}
// End Class UNaviHealthAndArmorText Function OnArmorChanged

// Begin Class UNaviHealthAndArmorText Function OnHealthChanged
struct Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics
{
	struct NaviHealthAndArmorText_eventOnHealthChanged_Parms
	{
		ULyraHealthComponent* HealthComponent;
		float OldValue;
		float NewValue;
		AActor* Instigator;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HealthComponent_MetaData[] = {
		{ "EditInline", "true" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_HealthComponent;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OldValue;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewValue;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Instigator;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_HealthComponent = { "HealthComponent", nullptr, (EPropertyFlags)0x0010000000080080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnHealthChanged_Parms, HealthComponent), Z_Construct_UClass_ULyraHealthComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HealthComponent_MetaData), NewProp_HealthComponent_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_OldValue = { "OldValue", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnHealthChanged_Parms, OldValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_NewValue = { "NewValue", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnHealthChanged_Parms, NewValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_Instigator = { "Instigator", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnHealthChanged_Parms, Instigator), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_HealthComponent,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_OldValue,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_NewValue,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NewProp_Instigator,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviHealthAndArmorText, nullptr, "OnHealthChanged", nullptr, nullptr, Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NaviHealthAndArmorText_eventOnHealthChanged_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::NaviHealthAndArmorText_eventOnHealthChanged_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviHealthAndArmorText::execOnHealthChanged)
{
	P_GET_OBJECT(ULyraHealthComponent,Z_Param_HealthComponent);
	P_GET_PROPERTY(FFloatProperty,Z_Param_OldValue);
	P_GET_PROPERTY(FFloatProperty,Z_Param_NewValue);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnHealthChanged(Z_Param_HealthComponent,Z_Param_OldValue,Z_Param_NewValue,Z_Param_Instigator);
	P_NATIVE_END;
}
// End Class UNaviHealthAndArmorText Function OnHealthChanged

// Begin Class UNaviHealthAndArmorText Function OnPossessedPawnChanged
struct Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics
{
	struct NaviHealthAndArmorText_eventOnPossessedPawnChanged_Parms
	{
		APawn* OldPawn;
		APawn* NewPawn;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_OldPawn;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_NewPawn;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NewProp_OldPawn = { "OldPawn", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnPossessedPawnChanged_Parms, OldPawn), Z_Construct_UClass_APawn_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NewProp_NewPawn = { "NewPawn", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(NaviHealthAndArmorText_eventOnPossessedPawnChanged_Parms, NewPawn), Z_Construct_UClass_APawn_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NewProp_OldPawn,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NewProp_NewPawn,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UNaviHealthAndArmorText, nullptr, "OnPossessedPawnChanged", nullptr, nullptr, Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::PropPointers), sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NaviHealthAndArmorText_eventOnPossessedPawnChanged_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::Function_MetaDataParams), Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::NaviHealthAndArmorText_eventOnPossessedPawnChanged_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UNaviHealthAndArmorText::execOnPossessedPawnChanged)
{
	P_GET_OBJECT(APawn,Z_Param_OldPawn);
	P_GET_OBJECT(APawn,Z_Param_NewPawn);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->OnPossessedPawnChanged(Z_Param_OldPawn,Z_Param_NewPawn);
	P_NATIVE_END;
}
// End Class UNaviHealthAndArmorText Function OnPossessedPawnChanged

// Begin Class UNaviHealthAndArmorText
void UNaviHealthAndArmorText::StaticRegisterNativesUNaviHealthAndArmorText()
{
	UClass* Class = UNaviHealthAndArmorText::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "OnArmorChanged", &UNaviHealthAndArmorText::execOnArmorChanged },
		{ "OnHealthChanged", &UNaviHealthAndArmorText::execOnHealthChanged },
		{ "OnPossessedPawnChanged", &UNaviHealthAndArmorText::execOnPossessedPawnChanged },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UNaviHealthAndArmorText);
UClass* Z_Construct_UClass_UNaviHealthAndArmorText_NoRegister()
{
	return UNaviHealthAndArmorText::StaticClass();
}
struct Z_Construct_UClass_UNaviHealthAndArmorText_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "UI/HUD/NaviHealthAndArmorText.h" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HealthAmount_TextBlock_MetaData[] = {
		{ "BindWidget", "" },
		{ "Category", "NaviHealthAndArmorText" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ArmorAmount_TextBlock_MetaData[] = {
		{ "BindWidget", "" },
		{ "Category", "NaviHealthAndArmorText" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/UI/HUD/NaviHealthAndArmorText.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_HealthAmount_TextBlock;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ArmorAmount_TextBlock;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UNaviHealthAndArmorText_OnArmorChanged, "OnArmorChanged" }, // 1031677216
		{ &Z_Construct_UFunction_UNaviHealthAndArmorText_OnHealthChanged, "OnHealthChanged" }, // 1509111642
		{ &Z_Construct_UFunction_UNaviHealthAndArmorText_OnPossessedPawnChanged, "OnPossessedPawnChanged" }, // 1341578450
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UNaviHealthAndArmorText>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviHealthAndArmorText_Statics::NewProp_HealthAmount_TextBlock = { "HealthAmount_TextBlock", nullptr, (EPropertyFlags)0x012408000008000d, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviHealthAndArmorText, HealthAmount_TextBlock), Z_Construct_UClass_UCommonNumericTextBlock_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HealthAmount_TextBlock_MetaData), NewProp_HealthAmount_TextBlock_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UNaviHealthAndArmorText_Statics::NewProp_ArmorAmount_TextBlock = { "ArmorAmount_TextBlock", nullptr, (EPropertyFlags)0x012408000008000d, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UNaviHealthAndArmorText, ArmorAmount_TextBlock), Z_Construct_UClass_UCommonNumericTextBlock_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ArmorAmount_TextBlock_MetaData), NewProp_ArmorAmount_TextBlock_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UNaviHealthAndArmorText_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviHealthAndArmorText_Statics::NewProp_HealthAmount_TextBlock,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UNaviHealthAndArmorText_Statics::NewProp_ArmorAmount_TextBlock,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviHealthAndArmorText_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UNaviHealthAndArmorText_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UUserWidget,
	(UObject* (*)())Z_Construct_UPackage__Script_NaviShooterCoreRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviHealthAndArmorText_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UNaviHealthAndArmorText_Statics::ClassParams = {
	&UNaviHealthAndArmorText::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UNaviHealthAndArmorText_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UNaviHealthAndArmorText_Statics::PropPointers),
	0,
	0x00B010A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UNaviHealthAndArmorText_Statics::Class_MetaDataParams), Z_Construct_UClass_UNaviHealthAndArmorText_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UNaviHealthAndArmorText()
{
	if (!Z_Registration_Info_UClass_UNaviHealthAndArmorText.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UNaviHealthAndArmorText.OuterSingleton, Z_Construct_UClass_UNaviHealthAndArmorText_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UNaviHealthAndArmorText.OuterSingleton;
}
template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<UNaviHealthAndArmorText>()
{
	return UNaviHealthAndArmorText::StaticClass();
}
UNaviHealthAndArmorText::UNaviHealthAndArmorText(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UNaviHealthAndArmorText);
UNaviHealthAndArmorText::~UNaviHealthAndArmorText() {}
// End Class UNaviHealthAndArmorText

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UNaviHealthAndArmorText, UNaviHealthAndArmorText::StaticClass, TEXT("UNaviHealthAndArmorText"), &Z_Registration_Info_UClass_UNaviHealthAndArmorText, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UNaviHealthAndArmorText), 320645742U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_2821640453(TEXT("/Script/NaviShooterCoreRuntime"),
	Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
