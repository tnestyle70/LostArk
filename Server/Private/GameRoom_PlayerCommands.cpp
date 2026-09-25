#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/KoukuArenaReadyAreas.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_Move(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MOVE& move)
{
	const bool entryTerraceMove = Is_KoukuRaidRunning() &&
		m_KoukuRaid.State.ePhase == LostArk::Shared::KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
	if (Is_KoukuRaidInputBlocked() && !entryTerraceMove) return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_MOVE", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_MOVE", "missing-player-state");
		return;
	}

	SERVER_PLAYER& player = playerIter->second;
	if (entryTerraceMove &&
		(move.eIntent != LostArk::Shared::PLAYER_MOVE_INTENT::GROUND_GOAL ||
		 !LostArk::Shared::Is_KoukuGate3EntryTerrace(player.fPositionX, player.fPositionY, player.fPositionZ) ||
		 !LostArk::Shared::Is_KoukuGate3EntryTerrace(move.fGoalX, player.fPositionY, move.fGoalZ))) return;
	std::string validationFailure;
	if (!Is_NewerSequence(move.iClientSequence, player.iLastMoveSequence))
	{
		validationFailure =
			"packet=C2S_MOVE validation=stale-sequence receivedSequence=" +
			std::to_string(move.iClientSequence) + " lastSequence=" +
			std::to_string(player.iLastMoveSequence);
	}
	else if (!std::isfinite(move.fGoalX) || !std::isfinite(move.fGoalZ))
	{
		validationFailure =
			"packet=C2S_MOVE validation=non-finite-goal";
	}
	else if (std::abs(move.fGoalX) > MAX_ABS_MOVE_GOAL ||
		std::abs(move.fGoalZ) > MAX_ABS_MOVE_GOAL)
	{
		validationFailure =
			"packet=C2S_MOVE validation=out-of-range-goal goalX=" +
			std::to_string(move.fGoalX) + " goalZ=" +
			std::to_string(move.fGoalZ);
	}
	if (!validationFailure.empty())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		{
			session->Request_Close(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED,
				WSAEINVAL,
				validationFailure);
		}
		return;
	}

	player.iLastMoveSequence = move.iClientSequence;
	if (((player.CardMaze.flags & 1u) && !m_KoukuCardMaze.Is_SoloHunter(player.iPlayerId)) ||
		player.CardMaze.transferStartTick) return;
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase &&
		!(VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				m_ValtanTimelineAudition.ePhase &&
			player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId))
	{
		return;
	}
	if (move.eIntent == LostArk::Shared::PLAYER_MOVE_INTENT::VEHICLE_FLIGHT)
	{
		if (player.iVehicleId == LostArk::Shared::ANCIENT_SEA_VEHICLE_ID && Can_RideVehicle(player) &&
			player.eVehicleFlightPhase == LostArk::Shared::VEHICLE_FLIGHT_PHASE::FLYING &&
			std::isfinite(move.fVerticalInput) && std::abs(move.fVerticalInput) <= 1.f &&
			move.fGoalX * move.fGoalX + move.fGoalZ * move.fGoalZ <= 1.0001f)
		{
			player.fVehicleFlightInputX = move.fGoalX;
			player.fVehicleFlightInputZ = move.fGoalZ;
			player.fVehicleFlightInputY = move.fVerticalInput;
			player.fVehicleFlightInputAge = 0.f;
		}
		return;
	}
	/* Walking away is the hunter's own input, so it ends a spent hammer swing
	instead of being refused for the rest of the pose. */
	(void)Cancel_MazeHammerRecovery(player, m_iServerTick);
	if (0u != player.iMarioStage || player.bPatternBound ||
		LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp ||
		player.fKnockbackRemainingSeconds > 0.f)
	{
		if (0u != player.iCurrentHp &&
			player.fKnockbackRemainingSeconds <= 0.f &&
			Is_MoveCancellableAction(player))
		{
			/* A stance swap's move-cancel window only opens once the swap has
			happened on screen, so walking out of the tail keeps the new stance
			instead of discarding it with the action. */
			if (const PLAYER_SKILL_DEFINITION* cancelled =
				m_GameplayCatalog.Find_Skill(player.iCurrentSkillId))
			{
				CPlayerSkillSystem::Commit_StanceChange(
					player, *cancelled, m_GameplayCatalog);
			}
			/* Leave the action the same way every other release does. A cleared
			eAction that still carries the old skill id and start tick is a
			half-ended action to every later reader. */
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.iComboStage = 0u;
			player.hasBufferedComboInput = false;
			player.PendingCommand.Clear();
			player.Clear_SkillTarget();
			(void)Commit_MoveGoal(player, move.fGoalX, move.fGoalZ);
			return;
		}
		if (0u != player.iCurrentHp &&
			player.fKnockbackRemainingSeconds <= 0.f &&
			Is_BufferableComboAction(player))
			player.PendingCommand.Set_Move(move);
		return;
	}
	(void)Commit_MoveGoal(player, move.fGoalX, move.fGoalZ);
}

bool LostArk::Server::CGameRoom::Is_MoveCancellableAction(
	const SERVER_PLAYER& player) const
{
	if (LostArk::Shared::PLAYER_ACTION_STATE::SKILL != player.eAction ||
		0u == player.iCurrentHp)
	{
		return false;
	}
	const PLAYER_SKILL_DEFINITION* skill =
		m_GameplayCatalog.Find_Skill(player.iCurrentSkillId);
	return nullptr != skill && !Is_DodgeSkill(*skill) &&
		Is_InsideCancelWindow(
			*skill, player.iComboStage, player.fActionElapsedSeconds,
			PLAYER_CANCEL_INPUT::MOVE);
}

bool LostArk::Server::CGameRoom::Is_BufferableComboAction(
	const SERVER_PLAYER& player) const
{
	if (LostArk::Shared::PLAYER_ACTION_STATE::SKILL != player.eAction ||
		0u == player.iCurrentHp)
	{
		return false;
	}
	const PLAYER_SKILL_DEFINITION* skill =
		m_GameplayCatalog.Find_Skill(player.iCurrentSkillId);
	return nullptr != skill &&
		LostArk::Shared::PLAYER_SKILL_KIND::COMBO == skill->eSkillKind;
}

bool LostArk::Server::CGameRoom::Commit_MoveGoal(
	SERVER_PLAYER& player,
	const float goalX,
	const float goalZ)
{
	if (m_ServerNavigation.Is_Loaded())
	{
		/* A held right mouse re-sends its goal about twenty times a second.
		Routing each one restarted A* from a start cell that had moved, and the
		first waypoint - always a neighbouring cell centre, since the search
		drops the start cell - flipped between the two equal-cost diagonals, so
		the player wove from side to side.

		The request is compared, not the projected goal: a cursor resting on an
		obstacle is not walkable, so it projects metres away and would never
		look like the same goal twice. */
		const float driftX = goalX - player.fMoveRequestX;
		const float driftZ = goalZ - player.fMoveRequestZ;
		if (player.hasMoveGoal && !player.MovePath.empty() &&
			driftX * driftX + driftZ * driftZ <=
				MOVE_ROUTE_KEEP_DISTANCE * MOVE_ROUTE_KEEP_DISTANCE)
		{
			player.isCombatReady = true;
			return true;
		}
		/* A goal the player can walk straight at has no waypoint to flip, so
		steer at it and let the existing body sweep slide along what it
		touches. Only a closed line needs a route. */
		SERVER_NAV_POINT exactGoal{};
		if (m_ServerNavigation.Sample_Position(
			goalX, goalZ, exactGoal, player.fPositionY) &&
			m_ServerNavigation.Has_LineOfSight(
				player.fPositionX, player.fPositionZ, exactGoal.x, exactGoal.z,
				player.fPositionY))
		{
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			player.fMoveRequestX = goalX;
			player.fMoveRequestZ = goalZ;
			player.fMoveGoalX = exactGoal.x;
			player.fMoveGoalZ = exactGoal.z;
			player.hasMoveGoal = true;
			player.isCombatReady = true;
			return true;
		}
	}
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.fMoveRequestX = goalX;
	player.fMoveRequestZ = goalZ;
	if (m_ServerNavigation.Is_Loaded())
	{
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX,
			player.fPositionZ,
			goalX,
			goalZ,
			player.MovePath,
			player.fPositionY))
		{
			player.hasMoveGoal = false;
			return false;
		}
		m_ServerNavigation.Smooth_Path(
			player.fPositionX,
			player.fPositionZ,
			goalX,
			goalZ,
			player.MovePath,
			player.fPositionY);
		const SERVER_NAV_POINT& goal = player.MovePath.back();
		player.fMoveGoalX = goal.x;
		player.fMoveGoalZ = goal.z;
	}
	else
	{
		player.fMoveGoalX = goalX;
		player.fMoveGoalZ = goalZ;
	}
	player.hasMoveGoal = true;
	player.isCombatReady = true;
	return true;
}

void LostArk::Server::CGameRoom::Commit_PendingPlayerCommand(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick)
{
	if (PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind)
		return;

	const SERVER_PENDING_PLAYER_COMMAND pending = player.PendingCommand;
	player.PendingCommand.Clear();
	player.hasBufferedComboInput = false;
	if (player.bPatternBound ||
		LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp ||
		player.fKnockbackRemainingSeconds > 0.f)
	{
		return;
	}

	if (PLAYER_PENDING_COMMAND_KIND::MOVE == pending.eKind)
	{
		if (std::isfinite(pending.fX) && std::isfinite(pending.fZ) &&
			std::abs(pending.fX) <= MAX_ABS_MOVE_GOAL &&
			std::abs(pending.fZ) <= MAX_ABS_MOVE_GOAL)
		{
			(void)Commit_MoveGoal(player, pending.fX, pending.fZ);
		}
		return;
	}

	if (PLAYER_PENDING_COMMAND_KIND::SKILL == pending.eKind)
	{
		if (0u != player.iSilenceEndTick &&
			!Has_ReachedServerTick(actionStartTick, player.iSilenceEndTick))
		{
			return;
		}
		LostArk::Shared::C2S_USE_SKILL command{};
		command.iClientSequence = pending.iClientSequence;
		command.iSkillId = pending.iSkillId;
		command.eTargetIntent = pending.eTargetIntent;
		command.fAimX = pending.fX;
		command.fAimZ = pending.fZ;
		if (m_PlayerSkillSystem.Try_StartPending(
				player, command, m_GameplayCatalog, actionStartTick,
				&m_ServerNavigation, m_eCooldownMode))
		{
			player.isCombatReady = true;
			Apply_SkillBuffs(player, command.iSkillId, actionStartTick);
		}
	}
}

void LostArk::Server::CGameRoom::Handle_UseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& useSkill)
{
	if (Is_KoukuRaidInputBlocked()) return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_SKILL", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_SKILL", "missing-player-state");
		return;
	}
	/* A mounted player's quick slots belong to the vehicle; class skills never
	start from the saddle. */
	if (LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId)
	{
		(void)Try_StartVehicleSkill(playerIter->second, useSkill);
		return;
	}
	/* While a KoukuSaydon interaction HUD is up only that HUD's slots act; the
	class skills the Client no longer shows are refused here as well. */
	if (0u != playerIter->second.iMarioStage || playerIter->second.bPatternBound ||
		playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		LostArk::Shared::KOUKU_HUD_MODE::NONE != playerIter->second.eKoukuHudMode ||
		(0u != playerIter->second.iSilenceEndTick &&
		 !Has_ReachedServerTick(m_iServerTick, playerIter->second.iSilenceEndTick)))
		return;

	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase &&
		!(VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				m_ValtanTimelineAudition.ePhase &&
			playerIter->second.iPlayerId ==
				m_ValtanTimelineAudition.iOwnerPlayerId))
	{
		return;
	}

	const std::uint32_t actionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
#ifdef _DEBUG
	/* Character Select Server Arena is the presentation audition room.  Keep its
	retries Server-authoritative with full resources so a class can be auditioned
	without farming its gauge; cooldowns stay on the authored balance so an
	audition shows the real rotation.  Action-running, sequence, class, aim and
	snapshot gates remain in CPlayerSkillSystem::Try_Start. */
	if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
	{
		playerIter->second.iCurrentResource =
			playerIter->second.iMaximumResource;
		playerIter->second.iResourceAccumulator = 0u;
	}
#endif
	if (m_PlayerSkillSystem.Try_StagePendingSkill(
			playerIter->second, useSkill, m_GameplayCatalog,
			&m_ServerNavigation))
	{
		playerIter->second.isCombatReady = true;
		return;
	}
	// A valid but currently unavailable skill is rejected as gameplay state;
	// malformed payloads are already closed at the ServerApp packet boundary.
	if (m_PlayerSkillSystem.Try_Start(
		playerIter->second,
		useSkill,
		m_GameplayCatalog,
		actionStartTick,
		&m_ServerNavigation, m_eCooldownMode))
	{
		playerIter->second.isCombatReady = true;
		Apply_SkillBuffs(playerIter->second, useSkill.iSkillId, actionStartTick);
	}

}

void LostArk::Server::CGameRoom::Apply_SkillBuffs(
	SERVER_PLAYER& caster,
	const std::uint32_t skillId,
	const std::uint32_t serverTick)
{
	const CGameplayCatalog& catalog = m_GameplayCatalog.Active();
	const std::vector<CGameplayCatalog::SKILL_BUFF_DEFINITION>* found =
		catalog.Find_SkillBuffs(skillId);
#ifdef _DEBUG
	/* Which skill asked for a buff and whether the catalog had one, so a buff that
	never reaches a HUD can be placed on this side or the other. */
	std::cout << "[SkillBuff] skill=" << skillId << " tick=" << serverTick
		<< " definitions=" << (nullptr == found ? 0u : found->size()) << '\n';
#endif
	if (nullptr == found)
		return;
	std::vector<SERVER_PLAYER*> allies;
	allies.reserve(m_Players.size());
	for (auto& entry : m_Players)
		allies.push_back(&entry.second);
	/* Existing buff runtime owns boss debuffs and monster stun admission. */
	std::vector<SERVER_WORLD_ENTITY*> enemies;
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if ((WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind || WORLD_BOOTSTRAP_KIND::MONSTER == entity.eKind) && 0u != entity.iCurrentHp)
			enemies.push_back(&entity);
	}
	CServerBuffRuntime::Apply_SkillBuffs(
		catalog, skillId, caster, allies, enemies, serverTick);
}

void LostArk::Server::CGameRoom::Handle_RevivePlayer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if ((WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		 WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId) ||
		sessionIter == m_PlayerIdBySessionId.end())
	{
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(
		revivePlayer.iClientSequence, player.iLastReviveSequence))
	{
		return;
	}
	player.iLastReviveSequence = revivePlayer.iClientSequence;
	if (0u != player.iCurrentHp || PLAYER_ACTION_STATE::DEAD != player.eAction)
		return;
	// A failed destination admission keeps the Mario corpse/pin for retry;
	// revival must not bypass that mandatory return and revive inside Mario.
	if (player.iMarioStage || player.MarioReturnPosition) return;

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return;
	/* Both destinations are staged before any player state changes: Kouku
	returns to its current gate start; Valtan retains its authored safe center. */
	float reviveX = player.fPositionX;
	float reviveY = player.fPositionY;
	float reviveZ = player.fPositionZ;
	float reviveYaw = player.fYawDegrees;
	const bool unadmittedCasinoFall = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
		!Resolve_CurrentKoukuGate() && player.bKoukuFallDeath && player.KoukuFallRevivePosition;
	if (unadmittedCasinoFall)
	{
		reviveX = (*player.KoukuFallRevivePosition)[0];
		reviveY = (*player.KoukuFallRevivePosition)[1];
		reviveZ = (*player.KoukuFallRevivePosition)[2];
	}
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		const WORLD_BOOTSTRAP_PLACEMENT* arenaCenter = Find_Placement("boss.valtan.center");
		if (nullptr == arenaCenter)
			return;
		reviveX = arenaCenter->fPositionX;
		reviveY = arenaCenter->fPositionY;
		reviveZ = arenaCenter->fPositionZ;
		reviveYaw = arenaCenter->fYawDegrees;
	}
	else if (!unadmittedCasinoFall)
	{
		SERVER_NAV_POINT start{};
		if (!Resolve_KoukuRevivePosition(player, start, reviveYaw)) return;
		reviveX = start.x; reviveY = start.y; reviveZ = start.z;
	}
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(
			reviveX, reviveZ, projected, reviveY))
			return;
		// Commit the navigation-admitted gate start, never the old death location.
		reviveX = projected.x;
		reviveZ = projected.z;
		reviveY = projected.y;
	}
	// A revive command is drained before Prepare_KoukuAuditionTick. Complete
	// the failed solo mechanic while the admitted corpse is still observable.
	if (WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
		m_KoukuSaydonPatternAudition.ePhase != KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE)
	{
		for (auto& member : m_KoukuSaydonPatternAudition.Members)
		{
			if (!member.bMarioSoloReturnRequired || member.bCompletionChainSuccessQueued ||
				member.iMarioEntrantPlayerId != player.iPlayerId ||
				member.iMarioEntrantSessionId != player.iSessionId ||
				member.iMarioEntrantNetEntityId != player.iNetEntityId) continue;
			member.bCompleted = true;
			m_strStatus = "Mario solo entrant died before phase 2; mechanic completed as failed";
			Clear_KoukuSaydonPatternAudition(true);
			break;
		}
	}
	player.fPositionX = reviveX;
	player.fPositionY = reviveY;
	player.fPositionZ = reviveZ;
	player.fYawDegrees = reviveYaw;
	player.KoukuFallRevivePosition.reset();
	player.bKoukuFallDeath = false;
	player.iCurrentHp = player.iMaximumHp;
	player.iCurrentResource = player.iMaximumResource;
	player.iResourceAccumulator = 0u;
	CPlayerSkillSystem::Reset_Gauges(player, m_GameplayCatalog);
	player.iCurrentMadness = 0u; player.dMadnessRemainder = 0.;
	player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	player.Clear_KoukuInteractionState();
	if (nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.eStance = profile->eDefaultStance;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.Clear_Attachment();
	player.Clear_PatternBindStatus();
	player.Clear_SilenceStatus();
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.fFallDeathPlaneY = 0.f;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = 0.f;
	player.fSkillAimDirectionZ = 1.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.CooldownEndTickBySkillId.clear();
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.TriggerMove = {};
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = player.bArenaEjectionActive = false;
	player.fKnockbackVelocityY = player.fKnockbackLaunchY = player.fKnockbackSupportY = 0.f;
	player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	// A successful authoritative revive immediately makes the player a valid
	// combat participant again so party-wipe recovery can resume the encounter.
	player.isCombatReady = true;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
}

void LostArk::Server::CGameRoom::Handle_DebugKillSelf(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf)
{
#ifndef _DEBUG
	/* Debug/Development-build test aid only, same convention as
	Evaluate_ValtanAudition: a Release-built Server never touches gameplay
	state for this command. */
	(void)sessionId;
	(void)debugKillSelf;
	return;
#else
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (0u == player.iCurrentHp && PLAYER_ACTION_STATE::DEAD == player.eAction)
		return;

	player.iCurrentHp = 0u;
	player.eAction = PLAYER_ACTION_STATE::DEAD;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.TriggerMove = {};
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.iKnockdownEndTick = 0u;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugEnterKakulSaydonArena(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& request)
{
#ifndef _DEBUG
	(void)sessionId;
	(void)request;
	return;
#else
	using namespace LostArk::Shared;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId ||
		0u == request.iRequestSequence)
	{
		return;
	}

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& player = playerIter->second;
	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return;
	}

	const bool isAlreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId ||
				std::find(pending.PartyBatchSessionIds.begin(),
					pending.PartyBatchSessionIds.end(), sessionId) !=
				pending.PartyBatchSessionIds.end();
		});
	if (isAlreadyStaged)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = WORLD_ID::KAKULSAYDON_ARENA;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iHonorTitleId = player.iHonorTitleId;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	transfer.CarriedInventory = player.Inventory;
	m_PendingWorldTransfers.push_back(std::move(transfer));
#endif
}

void LostArk::Server::CGameRoom::Handle_ReleaseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_RELEASE_SKILL", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_RELEASE_SKILL", "missing-player-state");
		return;
	}
	if (playerIter->second.bPatternBound ||
		playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		(0u != playerIter->second.iSilenceEndTick &&
		 !Has_ReachedServerTick(m_iServerTick, playerIter->second.iSilenceEndTick)))
		return;

	m_PlayerSkillSystem.Release(
		playerIter->second,
		releaseSkill,
		m_GameplayCatalog);
}

void LostArk::Server::CGameRoom::Handle_UpdateSkillAim(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_UPDATE_SKILL_AIM", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_UPDATE_SKILL_AIM", "missing-player-state");
		return;
	}
	if (playerIter->second.bPatternBound ||
		playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		(0u != playerIter->second.iSilenceEndTick &&
		 !Has_ReachedServerTick(m_iServerTick, playerIter->second.iSilenceEndTick)))
		return;

	m_PlayerSkillSystem.Update_Aim(
		playerIter->second,
		updateSkillAim,
		m_GameplayCatalog);
}

void LostArk::Server::CGameRoom::Handle_UseSquareHole(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_SQUAREHOLE", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_SQUAREHOLE", "missing-player-state");
		return;
	}
	SERVER_PLAYER& player = playerIter->second;
	/* The song is a fixed-length lock like the Esther call: only an idle, unmounted
	player on their feet may start it. The landing is resolved before the song starts,
	so a world without one (or a blocked one) never plays a song that ends nowhere.
	Update_Players lands the player once SQUAREHOLE_LOCK_TICKS have elapsed
	(Finish_SquareHoleSong), while the Client screen is fully black. */
	if (0u == player.iCurrentHp ||
		player.fKnockbackRemainingSeconds > 0.f ||
		player.bPatternBound ||
		LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId ||
		LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		return;
	}
	SERVER_NAV_POINT landing{};
	if (!Resolve_SquareHoleDestination(player, useSquareHole.iSquareHoleId, landing))
		return;
	player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG;
	player.iSquareHoleId = useSquareHole.iSquareHoleId;
	player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
	player.iActionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.hasMoveGoal = false;
}

bool LostArk::Server::CGameRoom::Resolve_SquareHoleDestination(
	const SERVER_PLAYER& player,
	const std::uint16_t squareHoleId,
	SERVER_NAV_POINT& ground)
{
	using namespace LostArk::Shared;
	if (0u == squareHoleId)
		return false;
	/* Every map-travel destination is a disabled triggerBox with one movePlayer target,
	editable in MapTool World Gameplay. The map's Set Sail button is explicitly routed to
	the authored ship row rather than smuggling its coordinates through the packet. */
	const std::string placementId =
		WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID == squareHoleId ? "ship" :
		"squarehole." + std::to_string(squareHoleId);
	const WORLD_BOOTSTRAP_PLACEMENT* destination = Find_Placement(placementId);
	if (nullptr == destination ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != destination->eKind ||
		1u != destination->TriggerActions.size() ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != destination->TriggerActions.front().eKind)
	{
		m_strStatus = "World map travel has no movePlayer placement in this world: " + placementId;
		return false;
	}
	const WORLD_TRIGGER_ACTION& move = destination->TriggerActions.front();
	C2S_DEBUG_TELEPORT_TO_POSITION target{};
	target.eWorldId = m_eWorldId;
	target.fPositionX = move.fTargetX;
	target.fPositionY = move.fTargetY;
	target.fPositionZ = move.fTargetZ;
	/* The same walkable / height / collision admission the gate-progress party move
	uses: a landing inside a wall or on an NPC is refused, never forced. */
	const DEBUG_TELEPORT_RESULT verdict =
		Validate_DebugTeleportDestination(player, target, ground);
	if (DEBUG_TELEPORT_RESULT::ACCEPTED != verdict)
	{
		m_strStatus = "World map travel landing was refused: " + placementId +
			" result=" + std::to_string(static_cast<int>(verdict));
		return false;
	}
	return true;
}

void LostArk::Server::CGameRoom::Finish_SquareHoleSong(SERVER_PLAYER& player)
{
	const std::uint16_t squareHoleId = player.iSquareHoleId;
	player.iSquareHoleId = 0u;
	SERVER_NAV_POINT landing{};
	/* Checked again now: the landing may have been blocked while the song played.
	A refused landing leaves the player where they stand and the lock releases as
	usual. Reset_PlayerForDebugTeleport also drops any move or skill queued during
	the song, which were aimed from the old position. */
	if (0u == squareHoleId ||
		!Resolve_SquareHoleDestination(player, squareHoleId, landing))
	{
		return;
	}
	Reset_PlayerForDebugTeleport(player);
	player.fPositionX = landing.x;
	player.fPositionY = landing.y;
	player.fPositionZ = landing.z;
	Update_MarioControlState(player);
}

LostArk::Server::SERVER_PLAYER* LostArk::Server::CGameRoom::Find_EstherCaster(
	const SESSION_ID sessionId,
	const char* pCommandName)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, pCommandName, "missing-player-binding");
		return nullptr;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, pCommandName, "missing-player-state");
		return nullptr;
	}
	if (playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId)
		return nullptr;
	/* The call locks the caster into ESTHER_CAST, so only an idle caster may
	start one: a running skill, knockdown, fall or death keeps the gauge full. */
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE !=
		playerIter->second.eAction)
	{
		return nullptr;
	}
	/* The entity id is checked before the gauge so a consume can never be
	followed by a failed spawn: rejecting here leaves the gauge untouched and
	the snapshot keeps telling every party member it is still full. */
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == m_iNextNetEntityId)
		return nullptr;
	return &playerIter->second;
}

void LostArk::Server::CGameRoom::Handle_UseEstherSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill)
{
	SERVER_PLAYER* pCaster = Find_EstherCaster(sessionId, "C2S_USE_ESTHER_SKILL");
	if (nullptr == pCaster)
		return;

	const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
	if (ESTHER_USE_REJECTION::NONE != m_EstherSkillSystem.Try_Consume(
		useEstherSkill.iSlotIndex, pRosterEntry) || nullptr == pRosterEntry)
	{
		return;
	}
	Begin_EstherCall(*pCaster, *pRosterEntry, useEstherSkill.fAimX, useEstherSkill.fAimZ);
}

void LostArk::Server::CGameRoom::Handle_DebugUseEsther(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request)
{
#ifdef _DEBUG
	/* Names an Esther directly: no gauge, no roster slot. The world must still
	own an Esther contract so the cast and summon presentation are the same as
	the slot path in that arena. */
	if (request.eWorldId != m_eWorldId || !m_EstherSkillSystem.Is_Enabled())
		return;
	const ESTHER_ROSTER_ENTRY* pRosterEntry = Find_EstherDefinition(request.eEsther);
	if (nullptr == pRosterEntry)
		return;
	SERVER_PLAYER* pCaster = Find_EstherCaster(sessionId, "C2S_DEBUG_USE_ESTHER");
	if (nullptr == pCaster)
		return;
	Begin_EstherCall(*pCaster, *pRosterEntry, request.fAimX, request.fAimZ);
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Begin_EstherCall(
	SERVER_PLAYER& caster,
	const ESTHER_ROSTER_ENTRY& rosterEntry,
	const float aimX,
	const float aimZ)
{
	/* The gauge is gone now; the summon lands after the delay, forward along
	the aim. Position, height and facing are frozen here so a caster who moves
	during the delay does not drag the landing spot with them. A degenerate
	aim (cursor on the caster) keeps the caster's yaw and lands at their feet. */
	PENDING_ESTHER_SUMMON pending{};
	pending.pRosterEntry = &rosterEntry;
	pending.fPositionX = caster.fPositionX;
	pending.fPositionY = caster.fPositionY;
	pending.fPositionZ = caster.fPositionZ;
	pending.fYawDegrees = caster.fYawDegrees;
	pending.fRemainingSeconds = ESTHER_SUMMON_DELAY_SECONDS;
	const float directionX = aimX - caster.fPositionX;
	const float directionZ = aimZ - caster.fPositionZ;
	const float directionLengthSq = directionX * directionX + directionZ * directionZ;
	if (std::isfinite(directionX) && std::isfinite(directionZ) &&
		directionLengthSq > 0.0001f)
	{
		pending.fYawDegrees = std::atan2(directionX, directionZ) * RADIANS_TO_DEGREES;
		const float directionLength = std::sqrt(directionLengthSq);
		const float targetX = caster.fPositionX +
			directionX / directionLength * ESTHER_SUMMON_FORWARD_METERS;
		const float targetZ = caster.fPositionZ +
			directionZ / directionLength * ESTHER_SUMMON_FORWARD_METERS;
		SERVER_NAV_POINT landing{};
		if (m_ServerNavigation.Sample_Position(
			targetX, targetZ, landing, caster.fPositionY))
		{
			pending.fPositionX = landing.x;
			pending.fPositionY = landing.y;
			pending.fPositionZ = landing.z;
		}
	}
	m_PendingEstherSummons.push_back(pending);

	/* The caster turns to the aim and holds the call animation; Update_Players
	returns the action to NONE once ESTHER_CAST_DURATION_MS has elapsed. */
	caster.eAction = LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST;
	caster.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
	caster.iActionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	caster.fActionElapsedSeconds = 0.f;
	caster.fYawDegrees = pending.fYawDegrees;
	caster.iComboStage = 0u;
	caster.hasBufferedComboInput = false;
	caster.hasMoveGoal = false;
	caster.MovePath.clear();
	caster.iMovePathIndex = 0;
	caster.Clear_SkillTarget();
	caster.PendingCommand.Clear();

	if (LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
		LostArk::Shared::ESTHER_ID::INANNA == rosterEntry.eEstherId)
	{
		const auto until = CKoukuSaydonLogicRuntime::Add_Ticks(caster.iActionStartTick,
			CKoukuSaydonLogicRuntime::Ticks_FromMs(ESTHER_INANNA_INVULNERABLE_DURATION_MS));
		for (auto& [id, player] : m_Players)
		{
			if (!player.iCurrentHp || player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::DEAD) continue;
			if (Is_KoukuRaidRunning() && std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), id) == m_KoukuRaid.PlayerIds.end()) continue;
			if (!player.iInvulnerableEndTick || CKoukuSaydonLogicRuntime::Has_ReachedTick(until, player.iInvulnerableEndTick))
				player.iInvulnerableEndTick = until;
		}
	}
}

void LostArk::Server::CGameRoom::Update_PendingEstherSummons(
	const float fixedDeltaSeconds)
{
	for (auto iter = m_PendingEstherSummons.begin();
		iter != m_PendingEstherSummons.end();)
	{
		iter->fRemainingSeconds -= fixedDeltaSeconds;
		if (iter->fRemainingSeconds > 0.f)
		{
			++iter;
			continue;
		}
		if (nullptr != iter->pRosterEntry)
		{
			Spawn_EstherSummon(
				*iter->pRosterEntry,
				iter->fPositionX,
				iter->fPositionY,
				iter->fPositionZ,
				iter->fYawDegrees);
		}
		iter = m_PendingEstherSummons.erase(iter);
	}
}

bool LostArk::Server::CGameRoom::Spawn_EstherSummon(
	const ESTHER_ROSTER_ENTRY& rosterEntry,
	const float positionX,
	const float positionY,
	const float positionZ,
	const float yawDegrees)
{
	if (nullptr == rosterEntry.pArchetypeId ||
		'\0' == rosterEntry.pArchetypeId[0] ||
		0u == rosterEntry.iStrikeMs ||
		LostArk::Shared::INVALID_NET_ENTITY_ID == m_iNextNetEntityId)
	{
		return false;
	}

	const std::uint32_t startTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;

	const std::string archetypeId = rosterEntry.pArchetypeId;
	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = m_iNextNetEntityId;
	staged.strPlacementId =
		"esther." + archetypeId + "." + std::to_string(staged.iNetEntityId);
	staged.strArchetypeId = archetypeId;
	staged.eKind = WORLD_BOOTSTRAP_KIND::NPC;
	staged.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
	staged.strActionId = ESTHER_ACTION_STRIKE;
	staged.isEstherSummon = true;
	staged.iEstherStrikeMs = rosterEntry.iStrikeMs;
	staged.fPositionX = positionX;
	staged.fPositionY = positionY;
	staged.fPositionZ = positionZ;
	staged.fYawDegrees = yawDegrees;
	staged.iActionStartTick = startTick;
	staged.iCurrentHp = 1u;
	staged.iMaximumHp = 1u;
	staged.PinnedDefinitionRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!staged.PinnedDefinitionRevision.Is_Valid())
		return false;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT
LostArk::Server::CGameRoom::Apply_CharacterClassChange(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD;
	if (!Is_NewerSequence(
		request.iClientSequence, player.iLastClassChangeSequence))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE;
	}
	if (!Is_Supported_Playable_Character_Class(request.eCharacterClass) ||
		nullptr == m_GameplayCatalog.Find_Player(request.eCharacterClass))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_UNSUPPORTED_CLASS;
	}
	if (request.eCharacterClass == player.eCharacterClass)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS;

	const bool isDead = 0u == player.iCurrentHp &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;
	if ((0u == player.iCurrentHp) !=
		(PLAYER_ACTION_STATE::DEAD == player.eAction))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
	}

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(request.eCharacterClass);
	SERVER_PLAYER staged = player;
	if (isDead)
	{
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			Find_Placement(player.strSpawnPlacementId);
		if (nullptr == spawn || !spawn->isEnabled ||
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn->eKind)
		{
			return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
		}
		staged.fPositionX = spawn->fPositionX;
		staged.fPositionY = spawn->fPositionY;
		staged.fPositionZ = spawn->fPositionZ;
		staged.fYawDegrees = spawn->fYawDegrees;
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX, staged.fPositionZ, projected,
				staged.fPositionY))
			{
				return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}

	staged.eCharacterClass = request.eCharacterClass;
	staged.iLastClassChangeSequence = request.iClientSequence;
	staged.fMoveGoalX = 0.f;
	staged.fMoveGoalZ = 0.f;
	staged.fMoveSpeed = profile->fMoveSpeed;
	staged.hasMoveGoal = false;
	staged.MovePath.clear();
	staged.iMovePathIndex = 0u;
	staged.iCurrentHp = profile->iMaximumHp;
	staged.iMaximumHp = profile->iMaximumHp;
	staged.iCurrentResource = profile->iMaximumResource;
	staged.iMaximumResource = profile->iMaximumResource;
	staged.iResourceAccumulator = 0u;
	staged.iCurrentMadness = 0u; staged.dMadnessRemainder = 0.;
	staged.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	staged.Clear_MarioControl();
	staged.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	End_VehicleSkill(staged);
	staged.iVehicleId = INVALID_VEHICLE_ID;
	staged.Clear_KoukuInteractionState();
	staged.eAction = PLAYER_ACTION_STATE::NONE;
	staged.eStance = profile->eDefaultStance;
	staged.iCurrentSkillId = INVALID_SKILL_ID;
	staged.Clear_SkillTarget();
	staged.iActionStartTick = 0u;
	staged.Clear_Attachment();
	staged.Clear_PatternBindStatus();
	staged.Clear_SilenceStatus();
	staged.TriggerMove = {};
	staged.fKnockbackRemainingSeconds = 0.f;
	staged.fKnockbackSpeed = 0.f;
	staged.iKnockdownEndTick = 0u;
	staged.iHitReactionGraceEndTick = 0u;
	staged.fActionElapsedSeconds = 0.f;
	staged.fSkillAimDirectionX = 0.f;
	staged.fSkillAimDirectionZ = 1.f;
	staged.hasAppliedSkillDamage = false;
	staged.iAppliedHitMask = 0;
	staged.iSpawnedProjectileMask = 0;
	staged.Projectiles.clear();
	staged.iComboStage = 0u;
	staged.hasBufferedComboInput = false;
	staged.PendingCommand.Clear();
	staged.hasReleasedHold = false;
	staged.CooldownEndTickBySkillId.clear();
	staged.isCombatReady = true;

	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player = std::move(staged);
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED;
}

void LostArk::Server::CGameRoom::Handle_ChangeCharacterClass(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	const CHARACTER_CLASS_CHANGE_RESULT result =
		Apply_CharacterClassChange(player, request);
	if (!Send_CharacterClassChangeResult(
		session, request, result, player.eCharacterClass))
	{
		session->Request_Close();
		return;
	}
	/* The new class cannot wear another class's gear. */
	if (Unequip_OtherClassItems(player) &&
		!Send_InventorySnapshot(session, 0u, player.Inventory))
	{
		session->Request_Close();
	}
}


LostArk::Shared::SET_COOLDOWN_MODE_RESULT LostArk::Server::CGameRoom::Apply_SetCooldownMode(
    const SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request)
{
    using namespace LostArk::Shared;
    if (request.eWorldId != m_eWorldId || request.eMode >= COOLDOWN_MODE::END) return SET_COOLDOWN_MODE_RESULT::WRONG_WORLD;
    const auto owner = m_PlayerIdBySessionId.find(sessionId);
    if (owner == m_PlayerIdBySessionId.end() || !m_Players.contains(owner->second)) return SET_COOLDOWN_MODE_RESULT::INVALID_PLAYER;
    auto& last = m_CooldownModeRequestSequences[sessionId];
    if (!request.iRequestSequence || static_cast<std::int32_t>(request.iRequestSequence - last) <= 0)
        return SET_COOLDOWN_MODE_RESULT::STALE_REQUEST;
    last = request.iRequestSequence;
    if (m_eCooldownMode != request.eMode)
    {
        for (auto& [id, player] : m_Players)
            CPlayerSkillSystem::Recalculate_Cooldowns(player, m_GameplayCatalog.Active(), m_iServerTick, request.eMode);
        m_eCooldownMode = request.eMode;
    }
    return SET_COOLDOWN_MODE_RESULT::ACCEPTED;
}

void LostArk::Server::CGameRoom::Handle_SetCooldownMode(
    const SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request)
{
    using namespace LostArk::Shared;
    S2C_SET_COOLDOWN_MODE_RESULT result{};
    result.iRequestSequence = request.iRequestSequence; result.eWorldId = m_eWorldId;
    result.eResult = Apply_SetCooldownMode(sessionId, request); result.eMode = m_eCooldownMode;
    const auto session = Find_Session(sessionId); CPacketWriter writer;
    if (session && Write_Message(writer, result) &&
        !session->Send_Frame(PACKET_TYPE::S2C_SET_COOLDOWN_MODE_RESULT, writer.Get_Buffer())) session->Request_Close();
}
