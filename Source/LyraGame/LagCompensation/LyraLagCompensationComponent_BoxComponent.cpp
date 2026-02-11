// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLagCompensationComponent_BoxComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

#include "Physics/LyraCollisionChannels.h"
#include "Character/LyraCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Player/LyraPlayerController.h"

DEFINE_LOG_CATEGORY(LogLagCompensation);


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
				const FFramePackage& CurrentFrame = FrameHistory.GetHead()->GetValue();
				DrawDebugFramePackage(CurrentFrame);
			}
		}
	}
#endif
}

void ULyraLagCompensationComponent_BoxComponent::UpdateFrameHistory()
{
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
}

void ULyraLagCompensationComponent_BoxComponent::SaveFramePackage(FFramePackage& Package)
{
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
	if (IsValid(HitActor))
	{
		FFramePackage FrameToCheck = GetHitTimeFrame(HitActor, HitTime);
		return ConfirmHit(FrameToCheck, HitActor, TraceStart, HitLocation);
	}
	return FServerSideRewindResult();
}

FServerSideRewindResult ULyraLagCompensationComponent_BoxComponent::ConfirmHit(const FFramePackage& FrameToCheck, AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
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
	AActor* Attacker = GetOwner();
	FCollisionQueryParams TraceParams;
	if (Attacker)
	{
		TraceParams.AddIgnoredActor(Attacker);
	}
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

bool ULyraLagCompensationComponent_BoxComponent::PerformHitCheck(const TArray<UBoxComponent*>& BoxesToCheck, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHit)
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

FFramePackage ULyraLagCompensationComponent_BoxComponent::GetHitTimeFrame(AActor* HitActor, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("LagCompensation::HistorySearch"));
	// 1. 유효성 검사
	if (!IsValid(HitActor)) return FFramePackage();
	ULyraLagCompensationComponent_BoxComponent* HitLagCompensation = HitActor->GetComponentByClass<ULyraLagCompensationComponent_BoxComponent>();
	
	if ((!IsValid(HitLagCompensation)) || (HitLagCompensation->FrameHistory.GetHead() == nullptr)|| (HitLagCompensation->FrameHistory.GetTail() == nullptr))
	{
		return FFramePackage();
	}


	// 피격된 캐릭터의 프레임 기록을 가져옴
	const float OldestHistoryTime = HitLagCompensation->FrameHistory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = HitLagCompensation->FrameHistory.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime) // Case 1: 요청된 시간이 기록 범위를 벗어남 (너무 과거의 요청)
	{
		return FFramePackage(); 
	}
	if (FMath::IsNearlyEqual(OldestHistoryTime, HitTime)) // Case2: 요청된 시간이 가장 오래된 기록과 같음
	{
		return HitLagCompensation->FrameHistory.GetTail()->GetValue(); 
	} 
	if (FMath::IsNearlyEqual(NewestHistoryTime, HitTime)) // Case 3: 요청 시간이 가장 최신 기록과 같음
	{
		return HitLagCompensation->FrameHistory.GetHead()->GetValue(); 
	}
	
	FFramePackage FrameToCheck;
	if (NewestHistoryTime < HitTime) // Case 4: HitTime이 근소하게 최신 기록보다 큼
	{
		FrameToCheck = ExtrapolateByTwoFrames(
			HitLagCompensation->FrameHistory.GetHead()->GetNextNode()->GetValue(),
			HitLagCompensation->FrameHistory.GetHead()->GetValue(), HitTime
		);
	}
	else // Case 5: HitTime이 기록 사이에 존재
	{
		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = HitLagCompensation->FrameHistory.GetHead();
		TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
		while (Older->GetValue().Time > HitTime)
		{
			if (Older->GetNextNode() == nullptr) break;
			Older = Older->GetNextNode();
			if (Older->GetValue().Time > HitTime)
			{
				Younger = Older;
			}
		}
		// 이 루프가 끝나면 Older->Time < HitTime < Younger->Time 상태가 됨
		if (Older->GetValue().Time == HitTime) // 정확히 일치하는 프레임 발견
		{
			FrameToCheck = Older->GetValue();
		}
		else
		{
			FrameToCheck = InterpolateBetweenTwoFrames(Older->GetValue(), Younger->GetValue(), HitTime);
		}
	}
	FrameToCheck.HitActor = HitActor;
	return FrameToCheck;
}

FFramePackage ULyraLagCompensationComponent_BoxComponent::InterpolateBetweenTwoFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
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

void ULyraLagCompensationComponent_BoxComponent::DrawDebugHitResult(FHitResult HitResult, bool bConfirmHit) const
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
