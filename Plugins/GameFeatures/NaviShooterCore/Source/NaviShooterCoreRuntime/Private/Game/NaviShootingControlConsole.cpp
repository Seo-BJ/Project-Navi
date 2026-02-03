// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NaviShootingControlConsole.h"
#include "Net/UnrealNetwork.h"
#include "Game/NaviShootingTestComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/ChildActorComponent.h"
#include "Game/NaviShootingControlConsoleButton.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ANaviShootingControlConsole::ANaviShootingControlConsole()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	StartButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartButton"));
	StartButton->SetupAttachment(Root);
	
	SpeedButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpeedButton"));
	SpeedButton->SetupAttachment(Root);
	
	StreakButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StreakButton"));
	StreakButton->SetupAttachment(Root);
	
	BotMoveButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BotMoveButton"));
	BotMoveButton->SetupAttachment(Root);
	
	ArmorButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmorButton"));
	ArmorButton->SetupAttachment(Root);
	
	AmmoButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoButton"));
	AmmoButton->SetupAttachment(Root);

	StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
	StatusText->SetupAttachment(Root);
	StatusText->SetRelativeLocation(FVector(0, 0, 200));
	StatusText->SetText(FText::FromString(TEXT("Shooting Range")));
	StatusText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
}

void ANaviShootingControlConsole::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>())
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void ANaviShootingControlConsole::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
	FindTestComponent();
	UpdateDisplay();
}

void ANaviShootingControlConsole::OnWeaponHit(const FHitResult& HitResult, AActor* Shooter)
{
	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	if (!IsValid(HitComp)) return;
	
	if (!TestComponent)
	{
		FindTestComponent();
		if (!TestComponent) return;
	}

	bool bSettingsChanged = false;
	

	if (HitComp == StartButton)
	{
		// 시작!
		TestComponent->StartTest(Shooter, CurrentSettings);
		// 시각적 피드백 (버튼 눌림 애니메이션 등은 BP에서 구현하거나 여기서 Material 변경)
		return;
	}
	else if (HitComp == SpeedButton)
	{
		// Enum 순환: Easy -> Med -> Hard -> Easy
		uint8 NextVal = (uint8)CurrentSettings.Speed + 1;
		if (NextVal > (uint8)ENaviTestSpeed::Hard) NextVal = 0;
		CurrentSettings.Speed = (ENaviTestSpeed)NextVal;
		bSettingsChanged = true;
	}
	else if (HitComp == StreakButton)
	{
		uint8 NextVal = (uint8)CurrentSettings.Streak + 1;
		if (NextVal > (uint8)ENaviTestStreak::Endless) NextVal = 0;
		CurrentSettings.Streak = (ENaviTestStreak)NextVal;
		bSettingsChanged = true;
	}
	else if (HitComp == BotMoveButton)
	{
		uint8 NextVal = (uint8)CurrentSettings.BotMovement + 1;
		if (NextVal > (uint8)ENaviBotMovement::Strafe) NextVal = 0; // Strafe가 끝
		CurrentSettings.BotMovement = (ENaviBotMovement)NextVal;
		bSettingsChanged = true;
	}
	else if (HitComp == ArmorButton)
	{
		CurrentSettings.bBotArmor = !CurrentSettings.bBotArmor;
		bSettingsChanged = true;
	}
	else if (HitComp == AmmoButton)
	{
		CurrentSettings.bInfiniteAmmo = !CurrentSettings.bInfiniteAmmo;
		bSettingsChanged = true;
	}

	if (bSettingsChanged)
	{
		// 테스트 컴포넌트에 알림 (이미 실행 중인 경우 설정 변경은 막힐 수 있음)
		TestComponent->UpdateSettings(CurrentSettings);
		UpdateDisplay();
	}
}


void ANaviShootingControlConsole::FindTestComponent()
{
	if (AGameStateBase* GS = UGameplayStatics::GetGameState(this))
	{
		TestComponent = GS->FindComponentByClass<UNaviShootingTestComponent>();
		
		if (TestComponent && LevelSpawnPoint)
		{
			TestComponent->SetSpawnPoint(LevelSpawnPoint);
		}
	}
}



void ANaviShootingControlConsole::UpdateDisplay()
{
	FString StatusStr = FString::Printf(TEXT(
		"SPEED: %s\n"
		"STREAK: %s\n"
		"BOTS: %s\n"
		"ARMOR: %s\n"
		"AMMO: %s\n"
		"\n[SHOOT START]"
	),
		*GetSpeedString(CurrentSettings.Speed),
		*GetStreakString(CurrentSettings.Streak),
		*GetMoveString(CurrentSettings.BotMovement),
		CurrentSettings.bBotArmor ? TEXT("ON") : TEXT("OFF"),
		CurrentSettings.bInfiniteAmmo ? TEXT("ON") : TEXT("OFF")
	);

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(StatusStr));
	}
}

FString ANaviShootingControlConsole::GetSpeedString(ENaviTestSpeed Speed) const
{
	switch (Speed)
	{
	case ENaviTestSpeed::Easy: return TEXT("Easy");
	case ENaviTestSpeed::Medium: return TEXT("Medium");
	case ENaviTestSpeed::Hard: return TEXT("Hard");
	default: return TEXT("Unknown");
	}
}

FString ANaviShootingControlConsole::GetStreakString(ENaviTestStreak Streak) const
{
	switch (Streak)
	{
	case ENaviTestStreak::Limit50: return TEXT("50");
	case ENaviTestStreak::Limit100: return TEXT("100");
	case ENaviTestStreak::Endless: return TEXT("Endless");
	default: return TEXT("Unknown");
	}
}

FString ANaviShootingControlConsole::GetMoveString(ENaviBotMovement Move) const
{
	switch (Move)
	{
	case ENaviBotMovement::Static: return TEXT("Stand");
	case ENaviBotMovement::Strafe: return TEXT("Strafe");
	default: return TEXT("Unknown");
	}
}

void ANaviShootingControlConsole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANaviShootingControlConsole, CurrentSettings);
}

void ANaviShootingControlConsole::OnRep_CurrentSettings()
{
	UpdateDisplay();
}
