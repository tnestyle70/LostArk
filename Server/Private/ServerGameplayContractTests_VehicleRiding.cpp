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

    // The same authority path drives all four phases; no Client transform is involved.
    const auto* sea = bern->m_VehicleCatalog.Find_Vehicle(ANCIENT_SEA_VEHICLE_ID);
    tests.Require(sea && sea->Has_Flight() && sea->fFlightMaximumHeight > sea->fFlightHoverHeight,
        "Ancient Sea loads published flight bounds and phase durations");
    if (sea && sea->Has_Flight())
    {
        tests.Require(bern->Apply_SetVehicleRiding(rider,
            Make_VehicleRequest(7u, WORLD_ID::BERN, ANCIENT_SEA_VEHICLE_ID)).eResult == VEHICLE_RIDING_RESULT::ACCEPTED,
            "Ancient Sea mounts through the existing authority contract");
        const float ground = rider.fPositionY;
        C2S_USE_SKILL toggle{}; toggle.iSkillId = 98523u; toggle.iClientSequence = 20u;
        tests.Require(bern->Try_StartVehicleSkill(rider, toggle) && rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::TAKEOFF,
            "E starts authoritative takeoff");
        const auto flyTicks = [&](unsigned count)
        {
            while (count--) { ++bern->m_iServerTick; bern->Update_VehicleSkill(rider, TICK_SECONDS); }
        };
        flyTicks(60u);
        tests.Require(rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::FLYING &&
            Near(rider.fPositionY, ground + sea->fFlightHoverHeight), "Takeoff becomes sustained flight at hover height");
        flyTicks(240u);
        tests.Require(rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::FLYING,
            "Flight continues past the original E action duration");
        C2S_MOVE input{}; input.eIntent = PLAYER_MOVE_INTENT::VEHICLE_FLIGHT;
        input.iClientSequence = rider.iLastMoveSequence + 1u; input.fVerticalInput = 1.f;
        bern->Handle_Move(RIDER_SESSION, input);
        flyTicks(6u);
        tests.Require(rider.fPositionY > ground + sea->fFlightHoverHeight, "Space input ascends on the Server");
        for (unsigned i = 0u; i < 220u; ++i)
        {
            ++input.iClientSequence; bern->Handle_Move(RIDER_SESSION, input); flyTicks(1u);
        }
        tests.Require(Near(rider.fPositionY, ground + sea->fFlightMaximumHeight), "Flight clamps at the authored altitude ceiling");
        const float top = rider.fPositionY;
        input.fVerticalInput = -1.f; ++input.iClientSequence;
        bern->Handle_Move(RIDER_SESSION, input); flyTicks(8u);
        tests.Require(rider.fPositionY < top, "Ctrl input descends without ending the wing loop");
        flyTicks(90u);
        const float stopped = rider.fPositionY; flyTicks(30u);
        tests.Require(std::abs(rider.fPositionY - stopped) < .001f,
            "Missing or focus-lost input expires instead of drifting indefinitely");
        const auto lastSequence = rider.iLastMoveSequence;
        input.fVerticalInput = 1.f; bern->Handle_Move(RIDER_SESSION, input);
        tests.Require(rider.iLastMoveSequence == lastSequence && rider.fVehicleFlightInputY == -1.f,
            "Replayed flight input cannot replace the last accepted intent");
        const float startX = rider.fPositionX, startZ = rider.fPositionZ;
        bool movedHorizontally = false;
        bool turnWasBounded = true;
        for (unsigned direction = 0u; direction < 4u && !movedHorizontally; ++direction)
        {
            input.fVerticalInput = 0.f;
            input.fGoalX = direction == 0u ? 1.f : (direction == 1u ? -1.f : 0.f);
            input.fGoalZ = direction == 2u ? 1.f : (direction == 3u ? -1.f : 0.f);
            for (unsigned tick = 0u; tick < 8u; ++tick)
            {
                ++input.iClientSequence; bern->Handle_Move(RIDER_SESSION, input);
                const float previousYaw = rider.fYawDegrees;
                flyTicks(1u);
                turnWasBounded &= std::abs(std::remainder(rider.fYawDegrees - previousYaw, 360.f)) <= 5.001f;
            }
            movedHorizontally = std::hypot(rider.fPositionX - startX, rider.fPositionZ - startZ) > .02f;
        }
        tests.Require(movedHorizontally && turnWasBounded,
            "Typed WASD moves along authoritative walkable ground with bounded heading turns");
        input.fGoalX = input.fGoalZ = 0.f;
        ++input.iClientSequence; bern->Handle_Move(RIDER_SESSION, input); flyTicks(30u);
        const float landingGround = rider.fVehicleFlightGroundY;
        toggle.iClientSequence = 21u;
        tests.Require(bern->Try_StartVehicleSkill(rider, toggle) && rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::LANDING,
            "A second E starts landing without an old skill cooldown blocking it");
        tests.Require(!bern->Try_StartVehicleSkill(rider, toggle), "A duplicate E cannot restart the landing clock");
        flyTicks(240u);
        tests.Require(rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::GROUNDED &&
            rider.eAction == PLAYER_ACTION_STATE::NONE && Near(rider.fPositionY, landingGround) &&
            rider.iVehicleFlightPhaseStartTick == 0u, "Landing commits the validated ground and clears flight state");
        toggle.iClientSequence = 22u;
        bern->Try_StartVehicleSkill(rider, toggle); flyTicks(60u);
        rider.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
        bern->Enforce_VehicleRidingState();
        tests.Require(rider.iVehicleId == INVALID_VEHICLE_ID &&
            rider.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::GROUNDED &&
            rider.eAction == PLAYER_ACTION_STATE::KNOCKDOWN && Near(rider.fPositionY, landingGround),
            "Forced dismount restores ground without erasing the Server forced action");
    }

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
			"A raid arena never replicates an ordinary mounted player");
        tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == valtan->Apply_SetVehicleRiding(raider,
            Make_VehicleRequest(2u, WORLD_ID::VALTAN_ARENA, ANCIENT_SEA_VEHICLE_ID)).eResult,
            "Valtan admits only the explicit Ancient Sea exception");
        valtan->Enforce_VehicleRidingState();
        tests.Require(raider.iVehicleId == ANCIENT_SEA_VEHICLE_ID, "Valtan snapshot enforcement retains Ancient Sea");
	}

    auto kouku = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
    tests.Require(kouku->Is_Ready(), "Kouku room loads for the Ancient Sea exception");
    if (kouku->Is_Ready())
    {
        SERVER_PLAYER& raider = kouku->m_Players[RIDER_PLAYER];
        raider.iPlayerId = RIDER_PLAYER; raider.iCurrentHp = raider.iMaximumHp = 50000u;
        tests.Require(kouku->Apply_SetVehicleRiding(raider, Make_VehicleRequest(1u,
            WORLD_ID::KAKULSAYDON_ARENA, GOLDEN_TERPEION)).eResult == VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED,
            "Kouku still rejects ordinary vehicles");
        tests.Require(kouku->Apply_SetVehicleRiding(raider, Make_VehicleRequest(2u,
            WORLD_ID::KAKULSAYDON_ARENA, ANCIENT_SEA_VEHICLE_ID)).eResult == VEHICLE_RIDING_RESULT::ACCEPTED,
            "Kouku admits Ancient Sea");
    }
	std::cout << "vehicle riding failures: " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
