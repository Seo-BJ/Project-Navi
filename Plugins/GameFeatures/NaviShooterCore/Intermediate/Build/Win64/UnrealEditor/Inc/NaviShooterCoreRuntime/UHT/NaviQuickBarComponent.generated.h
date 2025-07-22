// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Equipment/NaviQuickBarComponent.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class ULyraInventoryItemInstance;
struct FGameplayTag;
#ifdef NAVISHOOTERCORERUNTIME_NaviQuickBarComponent_generated_h
#error "NaviQuickBarComponent.generated.h already included, missing '#pragma once' in NaviQuickBarComponent.h"
#endif
#define NAVISHOOTERCORERUNTIME_NaviQuickBarComponent_generated_h

#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_21_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FNaviQuickBarSlotsChangedMessage_Statics; \
	NAVISHOOTERCORERUNTIME_API static class UScriptStruct* StaticStruct();


template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<struct FNaviQuickBarSlotsChangedMessage>();

#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_34_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FNaviQuickBarActiveIndexChangedMessage_Statics; \
	NAVISHOOTERCORERUNTIME_API static class UScriptStruct* StaticStruct();


template<> NAVISHOOTERCORERUNTIME_API UScriptStruct* StaticStruct<struct FNaviQuickBarActiveIndexChangedMessage>();

#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_RPC_WRAPPERS_NO_PURE_DECLS \
	virtual void SetActiveSlotIndex_Implementation(int32 NewIndex); \
	DECLARE_FUNCTION(execOnRep_ActiveSlotIndex); \
	DECLARE_FUNCTION(execOnRep_Slots); \
	DECLARE_FUNCTION(execRemoveItemFromSlot); \
	DECLARE_FUNCTION(execAddItemToSlot); \
	DECLARE_FUNCTION(execGetActiveSlotItem); \
	DECLARE_FUNCTION(execGetActiveSlotIndex); \
	DECLARE_FUNCTION(execGetSlots); \
	DECLARE_FUNCTION(execSetActiveSlotIndex); \
	DECLARE_FUNCTION(execCycleActiveSlotBackward); \
	DECLARE_FUNCTION(execCycleActiveSlotForward);


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_CALLBACK_WRAPPERS
#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUNaviQuickBarComponent(); \
	friend struct Z_Construct_UClass_UNaviQuickBarComponent_Statics; \
public: \
	DECLARE_CLASS(UNaviQuickBarComponent, UControllerComponent, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/NaviShooterCoreRuntime"), NO_API) \
	DECLARE_SERIALIZER(UNaviQuickBarComponent) \
	NO_API void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; \
	enum class ENetFields_Private : uint16 \
	{ \
		NETFIELD_REP_START=(uint16)((int32)Super::ENetFields_Private::NETFIELD_REP_END + (int32)1), \
		Slots=NETFIELD_REP_START, \
		ActiveSlotIndex, \
		NETFIELD_REP_END=ActiveSlotIndex	}; \
	NO_API virtual void ValidateGeneratedRepEnums(const TArray<struct FRepRecord>& ClassReps) const override;


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UNaviQuickBarComponent(UNaviQuickBarComponent&&); \
	UNaviQuickBarComponent(const UNaviQuickBarComponent&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UNaviQuickBarComponent); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UNaviQuickBarComponent); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UNaviQuickBarComponent) \
	NO_API virtual ~UNaviQuickBarComponent();


#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_47_PROLOG
#define FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_CALLBACK_WRAPPERS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_INCLASS_NO_PURE_DECLS \
	FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h_50_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> NAVISHOOTERCORERUNTIME_API UClass* StaticClass<class UNaviQuickBarComponent>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_binij_Documents_UnrealProjects_Navi_Plugins_GameFeatures_NaviShooterCore_Source_NaviShooterCoreRuntime_Public_Equipment_NaviQuickBarComponent_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
