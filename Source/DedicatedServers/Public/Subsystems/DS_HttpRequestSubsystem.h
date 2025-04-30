// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "DS_HttpRequestSubsystem.generated.h"


class UAPIData;
class UDSLocalPlayerSubsystem;
/**
 * 
 */
UCLASS(Blueprintable)
class DEDICATEDSERVERS_API UDS_HttpRequestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	UDSLocalPlayerSubsystem* GetDSLocalPlayerSubsystem() const;
	
protected:

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAPIData> APIData;

	bool ContainsErrors(TSharedPtr<FJsonObject> JsonObject);
	void DumpMetaData(TSharedPtr<FJsonObject> JsonObject);

	FString SerializeJsonContent(const TMap<FString, FString>& Params);
};
