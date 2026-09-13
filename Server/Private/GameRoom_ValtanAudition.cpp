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

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_ValtanPatternFlowLifecycle(
	const LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
	const SERVER_WORLD_ENTITY* boss,
	std::string reason)
{
	using namespace LostArk::Shared;
	VALTAN_PATTERN_FLOW_AUDITION_STATE& flow =
		m_ValtanPatternFlowAudition;
	if (!Is_ValtanPatternFlowRunning() ||
		INVALID_SESSION_ID == flow.iOwnerSessionId || flow.Slots.empty() ||
		flow.Sequence.PatternIds.empty())
	{
		return;
	}

	const bool hasReportedOccurrence = 0u != flow.iReportedPatternSequence &&
		flow.iReportedSequenceIndex < flow.Sequence.PatternIds.size();
	const bool isTerminal =
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD == state ||
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD == state ||
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED == state ||
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED == state;
	const bool hasCurrentBoss = nullptr != boss && boss->iNetEntityId == flow.iBossEntityId;
	std::size_t sequenceIndex = hasReportedOccurrence ? flow.iReportedSequenceIndex : 0u;
	if (hasCurrentBoss && !(isTerminal && hasReportedOccurrence))
	{
		sequenceIndex = static_cast<std::size_t>(boss->iRotationStepIndex);
	}
	if (sequenceIndex >= flow.Sequence.PatternIds.size())
		sequenceIndex = flow.Sequence.PatternIds.size() - 1u;
	const std::size_t slotIndex = flow.iStartSlotIndex + sequenceIndex;
	if (slotIndex >= flow.Slots.size())
		return;

	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE message{};
	message.iRequestSequence = flow.iRequestSequence;
	message.iRoomFlowEpoch = flow.iRoomFlowEpoch;
	const std::uint32_t observedSequence = hasCurrentBoss ? boss->iPatternSequence :
			flow.iReportedPatternSequence;
	message.iPatternSequence = isTerminal && hasReportedOccurrence ?
		flow.iReportedPatternSequence :
		(!hasReportedOccurrence ?
			Add_ServerTicksSkippingReservedZero(flow.iFirstPatternSequence,
				static_cast<std::uint32_t>(sequenceIndex)) :
			(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING == state ?
				Add_ServerTicksSkippingReservedZero(observedSequence, 1u) : observedSequence));
	message.strBossPlacementId = flow.strBossPlacementId;
	message.strFlowId = flow.strFlowId;
	message.strFlowRevision = flow.strFlowRevision;
	message.strStartSlotId = flow.strStartSlotId;
	message.strCurrentSlotId = flow.Slots[slotIndex].strSlotId;
	message.strCurrentPatternId = flow.Slots[slotIndex].strPatternId;
	message.iCurrentSlotOrdinal =
		static_cast<std::uint16_t>(slotIndex + 1u);
	message.iSlotCount = static_cast<std::uint16_t>(flow.Slots.size());
	message.eState = state;
	message.PinnedDefinitionRevision = flow.PinnedDefinitionRevision;
	message.strReason = std::move(reason);
	/* PENDING is an emitted occurrence too. Preserve the same slot/sequence
	   pair when Stop arrives before it starts or the authoritative boss disappears. */
	flow.iReportedSequenceIndex = sequenceIndex;
	flow.iReportedPatternSequence = message.iPatternSequence;
	const auto receipt = m_ValtanPatternFlowStartSequenceBySessionId.find(
		flow.iOwnerSessionId);
	if (m_ValtanPatternFlowStartSequenceBySessionId.end() != receipt &&
		receipt->second.iSequence == flow.iRequestSequence &&
		receipt->second.iRoomFlowEpoch == flow.iRoomFlowEpoch &&
		receipt->second.strFlowId == flow.strFlowId &&
		receipt->second.strFlowRevision == flow.strFlowRevision &&
		receipt->second.PinnedDefinitionRevision == flow.PinnedDefinitionRevision)
	{
		receipt->second.LastLifecycle = message;
	}
	m_PendingValtanPatternFlowLifecycle.push_back({
		flow.iOwnerSessionId, std::move(message) });
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Flush_ValtanPatternFlowLifecycle()
{
	using namespace LostArk::Shared;
	auto pending = std::move(m_PendingValtanPatternFlowLifecycle);
	m_PendingValtanPatternFlowLifecycle.clear();
	for (const TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE& targeted : pending)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(targeted.iSessionId);
		if (nullptr == session)
			continue;
		CPacketWriter writer;
		if (!Write_Message(writer, targeted.Message))
			return false;
		if (!session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE,
			writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Finish_ValtanPatternFlow(
	SERVER_WORLD_ENTITY& boss,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
	std::string reason)
{
	if (!Is_ValtanPatternFlowRunning() ||
		boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId)
	{
		return;
	}
	if (LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED !=
		terminalState &&
		LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED !=
		terminalState)
	{
		reason.clear();
	}
	Queue_ValtanPatternFlowLifecycle(
		terminalState, &boss, std::move(reason));
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = true;
	boss.bAutomaticPatternSequencePausedForRevive = false;
	boss.iAutomaticPatternSequencePauseLastTick = 0u;
	boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
	boss.MovePath.clear();
	boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
	m_ValtanPatternFlowAudition = {};
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Refresh_ValtanPatternFlowState(
	SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	if (!Is_ValtanPatternFlowRunning() ||
		boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId)
	{
		return;
	}
	VALTAN_PATTERN_FLOW_AUDITION_STATE& flow =
		m_ValtanPatternFlowAudition;
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		Finish_ValtanPatternFlow(
			boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
			"Valtan pattern-flow boss became unavailable");
		return;
	}
	if (boss.PendingPatternFollowup.Is_Pending() ||
		(boss.iPatternFollowupDepth > 0u && !boss.strPatternId.empty()))
	{
		/* The source already advanced the saved-flow cursor. Its targetless
		outcome children remain part of that same slot, so stop/final completion
		waits until the leaf commits. */
		return;
	}
	if (boss.iPatternFollowupDepth > 0u)
	{
		if (SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED !=
				boss.PatternTerminalReceipt.eResult ||
			boss.PatternTerminalReceipt.iRootPatternSequence !=
				boss.iPatternFollowupRootSequence ||
			boss.PinnedDefinitionRevision != flow.PinnedDefinitionRevision)
		{
			Finish_ValtanPatternFlow(
				boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
				"Valtan pattern flow outcome follow-up did not complete");
			return;
		}
		boss.iPatternFollowupDepth = 0u;
		boss.iPatternFollowupRootSequence = 0u;
	}

	const std::size_t sequenceIndex =
		static_cast<std::size_t>(boss.iRotationStepIndex);
	if (flow.bStopAfterCurrent &&
		!boss.bAutomaticPatternSequenceStepRunning && boss.strPatternId.empty())
	{
		Finish_ValtanPatternFlow(
			boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD,
			"Valtan pattern flow stopped after the current slot");
		return;
	}
	if (sequenceIndex >= flow.Sequence.PatternIds.size() &&
		!boss.bAutomaticPatternSequenceStepRunning && boss.strPatternId.empty())
	{
		Finish_ValtanPatternFlow(
			boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD,
			"Valtan pattern flow completed");
		return;
	}
	if (sequenceIndex >= flow.Sequence.PatternIds.size() ||
		boss.strRotationId != flow.Sequence.strSequenceId)
	{
		Finish_ValtanPatternFlow(
			boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
			"Valtan pattern flow lost its ordered runtime identity");
		return;
	}

	if (boss.bAutomaticPatternSequenceStepRunning)
	{
		if (boss.strPatternId != flow.Sequence.PatternIds[sequenceIndex])
		{
			Finish_ValtanPatternFlow(
				boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
				"Valtan pattern flow observed an unexpected pattern");
			return;
		}
		const bool paused = boss.bAutomaticPatternSequencePausedForRevive;
		if (VALTAN_PATTERN_FLOW_AUDITION_PHASE::ACTIVE != flow.ePhase ||
			flow.iReportedSequenceIndex != sequenceIndex ||
			flow.iReportedPatternSequence != boss.iPatternSequence ||
			flow.bReportedPausedForRevive != paused)
		{
			flow.ePhase = VALTAN_PATTERN_FLOW_AUDITION_PHASE::ACTIVE;
			flow.iReportedSequenceIndex = sequenceIndex;
			flow.iReportedPatternSequence = boss.iPatternSequence;
			flow.bReportedPausedForRevive = paused;
			Queue_ValtanPatternFlowLifecycle(
				paused ?
					VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE :
					VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE,
				&boss);
		}
		return;
	}

	if (VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING != flow.ePhase ||
		flow.iReportedSequenceIndex != sequenceIndex)
	{
		flow.ePhase = VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING;
		flow.iReportedSequenceIndex = sequenceIndex;
		flow.bReportedPausedForRevive = false;
		Queue_ValtanPatternFlowLifecycle(
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, &boss);
	}
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Abort_ValtanPatternFlowForOwner(
	const SESSION_ID sessionId,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (!Is_ValtanPatternFlowRunning() ||
		sessionId != m_ValtanPatternFlowAudition.iOwnerSessionId)
	{
		return;
	}
	SERVER_WORLD_ENTITY* boss = Find_AuditionBoss(
		m_ValtanPatternFlowAudition.strBossPlacementId);
	Queue_ValtanPatternFlowLifecycle(
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED, boss, std::move(reason));
	if (nullptr != boss)
	{
		std::string resetStatus;
		const std::uint32_t resetTick =
			0u == m_iServerTick ? 1u : m_iServerTick;
		if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			m_strStatus = std::move(resetStatus);
	}
	m_ValtanPatternFlowAudition = {};
}
#endif

bool LostArk::Server::CGameRoom::Build_ValtanBossOnlyAuditionReset(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t resetTick,
	SERVER_WORLD_ENTITY& outBoss,
	std::string& status)
{
	using LostArk::Shared::WORLD_ID;
	if ((WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		 WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId) ||
		0u == resetTick || WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
		"BOSS_VALTAN" != boss.strArchetypeId ||
		"ENCOUNTER_VALTAN" != boss.strEncounterId)
	{
		status = "Valtan boss-only audition reset is unavailable";
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(boss.strPlacementId);
	if (nullptr == placement || placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		placement->strArchetypeId != boss.strArchetypeId ||
		placement->strEncounterId != boss.strEncounterId)
	{
		status = "Valtan audition boss placement is unavailable";
		return false;
	}

	SERVER_WORLD_ENTITY stagedBoss{};
	if (!Build_WorldEntity(*placement, boss.iNetEntityId, stagedBoss))
	{
		status = m_strStatus;
		return false;
	}
	/* Both fields are Client edge identities. A reset creates fresh state but
	   does not rewind either stream, so the pattern queued after this reset and
	   the reset combat frame remain observable after any earlier audition. */
	stagedBoss.iPatternSequence = boss.iPatternSequence;
	if (stagedBoss.BossCombat.iStateRevision <=
		boss.BossCombat.iStateRevision)
	{
		stagedBoss.BossCombat.iStateRevision =
			(std::numeric_limits<std::uint32_t>::max)() ==
				boss.BossCombat.iStateRevision ?
			1u : boss.BossCombat.iStateRevision + 1u;
	}
	stagedBoss.bIntroPatternConsumed = true;
	outBoss = std::move(stagedBoss);
	status = "Valtan boss-only audition reset staged";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanBossOnlyAuditionState(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t resetTick,
	std::string& status)
{
	SERVER_WORLD_ENTITY stagedBoss{};
	if (!Build_ValtanBossOnlyAuditionReset(
			boss, resetTick, stagedBoss, status))
	{
		return false;
	}

	#ifdef _DEBUG
	Queue_ValtanPatternFlowLifecycle(
		LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
		&boss, "Flow replaced by an authoritative audition restart");
	#endif
	Clear_ValtanGhostRelocationState(boss);
	boss = std::move(stagedBoss);
	/* A boss-only restart preserves the arena and props, but no Debug pillar
	   reservation from the replaced occurrence may fire during the new one. */
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
	(void)Release_PlayerAttachments(
		boss.iNetEntityId, 0.f, 0u, false, 0u, resetTick);
	/* Complete Play preserves Valtan Arena wall, floor, prop, collision and Nav
	   state. Cancel only objects emitted by this boss; player-owned combat
	   objects belong to a different source and survive the audition reset. */
	m_CombatObjectRuntime.Cancel_Source(boss.iNetEntityId);
	m_TickBossCombatEvents.clear();
	m_iValtanAuditionArmedHealthBar = 0u;
#ifdef _DEBUG
	Cancel_ValtanPatternIdAudition("authoritative audition reset");
	// Keep session receipts so an epoch-zero live request cannot replay after reset.
	m_ValtanPatternFlowAudition = {};
	m_ValtanTimelineAudition = {};
	m_ValtanFightPageStart = {};
#endif
	status = "Valtan boss-only audition reset completed";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanAuditionState(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t resetTick,
	std::string& status)
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		0u == resetTick || !m_WorldDestructionRuntime.Is_Initialized())
	{
		status = "Valtan audition reset is unavailable";
		return false;
	}
	SERVER_WORLD_ENTITY stagedBoss{};
	if (!Build_ValtanBossOnlyAuditionReset(
			boss, resetTick, stagedBoss, status))
	{
		return false;
	}

	CWorldDestructionRuntime stagedDestruction = m_WorldDestructionRuntime;
	if (!stagedDestruction.Reset(status, resetTick))
		return false;
	CEncounterPropRuntime stagedProps = m_EncounterPropRuntime;
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!stagedProps.Reset(status, resetTick))
	{
		return false;
	}

	#ifdef _DEBUG
	Queue_ValtanPatternFlowLifecycle(
		LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
		&boss, "Flow replaced by an authoritative audition restart");
	#endif
	Clear_ValtanGhostRelocationState(boss);
	boss = std::move(stagedBoss);
	m_WorldDestructionRuntime = std::move(stagedDestruction);
	m_EncounterPropRuntime = std::move(stagedProps);
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iNextBossCombatEventSequence = 1u;
	m_TickBossCombatEvents.clear();
	/* A debug reset is also an encounter-owner reset. Release every attachment
	before the reset boss can move the old local offset to an unrelated pose. */
	(void)Release_PlayerAttachments(
		boss.iNetEntityId, 0.f, 0u, false, 0u, resetTick);
	/* All preflight work above succeeded. Combat objects belong to the same
	encounter epoch, so reset them only at this commit edge and keep reliable
	despawns for the players who are still observing the audition. */
	m_CombatObjectRuntime.Reset();
	m_iValtanAuditionArmedHealthBar = 0u;
#ifdef _DEBUG
	Cancel_ValtanPatternIdAudition("authoritative audition reset");
	// Keep session receipts so an epoch-zero live request cannot replay after reset.
	m_ValtanPatternFlowAudition = {};
	m_ValtanTimelineAudition = {};
	m_ValtanFightPageStart = {};
#endif
	/* The reset put every floor sector back, so a body that was still falling
	has solid ground under its own XZ again. */
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction)
			continue;
		player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0u;
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;
		player.PendingCommand.Clear();
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_Loaded() &&
			m_ServerNavigation.Project_Point(
				player.fPositionX, player.fPositionZ, ground))
		{
			player.fPositionX = ground.x;
			player.fPositionY = ground.y;
			player.fPositionZ = ground.z;
		}
	}
	Invalidate_DynamicNavigationPaths();

	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldDestructionFullSync(session))
			session->Request_Close();
	}
	Broadcast_EncounterPropSync();
	status = "Valtan audition reset and full-sync completed";
	return true;
}

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Prepare_ValtanTimelineArenaState(
	const CWorldDestructionRuntime& runtime,
	const SERVER_WORLD_ENTITY& boss,
	const VALTAN_TIMELINE_ARENA_STATE arenaState,
	const std::uint32_t requestTick,
	WORLD_DESTRUCTION_TRANSACTION& outTransaction,
	std::vector<std::string>& outExpectedGoneGroupIds,
	std::string& status) const
{
	outTransaction = {};
	outExpectedGoneGroupIds.clear();
	if (!runtime.Is_Initialized() || 0u == requestTick)
	{
		status = "Valtan timeline destruction runtime is unavailable";
		return false;
	}
	if (VALTAN_TIMELINE_ARENA_STATE::FRESH == arenaState)
	{
		status = "Valtan timeline fresh arena staged";
		return true;
	}

	bool includeOuter = false;
	bool includeFloor84 = false;
	bool includeFloor30 = false;
	switch (arenaState)
	{
	case VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE:
		break;
	case VALTAN_TIMELINE_ARENA_STATE::ALL_WALLS_GONE:
		includeOuter = true;
		break;
	case VALTAN_TIMELINE_ARENA_STATE::FLOOR84_GONE:
		includeOuter = true;
		includeFloor84 = true;
		break;
	case VALTAN_TIMELINE_ARENA_STATE::FLOOR30_GONE:
		includeOuter = true;
		includeFloor30 = true;
		break;
	case VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE:
		includeOuter = true;
		includeFloor84 = true;
		includeFloor30 = true;
		break;
	default:
		status = "Valtan timeline arena state is invalid";
		return false;
	}

	struct STAGED_TRANSITION final
	{
		WORLD_DESTRUCTION_BINDING_APPLICATION Application;
		WORLD_DESTRUCTION_STATE_TRANSITION Transition;
	};
	std::vector<STAGED_TRANSITION> staged;
	std::map<std::string, std::size_t> stagedIndexByGroupId;
	const auto append =
		[&](const WORLD_DESTRUCTION_TRANSACTION& transaction) -> bool
		{
			return Append_UniqueDestructionTransitions(
				transaction, runtime.Get_EncounterEpoch(), requestTick,
				staged, stagedIndexByGroupId);
		};
	std::uint32_t triggerSequence = boss.iPatternSequence;
	const auto nextSequence = [&triggerSequence]()
	{
		triggerSequence =
			(std::numeric_limits<std::uint32_t>::max)() == triggerSequence ?
			1u : triggerSequence + 1u;
		return triggerSequence;
	};

	const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& graph =
		m_WorldDestructionBootstrap.Get_DescriptorGraph();
	for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding : graph.Bindings)
	{
		if (WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT !=
			binding.eTriggerKind)
		{
			continue;
		}
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		const WORLD_DESTRUCTION_PREPARE_RESULT result =
			runtime.Prepare_ContactTrigger(
				binding.strImpactReceiverId, boss.iNetEntityId,
				nextSequence(), requestTick, transaction, status);
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
			!append(transaction))
		{
			status = "Valtan timeline ordinary-wall precondition failed: " + status;
			return false;
		}
	}

	const auto appendStage =
		[&](const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			const char* label) -> bool
		{
			WORLD_DESTRUCTION_TRANSACTION transaction{};
			const WORLD_DESTRUCTION_PREPARE_RESULT result =
				runtime.Prepare_StageTrigger(
					action, boss.iNetEntityId, nextSequence(), requestTick,
					transaction, status);
			if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == result &&
				append(transaction))
			{
				return true;
			}
			status = std::string("Valtan timeline ") + label +
				" precondition failed: " + status;
			return false;
		};
	if (includeOuter && !appendStage(
		WORLD_DESTRUCTION_ACTION_TUPLE{
			FINAL_ARENA_PATTERN_ID, FINAL_ARENA_STAGE_ID,
			FINAL_ARENA_ACTION_ID, 2u }, "109 outer-wall"))
	{
		return false;
	}
	if (includeFloor84 && !appendStage(
		WORLD_DESTRUCTION_ACTION_TUPLE{
			FINAL_ARENA_FLOOR_A_PATTERN_ID, FINAL_ARENA_FLOOR_A_STAGE_ID,
			FINAL_ARENA_FLOOR_A_ACTION_ID,
			FINAL_ARENA_FLOOR_A_STAGE_INDEX }, "84 floor"))
	{
		return false;
	}
	if (includeFloor30 && !appendStage(
		WORLD_DESTRUCTION_ACTION_TUPLE{
			FINAL_ARENA_FLOOR_B_PATTERN_ID, FINAL_ARENA_FLOOR_B_STAGE_ID,
			FINAL_ARENA_FLOOR_B_ACTION_ID,
			FINAL_ARENA_FLOOR_B_STAGE_INDEX }, "30 floor"))
	{
		return false;
	}
	if (staged.empty())
	{
		status = "Valtan timeline arena precondition selected no groups";
		return false;
	}

	std::sort(staged.begin(), staged.end(),
		[](const STAGED_TRANSITION& left, const STAGED_TRANSITION& right)
		{
			return left.Transition.strGroupId < right.Transition.strGroupId;
		});
	outTransaction.iEncounterEpoch = runtime.Get_EncounterEpoch();
	outTransaction.iRequestTick = requestTick;
	for (STAGED_TRANSITION& pair : staged)
	{
		outExpectedGoneGroupIds.push_back(pair.Transition.strGroupId);
		outTransaction.BindingApplications.push_back(std::move(pair.Application));
		outTransaction.Transitions.push_back(std::move(pair.Transition));
	}
	status = "Valtan timeline arena precondition staged";
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Stage_ValtanTimelineRowStart(
	const SESSION_ID sessionId,
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t commandId,
	const std::uint32_t startTick,
	SERVER_PLAYER& outOwner,
	std::string& status) const
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId || 0u == commandId ||
		0u == startTick ||
		!m_WorldDestructionRuntime.Is_Initialized())
	{
		status = "Valtan timeline reset is unavailable";
		return false;
	}

	const auto ownerId = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == ownerId)
	{
		status = "Valtan timeline owner is unavailable";
		return false;
	}
	const auto owner = m_Players.find(ownerId->second);
	if (m_Players.end() == owner)
	{
		status = "Valtan timeline player is unavailable";
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(boss.strPlacementId);
	if (nullptr == placement ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind)
	{
		status = "Valtan timeline boss placement is unavailable";
		return false;
	}

	const VALTAN_TIMELINE_ROW* row =
		m_GameplayCatalog.Find_ValtanTimelineRow(
			boss.strEncounterId, commandId);
	const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
		m_GameplayCatalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == row || nullptr == patterns)
	{
		status = "Valtan timeline row is unavailable";
		return false;
	}
	if (row->PatternActions.empty() ||
		row->iSectionHealthBar > boss.iMaximumHealthBars ||
		0u == CValtanBrain::Resolve_HealthBarHp(boss, row->iSectionHealthBar))
	{
		status = "Valtan timeline row contract is invalid";
		return false;
	}
	for (const VALTAN_TIMELINE_PATTERN_ACTION& action : row->PatternActions)
	{
		if (action.strPatternId.empty() || 0u == action.iRepeat ||
			action.iRepeat > 4u ||
			patterns->end() == std::find_if(
				patterns->begin(), patterns->end(),
				[&action](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return pattern.strPatternId == action.strPatternId;
				}))
		{
			status = "Valtan timeline pattern mapping is invalid";
			return false;
		}
	}

	CWorldDestructionRuntime stagedDestruction = m_WorldDestructionRuntime;
	if (!stagedDestruction.Reset(status, startTick))
		return false;
	WORLD_DESTRUCTION_TRANSACTION destructionTransaction{};
	std::vector<std::string> expectedGoneGroupIds;
	if (!Prepare_ValtanTimelineArenaState(
		stagedDestruction, boss, row->eArenaState, startTick,
		destructionTransaction, expectedGoneGroupIds, status) ||
		(!destructionTransaction.Transitions.empty() &&
		 !stagedDestruction.Commit(destructionTransaction, status)))
	{
		return false;
	}
	CEncounterPropRuntime stagedProps = m_EncounterPropRuntime;
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!stagedProps.Reset(status, startTick))
	{
		return false;
	}
	if (VALTAN_TIMELINE_PROP_STATE::FOUR_PILLARS_INTACT == row->ePropState)
	{
		if (!stagedProps.Is_Initialized())
		{
			status = "Valtan timeline pillar runtime is unavailable";
			return false;
		}
		ENCOUNTER_PROP_TRANSACTION propTransaction{};
		const std::uint32_t occurrenceSequence =
			(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence ?
			1u : boss.iPatternSequence + 1u;
		if (ENCOUNTER_PROP_PREPARE_RESULT::READY !=
				stagedProps.Prepare_Spawn(
					occurrenceSequence, startTick, propTransaction, status) ||
			!stagedProps.Commit(propTransaction, status))
		{
			return false;
		}
	}

	SERVER_PLAYER stagedOwner = owner->second;
	Prepare_TimelineAuditionPlayer(stagedOwner, startTick);
	if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
		stagedOwner, startTick))
	{
		status = "Valtan timeline bait placement is unavailable";
		return false;
	}
	outOwner = std::move(stagedOwner);
	status = "Valtan timeline row preflight completed";
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Start_ValtanTimelineRow(
	const SESSION_ID sessionId,
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t commandId,
	const std::uint32_t startTick,
	std::string& status)
{
	using namespace LostArk::Shared;
	SERVER_PLAYER stagedOwner{};
	if (!Stage_ValtanTimelineRowStart(
		sessionId, boss, commandId, startTick, stagedOwner, status))
	{
		return false;
	}
	const PLAYER_ID ownerPlayerId = stagedOwner.iPlayerId;
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
		m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}

	if (!Reset_ValtanAuditionState(boss, startTick, status))
		return false;
	const VALTAN_TIMELINE_ROW* selectedRow =
		m_GameplayCatalog.Find_ValtanTimelineRow(
			boss.strEncounterId, commandId);
	if (nullptr == selectedRow)
	{
		status = "Valtan timeline row disappeared during commit";
		return false;
	}
	const VALTAN_TIMELINE_ROW& row = *selectedRow;
	WORLD_DESTRUCTION_TRANSACTION destructionTransaction{};
	std::vector<std::string> expectedGoneGroupIds;
	if (!Prepare_ValtanTimelineArenaState(
		m_WorldDestructionRuntime, boss, row.eArenaState, startTick,
		destructionTransaction, expectedGoneGroupIds, status))
	{
		return false;
	}
	ENCOUNTER_PROP_TRANSACTION propTransaction{};
	const bool spawnPillars =
		VALTAN_TIMELINE_PROP_STATE::FOUR_PILLARS_INTACT == row.ePropState;
	if (spawnPillars)
	{
		const std::uint32_t occurrenceSequence =
			(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence ?
			1u : boss.iPatternSequence + 1u;
		if (!m_EncounterPropRuntime.Is_Initialized() ||
			ENCOUNTER_PROP_PREPARE_RESULT::READY !=
				m_EncounterPropRuntime.Prepare_Spawn(
					occurrenceSequence, startTick, propTransaction, status))
		{
			return false;
		}
	}
	if ((!destructionTransaction.Transitions.empty() &&
		 !Commit_WorldDestructionTransaction(
			destructionTransaction, {}, startTick, status)) ||
		(spawnPillars &&
		 !m_EncounterPropRuntime.Commit(propTransaction, status)))
	{
		const std::string failure = status;
		std::string rollbackStatus;
		(void)Reset_ValtanAuditionState(boss, startTick, rollbackStatus);
		status = "Valtan timeline precondition commit rolled back: " + failure;
		return false;
	}
	if (spawnPillars)
		Broadcast_EncounterPropSync();
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		Freeze_TimelineAuditionPlayer(player);
	}
	SERVER_PLAYER& refreshedOwner = m_Players.at(ownerPlayerId);
	refreshedOwner = std::move(stagedOwner);
	m_ServerTriggerSystem.Remove_Player(refreshedOwner.iPlayerId);

	boss.bIntroPatternConsumed = true;
	boss.bScriptedPatternPlayback = true;
	boss.PendingPatternIds.clear();
	const std::uint32_t targetHp =
		CValtanBrain::Resolve_HealthBarHp(boss, row.iSectionHealthBar);
	if (0u == targetHp)
	{
		status = "Valtan timeline health bar disappeared during commit";
		return false;
	}
	boss.iCurrentHp = targetHp;
	boss.iLastEvaluatedHealthBar = row.iSectionHealthBar;
	m_iValtanAuditionArmedHealthBar = 0u;
	m_ValtanTimelineAudition = {};
	m_ValtanTimelineAudition.ePhase = expectedGoneGroupIds.empty() ?
		VALTAN_TIMELINE_AUDITION_PHASE::READY :
		VALTAN_TIMELINE_AUDITION_PHASE::WAITING_ENVIRONMENT;
	m_ValtanTimelineAudition.iOwnerSessionId = sessionId;
	m_ValtanTimelineAudition.iOwnerPlayerId = refreshedOwner.iPlayerId;
	m_ValtanTimelineAudition.iBossEntityId = boss.iNetEntityId;
	m_ValtanTimelineAudition.iRowIndex = row.iOrdinal - 1u;
	m_ValtanTimelineAudition.iHeldBossHp = boss.iCurrentHp;
	m_ValtanTimelineAudition.iHeldBossHealthBar = row.iSectionHealthBar;
	m_ValtanTimelineAudition.PinnedDefinitionRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	m_ValtanTimelineAudition.iEnvironmentDeadlineTick =
		Add_ServerTicksSkippingReservedZero(startTick, 300u);
	m_ValtanTimelineAudition.bAllowProductPropBreak = spawnPillars;
	m_ValtanTimelineAudition.ExpectedGoneGroupIds =
		std::move(expectedGoneGroupIds);
	status = "Valtan timeline row " + std::to_string(row.iOrdinal) + " queued";
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Start_ValtanFightPage(
	const SESSION_ID sessionId,
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t commandId,
	const std::uint32_t startTick,
	std::string& status)
{
	using namespace LostArk::Shared;
	const VALTAN_TIMELINE_ROW* selectedRow =
		m_GameplayCatalog.Find_ValtanTimelineRow(
			boss.strEncounterId, commandId);
	const VALTAN_FIGHT_PAGE_POLICY* policy = nullptr == selectedRow ?
		nullptr : Find_ValtanFightPagePolicy(selectedRow->strRowId);
	if (nullptr == selectedRow || nullptr == policy ||
		VALTAN_TIMELINE_ENTRY_TYPE::MECHANIC != selectedRow->eEntryType ||
		VALTAN_TIMELINE_PROP_STATE::HIDDEN != selectedRow->ePropState)
	{
		status = "Valtan fight page command is not an authored page boundary";
		return false;
	}

	/* Reuse the row runner's complete parse/catalog/environment preflight. A
	valid page replaces an active isolated-row audition, but an invalid page
	must leave that existing audition untouched. */
	SERVER_PLAYER stagedOwner{};
	if (!Stage_ValtanTimelineRowStart(
		sessionId, boss, commandId, startTick, stagedOwner, status))
	{
		return false;
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
		m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}

	if (!Reset_ValtanAuditionState(boss, startTick, status))
		return false;
	selectedRow = m_GameplayCatalog.Find_ValtanTimelineRow(
		boss.strEncounterId, commandId);
	policy = nullptr == selectedRow ? nullptr :
		Find_ValtanFightPagePolicy(selectedRow->strRowId);
	if (nullptr == selectedRow || nullptr == policy)
	{
		status = "Valtan fight page disappeared during commit";
		return false;
	}
	const VALTAN_TIMELINE_ROW& row = *selectedRow;
	const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
		m_GameplayCatalog.Find_BossPatterns(boss.strEncounterId);
	const std::uint32_t targetHp =
		CValtanBrain::Resolve_HealthBarHp(boss, row.iSectionHealthBar);
	if (nullptr == patterns || 0u == targetHp)
	{
		status = "Valtan fight page mechanic ledger is unavailable";
		return false;
	}
	const std::size_t completedMechanicCount = static_cast<std::size_t>(
		std::count_if(
			patterns->begin(), patterns->end(),
			[&row](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection &&
					pattern.iTriggerHealthBar > row.iSectionHealthBar;
			}));
	if (completedMechanicCount > CValtanBrain::MAX_MECHANIC_OCCURRENCE_COUNT)
	{
		status = "Valtan fight page completed mechanic ledger overflow";
		return false;
	}

	WORLD_DESTRUCTION_TRANSACTION destructionTransaction{};
	std::vector<std::string> expectedGoneGroupIds;
	if (!Prepare_ValtanTimelineArenaState(
		m_WorldDestructionRuntime, boss, row.eArenaState, startTick,
		destructionTransaction, expectedGoneGroupIds, status))
	{
		return false;
	}

	/* A page start is a raid restart point, so every connected party member is
	revived and placed at the authored bait point. Stage all copies first to
	avoid a partial party reset if the map no longer provides that point. */
	std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers = m_Players;
	for (auto& [playerId, player] : stagedPlayers)
	{
		(void)playerId;
		Prepare_TimelineAuditionPlayer(player, startTick);
		if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
			player, startTick))
		{
			status = "Valtan fight page party placement is unavailable";
			return false;
		}
	}
	if (stagedPlayers.empty())
	{
		status = "Valtan fight page has no party player";
		return false;
	}

	if (!destructionTransaction.Transitions.empty() &&
		!Commit_WorldDestructionTransaction(
			destructionTransaction, {}, startTick, status))
	{
		const std::string failure = status;
		std::string rollbackStatus;
		(void)Reset_ValtanAuditionState(boss, startTick, rollbackStatus);
		status = "Valtan fight page environment commit rolled back: " + failure;
		return false;
	}
	m_Players = std::move(stagedPlayers);
	for (const auto& [playerId, player] : m_Players)
	{
		(void)player;
		m_ServerTriggerSystem.Remove_Player(playerId);
	}

	boss.iCurrentHp = targetHp;
	boss.iLastEvaluatedHealthBar =
		row.iSectionHealthBar >= boss.iMaximumHealthBars ?
			boss.iMaximumHealthBars : row.iSectionHealthBar + 1u;
	(void)CBossCombatRuntime::Set_GameplayPhase(
		boss, policy->iInitialGameplayPhase);
	boss.bIntroPatternConsumed = !policy->bPlayEntrance;
	/* A fight page deliberately starts at an authored entrance/health boundary,
	so let that one-shot queue run ahead of the Product ordered sequence. The
	Brain clears this override as soon as it consumes the intro or pending
	mechanic, then ordinary Product playback resumes. */
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = false;
	boss.PendingPatternIds.clear();
	boss.TriggeredPatternIds.clear();
	boss.MechanicOccurrences.clear();
	boss.bMechanicLedgerRequiresReset = false;
	boss.iLastHealthMechanicGenerationEpoch =
		m_GameplayCatalog.Get_ActiveGenerationEpoch();
	boss.strRotationId.clear();
	boss.iRotationStepIndex = 0u;
	boss.iConsecutivePatternUses = 0u;
	boss.PatternCooldowns.clear();

	/* Completed occurrences are the authoritative one-shot record. Merely
	populating TriggeredPatternIds is insufficient because current crossing
	logic consults the occurrence ledger. Do not add the selected boundary: the
	next Brain tick must cross and queue that mechanic itself. */
	const GameplayDataRevision revision =
		m_GameplayCatalog.Get_ActiveRevision();
	for (const BOSS_PATTERN_DEFINITION& pattern : *patterns)
	{
		if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection ||
			pattern.iTriggerHealthBar <= row.iSectionHealthBar)
		{
			continue;
		}
		SERVER_BOSS_MECHANIC_OCCURRENCE occurrence{};
		occurrence.strPatternId = pattern.strPatternId;
		occurrence.PinnedDefinitionRevision = revision;
		occurrence.eState = SERVER_BOSS_MECHANIC_STATE::COMPLETED;
		occurrence.iTriggerHealthBar = pattern.iTriggerHealthBar;
		occurrence.iQueuedTick = startTick;
		occurrence.iStartedTick = startTick;
		occurrence.iFinishedTick = startTick;
		boss.MechanicOccurrences.push_back(std::move(occurrence));
		boss.TriggeredPatternIds.push_back(pattern.strPatternId);
	}

	m_iValtanAuditionArmedHealthBar = 0u;
	m_ValtanFightPageStart = {};
	if (!expectedGoneGroupIds.empty())
	{
		boss.bScriptedPatternPlayback = true;
		m_ValtanFightPageStart.iBossEntityId = boss.iNetEntityId;
		m_ValtanFightPageStart.iCommandId = commandId;
		m_ValtanFightPageStart.iEnvironmentDeadlineTick =
			Add_ServerTicksSkippingReservedZero(startTick, 300u);
		m_ValtanFightPageStart.ExpectedGoneGroupIds =
			std::move(expectedGoneGroupIds);
	}
	else
	{
		boss.bScriptedPatternPlayback = false;
	}
	status = "Valtan fight page queued from " + row.strRowId;
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Prepare_ValtanFightPageBeforeBrain(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t updateTick)
{
	if (!m_ValtanFightPageStart.Is_Active())
		return true;
	if (boss.iNetEntityId != m_ValtanFightPageStart.iBossEntityId ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		m_strStatus = "Valtan fight page lost its boss before release";
		return false;
	}

	bool environmentReady = true;
	for (const std::string& groupId :
		m_ValtanFightPageStart.ExpectedGoneGroupIds)
	{
		WORLD_DESTRUCTION_GROUP_STATE state{};
		if (!m_WorldDestructionRuntime.Find_GroupState(groupId, state))
		{
			m_strStatus = "Valtan fight page lost a staged arena group";
			return false;
		}
		environmentReady = environmentReady &&
			WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
	}
	if (!environmentReady)
	{
		if (Has_ReachedServerTick(
			updateTick, m_ValtanFightPageStart.iEnvironmentDeadlineTick))
		{
			m_strStatus = "Valtan fight page arena precondition timed out";
		}
		return false;
	}

	boss.bScriptedPatternPlayback = false;
	const std::uint32_t commandId = m_ValtanFightPageStart.iCommandId;
	m_ValtanFightPageStart = {};
	m_strStatus = "Valtan fight page environment ready: " +
		std::to_string(commandId);
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Stop_ValtanTimelineRow(
	const bool resetEncounter)
{
	SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
	if (nullptr != boss &&
		boss->iNetEntityId == m_ValtanTimelineAudition.iBossEntityId)
	{
		boss->PendingPatternIds.clear();
		if (boss->strPatternId.empty())
		{
			boss->PendingPatternFollowup = {};
			boss->iPatternFollowupDepth = 0u;
			boss->iPatternFollowupRootSequence = 0u;
		}
		boss->bScriptedPatternPlayback = false;
	}
	if (const auto owner = m_Players.find(
		m_ValtanTimelineAudition.iOwnerPlayerId);
		m_Players.end() != owner)
	{
		Freeze_TimelineAuditionPlayer(owner->second);
	}
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
	m_ValtanTimelineAudition = {};
	if (resetEncounter && nullptr != boss)
	{
		std::string resetStatus;
		const std::uint32_t resetTick =
			0u == m_iServerTick ? 1u : m_iServerTick;
		if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
		{
			m_strStatus = "Valtan timeline stop reset failed: " + resetStatus;
			return false;
		}
	}
	m_strStatus = resetEncounter ?
		"Valtan timeline row stopped and reset" :
		"Valtan timeline row stopped";
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Prepare_ValtanTimelineRowBeforeBrain(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
		m_ValtanTimelineAudition.ePhase)
	{
		return true;
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD ==
			m_ValtanTimelineAudition.ePhase ||
		VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD ==
			m_ValtanTimelineAudition.ePhase)
	{
		return false;
	}

	const auto fail = [this, &boss](const char* reason)
	{
		boss.PendingPatternIds.clear();
		if (boss.strPatternId.empty())
		{
			boss.PendingPatternFollowup = {};
			boss.iPatternFollowupDepth = 0u;
			boss.iPatternFollowupRootSequence = 0u;
		}
		m_iPillarAuditionBreakTick = 0u;
		m_bPillarAuditionCycleArmed = false;
		if (const auto owner = m_Players.find(
			m_ValtanTimelineAudition.iOwnerPlayerId);
			m_Players.end() != owner)
		{
			owner->second.isCombatReady = false;
		}
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD;
		m_strStatus = reason;
	};
	if (boss.iNetEntityId != m_ValtanTimelineAudition.iBossEntityId ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		fail("Valtan timeline lost its boss");
		return false;
	}
	const CGameplayCatalog* timelineCatalog = m_GameplayCatalog.Resolve(
		m_ValtanTimelineAudition.PinnedDefinitionRevision);
	const VALTAN_TIMELINE_DEFINITION* definition = nullptr == timelineCatalog ?
		nullptr : timelineCatalog->Find_ValtanTimeline(boss.strEncounterId);
	if (nullptr == definition ||
		m_ValtanTimelineAudition.iRowIndex >= definition->Rows.size())
	{
		fail("Valtan timeline catalog disappeared");
		return false;
	}
	const VALTAN_TIMELINE_ROW& row =
		definition->Rows[m_ValtanTimelineAudition.iRowIndex];
	const auto owner = m_Players.find(m_ValtanTimelineAudition.iOwnerPlayerId);
	if (m_Players.end() == owner ||
		owner->second.iSessionId != m_ValtanTimelineAudition.iOwnerSessionId)
	{
		fail("Valtan timeline lost its driver player");
		return false;
	}
	const bool patternIsRunning =
		VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
			m_ValtanTimelineAudition.ePhase;
	for (auto& [playerId, player] : m_Players)
	{
		if (!patternIsRunning || playerId != owner->second.iPlayerId)
			Freeze_TimelineAuditionPlayer(player);
	}
	if (!patternIsRunning)
	{
		Prepare_TimelineAuditionPlayer(owner->second, updateTick);
		m_ServerTriggerSystem.Remove_Player(owner->second.iPlayerId);
		if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
			owner->second, updateTick))
		{
			fail("Valtan timeline could not restore its driver bait");
			return false;
		}
	}
	boss.iCurrentHp = m_ValtanTimelineAudition.iHeldBossHp;
	boss.iLastEvaluatedHealthBar =
		m_ValtanTimelineAudition.iHeldBossHealthBar;
	boss.bIntroPatternConsumed = true;
	boss.bScriptedPatternPlayback = true;

	for (;;)
	{
		switch (m_ValtanTimelineAudition.ePhase)
		{
		case VALTAN_TIMELINE_AUDITION_PHASE::WAITING_ENVIRONMENT:
		{
			bool environmentReady = true;
			for (const std::string& groupId :
				m_ValtanTimelineAudition.ExpectedGoneGroupIds)
			{
				WORLD_DESTRUCTION_GROUP_STATE state{};
				if (!m_WorldDestructionRuntime.Find_GroupState(groupId, state))
				{
					fail("Valtan timeline lost a staged arena group");
					return false;
				}
				environmentReady = environmentReady &&
					WORLD_DESTRUCTION_STATE::DESPAWNED == state.eState;
			}
			if (!environmentReady)
			{
				if (Has_ReachedServerTick(
					updateTick,
					m_ValtanTimelineAudition.iEnvironmentDeadlineTick))
				{
					fail("Valtan timeline arena precondition timed out");
				}
				return false;
			}
			m_ValtanTimelineAudition.ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::READY;
			continue;
		}
		case VALTAN_TIMELINE_AUDITION_PHASE::READY:
		{
			if (m_ValtanTimelineAudition.iActionIndex >=
				row.PatternActions.size())
			{
				owner->second.isCombatReady = false;
				m_iPillarAuditionBreakTick = 0u;
				m_bPillarAuditionCycleArmed = false;
				m_ValtanTimelineAudition.ePhase =
					VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD;
				m_strStatus = "Valtan timeline row " +
					std::to_string(row.iOrdinal) + " completed";
				return false;
			}
			if (SERVER_ENTITY_ACTION::IDLE != boss.eAction ||
				!boss.strPatternId.empty() || !boss.PendingPatternIds.empty())
			{
				fail("Valtan timeline found an unexpected boss action");
				return false;
			}
			const VALTAN_TIMELINE_PATTERN_ACTION& action =
				row.PatternActions[m_ValtanTimelineAudition.iActionIndex];
			m_ValtanTimelineAudition.strExpectedPatternId = action.strPatternId;
			m_ValtanTimelineAudition.iExpectedPatternSequence =
				(std::numeric_limits<std::uint32_t>::max)() ==
					boss.iPatternSequence ? 1u : boss.iPatternSequence + 1u;
			boss.PendingPatternIds.push_back(action.strPatternId);
			m_ValtanTimelineAudition.ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_START;
			m_bPillarAuditionCycleArmed = false;
			m_strStatus = "Valtan timeline row " +
				std::to_string(row.iOrdinal) + " running action " +
				std::to_string(m_ValtanTimelineAudition.iActionIndex + 1u);
			return true;
		}
		case VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_START:
			if (boss.iPatternSequence ==
					m_ValtanTimelineAudition.iExpectedPatternSequence &&
				boss.strPatternId ==
					m_ValtanTimelineAudition.strExpectedPatternId)
			{
				m_ValtanTimelineAudition.ePhase =
					VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH;
				return true;
			}
			if (boss.strPatternId.empty() &&
				!boss.PendingPatternIds.empty() &&
				boss.PendingPatternIds.front() ==
					m_ValtanTimelineAudition.strExpectedPatternId)
			{
				return true;
			}
			fail("Valtan timeline pattern failed to start");
			return false;
		case VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH:
			if (boss.iPatternSequence ==
					m_ValtanTimelineAudition.iExpectedPatternSequence &&
				boss.strPatternId ==
					m_ValtanTimelineAudition.strExpectedPatternId)
			{
				return true;
			}
			if (Is_ValtanOutcomeFollowupInFlight(
					boss,
					m_ValtanTimelineAudition.iExpectedPatternSequence,
					m_ValtanTimelineAudition.PinnedDefinitionRevision))
			{
				return true;
			}
			fail("Valtan timeline pattern identity changed");
			return false;
		case VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD:
		case VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD:
			return false;
		case VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE:
		default:
			return true;
		}
	}
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Restore_ValtanTimelineRowAfterBrain(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t updateTick)
{
	if (VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_START !=
			m_ValtanTimelineAudition.ePhase &&
		VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH !=
			m_ValtanTimelineAudition.ePhase)
	{
		return;
	}
	const CGameplayCatalog* timelineCatalog = m_GameplayCatalog.Resolve(
		m_ValtanTimelineAudition.PinnedDefinitionRevision);
	const VALTAN_TIMELINE_DEFINITION* definition = nullptr == timelineCatalog ?
		nullptr : timelineCatalog->Find_ValtanTimeline(boss.strEncounterId);
	const auto owner = m_Players.find(m_ValtanTimelineAudition.iOwnerPlayerId);
	if (nullptr == definition ||
		m_ValtanTimelineAudition.iRowIndex >= definition->Rows.size() ||
		m_Players.end() == owner)
	{
		boss.PendingPatternIds.clear();
		m_iPillarAuditionBreakTick = 0u;
		m_bPillarAuditionCycleArmed = false;
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD;
		m_strStatus = "Valtan timeline lost runtime state";
		return;
	}
	const VALTAN_TIMELINE_ROW& row =
		definition->Rows[m_ValtanTimelineAudition.iRowIndex];
	if (m_ValtanTimelineAudition.iActionIndex >= row.PatternActions.size())
	{
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD;
		m_strStatus = "Valtan timeline lost its action state";
		return;
	}
	boss.iCurrentHp = m_ValtanTimelineAudition.iHeldBossHp;
	boss.iLastEvaluatedHealthBar =
		m_ValtanTimelineAudition.iHeldBossHealthBar;

	const bool expectedSequence = boss.iPatternSequence ==
		m_ValtanTimelineAudition.iExpectedPatternSequence;
	const bool expectedPattern = boss.strPatternId ==
		m_ValtanTimelineAudition.strExpectedPatternId;
	if (expectedSequence && expectedPattern)
	{
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH;
		return;
	}
	if (Is_ValtanOutcomeFollowupInFlight(
			boss, m_ValtanTimelineAudition.iExpectedPatternSequence,
			m_ValtanTimelineAudition.PinnedDefinitionRevision))
	{
		return;
	}
	const bool completedOutcomeGroup = Has_ValtanOutcomeGroupCompleted(
		boss, m_ValtanTimelineAudition.iExpectedPatternSequence,
		m_ValtanTimelineAudition.PinnedDefinitionRevision);
	if (!completedOutcomeGroup)
	{
		owner->second.isCombatReady = false;
		boss.PendingPatternIds.clear();
		m_iPillarAuditionBreakTick = 0u;
		m_bPillarAuditionCycleArmed = false;
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD;
		m_strStatus = "Valtan timeline observed an unexpected pattern";
		return;
	}
	boss.iPatternFollowupDepth = 0u;
	boss.iPatternFollowupRootSequence = 0u;
	Prepare_TimelineAuditionPlayer(owner->second, updateTick);
	m_ServerTriggerSystem.Remove_Player(owner->second.iPlayerId);
	if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
		owner->second, updateTick))
	{
		owner->second.isCombatReady = false;
		boss.PendingPatternIds.clear();
		m_iPillarAuditionBreakTick = 0u;
		m_bPillarAuditionCycleArmed = false;
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::FAILED_HOLD;
		m_strStatus = "Valtan timeline could not reset its driver between actions";
		return;
	}

	const VALTAN_TIMELINE_PATTERN_ACTION& action =
		row.PatternActions[m_ValtanTimelineAudition.iActionIndex];
	++m_ValtanTimelineAudition.iRepeatIndex;
	const bool occurrenceCompleted =
		m_ValtanTimelineAudition.iRepeatIndex >= action.iRepeat;
	if (occurrenceCompleted)
	{
		++m_ValtanTimelineAudition.iActionIndex;
		m_ValtanTimelineAudition.iRepeatIndex = 0u;
	}
	m_ValtanTimelineAudition.strExpectedPatternId.clear();
	m_ValtanTimelineAudition.iExpectedPatternSequence = 0u;
	if (!occurrenceCompleted ||
		m_ValtanTimelineAudition.iActionIndex < row.PatternActions.size())
	{
		m_ValtanTimelineAudition.ePhase =
			VALTAN_TIMELINE_AUDITION_PHASE::READY;
		m_strStatus = "Valtan timeline row " +
			std::to_string(row.iOrdinal) + " advancing";
		return;
	}
	owner->second.isCombatReady = false;
	m_ValtanTimelineAudition.ePhase =
		VALTAN_TIMELINE_AUDITION_PHASE::COMPLETED_HOLD;
	m_strStatus = "Valtan timeline row " +
		std::to_string(row.iOrdinal) + " completed";
}
#endif

LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanPatternFlowStart(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
	std::uint32_t& outRoomFlowEpoch,
	LostArk::Shared::GameplayDataRevision& outPinnedRevision,
	std::string& outReason)
{
	using namespace LostArk::Shared;
	outRoomFlowEpoch = 0u;
	outPinnedRevision = {};
	outReason.clear();
#ifndef _DEBUG
	(void)sessionId;
	(void)request;
	outReason = "Valtan pattern flow is unavailable in a Release Server";
	return VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD;
#else
	static constexpr std::string_view CANONICAL_BOSS_TOOL_FLOW_ID =
		"flow.valtan.boss-tool.default";
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		outReason = "Valtan pattern flow requires the Valtan Arena";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_WRONG_WORLD;
	}
	auto receipt = m_ValtanPatternFlowStartSequenceBySessionId.find(sessionId);
	if (0u == request.iRequestSequence ||
		(m_ValtanPatternFlowStartSequenceBySessionId.end() != receipt &&
		 !Is_NewerSequence(
			 request.iRequestSequence, receipt->second.iSequence)))
	{
		if (m_ValtanPatternFlowStartSequenceBySessionId.end() != receipt &&
			request.iRequestSequence == receipt->second.iSequence &&
			Build_PatternFlowStartRequestIdentity(request) ==
				receipt->second.strRequestIdentity)
		{
			if (VALTAN_PATTERN_FLOW_RESULT::QUEUED == receipt->second.eResult &&
				receipt->second.iRoomFlowEpoch != 0u &&
				receipt->second.PinnedDefinitionRevision.Is_Valid())
			{
				outRoomFlowEpoch = receipt->second.iRoomFlowEpoch;
				outPinnedRevision = receipt->second.PinnedDefinitionRevision;
				/* The same identity is a receipt recovery, not another Restart. Replay
				   only its last edge so an UNCONFIRMED Client can recover ACTIVE or a
				   terminal hold after the original result/lifecycle was not observed. */
				if (receipt->second.LastLifecycle.has_value())
				{
					m_PendingValtanPatternFlowLifecycle.push_back({
						sessionId, *receipt->second.LastLifecycle });
				}
				return VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED;
			}
			outReason = receipt->second.strReason;
			return receipt->second.eResult;
		}
		outReason = "Valtan pattern-flow request sequence is invalid or stale";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
	}
	m_ValtanPatternFlowStartSequenceBySessionId.insert_or_assign(
		sessionId, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT{
			request.iRequestSequence, 0u, request.strFlowId,
			request.strFlowRevision, {}, {} });

	if (!request.ExpectedDefinitionRevision.Is_Valid() ||
		!Is_StablePatternFlowId(request.strBossPlacementId) ||
		!Is_StablePatternFlowId(request.strFlowId) ||
		!Is_PatternFlowRevision(request.strFlowRevision) ||
		!Is_StablePatternFlowId(request.strStartSlotId) ||
		request.Slots.empty() ||
		request.Slots.size() > MAX_VALTAN_PATTERN_FLOW_SLOTS ||
		request.iInterStepPursuitMs <
			MIN_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS ||
		request.iInterStepPursuitMs >
			MAX_VALTAN_PATTERN_FLOW_INTER_STEP_PURSUIT_MS)
	{
		outReason = "Valtan pattern-flow request is invalid";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	const GameplayDataRevision activeDefinitionRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (request.ExpectedDefinitionRevision != activeDefinitionRevision)
	{
		outReason =
			"Valtan pattern-flow expected definition revision is no longer active";
		m_ValtanPatternFlowStartSequenceBySessionId.insert_or_assign(
			sessionId, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT{
				request.iRequestSequence, 0u, request.strFlowId,
				request.strFlowRevision,
				Build_PatternFlowStartRequestIdentity(request), {},
				VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_DEFINITION,
				outReason });
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_DEFINITION;
	}
	if ("boss.valtan.center" != request.strBossPlacementId)
	{
		outReason = "Valtan pattern-flow boss placement is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_NO_BOSS;
	}
	if ((Is_ValtanPatternFlowRunning() &&
		 m_ValtanPatternFlowAudition.iOwnerSessionId != sessionId) ||
		(VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE != m_ValtanPatternIdAudition.ePhase &&
		 INVALID_SESSION_ID != m_ValtanPatternIdAudition.iOwnerSessionId &&
		 m_ValtanPatternIdAudition.iOwnerSessionId != sessionId) ||
		(m_ValtanNextPattern && m_ValtanNextPattern->iOwnerSessionId != sessionId) ||
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase || m_ValtanFightPageStart.Is_Active() ||
		0u != m_iValtanAuditionArmedHealthBar)
	{
		outReason = "Another owner or Timeline controls Valtan playback";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_CONFLICT;
	}

	SERVER_WORLD_ENTITY* boss = Find_AuditionBoss(request.strBossPlacementId);
	if (nullptr == boss)
	{
		outReason = "Valtan pattern-flow boss was not found";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_NO_BOSS;
	}
	if (0u == boss->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss->eAction)
	{
		outReason = "Valtan pattern-flow boss is dead";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_BOSS_DEAD;
	}

	const auto ownerId = m_PlayerIdBySessionId.find(sessionId);
	const auto owner = ownerId == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(ownerId->second);
	if (m_Players.end() == owner)
	{
		outReason = "Valtan pattern-flow player is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
	}
	const PLAYER_RUNTIME_PROFILE* playerProfile =
		m_GameplayCatalog.Find_Player(owner->second.eCharacterClass);
	if (nullptr == playerProfile)
	{
		outReason = "Valtan pattern-flow player profile is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
	}

	const GameplayDataRevision pinnedRevision = activeDefinitionRevision;
	const CGameplayCatalog* pinnedCatalog =
		m_GameplayCatalog.Resolve(pinnedRevision);
	const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
		nullptr == pinnedCatalog ? nullptr :
		pinnedCatalog->Find_BossPatterns(boss->strEncounterId);
	if (nullptr == patterns)
	{
		outReason = "Valtan pattern-flow gameplay revision is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}

	std::set<std::string> slotIds;
	std::size_t startSlotIndex = request.Slots.size();
	for (std::size_t index = 0u; index < request.Slots.size(); ++index)
	{
		const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot = request.Slots[index];
		if (!Is_StablePatternFlowId(slot.strSlotId) ||
			!Is_StablePatternFlowId(slot.strPatternId) ||
			!slotIds.insert(slot.strSlotId).second)
		{
			outReason = "Valtan pattern flow has an invalid or duplicate slot ID";
			return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
		}
		if (slot.strSlotId == request.strStartSlotId)
			startSlotIndex = index;
		if (patterns->end() == std::find_if(
				patterns->begin(), patterns->end(),
				[&slot](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return pattern.strPatternId == slot.strPatternId;
				}))
		{
			outReason = "Valtan pattern flow references an unknown pattern";
			return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
		}
	}
	if (startSlotIndex >= request.Slots.size())
	{
		outReason = "Valtan pattern-flow start slot is not in the flow";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	const BOSS_PATTERN_SEQUENCE_DEFINITION* savedCanonicalSequence = nullptr;
	if (CANONICAL_BOSS_TOOL_FLOW_ID == request.strFlowId)
	{
		savedCanonicalSequence =
			pinnedCatalog->Find_BossPatternSequence(boss->strEncounterId);
		const bool bMatchesSavedSequence = nullptr != savedCanonicalSequence &&
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE ==
				savedCanonicalSequence->eMode &&
			!request.Slots.empty() &&
			request.strStartSlotId == request.Slots.front().strSlotId &&
			savedCanonicalSequence->iInterStepPursuitMs ==
				request.iInterStepPursuitMs &&
			savedCanonicalSequence->TransitionPursuitMs.size() + 1u ==
				savedCanonicalSequence->PatternIds.size() &&
			savedCanonicalSequence->PatternIds.size() == request.Slots.size() &&
			std::equal(
				savedCanonicalSequence->PatternIds.begin(),
				savedCanonicalSequence->PatternIds.end(), request.Slots.begin(),
				[](const std::string& patternId,
					const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot)
				{
					return patternId == slot.strPatternId;
				});
		if (!bMatchesSavedSequence)
		{
			outReason =
				"Boss Tool pattern flow does not match the Server-active canonical scriptedSequence";
			return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
		}
	}

	const std::uint32_t resetTick =
		0u == m_iServerTick ? 1u : m_iServerTick;
	SERVER_PLAYER stagedOwner = owner->second;
	Prepare_TimelineAuditionPlayer(stagedOwner, resetTick);
	stagedOwner.eStance = playerProfile->eDefaultStance;
	stagedOwner.CooldownEndTickBySkillId.clear();
	stagedOwner.fKnockbackRemainingSeconds = 0.f;
	stagedOwner.fKnockbackSpeed = 0.f;
	stagedOwner.iKnockdownEndTick = 0u;
	stagedOwner.iHitReactionGraceEndTick = 0u;
	if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
			stagedOwner, resetTick))
	{
		outReason = "Valtan pattern-flow bait placement is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
	}
	SERVER_WORLD_ENTITY stagedBoss{};
	std::string resetStatus;
	if (!Build_ValtanBossOnlyAuditionReset(
			*boss, resetTick, stagedBoss, resetStatus))
	{
		outReason = std::move(resetStatus);
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	CWorldDestructionRuntime stagedDestruction = m_WorldDestructionRuntime;
	if (!stagedDestruction.Reset(resetStatus, resetTick))
	{
		outReason = std::move(resetStatus);
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	CEncounterPropRuntime stagedProps = m_EncounterPropRuntime;
	if (stagedProps.Is_Initialized() &&
		!stagedProps.Reset(resetStatus, resetTick))
	{
		outReason = std::move(resetStatus);
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	const float baitDeltaX = stagedOwner.fPositionX - stagedBoss.fPositionX;
	const float baitDeltaZ = stagedOwner.fPositionZ - stagedBoss.fPositionZ;
	if (baitDeltaX * baitDeltaX + baitDeltaZ * baitDeltaZ >
		stagedBoss.fEngageDistance * stagedBoss.fEngageDistance)
	{
		outReason = "Valtan pattern-flow bait is outside boss engage range";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
	}

	VALTAN_PATTERN_FLOW_AUDITION_STATE stagedFlow{};
	stagedFlow.ePhase = VALTAN_PATTERN_FLOW_AUDITION_PHASE::PENDING;
	stagedFlow.iOwnerSessionId = sessionId;
	stagedFlow.iOwnerPlayerId = stagedOwner.iPlayerId;
	stagedFlow.iBossEntityId = stagedBoss.iNetEntityId;
	stagedFlow.iRequestSequence = request.iRequestSequence;
	stagedFlow.iRoomFlowEpoch = m_iNextValtanPatternFlowEpoch;
	stagedFlow.iFirstPatternSequence =
		(std::numeric_limits<std::uint32_t>::max)() ==
			stagedBoss.iPatternSequence ? 1u : stagedBoss.iPatternSequence + 1u;
	stagedFlow.iStartSlotIndex = startSlotIndex;
	stagedFlow.strBossPlacementId = request.strBossPlacementId;
	stagedFlow.strFlowId = request.strFlowId;
	stagedFlow.strFlowRevision = request.strFlowRevision;
	stagedFlow.strStartSlotId = request.strStartSlotId;
	stagedFlow.Slots = request.Slots;
	stagedFlow.PinnedDefinitionRevision = pinnedRevision;
	stagedFlow.Sequence.strEncounterId = stagedBoss.strEncounterId;
	stagedFlow.Sequence.strSequenceId =
		"debug.valtan.pattern-flow." +
		std::to_string(stagedFlow.iRoomFlowEpoch);
	stagedFlow.Sequence.eMode =
		BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
	stagedFlow.Sequence.iInterStepPursuitMs =
		request.iInterStepPursuitMs;
	stagedFlow.Sequence.iInterStepPursuitTicks =
		static_cast<std::uint32_t>((
			static_cast<std::uint64_t>(request.iInterStepPursuitMs) *
			SERVER_TICK_HZ + 999u) / 1000u);
	for (std::size_t index = startSlotIndex;
		index < request.Slots.size(); ++index)
	{
		stagedFlow.Sequence.PatternIds.push_back(
			request.Slots[index].strPatternId);
		if (index + 1u < request.Slots.size())
		{
			const std::uint32_t pursuitMs = nullptr == savedCanonicalSequence ?
				request.iInterStepPursuitMs :
				savedCanonicalSequence->TransitionPursuitMs[index];
			stagedFlow.Sequence.TransitionPursuitMs.push_back(pursuitMs);
			stagedFlow.Sequence.TransitionPursuitTicks.push_back(
				static_cast<std::uint32_t>((
					static_cast<std::uint64_t>(pursuitMs) * SERVER_TICK_HZ +
					999u) / 1000u));
		}
	}
	stagedFlow.Sequence.iExpectedStepCount = static_cast<std::uint32_t>(
		stagedFlow.Sequence.PatternIds.size());

	if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
	{
		outReason = std::move(resetStatus);
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW;
	}
	owner->second = std::move(stagedOwner);
	m_CombatObjectRuntime.Cancel_Source(owner->second.iNetEntityId);
	m_ServerTriggerSystem.Remove_Player(owner->second.iPlayerId);
	boss->bIntroPatternConsumed = true;
	boss->bScriptedPatternPlayback = false;
	boss->bAutomaticPatternSequenceAuditionOverride = false;
	boss->bAutomaticPatternSequenceAuditionHold = false;
	m_ValtanPatternFlowAudition = std::move(stagedFlow);
	m_iNextValtanPatternFlowEpoch =
		(std::numeric_limits<std::uint32_t>::max)() ==
			m_iNextValtanPatternFlowEpoch ?
		1u : m_iNextValtanPatternFlowEpoch + 1u;
	outRoomFlowEpoch = m_ValtanPatternFlowAudition.iRoomFlowEpoch;
	outPinnedRevision = pinnedRevision;
	m_ValtanPatternFlowStartSequenceBySessionId.insert_or_assign(
		sessionId, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT{
			request.iRequestSequence, outRoomFlowEpoch,
			request.strFlowId, request.strFlowRevision,
			Build_PatternFlowStartRequestIdentity(request), pinnedRevision,
			VALTAN_PATTERN_FLOW_RESULT::QUEUED, {} });
	Queue_ValtanPatternFlowLifecycle(
		VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING, boss);
	m_strStatus = "Valtan pattern flow queued: " + request.strFlowId;
	return VALTAN_PATTERN_FLOW_RESULT::QUEUED;
#endif
}

LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanPatternFlowStopAfterCurrent(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
	std::uint32_t& outRoomFlowEpoch,
	LostArk::Shared::GameplayDataRevision& outPinnedRevision,
	std::string& outReason)
{
	using namespace LostArk::Shared;
	outRoomFlowEpoch = 0u;
	outPinnedRevision = {};
	outReason.clear();
#ifndef _DEBUG
	(void)sessionId;
	(void)request;
	outReason = "Valtan pattern flow is unavailable in a Release Server";
	return VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD;
#else
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		outReason = "Valtan pattern flow requires the Valtan Arena";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_WRONG_WORLD;
	}
	auto receipt = m_ValtanPatternFlowControlSequenceBySessionId.find(sessionId);
	if (0u == request.iControlSequence ||
		(m_ValtanPatternFlowControlSequenceBySessionId.end() != receipt &&
		 !Is_NewerSequence(
			 request.iControlSequence, receipt->second.iSequence)))
	{
		if (m_ValtanPatternFlowControlSequenceBySessionId.end() != receipt &&
			request.iControlSequence == receipt->second.iSequence &&
			request.strFlowId == receipt->second.strFlowId &&
			request.iRoomFlowEpoch == receipt->second.iRoomFlowEpoch &&
			receipt->second.iRoomFlowEpoch != 0u &&
			receipt->second.PinnedDefinitionRevision.Is_Valid())
		{
			outRoomFlowEpoch = receipt->second.iRoomFlowEpoch;
			outPinnedRevision = receipt->second.PinnedDefinitionRevision;
			return VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED;
		}
		outReason = "Valtan pattern-flow control sequence is invalid or stale";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
	}
	m_ValtanPatternFlowControlSequenceBySessionId.insert_or_assign(
		sessionId, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT{
			request.iControlSequence, 0u, request.strFlowId, {}, {}, {} });
	if (!Is_StablePatternFlowId(request.strFlowId) ||
		0u == request.iRoomFlowEpoch || !Is_ValtanPatternFlowRunning() ||
		sessionId != m_ValtanPatternFlowAudition.iOwnerSessionId ||
		request.strFlowId != m_ValtanPatternFlowAudition.strFlowId ||
		request.iRoomFlowEpoch != m_ValtanPatternFlowAudition.iRoomFlowEpoch)
	{
		outReason = "Valtan pattern-flow control targets a stale flow";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
	}
	SERVER_WORLD_ENTITY* boss = Find_AuditionBoss(
		m_ValtanPatternFlowAudition.strBossPlacementId);
	if (nullptr == boss ||
		boss->iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId)
	{
		outReason = "Valtan pattern-flow boss is unavailable";
		return VALTAN_PATTERN_FLOW_RESULT::REJECTED_NO_BOSS;
	}
	outRoomFlowEpoch = m_ValtanPatternFlowAudition.iRoomFlowEpoch;
	outPinnedRevision =
		m_ValtanPatternFlowAudition.PinnedDefinitionRevision;
	const std::string flowRevision =
		m_ValtanPatternFlowAudition.strFlowRevision;
	if (boss->bAutomaticPatternSequenceStepRunning ||
		!boss->strPatternId.empty() ||
		boss->PendingPatternFollowup.Is_Pending() ||
		boss->iPatternFollowupDepth > 0u)
	{
		m_ValtanPatternFlowAudition.bStopAfterCurrent = true;
	}
	else
	{
		Finish_ValtanPatternFlow(
			*boss, VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD,
			"Valtan pattern flow stopped before the next slot");
	}
	m_ValtanPatternFlowControlSequenceBySessionId.insert_or_assign(
		sessionId, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT{
			request.iControlSequence, outRoomFlowEpoch,
			request.strFlowId, flowRevision, {}, outPinnedRevision });
	return VALTAN_PATTERN_FLOW_RESULT::QUEUED;
#endif
}

LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	std::uint32_t& outCurrentHealthBar)
{
	using namespace LostArk::Shared;
	outCurrentHealthBar = 0u;

#ifndef _DEBUG
	/* A Release Server keeps the packet type known so this answers an explicit
	rejection instead of closing the session on an unrecognised frame, but it
	never moves a boss. */
	(void)sessionId;
	(void)request;
	return VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD;
#else
	if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID == request.eOperation ||
		VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID == request.eOperation ||
		VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == request.eOperation)
	{
		return Evaluate_ValtanNextPatternControl(sessionId, request, outCurrentHealthBar);
	}
	std::uint32_t& currentHealthBar = outCurrentHealthBar;
	const bool isPatternIdPlay =
		VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == request.eOperation;
	const bool isPatternIdRestart =
		VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID == request.eOperation;
	const bool isPatternIdCommand = isPatternIdPlay || isPatternIdRestart;
	const bool isArenaPreset =
		VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET == request.eOperation;
	/* Revision-unaware Level-owned Pattern, health-bar and timeline commands
	   were retired. Reject them before receipt, boss, player or arena state can
	   be touched; Product playback now has one stable-ID/revision path. */
	if (!isPatternIdCommand && !isArenaPreset)
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	bool shouldStorePatternIdReceipt = isPatternIdCommand;
	const auto evaluate = [&]() -> VALTAN_AUDITION_RESULT
	{
		const bool isValtanArena = WORLD_ID::VALTAN_ARENA == m_eWorldId;
		const bool isCharacterSelectArena =
			WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId;
		if ((!isPatternIdCommand && !isValtanArena) ||
			(isPatternIdCommand && !isValtanArena && !isCharacterSelectArena) ||
			!m_PlayerIdBySessionId.contains(sessionId))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
		}
		if (isPatternIdCommand)
		{
			const auto handled =
				m_ValtanPatternIdAuditionSequenceBySessionId.find(sessionId);
			if (0u == request.iRequestSequence)
			{
				shouldStorePatternIdReceipt = false;
				return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			}
			if (m_ValtanPatternIdAuditionSequenceBySessionId.end() != handled)
			{
				const auto& prior = handled->second;
				if (request.iRequestSequence == prior.Request.iRequestSequence)
				{
					shouldStorePatternIdReceipt = false;
					const bool exactRetry =
						request.eOperation == prior.Request.eOperation &&
						request.iTargetHealthBar == prior.Request.iTargetHealthBar &&
						request.strBossPlacementId ==
							prior.Request.strBossPlacementId &&
						request.strPatternId == prior.Request.strPatternId &&
						request.iPredecessorRoomAuditionEpoch ==
							prior.Request.iPredecessorRoomAuditionEpoch &&
						request.iPredecessorPatternSequence ==
							prior.Request.iPredecessorPatternSequence &&
						request.iExpectedNextRequestSequence ==
							prior.Request.iExpectedNextRequestSequence &&
						request.ExpectedDefinitionRevision ==
							prior.Request.ExpectedDefinitionRevision &&
						request.ReplacementDefinitionRevision ==
							prior.Request.ReplacementDefinitionRevision;
					if (!exactRetry)
						return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
					currentHealthBar = prior.iCurrentHealthBar;
					if (VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED ==
						prior.Result &&
						VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID ==
							prior.Request.eOperation)
					{
						SERVER_WORLD_ENTITY* const predecessorBoss =
							Find_AuditionBoss(prior.Request.strBossPlacementId);
						const auto& predecessor = m_ValtanPatternIdAudition;
						const bool sameIdentity = nullptr != predecessorBoss &&
							predecessor.iOwnerSessionId == sessionId &&
							predecessor.iRoomAuditionEpoch ==
								prior.Request.iPredecessorRoomAuditionEpoch &&
							predecessor.iExpectedPatternSequence ==
								prior.Request.iPredecessorPatternSequence &&
							predecessor.PinnedDefinitionRevision ==
								prior.Request.ExpectedDefinitionRevision &&
							predecessor.strBossPlacementId ==
								prior.Request.strBossPlacementId &&
							predecessor.strPatternId == prior.Request.strPatternId &&
							predecessorBoss->iNetEntityId ==
								predecessor.iBossEntityId &&
							!predecessorBoss->bMechanicLedgerRequiresReset;
						const bool activePredecessor = sameIdentity &&
							VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
								predecessor.ePhase &&
							((predecessorBoss->iPatternSequence ==
									predecessor.iExpectedPatternSequence &&
							  predecessorBoss->strPatternId == predecessor.strPatternId &&
							  predecessorBoss->PinnedDefinitionRevision ==
									predecessor.PinnedDefinitionRevision) ||
							 Is_ValtanOutcomeFollowupInFlight(
								 *predecessorBoss,
								 predecessor.iExpectedPatternSequence,
								 predecessor.PinnedDefinitionRevision));
						const bool completedPredecessor = sameIdentity &&
							VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
								predecessor.ePhase &&
							Has_ValtanOutcomeGroupCompleted(
								*predecessorBoss,
								predecessor.iExpectedPatternSequence,
								predecessor.PinnedDefinitionRevision);
						if (!activePredecessor && !completedPredecessor)
							return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
						Queue_ValtanAuditionLifecycle(
							predecessor.iOwnerSessionId,
							predecessor.iRequestSequence,
							predecessor.iRoomAuditionEpoch,
							predecessor.iExpectedPatternSequence,
							predecessor.strPatternId,
							predecessor.PinnedDefinitionRevision,
							activePredecessor ?
								VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE :
								VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
						return prior.Result;
					}
					bool replayedCurrentLifecycle = false;
					if (VALTAN_AUDITION_RESULT::QUEUED == prior.Result &&
						request.iRequestSequence ==
							m_ValtanPatternIdAudition.iRequestSequence &&
						!m_ValtanPatternIdAudition.bAdoptedLivePredecessor)
					{
						switch (m_ValtanPatternIdAudition.ePhase)
						{
						case VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING:
							Queue_ValtanPatternIdAuditionLifecycle(
								m_ValtanPatternIdAudition.bReportedWaitingForPlayer ?
									VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER :
									VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
							break;
						case VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE:
							Queue_ValtanPatternIdAuditionLifecycle(
								VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
							break;
						case VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD:
							Queue_ValtanPatternIdAuditionLifecycle(
								VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
							break;
						default: break;
						}
						replayedCurrentLifecycle = true;
					}
					if (VALTAN_AUDITION_RESULT::QUEUED == prior.Result &&
						!replayedCurrentLifecycle && prior.LastLifecycle)
					{
						m_PendingValtanAuditionLifecycle.push_back(
							{ sessionId, *prior.LastLifecycle });
					}
					return prior.Result;
				}
				if (!Is_NewerSequence(
						request.iRequestSequence,
						prior.Request.iRequestSequence))
				{
					shouldStorePatternIdReceipt = false;
					return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
				}
			}
		}
		else
		{
			const auto handled = m_ValtanAuditionSequenceBySessionId.find(sessionId);
			if (0u == request.iRequestSequence ||
				(m_ValtanAuditionSequenceBySessionId.end() != handled &&
				 !Is_NewerSequence(request.iRequestSequence, handled->second)))
			{
				return VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED;
			}
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
		}
		const GameplayDataRevision activeDefinitionRevision =
			m_GameplayCatalog.Get_ActiveRevision();
		/* Complete Play is a compare-and-swap against the exact generation the
		   Client admitted. Reject a delayed request before player staging, boss
		   reset, pending-pattern mutation, or any arena-owned state can change. */
		if (isPatternIdPlay &&
			request.ExpectedDefinitionRevision != activeDefinitionRevision)
		{
			m_strStatus =
				"Valtan Complete Play expected definition revision is no longer active";
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
		}
		if (Is_ValtanPatternIdAuditionRunning() &&
			!isPatternIdRestart)
		{
			m_strStatus = "Valtan stable-ID pattern audition is already "
				"pending or active: " +
				m_ValtanPatternIdAudition.strPatternId;
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (Is_ValtanPatternFlowRunning())
		{
			m_strStatus =
				"A Valtan pattern flow is already pending or active";
			return isPatternIdRestart ?
				VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION :
				VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (isPatternIdCommand)
		{
			CPacketWriter wireValidation;
			if (!Write_Message(wireValidation, request) ||
				0u != request.iTargetHealthBar ||
				request.strBossPlacementId.empty() || request.strPatternId.empty())
			{
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			const char* expectedPlacementId = isValtanArena ?
				VALTAN_ARENA_AUDITION_PLACEMENT_ID :
				CHARACTER_SELECT_AUDITION_PLACEMENT_ID;
			if (request.strBossPlacementId != expectedPlacementId)
				return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;

			SERVER_WORLD_ENTITY* boss =
				Find_AuditionBoss(request.strBossPlacementId);
			if (nullptr == boss)
				return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;
			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			if (0u == boss->iCurrentHp ||
				SERVER_ENTITY_ACTION::DEAD == boss->eAction)
			{
				return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
			}
			/* Restart is an exact occurrence CAS. IDs alone never authorize a
			   replacement, so a delayed packet cannot restart a later occurrence. */
			if (isPatternIdRestart)
			{
				const auto& predecessor = m_ValtanPatternIdAudition;
				const bool activeRootOccurrence =
					VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == predecessor.ePhase &&
					((boss->iPatternSequence == predecessor.iExpectedPatternSequence &&
					  boss->strPatternId == predecessor.strPatternId &&
					  boss->PinnedDefinitionRevision ==
						  predecessor.PinnedDefinitionRevision) ||
					 Is_ValtanOutcomeFollowupInFlight(
						 *boss, predecessor.iExpectedPatternSequence,
						 predecessor.PinnedDefinitionRevision));
				const bool completedRootOccurrence =
					VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
						predecessor.ePhase &&
					Has_ValtanOutcomeGroupCompleted(
						*boss, predecessor.iExpectedPatternSequence,
						predecessor.PinnedDefinitionRevision);
				if (m_ValtanNextPattern.has_value() ||
					request.iPredecessorRoomAuditionEpoch !=
						predecessor.iRoomAuditionEpoch ||
					request.iPredecessorPatternSequence !=
						predecessor.iExpectedPatternSequence ||
					request.ExpectedDefinitionRevision !=
						predecessor.PinnedDefinitionRevision ||
					request.strBossPlacementId != predecessor.strBossPlacementId ||
					request.strPatternId != predecessor.strPatternId ||
					boss->iNetEntityId != predecessor.iBossEntityId ||
					boss->strPlacementId != predecessor.strBossPlacementId ||
					boss->bMechanicLedgerRequiresReset ||
					(!activeRootOccurrence && !completedRootOccurrence))
				{
					m_strStatus =
						"Valtan restart predecessor occurrence is no longer current";
					return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
				}
				if (sessionId != predecessor.iOwnerSessionId)
					return VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER;
				if (!activeRootOccurrence && !completedRootOccurrence)
					return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
				/* The predecessor pin and the replacement generation are distinct
				   compare-and-swap inputs. Check the live occurrence first so only
				   an exact predecessor can receive the preserving rejection. */
				if (request.ReplacementDefinitionRevision !=
						activeDefinitionRevision)
				{
					m_strStatus =
						"Valtan restart replacement definition revision is no longer active";
					return VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED;
				}
			}
			const auto rejectPreservingExactPredecessor =
				[isPatternIdRestart](const VALTAN_AUDITION_RESULT ordinary)
				{
					return isPatternIdRestart ?
						VALTAN_AUDITION_RESULT::REJECTED_OCCURRENCE_PRESERVED :
						ordinary;
				};
			if (0u == m_iNextValtanAuditionEpoch ||
				(std::numeric_limits<std::uint32_t>::max)() == boss->iPatternSequence)
			{
				m_strStatus = "Valtan audition identity stream is exhausted";
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			const GameplayDataRevision commandDefinitionRevision =
				isPatternIdRestart ? request.ReplacementDefinitionRevision :
				activeDefinitionRevision;
			const CGameplayCatalog* const commandDefinitions =
				m_GameplayCatalog.Resolve(commandDefinitionRevision);
			const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
				nullptr == commandDefinitions ? nullptr :
				commandDefinitions->Find_BossPatterns(boss->strEncounterId);
			if (nullptr == patterns || patterns->end() == std::find_if(
					patterns->begin(), patterns->end(),
					[&request](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == request.strPatternId;
					}))
			{
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			if (isCharacterSelectArena &&
				Is_CharacterSelectEnvironmentDependentPattern(
					request.strPatternId))
			{
				m_strStatus =
					"Character Select cannot audition an arena-environment pattern";
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
			{
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}

			/* Valtan Arena keeps its existing one-click bait placement. Character
			   Select has no raid trigger/bait contract and therefore requires its
			   private-room player to already be engaged with the spawned boss. Stage
			   the raid player copy; no reject path may leak pose/combat mutations. */
			SERVER_PLAYER* livePlayer = nullptr;
			std::optional<SERVER_PLAYER> stagedPlayer;
			if (isValtanArena)
			{
				const auto playerId = m_PlayerIdBySessionId.find(sessionId);
				if (m_PlayerIdBySessionId.end() == playerId)
					return rejectPreservingExactPredecessor(
						VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);
				const auto player = m_Players.find(playerId->second);
				if (m_Players.end() == player || 0u == player->second.iCurrentHp)
					return rejectPreservingExactPredecessor(
						VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);
				livePlayer = &player->second;
				stagedPlayer = player->second;
				stagedPlayer->isCombatReady = true;
				if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
						*stagedPlayer,
						0u == m_iServerTick ? 1u : m_iServerTick))
				{
					return rejectPreservingExactPredecessor(
						VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);
				}
			}
			if (!isValtanArena && !Has_EngagedAuditionPlayer(*boss))
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);

			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			std::string resetStatus;
			/* Preflight the canonical boss replacement before mutating the live
			   occurrence. Both supported worlds use this boss-only reset: Complete
			   Play must never imply a Fresh-arena preset. */
			SERVER_WORLD_ENTITY resetProbe{};
			if (!Build_ValtanBossOnlyAuditionReset(
					*boss, resetTick, resetProbe, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			if (resetProbe.PinnedDefinitionRevision != commandDefinitionRevision)
			{
				m_strStatus =
					"Valtan restart reset did not use the admitted replacement definition";
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			if (stagedPlayer)
			{
				const float deltaX = stagedPlayer->fPositionX - resetProbe.fPositionX;
				const float deltaZ = stagedPlayer->fPositionZ - resetProbe.fPositionZ;
				if (0u == stagedPlayer->iCurrentHp ||
					!stagedPlayer->isCombatReady ||
					PLAYER_ACTION_STATE::DEAD == stagedPlayer->eAction ||
					PLAYER_ACTION_STATE::FALLING == stagedPlayer->eAction ||
					deltaX * deltaX + deltaZ * deltaZ >
						resetProbe.fEngageDistance * resetProbe.fEngageDistance)
				{
					return rejectPreservingExactPredecessor(
						VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);
				}
			}
			else if (!Has_EngagedAuditionPlayer(resetProbe))
			{
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED);
			}
			if (!Reset_ValtanBossOnlyAuditionState(
					*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return rejectPreservingExactPredecessor(
					VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE);
			}
			if (nullptr != livePlayer && stagedPlayer)
				*livePlayer = std::move(*stagedPlayer);

			boss->bIntroPatternConsumed = true;
			boss->PendingPatternIds.push_back(request.strPatternId);
			m_ValtanPatternIdAudition.ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
			m_ValtanPatternIdAudition.iOwnerSessionId = sessionId;
			m_ValtanPatternIdAudition.iBossEntityId = boss->iNetEntityId;
			m_ValtanPatternIdAudition.iExpectedPatternSequence =
				boss->iPatternSequence + 1u;
			m_ValtanPatternIdAudition.strBossPlacementId =
				boss->strPlacementId;
			m_ValtanPatternIdAudition.strPatternId = request.strPatternId;
			m_ValtanPatternIdAudition.iRequestSequence =
				request.iRequestSequence;
			m_ValtanPatternIdAudition.iRoomAuditionEpoch =
				m_iNextValtanAuditionEpoch;
			m_iNextValtanAuditionEpoch =
				(std::numeric_limits<std::uint32_t>::max)() ==
					m_iNextValtanAuditionEpoch ?
				0u : m_iNextValtanAuditionEpoch + 1u;
			m_ValtanPatternIdAudition.PinnedDefinitionRevision =
				commandDefinitionRevision;
			Queue_ValtanPatternIdAuditionLifecycle(
				VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
			m_iValtanAuditionArmedHealthBar = 0u;
			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			m_strStatus = "Valtan pattern ID audition queued: " +
				request.strPatternId;
			return VALTAN_AUDITION_RESULT::QUEUED;
		}
		/* SET_ARENA_PRESET is the only consumer that remains on the legacy
		   envelope. It stages environment state only; Product Pattern playback
		   returned through the stable-ID branch above. */
		if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
		{
			m_strStatus =
				"Stop the active Valtan timeline row before changing the arena preset";
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (m_ValtanFightPageStart.Is_Active())
		{
			m_strStatus =
				"Wait for the active Valtan fight page environment to finish";
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
		if (nullptr == boss)
		{
			(void)Activate_Encounter("boss.valtan.center");
			boss = Find_AuditionBoss();
		}
		if (nullptr == boss)
			return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;

		currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		if (0u == boss->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == boss->eAction)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
		}

		VALTAN_TIMELINE_ARENA_STATE arenaState =
			VALTAN_TIMELINE_ARENA_STATE::FRESH;
		const char* presetLabel = nullptr;
		if (!Resolve_ValtanArenaPreset(
				request.iTargetHealthBar, arenaState, presetLabel))
		{
			m_strStatus = "Valtan arena preset identity is invalid";
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		const std::uint32_t presetTick =
			0u == m_iServerTick ? 1u : m_iServerTick;
		/* Prove the complete destruction graph against a copy before the live
		   arena reset. A bad binding therefore cannot cost the inspected state. */
		CWorldDestructionRuntime stagedRuntime = m_WorldDestructionRuntime;
		std::string presetStatus;
		if (!stagedRuntime.Reset(presetStatus, presetTick))
		{
			m_strStatus = "Valtan arena preset reset preflight failed: " +
				presetStatus;
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		WORLD_DESTRUCTION_TRANSACTION stagedTransaction{};
		std::vector<std::string> stagedExpectedGone;
		if (!Prepare_ValtanTimelineArenaState(
				stagedRuntime, *boss, arenaState, presetTick,
				stagedTransaction, stagedExpectedGone, presetStatus) ||
			(!stagedTransaction.Transitions.empty() &&
			 !stagedRuntime.Commit(stagedTransaction, presetStatus)))
		{
			m_strStatus = "Valtan arena preset graph preflight failed: " +
				presetStatus;
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		if (!Reset_ValtanAuditionState(*boss, presetTick, presetStatus))
		{
			m_strStatus = std::move(presetStatus);
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		std::vector<std::string> expectedGone;
		if (!Prepare_ValtanTimelineArenaState(
				m_WorldDestructionRuntime, *boss, arenaState, presetTick,
				transaction, expectedGone, presetStatus) ||
			(!transaction.Transitions.empty() &&
			 !Commit_WorldDestructionTransaction(
				transaction, {}, presetTick, presetStatus)))
		{
			const std::string failure = presetStatus;
			std::string rollbackStatus;
			(void)Reset_ValtanAuditionState(
				*boss, presetTick, rollbackStatus);
			m_strStatus =
				"Valtan arena preset commit rolled back to Fresh: " + failure;
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		/* Keep the authoritative boss idle while the team inspects the chosen
		   wall/floor state. Stable-ID Pattern commands deliberately replace this
		   hold without implying a Fresh preset. */
		boss->bIntroPatternConsumed = true;
		boss->bAutomaticPatternSequenceAuditionOverride = true;
		boss->bAutomaticPatternSequenceAuditionHold = true;
		boss->PendingPatternIds.clear();
		currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		m_iValtanAuditionArmedHealthBar = 0u;
		m_ValtanAuditionSequenceBySessionId.insert_or_assign(
			sessionId, request.iRequestSequence);
		m_strStatus = std::string("Valtan arena preset staged: ") +
			(nullptr == presetLabel ? "UNKNOWN" : presetLabel);
		return VALTAN_AUDITION_RESULT::QUEUED;
	};
	const VALTAN_AUDITION_RESULT verdict = evaluate();
	if (shouldStorePatternIdReceipt)
	{
		VALTAN_PATTERN_ID_COMMAND_RECEIPT receipt{
			request, verdict, currentHealthBar };
		if (VALTAN_AUDITION_RESULT::QUEUED == verdict)
		{
			const auto lifecycle = std::find_if(
				m_PendingValtanAuditionLifecycle.rbegin(),
				m_PendingValtanAuditionLifecycle.rend(),
				[sessionId, &request](
					const TARGETED_VALTAN_AUDITION_LIFECYCLE& pending)
				{
					return sessionId == pending.iSessionId &&
						request.iRequestSequence ==
							pending.Message.iRequestSequence &&
						request.strPatternId == pending.Message.strPatternId;
				});
			if (m_PendingValtanAuditionLifecycle.rend() != lifecycle)
				receipt.LastLifecycle = lifecycle->Message;
		}
		m_ValtanPatternIdAuditionSequenceBySessionId.insert_or_assign(
			sessionId, std::move(receipt));
	}
	return verdict;
#endif
}

void LostArk::Server::CGameRoom::Handle_ValtanAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;

	std::uint32_t currentHealthBar = 0u;
	const LostArk::Shared::VALTAN_AUDITION_RESULT result =
		Evaluate_ValtanAudition(sessionId, request, currentHealthBar);
	if (!Send_ValtanAuditionResult(
		session, request, result, currentHealthBar))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_ValtanPatternFlowStart(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	std::uint32_t roomFlowEpoch = 0u;
	GameplayDataRevision pinnedRevision{};
	std::string reason;
	const VALTAN_PATTERN_FLOW_RESULT result = Evaluate_ValtanPatternFlowStart(
		sessionId, request, roomFlowEpoch, pinnedRevision, reason);
	if (!Send_ValtanPatternFlowResult(
			session, request.iRequestSequence,
			VALTAN_PATTERN_FLOW_COMMAND::START, result,
			request.strFlowId, request.strFlowRevision, roomFlowEpoch,
			pinnedRevision, reason))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_ValtanPatternFlowStopAfterCurrent(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	std::uint32_t roomFlowEpoch = 0u;
	GameplayDataRevision pinnedRevision{};
	std::string reason;
	const VALTAN_PATTERN_FLOW_RESULT result =
		Evaluate_ValtanPatternFlowStopAfterCurrent(
			sessionId, request, roomFlowEpoch, pinnedRevision, reason);
	std::string flowRevision;
#ifdef _DEBUG
	if (VALTAN_PATTERN_FLOW_RESULT::QUEUED == result ||
		VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == result)
	{
		const auto receipt =
			m_ValtanPatternFlowControlSequenceBySessionId.find(sessionId);
		if (m_ValtanPatternFlowControlSequenceBySessionId.end() != receipt)
			flowRevision = receipt->second.strFlowRevision;
	}
#endif
	if (!Send_ValtanPatternFlowResult(
			session, request.iControlSequence,
			VALTAN_PATTERN_FLOW_COMMAND::STOP_AFTER_CURRENT, result,
			request.strFlowId, flowRevision, roomFlowEpoch,
			pinnedRevision, reason))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Build_RequiredPinnedGameplayRevisions(
	std::vector<LostArk::Shared::GameplayDataRevision>& outRevisions) const
{
	using LostArk::Shared::GameplayDataRevision;
	outRevisions.clear();
	const GameplayDataRevision& active =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!active.Is_Valid())
		return false;

	const auto append = [&outRevisions, &active](
		const GameplayDataRevision& revision)
		{
			if (!revision.Is_Valid())
				return false;
			if (revision == active || outRevisions.end() != std::find(
				outRevisions.begin(), outRevisions.end(), revision))
			{
				return true;
			}
			if (outRevisions.size() >=
				LostArk::Shared::MAX_REQUIRED_PINNED_GAMEPLAY_REVISIONS)
			{
				return false;
			}
			outRevisions.push_back(revision);
			return true;
		};

	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (!append(entity.PinnedDefinitionRevision) ||
			(entity.ProductSequencePinnedDefinitionRevision.Is_Valid() &&
			 !append(entity.ProductSequencePinnedDefinitionRevision)) ||
			(entity.PendingPatternFollowup.Is_Pending() &&
			 !append(entity.PendingPatternFollowup.PinnedDefinitionRevision)))
		{
			outRevisions.clear();
			return false;
		}
		for (const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence :
			entity.MechanicOccurrences)
		{
			if ((SERVER_BOSS_MECHANIC_STATE::QUEUED == occurrence.eState ||
				SERVER_BOSS_MECHANIC_STATE::ACTIVE == occurrence.eState) &&
				!append(occurrence.PinnedDefinitionRevision))
			{
				outRevisions.clear();
				return false;
			}
		}
	}
	for (const SERVER_COMBAT_OBJECT& object :
		m_CombatObjectRuntime.Get_LiveObjects())
	{
		if (object.bReplicated && !append(object.PinnedDefinitionRevision))
		{
			outRevisions.clear();
			return false;
		}
	}
#ifdef _DEBUG
	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE !=
			m_KoukuSaydonPatternAudition.ePhase &&
		!append(m_KoukuSaydonPatternAudition.PinnedGameplayRevision))
	{
		outRevisions.clear();
		return false;
	}
	if (Is_ValtanPatternFlowRunning() &&
		!append(m_ValtanPatternFlowAudition.PinnedDefinitionRevision))
	{
		outRevisions.clear();
		return false;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE !=
			m_ValtanPatternIdAudition.ePhase &&
		!append(m_ValtanPatternIdAudition.PinnedDefinitionRevision))
	{
		outRevisions.clear();
		return false;
	}
	if (m_ValtanNextPattern && !append(m_ValtanNextPattern->PinnedDefinitionRevision))
	{
		outRevisions.clear();
		return false;
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase &&
		!append(m_ValtanTimelineAudition.PinnedDefinitionRevision))
	{
		outRevisions.clear();
		return false;
	}
#endif
	std::sort(outRevisions.begin(), outRevisions.end(),
		[](const GameplayDataRevision& left,
			const GameplayDataRevision& right)
		{
			return left.Bytes < right.Bytes;
		});
	return true;
}

const LostArk::Server::CGameplayCatalog*
LostArk::Server::CGameRoom::Resolve_ValtanGameplayCatalog(
	const SERVER_WORLD_ENTITY& boss) const noexcept
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId)
		return m_GameplayCatalog.Resolve(boss.PinnedDefinitionRevision);
	/* Explicit auditions own their existing pins. Product keeps one catalog
	   across every step and terminal idle; only a new encounter/reset selects
	   from process-active again. Unsequenced bosses still pin each occurrence. */
	#ifdef _DEBUG
	if (Is_ValtanPatternFlowRunning() &&
		boss.iNetEntityId == m_ValtanPatternFlowAudition.iBossEntityId)
	{
		return m_GameplayCatalog.Resolve(
			m_ValtanPatternFlowAudition.PinnedDefinitionRevision);
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase &&
		boss.iNetEntityId == m_ValtanTimelineAudition.iBossEntityId)
	{
		return m_GameplayCatalog.Resolve(
			m_ValtanTimelineAudition.PinnedDefinitionRevision);
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE !=
			m_ValtanPatternIdAudition.ePhase &&
		boss.iNetEntityId == m_ValtanPatternIdAudition.iBossEntityId)
	{
		return m_GameplayCatalog.Resolve(
			m_ValtanPatternIdAudition.PinnedDefinitionRevision);
	}
	#endif
	if (boss.PendingPatternFollowup.Is_Pending())
	{
		return m_GameplayCatalog.Resolve(
			boss.PendingPatternFollowup.PinnedDefinitionRevision);
	}
	if (boss.ProductSequencePinnedDefinitionRevision.Is_Valid())
		return m_GameplayCatalog.Resolve(boss.ProductSequencePinnedDefinitionRevision);
	if (boss.strPatternId.empty())
	{
		/* The active generation owns ordinary decisions, but the oldest queued
		   health mechanic already became an occurrence under its captured
		   generation. Resolve that catalog before SelectPattern consumes its ID;
		   BeginPattern then publishes the same revision on the running boss. */
		if (!boss.PendingPatternIds.empty())
		{
			const std::string& pendingPatternId =
				boss.PendingPatternIds.front();
			const auto pendingOccurrence = std::find_if(
				boss.MechanicOccurrences.begin(),
				boss.MechanicOccurrences.end(),
				[&pendingPatternId](
					const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
				{
					return occurrence.strPatternId == pendingPatternId &&
						SERVER_BOSS_MECHANIC_STATE::QUEUED ==
							occurrence.eState;
				});
			if (boss.MechanicOccurrences.end() != pendingOccurrence &&
				pendingOccurrence->PinnedDefinitionRevision.Is_Valid())
			{
				return m_GameplayCatalog.Resolve(
					pendingOccurrence->PinnedDefinitionRevision);
			}
		}
		return &m_GameplayCatalog.Active();
	}
	return m_GameplayCatalog.Resolve(boss.PinnedDefinitionRevision);
}
