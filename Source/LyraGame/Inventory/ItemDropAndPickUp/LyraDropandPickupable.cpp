// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDropandPickupable.h"

#include "Async/TaskGraphInterfaces.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ALyraDropandPickupable::ALyraDropandPickupable()
{
	PrimaryActorTick.bCanEverTick = false;

	// 물리 이동을 위한 충돌체
	MovementCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("MovementCollision"));
	MovementCollision->SetupAttachment(RootComponent);
	// MovementCollision->SetSphereRadius(10.0f);
	// WorldStatic(바닥, 벽 등)에만 충돌하도록 설정
	MovementCollision->SetCollisionProfileName(TEXT("BlockAllDynamic")); 
	MovementCollision->SetSimulatePhysics(false); // 물리 시뮬레이션은 ProjectileComponent가 담당
	RootComponent = MovementCollision;
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bAutoActivate = false;
	ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
	ProjectileMovementComponent->InitialSpeed = 0.0f;
	ProjectileMovementComponent->MaxSpeed = 4000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true; // Hit 이벤트를 발생시키기 위해 true로 설정
	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
}


void ALyraDropandPickupable::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

FInventoryPickup ALyraDropandPickupable::GetPickupInventory() const
{
	return StaticInventory;
}

void ALyraDropandPickupable::BeginPlay()
{
	Super::BeginPlay();
	if (MovementCollision)
	{
		MovementCollision->OnComponentHit.AddDynamic(this, &ALyraDropandPickupable::OnProjectileHit);
	}
}

void ALyraDropandPickupable::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 땅이나 벽 등 어떤 것에든 한 번 부딪히면 즉시 물리 효과를 정지시킴
	if (ProjectileMovementComponent && ProjectileMovementComponent->IsActive())
	{
		// 1. 발사체 컴포넌트 비활성화
		ProjectileMovementComponent->Deactivate();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;

		// 2. 물리 이동용 충돌체 비활성화 (더 이상 충돌 불필요)
		MovementCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 3. 루트 컴포넌트의 움직임 완전히 정지
		MovementCollision->SetSimulatePhysics(false);

		// 4. 바닥에 붙도록 위치와 회전 고정
		SetActorLocation(FVector(Hit.ImpactPoint.X, Hit.ImpactPoint.Y,	Hit.ImpactPoint.Z + 10.0f));
		SetActorRotation(FRotator(0, GetActorRotation().Yaw, 0)); // 바닥에 평평하게 눕도록 회전 조정

		// 델리게이트를 한 번만 실행하도록 바인딩 해제
		MovementCollision->OnComponentHit.RemoveDynamic(this, &ALyraDropandPickupable::OnProjectileHit);
	}
}
