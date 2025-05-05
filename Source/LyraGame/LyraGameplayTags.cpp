// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "LyraLogChannels.h"

namespace LyraGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
						  
	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending Lyra, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See LyraGameplayTags::CustomMovementModeTagMap.");


	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Team_Changed, "Lyra.Team.Changed", "");
	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class, "Agent.Class", "Agent class");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Duelist, "Agent.Class.Duelist", "Agent class is Duelist");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Initiator, "Agent.Class.Initiator", "Agent class is Initiator");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Sentinel, "Agent.Class.Sentinel", "Agent class is Sentinel");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Controller, "Agent.Class.Controller", "Agent class is Controller");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill, "Agent.Skill", "Agent Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_Passive, "Agent.Skill.Passive", "Agent Passive Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_Passive_1, "Agent.Skill.Passive.1", "First Passive Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_Passive_2, "Agent.Skill.Passive.2", "Second Passive Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_C, "Agent.Skill.C", "Agent C Skill, First Default Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_Q, "Agent.Skill.Q", "Agent Q Skill, Second Default Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_E, "Agent.Skill.E", "Agent E Skill, Unique Skill");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Skill_X, "Agent.Skill.X", "Agent X Skill, Ultimate Skill");

	// 타격대 (Duelist)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Duelist_Phoenix, "Agent.Class.Duelist.Phoenix", "Agent Phoenix ");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Duelist_Yoru, "Agent.Class.Duelist.Yoru", "Agent Yoru ");

	// 척후대 (Initiator)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Initiator_KayO, "Agent.Class.Initiator.KayO", "Agent KayO ");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Initiator_Sova, "Agent.Class.Initiator.Sova", "Agent Sova "); // 설명 추가

	// 감시자 (Sentinel)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Sentinel_Sage, "Agent.Class.Sentinel.Sage", "Agent Sage "); // 설명 수정
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Sentinel_Cypher, "Agent.Class.Sentinel.Cypher", "Agent Cypher "); // 설명 수정

	// 전략가 (Controller)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Controller_Viper, "Agent.Class.Controller.Viper", "Agent Viper "); // 설명 수정
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Class_Controller_Brimstone, "Agent.Class.Controller.Brimstone", "Agent Brimstone "); // 설명 수정

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Added, "Agent.Selection.Added", "GameplayMesshage ChannelTag When AgentSelection Added");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Removed, "Agent.Selection.Removed", "GameplayMesshage ChannelTag When AgentSelection Removed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Changed, "Agent.Selection.Changed", "GameplayMesshage ChannelTag When AgentSelection Changed");


	
	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogLyra, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}

