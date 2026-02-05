// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LyraPatrolPath.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ELagCompensationTestMode : uint8
{
	Normal,				// 단순 보행
	JumpEachTrip,		// 각 왕복 시작 시 점프
	CrouchEachTrip,		// 항상 앉아서 이동
	ToggleCrouchPerTrip	// 왕복할 때마다 앉기/서기 전환
};

UCLASS()
class LYRAGAME_API ALyraPatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	ALyraPatrolPath();

	FVector GetPoint1WorldLocation() const;
	FVector GetPoint2WorldLocation() const;

protected:
	virtual void BeginPlay() override;

	// 스폰할 액터 클래스 (TestCharacter 또는 TestActor)
	UPROPERTY(EditInstanceOnly, Category = "Spawn Settings")
	TSubclassOf<AActor> ActorClassToSpawn;

	// 테스트 모드 설정
	UPROPERTY(EditInstanceOnly, Category = "Spawn Settings")
	ELagCompensationTestMode TestMode = ELagCompensationTestMode::Normal;

	// 이동 속도 설정
	UPROPERTY(EditInstanceOnly, Category = "Spawn Settings")
	float MovementSpeed = 300.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Point1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Point2;

private:
	UPROPERTY()
	TObjectPtr<AActor> SpawnedActor;
};
