// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLagCompensationComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

#include "Physics/LyraCollisionChannels.h"
#include "Character/LyraCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Player/LyraPlayerController.h"

DEFINE_LOG_CATEGORY(LogLagCompensation);


ULyraLagCompensationComponent::ULyraLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULyraLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;
	UpdateFrameHistory();
	
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
}

void ULyraLagCompensationComponent::UpdateFrameHistory()
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

void ULyraLagCompensationComponent::SaveFramePackage(FFramePackage& Package)
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

FServerSideRewindResult ULyraLagCompensationComponent::ServerSideRewind(AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	if (IsValid(HitActor))
	{
		FFramePackage FrameToCheck = GetHitTimeFrame(HitActor, HitTime);
		return ConfirmHit(FrameToCheck, HitActor, TraceStart, HitLocation);
	}
	return FServerSideRewindResult();
}

FServerSideRewindResult ULyraLagCompensationComponent::ConfirmHit(const FFramePackage& FrameToCheck, AActor* HitActor, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	// 1. 유효성 검사
	if (!IsValid(HitActor)) return FServerSideRewindResult(); 
	ILagCompensationTarget* Target = Cast<ILagCompensationTarget>(HitActor);
	UWorld* World = GetWorld();
	if (!Target || !World) return FServerSideRewindResult();
	
	// 2. 현재 상태 저장
	FFramePackage CurrentFrame; // 현재 프레임 정보를 담을 변수
	CacheCurrentFrame(HitActor, CurrentFrame); // 되감기 전, 캐릭터의 '현재' 히트박스 위치들을 백업

	// 3. 엑터 상태 되감기 (Rewind)
	RewindFrame(HitActor, FrameToCheck); // 엑터 히트박스들을 FramePackage에 담긴 '과거'의 위치로 강제 이동
	SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::NoCollision); // 메시 콜리전은 잠시 비활성화

	// 4. 공격자 엑터 무시
	AActor* Attacker = GetOwner();
	FCollisionQueryParams TraceParams;
	if (Attacker)
	{
		TraceParams.AddIgnoredActor(Attacker);
	}
	
	// 5. 헤드샷 우선 판정
	UBoxComponent* HeadBox = nullptr; 
	const TMap<FName, TObjectPtr<UBoxComponent>>& HitBoxes = Target->GetHitCollisionBoxes();
	const TObjectPtr<UBoxComponent>* FoundBoxPtr = HitBoxes.Find(FName("head"));
	if (FoundBoxPtr)
	{
		HeadBox = *FoundBoxPtr;
	}
	
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 클라이언트가 쏜 위치보다 약간 더 길게 트레이스 설정

	if (IsValid(HeadBox))
	{	
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
		HeadBox->SetCollisionResponseToChannel(LagCompensation_TraceChannel_HitBox, ECollisionResponse::ECR_Block); 
	
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, LagCompensation_TraceChannel_HitBox, TraceParams);
		if (ConfirmHitResult.bBlockingHit) 
		{
			// 5-1. 헤드샷 성공 및 상태 복원
			ResetHitBoxes(HitActor, CurrentFrame); 
			SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::QueryAndPhysics); 
			if (bDrawHitResult)
			{
				DrawDebugConfirmHitResult(ConfirmHitResult);
			}
			return FServerSideRewindResult{ true, true }; // {명중 확정, 헤드샷} 결과를 반환
		}
	}
	// 6. 바디샷 판정
	for (auto& HitBoxPair : Target->GetHitCollisionBoxes())
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			HitBoxPair.Value->SetCollisionResponseToChannel(LagCompensation_TraceChannel_HitBox, ECollisionResponse::ECR_Block);
		}
	}
	World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, LagCompensation_TraceChannel_HitBox, TraceParams);
	if (ConfirmHitResult.bBlockingHit) 
	{
		// 6-1. 바디샷 성공 및 상태 복원
		ResetHitBoxes(HitActor, CurrentFrame); 
		SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::QueryAndPhysics);
		if (bDrawHitResult)
		{
			DrawDebugConfirmHitResult(ConfirmHitResult);
		}
		return FServerSideRewindResult{ true, false }; // {명중 확정, 헤드샷 아님} 결과를 반환
	}
	// 7. Hit 실패 및 상태 복원
	ResetHitBoxes(HitActor, CurrentFrame); // HitActor의 히트박스를 '현재' 위치로 복원
	SetMeshCollisionEnabledType(HitActor, ECollisionEnabled::QueryAndPhysics); // 콜리전 재활성화
	return FServerSideRewindResult{ false , false }; // {명중 실패, 헤드샷 아님} 결과를 반환
}

FFramePackage ULyraLagCompensationComponent::GetHitTimeFrame(AActor* HitActor, float HitTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("LagCompensation::HistorySearch"));
	// 1. 유효성 검사
	if (!IsValid(HitActor)) return FFramePackage();
	ULyraLagCompensationComponent* HitLagCompensation = HitActor->GetComponentByClass<ULyraLagCompensationComponent>();
	
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

FFramePackage ULyraLagCompensationComponent::InterpolateBetweenTwoFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
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
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpolateFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpolateFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FFramePackage ULyraLagCompensationComponent::ExtrapolateByTwoFrames(const FFramePackage& SecondNewestFrame,
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

void ULyraLagCompensationComponent::CacheCurrentFrame(AActor* HitActor, FFramePackage& OutFramePackage)
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

void ULyraLagCompensationComponent::RewindFrame(AActor* HitActor, const FFramePackage& Package)
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

void ULyraLagCompensationComponent::ResetHitBoxes(AActor* HitActor, const FFramePackage& Package)
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

void ULyraLagCompensationComponent::SetMeshCollisionEnabledType(AActor* HitActor, ECollisionEnabled::Type CollsionEnabled)
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

void ULyraLagCompensationComponent::DrawDebugFramePackage(const FFramePackage& FramePackage)
{
	for (auto& BoxInfo : FramePackage.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), FColor::Red, false, DrawDebugHitBoxTime);
	}
}

void ULyraLagCompensationComponent::DrawDebugConfirmHitResult(FHitResult ConfirmHitResult)
{
	if (ConfirmHitResult.Component.IsValid())
	{
		UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
		if (Box)
		{
			DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, DrawDebugHitBoxTime);
		}
	}
}
