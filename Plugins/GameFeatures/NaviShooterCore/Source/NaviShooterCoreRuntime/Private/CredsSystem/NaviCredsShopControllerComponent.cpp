// Fill out your copyright notice in the Description page of Project Settings.

#include "CredsSystem/NaviCredsShopControllerComponent.h"
#include "CredsSystem/NaviCredsShopComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"


UNaviCredsShopControllerComponent::UNaviCredsShopControllerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UNaviCredsShopControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UNaviCredsShopControllerComponent::ServerRequestPurchaseWeapon_Implementation(FGameplayTag WeaponTag)
{
	// 유효하지 않은 태그는 무시합니다.
	if (!WeaponTag.IsValid())
	{
		return;
	}

	// 컴포넌트의 오너인 플레이어 컨트롤러를 가져옵니다.
	AController* OwnerController = GetOwner<AController>();
	if (!OwnerController)
	{
		return;
	}

	// 월드와 게임 스테이트를 가져옵니다.
	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}

	// 게임 스테이트에서 상점 컴포넌트를 찾습니다.
	UNaviCredsShopComponent* ShopComponent = GameState->FindComponentByClass<UNaviCredsShopComponent>();
	if (ShopComponent)
	{
		// 찾은 상점 컴포넌트의 서버 함수를 호출합니다.
		// 이 함수는 서버 RPC이므로, 클라이언트에서 호출하면 서버로 요청이 전송됩니다.
		ShopComponent->PurchaseWeapon(OwnerController, WeaponTag);
	}
}