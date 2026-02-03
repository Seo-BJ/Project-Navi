// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NaviShootingTestComponent.h"
#include "Net/UnrealNetwork.h"
#include "InGameActor/NaviShootingTarget.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"


UNaviShootingTestComponent::UNaviShootingTestComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UNaviShootingTestComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNaviShootingTestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndTest();
	Super::EndPlay(EndPlayReason);
}

void UNaviShootingTestComponent::StartTest(AActor* Instigator, const FNaviShootingTestSettings& InSettings)
{
	if (bIsTestRunning)
	{
		EndTest();
	}

	TestInstigator = Instigator;
	CurrentSettings = InSettings;
	bIsTestRunning = true;
	CurrentKillCount = 0;
	
	// 목표 킬 수 설정
	switch (CurrentSettings.Streak)
	{
	case ENaviTestStreak::Limit50: TargetKillCount = 50; break;
	case ENaviTestStreak::Limit100: TargetKillCount = 100; break;
	case ENaviTestStreak::Endless: TargetKillCount = -1; break;
	}

	// 무한 탄창 적용
	if (CurrentSettings.bInfiniteAmmo)
	{
		SetInfiniteAmmo(true);
	}

	// Server Broadcast
	OnRep_IsTestRunning();
	OnRep_CurrentKillCount();

	// 첫 타겟 스폰 (약간의 딜레이 후 시작)
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &UNaviShootingTestComponent::SpawnNextTarget, 1.0f, false);
	
	DrawDebugRange();
}

void UNaviShootingTestComponent::EndTest()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!bIsTestRunning) return;

	bIsTestRunning = false;
	
	// 타이머 정지
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	// 남은 타겟 정리
	CleanupTargets();

	// 무한 탄창 제거
	SetInfiniteAmmo(false);
	TestInstigator = nullptr;

	OnRep_IsTestRunning();
}

void UNaviShootingTestComponent::UpdateSettings(const FNaviShootingTestSettings& NewSettings)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!bIsTestRunning)
	{
		CurrentSettings = NewSettings;
	}
}

void UNaviShootingTestComponent::SpawnNextTarget()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!bIsTestRunning || !TargetClass)
	{
		EndTest();
		return;
	}

	// 1. 기준점 및 방향 구하기
	FVector OriginLocation;
	FVector ForwardDir;

	// SpawnPoint를 기준점(Anchor)으로 사용
	if (SpawnPoint)
	{
		OriginLocation = SpawnPoint->GetActorLocation();
		ForwardDir = SpawnPoint->GetActorForwardVector();
	}
	else
	{
		// 스폰 포인트가 없으면 플레이어 기준으로 (Fallback)
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC && PC->GetPawn())
		{
			OriginLocation = PC->GetPawn()->GetActorLocation();
			ForwardDir = PC->GetPawn()->GetActorForwardVector();
		}
		else
		{
			EndTest(); // 기준점 없음
			return; 
		}
	}

	// 2. 랜덤 극좌표 생성 (부채꼴 범위)
	float RandomDist = FMath::RandRange(CurrentSettings.MinSpawnDistance, CurrentSettings.MaxSpawnDistance);
	float HalfAngle = CurrentSettings.SpawnAngleFOV * 0.5f;
	float RandomAngleDeg = FMath::RandRange(-HalfAngle, HalfAngle);

	// 3. 회전 적용 (Forward 벡터를 Z축 기준으로 회전)
	FVector SpawnDir = ForwardDir.RotateAngleAxis(RandomAngleDeg, FVector::UpVector);
	FVector TargetLocation = OriginLocation + (SpawnDir * RandomDist);

	// 4. 바닥 보정 (Z축 찾기)
	FHitResult HitResult;
	FVector TraceStart = TargetLocation + FVector(0, 0, 500.0f); // 위에서
	FVector TraceEnd = TargetLocation - FVector(0, 0, 1000.0f);  // 아래로
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	float AddedHeight = 0.0f;
	if (TargetClass)
	{
		if (const ANaviShootingTarget* CDO = Cast<ANaviShootingTarget>(TargetClass->GetDefaultObject()))
		{
			if (const UCapsuleComponent* Cap = CDO->GetCapsuleComponent())
			{
				AddedHeight = Cap->GetScaledCapsuleHalfHeight();
			}
		}
	}

	// WorldStatic(바닥)만 체크
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		TargetLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, AddedHeight);
	}
	else
	{
		// 바닥을 못 찾으면 기준점 높이 사용
		TargetLocation.Z = OriginLocation.Z + AddedHeight;
	}

	// 5. 스폰 (타겟이 기준점을 바라보게 회전)
	FRotator LookAtRot = (OriginLocation - TargetLocation).Rotation();
	LookAtRot.Pitch = 0.0f; // 수평 유지

	FTransform SpawnTransform(LookAtRot, TargetLocation);

	ANaviShootingTarget* NewTarget = GetWorld()->SpawnActorDeferred<ANaviShootingTarget>(
		TargetClass, 
		SpawnTransform, 
		GetOwner(), 
		nullptr, 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (NewTarget)
	{
		// 초기화
		NewTarget->InitTarget(GetTargetLifespan(), CurrentSettings.BotMovement == ENaviBotMovement::Strafe, CurrentSettings.bBotArmor);
		
		// 이벤트 바인딩
		NewTarget->OnTargetEliminated.AddDynamic(this, &UNaviShootingTestComponent::OnTargetEliminated);

		UGameplayStatics::FinishSpawningActor(NewTarget, SpawnTransform);
		ActiveTargets.Add(NewTarget);
	}

	// 범위 다시 그리기
	DrawDebugRange();
}

void UNaviShootingTestComponent::OnTargetEliminated(ANaviShootingTarget* Target, bool bWasKilled)
{
	if (Target)
	{
		ActiveTargets.Remove(Target);
	}

	if (!bIsTestRunning) return;

	if (bWasKilled)
	{
		CurrentKillCount++;
		OnRep_CurrentKillCount();

		// 목표 달성 체크
		if (TargetKillCount > 0 && CurrentKillCount >= TargetKillCount)
		{
			EndTest(); // 성공 종료
			return;
		}
	}
	else
	{
		// 시간 초과 (Miss)
	}

	// 다음 타겟 바로 스폰
	SpawnNextTarget();
}

void UNaviShootingTestComponent::CleanupTargets()
{
	for (ANaviShootingTarget* Target : ActiveTargets)
	{
		if (IsValid(Target))
		{
			Target->Destroy();
		}
	}
	ActiveTargets.Reset();
}

float UNaviShootingTestComponent::GetTargetLifespan() const
{
	switch (CurrentSettings.Speed)
	{
	case ENaviTestSpeed::Easy:   return 30.0f;
	case ENaviTestSpeed::Medium: return 1.0f;
	case ENaviTestSpeed::Hard:   return 0.5f;
	default: return 2.0f;
	}
}

void UNaviShootingTestComponent::SetInfiniteAmmo(bool bEnable)
{
	if (!TestInstigator) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TestInstigator))
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (ASC)
		{
			if (bEnable && InfiniteAmmoEffectClass)
			{
				if (!InfiniteAmmoHandle.IsValid())
				{
					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					InfiniteAmmoHandle = ASC->ApplyGameplayEffectToSelf(InfiniteAmmoEffectClass.GetDefaultObject(), 1.0f, Context);
				}
			}
			else if (!bEnable && InfiniteAmmoHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(InfiniteAmmoHandle);
				InfiniteAmmoHandle.Invalidate();
			}
		}
	}
}

void UNaviShootingTestComponent::DrawDebugRange()
{
#if UE_BUILD_SHIPPING
	return;
#endif

	if (!SpawnPoint) return;

	FVector Origin = SpawnPoint->GetActorLocation();
	FVector Forward = SpawnPoint->GetActorForwardVector();

	float MinDist = CurrentSettings.MinSpawnDistance;
	float MaxDist = CurrentSettings.MaxSpawnDistance;

	int32 Segments = 16;
	FColor Color = FColor::Green;
	float Duration = 5.0f;
	float Thickness = 3.0f;

	auto DrawArc = [&](float Radius)
	{
		FVector PrevPoint = Origin + Forward.RotateAngleAxis(-CurrentSettings.SpawnAngleFOV * 0.5f, FVector::UpVector) * Radius;

		for (int32 i = 1; i <= Segments; ++i)
		{
			float AngleDeg = -CurrentSettings.SpawnAngleFOV * 0.5f + (CurrentSettings.SpawnAngleFOV * ((float)i / Segments));
			FVector NextPoint = Origin + Forward.RotateAngleAxis(AngleDeg, FVector::UpVector) * Radius;

			DrawDebugLine(GetWorld(), PrevPoint, NextPoint, Color, false, Duration, 0, Thickness);
			PrevPoint = NextPoint;
		}
	};

	DrawArc(MinDist);
	DrawArc(MaxDist);

	FVector LeftDir = Forward.RotateAngleAxis(-CurrentSettings.SpawnAngleFOV * 0.5f, FVector::UpVector);
	FVector RightDir = Forward.RotateAngleAxis(CurrentSettings.SpawnAngleFOV * 0.5f, FVector::UpVector);

	DrawDebugLine(GetWorld(), Origin + LeftDir * MinDist, Origin + LeftDir * MaxDist, Color, false, Duration, 0, Thickness);
	DrawDebugLine(GetWorld(), Origin + RightDir * MinDist, Origin + RightDir * MaxDist, Color, false, Duration, 0, Thickness);
}

void UNaviShootingTestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNaviShootingTestComponent, CurrentSettings);
	DOREPLIFETIME(UNaviShootingTestComponent, bIsTestRunning);
	DOREPLIFETIME(UNaviShootingTestComponent, CurrentKillCount);
	DOREPLIFETIME(UNaviShootingTestComponent, TargetKillCount);
	DOREPLIFETIME(UNaviShootingTestComponent, TestInstigator);
}

void UNaviShootingTestComponent::OnRep_IsTestRunning()
{
	OnTestStateChanged.Broadcast(bIsTestRunning);
}

void UNaviShootingTestComponent::OnRep_CurrentKillCount()
{
	OnScoreUpdated.Broadcast(CurrentKillCount, TargetKillCount);
}