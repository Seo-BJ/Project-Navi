// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLagCompensationComponent_BoxComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

#include "Physics/LyraCollisionChannels.h"
#include "Character/LyraCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Player/LyraPlayerController.h"

DEFINE_LOG_CATEGORY(LogLagCompensation);

// 프로파일링 그룹 (BoxComponent 전용)
DECLARE_STATS_GROUP(TEXT("Lyra Lag Compensation Box"), STATGROUP_LyraLagBox, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("LagCompBox - UpdateHistory"), STAT_LyraLagBox_UpdateHistory, STATGROUP_LyraLagBox);
DECLARE_CYCLE_STAT(TEXT("LagCompBox - HistorySearch"), STAT_LyraLagBox_HistorySearch, STATGROUP_LyraLagBox);
DECLARE_CYCLE_STAT(TEXT("LagCompBox - Interpolate"), STAT_LyraLagBox_Interpolate, STATGROUP_LyraLagBox);
DECLARE_CYCLE_STAT(TEXT("LagCompBox - CacheFrame"), STAT_LyraLagBox_CacheFrame, STATGROUP_LyraLagBox);
DECLARE_CYCLE_STAT(TEXT("LagCompBox - Rewind"), STAT_LyraLagBox_Rewind, STATGROUP_LyraLagBox);
DECLARE_CYCLE_STAT(TEXT("LagCompBox - ConfirmHit"), STAT_LyraLagBox_ConfirmHit, STATGROUP_LyraLagBox);

DECLARE_DWORD_COUNTER_STAT(TEXT("LagCompBox - History Frames"), STAT_LyraLagBox_HistoryCount, STATGROUP_LyraLagBox);
DECLARE_MEMORY_STAT(TEXT("LagCompBox - History Memory"), STAT_LyraLagBox_HistoryMemory, STATGROUP_LyraLagBox);


ULyraLagCompensationComponent_BoxComponent::ULyraLagCompensationComponent_BoxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraLagCompensationComponent_BoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

#if ENABLE_DRAW_DEBUG
	if (bDrawFrameHistory)
	{
		TickCounter++;
		if (TickCounter % DrawDebugFrequency == 0)
		{
			if (FrameHistory.Num() > 0)
			{
				DrawDebugFramePackage(FrameHistory.First());
			}
		}
	}
#endif
}

void ULyraLagCompensationComponent_BoxComponent::UpdateFrameHistory()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagBox_UpdateHistory);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_UpdateHistory);

	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);

	FrameHistory.EmplaceFirst(MoveTemp(ThisFrame));

	if (FrameHistory.Num() >= 2)
	{
		float HistoryLength = FrameHistory.First().Time - FrameHistory.Last().Time;
		while (HistoryLength > MaxRecordTime && FrameHistory.Num() >= 2)
		{
			FrameHistory.PopLast();
			HistoryLength = FrameHistory.First().Time - FrameHistory.Last().Time;
		}
	}

	const int32 HistoryCount = FrameHistory.Num();
	const SIZE_T HistoryMemory = FrameHistory.GetAllocatedSize();

	INC_DWORD_STAT_BY(STAT_LyraLagBox_HistoryCount, HistoryCount);
	SET_MEMORY_STAT(STAT_LyraLagBox_HistoryMemory, HistoryMemory);
}

void ULyraLagCompensationComponent_BoxComponent::SaveFramePackage(FFramePackage& Package)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_CacheFrame);

	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor))
	{
		ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(OwnerActor);
		if (!Target) return;

		Package.HitActor = OwnerActor;
		Package.Time = GetWorld()->GetTimeSeconds(); // 서버 시간

		for (auto& BoxPair : Target->GetHitCollisionBoxes())
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FServerSideRewindResult ULyraLagCompensationComponent_BoxComponent::ServerSideRewind(AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LyraLagBox_ServerSideRewind);
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_Rewind);

	if (IsValid(HitActor))
	{
		FFramePackage FrameToCheck = GetHitTimeFrame(HitTime);
		FrameToCheck.HitActor = HitActor;
		return ConfirmHit(FrameToCheck, HitActor, TraceStart, HitLocation);
	}
	return FServerSideRewindResult();
}

FServerSideRewindResult ULyraLagCompensationComponent_BoxComponent::ConfirmHit(const FFramePackage& FrameToCheck, AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_ConfirmHit);

	// 1. 유효성 검사
	if (!IsValid(HitActor)) return FServerSideRewindResult();
	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	UWorld* World = GetWorld();
	if (!Target || !World) return FServerSideRewindResult();

	// 2. 현재 상태 저장
	FFramePackage CurrentFrame;
	CacheCurrentFrame(HitActor, CurrentFrame);

	// 3. 엑터 상태 되감기 (Rewind)
	RewindFrame(HitActor, FrameToCheck);
	SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::NoCollision);

	// 4. Trace 설정
	// 주의: GetOwner()는 피격자(HitActor)임. 발사자 정보는 이 시점에 보유하지 않음.
	// HitBox 채널만으로 trace하므로 자기 자신의 캡슐/메시는 SetMeshCollisionEnabledType로 이미 비활성화됨.
	FCollisionQueryParams TraceParams;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

#if ENABLE_DRAW_DEBUG
	// [DEBUG] 서버 트레이스 경로 시각화 (Trace 전)
	if (bDrawHitResult)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, DrawDebugHitBoxTime, 0, 1.0f);
		DrawDebugPoint(World, HitLocation, 10.0f, FColor::Cyan, false, DrawDebugHitBoxTime);
	}
#endif

	FHitResult ConfirmHitResult;
	bool bHitSuccess = false;
	bool bHeadShot = false;

	// 5. 헤드샷 우선 판정
	const TMap<FName, TObjectPtr<UBoxComponent>>& HitBoxes = Target->GetHitCollisionBoxes();
	const TObjectPtr<UBoxComponent>* FoundBoxPtr = HitBoxes.Find(FName("head"));

	if (FoundBoxPtr && *FoundBoxPtr)
	{
		TArray<UBoxComponent*> HeadBoxes;
		HeadBoxes.Add(*FoundBoxPtr);

		if (PerformHitCheck(HeadBoxes, TraceStart, TraceEnd, TraceParams, ConfirmHitResult))
		{
			bHitSuccess = true;
			bHeadShot = true;
		}
	}

	// 6. 바디샷 판정 (헤드샷 실패 시)
	if (!bHitSuccess)
	{
		TArray<UBoxComponent*> AllBoxes;
		for (auto& Pair : HitBoxes)
		{
			if (Pair.Value) AllBoxes.Add(Pair.Value);
		}

		if (PerformHitCheck(AllBoxes, TraceStart, TraceEnd, TraceParams, ConfirmHitResult))
		{
			bHitSuccess = true;
			bHeadShot = false;
		}
	}

	// 7. 결과 시각화 (상태 복원 전 수행하여 되감긴 위치에 그리기)
#if ENABLE_DRAW_DEBUG
	if (bDrawHitResult)
	{
		VisualizeConfirmHit(TraceStart, TraceEnd, bHitSuccess, ConfirmHitResult, HitActor);
	}
#endif

	// 8. 상태 복원
	ResetHitBoxes(HitActor, CurrentFrame);
	SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::QueryAndPhysics);

	return FServerSideRewindResult{ bHitSuccess, bHeadShot };
}

bool ULyraLagCompensationComponent_BoxComponent::PerformHitCheck(const TArray<UBoxComponent*>& BoxesToCheck, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHit) const
{
	for (UBoxComponent* Box : BoxesToCheck)
	{
		if (Box)
		{
			Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Box->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Block);
		}
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, Lyra_TraceChannel_LagCompensation_HitBox, Params);

	// Trace 후 콜리전 다시 끄기 (ResetHitBoxes에서 어차피 복구되지만, 안전을 위해)
	for (UBoxComponent* Box : BoxesToCheck)
	{
		if (Box)
		{
			Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Box->SetCollisionResponseToChannel(Lyra_TraceChannel_LagCompensation_HitBox, ECollisionResponse::ECR_Ignore);
		}
	}

	return bHit;
}

#if ENABLE_DRAW_DEBUG
void ULyraLagCompensationComponent_BoxComponent::VisualizeConfirmHit(const FVector& Start, const FVector& End, bool bSuccess, const FHitResult& HitResult, AActor* HitActor)
{
	if (bSuccess)
	{
		DrawDebugHitResult(HitResult, true);
		UE_LOG(LogLagCompensation, Log, TEXT("ConfirmHit SUCCESS. Actor: %s, Bone: %s"), *HitActor->GetName(), *HitResult.BoneName.ToString());
	}
	else
	{
		UE_LOG(LogLagCompensation, Warning, TEXT("ConfirmHit FAILED. TraceStart: %s, Actor: %s"), *Start.ToString(), *HitActor->GetName());

		ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
		if (Target)
		{
			for (auto& HitBoxPair : Target->GetHitCollisionBoxes())
			{
				if (HitBoxPair.Value)
				{
					DrawDebugBox(GetWorld(), HitBoxPair.Value->GetComponentLocation(), HitBoxPair.Value->GetScaledBoxExtent(), HitBoxPair.Value->GetComponentQuat(), FColor::Green, false, DrawDebugHitBoxTime);
				}
			}
		}
	}
}
#endif

FFramePackage ULyraLagCompensationComponent_BoxComponent::GetHitTimeFrame(float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("LagCompensation::HistorySearch"));
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_HistorySearch);

	// 이 컴포넌트는 피격자(owner)에 부착되어 있으므로 자신의 FrameHistory에서 검색.

	FFramePackage FrameToCheck;

	const TDeque<FFramePackage>& History = this->FrameHistory;
	if (History.Num() == 0)
	{
		return FFramePackage();
	}

	const float OldestHistoryTime = History.Last().Time;
	const float NewestHistoryTime = History.First().Time;

	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}
	if (FMath::IsNearlyEqual(OldestHistoryTime, HitTime))
	{
		return History.Last();
	}
	if (FMath::IsNearlyEqual(NewestHistoryTime, HitTime))
	{
		return History.First();
	}

	if (NewestHistoryTime < HitTime) // Case 4: 외삽
	{
		if (History.Num() >= 2)
		{
			FrameToCheck = ExtrapolateByTwoFrames(History[1], History[0], HitTime);
		}
		else
		{
			FrameToCheck = History.First();
		}
	}
	else // Case 5: 보간
	{
		// 최신(인덱스 0=First) -> 과거(인덱스 증가=Last) 방향으로 선형 탐색
		const int32 Count = History.Num();
		int32 YoungerIdx = 0;
		int32 OlderIdx = 0;
		while (OlderIdx + 1 < Count && History[OlderIdx].Time > HitTime)
		{
			++OlderIdx;
			if (History[OlderIdx].Time > HitTime)
			{
				YoungerIdx = OlderIdx;
			}
		}

		if (History[OlderIdx].Time == HitTime)
		{
			FrameToCheck = History[OlderIdx];
		}
		else
		{
			FrameToCheck = InterpolateBetweenTwoFrames(History[OlderIdx], History[YoungerIdx], HitTime);
		}
	}

	return FrameToCheck;
}

FFramePackage ULyraLagCompensationComponent_BoxComponent::InterpolateBetweenTwoFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_Interpolate);

	const float DeltaTime = YoungerFrame.Time - OlderFrame.Time;
	const float InterpolateFraction = FMath::Clamp((HitTime - OlderFrame.Time) / DeltaTime, 0, 1);
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation* OlderBoxPtr = OlderFrame.HitBoxInfo.Find(BoxInfoName);
		if (!OlderBoxPtr)
		{
			continue;
		}
	    const FBoxInformation& OlderBox = *OlderBoxPtr;
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::Lerp(OlderBox.Location, YoungerBox.Location, InterpolateFraction);

		FQuat OlderQuat(OlderBox.Rotation);
		FQuat YoungerQuat(YoungerBox.Rotation);
		FQuat InterpQuat = FQuat::Slerp(OlderQuat, YoungerQuat, InterpolateFraction);
		InterpBoxInfo.Rotation = InterpQuat.Rotator();

		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FFramePackage ULyraLagCompensationComponent_BoxComponent::ExtrapolateByTwoFrames(const FFramePackage& SecondNewestFrame,
	const FFramePackage& FirstNewestFrame, float HitTime)
{
	FFramePackage ExtrapolateFramePackage;
	ExtrapolateFramePackage.Time = HitTime;

	const float ExtrapolationTime = HitTime - SecondNewestFrame.Time;
	const float DeltaTime = FirstNewestFrame.Time - SecondNewestFrame.Time;

	if (FMath::IsNearlyZero(DeltaTime))
	{
		return FirstNewestFrame;
	}

	for (auto& FirstPair : FirstNewestFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = FirstPair.Key;
		const FBoxInformation* SecondBoxPtr = SecondNewestFrame.HitBoxInfo.Find(BoxInfoName);
		if (!SecondBoxPtr)
		{
			continue;
		}
		const FBoxInformation& SecondHitBox = *SecondBoxPtr;
		const FBoxInformation& FirstHitBox = FirstPair.Value;

		const FVector DeltaLocation = FirstHitBox.Location - SecondHitBox.Location;
		const FVector ExtrapolatedLocation = SecondHitBox.Location + (ExtrapolationTime/DeltaTime) * DeltaLocation;
		const FRotator DeltaRotation = (FirstHitBox.Rotation - SecondHitBox.Rotation).GetNormalized();
		const FRotator ExtrapolatedRotation = SecondHitBox.Rotation + (ExtrapolationTime/DeltaTime) * DeltaRotation;

		FBoxInformation ExtrapolatedBoxInfo;
		ExtrapolatedBoxInfo.Location = ExtrapolatedLocation;
		ExtrapolatedBoxInfo.Rotation = ExtrapolatedRotation;
		ExtrapolatedBoxInfo.BoxExtent = FirstHitBox.BoxExtent;

		ExtrapolateFramePackage.HitBoxInfo.Add(BoxInfoName, ExtrapolatedBoxInfo);
	}

	return ExtrapolateFramePackage;
}

void ULyraLagCompensationComponent_BoxComponent::CacheCurrentFrame(AActor* HitActor, FFramePackage& OutFramePackage)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraLagBox_CacheFrame);

	if (HitActor == nullptr) return;
	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	if (!Target) return;

	for (auto& HitBoxPair : Target->GetHitCollisionBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULyraLagCompensationComponent_BoxComponent::RewindFrame(AActor* HitActor, const FFramePackage& Package)
{
	if (HitActor == nullptr) return;
	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	if (!Target) return;

	for (auto& HitBoxPair : Target->GetHitCollisionBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
			}
		}
	}
}

void ULyraLagCompensationComponent_BoxComponent::ResetHitBoxes(AActor* HitActor, const FFramePackage& Package)
{
	if (HitActor == nullptr) return;
	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	if (!Target) return;

	for (auto& HitBoxPair : Target->GetHitCollisionBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULyraLagCompensationComponent_BoxComponent::SetMeshCollisionEnabledType(AActor* HitActor, ECollisionEnabled::Type CollsionEnabled)
{
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollsionEnabled);
		HitCharacter->GetCapsuleComponent()->SetCollisionEnabled(CollsionEnabled);
	}
	else
	{
		// @Todo: Actor가 여러 콜리전이 복합적으로 있을대 처리 필요.
	}
}

#if ENABLE_DRAW_DEBUG
void ULyraLagCompensationComponent_BoxComponent::DrawDebugFramePackage(const FFramePackage& FramePackage) const
{
	for (auto& BoxInfo : FramePackage.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), FColor::Green, false, DrawDebugHitBoxTime);
	}
}

void ULyraLagCompensationComponent_BoxComponent::DrawDebugHitResult(const FHitResult& HitResult, bool bConfirmHit) const
{
	if (HitResult.Component.IsValid())
	{
		if (UBoxComponent* Box = Cast<UBoxComponent>(HitResult.Component))
		{
			if (bConfirmHit)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, DrawDebugHitBoxTime);
			}
			else
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, DrawDebugHitBoxTime);
			}
		}
	}
}
#endif
