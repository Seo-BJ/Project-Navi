#include "LyraConnectionManager.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "LyraReplicationGraphSettings.h"
#include "LyraReplicationGraph.h"
#include "DrawDebugHelpers.h"
#include "Character/LyraCharacter.h"
#include "Components/BoxComponent.h"
#include "Camera/LyraCameraComponent.h"
#include "LagCompensation/LyraTimeSyncComponent.h"

TArray<ULyraConnectionManager*>* FTeamConnectionListMap::GetConnectionArrayForTeam(int32 Team)
{
	return Find(Team);
}

TArray<ULyraConnectionManager*> FTeamConnectionListMap::GetVisibleConnectionArrayForNonTeam(const APawn* Pawn,
	int32 Team)
{
	SCOPE_CYCLE_COUNTER(STAT_LyraRepGraph_GatherNonTeamVisibility);
	TArray<ULyraConnectionManager*> NonTeamConnections;

	if (!IsValid(Pawn))
	{
		return NonTeamConnections;
	}

	const ULyraReplicationGraphSettings* Settings = GetDefault<ULyraReplicationGraphSettings>();
	ECollisionChannel TraceChannel = Settings ? Settings->VisibleConnectionTraceChannel.GetValue() : ECC_GameTraceChannel1;

	// 쿼리 파라미터 설정 및 모든 팀 멤버 무시
	TArray<ULyraConnectionManager*>* TeamMembers = GetConnectionArrayForTeam(Team);
		
	FCollisionQueryParams TraceParams;
	if (TeamMembers)
	{
		for (const ULyraConnectionManager* ConnectionManager : *TeamMembers)
		{
			TraceParams.AddIgnoredActor(ConnectionManager->Pawn.Get());
		}
	}
	else
	{
		TraceParams.AddIgnoredActor(Pawn);
	}

	// 입력된 팀과 일치하지 않는 모든 팀들에 대해 반복
	TArray<int32> Teams;
	GetKeys(Teams);

	const UWorld* World = Pawn->GetWorld();
	const FVector TraceStart = GetTraceStartLocation(Pawn);
	
	UE_LOG(LogLyraRepGraph, VeryVerbose, TEXT("GetVisibleConnectionArrayForNonTeam: Checking visibility for Pawn [%s] Team [%d] at [%s]"), *Pawn->GetName(), Team, *TraceStart.ToString());

	for (int32 i = 0; i < Teams.Num(); i++)
	{
		const int32 TeamID = Teams[i];
		if (TeamID != Team)
		{
			const TArray<ULyraConnectionManager*>* OtherTeamMembers = GetConnectionArrayForTeam(TeamID);

			if (OtherTeamMembers)
			{
				for (ULyraConnectionManager* ConnectionManager : *OtherTeamMembers)
				{
					if (!ConnectionManager->Pawn.IsValid())
					{
						continue;
					}
					
					APawn* OtherPawn = ConnectionManager->Pawn.Get();
					
					TArray<FVector> TraceTargets;
					GetTraceTargetLocations(Pawn, OtherPawn, TraceTargets);

					if (IsTargetVisible(World, TraceStart, TraceTargets, TraceChannel, TraceParams))
					{
						NonTeamConnections.Add(ConnectionManager);
						UE_LOG(LogLyraRepGraph, VeryVerbose, TEXT("  - Visible: Enemy [%s] Team [%d]"), *OtherPawn->GetName(), TeamID);
					}
					else
					{
						UE_LOG(LogLyraRepGraph, VeryVerbose, TEXT("  - Blocked: Enemy [%s] Team [%d]"), *OtherPawn->GetName(), TeamID);
					}
				}
			}
		}
	}

	return NonTeamConnections;
}

FVector FTeamConnectionListMap::GetTraceStartLocation(const APawn* Pawn) const
{
	if (const ALyraCharacter* LyraChar = Cast<ALyraCharacter>(Pawn))
	{
		if (ULyraCameraComponent* CameraComponent = LyraChar->GetLyraCameraComponent())
		{
			return CameraComponent->GetComponentLocation();
		}
	}

	// 폴백(Fallback)
	const FVector TraceOffset = FVector(0.0f, 0.0f, 180.0f);
	return Pawn->GetActorLocation() + TraceOffset;
}

void FTeamConnectionListMap::GetTraceTargetLocations(const APawn* SourcePawn, const APawn* TargetPawn, TArray<FVector>& OutTargets) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraRepGraph_GetTraceTargets);
	const ALyraCharacter* OtherLyraChar = Cast<ALyraCharacter>(TargetPawn);
	if (!OtherLyraChar)
	{
		// LyraCharacter가 아닌 경우를 위한 폴백
		const FVector TraceOffset = FVector(0.0f, 0.0f, 180.0f);
		OutTargets.Add(TargetPawn->GetActorLocation() + TraceOffset);
		return;
	}

	// 1. 설정 확인 및 핑(지연 시간) 정보를 기반으로 미래 위치 예측 오프셋 계산
	const ULyraReplicationGraphSettings* Settings = GetDefault<ULyraReplicationGraphSettings>();
	const bool bUsePredictiveVisibility = Settings ? Settings->bEnablePredictiveVisibility : true;
	
	FVector ActorLocation = OtherLyraChar->GetActorLocation();
	FVector Velocity = OtherLyraChar->GetVelocity();
	FVector PredictionOffset = FVector::ZeroVector;

	if (bUsePredictiveVisibility && Velocity.SizeSquared() > 1.0f)
	{
		const UWorld* World = SourcePawn->GetWorld();
		const float DeltaSeconds = World->GetDeltaSeconds();
		
		float LookAheadTime = DeltaSeconds; // 최소 한 프레임은 내다봄
		
		if (const AController* SourceController = SourcePawn->GetController())
		{
			if (const ULyraTimeSyncComponent* TimeSyncComp = SourceController->FindComponentByClass<ULyraTimeSyncComponent>())
			{
				const float SelfSingleTrip = TimeSyncComp->GetSingleTripTime();

				float EngineSingleTrip = 0.f;
				if (const APlayerState* PS = SourceController->PlayerState)
				{
					EngineSingleTrip = PS->GetPingInMilliseconds() / 2000.f;
				}
				UE_LOG(LogTemp, Log,
					TEXT("[TimeSyncCompare/RepGraph] SingleTrip self=%.4f engine=%.4f diff=%.4f"),
					SelfSingleTrip, EngineSingleTrip, SelfSingleTrip - EngineSingleTrip);

				// 공식: RTT (SingleTripTime * 2) + 서버 프레임 시간(DeltaSeconds) + 추가 여유분
				// RTT를 사용함으로써 클라이언트의 움직임과 서버의 응답 시간을 모두 커버하는 공격적인 예측을 수행
				LookAheadTime = (SelfSingleTrip * 2.0f) + DeltaSeconds + 0.02f;
			}
		}

		PredictionOffset = Velocity * LookAheadTime;
	}

	// --- 10개의 타겟 지점 수집 (바운딩 박스를 이동 방향으로 늘림) ---
	
	// 1. 중심점 (현재 위치 유지)
	OutTargets.Add(ActorLocation);
	
	// 2. 카메라 위치 (현재 위치 유지)
	if (OtherLyraChar->GetLyraCameraComponent())
	{
		OutTargets.Add(OtherLyraChar->GetLyraCameraComponent()->GetComponentLocation());
	}
	else
	{
		OutTargets.Add(ActorLocation + FVector(0.0f, 0.0f, 60.0f));
	}
	
	// 3. 바운딩 박스의 8개 모서리 (이동 방향의 전면 모서리만 예측 오프셋 적용)
	if (UBoxComponent* BoundingBox = OtherLyraChar->FindComponentByClass<UBoxComponent>())
	{
		FVector Center = BoundingBox->GetComponentLocation();
		FVector Extents = BoundingBox->GetScaledBoxExtent();
		FRotator Rotation = BoundingBox->GetComponentRotation();
		FVector VelocityDir = Velocity.GetSafeNormal();
		const UWorld* World = OtherLyraChar->GetWorld();
		
		for (float X : {-1.0f, 1.0f})
		{
			for (float Y : {-1.0f, 1.0f})
			{
				for (float Z : {-1.0f, 1.0f})
				{
					FVector LocalCornerOffset = FVector(X * Extents.X, Y * Extents.Y, Z * Extents.Z);
					FVector OriginalCornerLocation = Center + Rotation.RotateVector(LocalCornerOffset);
					FVector FinalCornerLocation = OriginalCornerLocation;
					
					// 꼭지점이 이동 방향의 전면에 있다면 예측 오프셋만큼 전진시켜 박스를 늘림
					FVector RelativeFromCenter = OriginalCornerLocation - Center;
					bool bIsLeadingEdge = PredictionOffset.SizeSquared() > 1.0f && (RelativeFromCenter | VelocityDir) > 0.0f;
					
					if (bIsLeadingEdge)
					{
						FinalCornerLocation += PredictionOffset;
						
						// 시각화: 오프셋이 적용된 꼭짓점들의 원래 위치 (노란색) 및 적용된 위치 (하늘색)
						DrawDebugPoint(World, OriginalCornerLocation, 10.0f, FColor::Yellow, false, 0.1f);
						DrawDebugPoint(World, FinalCornerLocation, 12.0f, FColor::Cyan, false, 0.1f);
						DrawDebugLine(World, OriginalCornerLocation, FinalCornerLocation, FColor::Cyan, false, 0.1f, 0, 0.5f);
					}
					else
					{
						// 시각화: 오프셋을 적용하지 않는 반대편 꼭짓점 (파란색)
						DrawDebugPoint(World, OriginalCornerLocation, 10.0f, FColor::Blue, false, 0.1f);
					}

					OutTargets.Add(FinalCornerLocation);
				}
			}
		}
	}
}



bool FTeamConnectionListMap::IsTargetVisible(const UWorld* World, const FVector& Start, const TArray<FVector>& Targets,
	ECollisionChannel TraceChannel, const FCollisionQueryParams& TraceParams) const
{
	SCOPE_CYCLE_COUNTER(STAT_LyraRepGraph_PhysicalTrace);
	bool bAnyVisible = false;
	
	for (const FVector& TraceEnd : Targets)
	{
		FHitResult OutHit;
		bool bHit = World->LineTraceSingleByChannel(OutHit, Start, TraceEnd, TraceChannel, TraceParams);
		
		if (!bHit)
		{
			// 성공: 녹색 선 (장애물 없음)
			DrawDebugLine(World, Start, TraceEnd, FColor::Green, false, 0.1f, 0, 1.0f);
			bAnyVisible = true;
		}
		else
		{
			// 차단됨: 히트 지점까지 빨간색 선
			DrawDebugLine(World, Start, OutHit.ImpactPoint, FColor::Red, false, 0.1f, 0, 1.0f);
			// 히트 지점부터 원래 목표까지는 흐릿한 빨간색 선 (선택 사항)
			DrawDebugLine(World, OutHit.ImpactPoint, TraceEnd, FColor::Red.WithAlpha(64), false, 0.1f, 0, 0.5f);
		}
	}

	return bAnyVisible;
}


void FTeamConnectionListMap::AddConnectionToTeam(int32 Team, ULyraConnectionManager* ConnManager)
{
	TArray<ULyraConnectionManager*>& TeamList = FindOrAdd(Team);
	TeamList.Add(ConnManager);
}

void FTeamConnectionListMap::RemoveConnectionFromTeam(int32 Team, ULyraConnectionManager* ConnManager)
{
	if (TArray<ULyraConnectionManager*>* TeamList = Find(Team))
	{
		TeamList->RemoveSwap(ConnManager);

		// 팀에 더 이상 연결이 없으면 맵에서 삭제
		if (TeamList->Num() == 0)
		{
			Remove(Team);
		}
	}
}
