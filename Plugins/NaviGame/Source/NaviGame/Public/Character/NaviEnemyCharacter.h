// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NaviCharacterWithAbilities.h"
#include "NaviEnemyCharacter.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class ANaviAIController;
/**
 * 
 */
UCLASS()
class NAVIGAME_API ANaviEnemyCharacter : public ANaviCharacterWithAbilities
{
	GENERATED_BODY()

public:

	ANaviEnemyCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override;

protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<ANaviAIController> AuraAIController;

private:

};
