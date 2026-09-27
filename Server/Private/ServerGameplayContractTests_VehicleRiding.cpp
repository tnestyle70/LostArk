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
#include <vector>

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
        tests.Require(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED == valtan->Apply_SetVehicleRiding(raider,
            Make_VehicleRequest(3u, WORLD_ID::VALTAN_ARENA, 8200u)).eResult,
            "A raid arena refuses ships too");
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
	/* Bern3 voyage ships (EFTable_VoyageShip 8200..8208) ride like any vehicle: Bern only, at the
	speed the publisher derives from the table (MoveSpeed / 100) times the authored projectTuning
	moveSpeedMultiplier, with SPACE fast sail at the tuned boost multiple of that speed. */
	struct SHIP_EXPECTATION { VEHICLE_ID iId; float fSpeed; float fBoostSpeed; };
	constexpr SHIP_EXPECTATION SHIPS[] = {
		{ 8200u, 4.00f, 12.0f }, { 8201u, 3.80f, 11.4f }, { 8202u, 4.40f, 13.2f },
		{ 8203u, 3.60f, 10.8f }, { 8204u, 3.66f, 10.98f }, { 8205u, 3.70f, 11.1f },
		{ 8206u, 3.60f, 10.8f }, { 8207u, 4.20f, 12.6f }, { 8208u, 4.00f, 12.0f } };
	for (const SHIP_EXPECTATION& ship : SHIPS)
	{
		const SERVER_VEHICLE_DEFINITION* definition = bern->m_VehicleCatalog.Find_Vehicle(ship.iId);
		tests.Require(nullptr != definition && Near(definition->fMoveSpeed, ship.fSpeed),
			"A voyage ship publishes its tuned EFTable_VoyageShip MoveSpeed as metres per second");
		tests.Require(nullptr != definition && Near(definition->fBoostMoveSpeed, ship.fBoostSpeed),
			"A voyage ship publishes SPACE fast sail at the tuned multiple of its move speed");
	}
	SERVER_PLAYER& sailor = bern->m_Players[2u];
	sailor.iPlayerId = 2u;
	sailor.iNetEntityId = 102u;
	sailor.iSessionId = RIDER_SESSION + 1u;
	sailor.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
	sailor.eStance = PLAYER_STANCE_ID::WARLORD_NORMAL;
	sailor.iCurrentHp = sailor.iMaximumHp = 50000u;
	sailor.fMoveSpeed = 2.8f;
	/* Ships sail from a harbour: the sailor stands on the Bern3 pier of the harbour master the map owner
	   placed. Boarding carries the player out to the BernSea navigation region. */
	constexpr float PIER_X = 264.41f, PIER_Y = 11.79f, PIER_Z = -204.03f;
	constexpr float SEA_LEVEL = 10.95f;
	sailor.fPositionX = PIER_X;
	sailor.fPositionY = PIER_Y;
	sailor.fPositionZ = PIER_Z;
	sailor.fYawDegrees = 33.f;
	bern->m_PlayerIdBySessionId[RIDER_SESSION + 1u] = 2u;
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
		sailor, Make_VehicleRequest(1u, WORLD_ID::BERN, 8200u)).eResult &&
		8200u == sailor.iVehicleId && Near(bern->Resolve_PlayerMoveSpeed(sailor), 4.f),
		"Boarding a ship in Bern commits it and moves at the ship speed");
	const float seaX = sailor.fPositionX, seaY = sailor.fPositionY, seaZ = sailor.fPositionZ;
	tests.Require(sailor.bShipDockValid && Near(sailor.fShipDockX, PIER_X) &&
		Near(sailor.fShipDockY, PIER_Y) && Near(sailor.fShipDockZ, PIER_Z) &&
		Near(sailor.fShipDockYawDegrees, 33.f),
	"Boarding a ship keeps the pier position the player stood on");
	tests.Require(std::abs(seaY - SEA_LEVEL) < 0.01f &&
		std::hypot(seaX - PIER_X, seaZ - PIER_Z) > 3.f &&
		bern->m_ServerNavigation.Is_PointWalkableInRegion("BernSea", seaX, seaZ, SEA_LEVEL),
	"Boarding a ship carries the player out onto open sea navigation, away from the pier");
	std::cout << "[test] sea departure (" << seaX << ", " << seaY << ", " << seaZ << ") from the pier, distance "
		<< std::hypot(seaX - PIER_X, seaZ - PIER_Z) << " m\n";
	{
		/* A right-click on the water becomes a server path on the sea region at the ship speed. */
		bool foundGoal = false;
		std::vector<SERVER_NAV_POINT> path;
		SERVER_NAV_POINT goal{};
		for (const float offset : { 20.f, -20.f, 30.f, -30.f, 12.f, -12.f })
			for (int axis = 0; axis < 2 && !foundGoal; ++axis)
			{
				const float goalX = seaX + (0 == axis ? offset : 0.f);
				const float goalZ = seaZ + (1 == axis ? offset : 0.f);
				if (!bern->m_ServerNavigation.Is_PointWalkableInRegion("BernSea", goalX, goalZ, SEA_LEVEL))
					continue;
				path.clear();
				if (bern->m_ServerNavigation.Find_Path(seaX, seaZ, goalX, goalZ, path, seaY) && !path.empty())
				{
					goal = { goalX, SEA_LEVEL, goalZ };
					foundGoal = true;
				}
			}
		tests.Require(foundGoal && std::abs(path.back().y - SEA_LEVEL) < 0.6f &&
			std::hypot(path.back().x - goal.x, path.back().z - goal.z) < 1.5f,
		"A click on the water finds a path on the sea region from the departure point");
	}
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
		sailor, Make_VehicleRequest(2u, WORLD_ID::BERN, 8207u)).eResult &&
		8207u == sailor.iVehicleId && Near(bern->Resolve_PlayerMoveSpeed(sailor), 4.2f),
		"Switching ship changes the movement speed to the new ship");
	tests.Require(sailor.bShipDockValid && Near(sailor.fPositionX, seaX) && Near(sailor.fPositionZ, seaZ) &&
		Near(sailor.fShipDockX, PIER_X),
	"Switching ship at sea keeps the sea position and the pier the player left");
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
		sailor, Make_VehicleRequest(3u, WORLD_ID::BERN, INVALID_VEHICLE_ID)).eResult &&
		INVALID_VEHICLE_ID == sailor.iVehicleId && Near(bern->Resolve_PlayerMoveSpeed(sailor), 2.8f),
		"Leaving the ship restores the walking speed");
	tests.Require(!sailor.bShipDockValid && Near(sailor.fPositionX, PIER_X) && Near(sailor.fPositionY, PIER_Y) &&
		Near(sailor.fPositionZ, PIER_Z) && Near(sailor.fYawDegrees, 33.f),
	"Leaving the ship returns the player to the pier position it left");
	tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
		sailor, Make_VehicleRequest(4u, WORLD_ID::BERN, 8200u)).eResult && sailor.bShipDockValid,
	"A sailor can board again from the pier");
	sailor.iCurrentHp = 0u;
	bern->Enforce_VehicleRidingState();
	tests.Require(INVALID_VEHICLE_ID == sailor.iVehicleId, "A dead sailor leaves the ship");
	tests.Require(!sailor.bShipDockValid && Near(sailor.fPositionX, PIER_X) && Near(sailor.fPositionZ, PIER_Z),
	"A dead sailor is brought back to the pier");
	{
		/* A ship is a harbour service: from the city spawn there is no open sea within reach. */
		SERVER_PLAYER& landlubber = bern->m_Players[3u];
		landlubber.iPlayerId = 3u;
		landlubber.iNetEntityId = 103u;
		landlubber.iSessionId = RIDER_SESSION + 2u;
		landlubber.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		landlubber.eStance = PLAYER_STANCE_ID::WARLORD_NORMAL;
		landlubber.iCurrentHp = landlubber.iMaximumHp = 50000u;
		landlubber.fMoveSpeed = 2.8f;
		landlubber.fPositionX = spawn->fPositionX;
		landlubber.fPositionY = spawn->fPositionY;
		landlubber.fPositionZ = spawn->fPositionZ;
		bern->m_PlayerIdBySessionId[RIDER_SESSION + 2u] = 3u;
		tests.Require(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE == bern->Apply_SetVehicleRiding(
			landlubber, Make_VehicleRequest(1u, WORLD_ID::BERN, 8200u)).eResult &&
			INVALID_VEHICLE_ID == landlubber.iVehicleId && !landlubber.bShipDockValid &&
			Near(landlubber.fPositionX, spawn->fPositionX) && Near(landlubber.fPositionZ, spawn->fPositionZ),
		"Boarding a ship away from a harbour is refused and the player stays put");
		tests.Require(VEHICLE_RIDING_RESULT::ACCEPTED == bern->Apply_SetVehicleRiding(
			landlubber, Make_VehicleRequest(2u, WORLD_ID::BERN, GOLDEN_TERPEION)).eResult &&
			GOLDEN_TERPEION == landlubber.iVehicleId && !landlubber.bShipDockValid &&
			Near(landlubber.fPositionX, spawn->fPositionX),
		"A land mount never moves the player and never stores a pier position");
	}
	{
		/* The three Bern3 ship NPC placements the map owner chose: published, enabled, spawned as world
		   entities in the Bern room and standing on walkable Bern3 ground. A shipwright and a harbour
		   master that the room does not hold, or that stand off the navigation, can never be reached. */
		struct SHIP_NPC_EXPECTATION { const char* placementId; const char* archetypeId; float x, y, z; };
		constexpr SHIP_NPC_EXPECTATION SHIP_NPCS[] = {
			{ "npc.bern.ship.shipwright.1", "NPC_SHIP_SHIPWRIGHT", 271.59f, 12.43f, -196.7f },
			{ "npc.bern.ship.shipwright.2", "NPC_SHIP_SHIPWRIGHT", 229.61f, 12.47f, -196.5f },
			{ "npc.bern.ship.harbormaster.1", "NPC_SHIP_HARBORMASTER", 264.41f, 11.79f, -204.03f } };
		for (const SHIP_NPC_EXPECTATION& expected : SHIP_NPCS)
		{
			const auto placement = std::find_if(placements.begin(), placements.end(),
				[&expected](const WORLD_BOOTSTRAP_PLACEMENT& candidate)
				{
					return candidate.strPlacementId == expected.placementId;
				});
			tests.Require(placements.end() != placement && placement->isEnabled &&
				WORLD_BOOTSTRAP_KIND::NPC == placement->eKind &&
				placement->strArchetypeId == expected.archetypeId &&
				std::abs(placement->fPositionX - expected.x) < 0.001f &&
				std::abs(placement->fPositionY - expected.y) < 0.001f &&
				std::abs(placement->fPositionZ - expected.z) < 0.001f,
				"The published Bern world places this ship NPC at the map owner's Bern3 position");
			const auto spawned = std::find_if(bern->m_WorldEntities.begin(), bern->m_WorldEntities.end(),
				[&expected](const SERVER_WORLD_ENTITY& entity)
				{
					return entity.strPlacementId == expected.placementId;
				});
			tests.Require(bern->m_WorldEntities.end() != spawned &&
				WORLD_BOOTSTRAP_KIND::NPC == spawned->eKind &&
				spawned->strArchetypeId == expected.archetypeId &&
				std::abs(spawned->fPositionX - expected.x) < 0.5f &&
				std::abs(spawned->fPositionZ - expected.z) < 0.5f,
				"The Bern room spawns this ship NPC as a world entity");
			tests.Require(bern->m_ServerNavigation.Is_PointWalkableExact(expected.x, expected.z, expected.y),
				"This ship NPC stands on walkable Bern3 navigation");
		}
	}
	std::cout << "vehicle riding failures: " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
