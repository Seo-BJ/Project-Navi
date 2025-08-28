// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "EquipmentInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEquipmentInterface : public UInterface
{
	GENERATED_BODY()
};

class IEquipmentInterface
{
	GENERATED_BODY()

public:
	virtual void OnEquippedClient() = 0;
};