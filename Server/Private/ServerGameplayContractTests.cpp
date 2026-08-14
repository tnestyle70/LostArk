#include "ServerGameplayContractTests.h"

#include "Gameplay/CombatCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "WorldDestructionBootstrapContractTests.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>

namespace
{
	struct TESTS
	{
		void Require(const bool condition, const char* name)
		{
			std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
			if (!condition)
				++failures;
		}
		int failures = 0;
	};

	struct QUICK_SKILL_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		const char* inputSlot;
	};

	constexpr std::array QUICK_SKILLS
	{
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34040, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34540, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34090, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34550, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34100, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34560, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34160, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34570, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34580, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34120, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34590, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34110, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34150, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34000, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34500, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34520, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34650, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34610, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34630, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38020, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38050, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38120, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38200, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38180, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38210, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38260, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38290, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38250, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38320, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45050, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45620, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45300, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45070, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45190, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45600, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45810, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45820, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31200, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31430, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31480, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31460, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31420, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31490, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31470, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31950, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31110, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31050, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31910, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31930, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050100, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050120, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050160, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050180, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050210, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050220, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050240, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050230, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050500, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050520, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050540, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17030, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17080, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17110, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17090, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17040, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17100, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17140, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17240, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17820, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17170, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17250, "ALT_V" }
	};

	struct BASIC_ATTACK_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		std::size_t stageCount;
	};

	constexpr std::array BASIC_ATTACKS
	{
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34010, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34510, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050010, 4 }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	{
		using namespace LostArk::Shared::CombatCollision;

		const CIRCLE_XZ circle{ 0.f, 0.f, 2.f };
		const BODY_CIRCLE_XZ tangentCircle{ 3.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ missedCircle{ 3.001f, 0.f, 1.f };
		tests.Require(
			Circles_Overlap(circle, tangentCircle) &&
			!Circles_Overlap(circle, missedCircle),
			"Treat circle tangency as contact and reject a separated circle");

		const BODY_CIRCLE_XZ innerRingTangent{ 2.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ insideRingHole{ 1.9f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outerRingTangent{ 6.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outsideRing{ 6.001f, 0.f, 1.f };
		tests.Require(
			Circle_IntersectsRing(innerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(insideRingHole, 0.f, 0.f, 3.f, 5.f) &&
			Circle_IntersectsRing(outerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(outsideRing, 0.f, 0.f, 3.f, 5.f),
			"Respect both inclusive ring boundaries and reject both misses");

		const BODY_CIRCLE_XZ rotatedShapeHit{ 2.f, 2.f, 0.25f };
		const BODY_CIRCLE_XZ rotatedShapeMiss{ 0.f, 2.5f, 0.25f };
		tests.Require(
			Circle_IntersectsForwardBox(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsForwardBox(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a forward box in its rotated basis");
		tests.Require(
			Circle_IntersectsCone(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f) &&
			!Circle_IntersectsCone(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f),
			"Evaluate a cone in its rotated basis");
		tests.Require(
			Circle_IntersectsCross(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsCross(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a cross in its rotated basis");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 700u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.6f;
		monster.fAttackRange = 1.f;
		monster.fEngageDistance = 8.f;
		monster.fMoveSpeed = 2.f;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER protectedPlayer{};
		protectedPlayer.iPlayerId = 701u;
		protectedPlayer.iNetEntityId = 702u;
		protectedPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		protectedPlayer.iCurrentHp = 100u;
		protectedPlayer.iMaximumHp = 100u;
		protectedPlayer.fPositionX = 1.f;
		protectedPlayer.isCombatReady = false;
		players.emplace(protectedPlayer.iPlayerId, protectedPlayer);
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 1u, damageEvents);
		const bool ignoredProtectedPlayer =
			INVALID_NET_ENTITY_ID == monster.iTargetEntityId &&
			SERVER_ENTITY_ACTION::IDLE == monster.eAction;
		players.begin()->second.isCombatReady = true;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 2u, damageEvents);
		tests.Require(
			navigationLoaded && ignoredProtectedPlayer &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
			players.begin()->second.iNetEntityId == monster.iTargetEntityId,
			"Ignore protected players and acquire the same player after combat admission");
	}
	{
		CGameRoom room{ WORLD_ID::CHARACTER_SELECT_ARENA };
		tests.Require(room.Is_Ready(),
			"Initialize Character Select room for class changes");
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Find_AvailablePlayerSpawn();
		tests.Require(nullptr != spawn,
			"Resolve Character Select class-change respawn placement");
		if (room.Is_Ready() && nullptr != spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = 11u;
			player.iPlayerId = 12u;
			player.iNetEntityId = 112u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "ClassSwitch";
			player.strSpawnPlacementId = spawn->strPlacementId;
			player.fPositionX = 17.f;
			player.fPositionY = 3.f;
			player.fPositionZ = -9.f;
			player.fYawDegrees = 33.f;
			player.iCurrentHp = 10u;
			player.iMaximumHp = 100u;
			player.iCurrentResource = 2u;
			player.iMaximumResource = 10u;
			player.eAction = PLAYER_ACTION_STATE::SKILL;
			player.iCurrentSkillId = 34120u;
			player.iActionStartTick = 9u;
			player.iLastMoveSequence = 7u;
			player.iLastSkillSequence = 8u;
			player.hasMoveGoal = true;
			player.CooldownEndTickBySkillId.emplace(34120u, 100u);

			C2S_CHANGE_CHARACTER_CLASS request{};
			request.iClientSequence = 1u;
			request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass &&
				17.f == player.fPositionX && -9.f == player.fPositionZ &&
				12u == player.iPlayerId && 112u == player.iNetEntityId &&
				7u == player.iLastMoveSequence &&
				8u == player.iLastSkillSequence &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				INVALID_SKILL_ID == player.iCurrentSkillId &&
				!player.hasMoveGoal && player.CooldownEndTickBySkillId.empty() &&
				player.iCurrentHp == player.iMaximumHp &&
				player.iCurrentResource == player.iMaximumResource,
				"Change class during action, preserve identity/position/sequences, and reset state");

			C2S_USE_SKILL oldClassSkill{};
			oldClassSkill.iClientSequence = 9u;
			oldClassSkill.iSkillId = 34120u;
			oldClassSkill.fAimX = 1.f;
			oldClassSkill.fAimZ = 0.f;
			C2S_USE_SKILL newClassSkill = oldClassSkill;
			newClassSkill.iSkillId = 31200u;
			tests.Require(
				!room.m_PlayerSkillSystem.Try_Start(
					player, oldClassSkill, room.m_GameplayCatalog, 10u) &&
				room.m_PlayerSkillSystem.Try_Start(
					player, newClassSkill, room.m_GameplayCatalog, 10u) &&
				31200u == player.iCurrentSkillId &&
				9u == player.iLastSkillSequence,
				"Reject old-class skill and approve new-class skill after class change");

			const SERVER_PLAYER accepted = player;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE ==
					room.Apply_CharacterClassChange(player, request) &&
				accepted.eCharacterClass == player.eCharacterClass &&
				accepted.iCurrentHp == player.iCurrentHp,
				"Reject stale class change without mutating player");

			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.fPositionX = 999.f;
			player.fPositionY = 999.f;
			player.fPositionZ = 999.f;
			request.iClientSequence = 2u;
			request.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			SERVER_NAV_POINT projected{};
			const bool projectedSpawn = room.m_ServerNavigation.Project_Point(
				spawn->fPositionX, spawn->fPositionZ, projected);
			tests.Require(projectedSpawn &&
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
				projected.x == player.fPositionX &&
				projected.y == player.fPositionY &&
				projected.z == player.fPositionZ &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u != player.iCurrentHp,
				"Change dead player class and respawn at projected original spawn");

			CGameRoom bernRoom{ WORLD_ID::BERN };
			const SERVER_PLAYER beforeWrongWorld = player;
			request.iClientSequence = 3u;
			request.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
			tests.Require(bernRoom.Is_Ready() &&
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD ==
					bernRoom.Apply_CharacterClassChange(player, request) &&
				beforeWrongWorld.eCharacterClass == player.eCharacterClass &&
				beforeWrongWorld.iCurrentHp == player.iCurrentHp,
				"Reject class change outside Character Select without mutation");
		}
	}
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable skill binding");
		tests.Require(
			nullptr != skill &&
			(skill->strDamageProfileId.empty() ?
				0u == catalog.Find_DamageRatePercent(skill->strDamageProfileId) :
				0u != catalog.Find_DamageRatePercent(skill->strDamageProfileId)),
			"Resolve playable skill damage policy");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.eStance = nullptr != skill ?
			skill->eRequiredStance : PLAYER_STANCE_ID::NONE;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		/* Official CostMp runs 206..938 at the reference level, so the test pool
		matches the published class pool rather than the old 100. */
		quickPlayer.iCurrentResource = 1000;
		quickPlayer.iMaximumResource = 1000;
		/* Same idea for identityCost (Artist's moon/sun orbs): a class without
		a gauge finds nullptr and stays at 0, which is correct there too. */
		const PLAYER_RUNTIME_PROFILE* quickIdentityProfile =
			catalog.Find_Player(contract.characterClass);
		quickPlayer.iMaximumIdentity = nullptr != quickIdentityProfile ?
			quickIdentityProfile->iMaximumIdentity : 0u;
		quickPlayer.iCurrentIdentity = quickPlayer.iMaximumIdentity;
		C2S_USE_SKILL quickCommand{};
		quickCommand.iClientSequence = 1;
		quickCommand.iSkillId = contract.skillId;
		quickCommand.fAimX = 1.f;
		quickCommand.fAimZ = 0.f;
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1),
			"Approve playable skill command");
	}
	for (const BASIC_ATTACK_CONTRACT& contract : BASIC_ATTACKS)
	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != combo &&
			combo->eCharacterClass == contract.characterClass &&
			combo->strInputSlot == "LMB" &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			combo->ComboStages.size() == contract.stageCount &&
			0u == combo->ComboStages.back().iInputCloseMs,
			"Resolve playable basic attack combo");
		tests.Require(
			nullptr != combo &&
			0u != catalog.Find_DamageRatePercent(combo->strDamageProfileId),
			"Resolve playable basic attack damage rate");
		if (nullptr == combo || combo->ComboStages.size() < 2u)
			continue;

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = contract.characterClass;
		comboPlayer.eStance = combo->eRequiredStance;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = contract.skillId;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSystem;
		tests.Require(
			comboSystem.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve playable basic attack first stage");

		const PLAYER_COMBO_STAGE& firstStage = combo->ComboStages.front();
		comboPlayer.fActionElapsedSeconds =
			static_cast<float>(firstStage.iInputOpenMs +
				firstStage.iInputCloseMs) * 0.0005f;
		press.iClientSequence = 2;
		comboSystem.Try_Start(comboPlayer, press, catalog, 11);
		tests.Require(
			comboPlayer.hasBufferedComboInput,
			"Buffer playable basic attack inside its input window");

		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		for (std::uint32_t tick = 12;
			tick < 132 && comboPlayer.iComboStage < 2u;
			++tick)
		{
			comboSystem.Update(
				comboPlayer,
				noTargets,
				catalog,
				nullptr,
				1.f / 30.f,
				tick,
				noDamageEvents);
		}
		tests.Require(
			2u == comboPlayer.iComboStage,
			"Advance playable basic attack from Server-owned combo window");

		SERVER_PLAYER wrongClassPlayer{};
		wrongClassPlayer.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER == contract.characterClass ?
			CHARACTER_CLASS_ID::GUNSLINGER :
			CHARACTER_CLASS_ID::LANCE_MASTER;
		wrongClassPlayer.iCurrentHp = 1000;
		wrongClassPlayer.iMaximumHp = 1000;
		wrongClassPlayer.iCurrentResource = 1000;
		wrongClassPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem wrongClassSystem;
		tests.Require(
			!wrongClassSystem.Try_Start(
				wrongClassPlayer, press, catalog, 10),
			"Reject another class's basic attack");
	}
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::GUNSLINGER),
		"Resolve Gunslinger player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::SLAYER),
		"Resolve Slayer player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST),
		"Resolve Artist player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER),
		"Resolve DimensionMaster player profile");
	tests.Require(nullptr == catalog.Find_Player(CHARACTER_CLASS_ID::DESTROYER),
		"Reject unsupported Destroyer player profile");
	tests.Require(361u == catalog.Find_DamageRatePercent("damage.player.34120"),
		"Resolve player damage rate");
	tests.Require(361u == CGameplayCatalog::Resolve_Damage(100u, 361u),
		"Resolve damage as attack power times rate");
	tests.Require(100u == CGameplayCatalog::Resolve_Damage(100u, 100u),
		"Resolve basic attack rate as exactly one attack power");
	tests.Require(0u == CGameplayCatalog::Resolve_Damage(0u, 361u),
		"Resolve zero attack power as no damage");
	tests.Require(1u == CGameplayCatalog::Resolve_Damage(1u, 1u),
		"Clamp a connected hit to at least one damage");
	tests.Require(170u == CGameplayCatalog::Apply_Defense(350u, 105u),
		"Apply the documented project defense curve");
	tests.Require(1u == CGameplayCatalog::Apply_Defense(1u, 100000u),
		"Clamp a mitigated connected hit to at least one damage");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	/* Both endpoints are open arena floor on the same connected component. The
	   old goal sat inside the wall group at the arena centre, which only pathed
	   because that group carried no blocker region yet. */
	tests.Require(navigation.Find_Path(144.75f, -115.75f, 156.25f, -122.25f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

	constexpr const char* VALTAN_WALL_RECEIVER =
		"collision.valtan.wallgroup.11047903315509031966.receiver";
	constexpr const char* VALTAN_WALL_COLLISION_STATE =
		"collision.valtan.wallgroup.11047903315509031966";
	constexpr const char* VALTAN_WALL_CONDITION =
		"condition.valtan.wall.11047903315509031966.destroyed";
	constexpr float VALTAN_WALL_CENTER_X = 161.402061f;
	constexpr float VALTAN_WALL_CENTER_Y = 23.04f;
	constexpr float VALTAN_WALL_CENTER_Z = -133.312236f;
	constexpr float VALTAN_WALL_APPROACH_X = 163.315478f;
	constexpr float VALTAN_WALL_APPROACH_Z = -137.931633f;
	constexpr float VALTAN_WALL_EXIT_X = 159.488644f;
	constexpr float VALTAN_WALL_EXIT_Z = -128.692839f;
	std::string dynamicWorldStatus;
	SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
	std::vector<SERVER_NAV_POINT> wallPassagePath;
	tests.Require(
		navigation.Has_Condition(VALTAN_WALL_CONDITION) &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Keep the intact Valtan wall footprint and its cross-wall path dynamically blocked");
	const std::uint64_t navigationRevisionBeforeReject =
		navigation.Get_Revision();
	tests.Require(
		!navigation.Prepare_ConditionChanges(
			{ { "condition.valtan.wall.unknown", true } },
			navigationStage, dynamicWorldStatus) &&
		navigationRevisionBeforeReject == navigation.Get_Revision() &&
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z),
		"Reject an unknown navigation condition without changing the live blocker");
	tests.Require(
		navigation.Prepare_ConditionChanges(
			{ { VALTAN_WALL_CONDITION, true } },
			navigationStage, dynamicWorldStatus) &&
		navigationStage.bChanged &&
		navigationStage.iNextRevision == navigation.Get_Revision() + 1u,
		"Stage one runtime navigation condition without mutating the live grid");
	navigation.Commit_ConditionChanges(std::move(navigationStage));
	tests.Require(
		navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath) && !wallPassagePath.empty(),
		"Expose the baked floor and a complete cross-wall path after the blocker is removed");
	navigation.Reset_RuntimeBlockers();
	tests.Require(
		!navigation.Is_PointWalkableExact(
			VALTAN_WALL_CENTER_X, VALTAN_WALL_CENTER_Z) &&
		!navigation.Find_Path(
			160.25f, -130.75f, 162.25f, -135.75f,
			wallPassagePath),
		"Restore the intact Valtan wall blocker and closed path on encounter reset");

	CServerCollisionSystem valtanCollisionSystem;
	tests.Require(
		valtanCollisionSystem.Initialize(
			world.Get_Placements(), dynamicWorldStatus) &&
		/* 69 interior wall boxes plus the impact receiver, and one box per
		109 outer ring slab. */
		94u == valtanCollisionSystem.Get_CollisionBoxCount() &&
		valtanCollisionSystem.Has_CollisionBox(VALTAN_WALL_RECEIVER),
		"Load the stable Valtan wall impact receiver and player blocker");
	SERVER_BOSS_RECEIVER_HIT receiverHit{};
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit) &&
		receiverHit.strReceiverPlacementId == VALTAN_WALL_RECEIVER &&
		receiverHit.fHitRatio > 0.f && receiverHit.fHitRatio < 1.f &&
		!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, 100.f, VALTAN_WALL_CENTER_Z,
			175.f, 100.f, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Sweep a fast Valtan body into the exact receiver without tunneling or high-Y false hits");
	SERVER_PLAYER valtanWallPlayer{};
	valtanWallPlayer.fPositionX = VALTAN_WALL_APPROACH_X;
	valtanWallPlayer.fPositionY = VALTAN_WALL_CENTER_Y;
	valtanWallPlayer.fPositionZ = VALTAN_WALL_APPROACH_Z;
	float wallResolvedX = 0.f;
	float wallResolvedY = 0.f;
	float wallResolvedZ = 0.f;
	bool wallMoveBlocked = false;
	tests.Require(
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Block player movement through the intact Valtan wall receiver");
	SERVER_COLLISION_STATE_STAGE collisionStage{};
	const std::uint64_t collisionRevisionBeforeReject =
		valtanCollisionSystem.Get_Revision();
	tests.Require(
		!valtanCollisionSystem.Prepare_StateChanges(
			{ { "receiver.valtan.wall.unknown", false, false } },
			collisionStage, dynamicWorldStatus) &&
		collisionRevisionBeforeReject ==
			valtanCollisionSystem.Get_Revision(),
		"Reject an unknown collision state target without mutating the receiver");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, true, false } },
			collisionStage, dynamicWorldStatus),
		"Stage BREAKING collision channels with receiver impact disabled");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	tests.Require(
		!valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit) &&
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && wallMoveBlocked,
		"Keep BREAKING player collision while suppressing duplicate boss impacts");
	tests.Require(
		valtanCollisionSystem.Prepare_StateChanges(
			{ { VALTAN_WALL_COLLISION_STATE, false, false } },
			collisionStage, dynamicWorldStatus),
		"Stage the persistent FRACTURED collision state");
	valtanCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	tests.Require(
		valtanCollisionSystem.Resolve_PlayerMove(
			valtanWallPlayer, VALTAN_WALL_EXIT_X, VALTAN_WALL_CENTER_Y,
			VALTAN_WALL_EXIT_Z, wallResolvedX, wallResolvedY,
			wallResolvedZ, wallMoveBlocked) && !wallMoveBlocked &&
		std::abs(wallResolvedX - VALTAN_WALL_EXIT_X) <= 0.001f &&
		std::abs(wallResolvedZ - VALTAN_WALL_EXIT_Z) <= 0.001f,
		"Open player collision only after the wall reaches FRACTURED");
	valtanCollisionSystem.Reset_RuntimeStates();
	tests.Require(
		valtanCollisionSystem.Sweep_BossCircleAgainstReceivers(
			145.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			175.f, VALTAN_WALL_CENTER_Y, VALTAN_WALL_CENTER_Z,
			1.f, receiverHit),
		"Restore the Valtan impact receiver when the room resets");
	SERVER_WORLD_ENTITY impactMotionBoss{};
	impactMotionBoss.strEncounterId = "ENCOUNTER_VALTAN";
	impactMotionBoss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
	impactMotionBoss.strPatternStageId = "WALL_CHARGE";
	impactMotionBoss.strActionId =
		"valtan.mechanic.armor-break-opening.charge";
	impactMotionBoss.eAction = SERVER_ENTITY_ACTION::PATTERN_WINDUP;
	impactMotionBoss.fPositionX = 150.f;
	impactMotionBoss.fPositionZ = -133.f;
	impactMotionBoss.fYawDegrees = 90.f;
	impactMotionBoss.fPatternForcedMotionSpeed = 30.f;
	float impactProposedX = 0.f;
	float impactProposedZ = 0.f;
	CValtanBrain impactMotionBrain;
	tests.Require(
		impactMotionBrain.Try_BuildImpactMotion(
			impactMotionBoss, 1.f / 30.f,
			impactProposedX, impactProposedZ) &&
		std::abs(impactProposedX - 151.f) <= 0.001f &&
		std::abs(impactProposedZ + 133.f) <= 0.001f,
		"Advance the opening charge from Server-authored fixed-tick motion");
	tests.Require(
		impactMotionBrain.Complete_ImpactStage(
			impactMotionBoss, catalog, 500u) &&
		impactMotionBoss.strPatternStageId == "GROGGY" &&
		0.f == impactMotionBoss.fPatternForcedMotionSpeed,
		"Advance the authoritative charge action to GROGGY only after impact");

	CWorldBootstrap bernWorld;
	const bool bernLoaded = bernWorld.Load(WORLD_ID::BERN);
	const auto& bernPlacements = bernWorld.Get_Placements();
	const auto bernNpc = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.beda.guide";
		});
	const auto bernCollision = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"collision.bern.editor-proof";
		});
	tests.Require(
		bernLoaded && bernPlacements.size() == 7u &&
		4u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})) &&
		bernNpc != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernNpc->eKind &&
		bernNpc->strArchetypeId == "NPC_BEDA" &&
		bernCollision != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == bernCollision->eKind,
		"Load Bern spawns, NPC_BEDA, trigger, and collision box");
	CServerCollisionSystem bernCollisionSystem;
	std::string bernCollisionStatus;
	tests.Require(
		bernCollisionSystem.Initialize(bernPlacements, bernCollisionStatus) &&
		1u == bernCollisionSystem.Get_CollisionBoxCount() &&
		std::all_of(
			bernPlacements.begin(), bernPlacements.end(),
			[&bernCollisionSystem](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != placement.eKind ||
					bernCollisionSystem.Is_PlayerSpawnClear(placement);
			}),
		"Stage Bern collision box without overlapping player spawns");
	SERVER_PLAYER collisionPlayer{};
	collisionPlayer.fPositionX = 138.f;
	collisionPlayer.fPositionY = 42.7f;
	collisionPlayer.fPositionZ = -65.3f;
	float resolvedX = 0.f;
	float resolvedY = 0.f;
	float resolvedZ = 0.f;
	bool wasBlocked = false;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-65.3f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		wasBlocked && resolvedX < 139.851f && resolvedX > 138.f,
		"Stop a fast player sweep before the Bern collision box");
	collisionPlayer.fPositionZ = -60.f;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-60.f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		!wasBlocked && std::abs(resolvedX - 143.f) < 0.001f,
		"Preserve movement that passes outside the collision box");

	CServerNavigation bernNavigation;
	tests.Require(
		bernNavigation.Load("LV_BER_BERNCASTLE"),
		"Load Bern server navigation");
	bool bernSpawnsOnNavigation = bernLoaded;
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : bernPlacements)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		bernSpawnsOnNavigation =
			bernSpawnsOnNavigation &&
			bernNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		bernSpawnsOnNavigation,
		"Project all Bern player spawns to baked navigation");
	/* The castle approach is why Bern needs a grid at all: without one the room
	keeps the spawn height for the whole session and straight-line movement walks
	through the staircase. The authoritative path from the spawn to the top of the
	baked stair run must therefore carry a real climb, and the stair run itself
	must stay a walkable slope instead of one vertical jump. */
	std::vector<SERVER_NAV_POINT> bernStairPath;
	const bool bernStairPathFound = bernNavigation.Find_Path(
		137.586334f,
		-22.4640217f,
		137.238007f,
		-116.688004f,
		bernStairPath);
	float bernStairClimb = 0.f;
	float bernStairRunStep = 0.f;
	if (bernStairPathFound && !bernStairPath.empty())
	{
		bernStairClimb = bernStairPath.back().y - bernStairPath.front().y;
		for (size_t index = 1u; index < bernStairPath.size(); ++index)
		{
			if (bernStairPath[index].y <= 47.f)
				continue;
			bernStairRunStep = (std::max)(
				bernStairRunStep,
				std::abs(bernStairPath[index].y -
					bernStairPath[index - 1u].y));
		}
	}
	tests.Require(
		bernStairPathFound && bernStairPath.size() > 100u &&
		bernStairPath.back().y > 49.f && bernStairClimb > 6.f,
		"Climb the Bern castle stairs along the authoritative path");

	CWorldBootstrap trainingWorld;
	CServerNavigation trainingNavigation;
	tests.Require(trainingWorld.Load(WORLD_ID::TRAINING_GROUND) &&
		trainingWorld.Get_AreaId() == "LV_DEV_TRAINING_GROUND" &&
		std::all_of(
			trainingWorld.Get_Placements().begin(),
			trainingWorld.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty();
			}),
		"Load class-neutral training player spawns");
	tests.Require(trainingNavigation.Load("LV_DEV_TRAINING_GROUND"),
		"Load training server navigation");
	SERVER_NAV_POINT trainingPoint{};
	tests.Require(trainingNavigation.Project_Point(0.f, -4.f, trainingPoint),
		"Project training spawn to walkable cell");
	tests.Require(!trainingNavigation.Project_Point(16.01f, 0.f, trainingPoint),
		"Reject training point beyond arena navigation bounds");

	CWorldBootstrap characterSelectWorld;
	CServerNavigation characterSelectNavigation;
	const bool characterSelectWorldLoaded =
		characterSelectWorld.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
	const auto& characterSelectSpawns =
		characterSelectWorld.Get_Placements();
	const auto lazyValtan = std::find_if(
		characterSelectSpawns.begin(),
		characterSelectSpawns.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"boss.valtan.character-select.lazy";
		});
	tests.Require(
		characterSelectWorldLoaded &&
		characterSelectWorld.Get_AreaId() ==
			"LV_LOBBY_CLASSSELECT_SL00" &&
		characterSelectSpawns.size() == 5 &&
		4u == static_cast<size_t>(std::count_if(
			characterSelectSpawns.begin(),
			characterSelectSpawns.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty() &&
					placement.isEnabled;
			})),
		"Load class-neutral Character Select arena player spawns");
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		!lazyValtan->isEnabled &&
		lazyValtan->eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
		lazyValtan->strArchetypeId == "BOSS_VALTAN" &&
		lazyValtan->strEncounterId == "ENCOUNTER_VALTAN",
		"Load disabled Character Select Valtan lazy template");
	tests.Require(
		characterSelectNavigation.Load("LV_LOBBY_CLASSSELECT_SL00"),
		"Load Character Select arena server navigation");
	bool characterSelectSpawnsOnNavigation =
		characterSelectWorldLoaded && characterSelectSpawns.size() == 5;
	SERVER_NAV_POINT characterSelectPoint{};
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : characterSelectSpawns)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		characterSelectSpawnsOnNavigation =
			characterSelectSpawnsOnNavigation &&
			characterSelectNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		characterSelectSpawnsOnNavigation,
		"Project all Character Select spawns to baked navigation");
	SERVER_NAV_POINT lazyValtanPoint{};
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		characterSelectNavigation.Project_Point(
			lazyValtan->fPositionX,
			lazyValtan->fPositionZ,
			lazyValtanPoint) &&
		std::abs(lazyValtanPoint.y - lazyValtan->fPositionY) <= 0.25f,
		"Project disabled Character Select Valtan template to navigation");
	if (!characterSelectSpawns.empty())
	{
		characterSelectNavigation.Project_Point(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectPoint);
	}
	std::vector<SERVER_NAV_POINT> characterSelectPath;
	tests.Require(
		characterSelectSpawns.size() >= 2 &&
		characterSelectNavigation.Find_Path(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectSpawns[1].fPositionX,
			characterSelectSpawns[1].fPositionZ,
			characterSelectPath) &&
		characterSelectPath.size() >= 2 &&
		std::adjacent_find(
			characterSelectPath.begin(),
			characterSelectPath.end(),
			[](const SERVER_NAV_POINT& left, const SERVER_NAV_POINT& right)
			{
				return std::abs(left.y - right.y) > 0.6f;
			}) == characterSelectPath.end(),
		"Find Character Select arena navigation path");
	SERVER_NAV_POINT characterSelectOutside{};
	tests.Require(
		!characterSelectNavigation.Project_Point(
			-787.6f,
			197.5f,
			characterSelectOutside),
		"Reject point beyond Character Select arena navigation bounds");

	SERVER_PLAYER arenaSkillPlayer{};
	arenaSkillPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	arenaSkillPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	arenaSkillPlayer.iCurrentHp = 1000;
	arenaSkillPlayer.iMaximumHp = 1000;
	arenaSkillPlayer.iCurrentResource = 1000;
	arenaSkillPlayer.iMaximumResource = 1000;
	arenaSkillPlayer.fPositionX = characterSelectPoint.x;
	arenaSkillPlayer.fPositionY = characterSelectPoint.y;
	arenaSkillPlayer.fPositionZ = characterSelectPoint.z;
	C2S_USE_SKILL arenaSkillCommand{};
	arenaSkillCommand.iClientSequence = 1;
	arenaSkillCommand.iSkillId = 34120;
	arenaSkillCommand.fAimX = characterSelectPoint.x + 3.f;
	arenaSkillCommand.fAimZ = characterSelectPoint.z;
	CPlayerSkillSystem arenaSkillSystem;
	std::vector<SERVER_WORLD_ENTITY> arenaEntities;
	tests.Require(
		arenaSkillSystem.Try_Start(
			arenaSkillPlayer,
			arenaSkillCommand,
			catalog,
			10) &&
		PLAYER_ACTION_STATE::SKILL == arenaSkillPlayer.eAction &&
		34120u == arenaSkillPlayer.iCurrentSkillId &&
		10u == arenaSkillPlayer.iActionStartTick,
		"Start Character Select arena skill action");
	std::vector<DAMAGE_EVENT> arenaDamageEvents;
	arenaSkillSystem.Update(
		arenaSkillPlayer,
		arenaEntities,
		catalog,
		&characterSelectNavigation,
		1.f / 30.f,
		11,
		arenaDamageEvents);
	SERVER_NAV_POINT arenaSkillPoint{};
	tests.Require(
		characterSelectNavigation.Project_Point(
			arenaSkillPlayer.fPositionX,
			arenaSkillPlayer.fPositionZ,
			arenaSkillPoint),
		"Keep Character Select skill action position on baked navigation");

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	player.iCurrentResource = 1000;
	player.iMaximumResource = 1000;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.iNetEntityId = 900u;
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.strArchetypeId = "BOSS_VALTAN";
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	boss.fPositionZ = -122.f;
	boss.fCollisionRadius = 3.f;
	std::vector<SERVER_WORLD_ENTITY> entities{ boss };
	C2S_USE_SKILL useSkill{};
	useSkill.iClientSequence = 1;
	useSkill.iSkillId = 34120;
	useSkill.fAimX = boss.fPositionX;
	useSkill.fAimZ = boss.fPositionZ;
	CPlayerSkillSystem skills;
	tests.Require(skills.Try_Start(player, useSkill, catalog, 10),
		"Approve valid skill command");
	tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
		"Reject duplicate skill command while action is active");
	std::vector<DAMAGE_EVENT> damageEvents;
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, 1.f / 30.f, tick,
			damageEvents);
	/* 34120 is official rate 361 at attack power 100. */
	tests.Require(9639u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage once");
	tests.Require(
		1u == damageEvents.size() &&
		361u == damageEvents[0].iAmount &&
		damageEvents[0].isOutgoing &&
		entities[0].iNetEntityId == damageEvents[0].iTargetNetEntityId,
		"Emit one outgoing damage event for the resolved hit");
	C2S_USE_SKILL cooldownAttempt = useSkill;
	cooldownAttempt.iClientSequence = 2;
	tests.Require(!skills.Try_Start(player, cooldownAttempt, catalog, 70),
		"Reject skill during authoritative cooldown");

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(34010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve LanceMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		std::vector<DAMAGE_EVENT> comboDamageEvents;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 34010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve basic attack first stage");

		// 329ms is where stage one opens; 100ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.1f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.4f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer combo input inside the window");

		press.iClientSequence = 4;
		comboSkills.Try_Start(comboPlayer, press, catalog, 15);
		tests.Require(1u == comboPlayer.iComboStage,
			"Ignore a second press inside the same window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 5;
		other.iSkillId = 34120;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 16) &&
			34010u == comboPlayer.iCurrentSkillId,
			"Reject a different skill during a combo");

		/* Stage one is 1633 ms long but its hit lands at 470 ms, so a buffered
		press has to cut in there rather than waiting out the clip. 20 ticks is
		about 667 ms: past the hit, nowhere near the full duration. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the next combo stage once the hit has landed");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	target.isCombatReady = false;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	/* The brain resolves damage through the boss's own catalog profile, so the
	test entity carries the archetype the room would have stamped on it. */
	valtan.strArchetypeId = "BOSS_VALTAN";
	valtan.strEncounterId = "ENCOUNTER_VALTAN";
	valtan.iCurrentHp = 48750;
	valtan.iMaximumHp = 60000;
	valtan.iMaximumHealthBars = 160;
	valtan.iLastEvaluatedHealthBar = 131;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	CValtanBrain brain;
	std::vector<DAMAGE_EVENT> valtanDamageEvents;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 99,
		valtanDamageEvents);
	tests.Require(
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		1000u == players.begin()->second.iCurrentHp &&
		valtanDamageEvents.empty(),
		"Protect Valtan entrant until first accepted gameplay intent");
	players.begin()->second.isCombatReady = true;
	for (std::uint32_t tick = 100; tick < 140 && valtanDamageEvents.empty(); ++tick)
		brain.Update(valtan, players, catalog, navigation, 0.1f, tick,
			valtanDamageEvents);
	tests.Require(781u == players.begin()->second.iCurrentHp,
		"Apply the queued 130-bar Valtan circle hit once");
	tests.Require(
		1u == valtanDamageEvents.size() &&
		219u == valtanDamageEvents[0].iAmount &&
		!valtanDamageEvents[0].isOutgoing &&
		players.begin()->second.iNetEntityId ==
			valtanDamageEvents[0].iTargetNetEntityId,
		"Emit one incoming damage event for the 130-bar boss hit");
	tests.Require(
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.PendingPatternIds.empty() &&
		1u == valtan.TriggeredPatternIds.size() &&
		1u == valtan.iPatternSequence &&
		1u == valtan.iPatternStageIndex,
		"Queue and advance the staged 130-bar scripted mechanic");
	valtan.iCurrentHp = 30000;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 141,
		valtanDamageEvents);
	tests.Require(2u == valtan.iPhase, "Advance Valtan phase from server HP");

	SERVER_WORLD_ENTITY openingChargeBoss{};
	openingChargeBoss.iNetEntityId = 901u;
	openingChargeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	openingChargeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
	openingChargeBoss.strArchetypeId = "BOSS_VALTAN";
	openingChargeBoss.strEncounterId = "ENCOUNTER_VALTAN";
	openingChargeBoss.iCurrentHp = 59625u;
	openingChargeBoss.iMaximumHp = 60000u;
	openingChargeBoss.iMaximumHealthBars = 160u;
	openingChargeBoss.iLastEvaluatedHealthBar = 160u;
	openingChargeBoss.iPhaseTwoHpPercent = 50u;
	openingChargeBoss.fPositionX = 151.f;
	openingChargeBoss.fPositionY = 22.97f;
	openingChargeBoss.fPositionZ = -122.f;
	openingChargeBoss.fEngageDistance = 35.f;
	openingChargeBoss.fMoveSpeed = 3.f;
	brain.Update(
		openingChargeBoss, players, catalog, navigation,
		1.f / 30.f, 200u, valtanDamageEvents);
	tests.Require(
		openingChargeBoss.strPatternId == "VALTAN_ARMOR_BREAK_OPENING" &&
		openingChargeBoss.strPatternStageId == "WALL_CHARGE" &&
		openingChargeBoss.strActionId ==
			"valtan.mechanic.armor-break-opening.charge" &&
		std::abs(openingChargeBoss.fPatternForcedMotionSpeed -
			(100.f / 1.5f)) <= 0.001f,
		"Use authored charge maximum range instead of stopping at the bait player");

	{
		SERVER_PLAYER meleePlayer{};
		meleePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		meleePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		meleePlayer.iCurrentHp = 1000;
		meleePlayer.iMaximumHp = 1000;
		meleePlayer.iCurrentResource = 1000;
		meleePlayer.iMaximumResource = 1000;
		meleePlayer.fPositionX = 0.f;
		meleePlayer.fPositionZ = 0.f;
		SERVER_WORLD_ENTITY meleeBoss{};
		meleeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		meleeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		meleeBoss.strArchetypeId = "BOSS_VALTAN";
		meleeBoss.iCurrentHp = 10000;
		meleeBoss.iMaximumHp = 10000;
		meleeBoss.fPositionX = 0.f;
		/* 34090 reaches 2.8 on its own; 3.5 is inside reach only because the
		boss's 3.0 collision radius extends the centre-to-centre test. */
		meleeBoss.fPositionZ = 3.5f;
		std::vector<SERVER_WORLD_ENTITY> meleeEntities{ meleeBoss };
		C2S_USE_SKILL melee{};
		melee.iClientSequence = 1;
		melee.iSkillId = 34090;
		melee.fAimX = 0.f;
		melee.fAimZ = 3.5f;
		CPlayerSkillSystem meleeSkills;
		std::vector<DAMAGE_EVENT> meleeDamageEvents;
		tests.Require(meleeSkills.Try_Start(meleePlayer, melee, catalog, 10),
			"Approve melee skill command");
		for (std::uint32_t tick = 11; tick < 60; ++tick)
		{
			meleeSkills.Update(
				meleePlayer, meleeEntities, catalog, nullptr, 1.f / 30.f, tick,
				meleeDamageEvents);
		}
		tests.Require(10000u - 1050u == meleeEntities[0].iCurrentHp,
			"Reach the boss through its collision radius");
	}

	{
		namespace fs = std::filesystem;
		const fs::path triggerRoot =
			fs::temp_directory_path() / L"LostArkWorldTriggerContractTest";
		std::error_code prepareError;
		fs::remove_all(triggerRoot, prepareError);
		fs::create_directories(triggerRoot / L"World");
		const fs::path bootstrapPath =
			triggerRoot / L"World" / L"VALTAN_ARENA.worldbootstrap";
		const auto writeTriggerBootstrap =
			[&bootstrapPath](const float durationSeconds)
			{
				std::ofstream bootstrap(bootstrapPath, std::ios::binary);
				bootstrap <<
					"LOSTARK_WORLD_BOOTSTRAP\t6\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t3\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\tmovePlayer\t5\t10\t0\t0\t"
					<< durationSeconds << "\t4\n"
					"collision.contract.wall\tcollisionBox\t-\t-\t4\t1\t0\t0\t1"
					"\t0.5\t1\t2\n";
			};
		writeTriggerBootstrap(1.f);

		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", triggerRoot.c_str());
		CWorldBootstrap triggerBootstrap;
		const bool loadedTriggerBootstrap = triggerBootstrap.Load(
			WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loadedTriggerBootstrap &&
			3u == triggerBootstrap.Get_Placements().size() &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ==
				triggerBootstrap.Get_Placements()[1].eKind &&
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX ==
				triggerBootstrap.Get_Placements()[2].eKind &&
			1u == triggerBootstrap.Get_Placements()[1].TriggerActions.size(),
			"Parse trigger and collision box from world bootstrap v6");

		writeTriggerBootstrap(-1.f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == triggerBootstrap.Get_Placements().size(),
			"Reject invalid trigger bootstrap without replacing committed world");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(triggerRoot, cleanupError);
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.jump";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = false;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 10.f;
		move.fTargetY = 0.f;
		move.fTargetZ = 0.f;
		move.fDurationSeconds = 1.f;
		move.fArcHeight = 4.f;
		trigger.TriggerActions.push_back(move);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus) &&
			1u == triggerSystem.Get_TriggerCount(),
			"Initialize enabled movePlayer trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> triggerPlayers;
		SERVER_PLAYER triggerPlayer{};
		triggerPlayer.iPlayerId = 1;
		triggerPlayer.fPositionX = 2.4f;
		triggerPlayer.iCurrentHp = 100;
		triggerPlayer.iMaximumHp = 100;
		triggerPlayers.emplace(1, triggerPlayer);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(triggerPlayers, 10, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			triggerPlayers.begin()->second.TriggerMove.isActive &&
			10u == triggerPlayers.begin()->second.iActionStartTick,
			"Fire trigger on OBB entry");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 6.2f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY - 4.f) < 0.001f,
			"Advance movePlayer with authored parabolic arc");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 10.f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == triggerPlayers.begin()->second.eAction &&
			!triggerPlayers.begin()->second.TriggerMove.isActive,
			"Complete movePlayer at exact authored destination");
		triggerSystem.Evaluate_Entries(triggerPlayers, 11, transfers, {});
		triggerPlayers.begin()->second.fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(triggerPlayers, 12, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			12u == triggerPlayers.begin()->second.iActionStartTick,
			"Rearm non-once trigger after player exits");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.change-level";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION changeLevel{};
		changeLevel.eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
		changeLevel.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		trigger.TriggerActions.push_back(changeLevel);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled changeLevel trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iSessionId = 7;
		player.iPlayerId = 3;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "TriggerTransfer";
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(players, 20, transfers, {});
		tests.Require(
			1u == transfers.size() &&
			7u == transfers.front().iSessionId &&
			WORLD_ID::VALTAN_ARENA == transfers.front().eTargetWorldId &&
			CHARACTER_CLASS_ID::LANCE_MASTER ==
				transfers.front().eCharacterClass &&
			"TriggerTransfer" == transfers.front().strNickName,
			"Emit one typed Server world transfer request on OBB entry");
		triggerSystem.Evaluate_Entries(players, 21, transfers, {});
			tests.Require(
			transfers.empty(),
			"Do not repeat a triggerOnce world transfer while occupied");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-spawn-group";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateSpawnGroup trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 4;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			30,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind ||
					"spawn.valtan.stage01" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateSpawnGroup target on OBB entry");
		triggerSystem.Evaluate_Entries(
			players,
			31,
			transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount,
			"Do not repeat a triggerOnce spawn-group activation while occupied");
	}

	{
		/* The Valtan route has no playable monster damage yet. Debug replaces only
		the four progression spawn triggers with authored moves; Release must keep
		the product activation action unchanged. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_1";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug Valtan stage-route bypass");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 404u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 41u, transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			});
#ifdef _DEBUG
		const SERVER_PLAYER& moving = players.begin()->second;
		tests.Require(
			0u == activationCount &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == moving.eAction &&
			moving.TriggerMove.isActive &&
			std::abs(moving.TriggerMove.fTargetX - 46.741f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetZ + 61.417f) < 0.001f,
			"Bypass the unkillable Stage_1 group and move toward the next trigger in Debug");
		triggerSystem.Update_PlayerMotion(players.begin()->second, 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			std::abs(players.begin()->second.fPositionX - 46.741f) < 0.001f &&
			std::abs(players.begin()->second.fPositionZ + 61.417f) < 0.001f,
			"Complete the Debug stage bypass at the authored next-stage approach point");
#else
		tests.Require(
			1u == activationCount &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Keep the original Valtan spawn-group trigger unchanged in Release");
#endif
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-encounter";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
		activate.strTargetId = "boss.valtan.center";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateEncounter trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 5;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			40,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind ||
					"boss.valtan.center" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateEncounter target on OBB entry");
	}

	{
		/* A bootstrap whose skill cost exceeds every class pool must fail load:
		the publisher enforces the same bound, so acceptance here would mean the
		two sides disagree about the same document. */
		namespace fs = std::filesystem;
		const fs::path overCostRoot =
			fs::temp_directory_path() / L"LostArkBalanceContractTest";
		std::error_code prepareError;
		fs::remove_all(overCostRoot, prepareError);
		fs::create_directories(overCostRoot / L"Gameplay");
		{
			std::ofstream bootstrap(
				overCostRoot / L"Gameplay" / L"Gameplay.bootstrap",
				std::ios::binary);
			bootstrap <<
				"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t6\n"
				"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
				"DAMAGE\tdamage.player.34120\t361\n"
				"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\n"
				"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
				"SKILL\t34120\tLANCE_MASTER\tQ\tlancemaster.skill.34120\t10000\t2266"
				"\t1510\t2000\t0\t0\t8\tdamage.player.34120\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
		}
		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		CGameplayCatalog rollbackCatalog;
		tests.Require(rollbackCatalog.Load(),
			"Stage a valid gameplay catalog before rollback test");
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", overCostRoot.c_str());
		CGameplayCatalog overCostCatalog;
		tests.Require(!overCostCatalog.Load(),
			"Reject bootstrap skill cost above every class pool");
		tests.Require(
			!rollbackCatalog.Load() &&
			nullptr != rollbackCatalog.Find_Skill(34010) &&
			nullptr != rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN") &&
			31u == rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN")->size(),
			"Preserve the committed catalog after a corrupt replacement fails");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(overCostRoot, cleanupError);
	}

	{
		/* A skill with no damage profile never resolves a hit, so the hit time and
		reach that only describe that hit must be zero. Accepting a reach here would
		let a movement skill silently keep a damage window. */
		namespace fs = std::filesystem;
		const fs::path noDamageRoot =
			fs::temp_directory_path() / L"LostArkNoDamageContractTest";
		const auto loadWithMovementSkill =
			[&noDamageRoot](const char* hitTimeMs, const char* maximumRange)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t6\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", noDamageRoot.c_str());
			CGameplayCatalog catalog;
			const bool loaded = catalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithMovementSkill("0", "0"),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill("0", "3"),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill("400", "0"),
			"Reject a damageless skill that still claims a hit time");
		std::error_code noDamageCleanupError;
		fs::remove_all(noDamageRoot, noDamageCleanupError);
	}

	{
		/* A staged skill carries movement per stage, because a stage advance
		resets the action clock the curve is sampled on. */
		const PLAYER_SKILL_DEFINITION* basicAttack = catalog.Find_Skill(34010);
		bool everyStageCurveFits = nullptr != basicAttack &&
			!basicAttack->ComboStages.empty() &&
			basicAttack->RootMotion.empty();
		bool anyStageMoves = false;
		if (nullptr != basicAttack)
		{
			for (const PLAYER_COMBO_STAGE& stage : basicAttack->ComboStages)
			{
				if (stage.RootMotion.empty())
					continue;
				anyStageMoves = true;
				everyStageCurveFits = everyStageCurveFits &&
					stage.RootMotion.size() >= 2u &&
					stage.RootMotion.back().iTimeMs <= stage.iActionDurationMs;
			}
		}
		tests.Require(everyStageCurveFits && anyStageMoves,
			"Resolve per-stage root motion inside each combo stage duration");

		namespace fs = std::filesystem;
		const fs::path stageRoot =
			fs::temp_directory_path() / L"LostArkStageRootMotionContractTest";
		std::error_code stagePrepareError;
		const auto loadWithStageRow = [&](const char* stageIndex)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t8\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34010\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t329\t658\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", stageRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithStageRow("0"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("1"),
			"Reject a root motion row past the last combo stage");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* 절룡세 guards, and a hit taken inside that window is what buys the
		counter: no press advances it and the guard itself lands nothing. */
		SERVER_PLAYER counterPlayer{};
		counterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		counterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		counterPlayer.iCurrentHp = 1000;
		counterPlayer.iMaximumHp = 1000;
		counterPlayer.iCurrentResource = 1000;
		counterPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem counterSkills;

		C2S_USE_SKILL counterCommand{};
		counterCommand.iClientSequence = 1;
		counterCommand.iSkillId = 34580;
		counterCommand.fAimX = 1.f;
		counterCommand.fAimZ = 0.f;
		tests.Require(
			counterSkills.Try_Start(counterPlayer, counterCommand, catalog, 10) &&
			1u == counterPlayer.iComboStage,
			"Approve the counter guard stage");

		std::vector<SERVER_WORLD_ENTITY> counterEntities;
		std::vector<DAMAGE_EVENT> counterDamageEvents;
		counterSkills.Update(counterPlayer, counterEntities, catalog, nullptr,
			1.f / 30.f, 11, counterDamageEvents);
		tests.Require(
			1u == counterPlayer.iComboStage && counterDamageEvents.empty(),
			"Hold the guard stage and land no damage while it runs");

		const std::uint32_t hpBeforeCounter = counterPlayer.iCurrentHp;
		tests.Require(
			CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 12) &&
			2u == counterPlayer.iComboStage &&
			hpBeforeCounter == counterPlayer.iCurrentHp &&
			0.f == counterPlayer.fActionElapsedSeconds,
			"Absorb the hit inside the guard window and promote to the counter");
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 13),
			"Do not counter twice from one guard");

		SERVER_PLAYER lateCounter{};
		lateCounter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		lateCounter.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		lateCounter.iCurrentHp = 1000;
		lateCounter.iMaximumHp = 1000;
		lateCounter.iCurrentResource = 1000;
		lateCounter.iMaximumResource = 1000;
		CPlayerSkillSystem lateSkills;
		C2S_USE_SKILL lateCommand = counterCommand;
		lateSkills.Try_Start(lateCounter, lateCommand, catalog, 10);
		lateCounter.fActionElapsedSeconds = 1.5f;
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(lateCounter, catalog, 20) &&
			1u == lateCounter.iComboStage,
			"Reject a hit that lands after the guard window closed");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem comboSkills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1;
		basicAttack.iSkillId = 34010;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		comboSkills.Try_Start(comboPlayer, basicAttack, catalog, 10);
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(comboPlayer, catalog, 11),
			"Never counter out of a skill that is not a COUNTER");
	}

	{
		SERVER_PLAYER stancePlayer{};
		stancePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		stancePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		stancePlayer.iCurrentHp = 1000;
		stancePlayer.iMaximumHp = 1000;
		stancePlayer.iCurrentResource = 1000;
		stancePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem stanceSkills;

		C2S_USE_SKILL shortOnlySkill{};
		shortOnlySkill.iClientSequence = 1;
		shortOnlySkill.iSkillId = 34540;
		shortOnlySkill.fAimX = 1.f;
		shortOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, shortOnlySkill, catalog, 10),
			"Reject a short spear skill while in the long spear stance");

		C2S_USE_SKILL switchToShort{};
		switchToShort.iClientSequence = 2;
		switchToShort.iSkillId = 34000;
		switchToShort.fAimX = 1.f;
		switchToShort.fAimZ = 0.f;
		tests.Require(stanceSkills.Try_Start(stancePlayer, switchToShort, catalog, 10),
			"Approve the long to short spear stance transition");
		std::vector<SERVER_WORLD_ENTITY> stanceEntities;
		std::vector<DAMAGE_EVENT> stanceDamageEvents;
		for (std::uint32_t tick = 11; tick < 40; ++tick)
		{
			stanceSkills.Update(stancePlayer, stanceEntities, catalog, nullptr,
				1.f / 30.f, tick, stanceDamageEvents);
		}
		tests.Require(
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == stancePlayer.eStance &&
			PLAYER_ACTION_STATE::NONE == stancePlayer.eAction,
			"Flip to the short spear stance once the transition action completes");

		C2S_USE_SKILL longOnlySkill{};
		longOnlySkill.iClientSequence = 3;
		longOnlySkill.iSkillId = 34120;
		longOnlySkill.fAimX = 1.f;
		longOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, longOnlySkill, catalog, 40),
			"Reject a long spear skill after switching to the short spear stance");

		C2S_USE_SKILL shortSkillNow = shortOnlySkill;
		shortSkillNow.iClientSequence = 4;
		tests.Require(stanceSkills.Try_Start(stancePlayer, shortSkillNow, catalog, 40),
			"Approve a short spear skill after switching to the short spear stance");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded =
			spawnBootstrap.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
		const auto& groups = spawnBootstrap.Get_Groups();
		const auto monsterGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		const auto minibossGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.miniboss";
			});
		const SPAWN_GROUP_ANCHOR* monsterAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.monster");
		const SPAWN_GROUP_ANCHOR* minibossAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.miniboss");
		const MONSTER_RUNTIME_PROFILE* monsterProfile =
			spawnBootstrap.Find_Profile("MONSTER_VALTAN_PADD_01");
		const MONSTER_RUNTIME_PROFILE* minibossProfile =
			spawnBootstrap.Find_Profile("MINIBOSS_LUGARU");
		tests.Require(
			loaded && 1u == spawnBootstrap.Get_Revision() && 2u == groups.size() &&
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			nullptr != monsterAnchor && nullptr != minibossAnchor &&
			nullptr != monsterProfile && nullptr != minibossProfile,
			"Load two Character Select spawn groups, anchors, and profiles");

		const auto hasImmediateEntry = [](
			const SPAWN_GROUP_DEFINITION& group,
			const char* archetypeId,
			const char* anchorId)
		{
			return group.strRequiredCompletedGroupId.empty() &&
				1u == group.iMaxAlive && 1u == group.Waves.size() &&
				0u == group.Waves[0].iStartDelayMs &&
				1u == group.Waves[0].Entries.size() &&
				group.Waves[0].Entries[0].strArchetypeId == archetypeId &&
				1u == group.Waves[0].Entries[0].iCount &&
				group.Waves[0].Entries[0].strAnchorId == anchorId &&
				0u == group.Waves[0].Entries[0].iInitialDelayMs &&
				0u == group.Waves[0].Entries[0].iSpawnIntervalMs;
		};
		tests.Require(
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			hasImmediateEntry(
				*monsterGroup,
				"MONSTER_VALTAN_PADD_01",
				"anchor.character-select.monster") &&
			hasImmediateEntry(
				*minibossGroup,
				"MINIBOSS_LUGARU",
				"anchor.character-select.miniboss"),
			"Keep Character Select audition groups single-wave and zero-delay");

		CSpawnGroupRuntime immediateRuntime;
		std::string immediateStatus;
		std::uint32_t immediateSpawnCount = 0u;
		const bool immediateInitialized =
			immediateRuntime.Initialize(spawnBootstrap, immediateStatus);
		const bool failedImmediatePreservedDormant = immediateInitialized &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				[](const std::string&, const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&, std::uint32_t)
				{
					return false;
				}) &&
			!immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster");
		const auto countImmediateSpawn = [&immediateSpawnCount](
			const std::string&, const SPAWN_GROUP_ENTRY&,
			const SPAWN_GROUP_ANCHOR&, const MONSTER_RUNTIME_PROFILE&,
			std::uint32_t)
			{
				++immediateSpawnCount;
				return true;
			};
		tests.Require(
			failedImmediatePreservedDormant &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn) &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.miniboss",
				spawnBootstrap,
				countImmediateSpawn) &&
			2u == immediateSpawnCount &&
			immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn),
			"Commit immediate audition activation only after its spawn callback succeeds");

		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		const bool initialized =
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus);
		tests.Require(
			initialized &&
			spawnRuntime.Activate("spawn.character-select.monster") &&
			spawnRuntime.Activate("spawn.character-select.miniboss") &&
			spawnRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!spawnRuntime.Activate("spawn.character-select.monster"),
			"Activate both Character Select audition groups");
		std::array<std::uint32_t, 2> scheduledByGroup{};
		bool callbackContractValid = true;
		spawnRuntime.Update(
			1.f / 30.f,
			spawnBootstrap,
			[](const std::string&) { return 0u; },
			[&scheduledByGroup, &callbackContractValid](
				const std::string& spawnGroupId,
				const SPAWN_GROUP_ENTRY& entry,
				const SPAWN_GROUP_ANCHOR& anchor,
				const MONSTER_RUNTIME_PROFILE& profile,
				const std::uint32_t ordinal)
			{
				if (spawnGroupId == "spawn.character-select.monster")
				{
					++scheduledByGroup[0];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MONSTER_VALTAN_PADD_01" &&
						anchor.strAnchorId == "anchor.character-select.monster" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else if (spawnGroupId == "spawn.character-select.miniboss")
				{
					++scheduledByGroup[1];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MINIBOSS_LUGARU" &&
						anchor.strAnchorId == "anchor.character-select.miniboss" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else
				{
					callbackContractValid = false;
				}
				return true;
			});
		tests.Require(
			callbackContractValid && 1u == scheduledByGroup[0] &&
			1u == scheduledByGroup[1] &&
			2u == scheduledByGroup[0] + scheduledByGroup[1],
			"Schedule exactly two Character Select callbacks in the first update");

		CGameRoom resetRoom{ WORLD_ID::CHARACTER_SELECT_ARENA };
		SERVER_PLAYER resetPlayer{};
		resetPlayer.iSessionId = 501u;
		resetPlayer.iPlayerId = 502u;
		resetPlayer.iNetEntityId = 503u;
		resetPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		resetPlayer.strNickName = "ResetFixture";
		resetPlayer.iCurrentHp = 100u;
		resetPlayer.iMaximumHp = 100u;
		resetPlayer.isCombatReady = true;
		resetRoom.m_Players.emplace(resetPlayer.iPlayerId, resetPlayer);
		resetRoom.m_PlayerIdBySessionId.emplace(
			resetPlayer.iSessionId, resetPlayer.iPlayerId);
		resetRoom.m_PlayerIdByEntityId.emplace(
			resetPlayer.iNetEntityId, resetPlayer.iPlayerId);
		const bool resetGroupActivated =
			resetRoom.m_SpawnGroupRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				resetRoom.m_SpawnGroupBootstrap,
				[&resetRoom](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return resetRoom.Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				});
		const bool spawnedBeforeLeave = std::any_of(
			resetRoom.m_WorldEntities.begin(),
			resetRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return entity.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		resetRoom.Leave(
			resetPlayer.iSessionId,
			PLAYER_DESPAWN_REASON::DISCONNECTED);
		tests.Require(
			resetGroupActivated && spawnedBeforeLeave &&
			resetRoom.m_Players.empty() &&
			resetRoom.m_WorldEntities.empty() &&
			resetRoom.m_SpawnGroupRuntime.Activate(
				"spawn.character-select.monster"),
			"Reset Character Select dynamic entities and spawn groups after the room becomes empty");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		tests.Require(
			spawnBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == spawnBootstrap.Get_Groups().size(),
			"Load three authored Valtan spawn groups");
		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		tests.Require(
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus),
			"Initialize Valtan spawn group runtime");
		tests.Require(
			!spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Reject miniboss group before Stage 1 completion");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage01"),
			"Activate Stage 1 spawn group exactly once");
		std::uint32_t scheduledMonsterCount = 0;
		for (std::uint32_t step = 0; step < 64u &&
			!spawnRuntime.Is_Completed("spawn.valtan.stage01"); ++step)
		{
			spawnRuntime.Update(
				1.f,
				spawnBootstrap,
				[](const std::string&) { return 0u; },
				[&scheduledMonsterCount](const std::string&,
					const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&,
					const std::uint32_t)
				{
					++scheduledMonsterCount;
					return true;
				});
		}
		tests.Require(
			15u == scheduledMonsterCount &&
			spawnRuntime.Is_Completed("spawn.valtan.stage01"),
			"Schedule all Stage 1 waves and complete after all entities clear");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Unlock miniboss group after Stage 1 completion");
	}

	{
		auto makeDestructionGraph = [](
			const std::uint32_t breakingDurationTicks,
			const WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
			const bool hasWorldMutationChannels = true)
		{
			WORLD_DESTRUCTION_DESCRIPTOR_GRAPH graph{};
			graph.Groups.push_back({
				"destroyable.group.valtan.wall.3705102",
				{
					"deploy.valtan.wall.3705102.0",
					"deploy.valtan.wall.3705102.1",
					"deploy.valtan.wall.3705102.2",
					"deploy.valtan.wall.3705102.3",
					"deploy.valtan.wall.3705102.4"
				},
				WORLD_DESTRUCTION_STATE::INTACT });
			graph.Mutations.push_back({
				"mutation.valtan.wall.3705102.fracture",
				"destroyable.group.valtan.wall.3705102",
				WORLD_DESTRUCTION_STATE::FRACTURED,
				breakingDurationTicks,
				hasWorldMutationChannels ?
					"collision.valtan.wall.3705102.fractured" : "",
				hasWorldMutationChannels ?
					"navigation.valtan.wall.3705102.open" : "" });
			graph.Bindings.push_back({
				"binding.valtan.wall.3705102.impact",
				"mutation.valtan.wall.3705102.fracture",
				triggerKind,
				"VALTAN_ARMOR_BREAK_OPENING",
				"WALL_CHARGE",
				"valtan.armor_break.wall_charge",
				1u,
				WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT == triggerKind ?
					"receiver.valtan.wall.3705102" : "" });
			return graph;
		};

		const WORLD_DESTRUCTION_ACTION_TUPLE exactAction{
			"VALTAN_ARMOR_BREAK_OPENING",
			"WALL_CHARGE",
			"valtan.armor_break.wall_charge",
			1u };
		constexpr std::uint64_t SOURCE_BOSS_ENTITY_ID = 7001u;
		CWorldDestructionRuntime runtime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		tests.Require(
			runtime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			1u == runtime.Get_EncounterEpoch(),
			"Initialize a nonzero-epoch world destruction graph transactionally");

		WORLD_DESTRUCTION_ACTION_TUPLE wrongAction = exactAction;
		wrongAction.strActionId = "valtan.armor_break.wall_charge.wrong";
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					wrongAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			transaction.Transitions.empty() &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.other",
					SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status),
			"Reject non-exact action tuples and impact receivers");

		const bool preparedBreaking =
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			runtime.Prepare_ImpactTrigger(
				exactAction, "receiver.valtan.wall.3705102",
				SOURCE_BOSS_ENTITY_ID, 7u, 10u, transaction, status) &&
			1u == transaction.BindingApplications.size() &&
			1u == transaction.Transitions.size() &&
			WORLD_DESTRUCTION_STATE::INTACT ==
				transaction.Transitions.front().ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING ==
				transaction.Transitions.front().eNextState &&
			13u == transaction.Transitions.front().iCommitTick &&
			!transaction.Transitions.front().bApplyPersistentMutation &&
			5u == transaction.Transitions.front().MemberPlacementIds.size() &&
			transaction.Transitions.front().strCollisionStateId ==
				"collision.valtan.wall.3705102.fractured" &&
			transaction.Transitions.front().strNavigationStateId ==
				"navigation.valtan.wall.3705102.open";
		tests.Require(
			preparedBreaking && runtime.Commit(transaction, status),
			"Prepare and atomically commit one BREAKING transition");

		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			2u == groupState.iStateVersion && 10u == groupState.iStateStartTick &&
			13u == groupState.iCommitTick &&
			groupState.strPendingMutationId ==
				"mutation.valtan.wall.3705102.fracture",
			"Persist the BREAKING version and exact final commit tick");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 11u, transaction, status) &&
			transaction.Transitions.empty(),
			"Treat the same pattern-sequence binding request as an idempotent no-op");
		tests.Require(
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				runtime.Prepare_DueStateCommits(12u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_DueStateCommits(13u, transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			runtime.Commit(transaction, status),
			"Commit the persistent wall, collision, and navigation plan at the exact tick");
		tests.Require(
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			3u == groupState.iStateVersion && 13u == groupState.iStateStartTick &&
			0u == groupState.iCommitTick,
			"Converge on a persistent FRACTURED state with one final version");

		const std::uint32_t previousEpoch = runtime.Get_EncounterEpoch();
		tests.Require(
			runtime.Reset(status) &&
			previousEpoch + 1u == runtime.Get_EncounterEpoch() &&
			runtime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::INTACT == groupState.eState &&
			1u == groupState.iStateVersion && 1u == groupState.iStateStartTick &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				runtime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 7u, 20u, transaction, status),
			"Reset to a new epoch/version baseline and admit the same sequence in the new encounter");
		tests.Require(
			1u == runtime.Get_GroupStates().size() &&
			"destroyable.group.valtan.wall.3705102" ==
				runtime.Get_GroupStates().front().strGroupId,
			"Enumerate persistent group state in canonical stable-ID order");

		CWorldDestructionRuntime zeroDurationRuntime;
		tests.Require(
			zeroDurationRuntime.Initialize(makeDestructionGraph(
				0u, WORLD_DESTRUCTION_TRIGGER_KIND::STAGE, false), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				zeroDurationRuntime.Prepare_StageTrigger(
					exactAction, SOURCE_BOSS_ENTITY_ID, 1u, 30u,
					transaction, status) &&
			WORLD_DESTRUCTION_STATE::FRACTURED ==
				transaction.Transitions.front().eNextState &&
			30u == transaction.Transitions.front().iCommitTick &&
			transaction.Transitions.front().strCollisionStateId.empty() &&
			transaction.Transitions.front().strNavigationStateId.empty() &&
			transaction.Transitions.front().bApplyPersistentMutation &&
			zeroDurationRuntime.Commit(transaction, status) &&
			zeroDurationRuntime.Find_GroupState(
				"destroyable.group.valtan.wall.3705102", groupState) &&
			WORLD_DESTRUCTION_STATE::FRACTURED == groupState.eState &&
			2u == groupState.iStateVersion,
			"Commit a zero-duration stage binding directly in one version");

		CWorldDestructionRuntime wrapRuntime;
		const std::uint32_t beforeWrap =
			(std::numeric_limits<std::uint32_t>::max)() - 1u;
		tests.Require(
			wrapRuntime.Initialize(makeDestructionGraph(
				3u, WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_ImpactTrigger(
					exactAction, "receiver.valtan.wall.3705102",
					SOURCE_BOSS_ENTITY_ID, 1u, beforeWrap,
					transaction, status) &&
			2u == transaction.Transitions.front().iCommitTick &&
			wrapRuntime.Commit(transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(
					(std::numeric_limits<std::uint32_t>::max)(),
					transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE ==
				wrapRuntime.Prepare_DueStateCommits(1u, transaction, status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				wrapRuntime.Prepare_DueStateCommits(2u, transaction, status) &&
			wrapRuntime.Commit(transaction, status),
			"Skip reserved tick zero and commit exactly after uint32 wrap");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7001u;
		boss.strPatternId = "VALTAN_ARENA_BREAK_109";
		boss.strPatternStageId = "IMPACT";
		boss.strActionId =
			"valtan.mechanic.arena-break-109.impact";
		boss.iPatternStageIndex = 2u;
		boss.iPatternSequence = 15u;
		boss.fPositionX = 151.25f;
		boss.fPositionY = 22.97f;
		boss.fPositionZ = -121.75f;
		boss.fYawDegrees = 90.f;

		const bool applied = room.Is_Ready() &&
			room.Apply_WorldDestructionStageEntry(boss, 450u);
		const auto breakingStates =
			room.m_WorldDestructionRuntime.Get_GroupStates();
		const std::size_t breakingCount = static_cast<std::size_t>(
			std::count_if(breakingStates.begin(), breakingStates.end(),
				[](const WORLD_DESTRUCTION_GROUP_STATE& state)
				{
					return WORLD_DESTRUCTION_STATE::BREAKING == state.eState;
				}));
		tests.Require(
			applied && 21u == breakingCount &&
			22u == room.m_iNextWorldDestructionEventSequence,
			"Emit one monotonically sequenced live event for every 109-bar wall group");

		const std::uint64_t sequenceAfterFirstEdge =
			room.m_iNextWorldDestructionEventSequence;
		tests.Require(
			room.Apply_WorldDestructionStageEntry(boss, 451u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Do not allocate a live event for a duplicate pattern-stage edge");

		WORLD_DESTRUCTION_ACTION_TUPLE action{};
		action.strPatternId = boss.strPatternId;
		action.strStageId = boss.strPatternStageId;
		action.strActionId = boss.strActionId;
		action.iStageIndex = boss.iPatternStageIndex;
		CWorldDestructionRuntime isolatedRuntime;
		std::string status;
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		const bool prepared = isolatedRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				isolatedRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		CWorldDestructionRuntime activeRuntime =
			std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(isolatedRuntime);
		room.m_iNextWorldDestructionEventSequence = 1u;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> firstEvents;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> repeatedEvents;
		const bool builtFirst = prepared &&
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, firstEvents, status);
		const bool builtRepeated =
			room.Build_WorldDestructionLiveEvents(
				transaction, boss, repeatedEvents, status);
		room.m_WorldDestructionRuntime = std::move(activeRuntime);
		tests.Require(
			builtFirst && builtRepeated && 21u == firstEvents.size() &&
			firstEvents.size() == repeatedEvents.size() &&
			1u == firstEvents.front().iEventSequence &&
			21u == firstEvents.back().iEventSequence &&
			firstEvents.front().iRandomSeed ==
				repeatedEvents.front().iRandomSeed &&
			firstEvents.front().fImpactOriginX == boss.fPositionX &&
			firstEvents.front().fImpactOriginY == boss.fPositionY &&
			firstEvents.front().fImpactOriginZ == boss.fPositionZ &&
			std::fabs(firstEvents.front().fImpactDirectionX - 1.f) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionY) <= 0.001f &&
			std::fabs(firstEvents.front().fImpactDirectionZ) <= 0.001f,
			"Build canonical deterministic events from the authoritative boss pose");

		room.m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() - 9u;
		std::vector<WORLD_DESTRUCTION_EVENT_WIRE> exhaustedEvents;
		CWorldDestructionRuntime exhaustionRuntime;
		const bool preparedExhaustion = exhaustionRuntime.Initialize(
			room.m_WorldDestructionBootstrap.Get_DescriptorGraph(), status) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				exhaustionRuntime.Prepare_StageTrigger(
					action, boss.iNetEntityId, boss.iPatternSequence,
					450u, transaction, status);
		activeRuntime = std::move(room.m_WorldDestructionRuntime);
		room.m_WorldDestructionRuntime = std::move(exhaustionRuntime);
		tests.Require(
			preparedExhaustion &&
			!room.Build_WorldDestructionLiveEvents(
				transaction, boss, exhaustedEvents, status) &&
			exhaustedEvents.empty(),
			"Fail closed before a destruction live-event sequence can wrap");
		room.m_WorldDestructionRuntime = std::move(activeRuntime);

		room.m_iNextWorldDestructionEventSequence = sequenceAfterFirstEdge;
		tests.Require(
			room.Commit_DueWorldDestruction(458u) &&
			sequenceAfterFirstEdge ==
				room.m_iNextWorldDestructionEventSequence,
			"Commit due FRACTURED states without emitting a second live event");
		const std::uint32_t previousEpoch =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			previousEpoch + 1u ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			1u == room.m_iNextWorldDestructionEventSequence,
			"Reset the room live-event ledger only with the encounter epoch");
	}

	{
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 7002u;
		boss.strPatternId = "VALTAN_ARMOR_BREAK_OPENING";
		boss.strPatternStageId = "WALL_CHARGE";
		boss.strActionId =
			"valtan.mechanic.armor-break-opening.charge";
		boss.iPatternStageIndex = 0u;
		boss.iPatternSequence = 159u;
		boss.fPositionX = 151.f;
		boss.fPositionY = 23.04f;
		boss.fPositionZ = -133.312236f;
		boss.fYawDegrees = 90.f;
		bool triggered = false;
		WORLD_DESTRUCTION_GROUP_STATE groupState{};
		std::vector<SERVER_NAV_POINT> wallPassagePath;
		tests.Require(
			room.Is_Ready() &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			!room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) &&
			room.Apply_WorldDestructionImpact(
				boss, "collision.valtan.wallgroup.11047903315509031966.receiver",
				500u, triggered) && triggered &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.deploy.11047903315509031966",
				groupState) &&
			WORLD_DESTRUCTION_STATE::BREAKING == groupState.eState &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Commit one exact Valtan impact while keeping BREAKING navigation blocked");
		SERVER_BOSS_RECEIVER_HIT duplicateHit{};
		tests.Require(
			!room.m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
				145.f, 23.04f, -133.312236f,
				175.f, 23.04f, -133.312236f,
				1.f, duplicateHit) &&
			room.Commit_DueWorldDestruction(507u) &&
			!room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f),
			"Suppress repeated receiver impacts and keep the wall closed before its due tick");
		tests.Require(
			room.Commit_DueWorldDestruction(508u) &&
			room.m_WorldDestructionRuntime.Find_GroupState(
				"destroyable.group.valtan.deploy.11047903315509031966",
				groupState) &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == groupState.eState &&
			room.m_ServerNavigation.Is_PointWalkableExact(
				161.402061f, -133.312236f) &&
			room.m_ServerNavigation.Find_Path(
				160.25f, -130.75f, 162.25f, -135.75f,
				wallPassagePath) && !wallPassagePath.empty(),
			"Atomically expose the Valtan passage at the persistent DESPAWNED due tick");
	}

#ifdef _DEBUG
	{
		/* Entering Stage_Boss in Debug is the complete manual smoke route: the
		canonical trigger still activates the boss, the player is placed at the
		wall-charge bait point, and the next brain tick queues only 159. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		SERVER_PLAYER player{};
		player.iSessionId = 606u;
		player.iPlayerId = 607u;
		player.iNetEntityId = 608u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 5500u;
		player.iMaximumHp = 5500u;
		player.isCombatReady = true;
		player.fPositionX = 130.181f;
		player.fPositionY = 23.0607529f;
		player.fPositionZ = -95.8730011f;
		room.m_Players.emplace(player.iPlayerId, player);
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const SERVER_PLAYER& baitPlayer = room.m_Players.begin()->second;
		float baitResolvedX = 0.f;
		float baitResolvedY = 0.f;
		float baitResolvedZ = 0.f;
		bool baitBlocked = false;
		const bool baitIsCollisionClear =
			room.m_ServerCollisionSystem.Resolve_PlayerMove(
				baitPlayer,
				baitPlayer.fPositionX + 0.01f,
				baitPlayer.fPositionY,
				baitPlayer.fPositionZ,
				baitResolvedX, baitResolvedY, baitResolvedZ, baitBlocked) &&
			!baitBlocked;
		tests.Require(
			room.Is_Ready() && nullptr != boss &&
			"VALTAN_ARMOR_BREAK_OPENING" ==
				(nullptr == boss ? std::string{} : boss->strPatternId) &&
			159u == (nullptr == boss ? 0u :
				CValtanBrain::Calculate_HealthBar(*boss)) &&
			std::abs(baitPlayer.fPositionX - 154.296f) < 0.001f &&
			std::abs(baitPlayer.fPositionZ + 125.219f) < 0.001f &&
			baitIsCollisionClear,
			"Enter the real Debug Stage_Boss trigger and immediately run the 159-bar opening");
	}
#endif

	{
		/* Debug Valtan audition. The point of the two-step ARM/CROSS contract is
		that dropping straight onto a low bar crosses every threshold above it,
		so this checks the single-crossing property against the real encounter
		patterns rather than a synthetic pair. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr SESSION_ID AUDITION_SESSION = 4242u;
		constexpr PLAYER_ID AUDITION_PLAYER = 77u;
		constexpr std::uint32_t TARGET_BAR = 109u;

		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* auditionBoss = room.Find_AuditionBoss();
		tests.Require(
			activated && nullptr != auditionBoss &&
			60000u == (nullptr == auditionBoss ? 0u : auditionBoss->iMaximumHp) &&
			160u == (nullptr == auditionBoss ?
				0u : auditionBoss->iMaximumHealthBars),
			"Activate the audition Valtan with its authored health bar scale");

		tests.Require(
			nullptr != auditionBoss &&
			30375u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 81u) &&
			30000u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 80u) &&
			0u == CValtanBrain::Resolve_HealthBarHp(*auditionBoss, 161u),
			"Resolve authored health bar boundaries and reject bars off the scale");

		C2S_VALTAN_AUDITION_REQUEST arm{};
		arm.iRequestSequence = 1u;
		arm.eOperation = VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		arm.iTargetHealthBar = TARGET_BAR;
		std::uint32_t reportedBar = 0u;

		SERVER_PLAYER auditionPlayer{};
		auditionPlayer.iPlayerId = AUDITION_PLAYER;
		auditionPlayer.iNetEntityId = 900u;
		auditionPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		auditionPlayer.iCurrentHp = 1000u;
		auditionPlayer.iMaximumHp = 1000u;
		/* SERVER_PLAYER starts combat-ready, so the not-engaged path has to be
		asked for explicitly rather than left to the default. */
		auditionPlayer.isCombatReady = false;
		if (nullptr != auditionBoss)
		{
			auditionPlayer.fPositionX = auditionBoss->fPositionX + 2.f;
			auditionPlayer.fPositionY = auditionBoss->fPositionY;
			auditionPlayer.fPositionZ = auditionBoss->fPositionZ;
		}
		room.m_Players.emplace(AUDITION_PLAYER, auditionPlayer);

#ifndef _DEBUG
		/* A Release Server never auditions. It still answers, because the packet
		type stays known so a Debug Client gets a verdict instead of a closed
		socket, but the boss must not move even for an otherwise valid request. */
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;
		const std::uint32_t releaseHpBefore =
			nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp;
		reportedBar = 12345u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, arm, reportedBar) &&
			0u == reportedBar &&
			nullptr != auditionBoss &&
			releaseHpBefore ==
				(nullptr == auditionBoss ? 1u : auditionBoss->iCurrentHp),
			"Reject every Valtan audition in a Release Server without moving the boss");
#else
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar),
			"Reject a Valtan audition from a session that never joined the room");
		room.m_PlayerIdBySessionId.emplace(AUDITION_SESSION, AUDITION_PLAYER);

		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar),
			"Reject a Valtan audition the brain would drop for want of a target");
		room.m_Players.at(AUDITION_PLAYER).isCombatReady = true;

		C2S_VALTAN_AUDITION_REQUEST cross = arm;
		cross.iRequestSequence = 2u;
		cross.eOperation = VALTAN_AUDITION_OPERATION::CROSS_HEALTH_BAR;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, cross, reportedBar),
			"Reject a Valtan crossing that was never armed at the same bar");

		C2S_VALTAN_AUDITION_REQUEST unknownBar = arm;
		unknownBar.iRequestSequence = 3u;
		unknownBar.iTargetHealthBar = 79u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, unknownBar, reportedBar),
			"Reject a Valtan audition on a bar that carries no authored pattern");

		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 4u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		play.iTargetHealthBar = 33u;
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, play, reportedBar) &&
			33u == reportedBar && nullptr != auditionBoss &&
			34u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar) &&
			12375u == (nullptr == auditionBoss ?
				0u : auditionBoss->iCurrentHp),
			"Prime and cross one authored bar atomically for one-click audition");

		arm.iRequestSequence = 5u;
		const VALTAN_AUDITION_RESULT armResult =
			room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::ARMED == armResult &&
			110u == reportedBar &&
			nullptr != auditionBoss &&
			41250u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp) &&
			110u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar),
			"Arm the audition one bar above the target without crossing it");

		tests.Require(
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED ==
				room.Evaluate_ValtanAudition(AUDITION_SESSION, arm, reportedBar) &&
			nullptr != auditionBoss &&
			41250u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp),
			"Answer a resent Valtan audition sequence without moving the boss");

		cross.iRequestSequence = 6u;
		const VALTAN_AUDITION_RESULT crossResult =
			room.Evaluate_ValtanAudition(AUDITION_SESSION, cross, reportedBar);
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED == crossResult &&
			TARGET_BAR == reportedBar &&
			nullptr != auditionBoss &&
			40875u == (nullptr == auditionBoss ? 0u : auditionBoss->iCurrentHp) &&
			110u == (nullptr == auditionBoss ?
				0u : auditionBoss->iLastEvaluatedHealthBar) &&
			(nullptr == auditionBoss || auditionBoss->PendingPatternIds.empty()),
			"Cross onto the target bar and leave the queueing to CValtanBrain");

		CValtanBrain auditionBrain;
		std::vector<DAMAGE_EVENT> auditionDamage;
		if (nullptr != auditionBoss)
		{
			auditionBrain.Update(
				*auditionBoss, room.m_Players, room.m_GameplayCatalog,
				room.m_ServerNavigation, 1.f / 30.f, 500u, auditionDamage);
		}
		const bool queuedOnlyTarget = nullptr != auditionBoss &&
			1u == auditionBoss->TriggeredPatternIds.size() &&
			"VALTAN_ARENA_BREAK_109" == auditionBoss->TriggeredPatternIds.front();
		tests.Require(
			queuedOnlyTarget,
			"Queue only the 109-bar pattern from an armed single-bar crossing");
		tests.Require(
			nullptr != auditionBoss &&
			auditionBoss->TriggeredPatternIds.end() == std::find_if(
				auditionBoss->TriggeredPatternIds.begin(),
				auditionBoss->TriggeredPatternIds.end(),
				[](const std::string& patternId)
				{
					return "VALTAN_ARMOR_BREAK_OPENING" == patternId ||
						"VALTAN_FLOOR_WIPE_130" == patternId ||
						"VALTAN_FOUR_PILLARS_105" == patternId;
				}),
			"Leave the 159, 130 and 105 bar patterns unqueued by a 109-bar audition");
		tests.Require(
			nullptr != auditionBoss &&
			"VALTAN_ARENA_BREAK_109" ==
				(nullptr == auditionBoss ? std::string{} :
					auditionBoss->strPatternId) &&
			1u == (nullptr == auditionBoss ? 0u : auditionBoss->iPatternSequence),
			"Advance the audition pattern sequence exactly once");

		C2S_VALTAN_AUDITION_REQUEST whileRunning = arm;
		whileRunning.iRequestSequence = 7u;
		whileRunning.iTargetHealthBar = 33u;
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, whileRunning, reportedBar),
			"Reject a Valtan audition while an authored pattern is still running");

		const std::uint32_t epochBeforeRepeatPlay =
			room.m_WorldDestructionRuntime.Get_EncounterEpoch();
		C2S_VALTAN_AUDITION_REQUEST repeatPlay{};
		repeatPlay.iRequestSequence = 8u;
		repeatPlay.eOperation = VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR;
		repeatPlay.iTargetHealthBar = TARGET_BAR;
		tests.Require(
			VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(
					AUDITION_SESSION, repeatPlay, reportedBar) &&
			epochBeforeRepeatPlay + 1u ==
				room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
			TARGET_BAR == reportedBar && nullptr != auditionBoss &&
			auditionBoss->strPatternId.empty() &&
			auditionBoss->TriggeredPatternIds.empty() &&
			110u == auditionBoss->iLastEvaluatedHealthBar &&
			40875u == auditionBoss->iCurrentHp,
			"Reset a running audition and queue 109 again from one repeatable Play request");
#endif

		room.m_Players.clear();
		room.m_PlayerIdBySessionId.clear();
		tests.Require(
			room.Reset_ValtanArenaWhenEmpty() &&
			0u == room.m_iValtanAuditionArmedHealthBar,
			"Drop the armed audition bar with the encounter reset");
	}

	{
		/* The 109 leap. No jump clip exists in the converted Valtan model, so
		the arc itself is Server state and has to be checked as authority, not
		as presentation: it must leave the ground during TAKEOFF and land back
		exactly on the authored placement by IMPACT. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr PLAYER_ID LEAP_PLAYER = 78u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* leapBoss = room.Find_AuditionBoss();

		SERVER_PLAYER leapPlayer{};
		leapPlayer.iPlayerId = LEAP_PLAYER;
		leapPlayer.iNetEntityId = 901u;
		leapPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		leapPlayer.iCurrentHp = 1000u;
		leapPlayer.iMaximumHp = 1000u;
		leapPlayer.isCombatReady = true;
		if (nullptr != leapBoss)
		{
			leapPlayer.fPositionX = leapBoss->fPositionX + 2.f;
			leapPlayer.fPositionY = leapBoss->fPositionY;
			leapPlayer.fPositionZ = leapBoss->fPositionZ;
		}
		room.m_Players.emplace(LEAP_PLAYER, leapPlayer);

		const float groundY =
			nullptr == leapBoss ? 0.f : leapBoss->fSpawnPositionY;
		tests.Require(
			activated && nullptr != leapBoss &&
			std::abs(groundY - (nullptr == leapBoss ?
				1.f : leapBoss->fPositionY)) < 0.001f,
			"Record the authored Valtan placement as the leap landing point");

		/* Drive the real pattern rather than assigning stages by hand, so the
		arc is exercised through the same edges the room replicates. */
		if (nullptr != leapBoss)
		{
			leapBoss->fPositionX = leapBoss->fSpawnPositionX + 6.f;
			leapBoss->fPositionZ = leapBoss->fSpawnPositionZ + 6.f;
			leapBoss->iCurrentHp =
				CValtanBrain::Resolve_HealthBarHp(*leapBoss, 109u);
			leapBoss->iLastEvaluatedHealthBar = 110u;
		}
		CValtanBrain leapBrain;
		std::vector<DAMAGE_EVENT> leapDamage;
		std::uint32_t leapTick = 600u;
		const auto tickLeap = [&](const std::uint32_t count)
		{
			for (std::uint32_t index = 0u; index < count; ++index)
			{
				if (nullptr == leapBoss)
					return;
				leapBrain.Update(
					*leapBoss, room.m_Players, room.m_GameplayCatalog,
					room.m_ServerNavigation, 1.f / 30.f, leapTick++, leapDamage);
			}
		};

		tickLeap(1u);
		tests.Require(
			nullptr != leapBoss &&
			"VALTAN_ARENA_BREAK_109" ==
				(nullptr == leapBoss ? std::string{} : leapBoss->strPatternId) &&
			"TAKEOFF" == (nullptr == leapBoss ?
				std::string{} : leapBoss->strPatternStageId),
			"Begin the 109 phase transition on the authored crossing");

		/* Half of the 1000ms TAKEOFF stage at 30Hz. */
		tickLeap(15u);
		tests.Require(
			nullptr != leapBoss &&
			leapBoss->fPositionY > groundY + 1.f,
			"Lift Valtan off the ground during the authored TAKEOFF stage");

		/* Follow the rest of TAKEOFF and all of DROP one tick at a time so the
		arc itself is checked, not just its endpoints: it has to reach the
		authored apex and come all the way back down to the floor. */
		float peakY = nullptr == leapBoss ? 0.f : leapBoss->fPositionY;
		float peakPlanarError = 0.f;
		for (std::uint32_t index = 0u; index < 39u; ++index)
		{
			tickLeap(1u);
			if (nullptr == leapBoss)
				break;
			if (leapBoss->fPositionY > peakY)
				peakY = leapBoss->fPositionY;
			if ("DROP" == leapBoss->strPatternStageId)
			{
				const float dx = leapBoss->fPositionX - leapBoss->fSpawnPositionX;
				const float dz = leapBoss->fPositionZ - leapBoss->fSpawnPositionZ;
				peakPlanarError = (std::max)(
					peakPlanarError, std::sqrt(dx * dx + dz * dz));
			}
		}
		tests.Require(
			nullptr != leapBoss &&
			std::abs(peakY - (groundY + 12.f)) < 0.5f &&
			peakPlanarError > 1.f &&
			leapBoss->fPositionY <= groundY + 0.001f,
			"Carry Valtan through the authored apex and back down to the floor");

		/* TAKEOFF (30 ticks) plus DROP (24) lands inside the 12-tick IMPACT. */
		tickLeap(1u);
		const bool landedExactly = nullptr != leapBoss &&
			"IMPACT" == leapBoss->strPatternStageId &&
			std::abs(leapBoss->fPositionX - leapBoss->fSpawnPositionX) < 0.001f &&
			std::abs(leapBoss->fPositionY - leapBoss->fSpawnPositionY) < 0.001f &&
			std::abs(leapBoss->fPositionZ - leapBoss->fSpawnPositionZ) < 0.001f;
		tests.Require(
			landedExactly,
			"Land the 109 leap exactly on the authored placement at IMPACT");
	}

	{
		/* The 109 outer ring encloses the arena, but it must not quietly wall
		off the two floors the level already depends on: the corridor the
		player walks in through, and the passage the 159-bar wall opens. Both
		bearings were measured against the published navgrid. */
		CGameRoom room{ WORLD_ID::VALTAN_ARENA };
		constexpr float ARENA_CENTER_X = 156.03f;
		constexpr float ARENA_CENTER_Z = -122.06f;
		/* 131 degrees is the mouth of the entrance corridor: no interior
		collision box reaches past the arena floor there, so the ring alone
		decides that sweep. The 159 passage is covered by the FRACTURED
		collision test above, where interior boxes still reach out to 17.5. */
		const auto sweepAcrossRing = [&](const float bearingDegrees)
		{
			const float radians = bearingDegrees * 3.14159265f / 180.f;
			SERVER_PLAYER walker{};
			walker.fPositionX = ARENA_CENTER_X + std::cos(radians) * 15.f;
			walker.fPositionY = 23.04f;
			walker.fPositionZ = ARENA_CENTER_Z + std::sin(radians) * 15.f;
			float resolvedX = 0.f;
			float resolvedY = 0.f;
			float resolvedZ = 0.f;
			bool blocked = false;
			const bool resolved = room.m_ServerCollisionSystem.Resolve_PlayerMove(
				walker,
				ARENA_CENTER_X + std::cos(radians) * 21.f,
				walker.fPositionY,
				ARENA_CENTER_Z + std::sin(radians) * 21.f,
				resolvedX, resolvedY, resolvedZ, blocked);
			return resolved && blocked;
		};
		tests.Require(
			room.Is_Ready() && sweepAcrossRing(0.f) && sweepAcrossRing(60.f) &&
			sweepAcrossRing(216.f),
			"Block outward movement through the intact 109 outer ring");
		tests.Require(
			room.Is_Ready() && !sweepAcrossRing(131.f),
			"Leave the authored entrance corridor open through the 109 ring");

		/* Every wall that stands on authored floor now owns a blocker region,
		so pathfinding stops at it instead of walking through, and the boss
		spawn stays reachable because its cells were kept out of them. */
		std::vector<SERVER_NAV_POINT> wallPath;
		const bool bossActivated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		const SERVER_WORLD_ENTITY* spawnedBoss = room.Find_AuditionBoss();
		std::vector<SERVER_NAV_POINT> bossPath;
		tests.Require(
			bossActivated && nullptr != spawnedBoss &&
			room.m_ServerNavigation.Find_Path(
				144.75f, -115.75f, 156.25f, -122.25f, wallPath) &&
			!wallPath.empty() &&
			room.m_ServerNavigation.Find_Path(
				spawnedBoss->fSpawnPositionX, spawnedBoss->fSpawnPositionZ,
				156.25f, -122.25f, bossPath) &&
			!bossPath.empty(),
			"Keep the arena and the Valtan spawn connected under the wall blockers");
	}

	tests.failures += Run_WorldDestructionBootstrapContractTests();
	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
