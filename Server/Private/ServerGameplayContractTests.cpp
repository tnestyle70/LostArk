#include "ServerGameplayContractTests.h"

#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"

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
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	tests.Require(nullptr != catalog.Find_Skill(34060), "Resolve LanceMaster Q skill");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(650u == catalog.Find_Damage("damage.player.34060"),
		"Resolve player damage profile");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(navigation.Find_Path(141.f, -137.f, 151.f, -122.f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

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
	useSkill.iSkillId = 34060;
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
