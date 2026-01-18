#include "SmokeGrenade/NaviSmokeGrenadeActor.h"
#include "Components/StaticMeshComponent.h"

ANaviSmokeGrenadeActor::ANaviSmokeGrenadeActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SmokeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmokeMeshComponent"));
	RootComponent = SmokeMeshComponent;

	// Default mesh setup
	SmokeMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SmokeMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// Block Visibility for AI/LineOfSight checks
	SmokeMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Default values
	SmokeDuration = 15.0f;
	SmokeRadius = 250.0f; // Default radius 250 units
}

void ANaviSmokeGrenadeActor::BeginPlay()
{
	Super::BeginPlay();

	// Create Dynamic Material Instance
	if (SmokeMeshComponent)
	{
		SmokeMaterialInstance = SmokeMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	}

	// Apply Scale based on Radius
	// Standard Sphere is 100 units diameter (radius 50)
	if (SmokeRadius > 0.0f)
	{
		float ScaleFactor = SmokeRadius / 50.0f; 
		SetActorScale3D(FVector(ScaleFactor));
	}

	if (HasAuthority())
	{
		// Start timer for dissipation instead of hard SetLifeSpan
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_SmokeDuration, this, &ANaviSmokeGrenadeActor::OnDissipate, SmokeDuration, false);
	}
}

void ANaviSmokeGrenadeActor::FinishDissipation()
{
	Destroy();
}

void ANaviSmokeGrenadeActor::SetSmokeScalarParameter(FName ParamName, float Value)
{
	if (SmokeMaterialInstance)
	{
		SmokeMaterialInstance->SetScalarParameterValue(ParamName, Value);
	}
}
