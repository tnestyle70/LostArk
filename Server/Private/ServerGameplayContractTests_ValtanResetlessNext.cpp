#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "EncounterPropRuntime.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "Network/PacketReader.h"
#include "ServerNavigation.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanResetlessNext(TESTS& tests, const char* VALTAN_WALL_COLLISION_STATE)
{


#ifdef _DEBUG
	constexpr SESSION_ID NEXT_OWNER_SESSION = 73401u;
	constexpr PLAYER_ID NEXT_OWNER_PLAYER = 73402u;
	struct RESETLESS_NEXT_ROOM_FIXTURE final
	{
		std::unique_ptr<CGameRoom> Room;
		std::shared_ptr<CClientSession> Session;
	};
	const auto makeResetlessNextRoom = [&](const char* patternId, const bool startIsolated = true)
	{
		RESETLESS_NEXT_ROOM_FIXTURE fixture{};
		fixture.Room = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *fixture.Room;
		fixture.Session = std::make_shared<CClientSession>(NEXT_OWNER_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		fixture.Session->m_isSendRunning.store(true);
		room.m_Sessions.emplace(NEXT_OWNER_SESSION, fixture.Session);
		const bool activated = room.Is_Ready() && room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER player{};
		player.iSessionId = NEXT_OWNER_SESSION;
		player.iPlayerId = NEXT_OWNER_PLAYER;
		player.iNetEntityId = 73403u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 1000000000u;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(player.iPlayerId, player);
		room.m_PlayerIdBySessionId.emplace(NEXT_OWNER_SESSION, player.iPlayerId);
		room.m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
		if (!startIsolated)
		{
			tests.Require(activated && nullptr != boss && boss->strPatternId.empty(),
				"Prepare a real untouched Product room for live Next and Flow restart");
			return fixture;
		}
		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		play.strBossPlacementId = "boss.valtan.center";
		play.strPatternId = patternId;
		play.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t bar = 0u;
		const auto verdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, play, bar);
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(activated && VALTAN_AUDITION_RESULT::QUEUED == verdict &&
			room.Is_Ready() && nullptr != boss && boss->strPatternId == patternId &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == room.m_ValtanPatternIdAudition.ePhase,
			"Start a real isolated Valtan occurrence for resetless Next contracts");
		return fixture;
	};
	const auto makeNextControl = [](CGameRoom& room, const std::uint32_t sequence,
		const std::string& patternId, const std::uint32_t expectedNext = 0u)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = sequence;
		request.eOperation = VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID;
		request.strBossPlacementId = room.m_ValtanPatternIdAudition.strBossPlacementId;
		request.strPatternId = patternId;
		request.iPredecessorRoomAuditionEpoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		request.iPredecessorPatternSequence = room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		request.iExpectedNextRequestSequence = expectedNext;
		request.ExpectedDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		return request;
	};
	const auto readNextLifecycle = [](const std::shared_ptr<CClientSession>& session)
	{
		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> result;
		for (const auto& frame : session->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(frame.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE != header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				frame.Bytes.data() + PACKET_HEADER_BYTES, frame.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) && 0u == reader.Get_RemainingSize())
				result.push_back(std::move(decoded));
		}
		return result;
	};
	const auto runResetlessNextUntilHold = [](CGameRoom& room)
	{
		for (std::uint32_t tick = 0u; tick < 900u && room.Is_Ready() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == room.m_ValtanPatternIdAudition.ePhase;
			++tick)
		{
			room.Tick(1.f / 30.f);
		}
		const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		return room.Is_Ready() && nullptr != boss &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == room.m_ValtanPatternIdAudition.ePhase &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult;
	};
	const auto makeLiveNext = [](CGameRoom& room, const std::uint32_t sequence,
		const std::string& patternId)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = sequence;
		request.eOperation = VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID;
		request.strBossPlacementId = "boss.valtan.center";
		request.strPatternId = patternId;
		const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		request.iPredecessorPatternSequence = nullptr == boss ? 0u : boss->iPatternSequence;
		request.ExpectedDefinitionRevision =
			room.Is_ValtanPatternFlowRunning() ?
				room.m_ValtanPatternFlowAudition.PinnedDefinitionRevision :
				(nullptr != boss && !boss->strPatternId.empty() ?
					boss->PinnedDefinitionRevision :
					room.m_GameplayCatalog.Get_ActiveRevision());
		return request;
	};
	const auto captureLiveNextGameplay = [](RESETLESS_NEXT_ROOM_FIXTURE& fixture)
	{
		CGameRoom& room = *fixture.Room;
		room.Broadcast_WorldSnapshot();
		(void)room.Send_WorldDestructionFullSync(fixture.Session);
		(void)room.Send_EncounterPropSync(fixture.Session);
		std::array<std::vector<std::uint8_t>, 3u> bytes;
		for (const auto& frame : fixture.Session->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(frame.Bytes, header))
				continue;
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == header.ePacketType)
				bytes[0] = frame.Bytes;
			else if (PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC == header.ePacketType)
				bytes[1] = frame.Bytes;
			else if (PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC == header.ePacketType)
				bytes[2] = frame.Bytes;
		}
		return bytes;
	};
	const auto makeRestartFlow = []()
	{
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START request{};
		request.iRequestSequence = 1u;
		request.strBossPlacementId = "boss.valtan.center";
		request.strFlowId = "flow.valtan.live-restart-contract";
		request.strFlowRevision = std::string(64u, 'a');
		request.iInterStepPursuitMs = 1000u;
		request.Slots = {
			{ "flow.slot.000091", "VALTAN_WHIRLWIND" },
			{ "flow.slot.000003", "VALTAN_FOUR_SLASH" },
			{ "flow.slot.000030", "VALTAN_FIST_IN_OUT" }
		};
		request.strStartSlotId = request.Slots.front().strSlotId;
		return request;
	};
	{
		/* The Boss Tool's saved Flow is not an arbitrary Debug program: it is the
		   exact projection of the scriptedSequence in the Server-active gameplay
		   generation. Reject a reordered client copy before the arena reset, then
		   admit that generation's exact Pattern 01 sequence and reset once. */
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		const CGameplayCatalog* const activeCatalog =
			room.m_GameplayCatalog.Resolve(
				room.m_GameplayCatalog.Get_ActiveRevision());
		const BOSS_PATTERN_SEQUENCE_DEFINITION* const savedSequence =
			nullptr == activeCatalog ? nullptr :
			activeCatalog->Find_BossPatternSequence("ENCOUNTER_VALTAN");
		const bool canonicalReady = nullptr != savedSequence &&
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE ==
				savedSequence->eMode &&
			savedSequence->PatternIds.size() >= 2u;
		if (canonicalReady)
		{
			C2S_DEBUG_VALTAN_PATTERN_FLOW_START exact{};
			exact.iRequestSequence = 1u;
			exact.ExpectedDefinitionRevision =
				room.m_GameplayCatalog.Get_ActiveRevision();
			exact.strBossPlacementId = "boss.valtan.center";
			exact.strFlowId = "flow.valtan.boss-tool.default";
			exact.strFlowRevision = std::string(64u, 'd');
			exact.iInterStepPursuitMs =
				savedSequence->iInterStepPursuitMs;
			for (std::size_t index = 0u;
				index < savedSequence->PatternIds.size(); ++index)
			{
				exact.Slots.push_back({
					"flow.valtan.boss-tool.default.node." +
						std::to_string(index + 1u),
					savedSequence->PatternIds[index] });
			}
			exact.strStartSlotId = exact.Slots.front().strSlotId;

			SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			if (nullptr != boss)
				boss->iCurrentHp -= 123u;
			room.m_Players.at(NEXT_OWNER_PLAYER).
				CooldownEndTickBySkillId.emplace(34000u, 9000u);
			const auto beforeMismatch = captureLiveNextGameplay(fixture);
			const auto navigationRevision =
				room.m_ServerNavigation.Get_Revision();
			const auto collisionRevision =
				room.m_ServerCollisionSystem.Get_Revision();
			auto reordered = exact;
			std::swap(reordered.Slots[0], reordered.Slots[1]);
			std::uint32_t roomFlowEpoch = 0u;
			GameplayDataRevision pinnedRevision{};
			std::string reason;
			const auto rejected = room.Evaluate_ValtanPatternFlowStart(
				NEXT_OWNER_SESSION, reordered, roomFlowEpoch,
				pinnedRevision, reason);
			boss = room.Find_AuditionBoss();
			const bool rejectedBeforeReset = nullptr != boss &&
				VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW == rejected &&
				!reason.empty() && 0u == roomFlowEpoch &&
				!pinnedRevision.Is_Valid() &&
				beforeMismatch == captureLiveNextGameplay(fixture) &&
				navigationRevision == room.m_ServerNavigation.Get_Revision() &&
				collisionRevision == room.m_ServerCollisionSystem.Get_Revision() &&
				boss->iCurrentHp + 123u == boss->iMaximumHp &&
				room.m_Players.at(NEXT_OWNER_PLAYER).
					CooldownEndTickBySkillId.contains(34000u) &&
				!room.Is_ValtanPatternFlowRunning();

			exact.iRequestSequence = 2u;
			const auto admitted = room.Evaluate_ValtanPatternFlowStart(
				NEXT_OWNER_SESSION, exact, roomFlowEpoch,
				pinnedRevision, reason);
			boss = room.Find_AuditionBoss();
			const bool freshResetCommitted = nullptr != boss &&
				VALTAN_PATTERN_FLOW_RESULT::QUEUED == admitted &&
				reason.empty() && 0u != roomFlowEpoch &&
				pinnedRevision == exact.ExpectedDefinitionRevision &&
				boss->iCurrentHp == boss->iMaximumHp &&
				room.m_Players.at(NEXT_OWNER_PLAYER).
					CooldownEndTickBySkillId.empty() &&
				room.Is_ValtanPatternFlowRunning() &&
				0u == room.m_ValtanPatternFlowAudition.iStartSlotIndex &&
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds ==
					savedSequence->PatternIds &&
				room.m_ValtanPatternFlowAudition.Sequence.TransitionPursuitMs ==
					savedSequence->TransitionPursuitMs;
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			const bool startedAtPattern01 = nullptr != boss &&
				boss->strPatternId == savedSequence->PatternIds.front() &&
				boss->PinnedDefinitionRevision ==
					exact.ExpectedDefinitionRevision;
			tests.Require(
				rejectedBeforeReset && freshResetCommitted &&
				startedAtPattern01,
				"Boss Tool Restart rejects a reordered saved Flow without mutation, then fresh-resets and starts exact Server-active scriptedSequence Pattern 01");
		}
		else
		{
			tests.Require(false,
				"Boss Tool Restart canonical scriptedSequence fixture is available");
		}
		fixture.Session->Request_Close();
	}
	{
		/* Restart while the previous program is still QUEUED. Admission must
		   invalidate that exact pending epoch, reset once, and stage the new saved
		   first slot. Retrying the accepted identity only replays its receipt edge. */
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		auto original = makeRestartFlow();
		original.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		const auto originalQueued = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION, original, epoch, pin, reason);
		const std::uint32_t originalEpoch = epoch;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		if (nullptr != boss)
			boss->iCurrentHp -= 123u;
		room.m_Players.at(NEXT_OWNER_PLAYER).
			CooldownEndTickBySkillId.emplace(34000u, 9000u);

		auto replacement = original;
		replacement.iRequestSequence = 2u;
		replacement.strFlowRevision = std::string(64u, 'b');
		std::rotate(
			replacement.Slots.begin(), replacement.Slots.begin() + 2u,
			replacement.Slots.end());
		replacement.strStartSlotId = replacement.Slots.front().strSlotId;
		const auto replacementQueued = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION, replacement, epoch, pin, reason);
		const std::uint32_t replacementEpoch = epoch;
		boss = room.Find_AuditionBoss();
		const bool replacedQueuedFresh = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == originalQueued &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == replacementQueued &&
			originalEpoch != replacementEpoch &&
			pin == replacement.ExpectedDefinitionRevision &&
			boss->iCurrentHp == boss->iMaximumHp &&
			room.m_Players.at(NEXT_OWNER_PLAYER).
				CooldownEndTickBySkillId.empty() &&
			CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternFlowAudition.ePhase &&
			0u == room.m_ValtanPatternFlowAudition.iStartSlotIndex &&
			std::vector<std::string>{
				"VALTAN_FIST_IN_OUT", "VALTAN_WHIRLWIND",
				"VALTAN_FOUR_SLASH" } ==
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds;
		const std::size_t lifecycleCountBeforeRetry =
			room.m_PendingValtanPatternFlowLifecycle.size();
		if (nullptr != boss)
			boss->iCurrentHp -= 7u;
		const std::uint32_t hpBeforeRetry = nullptr == boss ?
			0u : boss->iCurrentHp;
		std::uint32_t retryEpoch = 0u;
		GameplayDataRevision retryPin{};
		std::string retryReason;
		const auto duplicate = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION, replacement, retryEpoch, retryPin, retryReason);
		const bool duplicateReplayedPending = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicate &&
			retryEpoch == replacementEpoch && retryPin == pin &&
			boss->iCurrentHp == hpBeforeRetry &&
			room.m_ValtanPatternFlowAudition.iRoomFlowEpoch == replacementEpoch &&
			room.m_PendingValtanPatternFlowLifecycle.size() ==
				lifecycleCountBeforeRetry + 1u &&
			room.m_PendingValtanPatternFlowLifecycle.back().Message.eState ==
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING &&
			room.m_PendingValtanPatternFlowLifecycle.back().Message.
				iRoomFlowEpoch == replacementEpoch;
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(
			replacedQueuedFresh && duplicateReplayedPending && nullptr != boss &&
			boss->strPatternId == replacement.Slots.front().strPatternId &&
			boss->PinnedDefinitionRevision == replacement.ExpectedDefinitionRevision,
			"Restart replaces a QUEUED saved Flow from Pattern 01, while an exact unconfirmed retry only replays its PENDING receipt without a second reset");
		fixture.Session->Request_Close();
	}
	{
		/* Lifecycle correlation is a wire contract on its own. Exercise it
		   without relying on one pattern's duration: the second PENDING crosses
		   UINT32_MAX, then Stop loses the authoritative boss pointer. */
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		auto& flow = room.m_ValtanPatternFlowAudition;
		flow.ePhase = CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING;
		flow.iOwnerSessionId = NEXT_OWNER_SESSION;
		flow.iOwnerPlayerId = NEXT_OWNER_PLAYER;
		flow.iBossEntityId = nullptr == boss ? INVALID_NET_ENTITY_ID : boss->iNetEntityId;
		flow.iRequestSequence = 91u;
		flow.iRoomFlowEpoch = 92u;
		flow.iFirstPatternSequence = (std::numeric_limits<std::uint32_t>::max)();
		flow.iStartSlotIndex = 1u;
		flow.strBossPlacementId = "boss.valtan.center";
		flow.strFlowId = "flow.valtan.lifecycle-correlation-contract";
		flow.strFlowRevision = std::string(64u, 'c');
		flow.strStartSlotId = "flow.slot.000091";
		flow.Slots = {
			{ "flow.slot.000007", "VALTAN_FIST_IN_OUT" },
			{ "flow.slot.000091", "VALTAN_WHIRLWIND" },
			{ "flow.slot.000003", "VALTAN_FOUR_SLASH" }
		};
		flow.Sequence.PatternIds = { "VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH" };
		flow.PinnedDefinitionRevision = room.m_GameplayCatalog.Get_ActiveRevision();
		if (nullptr != boss)
		{
			boss->iRotationStepIndex = 0u;
			boss->iPatternSequence = (std::numeric_limits<std::uint32_t>::max)();
			room.Queue_ValtanPatternFlowLifecycle(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, boss);
			room.Queue_ValtanPatternFlowLifecycle(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE, boss);
			boss->iRotationStepIndex = 1u;
			room.Queue_ValtanPatternFlowLifecycle(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, boss);
			room.Queue_ValtanPatternFlowLifecycle(
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD, nullptr);
		}
		const auto edges = room.m_PendingValtanPatternFlowLifecycle;
		flow.iOwnerSessionId = INVALID_SESSION_ID;
		room.Queue_ValtanPatternFlowLifecycle(
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED, nullptr);
		const bool missingOwnerDropped =
			edges.size() == room.m_PendingValtanPatternFlowLifecycle.size();
		const bool exactCorrelation = 4u == edges.size() &&
			edges[0].iSessionId == NEXT_OWNER_SESSION &&
			edges[0].Message.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING &&
			edges[0].Message.iPatternSequence == (std::numeric_limits<std::uint32_t>::max)() &&
			edges[0].Message.strCurrentSlotId == "flow.slot.000091" &&
			edges[0].Message.iCurrentSlotOrdinal == 2u &&
			edges[1].Message.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE &&
			edges[1].Message.iPatternSequence == edges[0].Message.iPatternSequence &&
			edges[1].Message.strCurrentSlotId == edges[0].Message.strCurrentSlotId &&
			edges[2].Message.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING &&
			edges[2].Message.iPatternSequence == 1u &&
			edges[2].Message.strCurrentSlotId == "flow.slot.000003" &&
			edges[2].Message.iCurrentSlotOrdinal == 3u &&
			edges[3].Message.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD &&
			edges[3].Message.iPatternSequence == edges[2].Message.iPatternSequence &&
			edges[3].Message.strCurrentSlotId == edges[2].Message.strCurrentSlotId &&
			edges[3].Message.iCurrentSlotOrdinal == edges[2].Message.iCurrentSlotOrdinal;
		tests.Require(nullptr != boss && exactCorrelation && missingOwnerDropped,
			"Keep the last emitted slot/sequence through wrap and PENDING-to-Stop with no boss, and never fabricate a missing owner recipient");
		room.m_PendingValtanPatternFlowLifecycle.clear();
		room.m_ValtanPatternFlowAudition = {};
		fixture.Session->Request_Close();
	}
	for (std::uint32_t mode = 0u; mode < 3u; ++mode)
	{
		auto fixture = makeResetlessNextRoom(mode == 0u ? "VALTAN_GHOST_FINALE" : "", mode == 0u);
		CGameRoom& room = *fixture.Room;
		auto flow = makeRestartFlow();
		flow.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		flow.Slots = { { "flow.slot.000091", "VALTAN_GHOST_FINALE" },
			{ "flow.slot.000003", "VALTAN_FIST_IN_OUT" } };
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		bool admitted = true;
		if (mode != 0u)
			admitted = VALTAN_PATTERN_FLOW_RESULT::QUEUED ==
				room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, flow, epoch, pin, reason);
		for (std::uint32_t tick = 0u; tick < 900u && room.Is_Ready(); ++tick)
		{
			const SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
			if (nullptr != boss && boss->iPatternSequence >= 3u) break;
			room.Tick(1.f / 30.f);
		}
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const std::uint32_t predecessor = nullptr == boss ? 0u : boss->iPatternSequence;
		const bool observedCycles = admitted && room.Is_Ready() && nullptr != boss &&
			predecessor >= 3u && boss->strPatternId == "VALTAN_GHOST_FINALE";
		bool queued = false;
		if (mode == 2u)
		{
			C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
			stop.iControlSequence = 1u;
			stop.iRoomFlowEpoch = epoch;
			stop.strFlowId = flow.strFlowId;
			queued = VALTAN_PATTERN_FLOW_RESULT::QUEUED ==
				room.Evaluate_ValtanPatternFlowStopAfterCurrent(NEXT_OWNER_SESSION, stop, epoch, pin, reason);
		}
		else
		{
			const auto next = mode == 0u ? makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT") :
				makeLiveNext(room, 2u, "VALTAN_FIST_IN_OUT");
			std::uint32_t bar = 0u;
			queued = VALTAN_AUDITION_RESULT::QUEUED ==
				room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, next, bar);
		}
		bool terminal = false;
		for (std::uint32_t tick = 0u; tick < 500u && room.Is_Ready(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss) break;
			terminal = mode == 2u ?
				(!room.Is_ValtanPatternFlowRunning() && boss->strPatternId.empty() &&
					boss->bAutomaticPatternSequenceAuditionHold && boss->iPatternSequence == predecessor) :
				(boss->strPatternId == "VALTAN_FIST_IN_OUT" && boss->iPatternSequence == predecessor + 1u);
			if (terminal) break;
		}
		const bool noGhosts = std::none_of(room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
			[](const auto& entity) { return entity.iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID; });
		tests.Require(observedCycles && queued && terminal && noGhosts,
			mode == 2u ? "Flow Stop after two finale cycles finishes the current cycle and removes ghosts" :
				"Next after two finale cycles preserves current predecessor identity and starts exactly one successor");
		if (mode == 2u)
		{
			std::set<std::uint32_t> reportedCycles;
			bool exactStopSequence = false;
			for (const auto& frame : fixture.Session->m_OutboundFrames)
			{
				PACKET_HEADER header{};
				if (!Read_Packet_Header(frame.Bytes, header) ||
					PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE != header.ePacketType) continue;
				CPacketReader reader{ std::span<const std::uint8_t>(
					frame.Bytes.data() + PACKET_HEADER_BYTES, frame.Bytes.size() - PACKET_HEADER_BYTES) };
				S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE edge{};
				if (!Read_Message(reader, edge)) continue;
				if (edge.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE &&
					edge.strCurrentSlotId == "flow.slot.000091")
					reportedCycles.insert(edge.iPatternSequence);
				if (edge.eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD)
					exactStopSequence = edge.iPatternSequence == predecessor;
			}
			tests.Require(reportedCycles.size() >= 3u && exactStopSequence,
				"Same-slot finale Flow republishes every fresh occurrence and stops with its actual latest sequence");
		}
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const auto predecessorSequence = boss->iPatternSequence;
		const std::string predecessorId = boss->strPatternId;
		boss->iCurrentHp -= 123u;
		room.m_Players.at(NEXT_OWNER_PLAYER).iCurrentResource = 31u;
		room.m_Players.at(NEXT_OWNER_PLAYER).CooldownEndTickBySkillId.emplace(34000u, 9000u);
		const auto before = captureLiveNextGameplay(fixture);
		const auto navigationRevision = room.m_ServerNavigation.Get_Revision();
		const auto collisionRevision = room.m_ServerCollisionSystem.Get_Revision();
		auto request = makeLiveNext(room, 1u, "VALTAN_FIST_IN_OUT");
		std::uint32_t bar = 0u;
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, request, bar);
		const auto after = captureLiveNextGameplay(fixture);
		const auto epoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		const auto duplicate = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, request, bar);
		tests.Require(!predecessorId.empty() && 0u != predecessorSequence &&
			VALTAN_AUDITION_RESULT::QUEUED == queued && duplicate == queued &&
			before == after && !before[0].empty() &&
			navigationRevision == room.m_ServerNavigation.Get_Revision() &&
			collisionRevision == room.m_ServerCollisionSystem.Get_Revision() &&
			0u != epoch && epoch == room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
			room.m_ValtanPatternIdAudition.bAdoptedLivePredecessor &&
			room.m_ValtanNextPattern && predecessorSequence + 1u == room.m_ValtanNextPattern->iExpectedPatternSequence,
			"Live Next adopts the real Product occurrence without reset and exact retry keeps its epoch");
		std::uint32_t terminalTick = 0u;
		std::uint32_t nextStartTick = 0u;
		for (std::uint32_t tick = 0u; tick < 1800u && room.Is_Ready(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (predecessorSequence == boss->iPatternSequence &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult)
				terminalTick = room.m_iServerTick;
			if (boss->iPatternSequence > predecessorSequence)
			{
				nextStartTick = room.m_iServerTick;
				break;
			}
		}
		const auto edges = readNextLifecycle(fixture.Session);
		const bool noInventedCurrent = std::all_of(edges.begin(), edges.end(),
			[predecessorSequence](const auto& edge)
			{
				return 1u == edge.iRequestSequence && edge.iPatternSequence == predecessorSequence + 1u &&
					edge.strPatternId == "VALTAN_FIST_IN_OUT";
			});
		tests.Require(room.Is_Ready() && 0u != terminalTick && nextStartTick == terminalTick + 1u &&
			boss->iPatternSequence == predecessorSequence + 1u && boss->strPatternId == "VALTAN_FIST_IN_OUT" &&
			noInventedCurrent && runResetlessNextUntilHold(room),
			"Live Product Next starts once on the tick after actual completion and never fabricates a predecessor lifecycle");
		fixture.Session->Request_Close();
	}
	for (const bool waitForPlayer : { false, true })
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		if (waitForPlayer)
		{
			auto& player = room.m_Players.at(NEXT_OWNER_PLAYER);
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.isCombatReady = false;
		}
		const auto before = captureLiveNextGameplay(fixture);
		auto request = makeLiveNext(room, 1u, "VALTAN_FIST_IN_OUT");
		std::uint32_t bar = 0u;
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, request, bar);
		const bool noReset = before == captureLiveNextGameplay(fixture);
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const bool consumedAtCommit = 0u == boss->iPatternSequence && boss->strPatternId.empty() &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::NONE == boss->PatternTerminalReceipt.eResult &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == room.m_ValtanPatternIdAudition.ePhase &&
			!room.m_ValtanNextPattern;
		bool waitedWithoutRevive = true;
		if (waitForPlayer)
		{
			for (std::uint32_t tick = 0u; tick < 30u; ++tick)
				room.Tick(1.f / 30.f);
			waitedWithoutRevive = 0u == room.m_Players.at(NEXT_OWNER_PLAYER).iCurrentHp &&
				0u == boss->iPatternSequence && room.m_ValtanPatternIdAudition.bReportedWaitingForPlayer;
			C2S_REVIVE_PLAYER revive{};
			revive.iClientSequence = 1u;
			room.Handle_RevivePlayer(NEXT_OWNER_SESSION, revive);
		}
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && noReset && consumedAtCommit &&
			waitedWithoutRevive && 1u == boss->iPatternSequence && boss->strPatternId == request.strPatternId,
			"Idle live Next consumes at the commit seam, starts on the following tick and never fabricates completion or revive");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const auto before = captureLiveNextGameplay(fixture);
		std::uint32_t bar = 0u;
		auto stale = makeLiveNext(room, 1u, "VALTAN_FIST_IN_OUT");
		++stale.iPredecessorPatternSequence;
		const auto staleVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, stale, bar);
		auto unknown = makeLiveNext(room, 2u, "VALTAN_NO_SUCH_PATTERN");
		const auto unknownVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, unknown, bar);
		const auto* patterns = room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto unmanaged = std::find_if(patterns->begin(), patterns->end(),
			[](const BOSS_PATTERN_DEFINITION& pattern) { return !pattern.bAuthoringMasterManaged; });
		auto retired = makeLiveNext(room, 3u, unmanaged == patterns->end() ? "VALTAN_NO_SUCH_PATTERN" : unmanaged->strPatternId);
		const auto unmanagedVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, retired, bar);
		const bool rejectionPreserved = before == captureLiveNextGameplay(fixture) && !room.m_ValtanNextPattern;
		auto staleDefinition = makeLiveNext(room, 4u, "VALTAN_FIST_IN_OUT");
		staleDefinition.ExpectedDefinitionRevision.Bytes.front() ^= 0xffu;
		const auto staleDefinitionVerdict =
			room.Evaluate_ValtanAudition(
				NEXT_OWNER_SESSION, staleDefinition, bar);
		const auto staleDefinitionRetry =
			room.Evaluate_ValtanAudition(
				NEXT_OWNER_SESSION, staleDefinition, bar);
		boss->bMechanicLedgerRequiresReset = true;
		const auto failedBefore = captureLiveNextGameplay(fixture);
		auto failed = makeLiveNext(room, 5u, "VALTAN_FIST_IN_OUT");
		const auto failedVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, failed, bar);
		tests.Require(VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION == staleVerdict &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == unknownVerdict &&
			unmanaged != patterns->end() && VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == unmanagedVerdict &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION ==
				staleDefinitionVerdict &&
			staleDefinitionVerdict == staleDefinitionRetry &&
			rejectionPreserved && VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == failedVerdict &&
			failedBefore == captureLiveNextGameplay(fixture) && !room.m_ValtanNextPattern,
			"Stale live sequence, unknown/unmanaged pattern and a failed mechanic ledger preserve existing gameplay");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		auto flow = makeRestartFlow();
		flow.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		const auto started = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, flow, epoch, pin, reason);
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const auto predecessor = boss->iPatternSequence;
		room.m_PlayerIdBySessionId.emplace(NEXT_OWNER_SESSION + 1u, NEXT_OWNER_PLAYER);
		auto live = makeLiveNext(room, 1u, "VALTAN_FIST_IN_OUT");
		std::uint32_t bar = 0u;
		const auto foreignBefore = captureLiveNextGameplay(fixture);
		const auto foreign = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION + 1u, live, bar);
		const bool foreignPreserved = foreignBefore == captureLiveNextGameplay(fixture) && room.Is_ValtanPatternFlowRunning();
		room.m_PlayerIdBySessionId.erase(NEXT_OWNER_SESSION + 1u);
		auto& player = room.m_Players.at(NEXT_OWNER_PLAYER);
		player.iCurrentHp = 0u;
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.isCombatReady = false;
		room.Tick(1.f / 30.f);
		const bool predecessorWaitedIdle =
			boss->iPatternSequence == predecessor &&
			boss->strPatternId.empty() &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			!boss->bAutomaticPatternSequenceStepRunning &&
			!boss->bAutomaticPatternSequencePausedForRevive &&
			!boss->bMechanicLedgerRequiresReset &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED ==
				boss->PatternTerminalReceipt.eResult &&
			boss->PatternTerminalReceipt.iPatternSequence == predecessor;
		const auto before = captureLiveNextGameplay(fixture);
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, live, bar);
		const bool adopted = before == captureLiveNextGameplay(fixture) &&
			!room.Is_ValtanPatternFlowRunning();
		for (std::uint32_t tick = 0u; tick < 15u; ++tick)
			room.Tick(1.f / 30.f);
		const bool reservedNextWaitedForTarget =
			boss->iPatternSequence == predecessor &&
			boss->strPatternId.empty() && SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			!boss->bMechanicLedgerRequiresReset &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternIdAudition.ePhase &&
			room.m_ValtanPatternIdAudition.bReportedWaitingForPlayer &&
			room.m_ValtanPatternIdAudition.strPatternId == live.strPatternId &&
			boss->PendingPatternIds.end() != std::find(
				boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(),
				live.strPatternId);
		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(NEXT_OWNER_SESSION, revive);
		room.Tick(1.f / 30.f);
		const bool promotedAfterTargetAdmission =
			boss->iPatternSequence == predecessor + 1u &&
			boss->strPatternId == live.strPatternId &&
			!boss->bMechanicLedgerRequiresReset;
		tests.Require(VALTAN_PATTERN_FLOW_RESULT::QUEUED == started &&
			VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER == foreign && foreignPreserved &&
			VALTAN_AUDITION_RESULT::QUEUED == queued && adopted &&
			predecessorWaitedIdle && reservedNextWaitedForTarget &&
			promotedAfterTargetAdmission,
			"Live Next replaces its owner's Flow remainder after a targetless predecessor parks IDLE, then starts the reserved pattern only after target admission");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		auto original = makeRestartFlow();
		original.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		const auto started = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, original, epoch, pin, reason);
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		const auto originalEpoch = epoch;
		boss->iCurrentHp -= 123u;
		room.m_Players.at(NEXT_OWNER_PLAYER).CooldownEndTickBySkillId.emplace(34000u, 9000u);
		const auto before = captureLiveNextGameplay(fixture);
		auto invalid = original;
		invalid.iRequestSequence = 2u;
		invalid.Slots.back().strPatternId = "VALTAN_NO_SUCH_PATTERN";
		const auto invalidVerdict = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, invalid, epoch, pin, reason);
		const bool invalidPreserved = before == captureLiveNextGameplay(fixture) &&
			room.m_ValtanPatternFlowAudition.iRoomFlowEpoch == originalEpoch;
		room.m_PlayerIdBySessionId.emplace(NEXT_OWNER_SESSION + 1u, NEXT_OWNER_PLAYER);
		const auto foreign = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION + 1u, original, epoch, pin, reason);
		const bool foreignPreserved = before == captureLiveNextGameplay(fixture);
		room.m_PlayerIdBySessionId.erase(NEXT_OWNER_SESSION + 1u);
		auto* placement = const_cast<WORLD_BOOTSTRAP_PLACEMENT*>(room.Find_Placement("boss.valtan.center"));
		placement->isEnabled = true;
		auto preflightFailure = original;
		preflightFailure.iRequestSequence = 3u;
		const auto failedReset = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, preflightFailure, epoch, pin, reason);
		placement->isEnabled = false;
		const bool failedResetPreserved = before == captureLiveNextGameplay(fixture) &&
			room.m_ValtanPatternFlowAudition.iRoomFlowEpoch == originalEpoch;
		const GameplayDataRevision predecessorDefinitionRevision =
			room.m_ValtanPatternFlowAudition.PinnedDefinitionRevision;
		GameplayDataRevision replacementDefinitionRevision =
			predecessorDefinitionRevision;
		replacementDefinitionRevision.Bytes[29] ^= 0x57u;
		std::vector<wchar_t> flowRestartPathBuffer(32768u);
		std::filesystem::path flowRestartDataRoot;
		const DWORD flowRestartConfigured = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", flowRestartPathBuffer.data(),
			static_cast<DWORD>(flowRestartPathBuffer.size()));
		if (flowRestartConfigured > 0u &&
			flowRestartConfigured < flowRestartPathBuffer.size())
		{
			flowRestartDataRoot = flowRestartPathBuffer.data();
		}
		else if (GetModuleFileNameW(
			nullptr, flowRestartPathBuffer.data(),
			static_cast<DWORD>(flowRestartPathBuffer.size())) > 0u)
		{
			flowRestartDataRoot =
				std::filesystem::path(flowRestartPathBuffer.data()).
					parent_path().parent_path() / L"DataFiles";
		}
		std::error_code flowRestartPathError;
		const auto flowRestartBootstrapPath = std::filesystem::canonical(
			flowRestartDataRoot / L"Gameplay" / L"Gameplay.bootstrap",
			flowRestartPathError);
		auto flowReplacementGeneration =
			std::make_shared<CGameplayCatalog>();
		std::string flowReplacementActivationStatus;
		const bool flowReplacementActivated = !flowRestartPathError &&
			flowReplacementGeneration->Load_FromBootstrap(
				flowRestartBootstrapPath, predecessorDefinitionRevision,
				replacementDefinitionRevision) &&
			room.Stage_GameplayGeneration(
				7002u, predecessorDefinitionRevision,
				flowReplacementGeneration, flowReplacementActivationStatus) &&
			room.Commit_GameplayGeneration(7002u) &&
			predecessorDefinitionRevision ==
				room.m_ValtanPatternFlowAudition.PinnedDefinitionRevision &&
			predecessorDefinitionRevision == boss->PinnedDefinitionRevision &&
			replacementDefinitionRevision ==
				room.m_GameplayCatalog.Get_ActiveRevision();
		auto replacement = original;
		replacement.iRequestSequence = 4u;
		replacement.ExpectedDefinitionRevision =
			replacementDefinitionRevision;
		replacement.strFlowRevision = std::string(64u, 'b');
		replacement.Slots = { original.Slots[2], original.Slots[0], original.Slots[1] };
		replacement.strStartSlotId = replacement.Slots.front().strSlotId;
		const auto restarted = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, replacement, epoch, pin, reason);
		const auto newEpoch = epoch;
		const bool resetOnce = boss->iCurrentHp == boss->iMaximumHp &&
			room.m_Players.at(NEXT_OWNER_PLAYER).CooldownEndTickBySkillId.empty() &&
			pin == replacementDefinitionRevision &&
			room.m_ValtanPatternFlowAudition.PinnedDefinitionRevision ==
				replacementDefinitionRevision &&
			boss->PinnedDefinitionRevision == replacementDefinitionRevision;
		boss->iCurrentHp -= 17u;
		const auto after = captureLiveNextGameplay(fixture);
		const auto duplicate = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, replacement, epoch, pin, reason);
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT staleStop{};
		staleStop.iControlSequence = 1u;
		staleStop.strFlowId = original.strFlowId;
		staleStop.iRoomFlowEpoch = originalEpoch;
		const auto stopped = room.Evaluate_ValtanPatternFlowStopAfterCurrent(NEXT_OWNER_SESSION, staleStop, epoch, pin, reason);
		const bool duplicatePreserved = after == captureLiveNextGameplay(fixture) &&
			room.m_ValtanPatternFlowAudition.iRoomFlowEpoch == newEpoch;
		std::vector<std::string> observed;
		auto priorSequence = boss->iPatternSequence;
		for (std::uint32_t tick = 0u; tick < 1500u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
			if (!boss->strPatternId.empty() && boss->iPatternSequence != priorSequence)
			{
				observed.push_back(boss->strPatternId);
				priorSequence = boss->iPatternSequence;
			}
		}
		const std::vector<std::string> expected{ "VALTAN_FIST_IN_OUT", "VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH" };
		boss = room.Find_AuditionBoss();
		const std::uint32_t completedSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		const std::uint32_t completedHp = nullptr == boss ?
			0u : boss->iCurrentHp;
		const std::size_t lifecycleCountBeforeCompletedRetry =
			room.m_PendingValtanPatternFlowLifecycle.size();
		std::uint32_t completedRetryEpoch = 0u;
		GameplayDataRevision completedRetryPin{};
		std::string completedRetryReason;
		const auto completedRetry = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION, replacement, completedRetryEpoch,
			completedRetryPin, completedRetryReason);
		const bool completedReceiptReplayedWithoutRestart = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == completedRetry &&
			completedRetryEpoch == newEpoch &&
			completedRetryPin == replacementDefinitionRevision &&
			!room.Is_ValtanPatternFlowRunning() &&
			boss->iPatternSequence == completedSequence &&
			boss->iCurrentHp == completedHp &&
			room.m_PendingValtanPatternFlowLifecycle.size() ==
				lifecycleCountBeforeCompletedRetry + 1u &&
			room.m_PendingValtanPatternFlowLifecycle.back().Message.eState ==
				VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD &&
			room.m_PendingValtanPatternFlowLifecycle.back().Message.
				iRoomFlowEpoch == newEpoch;
		tests.Require(VALTAN_PATTERN_FLOW_RESULT::QUEUED == started &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW == invalidVerdict && invalidPreserved &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_CONFLICT == foreign && foreignPreserved &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW == failedReset && failedResetPreserved &&
			flowReplacementActivated &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == restarted && newEpoch != originalEpoch && resetOnce &&
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicate &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW == stopped && duplicatePreserved &&
			observed == expected && !room.Is_ValtanPatternFlowRunning() &&
			boss->bAutomaticPatternSequenceAuditionHold &&
			completedReceiptReplayedWithoutRestart,
			"Reload replaces an owned active Flow with the exact new saved array and newly Server-active definition revision from its first sparse slot; duplicate retries preserve the admitted run and replay its completed receipt without restarting");
		fixture.Session->Request_Close();
	}
	for (const bool finishPredecessor : { false, true })
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		const bool ready = !finishPredecessor || runResetlessNextUntilHold(room);
		const auto before = captureLiveNextGameplay(fixture);
		const auto oldEpoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		const auto oldPatternSequence = room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		auto flow = makeRestartFlow();
		flow.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		room.m_PlayerIdBySessionId.emplace(NEXT_OWNER_SESSION + 1u, NEXT_OWNER_PLAYER);
		const auto foreign = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION + 1u, flow, epoch, pin, reason);
		const bool preserved = before == captureLiveNextGameplay(fixture) &&
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch == oldEpoch;
		room.m_PlayerIdBySessionId.erase(NEXT_OWNER_SESSION + 1u);
		const auto owned = room.Evaluate_ValtanPatternFlowStart(
			NEXT_OWNER_SESSION, flow, epoch, pin, reason);
		const bool sentLifecycle = room.Flush_ValtanPatternIdAuditionLifecycle();
		const auto edges = readNextLifecycle(fixture.Session);
		const bool invalidatedPreviousIdentity = std::any_of(edges.begin(), edges.end(),
			[oldEpoch, oldPatternSequence](const auto& edge)
			{
				return edge.iRequestSequence == 1u && edge.iRoomAuditionEpoch == oldEpoch &&
					edge.iPatternSequence == oldPatternSequence && edge.strPatternId == "VALTAN_FIST_IN_OUT" &&
					VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == edge.eState;
			});
		tests.Require(ready && VALTAN_PATTERN_FLOW_RESULT::REJECTED_CONFLICT == foreign &&
			preserved && VALTAN_PATTERN_FLOW_RESULT::QUEUED == owned && sentLifecycle && invalidatedPreviousIdentity &&
			room.Is_ValtanPatternFlowRunning() && !room.m_ValtanNextPattern &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE == room.m_ValtanPatternIdAudition.ePhase,
			"Reload requires the isolated owner and invalidates the exact active or completed-hold Next identity");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("", false);
		CGameRoom& room = *fixture.Room;
		room.Tick(1.f / 30.f);
		auto live = makeLiveNext(room, 1u, "VALTAN_FIST_IN_OUT");
		std::uint32_t bar = 0u;
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, live, bar);
		const auto oldEpoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		auto flow = makeRestartFlow();
		flow.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t epoch = 0u;
		GameplayDataRevision pin{};
		std::string reason;
		const auto restarted = room.Evaluate_ValtanPatternFlowStart(NEXT_OWNER_SESSION, flow, epoch, pin, reason);
		const auto before = captureLiveNextGameplay(fixture);
		const auto duplicateLive = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, live, bar);
		auto staleNext = live;
		staleNext.iRequestSequence = 2u;
		staleNext.eOperation = VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID;
		staleNext.iPredecessorRoomAuditionEpoch = oldEpoch;
		const auto stale = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, staleNext, bar);
		tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == restarted && duplicateLive == queued &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION == stale &&
			before == captureLiveNextGameplay(fixture) && room.Is_ValtanPatternFlowRunning() &&
			!room.m_ValtanNextPattern && room.m_ValtanPatternFlowAudition.iRoomFlowEpoch == epoch,
			"Reload can replace an owned live Next chain and stale epoch-zero replay or old Next cannot retake the new Flow");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		const auto captureGameplay = [&room, &fixture]()
		{
			room.Broadcast_WorldSnapshot();
			(void)room.Send_WorldDestructionFullSync(fixture.Session);
			(void)room.Send_EncounterPropSync(fixture.Session);
			std::array<std::vector<std::uint8_t>, 3u> bytes;
			for (const auto& frame : fixture.Session->m_OutboundFrames)
			{
				PACKET_HEADER header{};
				if (!Read_Packet_Header(frame.Bytes, header))
					continue;
				if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == header.ePacketType)
					bytes[0] = frame.Bytes;
				else if (PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC == header.ePacketType)
					bytes[1] = frame.Bytes;
				else if (PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC == header.ePacketType)
					bytes[2] = frame.Bytes;
			}
			return bytes;
		};
		room.m_Players.at(NEXT_OWNER_PLAYER).CooldownEndTickBySkillId.emplace(34000u, 9000u);
		const auto before = captureGameplay();
		const auto navigationRevision = room.m_ServerNavigation.Get_Revision();
		const auto collisionRevision = room.m_ServerCollisionSystem.Get_Revision();
		std::uint32_t bar = 0u;
		auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		const auto queuedBar = bar;
		const auto after = captureGameplay();
		const auto eventCount = room.m_PendingValtanAuditionLifecycle.size();
		const auto duplicate = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && duplicate == queued && bar == queuedBar &&
			!before[0].empty() && !before[1].empty() && !before[2].empty() && before == after &&
			navigationRevision == room.m_ServerNavigation.Get_Revision() &&
			collisionRevision == room.m_ServerCollisionSystem.Get_Revision() &&
			eventCount == room.m_PendingValtanAuditionLifecycle.size(),
			"Next reservation and exact duplicate preserve live player/boss/objects/props/world/navigation state");
		auto changedDuplicate = queue;
		changedDuplicate.strPatternId = "VALTAN_WHIRLWIND";
		const auto changedDuplicateVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, changedDuplicate, bar);
		auto invalidReplacement = makeNextControl(room, 3u, "VALTAN_NO_SUCH_PATTERN", 2u);
		const auto rejected = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, invalidReplacement, bar);
		const auto rejectedDuplicate = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, invalidReplacement, bar);
		tests.Require(VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST == changedDuplicateVerdict &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE == rejected && rejected == rejectedDuplicate &&
			room.m_ValtanNextPattern && 2u == room.m_ValtanNextPattern->iRequestSequence,
			"Changed duplicate and rejected replacement replay cannot discard the approved Next slot");
		auto replacement = makeNextControl(room, 4u, "VALTAN_WHIRLWIND", 2u);
		const auto replaced = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, replacement, bar);
		auto staleClear = makeNextControl(room, 5u, "VALTAN_FIST_IN_OUT", 2u);
		staleClear.eOperation = VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID;
		const auto staleClearVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, staleClear, bar);
		auto clear = makeNextControl(room, 6u, "VALTAN_WHIRLWIND", 4u);
		clear.eOperation = VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID;
		const bool replacedPreserved = room.m_ValtanNextPattern && 4u == room.m_ValtanNextPattern->iRequestSequence;
		const auto cleared = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, clear, bar);
		const auto duplicateClear = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, clear, bar);
		tests.Require(VALTAN_AUDITION_RESULT::QUEUED == replaced && replacedPreserved &&
			VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED == staleClearVerdict &&
			VALTAN_AUDITION_RESULT::CLEARED == cleared && duplicateClear == cleared && !room.m_ValtanNextPattern,
			"Next replace/clear compare the exact reservation token and replay one terminal verdict");
		queue = makeNextControl(room, 7u, "VALTAN_FIST_IN_OUT");
		(void)room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		room.m_PlayerIdBySessionId.emplace(NEXT_OWNER_SESSION + 10u, NEXT_OWNER_PLAYER);
		auto notOwner = makeNextControl(room, 1u, "VALTAN_WHIRLWIND", 7u);
		const auto ownerVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION + 10u, notOwner, bar);
		room.m_PlayerIdBySessionId.erase(NEXT_OWNER_SESSION + 10u);
		auto staleEpoch = makeNextControl(room, 8u, "VALTAN_WHIRLWIND", 7u);
		++staleEpoch.iPredecessorRoomAuditionEpoch;
		const auto epochVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, staleEpoch, bar);
		const auto* patterns = room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		bool legacyRejected = false;
		if (nullptr != patterns)
		{
			const auto legacy = std::find_if(patterns->begin(), patterns->end(),
				[](const auto& pattern) { return !pattern.bAuthoringMasterManaged; });
			if (patterns->end() != legacy)
			{
				auto legacyQueue = makeNextControl(room, 9u, legacy->strPatternId, 7u);
				legacyRejected = VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
					room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, legacyQueue, bar);
			}
		}
		auto staleDefinition = makeNextControl(
			room, 10u, "VALTAN_WHIRLWIND", 7u);
		staleDefinition.ExpectedDefinitionRevision.Bytes.front() ^= 0xffu;
		const auto staleDefinitionVerdict =
			room.Evaluate_ValtanAudition(
				NEXT_OWNER_SESSION, staleDefinition, bar);
		const auto staleDefinitionRetry =
			room.Evaluate_ValtanAudition(
				NEXT_OWNER_SESSION, staleDefinition, bar);
		tests.Require(VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER == ownerVerdict &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION == epochVerdict &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION ==
				staleDefinitionVerdict &&
			staleDefinitionVerdict == staleDefinitionRetry && legacyRejected &&
			room.m_ValtanNextPattern && 7u == room.m_ValtanNextPattern->iRequestSequence,
			"Next admission requires current owner/epoch and a split-owned pattern in the pinned catalog");
		(void)room.Flush_ValtanPatternIdAuditionLifecycle();
		const auto lifecycle = readNextLifecycle(fixture.Session);
		const auto abortedCount = [&lifecycle](const std::uint32_t sequence, const char* reason)
		{
			return std::count_if(lifecycle.begin(), lifecycle.end(), [sequence, reason](const auto& edge)
				{ return edge.iRequestSequence == sequence && edge.eState == VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED && edge.strReason == reason; });
		};
		tests.Require(1 == abortedCount(2u, "replaced") && 1 == abortedCount(4u, "cleared"),
			"Replacing and clearing Next emit exactly one aborted edge for the old reservation identity");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		if (nullptr != boss)
		{
			boss->iCurrentHp -= 123u;
			const auto hp = boss->iCurrentHp;
			const auto epoch = room.m_WorldDestructionRuntime.Get_EncounterEpoch();
			const auto chainEpoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
			room.m_Players.at(NEXT_OWNER_PLAYER).CooldownEndTickBySkillId.emplace(34000u, 9000u);
			std::uint32_t bar = 0u;
			auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
			const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
			const std::string wallGroupId =
				"destroyable.group.valtan.wall159.15719065619666776634";
			WORLD_DESTRUCTION_GROUP_STATE stagedWall{};
			const bool wallTriggered = room.Apply_WorldDestructionContacts(
				*boss, { VALTAN_WALL_COLLISION_STATE }, room.m_iServerTick) &&
				room.m_WorldDestructionRuntime.Find_GroupState(wallGroupId, stagedWall) &&
				WORLD_DESTRUCTION_STATE::BREAKING == stagedWall.eState &&
				stagedWall.iCommitTick > room.m_iServerTick &&
				room.m_ServerCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE);
			for (std::uint32_t tick = 0u; tick < 180u && room.Is_Ready() &&
				1u == room.m_ValtanPatternIdAudition.iRequestSequence; ++tick)
			{
				room.Tick(1.f / 30.f);
			}
			const auto terminalTick = room.m_iServerTick;
			const float terminalX = boss->fPositionX;
			const float terminalZ = boss->fPositionZ;
			const SERVER_PLAYER terminalPlayer = room.m_Players.at(NEXT_OWNER_PLAYER);
			WORLD_DESTRUCTION_GROUP_STATE terminalWall{};
			const bool pendingAfterTerminal = room.Is_Ready() && boss->strPatternId.empty() &&
				1u == boss->iPatternSequence && 2u == room.m_ValtanPatternIdAudition.iRequestSequence &&
				CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == room.m_ValtanPatternIdAudition.ePhase &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult &&
				room.m_WorldDestructionRuntime.Find_GroupState(wallGroupId, terminalWall) &&
				WORLD_DESTRUCTION_STATE::BREAKING == terminalWall.eState &&
				terminalWall.iStateVersion == stagedWall.iStateVersion &&
				terminalWall.iCommitTick == stagedWall.iCommitTick &&
				terminalWall.strPendingMutationId == stagedWall.strPendingMutationId &&
				room.m_ServerCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE);
			room.Tick(1.f / 30.f);
			const SERVER_PLAYER afterStart = room.m_Players.at(NEXT_OWNER_PLAYER);
			WORLD_DESTRUCTION_GROUP_STATE successorWall{};
			const bool successorPreservedDueWork =
				room.m_WorldDestructionRuntime.Find_GroupState(wallGroupId, successorWall) &&
				WORLD_DESTRUCTION_STATE::BREAKING == successorWall.eState &&
				successorWall.iStateVersion == stagedWall.iStateVersion &&
				successorWall.iStateStartTick == stagedWall.iStateStartTick &&
				successorWall.iCommitTick == stagedWall.iCommitTick &&
				successorWall.strPendingMutationId == stagedWall.strPendingMutationId &&
				room.m_ServerCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE);
			tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && wallTriggered && pendingAfterTerminal &&
				2u == boss->iPatternSequence && "VALTAN_FIST_IN_OUT" == boss->strPatternId &&
				boss->iActionStartTick > terminalTick && boss->fPositionX == terminalX && boss->fPositionZ == terminalZ &&
				boss->iCurrentHp == hp && epoch == room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
				chainEpoch == room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
				successorPreservedDueWork &&
				terminalPlayer.fPositionX == afterStart.fPositionX && terminalPlayer.fPositionZ == afterStart.fPositionZ &&
				terminalPlayer.iCurrentHp == afterStart.iCurrentHp &&
				afterStart.CooldownEndTickBySkillId.at(34000u) == 9000u,
				"A-to-A starts a new occurrence on the tick after real completion without resetting HP, position, cooldown or pending world mutation");
			for (std::uint32_t tick = 0u; tick < 32u && room.Is_Ready() &&
				room.m_iServerTick < stagedWall.iCommitTick; ++tick)
			{
				room.Tick(1.f / 30.f);
			}
			WORLD_DESTRUCTION_GROUP_STATE committedWall{};
			tests.Require(room.Is_Ready() && room.m_iServerTick == stagedWall.iCommitTick &&
				epoch == room.m_WorldDestructionRuntime.Get_EncounterEpoch() &&
				room.m_WorldDestructionRuntime.Find_GroupState(wallGroupId, committedWall) &&
				WORLD_DESTRUCTION_STATE::DESPAWNED == committedWall.eState &&
				committedWall.iStateVersion == stagedWall.iStateVersion + 1u &&
				!room.m_ServerCollisionSystem.Is_PlayerBlocking(VALTAN_WALL_COLLISION_STATE),
				"Commit A's pending wall mutation on its original 267ms due tick while B owns the live pattern clock");
			const bool held = runResetlessNextUntilHold(room);
			const auto sequenceBeforeLateQueue = boss->iPatternSequence;
			const float holdX = boss->fPositionX;
			auto lateQueue = makeNextControl(room, 3u, "VALTAN_FIST_IN_OUT");
			const auto lateVerdict = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, lateQueue, bar);
			room.Tick(1.f / 30.f);
			room.Tick(1.f / 30.f);
			tests.Require(held && VALTAN_AUDITION_RESULT::QUEUED == lateVerdict &&
				sequenceBeforeLateQueue + 1u == boss->iPatternSequence && holdX == boss->fPositionX &&
				3u == room.m_ValtanPatternIdAudition.iRequestSequence,
				"A completed isolated anchor accepts a late Next without a new reset or room epoch");
			const auto edges = readNextLifecycle(fixture.Session);
			const auto findEdge = [&edges](const std::uint32_t request, const auto state)
			{
				return std::find_if(edges.begin(), edges.end(), [request, state](const auto& edge)
					{ return edge.iRequestSequence == request && edge.eState == state; });
			};
			const auto completedA = findEdge(1u, VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
			const auto pendingB = findEdge(2u, VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
			const auto activeB = findEdge(2u, VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
			tests.Require(completedA != edges.end() && pendingB != edges.end() && activeB != edges.end() &&
				completedA < pendingB && pendingB < activeB && 2u == activeB->iPatternSequence,
				"Current completion precedes Next pending and active lifecycle identities");
		}
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		const bool held = runResetlessNextUntilHold(room);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER& player = room.m_Players.at(NEXT_OWNER_PLAYER);
		player.iCurrentHp = 0u;
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.isCombatReady = false;
		std::uint32_t bar = 0u;
		auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		for (std::uint32_t tick = 0u; tick < 500u && room.Is_Ready(); ++tick)
			room.Tick(1.f / 30.f);
		const auto waitingEdges = readNextLifecycle(fixture.Session);
		const auto waitingCount = std::count_if(waitingEdges.begin(), waitingEdges.end(), [](const auto& edge)
			{ return 2u == edge.iRequestSequence && VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER == edge.eState; });
		const bool waiting = held && VALTAN_AUDITION_RESULT::QUEUED == queued && room.Is_Ready() &&
			nullptr != boss && 1u == boss->iPatternSequence && boss->strPatternId.empty() &&
			room.m_ValtanNextPattern && 0u == player.iCurrentHp && 1 == waitingCount;
		C2S_REVIVE_PLAYER revive{};
		revive.iClientSequence = 1u;
		room.Handle_RevivePlayer(NEXT_OWNER_SESSION, revive);
		room.Tick(1.f / 30.f);
		const bool promoted = CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
			room.m_ValtanPatternIdAudition.ePhase && 2u == room.m_ValtanPatternIdAudition.iRequestSequence;
		player.iCurrentHp = 0u;
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.isCombatReady = false;
		room.Tick(1.f / 30.f);
		const bool retainedPending = nullptr != boss && 1u == boss->iPatternSequence &&
			boss->strPatternId.empty() && 1u == boss->PendingPatternIds.size() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == room.m_ValtanPatternIdAudition.ePhase;
		revive.iClientSequence = 2u;
		room.Handle_RevivePlayer(NEXT_OWNER_SESSION, revive);
		room.Tick(1.f / 30.f);
		tests.Require(waiting && promoted && retainedPending && room.Is_Ready() &&
			nullptr != boss && 2u == boss->iPatternSequence && "VALTAN_FIST_IN_OUT" == boss->strPatternId &&
			boss->PendingPatternIds.empty(),
			"Next waits beyond fifteen seconds without auto-revive and retains promoted B when its target disappears before BeginPattern");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FOUR_PILLARS_105");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		for (std::uint32_t tick = 0u; tick < 900u && room.Is_Ready() &&
			nullptr != boss && "RECOVERY" != boss->strPatternStageId; ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		const auto firstSequence = nullptr == boss ? 0u : boss->iPatternSequence;
		const auto propEpoch = room.m_EncounterPropRuntime.Get_EncounterEpoch();
		const auto firstSlots = room.m_EncounterPropRuntime.Get_SlotStates();
		const bool raised = nullptr != boss && "RECOVERY" == boss->strPatternStageId &&
			4u == firstSlots.size() && std::all_of(firstSlots.begin(), firstSlots.end(),
				[firstSequence](const auto& slot)
				{
					return ENCOUNTER_PROP_STATE::INTACT == slot.eState &&
						firstSequence == slot.iOccurrenceSequence;
				});
		const auto propsUnchanged = [&room, &firstSlots, propEpoch]()
		{
			const auto& slots = room.m_EncounterPropRuntime.Get_SlotStates();
			return propEpoch == room.m_EncounterPropRuntime.Get_EncounterEpoch() &&
				slots.size() == firstSlots.size() &&
				std::equal(slots.begin(), slots.end(), firstSlots.begin(),
					[](const auto& current, const auto& before)
					{
						return current.strSlotId == before.strSlotId &&
							current.eState == before.eState &&
							current.iOccurrenceSequence == before.iOccurrenceSequence &&
							current.iStateVersion == before.iStateVersion &&
							current.iStateStartTick == before.iStateStartTick;
					});
		};
		const auto framesBeforeDuplicate = fixture.Session->m_OutboundFrames.size();
		const bool duplicateAccepted = raised &&
			room.Apply_EncounterPropStageEntry(*boss, room.m_iServerTick);
		tests.Require(duplicateAccepted && room.Is_Ready() && propsUnchanged() &&
			framesBeforeDuplicate == fixture.Session->m_OutboundFrames.size(),
			"A duplicate pillar stage entry remains NO_CHANGE without a second prop sync");

		const bool held = runResetlessNextUntilHold(room);
		const CEncounterPropRuntime previousPropOccurrence = room.m_EncounterPropRuntime;
		std::uint32_t bar = 0u;
		/* FOUR_PILLARS_105 is a legacy isolated audition, not a split-owned Next
		   target. Start it through the supported Play operation, then inject the
		   prior valid prop runtime after that reset as the deliberate failure
		   input. The queued Next below still uses an admitted split-owned ID. */
		C2S_VALTAN_AUDITION_REQUEST repeatPillars{};
		repeatPillars.iRequestSequence = 2u;
		repeatPillars.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		repeatPillars.strBossPlacementId = "boss.valtan.center";
		repeatPillars.strPatternId = "VALTAN_FOUR_PILLARS_105";
		repeatPillars.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const auto queuedB = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, repeatPillars, bar);
		room.m_EncounterPropRuntime = previousPropOccurrence;
		room.Tick(1.f / 30.f);
		const bool startedB = nullptr != boss && firstSequence + 1u == boss->iPatternSequence &&
			"VALTAN_FOUR_PILLARS_105" == boss->strPatternId &&
			2u == room.m_ValtanPatternIdAudition.iRequestSequence;
		const auto follow = makeNextControl(room, 3u, "VALTAN_FIST_IN_OUT");
		const auto queuedC = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, follow, bar);
		/* The current audition owns a new occurrence but the injected prop state
		   still belongs to the previous one. Its real RECOVERY entry must fail
		   rather than silently promoting Next C. No stage or admission is forged. */
		for (std::uint32_t tick = 0u; tick < 900u && room.Is_Ready() &&
			nullptr != boss && boss->iPatternSequence <= firstSequence + 1u; ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		const auto stoppedTick = room.m_iServerTick;
		room.Tick(1.f / 30.f);
		const auto edges = readNextLifecycle(fixture.Session);
		const bool noFalseCompletionOrStart = std::none_of(edges.begin(), edges.end(),
			[](const auto& edge)
			{
				return (2u == edge.iRequestSequence &&
					VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == edge.eState) ||
					(3u == edge.iRequestSequence &&
					 (VALTAN_AUDITION_LIFECYCLE_STATE::PENDING == edge.eState ||
					  VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == edge.eState));
			});
		tests.Require(raised && held && startedB &&
			VALTAN_AUDITION_RESULT::QUEUED == queuedB &&
			VALTAN_AUDITION_RESULT::QUEUED == queuedC &&
			!room.Is_Ready() && stoppedTick == room.m_iServerTick &&
			nullptr != boss && firstSequence + 1u == boss->iPatternSequence &&
			"RECOVERY" == boss->strPatternStageId &&
			boss->PatternTerminalReceipt.iPatternSequence == boss->iPatternSequence &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED == boss->PatternTerminalReceipt.eResult &&
			room.m_strStatus.find("Encounter prop slot is not in the expected state:") != std::string::npos &&
			propsUnchanged() && noFalseCompletionOrStart,
			"An injected prior pillar occurrence fails the real stage entry and blocks completion and Next promotion");
		fixture.Session->Request_Close();
	}
	for (std::uint32_t failure = 0u; failure < 5u; ++failure)
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		std::uint32_t bar = 0u;
		auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		if (nullptr != boss)
		{
			if (0u == failure)
			{
				auto& player = room.m_Players.at(NEXT_OWNER_PLAYER);
				player.iCurrentHp = 0u;
				player.eAction = PLAYER_ACTION_STATE::DEAD;
				player.isCombatReady = false;
			}
			else if (1u == failure)
			{
				// Deliberate corrupted/discarded identity is a negative fixture;
				// successful fixtures above only use the actual Brain terminal path.
				boss->strPatternId.clear();
				boss->PatternTerminalReceipt = {};
				(void)room.Refresh_ValtanPatternIdAuditionState();
			}
			else if (2u == failure)
			{
				boss->iCurrentHp = 0u;
				boss->eAction = SERVER_ENTITY_ACTION::DEAD;
			}
			else if (3u == failure)
			{
				SERVER_PLAYER observer = room.m_Players.at(NEXT_OWNER_PLAYER);
				observer.iSessionId += 1u;
				observer.iPlayerId += 1u;
				observer.iNetEntityId += 1u;
				room.m_Players.emplace(observer.iPlayerId, observer);
				room.m_PlayerIdBySessionId.emplace(observer.iSessionId, observer.iPlayerId);
				room.m_PlayerIdByEntityId.emplace(observer.iNetEntityId, observer.iPlayerId);
				room.Leave(NEXT_OWNER_SESSION, PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
			}
			else
			{
				std::string status;
				tests.Require(room.Reset_ValtanAuditionState(*boss, room.m_iServerTick, status),
					"An explicitly requested authoritative reset can cancel a reserved Next");
			}
			room.Tick(1.f / 30.f);
			const auto edges = readNextLifecycle(fixture.Session);
			const bool noNextStart = std::none_of(edges.begin(), edges.end(), [](const auto& edge)
				{ return 2u == edge.iRequestSequence && VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == edge.eState; });
			const bool noFalseCompletion = std::none_of(edges.begin(), edges.end(), [](const auto& edge)
				{ return 1u == edge.iRequestSequence && VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == edge.eState; });
			tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && !room.m_ValtanNextPattern &&
				noNextStart && noFalseCompletion &&
				(3u != failure || !room.m_ValtanNextPatternReceiptBySessionId.contains(NEXT_OWNER_SESSION)),
				"Abort, missing completion receipt, boss death, owner leave and reset cannot promote a reserved Next");
		}
		fixture.Session->Request_Close();
	}
	for (const bool failWorldCommit : { false, true })
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		std::uint32_t bar = 0u;
		auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		if (nullptr != boss)
		{
			for (std::uint32_t tick = 0u; tick < 180u && room.Is_Ready() &&
				(static_cast<std::uint64_t>(room.m_iServerTick + 2u - boss->iPatternStageFirstEvaluationTick) * 1000u <
				 static_cast<std::uint64_t>(boss->iPatternStageDurationMs) * 30u); ++tick)
			{
				room.Tick(1.f / 30.f);
			}
			const auto tickBeforeTerminal = room.m_iServerTick;
			const auto lastSnapshot = [&fixture]()
			{
				std::vector<std::uint8_t> result;
				for (const auto& frame : fixture.Session->m_OutboundFrames)
				{
					PACKET_HEADER header{};
					if (Read_Packet_Header(frame.Bytes, header) && PACKET_TYPE::S2C_WORLD_SNAPSHOT == header.ePacketType)
						result = frame.Bytes;
				}
				return result;
			};
			const auto snapshotBeforeTerminal = lastSnapshot();
			bool injected = false;
			if (failWorldCommit)
			{
				WORLD_DESTRUCTION_DESCRIPTOR_GRAPH graph{};
				graph.Groups.push_back({ "next.failure.group", { "next.failure.member" }, WORLD_DESTRUCTION_STATE::INTACT });
				WORLD_DESTRUCTION_MUTATION_DESCRIPTOR mutation{};
				mutation.strMutationId = "next.failure.mutation";
				mutation.strGroupId = "next.failure.group";
				mutation.eFinalState = WORLD_DESTRUCTION_STATE::DESPAWNED;
				mutation.iBreakingDurationTicks = 1u;
				mutation.strNavigationStateId = "next.failure.missing-nav-condition";
				graph.Mutations.push_back(mutation);
				WORLD_DESTRUCTION_BINDING_DESCRIPTOR binding{};
				binding.strBindingId = "next.failure.binding";
				binding.strMutationId = mutation.strMutationId;
				binding.eTriggerKind = WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT;
				binding.strImpactReceiverId = "next.failure.contact";
				graph.Bindings.push_back(binding);
				CWorldDestructionRuntime failingRuntime;
				WORLD_DESTRUCTION_TRANSACTION transaction{};
				std::string status;
				injected = failingRuntime.Initialize(graph, status, tickBeforeTerminal) &&
					WORLD_DESTRUCTION_PREPARE_RESULT::READY == failingRuntime.Prepare_ContactTrigger(
						binding.strImpactReceiverId, boss->iNetEntityId, 1u, tickBeforeTerminal, transaction, status) &&
					failingRuntime.Commit(transaction, status);
				if (injected)
					room.m_WorldDestructionRuntime = std::move(failingRuntime);
			}
			else
			{
				// No pillar occurrence exists: the scheduled break must reject,
				// and that failure must not be swallowed after A's final Brain hit.
				injected = 0u == room.m_EncounterPropRuntime.Get_OccurrenceSequence();
				room.m_iPillarAuditionBreakTick = tickBeforeTerminal + 1u;
			}
			room.Tick(1.f / 30.f);
			const auto edges = readNextLifecycle(fixture.Session);
			tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && injected && !room.Is_Ready() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult &&
				tickBeforeTerminal == room.m_iServerTick && room.m_ValtanNextPattern &&
				1u == room.m_ValtanPatternIdAudition.iRequestSequence && snapshotBeforeTerminal == lastSnapshot() &&
				std::none_of(edges.begin(), edges.end(), [](const auto& edge)
					{ return VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED == edge.eState ||
						(2u == edge.iRequestSequence && VALTAN_AUDITION_LIFECYCLE_STATE::PENDING == edge.eState); }),
				"Final prop/world commit failure suppresses completion, Next promotion and the partial success snapshot");
		}
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		const GameplayDataRevision pinned = room.m_GameplayCatalog.Get_ActiveRevision();
		GameplayDataRevision nextRevision = pinned;
		nextRevision.Bytes[0] ^= 0x80u;
		std::vector<wchar_t> pathBuffer(32768u);
		std::filesystem::path dataRoot;
		const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (configured > 0u && configured < pathBuffer.size())
			dataRoot = pathBuffer.data();
		else if (GetModuleFileNameW(nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size())) > 0u)
			dataRoot = std::filesystem::path(pathBuffer.data()).parent_path().parent_path() / L"DataFiles";
		std::error_code pathError;
		const auto bootstrapPath = std::filesystem::canonical(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", pathError);
		auto generation = std::make_shared<CGameplayCatalog>();
		std::string status;
		std::uint32_t bar = 0u;
		auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		const bool activated = !pathError && generation->Load_FromBootstrap(bootstrapPath, pinned, nextRevision) &&
			room.Stage_GameplayGeneration(5001u, pinned, generation, status) && room.Commit_GameplayGeneration(5001u);
		std::vector<GameplayDataRevision> pins;
		const bool collected = room.Build_RequiredPinnedGameplayRevisions(pins);
		room.m_GameplayCatalog.Collect_Garbage(pins);
		const bool keptPin = collected && pins.end() != std::find(pins.begin(), pins.end(), pinned) &&
			nullptr != room.m_GameplayCatalog.Resolve(pinned);
		for (std::uint32_t tick = 0u; tick < 180u && room.Is_Ready() &&
			1u == room.m_ValtanPatternIdAudition.iRequestSequence; ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		room.Tick(1.f / 30.f);
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && activated && keptPin &&
			room.Is_Ready() && nullptr != boss && 2u == boss->iPatternSequence &&
			pinned == boss->PinnedDefinitionRevision && pinned == room.m_ValtanPatternIdAudition.PinnedDefinitionRevision &&
			nextRevision == room.m_GameplayCatalog.Get_ActiveRevision(),
			"A-to-B retains the immutable R1 pin across R2 activation and generation collection");
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_FIST_IN_OUT");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		if (nullptr != boss)
		{
			boss->iPatternSequence = (std::numeric_limits<std::uint32_t>::max)();
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence = boss->iPatternSequence;
			auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
			std::uint32_t bar = 0u;
			tests.Require(VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar) && !room.m_ValtanNextPattern &&
				(std::numeric_limits<std::uint32_t>::max)() == boss->iPatternSequence,
				"Next rejects occurrence sequence exhaustion rather than wrapping to a reused identity");
		}
		fixture.Session->Request_Close();
	}
	{
		auto fixture = makeResetlessNextRoom("VALTAN_TRASH");
		CGameRoom& room = *fixture.Room;
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		std::uint32_t bar = 0u;
		const auto queue = makeNextControl(room, 2u, "VALTAN_FIST_IN_OUT");
		const auto queued = room.Evaluate_ValtanAudition(NEXT_OWNER_SESSION, queue, bar);
		bool captured = false, executed = false;
		std::uint32_t counterTick = 0u, impactTick = 0u, completedTick = 0u;
		if (nullptr != boss)
		{
			const auto bossHp = boss->iCurrentHp;
			const auto roomEpoch = room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
			const auto worldEpoch = room.m_WorldDestructionRuntime.Get_EncounterEpoch();
			for (std::uint32_t step = 0u; step < 1000u && room.Is_Ready() &&
				CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == room.m_ValtanPatternIdAudition.ePhase; ++step)
			{
				SERVER_PLAYER& player = room.m_Players.at(NEXT_OWNER_PLAYER);
				if ("STEP_08" == boss->strPatternStageId && PLAYER_ACTION_STATE::GRABBED != player.eAction)
				{
					// Position the real participant in the real 13_04 frontal hit volume.
					// Capture, damage, detach and completion are never injected by the fixture.
					const float radians = boss->fYawDegrees * 3.14159265358979323846f / 180.f;
					player.fPositionX = boss->fPositionX + std::sin(radians) * 2.f;
					player.fPositionY = boss->fPositionY;
					player.fPositionZ = boss->fPositionZ + std::cos(radians) * 2.f;
				}
				const auto hpBeforeTick = player.iCurrentHp;
				room.Tick(1.f / 30.f);
				captured = captured || (PLAYER_ACTION_STATE::GRABBED == player.eAction &&
					player.iAttachmentOwnerNetEntityId == boss->iNetEntityId &&
					player.iAttachmentPatternSequence == boss->iPatternSequence);
				if ("CATCH_COUNTER" == boss->strPatternStageId && 0u == counterTick)
					counterTick = room.m_iServerTick;
				if ("EXECUTE_TAIL" == boss->strPatternStageId && 0u == impactTick)
				{
					impactTick = room.m_iServerTick;
					executed = hpBeforeTick > 0u && 0u == player.iCurrentHp &&
						PLAYER_ACTION_STATE::DEAD == player.eAction && !player.isCombatReady &&
						PLAYER_ATTACHMENT_SLOT::NONE == player.eAttachmentSlot &&
						INVALID_NET_ENTITY_ID == player.iAttachmentOwnerNetEntityId &&
						0u == player.iAttachmentPatternSequence && room.m_TickDamageEvents.size() == 1u &&
						room.m_TickDamageEvents.front().iAmount == hpBeforeTick &&
						boss->iGrabExecutionCommittedPatternSequence == boss->iPatternSequence;
				}
				if (CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == room.m_ValtanPatternIdAudition.ePhase)
					completedTick = room.m_iServerTick;
			}
			SERVER_PLAYER& player = room.m_Players.at(NEXT_OWNER_PLAYER);
			const bool completedAfterTail = room.Is_Ready() && captured && executed &&
				0u != counterTick && impactTick - counterTick == 45u && completedTick - impactTick == 45u &&
				boss->strPatternId.empty() && 1u == boss->iPatternSequence &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult;
			for (std::uint32_t step = 0u; step < 500u && room.Is_Ready(); ++step)
				room.Tick(1.f / 30.f);
			const auto waitingEdges = readNextLifecycle(fixture.Session);
			const auto completed = std::find_if(waitingEdges.begin(), waitingEdges.end(), [](const auto& edge)
				{ return edge.iRequestSequence == 1u && edge.eState == VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED; });
			const auto waiting = std::find_if(waitingEdges.begin(), waitingEdges.end(), [](const auto& edge)
				{ return edge.iRequestSequence == 2u && edge.eState == VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER; });
			const auto waitingCount = std::count_if(waitingEdges.begin(), waitingEdges.end(), [](const auto& edge)
				{ return edge.iRequestSequence == 2u && edge.eState == VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER; });
			const bool heldForRevive = completedAfterTail && room.Is_Ready() && room.m_ValtanNextPattern &&
				0u == player.iCurrentHp && 1u == boss->iPatternSequence && boss->strPatternId.empty() &&
				completed != waitingEdges.end() && waiting != waitingEdges.end() && completed < waiting && waitingCount == 1;
			C2S_REVIVE_PLAYER revive{};
			revive.iClientSequence = 1u;
			room.Handle_RevivePlayer(NEXT_OWNER_SESSION, revive);
			room.Tick(1.f / 30.f);
			const bool promoted = CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternIdAudition.ePhase && 2u == room.m_ValtanPatternIdAudition.iRequestSequence;
			room.Tick(1.f / 30.f);
			tests.Require(VALTAN_AUDITION_RESULT::QUEUED == queued && heldForRevive && promoted &&
				room.Is_Ready() && "VALTAN_FIST_IN_OUT" == boss->strPatternId && 2u == boss->iPatternSequence &&
				CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == room.m_ValtanPatternIdAudition.ePhase &&
				0u != player.iCurrentHp && bossHp == boss->iCurrentHp &&
				roomEpoch == room.m_ValtanPatternIdAudition.iRoomAuditionEpoch &&
				worldEpoch == room.m_WorldDestructionRuntime.Get_EncounterEpoch(),
				"Real Trash capture executes all living players, completes its tail, waits without auto-revive, then starts Next after typed revive");
		}
		fixture.Session->Request_Close();
	}
	{
		/* Effect Tool names the already-spawned private-room boss and pattern by
		stable ID. The request resets only that boss, then CValtanBrain starts the
		exact product pattern on the following fixed tick. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID TOOL_SESSION = 4343u;
		constexpr PLAYER_ID TOOL_PLAYER = 79u;
		constexpr NET_ENTITY_ID TOOL_PLAYER_ENTITY = 902u;
		constexpr NET_ENTITY_ID TOOL_BOSS_ENTITY = 904u;
		const std::string bossPlacementId =
			"boss.valtan.character-select.lazy";
		const WORLD_BOOTSTRAP_PLACEMENT* placement =
			room.Find_Placement(bossPlacementId);
		SERVER_WORLD_ENTITY stagedBoss{};
		const bool builtBoss = room.Is_Ready() && nullptr != placement &&
			!placement->isEnabled &&
			room.Build_WorldEntity(
				*placement, TOOL_BOSS_ENTITY, stagedBoss);
		if (builtBoss)
			room.m_WorldEntities.push_back(std::move(stagedBoss));
		SERVER_WORLD_ENTITY* boss =
			room.Find_AuditionBoss(bossPlacementId);

		SERVER_PLAYER player{};
		player.iSessionId = TOOL_SESSION;
		player.iPlayerId = TOOL_PLAYER;
		player.iNetEntityId = TOOL_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
			boss->iPatternSequence = 9u;
			boss->BossCombat.iStateRevision = 15u;
			boss->iCurrentHp = 1u;
			boss->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			boss->strPatternId = "previous.pattern";
		}
		room.m_Players.emplace(TOOL_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(TOOL_SESSION, TOOL_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			TOOL_PLAYER_ENTITY, TOOL_PLAYER);
		/* The legacy Level panel and Effect Tool own independent sequence
		counters. A high legacy sequence must not make the Tool's first request
		look stale. */
		room.m_ValtanAuditionSequenceBySessionId.emplace(TOOL_SESSION, 100u);

		C2S_VALTAN_AUDITION_REQUEST blocked{};
		blocked.iRequestSequence = 1u;
		blocked.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		blocked.strBossPlacementId = bossPlacementId;
		blocked.strPatternId = "VALTAN_ARENA_BREAK_109";
		blocked.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT blockedResult =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, blocked, reportedBar);
		const bool blockedWithoutMutation = nullptr != boss &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				blockedResult &&
			1u == boss->iCurrentHp && 9u == boss->iPatternSequence &&
			"previous.pattern" == boss->strPatternId &&
			boss->PendingPatternIds.empty();

		C2S_VALTAN_AUDITION_REQUEST dash = blocked;
		dash.iRequestSequence = 2u;
		dash.strPatternId = "VALTAN_DASH_CHARGE";
		const VALTAN_AUDITION_RESULT queued =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, dash, reportedBar);
		boss = room.Find_AuditionBoss(bossPlacementId);
		const bool resetAndQueued = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == queued &&
			TOOL_BOSS_ENTITY == boss->iNetEntityId &&
			9u == boss->iPatternSequence &&
			boss->BossCombat.iStateRevision > 15u &&
			boss->iCurrentHp == boss->iMaximumHp &&
			SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
			boss->strPatternId.empty() &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_DASH_CHARGE" == boss->PendingPatternIds.front();
		const VALTAN_AUDITION_RESULT duplicate =
			room.Evaluate_ValtanAudition(
				TOOL_SESSION, dash, reportedBar);
		const bool receiptReplayDidNotQueueAgain = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == duplicate &&
			1u == boss->PendingPatternIds.size();

		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss(bossPlacementId);
		tests.Require(
			builtBoss && blockedWithoutMutation && resetAndQueued &&
			receiptReplayDidNotQueueAgain && nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_DASH_CHARGE" == boss->strPatternId &&
			10u == boss->iPatternSequence &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction,
			"Play the stable-ID Dash Charge through the Character Select Server boss while blocking arena-only patterns");
	}

	{
		/* A shared Valtan room can receive commands from two sessions in one
		   command-drain tick. The first stable-ID request owns its pending/active
		   occurrence; the second verdict is consumed but cannot reset it away. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID FIRST_SESSION = 4344u;
		constexpr SESSION_ID SECOND_SESSION = 4345u;
		constexpr PLAYER_ID FIRST_PLAYER = 81u;
		constexpr PLAYER_ID SECOND_PLAYER = 82u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER first{};
		first.iSessionId = FIRST_SESSION;
		first.iPlayerId = FIRST_PLAYER;
		first.iNetEntityId = 905u;
		first.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		first.iCurrentHp = 5500u;
		first.iMaximumHp = 5500u;
		first.isCombatReady = true;
		if (nullptr != boss)
		{
			first.fPositionX = boss->fPositionX + 2.f;
			first.fPositionY = boss->fPositionY;
			first.fPositionZ = boss->fPositionZ;
		}
		SERVER_PLAYER second = first;
		second.iSessionId = SECOND_SESSION;
		second.iPlayerId = SECOND_PLAYER;
		second.iNetEntityId = 906u;
		if (nullptr != boss)
			second.fPositionX = boss->fPositionX + 3.f;
		room.m_Players.emplace(FIRST_PLAYER, first);
		room.m_Players.emplace(SECOND_PLAYER, second);
		room.m_PlayerIdBySessionId.emplace(FIRST_SESSION, FIRST_PLAYER);
		room.m_PlayerIdBySessionId.emplace(SECOND_SESSION, SECOND_PLAYER);
		room.m_PlayerIdByEntityId.emplace(first.iNetEntityId, FIRST_PLAYER);
		room.m_PlayerIdByEntityId.emplace(second.iNetEntityId, SECOND_PLAYER);
		auto lifecycleSession = std::make_shared<CClientSession>(
			FIRST_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FIRST_SESSION, lifecycleSession);

		C2S_VALTAN_AUDITION_REQUEST firstRequest{};
		firstRequest.iRequestSequence = 1u;
		firstRequest.eOperation =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		firstRequest.strBossPlacementId = "boss.valtan.center";
		firstRequest.strPatternId = "VALTAN_DASH_CHARGE";
		firstRequest.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		C2S_VALTAN_AUDITION_REQUEST secondRequest = firstRequest;
		secondRequest.strPatternId = "VALTAN_WHIRLWIND";
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT firstResult =
			room.Evaluate_ValtanAudition(
				FIRST_SESSION, firstRequest, reportedBar);
		const VALTAN_AUDITION_RESULT secondResult =
			room.Evaluate_ValtanAudition(
				SECOND_SESSION, secondRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		const bool firstStillOwnsPending = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == firstResult &&
			VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE ==
				secondResult &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_DASH_CHARGE" == boss->PendingPatternIds.front() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternIdAudition.ePhase &&
			FIRST_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId &&
			1u == room.m_ValtanPatternIdAuditionSequenceBySessionId.at(
				SECOND_SESSION).Request.iRequestSequence;

		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		tests.Require(
			activated && firstStillOwnsPending && nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_DASH_CHARGE" == boss->strPatternId &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			FIRST_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId,
			"Keep the first shared-room stable-ID pattern when two sessions request different patterns before the fixed tick");

		const std::uint32_t firstActiveEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		const std::uint32_t firstActivePatternSequence =
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		C2S_VALTAN_AUDITION_REQUEST otherOwnerRestart = firstRequest;
		otherOwnerRestart.iRequestSequence = 2u;
		otherOwnerRestart.eOperation =
			VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID;
		otherOwnerRestart.iPredecessorRoomAuditionEpoch = firstActiveEpoch;
		otherOwnerRestart.iPredecessorPatternSequence =
			firstActivePatternSequence;
		otherOwnerRestart.ExpectedDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		otherOwnerRestart.ReplacementDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const VALTAN_AUDITION_RESULT otherOwnerRestartResult =
			room.Evaluate_ValtanAudition(
				SECOND_SESSION, otherOwnerRestart, reportedBar);
		const VALTAN_AUDITION_RESULT otherOwnerRetryResult =
			room.Evaluate_ValtanAudition(
				SECOND_SESSION, otherOwnerRestart, reportedBar);
		C2S_VALTAN_AUDITION_REQUEST alteredOtherOwnerRetry =
			otherOwnerRestart;
		alteredOtherOwnerRetry.strPatternId = "VALTAN_WHIRLWIND";
		const VALTAN_AUDITION_RESULT alteredOtherOwnerRetryResult =
			room.Evaluate_ValtanAudition(
				SECOND_SESSION, alteredOtherOwnerRetry, reportedBar);
		boss = room.Find_AuditionBoss();
		tests.Require(
			VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER ==
				otherOwnerRestartResult &&
			VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER ==
				otherOwnerRetryResult &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
				alteredOtherOwnerRetryResult &&
			nullptr != boss &&
			"VALTAN_DASH_CHARGE" == boss->strPatternId &&
			firstActivePatternSequence == boss->iPatternSequence &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			FIRST_SESSION ==
				room.m_ValtanPatternIdAudition.iOwnerSessionId &&
			1u == room.m_ValtanPatternIdAudition.iRequestSequence &&
			firstActiveEpoch ==
				room.m_ValtanPatternIdAudition.iRoomAuditionEpoch,
			"Reject another session's attempt to restart the active stable-ID occurrence without replacing its owner or identity");

		room.m_Players.at(FIRST_PLAYER).iCurrentHp = 1000000000u;
		room.m_Players.at(FIRST_PLAYER).iMaximumHp = 1000000000u;
		room.m_Players.at(SECOND_PLAYER).iCurrentHp = 1000000000u;
		room.m_Players.at(SECOND_PLAYER).iMaximumHp = 1000000000u;
		for (std::uint32_t tick = 0u; tick < 600u && room.Is_Ready() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase; ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		boss = room.Find_AuditionBoss();
		const bool completedDetected =
			nullptr != boss &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
				room.m_ValtanPatternIdAudition.ePhase &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss->PatternTerminalReceipt.eResult &&
			room.Flush_ValtanPatternIdAuditionLifecycle();

		C2S_VALTAN_AUDITION_REQUEST abortedRequest = firstRequest;
		abortedRequest.iRequestSequence = 2u;
		abortedRequest.strPatternId = "VALTAN_WHIRLWIND";
		const VALTAN_AUDITION_RESULT secondQueued =
			room.Evaluate_ValtanAudition(
				FIRST_SESSION, abortedRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		if (nullptr != boss)
		{
			boss->iCurrentHp = 0u;
			boss->eAction = SERVER_ENTITY_ACTION::DEAD;
		}
		const bool abortedDetected =
			!room.Refresh_ValtanPatternIdAuditionState() &&
			room.Flush_ValtanPatternIdAuditionLifecycle();

		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const GameplayDataRevision lifecycleRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const auto matchesLifecycleIdentity = [&lifecycleRevision](
			const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle,
			const C2S_VALTAN_AUDITION_REQUEST& request,
			const std::uint32_t epoch, const std::uint32_t patternSequence)
		{
			return request.iRequestSequence == lifecycle.iRequestSequence &&
				epoch == lifecycle.iRoomAuditionEpoch &&
				patternSequence == lifecycle.iPatternSequence &&
				request.strPatternId == lifecycle.strPatternId &&
				lifecycleRevision == lifecycle.PinnedDefinitionRevision;
		};
		// Reset invalidates the first completed hold before the replacement's
		// PENDING edge. Both ABORTED messages must keep their own exact identity.
		const std::uint32_t secondLifecyclePatternSequence =
			6u == lifecycleMessages.size() ?
				lifecycleMessages[4].iPatternSequence : 0u;
		const bool completeLifecycle = 6u == lifecycleMessages.size() &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[0].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				lifecycleMessages[1].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
				lifecycleMessages[2].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				lifecycleMessages[3].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[4].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				lifecycleMessages[5].eState &&
			secondLifecyclePatternSequence > firstActivePatternSequence &&
			"authoritative audition reset" == lifecycleMessages[3].strReason &&
			!lifecycleMessages[5].strReason.empty() &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.begin() + 4,
				[&](const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
				{
					return matchesLifecycleIdentity(lifecycle, firstRequest, 1u, 1u);
				}) &&
			std::all_of(lifecycleMessages.begin() + 4, lifecycleMessages.end(),
				[&, secondLifecyclePatternSequence](
					const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
				{
					return matchesLifecycleIdentity(
						lifecycle, abortedRequest, 2u,
						secondLifecyclePatternSequence);
				});
		tests.Require(
			completedDetected &&
			VALTAN_AUDITION_RESULT::QUEUED == secondQueued &&
			abortedDetected && completeLifecycle,
			"Emit correlated PENDING/ACTIVE/COMPLETED/ABORTED Valtan audition lifecycle edges with pinned revision");
		lifecycleSession->Request_Close();
	}

	{
		/* A direct Evaluate call cannot prove that the room handler preserves the
		   typed result/lifecycle order on the real session queue. Drive one exact
		   Restart through Handle_ValtanAudition, then inspect only the two protocol
		   frame kinds while ordinary snapshots are free to share the same FIFO. */
		constexpr SESSION_ID HANDLER_SESSION = 43460u;
		constexpr PLAYER_ID HANDLER_PLAYER = 43461u;
		constexpr NET_ENTITY_ID HANDLER_PLAYER_ENTITY = 43462u;
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		auto session = std::make_shared<CClientSession>(
			HANDLER_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		session->m_isSendRunning.store(true);
		room.m_Sessions.emplace(HANDLER_SESSION, session);
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER player{};
		player.iSessionId = HANDLER_SESSION;
		player.iPlayerId = HANDLER_PLAYER;
		player.iNetEntityId = HANDLER_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 1000000000u;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(HANDLER_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(HANDLER_SESSION, HANDLER_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			HANDLER_PLAYER_ENTITY, HANDLER_PLAYER);

		const auto decodeResultFrame = [](
			const auto& outbound, S2C_VALTAN_AUDITION_RESULT& result)
		{
			if (PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT !=
				outbound.ePacketType)
			{
				return false;
			}
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT != header.ePacketType ||
				outbound.Bytes.size() < PACKET_HEADER_BYTES)
			{
				return false;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			return Read_Message(reader, result) &&
				0u == reader.Get_RemainingSize();
		};
		const auto decodeLifecycleFrame = [](
			const auto& outbound, S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
		{
			if (PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
				outbound.ePacketType)
			{
				return false;
			}
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
					header.ePacketType ||
				outbound.Bytes.size() < PACKET_HEADER_BYTES)
			{
				return false;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			return Read_Message(reader, lifecycle) &&
				0u == reader.Get_RemainingSize();
		};
		struct AUDITION_OUTBOUND final
		{
			bool isResult = false;
			S2C_VALTAN_AUDITION_RESULT Result{};
			S2C_VALTAN_AUDITION_LIFECYCLE Lifecycle{};
		};
		const auto readAuditionOutbound = [&session, &decodeResultFrame,
			&decodeLifecycleFrame](
				const C2S_VALTAN_AUDITION_REQUEST& source)
		{
			std::vector<AUDITION_OUTBOUND> decoded;
			bool collecting = false;
			for (const auto& outbound : session->m_OutboundFrames)
			{
				AUDITION_OUTBOUND item{};
				if (decodeResultFrame(outbound, item.Result))
				{
					const bool exactSource =
						item.Result.iRequestSequence == source.iRequestSequence &&
						item.Result.eOperation == source.eOperation &&
						item.Result.strBossPlacementId == source.strBossPlacementId &&
						item.Result.strPatternId == source.strPatternId &&
						item.Result.iPredecessorRoomAuditionEpoch ==
							source.iPredecessorRoomAuditionEpoch &&
						item.Result.iPredecessorPatternSequence ==
							source.iPredecessorPatternSequence &&
						item.Result.iExpectedNextRequestSequence ==
							source.iExpectedNextRequestSequence &&
						item.Result.ExpectedDefinitionRevision ==
							source.ExpectedDefinitionRevision &&
						item.Result.ReplacementDefinitionRevision ==
							source.ReplacementDefinitionRevision;
					if (exactSource)
					{
						/* Snapshot coalescing can erase a frame before a numeric
						   deque cursor. Anchor the oracle on the latest exact result
						   tuple instead, then retain its following lifecycle FIFO. */
						decoded.clear();
						collecting = true;
						item.isResult = true;
						decoded.push_back(std::move(item));
					}
				}
				else if (collecting &&
					decodeLifecycleFrame(outbound, item.Lifecycle))
				{
					decoded.push_back(std::move(item));
				}
			}
			return decoded;
		};

		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		play.strBossPlacementId = "boss.valtan.center";
		play.strPatternId = "VALTAN_FIST_IN_OUT";
		play.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		room.Handle_ValtanAudition(HANDLER_SESSION, play);
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const bool started = activated && nullptr != boss && room.Is_Ready() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			play.strPatternId == boss->strPatternId;

		C2S_VALTAN_AUDITION_REQUEST restart = play;
		restart.iRequestSequence = 2u;
		restart.eOperation = VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID;
		restart.iPredecessorRoomAuditionEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		restart.iPredecessorPatternSequence =
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		restart.ExpectedDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		restart.ReplacementDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const std::uint32_t predecessorSequence =
			restart.iPredecessorPatternSequence;
		room.Handle_ValtanAudition(HANDLER_SESSION, restart);
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const auto successOutbound = readAuditionOutbound(restart);
		const bool successFifo = successOutbound.size() >= 4u &&
			successOutbound[0].isResult &&
			VALTAN_AUDITION_RESULT::QUEUED ==
				successOutbound[0].Result.eResult &&
			restart.iRequestSequence ==
				successOutbound[0].Result.iRequestSequence &&
			!successOutbound[1].isResult &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				successOutbound[1].Lifecycle.eState &&
			play.iRequestSequence ==
				successOutbound[1].Lifecycle.iRequestSequence &&
			!successOutbound[2].isResult &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				successOutbound[2].Lifecycle.eState &&
			restart.iRequestSequence ==
				successOutbound[2].Lifecycle.iRequestSequence &&
			predecessorSequence + 1u ==
				successOutbound[2].Lifecycle.iPatternSequence &&
			!successOutbound[3].isResult &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				successOutbound[3].Lifecycle.eState &&
			restart.iRequestSequence ==
				successOutbound[3].Lifecycle.iRequestSequence &&
			predecessorSequence + 1u ==
				successOutbound[3].Lifecycle.iPatternSequence;

		const std::uint32_t successorEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		const std::uint32_t successorSequence =
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		room.Handle_ValtanAudition(HANDLER_SESSION, restart);
		const bool retryFlushed = room.Flush_ValtanPatternIdAuditionLifecycle();
		const auto retryOutbound = readAuditionOutbound(restart);
		const bool exactRetryFifo = retryFlushed && 2u == retryOutbound.size() &&
			retryOutbound[0].isResult &&
			VALTAN_AUDITION_RESULT::QUEUED ==
				retryOutbound[0].Result.eResult &&
			!retryOutbound[1].isResult &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				retryOutbound[1].Lifecycle.eState &&
			restart.iRequestSequence ==
				retryOutbound[1].Lifecycle.iRequestSequence &&
			successorEpoch == retryOutbound[1].Lifecycle.iRoomAuditionEpoch &&
			successorSequence == retryOutbound[1].Lifecycle.iPatternSequence &&
			nullptr != boss && successorSequence == boss->iPatternSequence;

		C2S_VALTAN_AUDITION_REQUEST altered = restart;
		altered.strPatternId = "VALTAN_WHIRLWIND";
		room.Handle_ValtanAudition(HANDLER_SESSION, altered);
		const bool staleFlushed = room.Flush_ValtanPatternIdAuditionLifecycle();
		const auto staleOutbound = readAuditionOutbound(altered);
		const bool alteredStale = staleFlushed && 1u == staleOutbound.size() &&
			staleOutbound[0].isResult &&
			VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
				staleOutbound[0].Result.eResult &&
			altered.strPatternId == staleOutbound[0].Result.strPatternId &&
			nullptr != boss && successorSequence == boss->iPatternSequence &&
			restart.strPatternId == boss->strPatternId;

		/* A closed outbound boundary may not silently swallow the handler reply.
		   Re-evaluating the exact receipt is non-mutating; Send_Frame fails and the
		   handler must transition the session to terminal close. */
		session->m_isSendRunning.store(false);
		room.Handle_ValtanAudition(HANDLER_SESSION, restart);
		const bool sendFailureClosed = session->Is_Closing() &&
			!session->m_isSendRunning.load() &&
			session->m_OutboundFrames.empty() && nullptr != boss &&
			successorSequence == boss->iPatternSequence &&
			restart.strPatternId == boss->strPatternId;
		tests.Require(started,
			"Prepare one ACTIVE occurrence for the Restart room-handler wire oracle");
		tests.Require(successFifo,
			"Emit Restart QUEUED before predecessor ABORTED and successor PENDING/ACTIVE on the session FIFO");
		tests.Require(exactRetryFifo,
			"Replay an exact Restart receipt without resetting and emit its current ACTIVE lifecycle after the result");
		tests.Require(alteredStale,
			"Reject an altered same-sequence Restart receipt without lifecycle or boss mutation");
		tests.Require(sendFailureClosed,
			"Fail-close the Restart session when its typed result cannot enter the outbound FIFO");
	}

	{
		/* A QUEUED receipt outlives its current room slot. Preserve the last
		   authoritative terminal edge so an exact retry after the successor aborts
		   cannot strand the Client in QUEUED forever. */
		constexpr SESSION_ID RECEIPT_SESSION = 43470u;
		constexpr PLAYER_ID RECEIPT_PLAYER = 43471u;
		constexpr NET_ENTITY_ID RECEIPT_PLAYER_ENTITY = 43472u;
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		auto session = std::make_shared<CClientSession>(
			RECEIPT_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		session->m_isSendRunning.store(true);
		room.m_Sessions.emplace(RECEIPT_SESSION, session);
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER player{};
		player.iSessionId = RECEIPT_SESSION;
		player.iPlayerId = RECEIPT_PLAYER;
		player.iNetEntityId = RECEIPT_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = player.iMaximumHp = 1000000000u;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(RECEIPT_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(RECEIPT_SESSION, RECEIPT_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			RECEIPT_PLAYER_ENTITY, RECEIPT_PLAYER);

		C2S_VALTAN_AUDITION_REQUEST play{};
		play.iRequestSequence = 1u;
		play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		play.strBossPlacementId = "boss.valtan.center";
		play.strPatternId = "VALTAN_FIST_IN_OUT";
		play.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		room.Handle_ValtanAudition(RECEIPT_SESSION, play);
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		C2S_VALTAN_AUDITION_REQUEST restart = play;
		restart.iRequestSequence = 2u;
		restart.eOperation = VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID;
		restart.iPredecessorRoomAuditionEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		restart.iPredecessorPatternSequence =
			room.m_ValtanPatternIdAudition.iExpectedPatternSequence;
		restart.ExpectedDefinitionRevision =
			room.m_ValtanPatternIdAudition.PinnedDefinitionRevision;
		restart.ReplacementDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		room.Handle_ValtanAudition(RECEIPT_SESSION, restart);
		boss = room.Find_AuditionBoss();
		const std::uint32_t queuedSuccessorSequence = nullptr == boss ? 0u :
			boss->iPatternSequence + 1u;
		const std::uint32_t queuedSuccessorEpoch =
			room.m_ValtanPatternIdAudition.iRoomAuditionEpoch;
		if (nullptr != boss)
		{
			boss->iCurrentHp = 0u;
			boss->eAction = SERVER_ENTITY_ACTION::DEAD;
		}
		const bool aborted =
			!room.Refresh_ValtanPatternIdAuditionState() &&
			CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
				room.m_ValtanPatternIdAudition.ePhase &&
			room.Flush_ValtanPatternIdAuditionLifecycle();
		const auto receipt =
			room.m_ValtanPatternIdAuditionSequenceBySessionId.find(
				RECEIPT_SESSION);
		const bool storedAbort =
			room.m_ValtanPatternIdAuditionSequenceBySessionId.end() != receipt &&
			VALTAN_AUDITION_RESULT::QUEUED == receipt->second.Result &&
			receipt->second.LastLifecycle.has_value() &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				receipt->second.LastLifecycle->eState &&
			restart.iRequestSequence ==
				receipt->second.LastLifecycle->iRequestSequence &&
			queuedSuccessorEpoch ==
				receipt->second.LastLifecycle->iRoomAuditionEpoch &&
			queuedSuccessorSequence ==
				receipt->second.LastLifecycle->iPatternSequence;

		const std::size_t retryBegin = session->m_OutboundFrames.size();
		room.Handle_ValtanAudition(RECEIPT_SESSION, restart);
		const bool retryFlushed = room.Flush_ValtanPatternIdAuditionLifecycle();
		std::vector<S2C_VALTAN_AUDITION_RESULT> results;
		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> lifecycles;
		std::vector<bool> frameOrder;
		for (std::size_t index = retryBegin;
			index < session->m_OutboundFrames.size(); ++index)
		{
			const auto& outbound = session->m_OutboundFrames[index];
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				outbound.Bytes.size() < PACKET_HEADER_BYTES)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			if (PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT ==
				outbound.ePacketType)
			{
				S2C_VALTAN_AUDITION_RESULT result{};
				if (Read_Message(reader, result) &&
					0u == reader.Get_RemainingSize())
				{
					results.push_back(std::move(result));
					frameOrder.push_back(true);
				}
			}
			else if (PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE ==
				outbound.ePacketType)
			{
				S2C_VALTAN_AUDITION_LIFECYCLE lifecycle{};
				if (Read_Message(reader, lifecycle) &&
					0u == reader.Get_RemainingSize())
				{
					lifecycles.push_back(std::move(lifecycle));
					frameOrder.push_back(false);
				}
			}
		}
		const bool reconciled = retryFlushed &&
			2u == frameOrder.size() && frameOrder[0] && !frameOrder[1] &&
			1u == results.size() && 1u == lifecycles.size() &&
			VALTAN_AUDITION_RESULT::QUEUED == results[0].eResult &&
			restart.iRequestSequence == results[0].iRequestSequence &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED ==
				lifecycles[0].eState &&
			restart.iRequestSequence == lifecycles[0].iRequestSequence &&
			queuedSuccessorEpoch == lifecycles[0].iRoomAuditionEpoch &&
			queuedSuccessorSequence == lifecycles[0].iPatternSequence;
		tests.Require(
			activated && nullptr != boss && aborted && storedAbort && reconciled,
			"Replay a queued Restart receipt with its stored ABORTED lifecycle after the successor is no longer current");
		session->Request_Close();
	}

	{
		/* Exercise the 130-bar floor wipe through the same stable-ID command and
		   fixed room tick path the Effect Tool uses. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID FLOOR_SESSION = 4346u;
		constexpr PLAYER_ID FLOOR_PLAYER = 83u;
		constexpr NET_ENTITY_ID FLOOR_PLAYER_ENTITY = 907u;
		constexpr std::uint32_t FLOOR_PLAYER_HP = 1000000000u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const BOSS_PATTERN_DEFINITION* floorPattern = nullptr;
		if (nullptr != patterns)
		{
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return "VALTAN_FLOOR_WIPE_130" == pattern.strPatternId;
				});
			if (patterns->end() != found)
				floorPattern = &*found;
		}
		const BOSS_PATTERN_STAGE_DEFINITION* intervalDefinition = nullptr;
		const BOSS_PATTERN_STAGE_DEFINITION* secondSmashDefinition = nullptr;
		if (nullptr != floorPattern)
		{
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage :
				floorPattern->Stages)
			{
				if ("INTERVAL" == stage.strStageId)
					intervalDefinition = &stage;
				else if ("SECOND_SMASH" == stage.strStageId)
					secondSmashDefinition = &stage;
			}
		}
		const auto hasSameRootMotion = [](
			const std::vector<ROOT_MOTION_SAMPLE>& left,
			const std::vector<ROOT_MOTION_SAMPLE>& right)
		{
			return left.size() == right.size() && std::equal(
				left.begin(), left.end(), right.begin(),
				[](const ROOT_MOTION_SAMPLE& leftSample,
					const ROOT_MOTION_SAMPLE& rightSample)
				{
					return leftSample.iTimeMs == rightSample.iTimeMs &&
						leftSample.fForward == rightSample.fForward &&
						leftSample.fLateral == rightSample.fLateral;
				});
		};
		const bool catalogCarriesFloorMotions =
			nullptr != intervalDefinition &&
			2000u == intervalDefinition->iDurationMs &&
			!intervalDefinition->Motion.RootMotion.empty() &&
			2000u == intervalDefinition->Motion.RootMotion.back().iTimeMs &&
			nullptr != secondSmashDefinition &&
			500u == secondSmashDefinition->iDurationMs &&
			!secondSmashDefinition->Motion.RootMotion.empty() &&
			500u == secondSmashDefinition->Motion.RootMotion.back().iTimeMs;

		SERVER_PLAYER player{};
		player.iSessionId = FLOOR_SESSION;
		player.iPlayerId = FLOOR_PLAYER;
		player.iNetEntityId = FLOOR_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		/* The authored wipe still lands during this lifecycle test. Keep the
		   engaged observer alive so RECOVERY can finish naturally. */
		player.iCurrentHp = FLOOR_PLAYER_HP;
		player.iMaximumHp = FLOOR_PLAYER_HP;
		player.isCombatReady = true;
		if (nullptr != boss)
		{
			player.fPositionX = boss->fPositionX + 2.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
		}
		room.m_Players.emplace(FLOOR_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(FLOOR_SESSION, FLOOR_PLAYER);
		room.m_PlayerIdByEntityId.emplace(
			FLOOR_PLAYER_ENTITY, FLOOR_PLAYER);
		auto lifecycleSession = std::make_shared<CClientSession>(
			FLOOR_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FLOOR_SESSION, lifecycleSession);

		C2S_VALTAN_AUDITION_REQUEST request{};
		request.iRequestSequence = 1u;
		request.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
		request.strBossPlacementId = "boss.valtan.center";
		request.strPatternId = "VALTAN_FLOOR_WIPE_130";
		request.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		std::uint32_t reportedBar = 0u;
		const VALTAN_AUDITION_RESULT queued = room.Evaluate_ValtanAudition(
			FLOOR_SESSION, request, reportedBar);

		std::vector<std::string> observedStageIds;
		bool runtimeStageActionsMatch = true;
		bool intervalMotionActivated = false;
		bool secondSmashMotionActivated = false;
		bool observedPatternInvulnerability = false;
		bool keptPatternInvulnerability = true;
		std::uint32_t firstSmashDamagePulseCount = 0u;
		std::uint32_t secondSmashDamagePulseCount = 0u;
		std::uint64_t appliedFloorDamage = 0u;
		bool completedNaturally = false;
		for (std::uint32_t tick = 0u; tick < 240u && !completedNaturally;
			++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
			{
				runtimeStageActionsMatch = false;
				break;
			}
			if ("VALTAN_FLOOR_WIPE_130" == boss->strPatternId &&
				!boss->strPatternStageId.empty())
			{
				observedPatternInvulnerability = true;
				keptPatternInvulnerability = keptPatternInvulnerability &&
					boss->bPatternInvulnerable;
				if (observedStageIds.empty() ||
					observedStageIds.back() != boss->strPatternStageId)
				{
					observedStageIds.push_back(boss->strPatternStageId);
					const bool actionMatches =
						(("WINDUP" == boss->strPatternStageId ||
						  "INTERVAL" == boss->strPatternStageId) &&
						 SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction) ||
						(("FIRST_SMASH" == boss->strPatternStageId ||
						  "SECOND_SMASH" == boss->strPatternStageId) &&
						 SERVER_ENTITY_ACTION::PATTERN_ACTIVE == boss->eAction) ||
						("RECOVERY" == boss->strPatternStageId &&
						 SERVER_ENTITY_ACTION::PATTERN_RECOVERY == boss->eAction);
					runtimeStageActionsMatch =
						runtimeStageActionsMatch && actionMatches;
				}
				if ("INTERVAL" == boss->strPatternStageId &&
					nullptr != intervalDefinition)
				{
					intervalMotionActivated =
						2000u == boss->iPatternStageDurationMs &&
						hasSameRootMotion(
							boss->PatternStageRootMotion,
							intervalDefinition->Motion.RootMotion);
				}
				else if ("SECOND_SMASH" == boss->strPatternStageId &&
					nullptr != secondSmashDefinition)
				{
					secondSmashMotionActivated =
						500u == boss->iPatternStageDurationMs &&
						hasSameRootMotion(
							boss->PatternStageRootMotion,
							secondSmashDefinition->Motion.RootMotion);
				}
			}
			for (const DAMAGE_EVENT& damage : room.m_TickDamageEvents)
			{
				if (FLOOR_PLAYER_ENTITY != damage.iTargetNetEntityId ||
					damage.isOutgoing)
				{
					continue;
				}
				appliedFloorDamage += damage.iAmount;
				if (nullptr != boss &&
					"FIRST_SMASH" == boss->strPatternStageId)
				{
					++firstSmashDamagePulseCount;
				}
				else if (nullptr != boss &&
					"SECOND_SMASH" == boss->strPatternStageId)
				{
					++secondSmashDamagePulseCount;
				}
			}
			completedNaturally =
				CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
					room.m_ValtanPatternIdAudition.ePhase &&
				!observedStageIds.empty() &&
				"RECOVERY" == observedStageIds.back() &&
				boss->strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction;
		}
		const auto finishedPlayer = room.m_Players.find(FLOOR_PLAYER);
		const bool bothDamagePulsesApplied =
			room.m_Players.end() != finishedPlayer &&
			1u == firstSmashDamagePulseCount &&
			1u == secondSmashDamagePulseCount &&
			0u != appliedFloorDamage &&
			static_cast<std::uint64_t>(finishedPlayer->second.iCurrentHp) +
				appliedFloorDamage == FLOOR_PLAYER_HP;
		const bool invulnerabilityLifecycle =
			observedPatternInvulnerability && keptPatternInvulnerability &&
			nullptr != boss && !boss->bPatternInvulnerable;

		std::vector<S2C_VALTAN_AUDITION_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_VALTAN_AUDITION_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const GameplayDataRevision lifecycleRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		const bool completedLifecycle = 3u == lifecycleMessages.size() &&
			VALTAN_AUDITION_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages[0].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
				lifecycleMessages[1].eState &&
			VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
				lifecycleMessages[2].eState &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.end(),
				[&lifecycleRevision](
					const S2C_VALTAN_AUDITION_LIFECYCLE& lifecycle)
				{
					return 1u == lifecycle.iRequestSequence &&
						1u == lifecycle.iRoomAuditionEpoch &&
						1u == lifecycle.iPatternSequence &&
						"VALTAN_FLOOR_WIPE_130" == lifecycle.strPatternId &&
						lifecycle.PinnedDefinitionRevision == lifecycleRevision;
				});
		const std::uint32_t completedPatternSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		const std::uint32_t completedRotationStepIndex = nullptr == boss ?
			0u : boss->iRotationStepIndex;
		bool heldIdleAfterCompletion = nullptr != boss &&
			boss->bAutomaticPatternSequenceAuditionOverride &&
			boss->bAutomaticPatternSequenceAuditionHold;
		for (std::uint32_t tick = 0u; tick < 60u; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			heldIdleAfterCompletion = heldIdleAfterCompletion &&
				nullptr != boss && boss->strPatternId.empty() &&
				boss->PendingPatternIds.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
				completedPatternSequence == boss->iPatternSequence &&
				completedRotationStepIndex == boss->iRotationStepIndex &&
				boss->bAutomaticPatternSequenceAuditionOverride &&
				boss->bAutomaticPatternSequenceAuditionHold;
		}

		C2S_VALTAN_AUDITION_REQUEST replayRequest = request;
		replayRequest.iRequestSequence = 2u;
		const VALTAN_AUDITION_RESULT replayQueued =
			room.Evaluate_ValtanAudition(
				FLOOR_SESSION, replayRequest, reportedBar);
		boss = room.Find_AuditionBoss();
		const bool resetReleasedHold = nullptr != boss &&
			VALTAN_AUDITION_RESULT::QUEUED == replayQueued &&
			!boss->bAutomaticPatternSequenceAuditionOverride &&
			!boss->bAutomaticPatternSequenceAuditionHold &&
			1u == boss->PendingPatternIds.size() &&
			"VALTAN_FLOOR_WIPE_130" == boss->PendingPatternIds.front();
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const std::uint32_t expectedReplayPatternSequence =
			(std::numeric_limits<std::uint32_t>::max)() ==
				completedPatternSequence ?
			1u : completedPatternSequence + 1u;
		const bool replayStarted = nullptr != boss &&
			boss->PendingPatternIds.empty() &&
			"VALTAN_FLOOR_WIPE_130" == boss->strPatternId &&
			expectedReplayPatternSequence == boss->iPatternSequence &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == boss->eAction;
		const std::vector<std::string> expectedStageIds{
			"WINDUP", "FIRST_SMASH", "INTERVAL",
			"SECOND_SMASH", "RECOVERY" };
		tests.Require(
			activated && catalogCarriesFloorMotions &&
			VALTAN_AUDITION_RESULT::QUEUED == queued &&
			observedStageIds == expectedStageIds &&
			runtimeStageActionsMatch && intervalMotionActivated &&
			secondSmashMotionActivated && bothDamagePulsesApplied &&
			invulnerabilityLifecycle && completedNaturally && completedLifecycle,
			"Play FLOOR_WIPE_130 through all five room-ticked stages with its 2000/500 ms root-motion clips, both damage pulses, invulnerability, and completed lifecycle");
		tests.Require(
			heldIdleAfterCompletion && resetReleasedHold && replayStarted,
			"Hold a completed stable-ID audition idle without advancing the Product rotation, then reset and replay the next stable-ID request");
		lifecycleSession->Request_Close();
	}

	{
		/* Boss Tool flows are Debug authoring programs, but every slot still runs
		   through the Product ordered-sequence brain. The test starts from slot two,
		   retries its stable cursor after a targetless IDLE wait, repeats the pattern
		   under the next stable slot ID, and verifies stop-after-current on a fresh run. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID FLOW_SESSION = 4350u;
		constexpr PLAYER_ID FLOW_PLAYER = 90u;
		constexpr NET_ENTITY_ID FLOW_PLAYER_ENTITY = 920u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		SERVER_PLAYER player{};
		player.iSessionId = FLOW_SESSION;
		player.iPlayerId = FLOW_PLAYER;
		player.iNetEntityId = FLOW_PLAYER_ENTITY;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 1u;
		player.iMaximumHp = 1000000000u;
		player.isCombatReady = false;
		room.m_Players.emplace(FLOW_PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(FLOW_SESSION, FLOW_PLAYER);
		room.m_PlayerIdByEntityId.emplace(FLOW_PLAYER_ENTITY, FLOW_PLAYER);

		C2S_DEBUG_VALTAN_PATTERN_FLOW_START request{};
		request.iRequestSequence = 1u;
		request.ExpectedDefinitionRevision =
			room.m_GameplayCatalog.Get_ActiveRevision();
		request.strBossPlacementId = "boss.valtan.center";
		request.strFlowId = "flow.valtan.server-contract";
		request.strFlowRevision = std::string(64u, 'a');
		request.strStartSlotId = "flow.slot.000002";
		request.iInterStepPursuitMs = 1000u;
		request.Slots = {
			{ "flow.slot.000001", "VALTAN_DASH_CHARGE" },
			{ "flow.slot.000002", "VALTAN_WHIRLWIND" },
			{ "flow.slot.000003", "VALTAN_WHIRLWIND" }
		};
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START duplicateSlot = request;
		duplicateSlot.Slots[1].strSlotId = duplicateSlot.Slots[0].strSlotId;
		std::uint32_t roomFlowEpoch = 0u;
		GameplayDataRevision pinnedRevision{};
		std::string flowReason;
		const std::uint32_t patternSequenceBefore = nullptr == boss ?
			0u : boss->iPatternSequence;
		const VALTAN_PATTERN_FLOW_RESULT duplicateSlotResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, duplicateSlot, roomFlowEpoch,
				pinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const bool invalidWasTransactional = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW ==
				duplicateSlotResult &&
			patternSequenceBefore == boss->iPatternSequence &&
			!room.Is_ValtanPatternFlowRunning();

		request.iRequestSequence = 2u;
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START staleDefinition = request;
		staleDefinition.ExpectedDefinitionRevision.Bytes.front() ^= 0xffu;
		const std::uint32_t stalePatternSequenceBefore =
			nullptr == boss ? 0u : boss->iPatternSequence;
		const SERVER_PLAYER stalePlayerBefore = room.m_Players.at(FLOW_PLAYER);
		const VALTAN_PATTERN_FLOW_RESULT staleDefinitionResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, staleDefinition, roomFlowEpoch,
				pinnedRevision, flowReason);
		const std::string staleDefinitionReason = flowReason;
		const VALTAN_PATTERN_FLOW_RESULT staleDefinitionRetry =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, staleDefinition, roomFlowEpoch,
				pinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const bool staleDefinitionWasTransactional = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_DEFINITION ==
				staleDefinitionResult &&
			staleDefinitionResult == staleDefinitionRetry &&
			staleDefinitionReason == flowReason &&
			0u == roomFlowEpoch && !pinnedRevision.Is_Valid() &&
			stalePatternSequenceBefore == boss->iPatternSequence &&
			stalePlayerBefore.iCurrentHp ==
				room.m_Players.at(FLOW_PLAYER).iCurrentHp &&
			stalePlayerBefore.isCombatReady ==
				room.m_Players.at(FLOW_PLAYER).isCombatReady &&
			!room.Is_ValtanPatternFlowRunning();

		request.iRequestSequence = 3u;
		auto lifecycleSession = std::make_shared<CClientSession>(
			FLOW_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		lifecycleSession->m_isSendRunning.store(true);
		room.m_Sessions.emplace(FLOW_SESSION, lifecycleSession);
		const VALTAN_PATTERN_FLOW_RESULT queued =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, roomFlowEpoch,
				pinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const bool stagedSuffix = staleDefinitionWasTransactional &&
			nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == queued &&
			flowReason.empty() && 0u != roomFlowEpoch &&
			pinnedRevision.Is_Valid() &&
			CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
				room.m_ValtanPatternFlowAudition.ePhase &&
			1u == room.m_ValtanPatternFlowAudition.iStartSlotIndex &&
			2u == room.m_ValtanPatternFlowAudition.Sequence.PatternIds.size() &&
			std::vector<std::uint32_t>{ 1000u } ==
				room.m_ValtanPatternFlowAudition.Sequence.TransitionPursuitMs &&
			std::vector<std::uint32_t>{ 30u } ==
				room.m_ValtanPatternFlowAudition.Sequence.TransitionPursuitTicks &&
			"VALTAN_WHIRLWIND" ==
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds[0] &&
			"VALTAN_WHIRLWIND" ==
				room.m_ValtanPatternFlowAudition.Sequence.PatternIds[1] &&
			room.m_Players.at(FLOW_PLAYER).iCurrentHp ==
				room.m_Players.at(FLOW_PLAYER).iMaximumHp &&
			room.m_Players.at(FLOW_PLAYER).isCombatReady;
		std::uint32_t duplicateEpoch = 0u;
		GameplayDataRevision duplicateRevision{};
		std::string duplicateReason;
		const VALTAN_PATTERN_FLOW_RESULT duplicateStart =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, duplicateEpoch,
				duplicateRevision, duplicateReason);
		const bool duplicateKeptOneProgram =
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicateStart &&
			roomFlowEpoch == duplicateEpoch && pinnedRevision == duplicateRevision &&
			room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedDuplicate = request;
		mismatchedDuplicate.strFlowId = "flow.valtan.other-contract";
		std::uint32_t mismatchedEpoch = 123u;
		GameplayDataRevision mismatchedRevision = pinnedRevision;
		std::string mismatchedReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedDuplicateResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedDuplicate, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedDuplicateRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedDuplicateResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedFlowRevision = request;
		mismatchedFlowRevision.strFlowRevision = std::string(64u, 'b');
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedFlowRevisionResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedFlowRevision, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedFlowRevisionRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedFlowRevisionResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedSuffix = request;
		mismatchedSuffix.strStartSlotId = mismatchedSuffix.Slots.front().strSlotId;
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedSuffixResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedSuffix, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedSuffixRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedSuffixResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START mismatchedSlotPayload = request;
		mismatchedSlotPayload.Slots.front().strPatternId = "VALTAN_WHIRLWIND";
		mismatchedEpoch = 123u;
		mismatchedRevision = pinnedRevision;
		mismatchedReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT mismatchedSlotPayloadResult =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, mismatchedSlotPayload, mismatchedEpoch,
				mismatchedRevision, mismatchedReason);
		const bool mismatchedSlotPayloadRejected =
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedSlotPayloadResult &&
			0u == mismatchedEpoch && !mismatchedRevision.Is_Valid() &&
			!mismatchedReason.empty() && room.Is_ValtanPatternFlowRunning();

		std::vector<std::string> observedStarts;
		std::uint32_t lastObservedPatternSequence = patternSequenceBefore;
		std::uint32_t firstObservedPatternSequence = 0u;
		bool injectedDeath = false;
		bool observedIdleWait = false;
		bool observedRetry = false;
		bool markedStateBetweenSlots = false;
		bool stateSurvivedIntoSecondSlot = false;
		for (std::uint32_t tick = 0u;
			tick < 900u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr == boss)
				break;
			if (!boss->strPatternId.empty() &&
				boss->iPatternSequence != lastObservedPatternSequence)
			{
				observedStarts.push_back(boss->strPatternId);
				lastObservedPatternSequence = boss->iPatternSequence;
				if (1u == observedStarts.size())
					firstObservedPatternSequence = boss->iPatternSequence;
				if (2u == observedStarts.size() &&
					0u == boss->iRotationStepIndex &&
					boss->iPatternSequence > firstObservedPatternSequence)
				{
					observedRetry = true;
				}
				if (1u == boss->iRotationStepIndex)
					stateSurvivedIntoSecondSlot =
						boss->iCurrentHp == boss->iMaximumHp - 123u;
			}
			if (!injectedDeath && 1u == observedStarts.size())
			{
				SERVER_PLAYER& livePlayer = room.m_Players.at(FLOW_PLAYER);
				livePlayer.iCurrentHp = 0u;
				livePlayer.eAction = PLAYER_ACTION_STATE::DEAD;
				livePlayer.isCombatReady = false;
				injectedDeath = true;
				continue;
			}
			if (injectedDeath && !observedIdleWait &&
				boss->strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == boss->eAction &&
				!boss->bAutomaticPatternSequenceStepRunning &&
				!boss->bAutomaticPatternSequencePausedForRevive &&
				!boss->bMechanicLedgerRequiresReset &&
				0u == boss->iRotationStepIndex &&
				CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
					room.m_ValtanPatternFlowAudition.ePhase)
			{
				observedIdleWait = true;
				C2S_REVIVE_PLAYER revive{};
				revive.iClientSequence = 1u;
				room.Handle_RevivePlayer(FLOW_SESSION, revive);
				continue;
			}
			if (!markedStateBetweenSlots &&
				CGameRoom::VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING ==
					room.m_ValtanPatternFlowAudition.ePhase &&
				1u == boss->iRotationStepIndex)
			{
				boss->iCurrentHp = boss->iMaximumHp - 123u;
				markedStateBetweenSlots = true;
			}
		}
		boss = room.Find_AuditionBoss();
		const std::vector<std::string> expectedStarts{
			"VALTAN_WHIRLWIND", "VALTAN_WHIRLWIND",
			"VALTAN_WHIRLWIND" };
		bool heldAfterNaturalCompletion = nullptr != boss &&
			!room.Is_ValtanPatternFlowRunning() &&
			boss->bAutomaticPatternSequenceAuditionOverride &&
			boss->bAutomaticPatternSequenceAuditionHold &&
			boss->strPatternId.empty() &&
			observedStarts == expectedStarts;
		const std::uint32_t completedPatternSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		for (std::uint32_t tick = 0u; tick < 30u; ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			heldAfterNaturalCompletion = heldAfterNaturalCompletion &&
				nullptr != boss && boss->strPatternId.empty() &&
				completedPatternSequence == boss->iPatternSequence;
		}

		std::vector<S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE> lifecycleMessages;
		for (const auto& outbound : lifecycleSession->m_OutboundFrames)
		{
			PACKET_HEADER header{};
			if (!Read_Packet_Header(outbound.Bytes, header) ||
				PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE !=
					header.ePacketType)
			{
				continue;
			}
			CPacketReader reader{ std::span<const std::uint8_t>(
				outbound.Bytes.data() + PACKET_HEADER_BYTES,
				outbound.Bytes.size() - PACKET_HEADER_BYTES) };
			S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE decoded{};
			if (Read_Message(reader, decoded) &&
				0u == reader.Get_RemainingSize())
			{
				lifecycleMessages.push_back(std::move(decoded));
			}
		}
		const bool lifecycleCorrelated =
			!lifecycleMessages.empty() &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING ==
				lifecycleMessages.front().eState &&
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD ==
				lifecycleMessages.back().eState &&
			lifecycleMessages.end() == std::find_if(
				lifecycleMessages.begin(), lifecycleMessages.end(),
				[](const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& lifecycle)
				{
					return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE ==
						lifecycle.eState;
				}) &&
			2u <= static_cast<std::size_t>(std::count_if(
				lifecycleMessages.begin(), lifecycleMessages.end(),
				[](const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& lifecycle)
				{
					return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING ==
						lifecycle.eState;
				})) &&
			std::all_of(lifecycleMessages.begin(), lifecycleMessages.end(),
				[roomFlowEpoch, &pinnedRevision, &request](
					const S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& lifecycle)
				{
					return lifecycle.iRequestSequence == request.iRequestSequence &&
						lifecycle.iRoomFlowEpoch == roomFlowEpoch &&
						lifecycle.strFlowId == request.strFlowId &&
						lifecycle.strFlowRevision == request.strFlowRevision &&
						lifecycle.strStartSlotId == request.strStartSlotId &&
						3u == lifecycle.iSlotCount &&
						lifecycle.iCurrentSlotOrdinal >= 2u &&
						lifecycle.iCurrentSlotOrdinal <= 3u &&
						lifecycle.PinnedDefinitionRevision == pinnedRevision;
				});

		request.iRequestSequence = 4u;
		request.strStartSlotId = request.Slots.front().strSlotId;
		std::uint32_t stopFlowEpoch = 0u;
		GameplayDataRevision stopPinnedRevision{};
		flowReason.clear();
		const VALTAN_PATTERN_FLOW_RESULT replayQueued =
			room.Evaluate_ValtanPatternFlowStart(
				FLOW_SESSION, request, stopFlowEpoch,
				stopPinnedRevision, flowReason);
		boss = room.Find_AuditionBoss();
		const std::uint32_t stopStartSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		for (std::uint32_t tick = 0u;
			tick < 30u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
			boss = room.Find_AuditionBoss();
			if (nullptr != boss && boss->bAutomaticPatternSequenceStepRunning)
				break;
		}
		const std::uint32_t stopRunningPatternSequence = nullptr == boss ?
			0u : boss->iPatternSequence;
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
		stop.iControlSequence = 1u;
		stop.strFlowId = request.strFlowId;
		stop.iRoomFlowEpoch = stopFlowEpoch;
		std::uint32_t reportedStopEpoch = 0u;
		GameplayDataRevision reportedStopRevision{};
		std::string stopReason;
		const VALTAN_PATTERN_FLOW_RESULT stopQueued =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, reportedStopEpoch,
				reportedStopRevision, stopReason);
		std::uint32_t duplicateStopEpoch = 0u;
		GameplayDataRevision duplicateStopRevision{};
		std::string duplicateStopReason;
		const VALTAN_PATTERN_FLOW_RESULT duplicateStop =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, duplicateStopEpoch,
				duplicateStopRevision, duplicateStopReason);
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT mismatchedStop = stop;
		mismatchedStop.strFlowId = "flow.valtan.other-contract";
		std::uint32_t mismatchedStopEpoch = 123u;
		GameplayDataRevision mismatchedStopRevision = reportedStopRevision;
		std::string mismatchedStopReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedStopResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, mismatchedStop, mismatchedStopEpoch,
				mismatchedStopRevision, mismatchedStopReason);
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
			mismatchedStopEpochRequest = stop;
		++mismatchedStopEpochRequest.iRoomFlowEpoch;
		std::uint32_t mismatchedStopEpochReported = 123u;
		GameplayDataRevision mismatchedStopEpochRevision = reportedStopRevision;
		std::string mismatchedStopEpochReason;
		const VALTAN_PATTERN_FLOW_RESULT mismatchedStopEpochResult =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, mismatchedStopEpochRequest,
				mismatchedStopEpochReported, mismatchedStopEpochRevision,
				mismatchedStopEpochReason);
		const bool stopDuplicateIdentityExact =
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == duplicateStop &&
			duplicateStopEpoch == reportedStopEpoch &&
			duplicateStopRevision == reportedStopRevision &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedStopResult &&
			0u == mismatchedStopEpoch && !mismatchedStopRevision.Is_Valid() &&
			!mismatchedStopReason.empty() &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW ==
				mismatchedStopEpochResult &&
			0u == mismatchedStopEpochReported &&
			!mismatchedStopEpochRevision.Is_Valid() &&
			!mismatchedStopEpochReason.empty();
		for (std::uint32_t tick = 0u;
			tick < 600u && room.Is_ValtanPatternFlowRunning(); ++tick)
		{
			room.Tick(1.f / 30.f);
		}
		boss = room.Find_AuditionBoss();
		const bool stoppedAfterOne = nullptr != boss &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == replayQueued &&
			VALTAN_PATTERN_FLOW_RESULT::QUEUED == stopQueued &&
			stopReason.empty() && stopFlowEpoch == reportedStopEpoch &&
			stopPinnedRevision == reportedStopRevision &&
			!room.Is_ValtanPatternFlowRunning() &&
			0u != stopRunningPatternSequence &&
			stopRunningPatternSequence != stopStartSequence &&
			boss->strPatternId.empty() &&
			!boss->PendingPatternFollowup.Is_Pending() &&
			1u == boss->iRotationStepIndex &&
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				boss->PatternTerminalReceipt.eResult &&
			boss->PatternTerminalReceipt.iRootPatternSequence ==
				stopRunningPatternSequence &&
			boss->PatternTerminalReceipt.iPatternSequence ==
				boss->iPatternSequence &&
			boss->bAutomaticPatternSequenceAuditionHold;
		stop.iControlSequence = 2u;
		std::uint32_t staleEpoch = 0u;
		GameplayDataRevision staleRevision{};
		std::string staleReason;
		const VALTAN_PATTERN_FLOW_RESULT staleStop =
			room.Evaluate_ValtanPatternFlowStopAfterCurrent(
				FLOW_SESSION, stop, staleEpoch, staleRevision, staleReason);
		tests.Require(
			activated && invalidWasTransactional && stagedSuffix &&
			duplicateKeptOneProgram && mismatchedDuplicateRejected &&
			mismatchedFlowRevisionRejected && mismatchedSuffixRejected &&
			mismatchedSlotPayloadRejected &&
			observedIdleWait && observedRetry &&
			markedStateBetweenSlots && stateSurvivedIntoSecondSlot &&
			heldAfterNaturalCompletion && lifecycleCorrelated,
			"Run a saved slot-two suffix through a targetless IDLE retry and duplicate ordered patterns with preserved between-slot state, terminal hold, and correlated lifecycle");
		tests.Require(
			stopDuplicateIdentityExact && stoppedAfterOne &&
			VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW == staleStop &&
			!staleReason.empty(),
			"Stop a running Boss Tool flow after its current occurrence and reject a stale flow epoch without starting the next slot");
		lifecycleSession->Request_Close();
	}


	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		const bool activated =
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();
		constexpr std::uint32_t START_TICK =
			(std::numeric_limits<std::uint32_t>::max)() - 100u;
		constexpr std::uint32_t WRAPPED_BREAK_TICK = 375u;
		if (nullptr != boss)
		{
			boss->iPatternSequence = 1u;
			boss->strPatternId = "VALTAN_FOUR_PILLARS_105";
			boss->strPatternStageId = "RECOVERY";
		}
		room.m_bPillarAuditionCycleArmed = true;
		const bool staged = nullptr != boss &&
			room.Apply_EncounterPropStageEntry(*boss, START_TICK);
		const bool scheduledAcrossWrap = staged &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool keptBeforeWrap = room.Commit_DueEncounterProps(
			(std::numeric_limits<std::uint32_t>::max)()) &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool keptBeforeDeadline =
			room.Commit_DueEncounterProps(WRAPPED_BREAK_TICK - 1u) &&
			WRAPPED_BREAK_TICK == room.m_iPillarAuditionBreakTick;
		const bool committedAtDeadline =
			room.Commit_DueEncounterProps(WRAPPED_BREAK_TICK) &&
			0u == room.m_iPillarAuditionBreakTick;
		tests.Require(
			activated && scheduledAcrossWrap &&
			keptBeforeWrap && keptBeforeDeadline && committedAtDeadline,
			"Pillar audition deadline skips reserved zero and commits after tick wrap");
	}

	{
		/* Automatic health interrupts are suppressed by the authored sequence.
		Stage the same explicit health-bar audition override the Debug command owns,
		then let the real tick loop queue, select and run the stele mechanic. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID SESSION = 4319u;
		constexpr PLAYER_ID PLAYER = 97u;
		constexpr std::uint32_t PILLAR_TRIGGER_BAR = 100u;
		constexpr std::uint32_t MAX_PILLAR_TICKS = 3000u;
		const bool activated = room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* const spawnedBoss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 915u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		/* The boss kills an ordinary body long before the 100-bar mechanic
		comes round, and a dead player is filtered out of targeting, which
		parks the boss and strands the queue. The fixture keeps one body
		alive so the product selection path is what gets measured. */
		player.iCurrentHp = 100000000u;
		player.iMaximumHp = 100000000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		if (nullptr != spawnedBoss)
		{
			player.fPositionX = spawnedBoss->fPositionX + 2.f;
			player.fPositionY = spawnedBoss->fPositionY;
			player.fPositionZ = spawnedBoss->fPositionZ;
		}
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		/* One hit puts the boss on the boundary the mechanic watches for. */
		if (nullptr != spawnedBoss)
		{
			spawnedBoss->bAutomaticPatternSequenceAuditionOverride = true;
			spawnedBoss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(
				*spawnedBoss, PILLAR_TRIGGER_BAR);
		}

		bool sawPillarPattern = false;
		bool sawRecoveryStage = false;
		bool raised = false;
		for (std::uint32_t tick = 0u;
			tick < MAX_PILLAR_TICKS && !raised; ++tick)
		{
			room.Tick(1.f / 30.f);
			/* Top the body back up so it never leaves the target set. */
			SERVER_PLAYER& live = room.m_Players.at(PLAYER);
			live.iCurrentHp = live.iMaximumHp;
			const SERVER_WORLD_ENTITY* const boss = room.Find_AuditionBoss();
			if (nullptr != boss &&
				"VALTAN_FOUR_PILLARS_105" == boss->strPatternId)
			{
				sawPillarPattern = true;
				sawRecoveryStage = sawRecoveryStage ||
					"RECOVERY" == boss->strPatternStageId;
			}
			const auto& slots =
				room.m_EncounterPropRuntime.Get_SlotStates();
			raised = 4u == slots.size() &&
				std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					});
		}
		tests.Require(
			activated && nullptr != spawnedBoss && sawPillarPattern &&
			sawRecoveryStage && raised,
			"Raise the four stele through an explicit 100-bar audition and the product tick loop");
	}
	{
		/* The 100-bar cinematic is still Server movement even while the Client
		camera looks away. Drive the product room path so clip root motion cannot
		quietly add a second planar step after the leap arc has written its pose. */
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID SESSION = 4320u;
		constexpr PLAYER_ID PLAYER = 197u;
		constexpr std::uint32_t MAX_CINEMATIC_TICKS = 450u;
		const bool activated = room.Is_Ready() &&
			room.Activate_Encounter("boss.valtan.center");
		SERVER_WORLD_ENTITY* boss = room.Find_AuditionBoss();

		SERVER_PLAYER player{};
		player.iSessionId = SESSION;
		player.iPlayerId = PLAYER;
		player.iNetEntityId = 1915u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.iCurrentHp = 100000000u;
		player.iMaximumHp = 100000000u;
		player.iCurrentResource = 100u;
		player.iMaximumResource = 100u;
		player.isCombatReady = true;
		player.strSpawnPlacementId = "player_1";
		const float originX = nullptr == boss ? 0.f : boss->fPositionX;
		const float originY = nullptr == boss ? 0.f : boss->fPositionY;
		const float originZ = nullptr == boss ? 0.f : boss->fPositionZ;
		player.fPositionX = originX + 2.f;
		player.fPositionY = originY;
		player.fPositionZ = originZ;
		const float lockedLandingX = player.fPositionX;
		const float lockedLandingY = player.fPositionY;
		const float lockedLandingZ = player.fPositionZ;
		room.m_Players.emplace(PLAYER, player);
		room.m_PlayerIdBySessionId.emplace(SESSION, PLAYER);

		const BOSS_PATTERN_DEFINITION* cinematicPattern = nullptr;
		if (const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			room.m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN"))
		{
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& candidate)
				{
					return "VALTAN_FOUR_PILLARS_105" ==
						candidate.strPatternId;
				});
			if (patterns->end() != found)
				cinematicPattern = &(*found);
		}
		const bool authoredTimeline = nullptr != cinematicPattern &&
			100u == cinematicPattern->iTriggerHealthBar &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET ==
				cinematicPattern->Motion.eKind &&
			"anchor.valtan.four-pillars-105.landing" ==
				cinematicPattern->Motion.strAnchorId &&
			std::abs(cinematicPattern->Motion.fApexHeight - 46.f) < 0.001f &&
			4u == cinematicPattern->Stages.size() &&
			1933u == cinematicPattern->Stages[0].iDurationMs &&
			5500u == cinematicPattern->Stages[1].iDurationMs &&
			3200u == cinematicPattern->Stages[2].iDurationMs &&
			230u == cinematicPattern->Stages[2].iHitDelayMs &&
			1300u == cinematicPattern->Stages[3].iDurationMs;

		if (nullptr != boss)
		{
			boss->bIntroPatternConsumed = true;
			boss->bAutomaticPatternSequenceAuditionOverride = true;
			boss->iLastEvaluatedHealthBar = 101u;
			boss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(*boss, 100u);
		}
		room.Tick(1.f / 30.f);
		boss = room.Find_AuditionBoss();
		const bool lockedAtStart = nullptr != boss &&
			"VALTAN_FOUR_PILLARS_105" == boss->strPatternId &&
			"TAKEOFF" == boss->strPatternStageId &&
			player.iNetEntityId == boss->iPatternTargetEntityId &&
			std::abs(boss->fLeapLandingX - lockedLandingX) < 0.001f &&
			std::abs(boss->fLeapLandingY - lockedLandingY) < 0.001f &&
			std::abs(boss->fLeapLandingZ - lockedLandingZ) < 0.001f;
		/* Moving the player after the lock must not move the landing snapshot. */
		SERVER_PLAYER& moved = room.m_Players.at(PLAYER);
		moved.fPositionX = originX - 2.f;
		moved.fPositionZ = originZ;

		bool stayedVertical = true;
		bool sawLift = false;
		bool sawTravel = false;
		bool sawDescent = false;
		bool landedExactly = false;
		bool sawRecovery = false;
		bool raised = false;
		float peakY = originY;
		for (std::uint32_t tick = 0u;
			tick < MAX_CINEMATIC_TICKS && !raised; ++tick)
		{
			room.Tick(1.f / 30.f);
			SERVER_PLAYER& live = room.m_Players.at(PLAYER);
			live.iCurrentHp = live.iMaximumHp;
			boss = room.Find_AuditionBoss();
			if (nullptr == boss ||
				"VALTAN_FOUR_PILLARS_105" != boss->strPatternId)
			{
				continue;
			}
			peakY = (std::max)(peakY, boss->fPositionY);
			if ("TAKEOFF" == boss->strPatternStageId)
			{
				stayedVertical = stayedVertical &&
					std::abs(boss->fPositionX - originX) < 0.001f &&
					std::abs(boss->fPositionZ - originZ) < 0.001f;
				sawLift = sawLift || boss->fPositionY > originY + 5.f;
			}
			else if ("YELLOW_ZONE" == boss->strPatternStageId)
			{
				const float travelX = boss->fPositionX - originX;
				const float travelZ = boss->fPositionZ - originZ;
				sawTravel = sawTravel ||
					travelX * travelX + travelZ * travelZ > 0.25f;
				sawDescent = sawDescent || boss->fPositionY < peakY - 5.f;
			}
			else if ("TARGET_CONE" == boss->strPatternStageId)
			{
				landedExactly = landedExactly ||
					(std::abs(boss->fPositionX - lockedLandingX) < 0.001f &&
					 std::abs(boss->fPositionY - lockedLandingY) < 0.001f &&
					 std::abs(boss->fPositionZ - lockedLandingZ) < 0.001f &&
					 player.iNetEntityId == boss->iPatternTargetEntityId);
			}
			else if ("RECOVERY" == boss->strPatternStageId)
			{
				sawRecovery = true;
			}
			const auto& slots = room.m_EncounterPropRuntime.Get_SlotStates();
			raised = 4u == slots.size() &&
				std::all_of(slots.begin(), slots.end(),
					[](const ENCOUNTER_PROP_SLOT_STATE& slot)
					{
						return ENCOUNTER_PROP_STATE::INTACT == slot.eState;
					});
		}
		tests.Require(
			activated && authoredTimeline && lockedAtStart && stayedVertical &&
			sawLift && peakY > originY + 44.f && sawTravel && sawDescent &&
			landedExactly && sawRecovery && raised,
			"Run the auditioned 100-bar cutscene as a vertical leap, locked-target landing and four-stele recovery on the product room path");
	}


#endif
}

