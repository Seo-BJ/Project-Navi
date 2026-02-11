// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "LyraReplicationGraphTypes.h"
#include "LyraReplicationGraphSettings.generated.h"

/**
 * Lyra 레플리케이션 그래프를 위한 기본 설정입니다.
 */
UCLASS(config=Game, MinimalAPI)
class ULyraReplicationGraphSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	ULyraReplicationGraphSettings();

public:

	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	bool bDisableReplicationGraph = true;

	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph, meta = (MetaClass = "/Script/LyraGame.LyraReplicationGraph"))
	FSoftClassPath DefaultReplicationGraphClass;

	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Lyra.RepGraph.EnableFastSharedPath"))
	bool bEnableFastSharedPath = true;

	// FastShared 이동 업데이트에 사용할 대역폭입니다. 이는 NetDriver의 타겟 대역폭과는 별개로 계산됩니다.
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ForceUnits=Kilobytes, ConsoleVariable = "Lyra.RepGraph.TargetKBytesSecFastSharedPath"))
	int32 TargetKBytesSecFastSharedPath = 10;

	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Lyra.RepGraph.FastSharedPathCullDistPct"))
	float FastSharedPathCullDistPct = 0.80f;

	UPROPERTY(EditAnywhere, Category = DestructionInfo, meta = (ForceUnits = cm, ConsoleVariable = "Lyra.RepGraph.DestructInfo.MaxDist"))
	float DestructionInfoMaxDist = 30000.f;

	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.CellSize"))
	float SpatialGridCellSize = 10000.0f;

	// 레플리케이션을 위한 실질적인 "최소 X" 값입니다. 이는 초기값일 뿐이며, 액터가 이 범위를 벗어나면 시스템이 스스로 재설정합니다.
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.SpatialBiasX"))
	float SpatialBiasX = -200000.0f;

	// 레플리케이션을 위한 실질적인 "최소 Y" 값입니다. 이는 초기값일 뿐이며, 액터가 이 범위를 벗어나면 시스템이 스스로 재설정합니다.
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.SpatialBiasY"))
	float SpatialBiasY = -200000.0f;

	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta = (ConsoleVariable = "Lyra.RepGraph.DisableSpatialRebuilds"))
	bool bDisableSpatialRebuilds = true;

	// 동적 공간화 액터들을 분산시킬 버킷의 수입니다.
	// 숫자가 높을수록 버킷이 많아지고 실질적인 레플리케이션 빈도가 낮아집니다.
	// 이는 각 액터가 자신의 NetUpdateFrequency를 체크하기 전에 먼저 적용됩니다.
	UPROPERTY(EditAnywhere, Category = DynamicSpatialFrequency, meta = (ConsoleVariable = "Lyra.RepGraph.DynamicActorFrequencyBuckets"))
	int32 DynamicActorFrequencyBuckets = 3;

	/** 가시성 체크에 사용할 트레이스 채널입니다. */
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	TEnumAsByte<ECollisionChannel> VisibleConnectionTraceChannel;

	/** 미래 예측 가시성 체크 모델을 활성화할지 여부입니다. */
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	bool bEnablePredictiveVisibility = true;

	// 특정 클래스들을 위한 커스텀 설정 배열입니다.
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	TArray<FRepGraphActorClassSettings> ClassSettings;
};
