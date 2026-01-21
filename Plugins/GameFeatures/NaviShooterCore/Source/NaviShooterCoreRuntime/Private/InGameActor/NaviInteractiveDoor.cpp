#include "InGameActor/NaviInteractiveDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractionQuery.h"

ANaviInteractiveDoor::ANaviInteractiveDoor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create Components
	// Note: ALyraActorWithAbilities already has a BoxComponent (Root) and StaticMeshComponent.
	// We will use the existing StaticMeshComponent as the DoorFrame or hide it, 
	// but to follow the "Frame/Body/Button" requirement clearly, let's create our own hierarchy 
	// or attach to the existing root.
	
	// Let's assume BoxComponent is the Root.
	
	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	DoorFrameMesh->SetupAttachment(RootComponent);
	
	DoorBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorBodyMesh"));
	DoorBodyMesh->SetupAttachment(DoorFrameMesh);
	
	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	ButtonMesh->SetupAttachment(DoorFrameMesh);
	
	// Defaults
	MoveType = EDoorMoveType::TranslateHorizontal;
	MoveAmount = 100.0f;
	MoveSpeed = 2.0f; // Units per second or alpha per second? Let's implement interpolation speed.
	
	bIsOpen = false;
	bIsDestroyed = false;

	// Setup Button Collision for Interaction
	ButtonMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // Ensure trace hits it
}

void ANaviInteractiveDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANaviInteractiveDoor, bIsOpen);
	DOREPLIFETIME(ANaviInteractiveDoor, bIsDestroyed);
}

void ANaviInteractiveDoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsValid(DoorBodyMesh))
	{
		InitialDoorTransform = DoorBodyMesh->GetRelativeTransform();
		TargetDoorTransform = InitialDoorTransform;
	}
}

void ANaviInteractiveDoor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Subscribe to HealthSet
		if (HealthSet)
		{
			// HealthSet->OnOutOfHealth.AddUObject(this, &ANaviInteractiveDoor::OnOutOfHealth);
		}
	}
}

void ANaviInteractiveDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateDoorMovement(DeltaTime);
}

void ANaviInteractiveDoor::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder)
{
	if (bIsDestroyed)
	{
		return;
	}

	// Check if the trace hit the ButtonMesh
	if (InteractQuery.RequestingAvatar.IsValid())
	{
		// Simple distance check or check if the HitResult component is the button
		// FInteractionQuery contains a generic trace result?
		// Usually Lyra's interaction trace passes the hit result in the query context?
		// Actually FInteractionQuery has OptionalObjectData or similar.
		// But usually, the "InteractableTarget" is the Actor.
		
		// We'll just provide the option. If precise button-only interaction is needed,
		// we would check the trace component.
		// For now, allow interaction if close enough (handled by the gather range).
		
		FInteractionOption BuilderOption = InteractionOption;
		
		// Dynamic Text based on state
		if (BuilderOption.Text.IsEmpty())
		{
			BuilderOption.Text = bIsOpen ? FText::FromString(TEXT("Close")) : FText::FromString(TEXT("Open"));
		}
		
		// Target this actor for the ability info
		BuilderOption.InteractableTarget = this;
		
		OptionBuilder.AddInteractionOption(BuilderOption);
	}
}

void ANaviInteractiveDoor::ToggleDoor()
{
	if (bIsDestroyed)
	{
		return;
	}

	bIsOpen = !bIsOpen;
	
	// OnRep will handle the visual logic start on clients, 
	// but on Server we need to trigger it too.
	OnRep_IsOpen(); 
}

void ANaviInteractiveDoor::OnRep_IsOpen()
{
	// Recalculate Target Transform
	TargetDoorTransform = InitialDoorTransform;

	if (bIsOpen)
	{
		switch (MoveType)
		{
		case EDoorMoveType::TranslateVertical:
			TargetDoorTransform.AddToTranslation(FVector(0, 0, MoveAmount));
			break;
		case EDoorMoveType::TranslateHorizontal:
			TargetDoorTransform.AddToTranslation(FVector(0, MoveAmount, 0)); 
			// Note: Axis depends on mesh orientation, assuming Y here or using Forward/Right vector logic
			// Simpler: Just use relative Y or X. Let's assume Y for "Horizontal".
			break;
		case EDoorMoveType::Rotate:
			TargetDoorTransform.SetRotation((InitialDoorTransform.GetRotation().Rotator() + FRotator(0, MoveAmount, 0)).Quaternion());
			break;
		}
	}
}

void ANaviInteractiveDoor::OnRep_IsDestroyed()
{
	if (bIsDestroyed)
	{
		// Visual destruction logic
		if (DoorBodyMesh)
		{
			DoorBodyMesh->SetSimulatePhysics(true);
			DoorBodyMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
			// Apply some impulse?
		}
		
		if (ButtonMesh)
		{
			// Maybe spark effect?
			ButtonMesh->SetVisibility(false); // Hide button or make it look broken
		}
	}
}

void ANaviInteractiveDoor::OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
	if (!bIsDestroyed)
	{
		bIsDestroyed = true;
		OnRep_IsDestroyed();
	}
}

void ANaviInteractiveDoor::UpdateDoorMovement(float DeltaTime)
{
	if (bIsDestroyed || !DoorBodyMesh)
	{
		return;
	}

	FTransform CurrentTransform = DoorBodyMesh->GetRelativeTransform();
	
	// Simple VInterp / RInterp
	FVector CurrentLoc = CurrentTransform.GetLocation();
	FQuat CurrentRot = CurrentTransform.GetRotation();
	
	FVector TargetLoc = TargetDoorTransform.GetLocation();
	FQuat TargetRot = TargetDoorTransform.GetRotation();
	
	bool bIsMoving = false;

	if (!CurrentLoc.Equals(TargetLoc, 0.1f))
	{
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, MoveSpeed);
		DoorBodyMesh->SetRelativeLocation(NewLoc);
		bIsMoving = true;
	}
	
	if (!CurrentRot.Equals(TargetRot, 0.01f))
	{
		FQuat NewRot = FMath::QInterpTo(CurrentRot, TargetRot, DeltaTime, MoveSpeed);
		DoorBodyMesh->SetRelativeRotation(NewRot);
		bIsMoving = true;
	}
}
