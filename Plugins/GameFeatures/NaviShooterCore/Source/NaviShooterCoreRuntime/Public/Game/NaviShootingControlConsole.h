// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/NaviShootingTestTypes.h"
#include "Interaction/LyraWeaponInteractable.h"


#include "NaviShootingControlConsole.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UTextRenderComponent;
class UNaviShootingTestComponent;
class ULyraExperienceDefinition;

/**
 * ANaviShootingControlConsole
 * 
 * 사격 훈련 설정을 변경하고 시작할 수 있는 조작 패널입니다.
 * 각 버튼(메쉬)을 사격하여 설정을 변경합니다.
 */
UCLASS()
class NAVISHOOTERCORERUNTIME_API ANaviShootingControlConsole : public AActor, public ILyraWeaponInteractable
{
	GENERATED_BODY()
	
public:	
	ANaviShootingControlConsole();

	//~ILyraWeaponInteractable interface
	virtual void OnWeaponHit(const FHitResult& HitResult, AActor* Shooter) override;
	//~End ILyraWeaponInteractable interface

	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End AActor interface
	
protected:
	virtual void BeginPlay() override;
	
	// --- 버튼 컴포넌트들 (사격 대상) ---
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> StartButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> SpeedButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> StreakButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> BotMoveButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> ArmorButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UStaticMeshComponent> AmmoButton;

	// --- 디스플레이 ---
	
	/** 설정을 보여줄 3D 텍스트 또는 위젯 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<UTextRenderComponent> StatusText;

	/** 이 콘솔이 제어할 사격 훈련장의 스폰 포인트 (레벨에 배치된 TargetPoint 등을 연결) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Navi|Console")
	TObjectPtr<AActor> LevelSpawnPoint;

	// --- 내부 상태 ---

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSettings)
	FNaviShootingTestSettings CurrentSettings;

	UFUNCTION()
	void OnRep_CurrentSettings();

	/** 연동된 테스트 컴포넌트 (캐싱) */
	UPROPERTY()
	TObjectPtr<UNaviShootingTestComponent> TestComponent;

	// --- 함수 ---

	/** GameState에서 컴포넌트 찾기 */
	void FindTestComponent();

	/** UI 텍스트 갱신 */
	void UpdateDisplay();

	/** Experience 로딩 완료 시 호출 */
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	/** Enum -> String 변환 헬퍼 */
	FString GetSpeedString(ENaviTestSpeed Speed) const;
	FString GetStreakString(ENaviTestStreak Streak) const;
	FString GetMoveString(ENaviBotMovement Move) const;

};
