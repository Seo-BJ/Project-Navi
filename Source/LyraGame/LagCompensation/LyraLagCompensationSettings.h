// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "LyraLagCompensationSettings.generated.h"

/**
 * Lag Compensation 프로젝트 세팅 (DefaultGame.ini, Project Settings).
 * 게임 동작에 영향을 주는 스위치만 보유한다 — 디버그는 ULyraLagCompensationDeveloperSettings 참조.
 */
UCLASS(config=Game, MinimalAPI)
class ULyraLagCompensationSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	ULyraLagCompensationSettings();

	/** Lag Compensation(Server-Side Rewind) 사용 여부. OFF 시 서버는 현재 시점 트레이스로 폴백 검증한다. */
	UPROPERTY(config, EditAnywhere, Category = "Lag Compensation", meta = (ConsoleVariable = "lyra.LagComp.Enabled"))
	bool bUseLagCompensation;
};

/**
 * Lag Compensation 에디터 개인 설정 (EditorPerProjectUserSettings).
 * 디버그 시각화 옵션 등 개발 용도 — 프로젝트에 커밋되지 않는다.
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class ULyraLagCompensationDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	ULyraLagCompensationDeveloperSettings();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

	/** SkeletalMesh 기반 Lag Compensation 디버그 시각화 ON/OFF */
	UPROPERTY(config, EditAnywhere, Category = "Debug Drawing", meta = (ConsoleVariable = "lyra.LagComp.DrawMesh"))
	bool bDrawMeshLagCompensation;

	/** 디버그 시각화 유지 시간 (초) */
	UPROPERTY(config, EditAnywhere, Category = "Debug Drawing", meta = (ConsoleVariable = "lyra.LagComp.DrawMeshDuration", ForceUnits = s))
	float DrawMeshLagCompensationDuration;
};
