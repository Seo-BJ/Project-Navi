#include "FlashBang/NaviFlashbangProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Engine/World.h"

ANaviFlashbangProjectile::ANaviFlashbangProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(FName("Projectile"));
	CollisionComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->MaxSpeed = 2000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComponent->SetupAttachment(CollisionComponent);
	
}

void ANaviFlashbangProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::Explode, FuseTime, false);
	}
}

void ANaviFlashbangProjectile::Explode()
{
	if (!HasAuthority())
	{
		return;
	}

	// 1. Find potential victims within radius
	TArray<AActor*> Victims;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		ExplosionRadius,
		ObjectTypes,
		APawn::StaticClass(),
		{ this }, // Ignore self
		Victims
	);

	// Get Instigator's ASC for context
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetInstigator());
	if (!SourceASC || !FlashEffectClass)
	{
		Destroy();
		return;
	}

	// Team Check Helpers
	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	int32 InstigatorTeamID = INDEX_NONE;
	if (TeamSubsystem)
	{
		InstigatorTeamID = TeamSubsystem->FindTeamFromObject(GetInstigator());
	}

	FVector FlashLocation = GetActorLocation();

	for (AActor* Victim : Victims)
	{
		if (!Victim) continue;

		// 2. Team Check
		if (!bCanBlindAllies && TeamSubsystem)
		{
			int32 VictimTeamID = TeamSubsystem->FindTeamFromObject(Victim);
			// Determine if same team (Lyra logic: TeamID != INDEX_NONE and match)
			bool bIsAlly = (InstigatorTeamID != INDEX_NONE) && (InstigatorTeamID == VictimTeamID);
			
			if (bIsAlly)
			{
				continue;
			}
		}

		// 3. Calculate Intensity
		float FinalIntensity = CalculateFlashIntensity(Victim, FlashLocation);

		// 4. Apply Gameplay Effect
		if (FinalIntensity > 0.01f)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Victim);
			if (TargetASC)
			{
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				Context.AddSourceObject(this);
				Context.AddInstigator(GetInstigator(), this);

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(FlashEffectClass, 1.0f, Context);
				if (SpecHandle.IsValid())
				{
					// Pass the calculated intensity via SetByCaller
					SpecHandle.Data->SetSetByCallerMagnitude(FlashIntensityTag, FinalIntensity);
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}

	Destroy();
}

float ANaviFlashbangProjectile::CalculateFlashIntensity(AActor* Victim, const FVector& FlashLocation)
{
	if (!Victim) return 0.0f;

	// A. Occlusion Check (Trace)
	FVector VictimEyeLoc;
	FRotator VictimEyeRot;
	Victim->GetActorEyesViewPoint(VictimEyeLoc, VictimEyeRot);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Victim);

	// Slight offset from center to avoid getting stuck in geometry
	FVector StartTrace = FlashLocation;
	
	// Use SphereTrace for better occlusion reliability (anti-pixel-perfect blocking)
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartTrace,
		VictimEyeLoc,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(OcclusionTraceRadius),
		Params
	);

	if (bHit)
	{
		// Blocked by geometry
		return 0.0f;
	}

	// B. Distance Attenuation
	float Distance = FVector::Dist(FlashLocation, VictimEyeLoc);
	float DistanceFactor = FMath::Clamp(1.0f - (Distance / ExplosionRadius), 0.0f, 1.0f);

	// C. Angle Attenuation (Dot Product)
	FVector DirToFlash = (FlashLocation - VictimEyeLoc).GetSafeNormal();
	float DotProduct = FVector::DotProduct(VictimEyeRot.Vector(), DirToFlash);

	float AngleFactor = 0.0f;
	if (DotProduct > 0.5f) // Looking directly (~60 degrees)
	{
		AngleFactor = 1.0f;
	}
	else if (DotProduct > 0.0f) // Looking somewhat towards it
	{
		AngleFactor = 0.5f; 
	}
	else // Looking away
	{
		// Partial blind for being close but looking away
		AngleFactor = 0.1f;
	}

	return DistanceFactor * AngleFactor;
}
