#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	constexpr std::uint32_t VEHICLE_SKILL_TICK_HZ = 30u;
	constexpr float VEHICLE_SKILL_DEGREES_TO_RADIANS = 0.0174532925f;

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
			return vehicle->fMoveSpeed;
		}
	}
	return player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player);
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
		result = Apply_SetVehicleRiding(player->second, request);
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
		player.iVehicleId = INVALID_VEHICLE_ID;
		return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
	}
	if (!Is_VehicleRidingWorld(m_eWorldId, request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_WORLD_NOT_ALLOWED);
	if (nullptr == m_VehicleCatalog.Find_Vehicle(request.iVehicleId))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_UNKNOWN_VEHICLE);
	if (!Can_RideVehicle(player))
		return commit(VEHICLE_RIDING_RESULT::REJECTED_PLAYER_STATE);
	End_VehicleSkill(player);
	player.iVehicleId = request.iVehicleId;
	return commit(VEHICLE_RIDING_RESULT::ACCEPTED);
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
		player.iLastSkillSequence = command.iClientSequence;
		player.eVehicleFlightPhase = VEHICLE_FLIGHT_PHASE::LANDING;
		player.iVehicleFlightPhaseStartTick = m_iServerTick + 1u;
		if (!player.iVehicleFlightPhaseStartTick) player.iVehicleFlightPhaseStartTick = 1u;
		player.fVehicleFlightPhaseSeconds = 0.f;
		player.fVehicleFlightStartHeight = (std::max)(0.f, player.fPositionY - player.fVehicleFlightGroundY);
		player.fVehicleFlightInputX = player.fVehicleFlightInputZ = player.fVehicleFlightInputY = 0.f;
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
			SERVER_NAV_POINT reachable{player.fPositionX, player.fVehicleFlightGroundY, player.fPositionZ};
			bool clamped = false;
			if (m_ServerNavigation.Is_Loaded())
				CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation, player.fPositionX, player.fPositionZ,
					nextX, nextZ, reachable, clamped, player.fVehicleFlightGroundY);
			float resolvedX = reachable.x, resolvedY = reachable.y, resolvedZ = reachable.z;
			bool blocked = false;
			// A ground-height mirror keeps walls/raid navigation authoritative while flight is cosmetic altitude.
			const float airborneY = player.fPositionY;
			player.fPositionY = player.fVehicleFlightGroundY;
			const bool resolved = m_ServerCollisionSystem.Resolve_PlayerMove(player, reachable.x, reachable.y, reachable.z,
				resolvedX, resolvedY, resolvedZ, blocked);
			player.fPositionY = airborneY;
			if (resolved)
			{
				player.fPositionX = resolvedX; player.fPositionZ = resolvedZ;
				player.fVehicleFlightGroundY = resolvedY;
			}
			player.fPositionY = player.fVehicleFlightGroundY + std::clamp(
				height + player.fVehicleFlightVelocityY * fixedDeltaSeconds, .1f, vehicle->fFlightMaximumHeight);
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
		player.fPositionY = player.fVehicleFlightGroundY;
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
