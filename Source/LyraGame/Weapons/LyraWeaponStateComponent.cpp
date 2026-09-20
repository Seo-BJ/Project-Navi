// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWeaponStateComponent.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Weapons/LyraRangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraWeaponStateComponent)


ULyraWeaponStateComponent::ULyraWeaponStateComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraWeaponStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (ULyraEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<ULyraEquipmentManagerComponent>())
		{
			if (ULyraRangedWeaponInstance* CurrentWeapon = Cast<ULyraRangedWeaponInstance>(EquipmentManager->GetFirstInstanceOfType(ULyraRangedWeaponInstance::StaticClass())))
			{
				CurrentWeapon->Tick(DeltaTime);
			}
		}
	}
}

bool ULyraWeaponStateComponent::ShouldShowHitAsSuccess(const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();

	//@TODO: Don't treat a hit that dealt no damage (due to invulnerability or similar) as a success
	UWorld* World = GetWorld();
	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{
		return TeamSubsystem->CanCauseDamage(GetController<APlayerController>(), Hit.GetActor());
	}

	return false;
}

bool ULyraWeaponStateComponent::ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const
{
	//@TODO: Implement me, for the purposes of this component we really only care about damage caused by a weapon
	// or projectile fired from a weapon, and should filter to that
	// (or perhaps see if the causer is also the source of our active reticle config)
	return EffectContext.GetEffectCauser() != nullptr;
}

void ULyraWeaponStateComponent::ClientConfirmTargetData_Implementation(uint8 UniqueId, const TArray<FLyraConfirmedHit>& ConfirmedHits)
{
	for (int i = 0; i < UnconfirmedServerSideHitMarkers.Num(); i++)
	{
		FLyraServerSideHitMarkerBatch& Batch = UnconfirmedServerSideHitMarkers[i];
		if (Batch.UniqueId == UniqueId)
		{
			bool bFoundShowAsSuccessHit = false;

			// 서버가 확인한 타깃 데이터 항목에서 나온 마커만 확정한다.
			// 마커는 화면 투영에 성공한 히트만 만들어지므로, 배열 위치가 아니라 TargetDataIndex로 대조한다.
			for (const FLyraScreenSpaceHitLocation& Entry : Batch.Markers)
			{
				const FLyraConfirmedHit* ConfirmedHit = (Entry.TargetDataIndex >= 0)
					? ConfirmedHits.FindByPredicate([&Entry](const FLyraConfirmedHit& Hit) { return Hit.TargetDataIndex == Entry.TargetDataIndex; })
					: nullptr;

				if (ConfirmedHit && Entry.bShowAsSuccess)
				{
					// Only need to do this once
					if (!bFoundShowAsSuccessHit)
					{
						ActuallyUpdateDamageInstigatedTime();
					}

					bFoundShowAsSuccessHit = true;

					// 마커 모양(헤드샷 등)은 클라 트레이스가 아니라 서버가 판정한 부위로 정한다.
					FLyraScreenSpaceHitLocation& ConfirmedEntry = LastWeaponDamageScreenLocations.Add_GetRef(Entry);
					ConfirmedEntry.HitZone = ConfirmedHit->HitZone;
				}
			}

			UnconfirmedServerSideHitMarkers.RemoveAt(i);
			break;
		}
	}
}

void ULyraWeaponStateComponent::AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits)
{
	FLyraServerSideHitMarkerBatch& NewUnconfirmedHitMarker = UnconfirmedServerSideHitMarkers.Emplace_GetRef(InTargetData.UniqueId);

	if (APlayerController* OwnerPC = GetController<APlayerController>())
	{
		// StartRangedWeaponTargeting은 FoundHits를 같은 순서로 타깃 데이터 항목에 넣으므로, HitIndex가 곧 타깃 데이터 인덱스다.
		for (int32 HitIndex = 0; HitIndex < FoundHits.Num(); ++HitIndex)
		{
			const FHitResult& Hit = FoundHits[HitIndex];
			FVector2D HitScreenLocation;
			if (UGameplayStatics::ProjectWorldToScreen(OwnerPC, Hit.Location, /*out*/ HitScreenLocation, /*bPlayerViewportRelative=*/ false))
			{
				FLyraScreenSpaceHitLocation& Entry = NewUnconfirmedHitMarker.Markers.AddDefaulted_GetRef();
				Entry.Location = HitScreenLocation;
				Entry.TargetDataIndex = HitIndex;
				Entry.bShowAsSuccess = ShouldShowHitAsSuccess(Hit);
				// 부위(HitZone)는 여기서 정하지 않는다. 서버 확정(ClientConfirmTargetData) 때 서버 판정 값으로 채운다.
			}
		}
	}
}

void ULyraWeaponStateComponent::UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext)
{
	if (ShouldUpdateDamageInstigatedTime(EffectContext))
	{
		ActuallyUpdateDamageInstigatedTime();
	}
}

void ULyraWeaponStateComponent::ActuallyUpdateDamageInstigatedTime()
{
	// If our LastWeaponDamageInstigatedTime was not very recent, clear our LastWeaponDamageScreenLocations array
	UWorld* World = GetWorld();
	if (World->GetTimeSeconds() - LastWeaponDamageInstigatedTime > 0.1)
	{
		LastWeaponDamageScreenLocations.Reset();
	}
	LastWeaponDamageInstigatedTime = World->GetTimeSeconds();
}

double ULyraWeaponStateComponent::GetTimeSinceLastHitNotification() const
{
	UWorld* World = GetWorld();
	return World->TimeSince(LastWeaponDamageInstigatedTime);
}

