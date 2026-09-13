#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

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
#ifdef _DEBUG
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase &&
		!(VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				m_ValtanTimelineAudition.ePhase &&
			player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId))
	{
		return;
	}
#endif
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
			Is_BufferableComboAction(player))
			player.PendingCommand.Set_Move(move);
		return;
	}
	(void)Commit_MoveGoal(player, move.fGoalX, move.fGoalZ);
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
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	if (m_ServerNavigation.Is_Loaded())
	{
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX,
			player.fPositionZ,
			goalX,
			goalZ,
			player.MovePath))
		{
			player.hasMoveGoal = false;
			return false;
		}
		m_ServerNavigation.Smooth_Path(
			player.fPositionX,
			player.fPositionZ,
			goalX,
			goalZ,
			player.MovePath);
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
				&m_ServerNavigation))
		{
			player.isCombatReady = true;
		}
	}
}

void LostArk::Server::CGameRoom::Handle_UseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& useSkill)
{
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
	/* While a KoukuSaydon interaction HUD is up only that HUD's slots act; the
	class skills the Client no longer shows are refused here as well. */
	if (0u != playerIter->second.iMarioStage || playerIter->second.bPatternBound ||
		playerIter->second.fKnockbackRemainingSeconds > 0.f ||
		LostArk::Shared::KOUKU_HUD_MODE::NONE != playerIter->second.eKoukuHudMode ||
		(0u != playerIter->second.iSilenceEndTick &&
		 !Has_ReachedServerTick(m_iServerTick, playerIter->second.iSilenceEndTick)))
		return;

#ifdef _DEBUG
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
#endif

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
		&m_ServerNavigation))
	{
		playerIter->second.isCombatReady = true;
	}
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

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return;
	/* Kouku revives at the death position. Valtan retains its authored safe
	center. Navigation admission is staged before any player state changes. */
	float reviveX = player.fPositionX;
	float reviveY = player.fPositionY;
	float reviveZ = player.fPositionZ;
	float reviveYaw = player.fYawDegrees;
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
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(reviveX, reviveZ, projected))
			return;
		// Project_Point provides a safe floor if the death location is unwalkable.
		if (WORLD_ID::VALTAN_ARENA == m_eWorldId ||
			!m_ServerNavigation.Is_PointWalkableExact(reviveX, reviveZ))
		{
			reviveX = projected.x;
			reviveZ = projected.z;
		}
		reviveY = projected.y;
	}
	player.fPositionX = reviveX;
	player.fPositionY = reviveY;
	player.fPositionZ = reviveZ;
	player.fYawDegrees = reviveYaw;
	player.iCurrentHp = player.iMaximumHp;
	player.iCurrentResource = player.iMaximumResource;
	player.iResourceAccumulator = 0u;
	player.iCurrentIdentity = player.iMaximumIdentity;
	player.iIdentityAccumulator = 0u;
	player.iCurrentMadness = 0u;
	player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	player.Clear_KoukuInteractionState();
#ifdef _DEBUG
	if (nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
#endif
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

void LostArk::Server::CGameRoom::Handle_UseEstherSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_ESTHER_SKILL", "missing-player-binding");
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
	{
		Close_SessionForBindingFailure(
			sessionId, "C2S_USE_ESTHER_SKILL", "missing-player-state");
		return;
	}
	if (playerIter->second.fKnockbackRemainingSeconds > 0.f)
		return;
	/* The call locks the caster into ESTHER_CAST, so only an idle caster may
	start one: a running skill, knockdown, fall or death keeps the gauge full. */
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE !=
		playerIter->second.eAction)
	{
		return;
	}
	/* The entity id is checked before the gauge so a consume can never be
	followed by a failed spawn: rejecting here leaves the gauge untouched and
	the snapshot keeps telling every party member it is still full. */
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == m_iNextNetEntityId)
		return;

	const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
	if (ESTHER_USE_REJECTION::NONE != m_EstherSkillSystem.Try_Consume(
		useEstherSkill.iSlotIndex, pRosterEntry) || nullptr == pRosterEntry)
	{
		return;
	}

	/* The gauge is gone now; the summon lands after the delay, forward along
	the aim. Position, height and facing are frozen here so a caster who moves
	during the delay does not drag the landing spot with them. A degenerate
	aim (cursor on the caster) keeps the caster's yaw and lands at their feet. */
	SERVER_PLAYER& caster = playerIter->second;
	PENDING_ESTHER_SUMMON pending{};
	pending.pRosterEntry = pRosterEntry;
	pending.fPositionX = caster.fPositionX;
	pending.fPositionY = caster.fPositionY;
	pending.fPositionZ = caster.fPositionZ;
	pending.fYawDegrees = caster.fYawDegrees;
	pending.fRemainingSeconds = ESTHER_SUMMON_DELAY_SECONDS;
	const float directionX = useEstherSkill.fAimX - caster.fPositionX;
	const float directionZ = useEstherSkill.fAimZ - caster.fPositionZ;
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
		if (m_ServerNavigation.Sample_Position(targetX, targetZ, landing))
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
				staged.fPositionX, staged.fPositionZ, projected))
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
	staged.iCurrentMadness = 0u;
	staged.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	staged.Clear_MarioControl();
	staged.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
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
	}
}
