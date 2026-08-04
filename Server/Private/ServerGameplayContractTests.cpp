#include "ServerGameplayContractTests.h"

#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"

#include <algorithm>
#include <array>
#include <iostream>
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
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34120, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34080, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38020, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38050, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45050, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31210, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31230, "W" }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable Q/W skill binding");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		quickPlayer.iCurrentResource = 100;
		quickPlayer.iMaximumResource = 100;
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
			"Approve playable Q/W skill command");
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
	tests.Require(650u == catalog.Find_Damage("damage.player.34120"),
		"Resolve player damage profile");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(navigation.Find_Path(152.f, -137.f, 151.f, -122.f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

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

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.iCurrentResource = 100;
	player.iMaximumResource = 100;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	boss.fPositionZ = -122.f;
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
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, 1.f / 30.f, tick);
	tests.Require(9350u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage once");
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
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
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
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the next combo stage once the hit has landed");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr, 1.f / 30.f, tick);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	valtan.strDamageProfileId = "damage.valtan.basic-swing";
	valtan.iCurrentHp = 10000;
	valtan.iMaximumHp = 10000;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fPatternMaximumRange = 8.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	valtan.iPatternTelegraphMs = 1;
	valtan.iPatternActiveMs = 300;
	valtan.iPatternRecoveryMs = 1;
	CValtanBrain brain;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 100);
	brain.Update(valtan, players, catalog, navigation, 0.1f, 101);
	tests.Require(650u == players.begin()->second.iCurrentHp,
		"Apply server-authoritative Valtan damage once");
	valtan.iCurrentHp = 5000;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 102);
	tests.Require(2u == valtan.iPhase, "Advance Valtan phase from server HP");

	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
