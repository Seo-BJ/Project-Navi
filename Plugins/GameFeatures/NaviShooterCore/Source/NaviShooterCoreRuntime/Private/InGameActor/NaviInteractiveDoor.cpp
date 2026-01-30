#include "InGameActor/NaviInteractiveDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractionQuery.h"
#include "Interaction/Abilities/LyraGameplayAbility_SimpleInteract.h"

ANaviInteractiveDoor::ANaviInteractiveDoor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 컴포넌트 생성
	// 참고: ALyraActorWithAbilities는 이미 BoxComponent(루트)와 StaticMeshComponent를 가지고 있습니다.
	// 기존 StaticMeshComponent를 문 프레임으로 사용하거나 숨길 수 있지만,
	// "프레임/본체/버튼" 요구사항을 명확히 따르기 위해 별도의 계층 구조를 생성하거나 기존 루트에 부착합니다.
	
	// BoxComponent가 루트라고 가정합니다.
	
	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	DoorFrameMesh->SetupAttachment(RootComponent);
	
	DoorBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorBodyMesh"));
	DoorBodyMesh->SetupAttachment(DoorFrameMesh);
	
	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	ButtonMesh->SetupAttachment(DoorFrameMesh);
	
	InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
	InteractionPoint->SetupAttachment(DoorFrameMesh);

	// 기본값 설정
	MoveType = EDoorMoveType::TranslateHorizontal;
	MoveAmount = 100.0f;
	MoveSpeed = 2.0f; // 초당 이동 단위 또는 알파? 보간 속도를 구현합니다.
	
	bIsOpen = false;
	bIsDestroyed = false;

	// 상호작용을 위한 버튼 콜리전 설정
	ButtonMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 트레이스가 감지할 수 있도록 설정
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
		// HealthSet 구독
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

	// 트레이스가 ButtonMesh에 닿았는지 확인
	if (InteractQuery.RequestingAvatar.IsValid())
	{
		// 단순 거리 체크 또는 HitResult 컴포넌트가 버튼인지 확인
		// FInteractionQuery에 일반적인 트레이스 결과가 포함되어 있나요?
		// 보통 Lyra의 상호작용 트레이스는 쿼리 컨텍스트에 히트 결과를 전달합니다.
		
		// 일단 옵션을 제공합니다. 정밀한 버튼 전용 상호작용이 필요한 경우,
		// 트레이스된 컴포넌트를 확인할 수 있습니다.
		// 현재는 수집 범위 내에 있으면 상호작용을 허용합니다.
		
		FInteractionOption BuilderOption = InteractionOption;

		// 설정된 어빌리티가 없으면 기본 SimpleInteract 어빌리티 사용
		if (BuilderOption.InteractionAbilityToGrant == nullptr)
		{
			BuilderOption.InteractionAbilityToGrant = ULyraGameplayAbility_SimpleInteract::StaticClass();
			BuilderOption.TargetAbilitySystem = nullptr; // 문의 ASC에서 실행하지 않도록 설정
			BuilderOption.TargetInteractionAbilityHandle = FGameplayAbilitySpecHandle();
		}
		
		// 상태에 따른 동적 텍스트 설정
		if (BuilderOption.Text.IsEmpty())
		{
			BuilderOption.Text = bIsOpen ? FText::FromString(TEXT("닫기")) : FText::FromString(TEXT("열기"));
		}
		
		// 어빌리티 정보를 위해 이 액터를 타겟으로 설정
		BuilderOption.InteractableTarget = this;
		
		OptionBuilder.AddInteractionOption(BuilderOption);
	}
}

void ANaviInteractiveDoor::SimpleInteract_Implementation(AActor* InstigatorActor)
{
	ToggleDoor();
}

void ANaviInteractiveDoor::ToggleDoor()
{
	if (bIsDestroyed)
	{
		return;
	}

	bIsOpen = !bIsOpen;
	
	// OnRep은 클라이언트에서 시각적 로직을 시작하지만, 
	// 서버에서도 실행해야 합니다.
	OnRep_IsOpen(); 
}

void ANaviInteractiveDoor::OnRep_IsOpen()
{
	// 목표 트랜스폼 재계산
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
			// 참고: 축은 메쉬 오리엔테이션에 따라 다르며, 여기서는 Y축이나 Forward/Right 로직을 가정합니다.
			// 단순화: 상대 Y 또는 X를 사용합니다. "평행 이동"을 위해 Y를 가정합니다.
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
		// 시각적 파괴 로직
		if (DoorBodyMesh)
		{
			DoorBodyMesh->SetSimulatePhysics(true);
			DoorBodyMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
			// 충격량 적용?
		}
		
		if (ButtonMesh)
		{
			// 스파크 효과 등?
			ButtonMesh->SetVisibility(false); // 버튼을 숨기거나 고장 난 것처럼 보이게 설정
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
	
	// 단순 VInterp / RInterp 사용
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