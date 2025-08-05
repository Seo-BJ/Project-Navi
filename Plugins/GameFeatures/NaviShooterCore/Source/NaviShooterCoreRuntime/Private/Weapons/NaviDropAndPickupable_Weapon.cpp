// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NaviDropAndPickupable_Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h" 
#include "Equipment/NaviQuickBarComponent.h"
#include "Inventory/LyraInventoryManagerComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ANaviDropAndPickupable_Weapon::ANaviDropAndPickupable_Weapon()
{
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(RootComponent);
	OverlapCollision = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapCollision"));
	OverlapCollision->SetupAttachment(WeaponMeshComponent); // 무기 메시에 붙여 위치를 동기화합니다.
	OverlapCollision->SetSphereRadius(80.0f); // 넉넉한 감지 반경 설정
	OverlapCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Overlap 이벤트를 발생시키는 프로파일 사용
	OverlapCollision->SetGenerateOverlapEvents(true);
	
	bReplicates = true; 
	

}
void ANaviDropAndPickupable_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANaviDropAndPickupable_Weapon, ReplicatedWeaponMesh);
}

void ANaviDropAndPickupable_Weapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		OverlapCollision->OnComponentBeginOverlap.AddDynamic(this, &ANaviDropAndPickupable_Weapon::OnOverlapBegin);
	}
}


void ANaviDropAndPickupable_Weapon::SetSkeletalMesh(USkeletalMesh* SkeletalMesh)
{
	if (SkeletalMesh && HasAuthority())
	{
		ReplicatedWeaponMesh = SkeletalMesh;
		OnRep_WeaponMesh();
	}
}

void ANaviDropAndPickupable_Weapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// @TODO: 처음 Drop 할때는 Owner를 막기
	// @TODO: 던진 Actor에 대해서는 OWNER를 누구로 설정해야하는가? OUTER는?
	if (!HasAuthority() || !OtherActor || OtherActor == this)
	{
		return;
	}
	AController* OwnerController = GetOwner<AController>();
	if (OtherActor == OwnerController->GetPawn() && bIsOnGround == false)
	{
		return;
	}
	
	ULyraInventoryManagerComponent* InventoryManager = OtherActor->FindComponentByClass<ULyraInventoryManagerComponent>();
	UNaviQuickBarComponent* QuickBarComponent = OtherActor->FindComponentByClass<UNaviQuickBarComponent>();
	if (InventoryManager == nullptr || QuickBarComponent == nullptr) return;
	
	TArray<ULyraInventoryItemInstance*> ItemInstances = UPickupableStatics::AddPickupToInventory(InventoryManager, this);
	if (ItemInstances.Num() > 0 && ItemInstances[0] != nullptr)
	{
		int SlotIndex = QuickBarComponent->AddItemToSlot(ItemInstances[0]);
		QuickBarComponent->SetActiveSlotIndex(SlotIndex);
	}
	Destroy();
}

void ANaviDropAndPickupable_Weapon::OnRep_WeaponMesh()
{
	if (ReplicatedWeaponMesh && WeaponMeshComponent)
	{
		WeaponMeshComponent->SetSkeletalMeshAsset(ReplicatedWeaponMesh);
	}
}
