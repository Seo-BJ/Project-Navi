// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"

#include "LyraWeaponStateComponent.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;
struct FGameplayEffectContextHandle;
struct FHitResult;

// Hit markers are shown for ranged weapon impacts in the reticle
// A 'successful' hit marker is shown for impacts that damaged an enemy
struct FLyraScreenSpaceHitLocation
{
	/** Hit location in viewport screenspace */
	FVector2D Location;
	FGameplayTag HitZone;

	/** 이 마커를 만든 타깃 데이터 항목의 인덱스. 서버가 확인한 인덱스 목록과 맞출 때 사용한다. */
	int32 TargetDataIndex = INDEX_NONE;

	bool bShowAsSuccess = false;
};

struct FLyraServerSideHitMarkerBatch
{
	FLyraServerSideHitMarkerBatch() { }

	FLyraServerSideHitMarkerBatch(uint8 InUniqueId) :
		UniqueId(InUniqueId)
	{ }

	TArray<FLyraScreenSpaceHitLocation> Markers;

	uint8 UniqueId = 0;
};

/** 서버가 확인한 히트 1건. 히트 마커를 서버 판정으로 확정할 때 쓴다. */
USTRUCT()
struct FLyraConfirmedHit
{
	GENERATED_BODY()

	/** 클라가 보낸 타깃 데이터 핸들에서의 항목 인덱스 */
	UPROPERTY()
	uint8 TargetDataIndex = 0;

	/** 서버가 판정한 피격 부위 (Gameplay.Zone.*). 비어 있으면 바디샷 */
	UPROPERTY()
	FGameplayTag HitZone;
};

// Tracks weapon state and recent confirmed hit markers to display on screen
UCLASS()
class ULyraWeaponStateComponent : public UControllerComponent
{
	GENERATED_BODY()

public:

	ULyraWeaponStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 서버가 발사 1회의 히트 검증을 마친 뒤 호출한다.
	 * @param UniqueId 히트 마커 배치 ID (FGameplayAbilityTargetDataHandle::UniqueId)
	 * @param ConfirmedHits 서버가 확인한 히트(타깃 데이터 인덱스 + 서버 판정 부위). 비어 있으면 확정된 히트가 없다.
	 */
	UFUNCTION(Client, Reliable)
	void ClientConfirmTargetData(uint8 UniqueId, const TArray<FLyraConfirmedHit>& ConfirmedHits);

	void AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits);

	/** Updates this player's last damage instigated time */
	void UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext);

	/** Gets the array of most recent locations this player instigated damage, in screen-space */
	void GetLastWeaponDamageScreenLocations(TArray<FLyraScreenSpaceHitLocation>& WeaponDamageScreenLocations)
	{
		WeaponDamageScreenLocations = LastWeaponDamageScreenLocations;
	}

	/** Returns the elapsed time since the last (outgoing) damage hit notification occurred */
	double GetTimeSinceLastHitNotification() const;

	int32 GetUnconfirmedServerSideHitMarkerCount() const
	{
		return UnconfirmedServerSideHitMarkers.Num();
	}

protected:
	// This is called to filter hit results to determine whether they should be considered as a successful hit or not
	// The default behavior is to treat it as a success if being done to a team actor that belongs to a different team
	// to the owning controller's pawn
	virtual bool ShouldShowHitAsSuccess(const FHitResult& Hit) const;

	virtual bool ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const;

	void ActuallyUpdateDamageInstigatedTime();

private:
	/** Last time this controller instigated weapon damage */
	double LastWeaponDamageInstigatedTime = 0.0;

	/** Screen-space locations of our most recently instigated weapon damage (the confirmed hits) */
	TArray<FLyraScreenSpaceHitLocation> LastWeaponDamageScreenLocations;

	/** The unconfirmed hits */
	TArray<FLyraServerSideHitMarkerBatch> UnconfirmedServerSideHitMarkers;
};
