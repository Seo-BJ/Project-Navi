// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "UI/HUD/NaviHealthAndArmorText.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class AActor;
class APawn;
class ULyraHealthComponent;
#ifdef NAVISHOOTERCORERUNTIME_NaviHealthAndArmorText_generated_h
#error "NaviHealthAndArmorText.generated.h already included, missing '#pragma once' in NaviHealthAndArmorText.h"
#endif
#define NAVISHOOTERCORERUNTIME_NaviHealthAndArmorText_generated_h

#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execOnArmorChanged); \
	DECLARE_FUNCTION(execOnHealthChanged); \
	DECLARE_FUNCTION(execOnPossessedPawnChanged);


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUNaviHealthAndArmorText(); \
	friend struct Z_Construct_UClass_UNaviHealthAndArmorText_Statics; \
public: \
	DECLARE_CLASS(UNaviHealthAndArmorText, UUserWidget, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/NaviShooterCoreRuntime"), NO_API) \
	DECLARE_SERIALIZER(UNaviHealthAndArmorText)


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UNaviHealthAndArmorText(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UNaviHealthAndArmorText(UNaviHealthAndArmorText&&); \
	UNaviHealthAndArmorText(const UNaviHealthAndArmorText&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UNaviHealthAndArmorText); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UNaviHealthAndArmorText); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UNaviHealthAndArmorText) \
	NO_API virtual ~UNaviHealthAndArmorText();


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_15_PROLOG
#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_INCLASS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h_18_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<class UNaviHealthAndArmorText>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_UI_HUD_NaviHealthAndArmorText_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
