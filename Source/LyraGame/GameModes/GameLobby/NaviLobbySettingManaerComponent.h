
#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "LyraGameplayTags.h"

#include "NaviLobbySettingManaerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSettingTagChanged, FGameplayTag, OldSettingTag, FGameplayTag, NewSettingTag);

class ULyraExperienceDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UNaviLobbySettingManaerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	
	UNaviLobbySettingManaerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayTag GetMapTag();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayTag GetModTag();
	
	UFUNCTION(BlueprintCallable)
	void HandleMapSelectionRequest(const FGameplayTag& SelectedMapTag);

	UFUNCTION(BlueprintCallable)
	void HandleModSelectionRequest(const FGameplayTag& SelectedModTag);

	UPROPERTY(BlueprintAssignable)
	FSettingTagChanged OnMapSettingTagChanged;

	UPROPERTY(BlueprintAssignable)
	FSettingTagChanged OnModSettingTagChanged;

protected:

	virtual void BeginPlay() override;

	
private:
	
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	UPROPERTY(ReplicatedUsing = OnRep_MapTag)
	FGameplayTag MapTag;

	UPROPERTY(ReplicatedUsing = OnRep_ModTag)
	FGameplayTag ModTag;
	
	UFUNCTION()
	void OnRep_MapTag(FGameplayTag OldTag);

	UFUNCTION()
	void OnRep_ModTag(FGameplayTag OldTag);
};
