// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "LagCompensation/ILagCompensationTarget.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraCharacter.generated.h"

class AActor;
class AController;
class ALyraPlayerController;
class ALyraPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class ULyraAbilitySystemComponent;
class ULyraCameraComponent;
class ULyraHealthComponent;
class ULyraPawnExtensionComponent;
class UObject;
class USkeletalMeshComponent;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;
class UBoxComponent;
class ULyraLagCompensationComponent;

/**
 * FLyraReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FLyraReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/**
 * ALyraCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class LYRAGAME_API ALyraCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ILyraTeamAgentInterface, public ILagCompensationTarget 
{
	GENERATED_BODY()

public:

	ALyraCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ALyraPlayerController* GetLyraPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ALyraPlayerState* GetLyraPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	void ToggleCrouch();

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	//~APawn interface
	virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	//~ILyraTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	virtual bool UpdateSharedReplication();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const {return FirstPersonMesh.Get();}
	
	FORCEINLINE ULyraCameraComponent* GetLyraCameraComponent() const {return CameraComponent.Get();}

	//~ ILagCompensationTarget interface
	virtual const TMap<FName, TObjectPtr<UBoxComponent>>& GetHitCollisionBoxes() const override;
	//~ End ILagCompensationTarget interface
	
protected:

	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void InitializeGameplayTags();

	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);

	void DisableMovementAndCollision();
	void DestroyDueToDeath();
	void UninitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	void K2_OnDeathFinished();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FLyraReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraLagCompensationComponent> LagCompensation;

	UPROPERTY()
	TMap<FName, TObjectPtr<UBoxComponent>> HitCollisionBoxes;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> head;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Pelvis;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> spine_02;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> spine_03;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> UpperArm_L;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> lowerarm_l;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Hand_L;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> UpperArm_R;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> lowerarm_r;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Hand_R;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Thigh_L;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> calf_l;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Foot_L;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Thigh_R;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> calf_r;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Foot_R;
};
