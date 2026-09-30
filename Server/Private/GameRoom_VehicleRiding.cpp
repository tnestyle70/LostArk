#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	constexpr std::uint32_t VEHICLE_SKILL_TICK_HZ = 30u;
	constexpr float VEHICLE_SKILL_DEGREES_TO_RADIANS = 0.0174532925f;

	/* EFTable_VoyageShip primary keys (the nine base ships). */
	constexpr LostArk::Shared::VEHICLE_ID SHIP_VEHICLE_ID_FIRST = 8200u;
	constexpr LostArk::Shared::VEHICLE_ID SHIP_VEHICLE_ID_LAST = 8208u;
	/* Navigation level of the BernSea region: water surface 10.8 m plus 0.15 m of clearance, so a hull
	   lifted to its keel (Client modelLiftMeters) floats just above the surface. Both values are written by
	   Tools/ShipPipeline/build_sea_nav.py (SEA_SURFACE_Y - SHIP_DRAFT_M); change them together. */
	constexpr char SHIP_SEA_REGION_ID[] = "BernSea";
	constexpr float SHIP_SEA_NAV_LEVEL_Y = 10.95f;
	constexpr float SHIP_SEA_LEVEL_TOLERANCE_M = 0.6f;
	constexpr float SHIP_DEPARTURE_FIRST_RING_M = 4.f;
	constexpr float SHIP_DEPARTURE_RING_STEP_M = 2.f;
	constexpr float SHIP_DEPARTURE_MAX_SEARCH_M = 100.f;
	constexpr float SHIP_RADIANS_TO_DEGREES = 57.2957795f;

	bool Is_ShipVehicleId(const LostArk::Shared::VEHICLE_ID vehicleId)
	{
		return vehicleId >= SHIP_VEHICLE_ID_FIRST && vehicleId <= SHIP_VEHICLE_ID_LAST;
	}

	bool Is_VehicleRidingWorld(const LostArk::Shared::WORLD_ID worldId, const LostArk::Shared::VEHICLE_ID vehicleId)
	{
		return LostArk::Shared::WORLD_ID::BERN == worldId ||
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
			(vehicleId == LostArk::Shared::ANCIENT_SEA_VEHICLE_ID &&
			 (worldId == LostArk::Shared::WORLD_ID::VALTAN_ARENA ||
			  worldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA));
	}

	std::uint32_t Vehicle_TicksFromMs(const std::uint32_t milliseconds)
	{
		return (milliseconds * VEHICLE_SKILL_TICK_HZ + 999u) / 1000u;
	}

	void Sample_VehicleRootMotion(
		const std::vector<LostArk::Server::ROOT_MOTION_SAMPLE>& samples,
		const float elapsedSeconds,
		float& outForward,
		float& outLateral)
	{
		outForward = 0.f;
		outLateral = 0.f;
		if (samples.empty())
			return;
		const float elapsedMs = elapsedSeconds * 1000.f;
		if (elapsedMs <= static_cast<float>(samples.front().iTimeMs))
		{
			outForward = samples.front().fForward;
			outLateral = samples.front().fLateral;
			return;
		}
		for (std::size_t index = 1u; index < samples.size(); ++index)
		{
			const auto& previous = samples[index - 1u];
			const auto& current = samples[index];
			if (elapsedMs > static_cast<float>(current.iTimeMs))
				continue;
			const float span = static_cast<float>(current.iTimeMs - previous.iTimeMs);
			const float alpha = span <= 0.f ? 0.f :
				(elapsedMs - static_cast<float>(previous.iTimeMs)) / span;
			outForward = previous.fForward + (current.fForward - previous.fForward) * alpha;
			outLateral = previous.fLateral + (current.fLateral - previous.fLateral) * alpha;
			return;
		}
		outForward = samples.back().fForward;
		outLateral = samples.back().fLateral;
	}
}

float LostArk::Server::CGameRoom::Resolve_PlayerMoveSpeed(
	const SERVER_PLAYER& player) const
{
	if (LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId)
	{
		if (const SERVER_VEHICLE_DEFINITION* vehicle =
			m_VehicleCatalog.Find_Vehicle(player.iVehicleId))
		{
			/* Fast sail raises the authoritative speed until the boost tick passes. Movement and
			   the replicated SNAPSHOT_PLAYER fMoveSpeed both read this one resolver, so the boost
			   reaches the Client without a protocol field of its own. */
			if (vehicle->Has_Boost() && 0u != player.iShipBoostEndTick &&
				static_cast<std::int32_t>(player.iShipBoostEndTick - m_iServerTick) > 0)
			{
				return vehicle->fBoostMoveSpeed;
			}
			return vehicle->fMoveSpeed;
		}
	}
	/* Waterpang water gun E: the source speed-up buff raises the walking speed until its
	   end tick. It is Maharaka only, so a stale end tick can never follow a player to another room. */
	const bool waterGunSpeedUp = LostArk::Shared::WORLD_ID::MAHARAKA == m_eWorldId &&
		0u != player.iWaterGunSpeedEndTick &&
		static_cast<std::int32_t>(player.iWaterGunSpeedEndTick - m_iServerTick) > 0;
	return player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *
		(waterGunSpeedUp ? player.fWaterGunSpeedScale : 1.f);
}

bool LostArk::Server::CGameRoom::Can_RideVehicle(
	const SERVER_PLAYER& player) const
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp &&
		(PLAYER_ACTION_STATE::NONE == player.eAction ||
		 PLAYER_ACTION_STATE::VEHICLE_SKILL == player.eAction) &&
		!player.bPatternBound &&
		0u == player.iMarioStage &&
		PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm &&
		KOUKU_HUD_MODE::NONE == player.eKoukuHudMode &&
		INVALID_NET_ENTITY_ID == player.iAttachmentOwnerNetEntityId &&
		player.fKnockbackRemainingSeconds <= 0.f &&
		!player.TriggerMove.isActive &&
		0u == player.CardMaze.flags &&
		0u == player.CardMaze.transferStartTick;
}

void LostArk::Server::CGameRoom::Handle_SetVehicleRiding(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = VEHICLE_RIDING_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
	{
		result = Apply_SetVehicleRiding(player->second, request);
		std::cout << "[VehicleRiding] player=" << static_cast<unsigned>(player->second.iPlayerId)
			<< " requested=" << request.iVehicleId << " result=" << static_cast<unsigned>(result.eResult)
			<< " active=" << result.iActiveVehicleId << " world=" << static_cast<unsigned>(m_eWorldId) << "\n";
	}
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_SET_VEHICLE_RIDING_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT
LostArk::Server::CGameRoom::Apply_SetVehicleRiding(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request)
{
	using namespace LostArk::Shared;
	S2C_SET_VEHICLE_RIDING_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.iActiveVehicleId = player.iVehicleId;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const S2C_SET_VEHICLE_RIDING_RESULT& previous = player.LastVehicleRidingResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
	{
		return previous;
	}
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = VEHICLE_RIDING_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto commit = [&player, &result](const VEHICLE_RIDING_RESULT reason)
	{
		result.eResult = reason;
		result.iActiveVehicleId = player.iVehicleId;
		player.LastVehicleRidingResult = result;
		return result;
	};
	if (request.iVehicleId == player.iVehicleId)
		return commit(VEHICLE_RIDING_RESULT::REJECTED_SAME_STATE);
	if (INVALID_VEHICLE_ID == request.iVehicleId)
	{
		End_VehicleSkill(player);
		End_ShipVoyage(player, "left the ship");
		player.iVehicleId = INVALID_VEHICLE_ID;
		return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
	}
	if (!Is_VehicleRidingWorld(m_eWorldId, request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED);
	if (nullptr == m_VehicleCatalog.Find_Vehicle(request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE);
	if (!Can_RideVehicle(player))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE);
	/* A ship is an avatar of the sea: it only exists in Bern, and boarding carries the player to the
	   sea region. Any other vehicle taken from a ship first brings the player back to the pier. */
	const bool wasShip = Is_ShipVehicleId(player.iVehicleId);
	const bool nowShip = Is_ShipVehicleId(request.iVehicleId);
	if (nowShip && WORLD_ID::BERN != m_eWorldId)
		return commit(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED);
	if (nowShip && !wasShip)
	{
		// The voyage validates a sea destination before ending the current mount action.
		if (!Begin_ShipVoyage(player, request.iVehicleId))
			return commit(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE);
	}
	else
		End_VehicleSkill(player);
	if (wasShip && !nowShip)
		End_ShipVoyage(player, "changed to another vehicle");
	player.iVehicleId = request.iVehicleId;
	return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
}

bool LostArk::Server::CGameRoom::Begin_ShipVoyage(
	SERVER_PLAYER& player,
	const LostArk::Shared::VEHICLE_ID vehicleId)
{
	if (player.bShipDockValid)
		return true;
	const float startX = player.fPositionX;
	const float startZ = player.fPositionZ;
	for (float radius = SHIP_DEPARTURE_FIRST_RING_M; radius <= SHIP_DEPARTURE_MAX_SEARCH_M;
		radius += SHIP_DEPARTURE_RING_STEP_M)
	{
		/* The nearest ring that holds an open sea cell wins; inside a ring the closest cell to the
		   straight-out direction of the first hit is not needed because the region is one open water body. */
		const int samples = (std::max)(12, static_cast<int>(2.f * 3.14159265f * radius / 2.f));
		for (int sample = 0; sample < samples; ++sample)
		{
			const float angle = 2.f * 3.14159265f * static_cast<float>(sample) / static_cast<float>(samples);
			const float candidateX = startX + std::sin(angle) * radius;
			const float candidateZ = startZ + std::cos(angle) * radius;
			if (!m_ServerNavigation.Is_PointWalkableInRegion(
				SHIP_SEA_REGION_ID, candidateX, candidateZ, SHIP_SEA_NAV_LEVEL_Y))
				continue;
			SERVER_NAV_POINT sea{};
			if (!m_ServerNavigation.Sample_Position(candidateX, candidateZ, sea, SHIP_SEA_NAV_LEVEL_Y) ||
				std::abs(sea.y - SHIP_SEA_NAV_LEVEL_Y) > SHIP_SEA_LEVEL_TOLERANCE_M)
				continue;
			const float yaw = std::atan2(sea.x - startX, sea.z - startZ) * SHIP_RADIANS_TO_DEGREES;
			// No rejection remains after this point. A refused voyage must retain
			// the running mount skill/flight and its position as well as its vehicle ID.
			End_VehicleSkill(player);
			player.bShipDockValid = true;
			player.fShipDockX = player.fPositionX;
			player.fShipDockY = player.fPositionY;
			player.fShipDockZ = player.fPositionZ;
			player.fShipDockYawDegrees = player.fYawDegrees;
			Reset_PlayerForDebugTeleport(player);
			player.fPositionX = sea.x;
			player.fPositionY = sea.y;
			player.fPositionZ = sea.z;
			player.fYawDegrees = yaw;
			std::cout << "[ShipBoard] player=" << static_cast<unsigned>(player.iPlayerId)
				<< " ship=" << vehicleId << " pier=(" << player.fShipDockX << ", " << player.fShipDockY << ", "
				<< player.fShipDockZ << ") sea=(" << sea.x << ", " << sea.y << ", " << sea.z << ") yaw=" << yaw
				<< " distance=" << radius << "\n";
			return true;
		}
	}
	std::cout << "[ShipBoard] player=" << static_cast<unsigned>(player.iPlayerId) << " ship=" << vehicleId
		<< " refused: no open sea within " << SHIP_DEPARTURE_MAX_SEARCH_M << " m of ("
		<< startX << ", " << startZ << ")\n";
	return false;
}

void LostArk::Server::CGameRoom::End_ShipVoyage(SERVER_PLAYER& player, const char* reason)
{
	// Leaving the ship ends the fast sail; the walk back up the pier keeps the on-foot speed.
	player.iShipBoostEndTick = 0u;
	if (!player.bShipDockValid)
		return;
	const float seaX = player.fPositionX;
	const float seaZ = player.fPositionZ;
	Reset_PlayerForDebugTeleport(player);
	player.fPositionX = player.fShipDockX;
	player.fPositionY = player.fShipDockY;
	player.fPositionZ = player.fShipDockZ;
	player.fYawDegrees = player.fShipDockYawDegrees;
	player.bShipDockValid = false;
	std::cout << "[ShipBoard] player=" << static_cast<unsigned>(player.iPlayerId) << " back to pier=("
		<< player.fPositionX << ", " << player.fPositionY << ", " << player.fPositionZ << ") from sea=("
		<< seaX << ", " << seaZ << ") reason=" << (nullptr == reason ? "" : reason) << "\n";
}

void LostArk::Server::CGameRoom::Enforce_VehicleRidingState()
{
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::INVALID_VEHICLE_ID == player.iVehicleId)
			continue;
		if (!Is_VehicleRidingWorld(m_eWorldId, player.iVehicleId) ||
			nullptr == m_VehicleCatalog.Find_Vehicle(player.iVehicleId) ||
			!Can_RideVehicle(player))
		{
			End_VehicleSkill(player);
			End_ShipVoyage(player, "forced dismount");
			player.iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		}
	}
}

bool LostArk::Server::CGameRoom::Try_StartVehicleSkill(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command)
{
	using namespace LostArk::Shared;
	const SERVER_VEHICLE_DEFINITION* vehicle = m_VehicleCatalog.Find_Vehicle(player.iVehicleId);
	const SERVER_VEHICLE_SKILL* skill = nullptr == vehicle ? nullptr : vehicle->Find_Skill(command.iSkillId);
	const bool flightToggle = vehicle && vehicle->Has_Flight() && skill && skill->eSlot == VEHICLE_SKILL_SLOT::E;
	if (flightToggle && player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)
	{
		if (!Can_RideVehicle(player) || !Is_NewerSequence(command.iClientSequence, player.iLastSkillSequence) ||
			player.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::LANDING) return false;
		// Flight may cross nav gaps; landing must still have walkable ground directly below.
		SERVER_NAV_POINT landing{};
		if (!m_ServerNavigation.Is_PointWalkableExact(player.fPositionX, player.fPositionZ, player.fPositionY) ||
			!m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, landing, player.fPositionY) ||
			landing.y > player.fPositionY + .05f) return false;
		float x = landing.x, y = landing.y, z = landing.z;
		bool blocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(player, landing.x, landing.y, landing.z, x, y, z, blocked) || blocked)
			return false;
		player.VehicleFlightSafeLanding = landing;
		player.fVehicleFlightGroundY = landing.y;
		player.iLastSkillSequence = command.iClientSequence;
		player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::LANDING;
		player.iVehicleFlightPhaseStartTick = m_iServerTick + 1u;
		if (!player.iVehicleFlightPhaseStartTick) player.iVehicleFlightPhaseStartTick = 1u;
		player.fVehicleFlightPhaseSeconds = 0.f;
		player.fVehicleFlightStartHeight = (std::max)(0.f, player.fPositionY - player.fVehicleFlightGroundY);
		player.fVehicleFlightInputX = player.fVehicleFlightInputZ = player.fVehicleFlightInputY = 0.f;
		return true;
	}
	/* Fast sail is a sailing speed change, not a mount action: the ship is steered by
	   click-to-move, so unlike the generic branch below it must not enter VEHICLE_SKILL nor clear
	   the move goal and path. It only moves the boost's end tick and takes the skill's cooldown,
	   so pressing SPACE again refreshes the window instead of stacking a second multiplier. */
	const bool shipBoost = vehicle && vehicle->Has_Boost() && skill &&
		VEHICLE_SKILL_SLOT::SPACE == skill->eSlot;
	if (shipBoost)
	{
		if (!Is_VehicleRidingWorld(m_eWorldId, player.iVehicleId) ||
			PLAYER_ACTION_STATE::NONE != player.eAction || !Can_RideVehicle(player) ||
			!Is_NewerSequence(command.iClientSequence, player.iLastSkillSequence))
		{
			return false;
		}
		const std::uint32_t boostStartTick =
			(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
		const auto boostCooldown = player.CooldownEndTickBySkillId.find(skill->iSkillId);
		if (boostCooldown != player.CooldownEndTickBySkillId.end() &&
			static_cast<std::int32_t>(boostCooldown->second - boostStartTick) > 0)
		{
			return false;
		}
		player.iLastSkillSequence = command.iClientSequence;
		player.iShipBoostEndTick =
			boostStartTick + Vehicle_TicksFromMs(skill->iActionDurationMs);
		if (0u == player.iShipBoostEndTick) player.iShipBoostEndTick = 1u;
		player.CooldownEndTickBySkillId.insert_or_assign(
			skill->iSkillId, boostStartTick + Vehicle_TicksFromMs(skill->iCooldownMs));
		return true;
	}
	if (nullptr == skill || !Is_VehicleRidingWorld(m_eWorldId, player.iVehicleId) ||
		PLAYER_ACTION_STATE::NONE != player.eAction || !Can_RideVehicle(player) ||
		!Is_NewerSequence(command.iClientSequence, player.iLastSkillSequence))
	{
		return false;
	}
	const std::uint32_t startTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
	const auto cooldown = player.CooldownEndTickBySkillId.find(skill->iSkillId);
	if (!flightToggle && cooldown != player.CooldownEndTickBySkillId.end() &&
		static_cast<std::int32_t>(cooldown->second - startTick) > 0)
	{
		return false;
	}
	const float yawRadians = player.fYawDegrees * VEHICLE_SKILL_DEGREES_TO_RADIANS;
	player.iLastSkillSequence = command.iClientSequence;
	player.eAction = PLAYER_ACTION_STATE::VEHICLE_SKILL;
	player.iCurrentSkillId = skill->iSkillId;
	player.iActionStartTick = startTick;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = std::sin(yawRadians);
	player.fSkillAimDirectionZ = std::cos(yawRadians);
	player.Clear_SkillTarget();
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.PendingCommand.Clear();
	if (!flightToggle) player.CooldownEndTickBySkillId.insert_or_assign(
		skill->iSkillId, startTick + Vehicle_TicksFromMs(skill->iCooldownMs));
	if (flightToggle)
	{
		player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::TAKEOFF;
		player.iVehicleFlightPhaseStartTick = startTick;
		player.fVehicleFlightPhaseSeconds = 0.f;
		player.fVehicleFlightGroundY = player.fPositionY;
		player.VehicleFlightSafeLanding = {player.fPositionX, player.fPositionY, player.fPositionZ};
		player.fVehicleFlightStartHeight = 0.f;
		player.fVehicleFlightInputAge = 0.f;
		player.fVehicleFlightInputX = player.fVehicleFlightInputZ = player.fVehicleFlightInputY = 0.f;
		player.fVehicleFlightVelocityX = player.fVehicleFlightVelocityZ = player.fVehicleFlightVelocityY = 0.f;
	}
	return true;
}

void LostArk::Server::CGameRoom::Update_VehicleSkill(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::VEHICLE_SKILL != player.eAction)
		return;
	const SERVER_VEHICLE_DEFINITION* vehicle = m_VehicleCatalog.Find_Vehicle(player.iVehicleId);
	const SERVER_VEHICLE_SKILL* skill =
		nullptr == vehicle ? nullptr : vehicle->Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill)
	{
		End_VehicleSkill(player);
		return;
	}
	if (player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)
	{
		if (!vehicle->Has_Flight() || !Can_RideVehicle(player)) { End_VehicleSkill(player); return; }
		player.fActionElapsedSeconds += fixedDeltaSeconds;
		player.fVehicleFlightPhaseSeconds += fixedDeltaSeconds;
		player.fVehicleFlightInputAge += fixedDeltaSeconds;
		const float height = (std::max)(0.f, player.fPositionY - player.fVehicleFlightGroundY);
		if (player.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::TAKEOFF)
		{
			const float t = std::clamp(player.fVehicleFlightPhaseSeconds / vehicle->fFlightTakeoffSeconds, 0.f, 1.f);
			player.fPositionY = player.fVehicleFlightGroundY + vehicle->fFlightHoverHeight * t * t * (3.f - 2.f * t);
			if (t >= 1.f)
			{
				player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::FLYING;
				player.iVehicleFlightPhaseStartTick = m_iServerTick + 1u;
				if (!player.iVehicleFlightPhaseStartTick) player.iVehicleFlightPhaseStartTick = 1u;
				player.fVehicleFlightPhaseSeconds = 0.f;
			}
		}
		else if (player.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::LANDING)
		{
			// A raid floor or blocking body may change after E was accepted. Never
			// finish a descent onto stale ground; hold altitude and resume flight.
			SERVER_NAV_POINT landing{};
			bool validLanding = m_ServerNavigation.Is_PointWalkableExact(
				player.fPositionX, player.fPositionZ, player.fVehicleFlightGroundY) &&
				m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ,
					landing, player.fVehicleFlightGroundY) &&
				std::abs(landing.y - player.fVehicleFlightGroundY) <= .05f &&
				m_ServerCollisionSystem.Is_PlayerPositionClear(landing.x, landing.y, landing.z, player.iNetEntityId);
			if (validLanding)
			{
				float x = landing.x, y = landing.y, z = landing.z;
				bool blocked = false;
				validLanding = m_ServerCollisionSystem.Resolve_PlayerMove(player, landing.x, landing.y,
					landing.z, x, y, z, blocked) && !blocked;
			}
			if (!validLanding)
			{
				player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::FLYING;
				player.iVehicleFlightPhaseStartTick = m_iServerTick + 1u;
				if (!player.iVehicleFlightPhaseStartTick) player.iVehicleFlightPhaseStartTick = 1u;
				player.fVehicleFlightPhaseSeconds = 0.f;
				// E may have targeted a lower deck than the takeoff reference. Preserve
				// this altitude when returning to the bounded cruise integrator.
				player.fVehicleFlightGroundY = (std::max)(player.fVehicleFlightGroundY,
					player.fPositionY - vehicle->fFlightMaximumHeight);
				player.fVehicleFlightVelocityX = player.fVehicleFlightVelocityZ = player.fVehicleFlightVelocityY = 0.f;
				return;
			}
			player.VehicleFlightSafeLanding = landing;
			const float duration = (std::max)(vehicle->fFlightLandingSeconds,
				player.fVehicleFlightStartHeight / vehicle->fFlightVerticalSpeed);
			const float t = std::clamp(player.fVehicleFlightPhaseSeconds / duration, 0.f, 1.f);
			player.fPositionY = player.fVehicleFlightGroundY + player.fVehicleFlightStartHeight * (1.f - t * t * (3.f - 2.f * t));
			if (t >= 1.f) End_VehicleSkill(player);
		}
		else
		{
			const bool liveInput = player.fVehicleFlightInputAge <= .35f;
			const float blend = 1.f - std::exp(-8.f * fixedDeltaSeconds);
			const float targetX = liveInput ? player.fVehicleFlightInputX * vehicle->fFlightSpeed : 0.f;
			const float targetZ = liveInput ? player.fVehicleFlightInputZ * vehicle->fFlightSpeed : 0.f;
			const float targetY = liveInput ? player.fVehicleFlightInputY * vehicle->fFlightVerticalSpeed : 0.f;
			player.fVehicleFlightVelocityX += (targetX - player.fVehicleFlightVelocityX) * blend;
			player.fVehicleFlightVelocityZ += (targetZ - player.fVehicleFlightVelocityZ) * blend;
			player.fVehicleFlightVelocityY += (targetY - player.fVehicleFlightVelocityY) * blend;
			const float nextX = player.fPositionX + player.fVehicleFlightVelocityX * fixedDeltaSeconds;
			const float nextZ = player.fPositionZ + player.fVehicleFlightVelocityZ * fixedDeltaSeconds;
			const float nextY = player.fVehicleFlightGroundY + std::clamp(
				height + player.fVehicleFlightVelocityY * fixedDeltaSeconds, .1f, vehicle->fFlightMaximumHeight);
			float resolvedX = nextX, resolvedY = nextY, resolvedZ = nextZ;
			bool blocked = false;
			// Airborne travel uses the actual 3D collision volume, independent of walkable XZ.
			if (m_ServerCollisionSystem.Resolve_PlayerMove(player, nextX, nextY, nextZ,
				resolvedX, resolvedY, resolvedZ, blocked))
			{
				player.fPositionX = resolvedX; player.fPositionY = resolvedY; player.fPositionZ = resolvedZ;
			}
			SERVER_NAV_POINT ground{};
			if (m_ServerNavigation.Is_PointWalkableExact(player.fPositionX, player.fPositionZ, player.fPositionY) &&
				m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, ground, player.fPositionY) &&
				ground.y <= player.fPositionY)
			{
				float x = ground.x, y = ground.y, z = ground.z;
				bool landingBlocked = false;
				if (m_ServerCollisionSystem.Resolve_PlayerMove(player, ground.x, ground.y, ground.z,
					x, y, z, landingBlocked) && !landingBlocked)
					player.VehicleFlightSafeLanding = ground;
			}
			if (targetX * targetX + targetZ * targetZ > .0001f)
			{
				const float desiredYaw = std::atan2(targetX, targetZ) / VEHICLE_SKILL_DEGREES_TO_RADIANS;
				const float delta = std::remainder(desiredYaw - player.fYawDegrees, 360.f);
				player.fYawDegrees += std::clamp(delta, -150.f * fixedDeltaSeconds, 150.f * fixedDeltaSeconds);
				player.fYawDegrees = std::remainder(player.fYawDegrees, 360.f);
			}
		}
		return;
	}
	const float previousSeconds = player.fActionElapsedSeconds;
	player.fActionElapsedSeconds += fixedDeltaSeconds;
	float previousForward = 0.f;
	float previousLateral = 0.f;
	float currentForward = 0.f;
	float currentLateral = 0.f;
	Sample_VehicleRootMotion(skill->RootMotion, previousSeconds, previousForward, previousLateral);
	Sample_VehicleRootMotion(skill->RootMotion, player.fActionElapsedSeconds, currentForward, currentLateral);
	const float stepForward = currentForward - previousForward;
	const float stepLateral = currentLateral - previousLateral;
	if (0.f != stepForward || 0.f != stepLateral)
	{
		const float rightX = player.fSkillAimDirectionZ;
		const float rightZ = -player.fSkillAimDirectionX;
		const float nextX = player.fPositionX +
			player.fSkillAimDirectionX * stepForward + rightX * stepLateral;
		const float nextZ = player.fPositionZ +
			player.fSkillAimDirectionZ * stepForward + rightZ * stepLateral;
		SERVER_NAV_POINT reachable{ nextX, player.fPositionY, nextZ };
		if (m_ServerNavigation.Is_Loaded())
		{
			bool wasClamped = false;
			CPlayerSkillSystem::Clamp_StepToWalkable(
				m_ServerNavigation, player.fPositionX, player.fPositionZ,
				nextX, nextZ, reachable, wasClamped, player.fPositionY);
		}
		float resolvedX = reachable.x;
		float resolvedY = reachable.y;
		float resolvedZ = reachable.z;
		bool wasBlocked = false;
		if (m_ServerCollisionSystem.Resolve_PlayerMove(
				player, reachable.x, reachable.y, reachable.z,
				resolvedX, resolvedY, resolvedZ, wasBlocked))
		{
			player.fPositionX = resolvedX;
			player.fPositionY = resolvedY;
			player.fPositionZ = resolvedZ;
		}
	}
	if (player.fActionElapsedSeconds * 1000.f >= static_cast<float>(skill->iActionDurationMs))
		End_VehicleSkill(player);
}

void LostArk::Server::CGameRoom::End_VehicleSkill(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	if (player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)
	{
		// Forced dismount is a relocation, so validate the destination itself rather
		// than sweeping through the intervening airspace. Historical safe ground
		// can have collapsed or become occupied since the last flight snapshot.
		SERVER_NAV_POINT landing{};
		const auto resolveGround = [&](const SERVER_NAV_POINT& candidate)
		{
			return m_ServerNavigation.Is_PointWalkableExact(candidate.x, candidate.z, candidate.y) &&
				m_ServerNavigation.Sample_Position(candidate.x, candidate.z, landing, candidate.y) &&
				m_ServerCollisionSystem.Is_PlayerPositionClear(landing.x, landing.y, landing.z, player.iNetEntityId);
		};
		bool supported = resolveGround(player.VehicleFlightSafeLanding);
		SERVER_NAV_POINT projected{};
		if (!supported && m_ServerNavigation.Project_PointOnSameLevel(player.VehicleFlightSafeLanding.x,
			player.VehicleFlightSafeLanding.z, projected, player.VehicleFlightSafeLanding.y))
			supported = resolveGround(projected);
		if (!supported)
		{
			for (const auto& spawn : m_WorldBootstrap.Get_Placements())
			{
				if (!spawn.isEnabled || spawn.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN) continue;
				if (m_ServerNavigation.Project_Point(spawn.fPositionX, spawn.fPositionZ, projected, spawn.fPositionY) &&
					resolveGround(projected)) { supported = true; break; }
			}
		}
		if (supported)
		{
			player.fPositionX = landing.x;
			player.fPositionY = landing.y;
			player.fPositionZ = landing.z;
			player.VehicleFlightSafeLanding = landing;
		}
		else if (player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
			player.eAction != PLAYER_ACTION_STATE::FALLING)
		{
			// An area with no valid recovery floor uses the existing authoritative fall.
			Begin_PlayerFall(player, 0.f, m_iServerTick + 1u);
		}
		player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::GROUNDED;
		player.iVehicleFlightPhaseStartTick = 0u;
		player.fVehicleFlightPhaseSeconds = 0.f;
		player.fVehicleFlightInputX = player.fVehicleFlightInputZ = player.fVehicleFlightInputY = 0.f;
		player.fVehicleFlightVelocityX = player.fVehicleFlightVelocityZ = player.fVehicleFlightVelocityY = 0.f;
	}
	if (PLAYER_ACTION_STATE::VEHICLE_SKILL != player.eAction)
		return;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
}
