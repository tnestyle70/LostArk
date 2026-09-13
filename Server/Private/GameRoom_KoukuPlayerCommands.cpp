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

void LostArk::Server::CGameRoom::Handle_DebugTeleportToPlacement(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request)
{
#ifndef _DEBUG
	(void)sessionId;
	(void)request;
	return;
#else
	using namespace LostArk::Shared;
	if (request.strPlacementId == "player.spawn.kakul.party01")
	{
		const auto session = Find_Session(sessionId);
		if (!session) return;
		S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
		result.iRequestSequence = request.iRequestSequence; result.eWorldId = m_eWorldId;
		const auto owner = m_PlayerIdBySessionId.find(sessionId);
		const auto player = owner != m_PlayerIdBySessionId.end() ? m_Players.find(owner->second) : m_Players.end();
		if (player != m_Players.end() && player->second.iSessionId == sessionId)
			result = Apply_DebugReturnToKoukuStart(player->second, request.iRequestSequence);
		SERVER_ROOM_RUNTIME_FAILURE failure{};
		if (Try_GetRuntimeFailure(failure))
		{ session->Request_Close(); return; }
		CPacketWriter writer;
		if (!Write_Message(writer, result) || !session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, writer.Get_Buffer()))
			session->Request_Close();
		return;
	}
	constexpr const char* STAGE_WAYPOINT_PREFIX = "stage.kakul.";
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId ||
		0u == request.iRequestSequence ||
		0u != request.strPlacementId.rfind(STAGE_WAYPOINT_PREFIX, 0u))
	{
		return;
	}

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	SERVER_PLAYER& player = playerIter->second;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* waypoint =
		Find_Placement(request.strPlacementId);
	if (nullptr == waypoint ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != waypoint->eKind ||
		!m_ServerCollisionSystem.Is_PlayerSpawnClear(*waypoint) ||
		!m_ServerNavigation.Is_PointWalkableExact(
			waypoint->fPositionX, waypoint->fPositionZ))
	{
		return;
	}
	SERVER_NAV_POINT ground{};
	if (!m_ServerNavigation.Sample_Position(
		waypoint->fPositionX, waypoint->fPositionZ, ground))
	{
		return;
	}

	player.fPositionX = ground.x;
	player.fPositionY = ground.y;
	player.fPositionZ = ground.z;
	player.fYawDegrees = waypoint->fYawDegrees;
	Reset_PlayerForDebugTeleport(player);
	Update_MarioControlState(player);
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoFill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	/* The board only exists in the Kouku arena, and only a session that owns
	a player in this room may paint it. */
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.end() == m_PlayerIdBySessionId.find(sessionId))
	{
		return;
	}
	if (request.bReset)
		m_KoukuBingo.Reset();
	m_KoukuBingo.Fill(request.iCellMask);
	m_strStatus = "Bingo board filled by Debug request";
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoBomb(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	/* Same admission as the board fill: the Kouku arena only, and only a
	session that owns a living player in this room. */
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId)
	{
		return;
	}
	const auto session = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == session)
		return;
	const auto player = m_Players.find(session->second);
	if (m_Players.end() == player || 0u == player->second.iCurrentHp)
		return;
	/* The mark duration converted to fixed 30 Hz room ticks. */
	const std::uint32_t detonateTick = m_iServerTick +
		(KOUKU_BINGO_BOMB_MARK_MS * SERVER_TICK_HZ + 999u) / 1000u;
	m_strStatus = m_KoukuBingo.Start_Bomb(
		player->second.iNetEntityId, detonateTick) ?
		"Bingo bomb marked by Debug request" :
		"Bingo bomb refused: already marked or no free slot";
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoHammer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.end() == m_PlayerIdBySessionId.find(sessionId))
	{
		return;
	}
	/* One of the twenty row/column ends. The diagonals stay a bingo rule
	and are never swept. */
	const std::int32_t anchor = static_cast<std::int32_t>(
		Mix_DeterministicRandom(
			static_cast<std::uint64_t>(m_iServerTick) * 1099511628211ull +
			static_cast<std::uint64_t>(sessionId)) %
		static_cast<std::uint64_t>(KOUKU_BINGO_HAMMER_ANCHOR_COUNT));
	/* The motion is authored: one World Sequence template per sweep direction
	and one instance per anchor, so naming the instance is all the presentation
	needs. The phase clock below stays authoritative - the sequence shows those
	same five seconds rather than deciding them. */
	if (m_KoukuBingo.Start_Hammer(anchor, m_iServerTick,
		m_iServerTick +
			(KOUKU_BINGO_HAMMER_RAISE_MS * SERVER_TICK_HZ + 999u) / 1000u))
	{
		Broadcast_WorldSequencePlay(
			"world.sequence.instance.kouku.bingo.hammer.anchor." +
			std::to_string(anchor));
		m_strStatus = "Bingo hammer started by Debug request";
	}
	else
	{
		m_strStatus = "Bingo hammer refused: one is already running";
	}
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Update_KoukuBingo(const std::uint32_t tick)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId)
		return;
	(void)m_KoukuBingo.Advance_Hammer(tick);
	const auto& bombs = m_KoukuBingo.Get_Bombs();
	for (std::size_t slot = 0u; slot < bombs.size(); ++slot)
	{
		if (BINGO_BOMB_PHASE::PLANTED == bombs[slot].ePhase)
		{
			/* The fuse. Painting the cross is the bomb's whole effect, so the
			slot is freed in the same step; the cells it lit stay on the board,
			and Fill promotes any line the cross completed. */
			if (tick < bombs[slot].iDetonateTick)
				continue;
			m_KoukuBingo.Detonate(Kouku_BingoCrossMask(Kouku_BingoCellAt(
				bombs[slot].fPositionX, bombs[slot].fPositionZ)));
			m_KoukuBingo.Clear_Bomb(slot);
			continue;
		}
		if (BINGO_BOMB_PHASE::MARKED != bombs[slot].ePhase)
			continue;
		/* The carrier owns the drop position, so a carrier that died or
		left cancels its own mark instead of planting at a stale place. */
		const SERVER_PLAYER* carrier = nullptr;
		for (const auto& entry : m_Players)
		{
			if (entry.second.iNetEntityId == bombs[slot].iCarrierNetEntityId &&
				0u != entry.second.iCurrentHp)
			{
				carrier = &entry.second;
				break;
			}
		}
		if (nullptr == carrier)
		{
			m_KoukuBingo.Clear_Bomb(slot);
			continue;
		}
		if (tick >= bombs[slot].iDetonateTick)
		{
			m_KoukuBingo.Plant_Bomb(slot, carrier->fPositionX, carrier->fPositionZ,
				tick + (KOUKU_BINGO_BOMB_FUSE_MS * SERVER_TICK_HZ + 999u) / 1000u);
			/* The burning bomb is authored. One instance per slot, because a
			second play of the same instance restarts it in place, and the plant
			point rides the cue as its position offset. The fuse above stays
			authoritative; the sequence shows those same two seconds. */
			Broadcast_WorldSequencePlay(
				"world.sequence.instance.kouku.bingo.bomb.planted.slot." +
				std::to_string(slot), 1.f,
				carrier->fPositionX, 0.f, carrier->fPositionZ);
		}
	}
}

void LostArk::Server::CGameRoom::Handle_DebugSetMadnessForm(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_SET_MADNESS_FORM_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugMadnessForm(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_SET_MADNESS_FORM_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT
LostArk::Server::CGameRoom::Apply_DebugMadnessForm(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_SET_MADNESS_FORM_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eActiveForm = player.eMadnessForm;
#ifndef _DEBUG
	result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugMadnessFormResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_MADNESS_FORM_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugMadnessFormResult = result;
		return result;
	};
	/* The form only swaps the presented body. A dead, falling, grabbed or
	pattern-bound player keeps the body its current action was authored on. */
	if (0u != player.iMarioStage || 0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return reject(DEBUG_MADNESS_FORM_RESULT::REJECTED_PLAYER_STATE);
	if (request.eForm == player.eMadnessForm)
		return reject(DEBUG_MADNESS_FORM_RESULT::REJECTED_SAME_FORM);
	player.Clear_KoukuInteractionState();
	player.eMadnessForm = request.eForm;
	if (PLAYER_MADNESS_FORM::NORMAL == request.eForm &&
		nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.TriggerMove = {};
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.PendingCommand.Clear();
	result.eResult = DEBUG_MADNESS_FORM_RESULT::ACCEPTED;
	result.eActiveForm = player.eMadnessForm;
	player.LastDebugMadnessFormResult = result;
	return result;
#endif
}

void LostArk::Server::CGameRoom::Handle_InteractionSlot(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_INTERACTION_SLOT& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player == m_Players.end() || player->second.iSessionId != sessionId ||
		request.eWorldId != m_eWorldId ||
		!Is_Valid_InteractionSlot(request.eSlot) ||
		KOUKU_HUD_MODE::NONE == player->second.eKoukuHudMode)
		return;
	SERVER_PLAYER& state = player->second;
	if (KOUKU_HUD_MODE::MARIO == state.eKoukuHudMode &&
		(0u == state.iMarioStage || !state.isCombatReady || state.TriggerMove.isActive ||
		 (state.iSilenceEndTick && !Has_ReachedServerTick(m_iServerTick, state.iSilenceEndTick))))
		return;
	const std::int8_t skillIndex =
		state.ModeSkillIndexBySlot[static_cast<std::size_t>(request.eSlot)];
	if (skillIndex < 0)
	{
		m_strStatus = "KoukuSaydon interaction slot is empty in the current HUD mode";
		return;
	}
	/* Every interaction skill is one fixed-length action today: the press is
	refused while any action runs, exactly like a class skill, and Update_Players
	returns the action to NONE after KOUKU_INTERACTION_ACTION_MS. */
	/* The next swing is also the hunter's own input, so it may cut a spent
	one short; the shortened MAZE cooldown below is what paces the chain. */
	(void)Cancel_MazeHammerRecovery(state, m_iServerTick);
	if (state.bPatternBound || state.fKnockbackRemainingSeconds > 0.f ||
		0u == state.iCurrentHp || PLAYER_ACTION_STATE::NONE != state.eAction)
	{
		m_strStatus = "KoukuSaydon interaction slot refused while another action runs";
		return;
	}
	if (!Is_NewerSequence(request.iRequestSequence, state.iLastKoukuInteractionSequence))
		return;
	state.iLastKoukuInteractionSequence = request.iRequestSequence;
	const SKILL_ID cooldownId = Kouku_InteractionCooldownSkillId(
		state.eKoukuHudMode, static_cast<std::uint32_t>(skillIndex));
	const auto cooldown = state.CooldownEndTickBySkillId.find(cooldownId);
	if (state.CooldownEndTickBySkillId.end() != cooldown &&
		!CKoukuSaydonLogicRuntime::Has_ReachedTick(m_iServerTick, cooldown->second))
		return;
	state.eAction = PLAYER_ACTION_STATE::INTERACTION;
	state.iCurrentSkillId = static_cast<SKILL_ID>(skillIndex);
	state.iActionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	/* The maze hammer chains at swing speed; every other mode keeps the pose
	cooldown it was authored with. */
	state.CooldownEndTickBySkillId[cooldownId] = CKoukuSaydonLogicRuntime::Add_Ticks(
		state.iActionStartTick,
		CKoukuSaydonLogicRuntime::Ticks_FromMs(
			KOUKU_HUD_MODE::MAZE == state.eKoukuHudMode ?
				KOUKU_MAZE_HAMMER_COOLDOWN_MS : KOUKU_INTERACTION_COOLDOWN_MS));
	state.fActionElapsedSeconds = 0.f;
	state.iComboStage = 0u;
	state.hasBufferedComboInput = false;
	state.hasMoveGoal = false;
	state.MovePath.clear();
	state.iMovePathIndex = 0;
	state.Clear_SkillTarget();
	state.PendingCommand.Clear();
	m_strStatus = "KoukuSaydon interaction slot started mode skill " +
		std::to_string(skillIndex);
#ifdef _DEBUG
	/* The dance answer is judged only while an authored POSE_INPUT window of
	the running pattern is open; the Debug dance override plays the pose alone. */
	if (KOUKU_HUD_MODE::DANCE == state.eKoukuHudMode &&
		KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ==
			m_KoukuSaydonPatternAudition.ePhase &&
		nullptr != Active_KoukuPlayerLedger())
	{
		const CGameplayCatalog* pinnedCatalog = Resolve_KoukuProductCatalog();
		std::string status;
		const BOSS_PATTERN_DEFINITION* pattern = nullptr == pinnedCatalog ? nullptr :
			CKoukuSaydonBrain::Find_AnimationOnlyPattern(
				*pinnedCatalog, Active_KoukuPlayerLedger()->strPatternId, status);
		if (nullptr != pattern)
		{
			(void)CKoukuSaydonLogicRuntime::Record_InteractionSlot(
				*Active_KoukuPlayerLedger(), *pattern, state,
				request.eSlot, status);
			m_strStatus = "KoukuSaydon dance input: " + status;
		}
	}
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugSetKoukuHudMode(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugKoukuHudMode(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT
LostArk::Server::CGameRoom::Apply_DebugKoukuHudMode(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eActiveOverride = player.eDebugKoukuHudModeOverride;
#ifndef _DEBUG
	result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugKoukuHudModeResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	/* The clown HUD comes from the madness form alone. Mario, maze and the
	dance party may be forced so the HUD and its slot clips can be checked
	before the gimmick that owns them exists; a running dance pattern still
	wins over the forced dance layout. */
	if (KOUKU_HUD_MODE::NONE != request.eMode &&
		KOUKU_HUD_MODE::POLYMORPH != request.eMode &&
		KOUKU_HUD_MODE::MARIO != request.eMode &&
		KOUKU_HUD_MODE::MAZE != request.eMode &&
		KOUKU_HUD_MODE::DANCE != request.eMode)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_UNSUPPORTED_MODE;
		player.LastDebugKoukuHudModeResult = result;
		return result;
	}
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_UNSUPPORTED_MODE;
		player.LastDebugKoukuHudModeResult = result;
		return result;
	}
	player.Clear_KoukuInteractionState();
	if (nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
	player.eMadnessForm = (KOUKU_HUD_MODE::NONE == request.eMode || KOUKU_HUD_MODE::MAZE == request.eMode) ?
		PLAYER_MADNESS_FORM::NORMAL : PLAYER_MADNESS_FORM::CLOWN;
	player.iMadnessFormEndTick = 0u;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.TriggerMove = {};
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.PendingCommand.Clear();
	player.eDebugKoukuHudModeOverride = request.eMode;
	result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::ACCEPTED;
	result.eActiveOverride = player.eDebugKoukuHudModeOverride;
	player.LastDebugKoukuHudModeResult = result;
	return result;
#endif
}

void LostArk::Server::CGameRoom::Update_KoukuPlayerModes(
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	const BOSS_ENCOUNTER_MADNESS_POLICY* policy =
		WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId ?
			m_GameplayCatalog.Active().Find_KoukuMadnessPolicy(
				std::string(KOUKUSAYDON_G1_ENCOUNTER_ID)) :
			nullptr;
	const KOUKUSAYDON_LOGIC_LEDGER* ledger = nullptr;
#ifdef _DEBUG
	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ==
		m_KoukuSaydonPatternAudition.ePhase)
	{
		ledger = Active_KoukuPlayerLedger();
		if (const auto* product = Resolve_KoukuProductCatalog())
			policy = product->Find_KoukuMadnessPolicy(std::string(KOUKUSAYDON_G1_ENCOUNTER_ID));
	}
#endif
	CKoukuSaydonLogicRuntime::Update_PlayerModes(
		m_Players, ledger, policy, serverTick);

}

void LostArk::Server::CGameRoom::Apply_KoukuGateEntryCard(
	SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId)
		return;
	player.Clear_KoukuAssignedCard();
	if ("BOSS_KAKULSAYDON_G1_KOUKU" == boss.strArchetypeId ||
		"BOSS_KAKULSAYDON_G1_SAYDON" == boss.strArchetypeId)
		CKoukuSaydonLogicRuntime::Assign_EncounterCard(player, boss.iNetEntityId, m_iServerTick);
}

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Apply_KoukuLogicOutput(
	const KOUKUSAYDON_LOGIC_OUTPUT& output,
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	auto* member = Find_KoukuAuditionMember(boss.iNetEntityId);
	if (!member) return false;
    for (const auto& request : output.CaptureRequests)
    {
        const auto player = m_PlayerIdByEntityId.find(request.iPlayerNetEntityId);
        if (player == m_PlayerIdByEntityId.end() || request.iWindowIndex >= member->LogicLedger.Windows.size() ||
            member->LogicLedger.iPatternSequence != boss.iPatternSequence ||
            member->LogicLedger.Windows[request.iWindowIndex].iHoldEndTick != request.iHoldEndTick)
        { m_strStatus = "Kouku capture candidate lost its owning Hold window"; continue; }
        if (Capture_PlayerAttachment(request.iPlayerNetEntityId, boss.iNetEntityId,
            request.eAttachmentSlot, serverTick, request.iHoldEndTick))
        {
            member->LogicLedger.Windows[request.iWindowIndex].Answers[player->second] = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
            member->LogicLedger.Windows[request.iWindowIndex].InsidePlayers.insert(player->second);
        }
        else
            m_strStatus = "Kouku capture candidate was no longer eligible at room commit";
    }
	for (const auto& trigger : output.MechanicTriggers)
		m_PendingKoukuMechanicTriggers.push_back({ boss.iNetEntityId, boss.iPatternSequence, trigger });
	for (const KOUKUSAYDON_LOGIC_WORLD_PLAY& play : output.WorldSequencePlays)
	{
		S2C_WORLD_SEQUENCE_PLAY message;
		message.strSequenceInstanceId = play.strInstanceId; message.strTargetSequenceInstanceId = play.strTargetSequenceInstanceId;
		message.fPlaybackSpeed = play.fPlaybackSpeed; message.fPositionOffsetX = play.fPositionOffsetX;
		message.fPositionOffsetY = play.fPositionOffsetY; message.fPositionOffsetZ = play.fPositionOffsetZ; message.iDurationMs = play.iDurationMs;
		message.iRunEpoch = m_KoukuSaydonPatternAudition.iRoomAuditionEpoch; message.strMemberId = member->strMemberId;
		message.strCueId = "world." + std::to_string(boss.iPatternSequence) + "." + std::to_string(member->iNextWorldCue++);
		message.strOccurrenceId = play.strOccurrenceId;
		message.iStartTick = play.iStartTick ? play.iStartTick : serverTick; message.iServerTick = serverTick;
		message.iBossNetEntityId = boss.iNetEntityId; message.iPatternSequence = boss.iPatternSequence;
		if (play.Placement)
		{
			const auto& placement = *play.Placement; message.bHasPlacement = true;
			message.fWorldPositionX = placement.fPositionX; message.fWorldPositionY = placement.fPositionY; message.fWorldPositionZ = placement.fPositionZ;
			message.fWorldRotationXDegrees = placement.fRotationXDegrees; message.fWorldRotationYDegrees = placement.fRotationYDegrees; message.fWorldRotationZDegrees = placement.fRotationZDegrees;
			message.fWorldScaleX = placement.fScaleX; message.fWorldScaleY = placement.fScaleY; message.fWorldScaleZ = placement.fScaleZ;
		}
		if (!play.strTargetSequenceInstanceId.empty())
		{
			const auto& cues = play.strTargetWorldOccurrenceId.empty() ? member->WorldCueByInstance : member->WorldCueByOccurrence;
			const auto target = cues.find(play.strTargetWorldOccurrenceId.empty() ? play.strTargetSequenceInstanceId : play.strTargetWorldOccurrenceId);
			if (target == cues.end()) { m_strStatus = "World motion target has no owned occurrence"; continue; }
			message.strTargetCueId = target->second;
		}
		else
		{
			member->WorldCueByInstance[play.strInstanceId] = message.strCueId;
			if (!play.strOccurrenceId.empty()) member->WorldCueByOccurrence[play.strOccurrenceId] = message.strCueId;
		}
		m_KoukuSaydonPatternAudition.WorldPlays.push_back(message);
		Broadcast_OwnedWorldSequence(message);
	}
	/* A follow-up joins the audition right after the running slot, with a
	one-tick transition, so Play All and Play Selected both continue into it. */
	std::size_t insertAt = member->iPatternIndex + 1u;
	for (const std::string& followup : output.FollowupPatternIds)
	{
		if (insertAt > member->PatternIds.size())
			break;
		member->PatternIds.insert(
			member->PatternIds.begin() + insertAt, followup);
		member->TransitionTicks.insert(
			member->TransitionTicks.begin() + (insertAt - 1u), 1u);
		++insertAt;
	}
	if (!output.strStatus.empty())
		m_strStatus = "KoukuSaydon logic: " + output.strStatus;
	if (!output.bEndPatternEarly)
		return false;
	m_KoukuSaydonBrain.Complete_Pattern(boss, serverTick);
	return true;
}
#endif

LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT
LostArk::Server::CGameRoom::Apply_DebugReturnToKoukuStart(SERVER_PLAYER& player, const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = requestSequence; result.eWorldId = m_eWorldId;
#ifndef _DEBUG
	result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA)
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD; return result; }
	if (requestSequence != 0u && requestSequence == player.LastDebugTeleportResult.iRequestSequence)
		return player.LastDebugTeleportResult;
	if (!Is_NewerSequence(requestSequence, player.LastDebugTeleportResult.iRequestSequence))
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE; return result; }
	const auto* spawn = Find_Placement("player.spawn.kakul.party01");
	if (!spawn || spawn->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN || !spawn->isEnabled)
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION; return result; }
	// Stage fresh once/inside trigger state before changing any live state.
	CServerTriggerSystem triggers;
	std::string status;
	if (!triggers.Initialize(m_WorldBootstrap.Get_Placements(), status))
	{ m_strStatus = status; result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED; return result; }
	C2S_DEBUG_TELEPORT_TO_POSITION placement{};
	placement.iRequestSequence = requestSequence; placement.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
	placement.fPositionX = spawn->fPositionX; placement.fPositionY = spawn->fPositionY; placement.fPositionZ = spawn->fPositionZ;
	result = Apply_DebugTeleportToPosition(player, placement);
	if (result.eResult != DEBUG_TELEPORT_RESULT::ACCEPTED) return result;
	// Navigation, height, collision and player state were accepted. This explicit
	// shared-arena reset removes boss-owned state; other players are not teleported.
	player.fYawDegrees = spawn->fYawDegrees;
	Clear_KoukuSaydonPatternAudition();
	if (!Despawn_KoukuSaydonArenaDebugEntities(true))
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED;
		player.LastDebugTeleportResult = result;
		Mark_RuntimeFailure("kouku-start.boss-reset");
		return result;
	}
	m_ServerTriggerSystem = std::move(triggers);
	std::unordered_set<std::string> entrySequences;
	for (const auto& trigger : m_WorldBootstrap.Get_Placements())
		for (const auto& action : trigger.TriggerActions)
			if (action.eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE && entrySequences.insert(action.strTargetId).second)
				Broadcast_WorldSequencePlay(action.strTargetId, 1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
	Reset_CardMaze();
	m_KoukuBingo.Reset();
	player.Clear_KoukuInteractionState();
	player.Clear_KoukuAssignedCard();
	m_strStatus = "KoukuSaydon arena bosses and entry triggers reset; requesting player returned to authored start.";
	return result;
#endif
}

void LostArk::Server::CGameRoom::Reset_PlayerForDebugTeleport(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	player.Clear_MarioControl();
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.Clear_Attachment();
	player.Clear_PatternBindStatus();
	player.Clear_SilenceStatus();
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0u;
	player.iSpawnedProjectileMask = 0u;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = true;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
}

void LostArk::Server::CGameRoom::Handle_DebugTeleportToPosition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request)
{
	using namespace LostArk::Shared;
	const auto session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugTeleportToPosition(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT
LostArk::Server::CGameRoom::Apply_DebugTeleportToPosition(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
#ifndef _DEBUG
	(void)player;
	result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugTeleportResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_TELEPORT_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugTeleportResult = result;
		return result;
	};
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return reject(DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE);
	if (!std::isfinite(request.fPositionX) || !std::isfinite(request.fPositionY) ||
		!std::isfinite(request.fPositionZ) || std::abs(request.fPositionX) > 100000.f ||
		std::abs(request.fPositionY) > 100000.f || std::abs(request.fPositionZ) > 100000.f)
		return reject(DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION);
	SERVER_NAV_POINT ground{};
	if (!m_ServerNavigation.Is_PointWalkableExact(request.fPositionX, request.fPositionZ) ||
		!m_ServerNavigation.Sample_Position(request.fPositionX, request.fPositionZ, ground))
		return reject(DEBUG_TELEPORT_RESULT::REJECTED_NAVIGATION);
	/* The single-layer Server grid owns Y. A picked roof/prop/other deck must
	not silently land on whatever unrelated floor happens to share its XZ.
	One metre covers authored mesh-vs-cell sampling variation, never whole decks. */
	constexpr float MAX_PICKED_GROUND_HEIGHT_ERROR = 1.f;
	if (!std::isfinite(ground.y) ||
		std::abs(request.fPositionY - ground.y) > MAX_PICKED_GROUND_HEIGHT_ERROR)
		return reject(DEBUG_TELEPORT_RESULT::REJECTED_HEIGHT);
	Refresh_PlayerBlockingBodies();
	if (!m_ServerCollisionSystem.Is_PlayerPositionClear(
		ground.x, ground.y, ground.z, player.iNetEntityId))
		return reject(DEBUG_TELEPORT_RESULT::REJECTED_COLLISION);
	/* Validation is complete before any action, movement, projectile or
	trigger state is cleared. Only this session's player is mutated. */
	Reset_PlayerForDebugTeleport(player);
	player.fPositionX = ground.x;
	player.fPositionY = ground.y;
	player.fPositionZ = ground.z;
	result.eResult = DEBUG_TELEPORT_RESULT::ACCEPTED;
	Update_MarioControlState(player);
	result.fPositionX = ground.x;
	result.fPositionY = ground.y;
	result.fPositionZ = ground.z;
	player.LastDebugTeleportResult = result;
	return result;
#endif
}

bool LostArk::Server::CGameRoom::Configure_MarioRail(
	SERVER_PLAYER& player, const std::string& arrivalPlacementId)
{
	using namespace LostArk::Shared;
	if (0u == player.iMarioStage || player.iMarioStage > 4u || player.TriggerMove.isActive ||
		!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionZ))
		return false;
	const auto lane = std::find_if(MARIO_LANES.begin(), MARIO_LANES.end(),
		[&player, &arrivalPlacementId](const MARIO_LANE_BINDING& candidate)
		{ return candidate.stage == player.iMarioStage && arrivalPlacementId == candidate.arrival; });
	const auto* arrival = lane == MARIO_LANES.end() ? nullptr : Find_Placement(lane->arrival);
	const auto* exit = lane == MARIO_LANES.end() ? nullptr : Find_Placement(lane->exit);
	// The admitted stage uses arrival as rail metadata, not as a new trigger entry.
	// Disabled go boxes remain excluded by ServerTriggerSystem (including F1 entry).
	if (nullptr == arrival || nullptr == exit || !exit->isEnabled ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != arrival->eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != exit->eKind || arrival->TriggerActions.size() != 1u ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != arrival->TriggerActions.front().eKind)
	{
		player.bMarioRailReady = false;
		std::cout << "[MarioRail] invalid authored binding: " << arrivalPlacementId << '\n';
		return false;
	}
	const double dx = static_cast<double>(exit->fPositionX) - player.fPositionX;
	const double dz = static_cast<double>(exit->fPositionZ) - player.fPositionZ;
	const double length = std::hypot(dx, dz);
	if (!std::isfinite(length) || length < 0.1)
	{
		player.bMarioRailReady = false;
		std::cout << "[MarioRail] unusable arrival-to-exit axis: " << arrivalPlacementId << '\n';
		return false;
	}
	player.fMarioRailOriginX = player.fPositionX;
	player.fMarioRailOriginZ = player.fPositionZ;
	player.fMarioRailRightX = static_cast<float>(dx / length) * lane->rightSign;
	player.fMarioRailRightZ = static_cast<float>(dz / length) * lane->rightSign;
	player.strMarioRailArrivalId = arrivalPlacementId;
	player.bMarioRailReady = true;
	player.iMarioMoveExpiryTick = 0u;
	player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
	return true;
}

std::uint8_t LostArk::Server::CGameRoom::Begin_MarioStageObjects(const std::uint8_t stage)
{
	if (stage < 1u || stage > 4u) return 0u;
	for (const auto& [id, participant] : m_Players)
	{
		(void)id;
		if (participant.iMarioStage == stage && participant.iCurrentHp && participant.iMarioLayoutVariant)
			return participant.iMarioLayoutVariant;
	}
	const std::string groupId = "spawn.mario" + std::to_string(stage) + ".source";
	const auto& groups = m_SpawnGroupBootstrap.Get_Groups();
	const auto definition = std::find_if(groups.begin(), groups.end(), [&groupId](const auto& group) {
		return group.strSpawnGroupId == groupId;
	});
	if (definition == groups.end())
	{
		m_strStatus = "Mario source spawn group missing: " + groupId;
		return 0u;
	}
	// Validate every anchor before changing the old group or admitting this run.
	for (const auto& wave : definition->Waves)
		for (const auto& entry : wave.Entries)
		{
			const auto* anchor = m_SpawnGroupBootstrap.Find_Anchor(entry.strAnchorId);
			SERVER_NAV_POINT ground{};
			if (!anchor || !m_SpawnGroupBootstrap.Find_Profile(entry.strArchetypeId) ||
				!m_ServerNavigation.Sample_Position(anchor->fPositionX, anchor->fPositionZ, ground) ||
				std::abs(ground.y - anchor->fPositionY) > .25f)
			{
				m_strStatus = "Mario source anchor/profile rejected: " + entry.strAnchorId;
				return 0u;
			}
		}
	Reset_MarioStageObjects(stage);
	if (!m_SpawnGroupRuntime.Activate(groupId))
	{
		m_strStatus = "Mario source spawn activation failed: " + groupId;
		return 0u;
	}
	const auto roll = std::uniform_int_distribution<unsigned>{0u, 99u}(m_MarioLayoutRandom);
	return roll < 34u ? 1u : (roll < 67u ? 2u : 3u);
}

void LostArk::Server::CGameRoom::Reset_MarioStageObjects(const std::uint8_t stage)
{
	if (stage < 5u) m_MarioPoppedBalls[stage] = 0u;
	const std::string groupId = "spawn.mario" + std::to_string(stage) + ".source";
	for (auto entity = m_WorldEntities.begin(); entity != m_WorldEntities.end();)
	{
		if (entity->strSpawnGroupId != groupId) { ++entity; continue; }
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		entity = m_WorldEntities.erase(entity);
	}
	if (!Broadcast_CombatObjectLifecycle()) Mark_RuntimeFailure("mario.reset.combat-object-lifecycle");
	(void)m_SpawnGroupRuntime.Reset_Group(groupId);
}

void LostArk::Server::CGameRoom::Cleanup_EmptyMarioStages()
{
	if (m_eWorldId != LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA) return;
	for (std::uint8_t stage = 1u; stage <= 4u; ++stage)
	{
		const bool occupied = std::any_of(m_Players.begin(), m_Players.end(), [stage](const auto& pair) {
			return pair.second.iMarioStage == stage && pair.second.iCurrentHp;
		});
		if (!occupied && m_SpawnGroupRuntime.Is_ActiveOrCompleted("spawn.mario" + std::to_string(stage) + ".source"))
			Reset_MarioStageObjects(stage);
	}
}

void LostArk::Server::CGameRoom::Update_MarioControlState(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId || player.iMarioStage > 4u ||
		0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.Clear_MarioControl();
		return;
	}
	if (0u != player.iMarioStage)
	{
		/* A finish is the terminal exit in this stage's authored lane graph.
		Its destination can be edited to the arena rather than the old go box. */
		const bool terminalExit = player.TriggerMove.isActive &&
			std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& lane) {
				return lane.stage == player.iMarioStage && player.TriggerMove.strSourcePlacementId == lane.exit;
			}) && !std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& lane) {
				return lane.stage == player.iMarioStage && player.TriggerMove.strSourcePlacementId == lane.arrival;
			});
		const auto* entrance = Find_Placement("Mario" + std::to_string(player.iMarioStage) + "_go");
		if (terminalExit || (player.TriggerMove.isActive && nullptr != entrance &&
			std::abs(player.TriggerMove.fTargetX - entrance->fPositionX) < 0.05f &&
			std::abs(player.TriggerMove.fTargetY - entrance->fPositionY) < 0.05f &&
			std::abs(player.TriggerMove.fTargetZ - entrance->fPositionZ) < 0.05f))
		{
			player.Clear_MarioControl();
			return;
		}
		player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO;
		if (!player.bMarioRailReady && !player.TriggerMove.isActive &&
			PLAYER_ACTION_STATE::NONE == player.eAction)
			(void)Configure_MarioRail(player, player.strMarioRailArrivalId);
		return;
	}
	if (player.bPatternBound || INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
		(PLAYER_ACTION_STATE::NONE != player.eAction && PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction))
		return;
	for (std::uint8_t stage = 1u; stage <= 4u; ++stage)
	{
		const auto* intro = Find_Placement("Mario" + std::to_string(stage) + "_Intro");
		if (nullptr == intro || !intro->isEnabled || WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != intro->eKind ||
			!CServerTriggerSystem::Contains_Placement(*intro, player))
			continue;
		const std::uint8_t layout = Begin_MarioStageObjects(stage);
		if (!layout)
		{
			if ((m_iServerTick % 30u) == 0u) std::cerr << "[MarioLayout] " << m_strStatus << '\n';
			return;
		}
		player.iMarioLayoutVariant = layout;
		player.ePreMarioForm = player.eMadnessForm;
		player.iMarioStage = stage;
		player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		/* MARIO is what deals the two interaction slots and what the Client
		checks before it submits Q/W at all. The only authored source is
		Mario1_go's movePlayer, and every MarioN_go is a disabled config row
		whose action never fires, so the stage sets it here instead. */
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO;
		player.strMarioRailArrivalId = "Mario" + std::to_string(stage) + "_go";
		player.bMarioRailReady = false;
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.PendingCommand.Clear();
		if (!player.TriggerMove.isActive)
			(void)Configure_MarioRail(player, player.strMarioRailArrivalId);
		std::cout << "[MarioControl] player=" << player.iPlayerId << " stage=" << static_cast<unsigned>(stage)
			<< " layout=" << static_cast<unsigned>(layout) << '\n';
		return;
	}
}

void LostArk::Server::CGameRoom::Handle_MarioMove(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MARIO_MOVE& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	if (binding == m_PlayerIdBySessionId.end())
		return;
	const auto found = m_Players.find(binding->second);
	if (found == m_Players.end() || found->second.iSessionId != sessionId)
		return;
	SERVER_PLAYER& player = found->second;
	const bool stop = MARIO_DIRECTION::STOP == request.eDirection;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId || request.eWorldId != m_eWorldId ||
		!Is_NewerSequence(request.iClientSequence, player.iLastMarioMoveSequence) ||
		(!stop && MARIO_DIRECTION::LEFT != request.eDirection && MARIO_DIRECTION::RIGHT != request.eDirection))
		return;
	player.iLastMarioMoveSequence = request.iClientSequence;
	if (0u == player.iMarioStage || player.iMarioStage > 4u || 0u == player.iCurrentHp)
		return;
	/* A key-up packet during a jump only releases the directional lease. It
		never cancels the independently owned scripted motion or its action. */
	if (stop)
	{
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		return;
	}
	if (!player.bMarioRailReady || PLAYER_ACTION_STATE::NONE != player.eAction || player.bPatternBound ||
		player.TriggerMove.isActive || player.fKnockbackRemainingSeconds > 0.f ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return;
	const float sign = MARIO_DIRECTION::RIGHT == request.eDirection ? 1.f : -1.f;
	player.fMarioDirectionX = player.fMarioRailRightX * sign;
	player.fMarioDirectionZ = player.fMarioRailRightZ * sign;
	player.iMarioMoveExpiryTick = m_iServerTick + 9u;
	if (0u == player.iMarioMoveExpiryTick)
		player.iMarioMoveExpiryTick = 1u;
}

void LostArk::Server::CGameRoom::Update_MarioMoveGoal(
	SERVER_PLAYER& player, const std::uint32_t updateTick)
{
	if (0u == player.iMarioStage)
		return;
	if (!player.bMarioRailReady || 0u == player.iMarioMoveExpiryTick || Has_ReachedServerTick(updateTick, player.iMarioMoveExpiryTick))
	{
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		return;
	}
	/* Feed the ordinary one-step navigation/collision mover, not Find_Path:
		left/right must stop at a gap rather than walking an A* detour. */
	if (!player.MovePath.empty())
		player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fMoveGoalX = player.fPositionX + player.fMarioDirectionX * 0.75f;
	player.fMoveGoalZ = player.fPositionZ + player.fMarioDirectionZ * 0.75f;
	Project_MarioRailPoint(player, player.fMoveGoalX, player.fMoveGoalZ);
	player.hasMoveGoal = true;
}

void LostArk::Server::CGameRoom::Handle_DebugMarioJump(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request)
{
	using namespace LostArk::Shared;
	const auto session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_MARIO_JUMP_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugMarioJump(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_MARIO_JUMP_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
	std::cout << "[DebugMarioJump] session=" << sessionId
		<< " sequence=" << result.iClientSequence
		<< " result=" << static_cast<unsigned>(result.eResult) << '\n';
}

LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT
LostArk::Server::CGameRoom::Apply_DebugMarioJump(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_MARIO_JUMP_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
#ifndef _DEBUG
	(void)player;
	result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId || request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugMarioJumpResult;
	if (0u != request.iClientSequence && request.iClientSequence == previous.iClientSequence)
		return previous;
	if (!Is_NewerSequence(request.iClientSequence, previous.iClientSequence))
	{
		result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_MARIO_JUMP_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugMarioJumpResult = result;
		return result;
	};
	if (0u == player.iMarioStage || player.iMarioStage > 4u || !player.bMarioRailReady)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_OUTSIDE_MARIO);
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::NONE != player.eAction ||
		player.TriggerMove.isActive || player.bPatternBound || player.bArenaEjectionActive ||
		player.fKnockbackRemainingSeconds > 0.f || player.iKnockdownEndTick > m_iServerTick ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);
	if (MARIO_DIRECTION::LEFT != request.eDirection && MARIO_DIRECTION::RIGHT != request.eDirection)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_INVALID_TARGET);

	const auto sameStageGrid = [this, &player](const float x, const float z)
	{
		return 1u == player.iMarioStage ?
			m_ServerNavigation.Is_InSameNavigationGrid(player.fPositionX, player.fPositionZ, x, z) :
			m_ServerNavigation.Is_InSameDetailRegion(player.fPositionX, player.fPositionZ, x, z);
	};
	const auto* entrance = Find_Placement("Mario" + std::to_string(player.iMarioStage) + "_go");
	if (nullptr == entrance || entrance->TriggerActions.size() != 1u ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != entrance->TriggerActions.front().eKind ||
		!sameStageGrid(entrance->TriggerActions.front().fTargetX, entrance->TriggerActions.front().fTargetZ))
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_OUTSIDE_MARIO);
	SERVER_NAV_POINT startGround{};
	if (!m_ServerNavigation.Is_PointWalkableExact(player.fPositionX, player.fPositionZ) ||
		!m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, startGround) ||
		!std::isfinite(player.fPositionY) || !std::isfinite(startGround.y) ||
		std::abs(player.fPositionY - startGround.y) > 0.25f)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);

	const float sign = MARIO_DIRECTION::RIGHT == request.eDirection ? 1.f : -1.f;
	const float directionX = player.fMarioRailRightX * sign;
	const float directionZ = player.fMarioRailRightZ * sign;
	Refresh_PlayerBlockingBodies();
	for (int quarterMetres = 16; quarterMetres >= 3; --quarterMetres)
	{
		const float leapDistance = static_cast<float>(quarterMetres) * 0.25f;
		float x = player.fPositionX + directionX * leapDistance;
		float z = player.fPositionZ + directionZ * leapDistance;
		Project_MarioRailPoint(player, x, z);
		SERVER_NAV_POINT landing{};
		if (!sameStageGrid(x, z) ||
			!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
			!m_ServerNavigation.Sample_Position(x, z, landing) || !std::isfinite(landing.y) ||
			std::abs(landing.y - startGround.y) > 1.f ||
			!m_ServerCollisionSystem.Is_PlayerPositionClear(x, landing.y, z, player.iNetEntityId))
			continue;
		WORLD_TRIGGER_ACTION jump{};
		jump.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		jump.fTargetX = landing.x;
		jump.fTargetY = landing.y;
		jump.fTargetZ = landing.z;
		jump.fDurationSeconds = 0.6f;
		jump.fArcHeight = 1.5f;
		if (!CServerTriggerSystem::Begin_MovePlayer(player, jump, 0u == m_iServerTick ? 1u : m_iServerTick))
			return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		/* Begin_MovePlayer only changes the scripted displacement/action. HP,
		world-sequence state and trigger membership intentionally stay intact. */
		result.eResult = DEBUG_MARIO_JUMP_RESULT::ACCEPTED;
		player.LastDebugMarioJumpResult = result;
		return result;
	}
	return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_NO_LANDING);
#endif
}
