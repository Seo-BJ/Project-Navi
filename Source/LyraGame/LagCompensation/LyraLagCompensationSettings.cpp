// Fill out your copyright notice in the Description page of Project Settings.

#include "LyraLagCompensationSettings.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLagCompensationSettings)

// UDeveloperSettingsBackedByCVars 가 meta=(ConsoleVariable=...) 속성을 처리하려면
// 모듈 로드 시점에 해당 CVar들이 사전 등록되어 있어야 한다.
// 패키지(빌드)에서는 EditorPerProjectUserSettings 가 로드되지 않으므로
// CVar → UPROPERTY(CDO) 단방향 동기화 콜백을 달아 어디서든 토글 가능하게 한다.
namespace LyraLagCompensationCVars
{
	static void SyncUseLagCompensationToCDO(IConsoleVariable* Var)
	{
		ULyraLagCompensationSettings* Settings = GetMutableDefault<ULyraLagCompensationSettings>();
		Settings->bUseLagCompensation = Var->GetBool();
	}

	static void SyncDrawMeshToCDO(IConsoleVariable* Var)
	{
		ULyraLagCompensationDeveloperSettings* Settings = GetMutableDefault<ULyraLagCompensationDeveloperSettings>();
		Settings->bDrawMeshLagCompensation = Var->GetBool();
	}

	static void SyncDrawMeshDurationToCDO(IConsoleVariable* Var)
	{
		ULyraLagCompensationDeveloperSettings* Settings = GetMutableDefault<ULyraLagCompensationDeveloperSettings>();
		Settings->DrawMeshLagCompensationDuration = Var->GetFloat();
	}

	static bool bUseLagCompensation = true;
	static FAutoConsoleVariableRef CVarUseLagCompensation(
		TEXT("lyra.LagComp.Enabled"),
		bUseLagCompensation,
		TEXT("Lag Compensation(Server-Side Rewind) 사용 여부. ")
		TEXT("OFF 시 서버는 현재 시점 LineTrace로 폴백 검증한다."),
		FConsoleVariableDelegate::CreateStatic(&SyncUseLagCompensationToCDO),
		ECVF_Default);

	static bool bDrawMeshLagCompensation = false;
	static FAutoConsoleVariableRef CVarDrawMesh(
		TEXT("lyra.LagComp.DrawMesh"),
		bDrawMeshLagCompensation,
		TEXT("SkeletalMesh 기반 Lag Compensation 디버그 시각화 ON/OFF"),
		FConsoleVariableDelegate::CreateStatic(&SyncDrawMeshToCDO),
		ECVF_Default);

	static float DrawMeshLagCompensationDuration = 3.0f;
	static FAutoConsoleVariableRef CVarDrawMeshDuration(
		TEXT("lyra.LagComp.DrawMeshDuration"),
		DrawMeshLagCompensationDuration,
		TEXT("Lag Compensation 디버그 시각화 유지 시간 (초)"),
		FConsoleVariableDelegate::CreateStatic(&SyncDrawMeshDurationToCDO),
		ECVF_Default);
}

ULyraLagCompensationSettings::ULyraLagCompensationSettings()
	: bUseLagCompensation(true)
{
	CategoryName = TEXT("Game");
}

ULyraLagCompensationDeveloperSettings::ULyraLagCompensationDeveloperSettings()
	: bDrawMeshLagCompensation(false)
	, DrawMeshLagCompensationDuration(3.0f)
{
}

FName ULyraLagCompensationDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}
