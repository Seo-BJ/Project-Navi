// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"       // FTableRowBase 사용
#include "GameplayTagContainer.h"    // FGameplayTag 사용
#include "NaviUIDataTypes.generated.h" // 이 헤더 파일의 .generated 파일 포함 (파일 이름과 일치해야 함)

/**
 * @struct FNaviAgentSkillUIData
 * @brief 요원의 일반 정보 또는 특정 스킬(C, Q, E, X)의 UI 데이터를 정의하는 구조체입니다.
 * 데이터 테이블의 행으로 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FNaviAgentSkillUIData : public FTableRowBase // 데이터 테이블 행의 기반
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data", meta = (ToolTip = "Identifies if this data is for Agent Info or a specific Skill (e.g., Agent.NaviAgent.Info, Agent.NaviAgent.Ability.C)"))
    FGameplayTag AgentTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data", meta = (ToolTip = "Identifies if this data is for Agent Info or a specific Skill (e.g., Agent.NaviAgent.Info, Agent.NaviAgent.Ability.C)"))
    FGameplayTag IdentifierTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data", meta = (ToolTip = "Simple text like 'Info', 'C', 'Q', 'E', 'X' for direct UI display"))
    FText TypeText;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "Icon image for this ability or info (Soft reference)"))
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized display name (Skill or Class Name)"))
    FText SkillName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized description for the skill or class"))
    FText SkillDescription;
    
    FNaviAgentSkillUIData() {}
};


/**
 * @struct FNaviAgentDescriptionData
 * @brief 각 요원에 대한 설명을 별도로 관리하기 위한 구조체입니다.
 * 데이터 테이블의 행으로 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FNaviAgentDescriptionData : public FTableRowBase // 데이터 테이블 행의 기반
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data", meta = (ToolTip = "Identifies which Agent this description belongs to (e.g., Agent.NaviAgent)"))
    FGameplayTag AgentTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized detailed description for the agent"))
    FText AgentName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized detailed description for the agent"))
    FText AgentDescription;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "Icon image for this ability or info (Soft reference)"))
    TSoftObjectPtr<UTexture2D> Icon;
    
    FNaviAgentDescriptionData() {}
};

/**
 * @struct FNaviAgentClassData
 * @brief 각 요원에 대한 설명을 별도로 관리하기 위한 구조체입니다.
 * 데이터 테이블의 행으로 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FNaviAgentClassData : public FTableRowBase // 데이터 테이블 행의 기반
{
    GENERATED_BODY()

public:
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data", meta = (ToolTip = "Identifies which Agent this description belongs to (e.g., Agent.NaviAgent)"))
    FGameplayTag ClassTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized detailed description for the agent"))
    FText ClassName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ToolTip = "Localized detailed description for the agent"))
    FText ClassDescription;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ToolTip = "Icon image for this ability or info (Soft reference)"))
    TSoftObjectPtr<UTexture2D> Icon;
    
    FNaviAgentClassData() {}
};