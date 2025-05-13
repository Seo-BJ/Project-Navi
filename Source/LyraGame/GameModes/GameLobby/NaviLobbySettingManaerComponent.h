
#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "LyraGameplayTags.h"

#include "NaviLobbySettingManaerComponent.generated.h"

class UNaviMapDefinition;
class UNaviExperienceDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapDefinitionChanged, UNaviMapDefinition*, MapDefinition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExperienceDefinitionChanged, UNaviExperienceDefinition*, ExperienceDefinition);

class ULyraExperienceDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UNaviLobbySettingManaerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	
	UNaviLobbySettingManaerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	void HandleMapSelectionRequest(UNaviMapDefinition* SelectedMapDefinition);

	UFUNCTION(BlueprintCallable)
	void HandleExperienceSelectionRequest(UNaviExperienceDefinition* SeletedExperienceDefinition);

	UFUNCTION(BlueprintCallable)
	void SaveSelectedExperienceAndMapDefinition();
	
	UPROPERTY(BlueprintAssignable)
	FMapDefinitionChanged OnMapDefinitionChanged;

	UPROPERTY(BlueprintAssignable)
	FExperienceDefinitionChanged OnExperienceDefnitionChanged;

protected:

	virtual void BeginPlay() override;

	
private:
	
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	UPROPERTY(ReplicatedUsing = OnRep_MapDefinition)
	TObjectPtr<UNaviMapDefinition> MapDefinition;

	UPROPERTY(ReplicatedUsing = OnRep_ExperienceDefinition)
	TObjectPtr<UNaviExperienceDefinition> ExperienceDefinition;
	
	
	UFUNCTION()
	void OnRep_MapDefinition();

	UFUNCTION()
	void OnRep_ExperienceDefinition();
};
