

#include "NaviMapDefinition.h"
#include "UI/Foundation/LyraLoadingScreenSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "Blueprint/UserWidget.h"



void UNaviMapDefinition::RequestAndSetLoadingScreenWidget(UObject* WorldContextObject) const
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: WorldContextObject is null for %s."), *GetName());
        return;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: Could not get World from WorldContextObject for %s."), *GetName());
        return;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: Could not get GameInstance for %s."), *GetName());
        return;
    }

    // ULyraLoadingScreenSubsystem의 실제 클래스 명칭과 헤더를 확인하세요.
    ULyraLoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance->GetSubsystem<ULyraLoadingScreenSubsystem>();
    if (!LoadingScreenSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: ULyraLoadingScreenSubsystem not found for %s."), *GetName());
        return;
    }

    if (LoadingScreenWidget.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: LoadingScreenWidget is not set in %s. Setting to null (or default)."), *GetName());
        LoadingScreenSubsystem->SetLoadingScreenContentWidget(nullptr); // 혹은 기본 로딩 위젯 설정
        return;
    }
    
    if (TSubclassOf<UUserWidget> LoadedWidgetClass = LoadingScreenWidget.Get())
    {
        UE_LOG(LogTemp, Log, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: LoadingScreenWidget %s already loaded. Setting directly for %s."), *LoadedWidgetClass->GetName(), *GetName());
        LoadingScreenSubsystem->SetLoadingScreenContentWidget(LoadedWidgetClass);
        return;
    }

    // 비동기 로드 요청
    if (LoadingScreenWidget.IsPending()) // IsPending()은 TSoftObjectPtr에는 있지만 TSoftClassPtr에는 직접적으로는 없습니다. ToSoftObjectPath()로 변환 후 사용
    {
        UE_LOG(LogTemp, Log, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: Requesting async load for %s in %s."), *LoadingScreenWidget.ToSoftObjectPath().ToString(), *GetName());

        FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager(); // Lyra는 AssetManager를 사용합니다.
        
        TWeakObjectPtr<ULyraLoadingScreenSubsystem> WeakLoadingScreenSubsystem = LoadingScreenSubsystem;
        TSoftClassPtr<UUserWidget> WidgetToLoad = LoadingScreenWidget; // 람다에서 캡처하기 위해 복사
        FString MapDefName = GetName(); // 로그용

        StreamableManager.RequestAsyncLoad(WidgetToLoad.ToSoftObjectPath(),
            [WeakLoadingScreenSubsystem, WidgetToLoad, MapDefName]() {
                if (ULyraLoadingScreenSubsystem* StrongSubsystem = WeakLoadingScreenSubsystem.Get())
                {
                    TSubclassOf<UUserWidget> LoadedClass = WidgetToLoad.Get(); // 로드 완료 후 클래스 가져오기
                    if (LoadedClass)
                    {
                        StrongSubsystem->SetLoadingScreenContentWidget(LoadedClass);
                        UE_LOG(LogTemp, Log, TEXT("Async load complete: Set loading screen widget %s from map definition %s."), *LoadedClass->GetName(), *MapDefName);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Async load complete: Failed to get widget class %s from map definition %s after loading."), *WidgetToLoad.ToSoftObjectPath().ToString(), *MapDefName);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Async load complete: LyraLoadingScreenSubsystem became invalid for map definition %s, widget %s."), *MapDefName, *WidgetToLoad.ToSoftObjectPath().ToString());
                }
            });
    }
    else
    {
        // IsPending()이 false이고 Get()도 nullptr인 경우는 SoftClassPtr이 처음부터 Null인 경우입니다.
        // 이는 함수 시작 부분의 LoadingScreenWidget.IsNull() 체크에서 이미 처리됩니다.
        // 만약 SoftClassPtr이 유효한 경로를 가리키지만 로드되지 않았고 IsPending도 아니라면, 이는 특이한 상황일 수 있습니다.
         UE_LOG(LogTemp, Warning, TEXT("UNaviMapDefinition::RequestAndSetLoadingScreenWidget: LoadingScreenWidget %s is not pending but not loaded for %s. This state is unusual or widget was null."), *LoadingScreenWidget.ToSoftObjectPath().ToString(), *GetName());
    }
	
}
