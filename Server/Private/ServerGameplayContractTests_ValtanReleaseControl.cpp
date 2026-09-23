#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "ClientSession.h"
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

int LostArk::Server::CServerGameplayContractRunner::Run_ValtanPatternControl()
{
	TESTS tests;
	Run_ValtanReleaseControl(tests);
	std::cout << "Valtan pattern control failures=" << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}

void LostArk::Server::CServerGameplayContractRunner::Run_ValtanReleaseControl(TESTS& tests)
{
	auto storage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
	auto& room = *storage;
	constexpr SESSION_ID owner = 92401u;
	constexpr PLAYER_ID playerId = 92402u;
	const bool ready = room.Is_Ready() && room.Activate_Encounter("boss.valtan.center");
	auto* boss = room.Find_AuditionBoss();
	tests.Require(ready && boss, "Load the actual Valtan room and disabled boss for common pattern controls");
	if (!ready || !boss) return;
	const auto session = std::make_shared<CClientSession>(owner, INVALID_SOCKET,
		CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
	session->m_isSendRunning.store(true);
	room.m_Sessions.emplace(owner, session);
	SERVER_PLAYER player{};
	player.iSessionId = owner; player.iPlayerId = playerId; player.iNetEntityId = 92403u;
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.iCurrentHp = player.iMaximumHp = 1000000000u;
	player.fPositionX = boss->fPositionX + 2.f;
	player.fPositionY = boss->fPositionY; player.fPositionZ = boss->fPositionZ;
	room.m_Players.emplace(playerId, player);
	room.m_PlayerIdBySessionId.emplace(owner, playerId);
	room.m_PlayerIdByEntityId.emplace(player.iNetEntityId, playerId);
	const auto revision = room.m_GameplayCatalog.Get_ActiveRevision();
	C2S_VALTAN_AUDITION_REQUEST play{};
	play.iRequestSequence = 1u; play.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
	play.strBossPlacementId = "boss.valtan.center"; play.strPatternId = "VALTAN_FIST_IN_OUT";
	play.ExpectedDefinitionRevision = revision;
	std::uint32_t bar = 0u;
	const auto hpBefore = boss->iCurrentHp;
	tests.Require(room.Evaluate_ValtanAudition(owner + 1u, play, bar) == VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD &&
		boss->iCurrentHp == hpBefore && boss->PendingPatternIds.empty(),
		"Common Valtan play rejects a session outside the room before mutation");
	auto stale = play; stale.ExpectedDefinitionRevision.Bytes[0] ^= 0x5au;
	tests.Require(room.Evaluate_ValtanAudition(owner, stale, bar) == VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST &&
		boss->iCurrentHp == hpBefore && boss->PendingPatternIds.empty(),
		"Common Valtan play preserves the boss when the gameplay revision is stale");
	play.iRequestSequence = 2u;
	const auto playVerdict = room.Evaluate_ValtanAudition(owner, play, bar);
	room.Tick(1.f / 30.f);
	boss = room.Find_AuditionBoss();
	tests.Require(playVerdict == VALTAN_AUDITION_RESULT::QUEUED && room.Is_Ready() && boss &&
		boss->strPatternId == play.strPatternId && room.m_ValtanPatternIdAudition.ePhase == CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE &&
		!session->m_OutboundFrames.empty(),
		"Complete Play starts the admitted pattern on the fixed tick and emits its lifecycle in both builds");
	for (unsigned tick = 0u; tick < 900u && room.Is_ValtanPatternIdAuditionRunning(); ++tick)
		room.Tick(1.f / 30.f);
	tests.Require(room.Is_Ready() && room.m_ValtanPatternIdAudition.ePhase == CGameRoom::VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD,
		"Complete Play reaches its authoritative terminal receipt and holds instead of resuming ordinary AI");

	C2S_DEBUG_VALTAN_PATTERN_FLOW_START flow{};
	flow.iRequestSequence = 1u; flow.ExpectedDefinitionRevision = revision;
	flow.strBossPlacementId = "boss.valtan.center"; flow.strFlowId = "flow.valtan.release-contract";
	flow.strFlowRevision = std::string(64u, 'a'); flow.strStartSlotId = "flow.slot.000001";
	flow.iInterStepPursuitMs = MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS;
	flow.Slots = {{"flow.slot.000001", "VALTAN_FIST_IN_OUT"}};
	std::uint32_t epoch = 0u; GameplayDataRevision pinned{}; std::string reason;
	const auto flowVerdict = room.Evaluate_ValtanPatternFlowStart(owner, flow, epoch, pinned, reason);
	tests.Require(flowVerdict == VALTAN_PATTERN_FLOW_RESULT::QUEUED && epoch != 0u && pinned == revision,
		"Saved typed pattern flow is admitted with the same owner and pinned gameplay revision in both builds");
	C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT stop{};
	stop.iControlSequence = 1u; stop.strFlowId = flow.strFlowId; stop.iRoomFlowEpoch = epoch;
	std::uint32_t stopEpoch = 0u; GameplayDataRevision stopRevision{};
	const auto stopVerdict = room.Evaluate_ValtanPatternFlowStopAfterCurrent(owner, stop, stopEpoch, stopRevision, reason);
	for (unsigned tick = 0u; tick < 900u && room.Is_ValtanPatternFlowRunning(); ++tick)
		room.Tick(1.f / 30.f);
	boss = room.Find_AuditionBoss();
	const auto receipt = room.m_ValtanPatternFlowStartSequenceBySessionId.find(owner);
	tests.Require(stopVerdict == VALTAN_PATTERN_FLOW_RESULT::QUEUED && stopEpoch == epoch && stopRevision == revision &&
		room.Is_Ready() && !room.Is_ValtanPatternFlowRunning() && boss && boss->bAutomaticPatternSequenceAuditionHold &&
		receipt != room.m_ValtanPatternFlowStartSequenceBySessionId.end() && receipt->second.LastLifecycle &&
		receipt->second.LastLifecycle->eState == VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD,
		"Flow stop completes the current pattern and reports the same terminal hold in both builds");
	auto legacy = play; legacy.iRequestSequence = 3u; legacy.eOperation = VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
	tests.Require(room.Evaluate_ValtanAudition(owner, legacy, bar) == VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE,
		"Retired revision-unaware health-bar controls remain rejected");
#ifdef NDEBUG
	room.m_eWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
	tests.Require(room.Evaluate_ValtanAudition(owner, play, bar) == VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD,
		"Release does not enable the unrelated Character Select Valtan audition");
	room.m_eWorldId = WORLD_ID::VALTAN_ARENA;
#endif
}
