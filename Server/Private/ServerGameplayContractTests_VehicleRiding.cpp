#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "VehicleCatalog.h"
#include "WorldBootstrap.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace
{
	constexpr VEHICLE_ID GOLDEN_TERPEION = 6705u;
	constexpr SESSION_ID RIDER_SESSION = 11u;
	constexpr PLAYER_ID RIDER_PLAYER = 1u;

	C2S_SET_VEHICLE_RIDING Make_VehicleRequest(
		const std::uint32_t sequence, const WORLD_ID worldId, const VEHICLE_ID vehicleId)
	{
		C2S_SET_VEHICLE_RIDING request{};
		request.iRequestSequence = sequence;
		request.eWorldId = worldId;
		request.iVehicleId = vehicleId;
		return request;
	}

	bool Near(const float value, const float expected)
	{
		return std::abs(value - expected) < 0.0001f;
	}
}

int LostArk::Server::Run_ServerVehicleRidingContractTests()
{
	TESTS tests;
	auto bern = std::make_unique<CGameRoom>(WORLD_ID::BERN);
	tests.Require(bern->Is_Ready(), "Bern room loads the published vehicle bootstrap");
	if (!bern->Is_Ready())
	{
		std::cout << bern->Get_Status() << '\n';
		return 1;
	}
	const SERVER_VEHICLE_DEFINITION* terpeion =
		bern->m_VehicleCatalog.Find_Vehicle(GOLDEN_TERPEION);
	tests.Require(nullptr != terpeion && Near(terpeion->fMoveSpeed, 5.f),
		"Golden Terpeion carries EFTable_Vehicle MoveSpeed 500 as 5 m/s");

	const auto& placements = bern->m_WorldBootstrap.Get_Placements();
	const auto spawn = std::find_if(placements.begin(), placements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
		});
	tests.Require(placements.end() != spawn, "Bern publishes an enabled player spawn");
	if (placements.end() == spawn)
		return 1;

	SERVER_PLAYER& rider = bern->m_Players[RIDER_PLAYER];
	rider.iPlayerId = RIDER_PLAYER;
	rider.iNetEntityId = 101u;
	rider.iSessionId = RIDER_SESSION;
	rider.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
	rider.eStance = PLAYER_STANCE_ID::WARLORD_NORMAL;
	rider.iCurrentHp = rider.iMaximumHp = 50000u;
	rider.fMoveSpeed = 2.8f;
	rider.fPositionX = spawn->fPositionX;
	rider.fPositionY = spawn->fPositionY;
	rider.fPositionZ = spawn->fPositionZ;
	bern->m_PlayerIdBySessionId[RIDER_SESSION] = RIDER_PLAYER;

	const S2C_SET_VEHICLE_RIDING_RESULT mounted = bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(1u, WORLD_ID::BERN, GOLDEN_TERPEION));
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == mounted.eResult &&
		GOLDEN_TERPEION == mounted.iActiveVehicleId &&
		GOLDEN_TERPEION == rider.iVehicleId &&
		Near(bern->Resolve_PlayerMoveSpeed(rider), 5.f),
		"Mounting in Bern commits the vehicle and moves at the vehicle speed");

	const S2C_SET_VEHICLE_RIDING_RESULT replayed = bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(1u, WORLD_ID::BERN, INVALID_VEHICLE_ID));
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == replayed.eResult &&
		GOLDEN_TERPEION == rider.iVehicleId,
		"A replayed sequence answers the committed verdict without applying its payload");

	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(2u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult,
		"Mounting the vehicle already ridden is a typed no-op");

	C2S_USE_SKILL attack{};
	attack.iClientSequence = 1u;
	attack.iSkillId = 17000u;
	attack.fAimX = rider.fPositionX + 1.f;
	attack.fAimZ = rider.fPositionZ;
	bern->Handle_UseSkill(RIDER_SESSION, attack);
	tests.Require(PLAYER_ACTION_STATE::NONE == rider.eAction &&
		GOLDEN_TERPEION == rider.iVehicleId && 0u == rider.iLastSkillSequence,
		"A skill request while mounted is refused before the skill system runs");

	constexpr float TICK_SECONDS = 1.f / 30.f;
	const auto runVehicleSkill = [&](const std::uint32_t maximumTicks)
	{
		for (std::uint32_t tick = 0u;
			tick < maximumTicks && PLAYER_ACTION_STATE::VEHICLE_SKILL == rider.eAction; ++tick)
		{
			bern->Update_VehicleSkill(rider, TICK_SECONDS);
		}
	};
	const SERVER_VEHICLE_SKILL* victoryPose = nullptr == terpeion ? nullptr : terpeion->Find_Skill(96000u);
	const SERVER_VEHICLE_SKILL* terpeionDash = nullptr == terpeion ? nullptr : terpeion->Find_Skill(96030u);
	tests.Require(nullptr != victoryPose && VEHICLE_SKILL_SLOT::Q == victoryPose->eSlot &&
		5000u == victoryPose->iCooldownMs && 2333u == victoryPose->iActionDurationMs &&
		nullptr != terpeionDash && VEHICLE_SKILL_SLOT::SPACE == terpeionDash->eSlot &&
		!terpeionDash->RootMotion.empty(),
		"Golden Terpeion publishes its EFTable_Vehicle skills with slot, cooldown, length and dash root motion");

	C2S_USE_SKILL vehicleSkill{};
	vehicleSkill.iClientSequence = 2u;
	vehicleSkill.iSkillId = 96000u;
	vehicleSkill.fAimX = rider.fPositionX;
	vehicleSkill.fAimZ = rider.fPositionZ + 1.f;
	bern->Handle_UseSkill(RIDER_SESSION, vehicleSkill);
	tests.Require(PLAYER_ACTION_STATE::VEHICLE_SKILL == rider.eAction &&
		96000u == rider.iCurrentSkillId && 0u != rider.iActionStartTick &&
		GOLDEN_TERPEION == rider.iVehicleId &&
		rider.CooldownEndTickBySkillId.count(96000u) == 1u,
		"A mounted press of the ridden vehicle's skill starts VEHICLE_SKILL and its cooldown");
	bern->Enforce_VehicleRidingState();
	tests.Require(GOLDEN_TERPEION == rider.iVehicleId,
		"A running vehicle skill keeps the player mounted");
	runVehicleSkill(120u);
	tests.Require(PLAYER_ACTION_STATE::NONE == rider.eAction &&
		INVALID_SKILL_ID == rider.iCurrentSkillId,
		"The vehicle skill ends at its authored length");

	vehicleSkill.iClientSequence = 3u;
	bern->Handle_UseSkill(RIDER_SESSION, vehicleSkill);
	tests.Require(PLAYER_ACTION_STATE::NONE == rider.eAction,
		"A vehicle skill on cooldown is refused");

	vehicleSkill.iClientSequence = 4u;
	vehicleSkill.iSkillId = 98520u;
	bern->Handle_UseSkill(RIDER_SESSION, vehicleSkill);
	tests.Require(PLAYER_ACTION_STATE::NONE == rider.eAction,
		"Another vehicle's skill is refused");

	const float dashStartX = rider.fPositionX;
	const float dashStartZ = rider.fPositionZ;
	vehicleSkill.iClientSequence = 5u;
	vehicleSkill.iSkillId = 96030u;
	bern->Handle_UseSkill(RIDER_SESSION, vehicleSkill);
	const bool dashStarted = PLAYER_ACTION_STATE::VEHICLE_SKILL == rider.eAction;
	runVehicleSkill(120u);
	const float dashDistance = std::sqrt(
		(rider.fPositionX - dashStartX) * (rider.fPositionX - dashStartX) +
		(rider.fPositionZ - dashStartZ) * (rider.fPositionZ - dashStartZ));
	tests.Require(dashStarted && PLAYER_ACTION_STATE::NONE == rider.eAction &&
		dashDistance > 0.5f && dashDistance <= 10.f,
		"The Space dash advances the rider by its root motion, clamped to walkable ground");

	vehicleSkill.iClientSequence = 6u;
	vehicleSkill.iSkillId = 96020u;
	bern->Handle_UseSkill(RIDER_SESSION, vehicleSkill);
	const bool kickStarted = PLAYER_ACTION_STATE::VEHICLE_SKILL == rider.eAction;
	tests.Require(kickStarted && VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(3u, WORLD_ID::BERN, INVALID_VEHICLE_ID)).eResult &&
		PLAYER_ACTION_STATE::NONE == rider.eAction && INVALID_VEHICLE_ID == rider.iVehicleId,
		"Dismounting ends a running vehicle skill");
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(4u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult,
		"The rider mounts again for the forced-dismount checks");
	rider.iLastSkillSequence = 0u;
	rider.CooldownEndTickBySkillId.clear();
	rider.LastVehicleRidingResult = {};

	rider.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
	bern->Enforce_VehicleRidingState();
	tests.Require(INVALID_VEHICLE_ID == rider.iVehicleId &&
		Near(bern->Resolve_PlayerMoveSpeed(rider), 2.8f),
		"A forced action dismounts before the snapshot and restores the class speed");
	rider.eAction = PLAYER_ACTION_STATE::NONE;

	bern->Handle_UseSkill(RIDER_SESSION, attack);
	tests.Require(0u != rider.iLastSkillSequence,
		"The same skill request reaches the skill system once the player is on foot");
	rider.eAction = PLAYER_ACTION_STATE::NONE;
	rider.iCurrentSkillId = INVALID_SKILL_ID;

	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(3u, WORLD_ID::BERN, 1u)).eResult,
		"An id outside the published bootstrap is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_STALE_SEQUENCE == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(2u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult,
		"An older sequence is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::REJECTED_WRONG_WORLD == bern->Apply_SetVehicleRiding(
		rider, Make_VehicleRequest(4u, WORLD_ID::VALTAN_ARENA, GOLDEN_TERPEION)).eResult,
		"A request addressed to another world is rejected");
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(5u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult &&
		VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			rider, Make_VehicleRequest(6u, WORLD_ID::BERN, INVALID_VEHICLE_ID)).eResult &&
		INVALID_VEHICLE_ID == rider.iVehicleId,
		"Dismount clears the vehicle");

	auto valtan = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
	tests.Require(valtan->Is_Ready(), "Valtan room loads");
	if (valtan->Is_Ready())
	{
		SERVER_PLAYER& raider = valtan->m_Players[RIDER_PLAYER];
		raider.iPlayerId = RIDER_PLAYER;
		raider.iCurrentHp = raider.iMaximumHp = 50000u;
		tests.Require(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED ==
			valtan->Apply_SetVehicleRiding(raider,
				Make_VehicleRequest(1u, WORLD_ID::VALTAN_ARENA, GOLDEN_TERPEION)).eResult &&
			INVALID_VEHICLE_ID == raider.iVehicleId,
			"A raid arena refuses to mount");
		raider.iVehicleId = GOLDEN_TERPEION;
		valtan->Enforce_VehicleRidingState();
		tests.Require(INVALID_VEHICLE_ID == raider.iVehicleId,
			"A raid arena never replicates a mounted player");
	}

	std::cout << "vehicle riding failures: " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
