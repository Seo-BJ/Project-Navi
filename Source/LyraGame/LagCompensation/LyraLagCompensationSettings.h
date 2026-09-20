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

	/**
	 * 서버가 되감아 줄 최대 시간(초). (서버 현재 시각 - 클라 HitTime)이 이보다 크면 그 사격의 히트를 모두 거부한다.
	 * 서버가 사격을 처리하는 시점 기준으로, 정상 클라의 되감기 양은 대략 RTT(클라 화면의 지연 + 요청이 서버에 오는 지연) + 보간 지연이다.
	 * 지원할 최대 RTT보다 크게, 스냅샷 보관 시간(MaxRecordTime)보다 작게 둔다.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Lag Compensation", meta = (ConsoleVariable = "lyra.LagComp.MaxRewindTime", ForceUnits = s, ClampMin = "0.0"))
	float MaxRewindTime;

	/**
	 * 미래 HitTime 허용 오차(초). 클라 HitTime이 서버 현재 시각보다 이만큼 넘게 미래면 그 사격의 히트를 모두 거부한다.
	 * 정상 클라의 HitTime은 항상 서버 현재 시각보다 과거이므로, 시간 추정 오차만 흡수할 정도로 작게 둔다.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Lag Compensation", meta = (ConsoleVariable = "lyra.LagComp.FutureHitTimeTolerance", ForceUnits = s, ClampMin = "0.0"))
	float FutureHitTimeTolerance;
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

	/** Box 기반 Lag Compensation 디버그 시각화 ON/OFF */
	UPROPERTY(config, EditAnywhere, Category = "Debug Drawing", meta = (ConsoleVariable = "lyra.LagComp.DrawBox"))
	bool bDrawBoxLagCompensation;

	/** Box 기반 디버그 시각화 유지 시간 (초) */
	UPROPERTY(config, EditAnywhere, Category = "Debug Drawing", meta = (ConsoleVariable = "lyra.LagComp.DrawBoxDuration", ForceUnits = s, ClampMin = "0.0"))
	float DrawBoxLagCompensationDuration;
};
