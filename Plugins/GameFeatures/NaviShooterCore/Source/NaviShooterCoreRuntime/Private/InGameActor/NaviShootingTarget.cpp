// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameActor/NaviShootingTarget.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANaviShootingTarget::ANaviShootingTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	// 루트를 캡슐로 설정 (캐릭터처럼)
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn")); // 적절한 콜리전 프로필 설정
	RootComponent = CapsuleComp;

	// 메시 설정
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CapsuleComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f)); // 캡슐 바닥에 맞춤
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// 부모 클래스(LyraActorWithAbilities)의 BoxComponent 등은 필요하면 사용하거나 무시
	// 여기서는 자체 CapsuleComp를 루트로 씀.
}

void ANaviShootingTarget::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	if (HasAuthority())
	{
		// Lyra HealthSet 이벤트 구독
		const ULyraHealthSet* Health = AbilitySystemComponent->GetSet<ULyraHealthSet>();
		if (Health)
		{
			Health->OnOutOfHealth.AddUObject(this, &ANaviShootingTarget::HandleOutOfHealth);
		}
	}
}

void ANaviShootingTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	// 1. 수명 체크
	TimeSinceSpawn += DeltaTime;
	if (TimeSinceSpawn >= CurrentLifespan)
	{
		OnTimeExpired();
		return;
	}

	// 2. Strafe (좌우 이동) 로직
	if (bShouldStrafe)
	{
		FVector CurrentLoc = GetActorLocation();
		
		// 스폰 위치 기준으로 좌우 왕복
		// 간단하게 FMath::Sin을 쓰거나 직접 이동시킬 수 있음. 여기선 등속 이동 구현.
		
		FVector DirectionVec = GetActorRightVector() * StrafeDirection;
		FVector NewLoc = CurrentLoc + (DirectionVec * StrafeSpeed * DeltaTime);

		// 범위 체크
		float Dist = FVector::Dist(NewLoc, SpawnLocation);
		// 만약 범위를 벗어나려고 하면 방향 반전
		// (단순 거리 체크는 원형이 되므로, Y축(Right) 거리만 체크하는 게 정확함)
		// 로컬 좌표계 기준 Y 거리 계산:
		FVector LocalDiff = GetActorTransform().InverseTransformPosition(NewLoc);
		
		if (FMath::Abs(LocalDiff.Y) > StrafeRange)
		{
			StrafeDirection *= -1.0f; // 방향 전환
		}

		SetActorLocation(NewLoc);
	}
}

void ANaviShootingTarget::InitTarget(float InLifespan, bool bInStrafe, bool bInArmor)
{
	CurrentLifespan = InLifespan;
	bShouldStrafe = bInStrafe;
	TimeSinceSpawn = 0.0f;
	bIsDead = false;

	// 아머 적용
	if (bInArmor && ArmorEffectClass && AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ArmorEffectClass, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ANaviShootingTarget::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{

	if (bIsDead) return;
	bIsDead = true;

	// 컨트롤러에게 알림 (Kill)
	OnTargetEliminated.Broadcast(this, true);

	// 사망 연출 후 파괴 (여기선 즉시 파괴하지만, 래그돌이나 몽타주 재생 가능)
	Destroy();
}

void ANaviShootingTarget::OnTimeExpired()
{
	if (bIsDead) return;
	bIsDead = true;

	// 컨트롤러에게 알림 (Miss / Not Killed)
	OnTargetEliminated.Broadcast(this, false);

	// 사라짐 효과 후 파괴
	Destroy();
}
