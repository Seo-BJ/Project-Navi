// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "Engine/HitResult.h"
#include "GameplayTagContainer.h"

#include "LyraGameplayAbility_RangedWeapon.generated.h"

enum ECollisionChannel : int;

class APawn;
class ULyraRangedWeaponInstance;
class ULyraServerSideRewindSubsystem;
class UObject;
struct FCollisionQueryParams;
struct FFrame;
struct FHitResult;
struct FLyraGameplayAbilityTargetData_SingleTargetHit;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;
struct FGameplayTag;
struct FGameplayTagContainer;

/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class ELyraAbilityTargetingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the pawn's center, oriented towards camera focus
	PawnTowardsFocus,
	// From the weapon's muzzle or location, in the pawn's orientation
	WeaponForward,
	// From the weapon's muzzle or location, towards camera focus
	WeaponTowardsFocus,
	// Custom blueprint-specified source location
	Custom
};



/**
 * ULyraGameplayAbility_RangedWeapon
 *
 * An ability granted by and associated with a ranged weapon instance
 */
UCLASS()
class ULyraGameplayAbility_RangedWeapon : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	ULyraGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Lyra|Ability")
	ULyraRangedWeaponInstance* GetWeaponInstance() const;

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	struct FRangedWeaponFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// The weapon instance / source of weapon data
		ULyraRangedWeaponInstance* WeaponData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lyra|Ability")
	int32 BulletsPerCartridge = 1;

	/**
	 * 펠릿 하나가 만들 수 있는 히트 수 상한.
	 * Multi 트레이스는 관통하지 않고 첫 Block에서 멈추지만, 그 앞의 Overlap 히트도 함께 돌려준다.
	 * 무기 채널을 Overlap으로 받는 대상을 맞혀야 하는 무기만 1보다 크게 둔다.
	 * 서버는 클라가 보낸 타깃 데이터를 BulletsPerCartridge * MaxHitsPerBullet 개까지만 검증한다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lyra|Ability", meta=(ClampMin=1))
	int32 MaxHitsPerBullet = 1;

	/**
	 * 서버가 확인한 히트마다 실행할 피격 GameplayCue (예: GameplayCue.Weapon.Rifle.Impact).
	 * 클라는 이 Cue를 예측 재생하지 않는다. 서버가 판정 결과(서버 HitResult)로 예측 키 없이 실행하므로
	 * 쏜 사람을 포함한 모든 클라가 서버 판정 위치·부위로 재생한다. 비어 있으면 실행하지 않는다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lyra|Ability", meta=(Categories="GameplayCue"))
	FGameplayTag ImpactCueTag;

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated, bool bTraceForServerSideRewind = false ) const;
	

	FVector GetWeaponTargetingSourceLocation() const;
	FTransform GetTargetingTransform(APawn* SourcePawn, ELyraAbilityTargetingSource Source) const;

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	/** 서버의 히트 1건 검증 결과. 확인된 경우 ServerHit은 서버 판정으로 만든 HitResult다(명중점, 법선, 본, 물리 재질). */
	struct FServerHitValidationResult
	{
		bool bConfirmed = false;
		FHitResult ServerHit;
	};

	/** 서버가 아는 쏜 사람의 시점 위치. 확정 HitResult의 TraceStart(= 데미지 거리 계산의 Origin)로 쓴다. */
	FVector GetServerShotOrigin() const;

	/** 확정된 히트마다 ImpactCueTag를 예측 키 없이 실행한다. (서버 전용) */
	void ExecuteServerImpactCues(const FGameplayAbilityTargetDataHandle& ConfirmedTargetDataHandle);

	/**
	 * 클라가 보낸 히트 하나를 서버에서 검증한다.
	 * 대상에 활성화된 스냅샷 컴포넌트가 있고 RewindEvaluator가 주어지면 SSR로, 그 밖(벽, 상호작용 액터, SSR 꺼짐)은 현재 시각 트레이스로 판정한다.
	 * SSR은 판정 중 대상의 콜리전/애님 상태를 잠시 바꿨다 되돌리므로 const가 아니다.
	 */
	FServerHitValidationResult ValidateClientHitOnServer(const FLyraGameplayAbilityTargetData_SingleTargetHit& ClientHit, ULyraServerSideRewindSubsystem* RewindEvaluator);

	/** 현재 월드 상태에서 클라 히트의 시작점 → 명중점 방향으로 다시 트레이스해 같은 액터가 맞는지 확인한다. */
	FServerHitValidationResult ValidateHitAtCurrentTime(const FHitResult& ClientHitResult) const;

	/**
	 * StartRangedWeaponTargeting는 원거리 무기 조준 절차를 시작하며,
	 * 다양한 컴포넌트가 존재하고 유효한지 확인합니다. 그 후 로컬 조준(local targeting)을 수행하고,
	 * 그 결과로 나온 히트(hit) 정보를 바탕으로 타겟 데이터를 생성합니다.
	 * 마지막으로 히트 마커 정보를 서버로 전송하고 타겟 데이터를 처리합니다.
	 */
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();
	
	/**
	 * PerformLocalTargeting() 함수는
	 * RangedWeaponFiringInput 데이터(조준 방향, 트레이스 벡터 등)를 결정하고,
	 * 해당 데이터를 TraceBulletsInCartridge() 함수로 전달합니다. 결과는 OutHits 배열에 저장됩니다.
	 * 
	 * @param OutHits Hit 결과
	 */
	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	// Traces all of the bullets in a single cartridge
	void TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;
	
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;
	
	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

};
