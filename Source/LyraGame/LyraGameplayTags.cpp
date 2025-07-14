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
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_ShooterGame_Elimination, "Lyra.Experience.ShooterGame.Elimination", "Lyra 슈터 게임 경험: 상대방 제거에 중점을 둡니다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_ShooterGame_ControlPoints, "Lyra.Experience.ShooterGame.ControlPoints", "Lyra 슈터 게임 경험: 거점 점령 및 방어에 중점을 둡니다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_ShooterGame_Test_Basic, "Lyra.Experience.ShooterGame.Test.Basic", "Lyra 슈터 게임의 기본적인 기능을 테스트하기 위한 경험입니다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_ShooterGame_Test_Automated, "Lyra.Experience.ShooterGame.Test.Automated", "Lyra 슈터 게임의 자동화된 테스트를 위한 경험입니다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_ShooterGame_Test_Perf, "Lyra.Experience.ShooterGame.Test.Perf", "Lyra 슈터 게임의 성능 테스트를 위한 경험입니다.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_TopDownArena, "Lyra.Experience.TopDownArena", "Lyra 탑다운 아레나 게임 모드 경험입니다 (싱글 플레이어 또는 로컬).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_TopDownArena_Multiplayer, "Lyra.Experience.TopDownArena.Multiplayer", "Lyra 멀티플레이어 탑다운 아레나 게임 모드 경험입니다.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_Default, "Lyra.Experience.Default", "기본 Lyra 경험으로, 폴백 또는 기반으로 사용됩니다.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_FrontEnd, "Lyra.Experience.FrontEnd", "메인 메뉴 또는 프론트엔드 UI를 위한 Lyra 경험입니다.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lyra_Experience_TestInventory, "Lyra.Experience.TestInventory", "인벤토리 시스템 테스트를 위한 Lyra 경험입니다.");
	
	
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

namespace NaviGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Experience_FrontEnd, "Navi.Experience.FrontEnd", "Navi 프로젝트의 메인 메뉴 또는 프론트엔드 UI를 위한 경험입니다. (예: 발로란트의 초기 화면)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Experience_Lobby, "Navi.Experience.Lobby", "Navi 프로젝트의 게임 전 로비 경험입니다. 플레이어들이 파티를 구성하고 매치메이킹을 대기하는 공간입니다. (예: 발로란트의 파티 화면)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Experience_AgentSelection, "Navi.Experience.AgentSelection", "Navi 프로젝트의 요원(캐릭터) 선택 단계를 위한 경험입니다. 매치 시작 전 요원을 선택하는 과정을 담당합니다. (예: 발로란트의 요원 선택창)");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Experience_Normal, "Navi.Experience.Normal", "Normal Mod with Unrated");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Experience_TeamDeathMatch, "Navi.Experience.TeamDeathMatch", "Team Deathmatch Mod");

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

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Added, "Agent.Selection.Added", "GameplayMessage ChannelTag When AgentSelection Added");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Removed, "Agent.Selection.Removed", "GameplayMessage ChannelTag When AgentSelection Removed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Changed, "Agent.Selection.Changed", "GameplayMessage ChannelTag When AgentSelection Changed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_Confirm, "Agent.Selection.Confirm", "GameplayMessage ChannelTag When Player confirm selection");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Agent_Selection_AllPlayerConfirm, "Agent.Selection.AllPlayerConfirm", "GameplayMessage ChannelTag When All Player Confirm selection");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Map_ShooterGym, "Navi.Map.ShooterGym", "Navi Shooter Gym");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Map_PerformanceTest, "Navi.Map.PerformanceTest", "Navi Performance Test Map");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Map_FeatureTest, "Navi.Map.FeatureTest", "Navi Feature Test Map");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Map_Ascent, "Navi.Map.Ascent", "Valorant Ascent Map");

	// Game 관련 Tag
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Navi_Elimination_Message, "Navi.Elimination.Message", "");

	// 무기, 방어 Tag
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_Classic, "Weapon.Sidearms.Classic", "Weapon Classic");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_Shorty, "Weapon.Sidearms.Shorty", "Weapon Shorty");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_Frenzy, "Weapon.Sidearms.Frenzy", "Weapon Frenzy");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_Ghost, "Weapon.Sidearms.Ghost", "Weapon Ghost");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_Sheriff, "Weapon.Sidearms.Sheriff", "Weapon Sheriff");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_GoldenGun, "Weapon.Sidearms.GoldenGun", "Weapon Golden Gun (Spike Rush Only)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Sidearms_SnowballLauncher, "Weapon.Sidearms.SnowballLauncher", "Weapon Snowball Launcher (Event Only)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_SMGs_Stinger, "Weapon.SMGs.Stinger", "Weapon Stinger");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_SMGs_Spectre, "Weapon.SMGs.Spectre", "Weapon Spectre");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Shotguns_Bucky, "Weapon.Shotguns.Bucky", "Weapon Bucky");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Shotguns_Judge, "Weapon.Shotguns.Judge", "Weapon Judge");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Rifles_Bulldog, "Weapon.Rifles.Bulldog", "Weapon Bulldog");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Rifles_Guardian, "Weapon.Rifles.Guardian", "Weapon Guardian");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Rifles_Phantom, "Weapon.Rifles.Phantom", "Weapon Phantom");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Rifles_Vandal, "Weapon.Rifles.Vandal", "Weapon Vandal");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_SniperRifles_Marshal, "Weapon.SniperRifles.Marshal", "Weapon Marshal");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_SniperRifles_Outlaw, "Weapon.SniperRifles.Outlaw", "Weapon Outlaw");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_SniperRifles_Operator, "Weapon.SniperRifles.Operator", "Weapon Operator");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_MachineGuns_Ares, "Weapon.MachineGuns.Ares", "Weapon Ares");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_MachineGuns_Odin, "Weapon.MachineGuns.Odin", "Weapon Odin");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon_Melee_Default, "Weapon.Melee.Default", "Default Melee Weapon");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Armor_LightArmor, "Armor.LightArmor", "Light Armor");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Armor_RegenShield, "Armor.RegenShield", "Regen Shield");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Armor_HeavyArmor, "Armor.HeavyArmor", "Heavy Armor");
	
	// Creds System Message
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Award, "Navi.Creds.Award");
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Spend, "Navi.Creds.Spend");
	
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Award_PistolRound, "Navi.Creds.Award.PistolRound");
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Award_Round_Win, "Navi.Creds.Award.Round.Win");
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Award_Round_Defeat, "Navi.Creds.Award.Round.Defeat");
	UE_DEFINE_GAMEPLAY_TAG(Navi_Creds_Award_Kill, "Navi.Creds.Award.Round.Kill");
	UE_DEFINE_GAMEPLAY_TAG(Navi_Experience_PlantSpike, "Navi.Creds.Award.PlantSpike");
	
	UE_DEFINE_GAMEPLAY_TAG(Navi_Cheat_InfiniteCreds, "Navi.Cheat.InfiniteCreds");

	
}

