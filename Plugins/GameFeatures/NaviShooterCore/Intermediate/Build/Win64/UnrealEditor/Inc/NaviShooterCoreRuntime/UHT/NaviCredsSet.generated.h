// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "NaviCredsSet.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "Net/Core/PushModel/PushModelMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
struct FGameplayAttributeData;
#ifdef NAVISHOOTERCORERUNTIME_NaviCredsSet_generated_h
#error "NaviCredsSet.generated.h already included, missing '#pragma once' in NaviCredsSet.h"
#endif
#define NAVISHOOTERCORERUNTIME_NaviCredsSet_generated_h

#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execOnRep_MaxCreds); \
	DECLARE_FUNCTION(execOnRep_Creds);


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUNaviCredsSet(); \
	friend struct Z_Construct_UClass_UNaviCredsSet_Statics; \
public: \
	DECLARE_CLASS(UNaviCredsSet, ULyraAttributeSet, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/NaviShooterCoreRuntime"), NO_API) \
	DECLARE_SERIALIZER(UNaviCredsSet) \
	NO_API void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; \
	enum class ENetFields_Private : uint16 \
	{ \
		NETFIELD_REP_START=(uint16)((int32)Super::ENetFields_Private::NETFIELD_REP_END + (int32)1), \
		Creds=NETFIELD_REP_START, \
		MaxCreds, \
		NETFIELD_REP_END=MaxCreds	}; \
	NO_API virtual void ValidateGeneratedRepEnums(const TArray<struct FRepRecord>& ClassReps) const override; \
private: \
	REPLICATED_BASE_CLASS(UNaviCredsSet) \
public:


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UNaviCredsSet(UNaviCredsSet&&); \
	UNaviCredsSet(const UNaviCredsSet&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UNaviCredsSet); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UNaviCredsSet); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UNaviCredsSet) \
	NO_API virtual ~UNaviCredsSet();


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_18_PROLOG
#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_INCLASS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h_21_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<class UNaviCredsSet>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_NaviCredsSet_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
