// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAgentDescriptionPanel.h"
#include "CommonTextBlock.h"
#include "Engine/DataTable.h"
#include "ui/Foundation/LyraButtonBase.h"
#include "LyraGameplayTags.h"
#include "LyraLogChannels.h"    

void UNaviAgentDescriptionPanel::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태 설정 (예: 텍스트 클리어)
    if (Description_TextBlock)
    {
        Description_TextBlock->SetText(FText::GetEmpty());
    }
    if (AgentName_TextBlock)
    {
        AgentName_TextBlock->SetText(FText::GetEmpty());
    }
    if (AgentDescription_TextBlock)
    {
        AgentDescription_TextBlock->SetText(FText::GetEmpty());
    }
    if (Name_TextBlock)
    {
        Name_TextBlock->SetText(FText::GetEmpty());
    }
    if (Description_TextBlock)
    {
        Description_TextBlock->SetText(FText::GetEmpty());
    }

    AgentClass_Button->OnClicked().AddUObject(this, &ThisClass::UpdateClassDescriptionTextBlock);
    CSkill_Button->OnClicked().AddLambda([this]()
        {
            // 람다 함수 본문에서 실제 함수 호출
            this->UpdateSkillTextBlocks(LyraGameplayTags::Agent_Skill_C);
        });
    ESkill_Button->OnClicked().AddLambda([this]() 
      {
          // 람다 함수 본문에서 실제 함수 호출
          this->UpdateSkillTextBlocks(LyraGameplayTags::Agent_Skill_E);
      });
    QSkill_Button->OnClicked().AddLambda([this]() 
    {
        // 람다 함수 본문에서 실제 함수 호출
        this->UpdateSkillTextBlocks(LyraGameplayTags::Agent_Skill_Q);
    });
    XSkill_Button->OnClicked().AddLambda([this]() 
      {
          // 람다 함수 본문에서 실제 함수 호출
          this->UpdateSkillTextBlocks(LyraGameplayTags::Agent_Skill_X);
      });
}

void UNaviAgentDescriptionPanel::UpdateAgentTextBlocks(const FGameplayTag& TargetAgentTag)
{
    if (TargetAgentTag == CurrentAgentTag || !AgentSkillUIData || !AgentDescriptionData || !AgentClassData) return;
    
    AgentDescription_TextBlock->SetVisibility(ESlateVisibility::Visible);
    CurrentAgentTag = TargetAgentTag;

    const FName AgentTagName = TargetAgentTag.GetTagName();
    const FName ClassTagName = TargetAgentTag.RequestDirectParent().GetTagName();

    const FString AgentNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *AgentTagName.ToString(), *AgentDescriptionData->GetName(), *GetName());
    const FString ClassNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *ClassTagName.ToString(), *AgentClassData->GetName(), *GetName());

    FNaviAgentDescriptionData* FoundAgentDescriptionRow = AgentDescriptionData->FindRow<FNaviAgentDescriptionData>(AgentTagName, AgentNameString);
    if (FoundAgentDescriptionRow)
    {
        const FText AgentName = FoundAgentDescriptionRow->AgentName;
        const FText AgentDescription = FoundAgentDescriptionRow->AgentDescription;

        AgentName_TextBlock->SetText(AgentName);
        AgentDescription_TextBlock->SetText(AgentDescription);
    }
    else
    {
        AgentName_TextBlock->SetText(FText::GetEmpty());
        AgentDescription_TextBlock->SetText(FText::GetEmpty());
    }
    
    FNaviAgentClassData* FoundClassRow = AgentClassData->FindRow<FNaviAgentClassData>(ClassTagName, ClassNameString);
    if (FoundClassRow)
    {
        const FText ClassName = FoundClassRow->ClassName;
        const FText ClassDescription = FoundClassRow->ClassDescription;

        AgentClass_TextBlock->SetText(ClassName);
        Name_TextBlock->SetText(ClassName);
        Description_TextBlock->SetText(ClassDescription);
    }
    else
    {
        AgentClass_TextBlock->SetText(FText::GetEmpty()); // 혹은 FText::FromString("Not Found") 등
        Name_TextBlock->SetText(FText::GetEmpty());
        Description_TextBlock->SetText(FText::GetEmpty());
    }
}

void UNaviAgentDescriptionPanel::UpdateSkillTextBlocks(const FGameplayTag& TargetSkillTag)
{
    FString AgentString = CurrentAgentTag.ToString();
    FString AgentShortName;
    if (!AgentString.Split(TEXT("."), nullptr, &AgentShortName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        AgentShortName = AgentString; 
        UE_LOG(LogTemp, Warning, TEXT("Could not extract short name from AgentTag: %s"), *AgentString);
        // 필요 시 에러 처리
    }
    
    FString SkillShortID;
    
    if (TargetSkillTag == LyraGameplayTags::Agent_Skill_C)           SkillShortID = TEXT("C");
    else if (TargetSkillTag == LyraGameplayTags::Agent_Skill_Q)      SkillShortID = TEXT("Q");
    else if (TargetSkillTag == LyraGameplayTags::Agent_Skill_E)      SkillShortID = TEXT("E");
    else if (TargetSkillTag == LyraGameplayTags::Agent_Skill_X)      SkillShortID = TEXT("X");
    else if (TargetSkillTag == LyraGameplayTags::Agent_Skill_Passive_1)      SkillShortID = TEXT("P.1");
    else if (TargetSkillTag == LyraGameplayTags::Agent_Skill_Passive_2)      SkillShortID = TEXT("P.2");
    else SkillShortID = TEXT("UnKnown");

    FName ConstructedRowName;
    if (!AgentShortName.IsEmpty() && SkillShortID != TEXT("Unknown"))
    {
        ConstructedRowName = FName(*FString::Printf(TEXT("%s.%s"), *AgentShortName, *SkillShortID));
    }
    else
    {
        if (Name_TextBlock) Name_TextBlock->SetText(FText::GetEmpty());
        if (Description_TextBlock) Description_TextBlock->SetText(FText::GetEmpty());
        return; 
    }
    
    if (AgentDescription_TextBlock) AgentDescription_TextBlock->SetVisibility(ESlateVisibility::Collapsed);
    const FString ContextString = FString::Printf(TEXT("Looking for constructed Row '%s' in DataTable %s for Widget %s"), *ConstructedRowName.ToString(), AgentSkillUIData ? *AgentSkillUIData->GetName() : TEXT("None"), *GetName());
    
    FNaviAgentSkillUIData* FoundRowData = AgentSkillUIData->FindRow<FNaviAgentSkillUIData>(ConstructedRowName, ContextString);
    if (FoundRowData)
    {
        if (Name_TextBlock && Description_TextBlock)
        {

            Name_TextBlock->SetText(FoundRowData->SkillName);
            Description_TextBlock->SetText(FoundRowData->SkillDescription);
        }
    }
    else
    {
        if (Name_TextBlock) Name_TextBlock->SetText(FText::GetEmpty());
        if (Description_TextBlock) Description_TextBlock->SetText(FText::GetEmpty());
    }
}

void UNaviAgentDescriptionPanel::UpdateClassDescriptionTextBlock()
{
    AgentDescription_TextBlock->SetVisibility(ESlateVisibility::Visible);
    
    const FName AgentTagName = CurrentAgentTag.GetTagName();
    const FName ClassTagName = CurrentAgentTag.RequestDirectParent().GetTagName();

    const FString AgentNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *AgentTagName.ToString(), *AgentDescriptionData->GetName(), *GetName());
    const FString ClassNameString = FString::Printf(TEXT("Looking for Row %s in DataTable %s for Widget %s"), *ClassTagName.ToString(), *AgentClassData->GetName(), *GetName());

    FNaviAgentDescriptionData* FoundAgentDescriptionRow = AgentDescriptionData->FindRow<FNaviAgentDescriptionData>(AgentTagName, AgentNameString);
    if (FoundAgentDescriptionRow)
    {
        const FText AgentDescription = FoundAgentDescriptionRow->AgentDescription;
        AgentDescription_TextBlock->SetText(AgentDescription);
    }
    else
    {
        AgentDescription_TextBlock->SetText(FText::GetEmpty());
    }
    FNaviAgentClassData* FoundClassRow = AgentClassData->FindRow<FNaviAgentClassData>(ClassTagName, ClassNameString);
    if (FoundClassRow)
    {
        const FText ClassName = FoundClassRow->ClassName;
        const FText ClassDescription = FoundClassRow->ClassDescription;
        
        Name_TextBlock->SetText(ClassName);
        Description_TextBlock->SetText(ClassDescription);
    }
    else
    {
        Name_TextBlock->SetText(FText::GetEmpty());
        Description_TextBlock->SetText(FText::GetEmpty());
    }
}
