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

LostArk::Server::CGameRoom::CGameRoom(
	const LostArk::Shared::WORLD_ID worldId,
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration)
	: m_eWorldId(worldId)
{
	if (!LostArk::Shared::Is_Known_World_Id(worldId))
	{
		m_strStatus = "Unknown room world ID";
		return;
	}
	if (!m_WorldBootstrap.Load(worldId))
	{
		m_strStatus = m_WorldBootstrap.Get_Status();
		return;
	}
	if ((nullptr != initialGameplayGeneration &&
			!m_GameplayCatalog.Initialize(initialGameplayGeneration)) ||
		(nullptr == initialGameplayGeneration && !m_GameplayCatalog.Load()))
	{
		m_strStatus = m_GameplayCatalog.Get_Status();
		return;
	}
	if (!m_ItemCatalog.Load())
	{
		m_strStatus = m_ItemCatalog.Get_Status();
		return;
	}
	if (!m_ValtanClearRewards.Load())
	{
		m_strStatus = m_ValtanClearRewards.Get_Status();
		return;
	}
	if (!m_SpawnGroupBootstrap.Load(worldId))
	{
		m_strStatus = m_SpawnGroupBootstrap.Get_Status();
		return;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, m_strStatus))
		return;
	m_EstherSkillSystem.Initialize(worldId);
	/* Bern joins the areas that require navigation. Without a grid the room keeps
	the spawn height for the whole session and straight-line XZ movement walks
	through the castle stairs, so a missing or malformed grid fails admission here
	instead of degrading silently. */
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::BERN == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (!m_NpcBehaviorRuntime.Validate_Admission(
		m_WorldBootstrap.Get_Placements(), m_ServerNavigation, m_strStatus))
	{
		return;
	}
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus,
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId))
	{
		return;
	}
	if (!m_ServerCollisionSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
			!m_ServerCollisionSystem.Is_PlayerSpawnClear(placement))
		{
			m_strStatus = "Player spawn overlaps a collision box: " +
				placement.strPlacementId;
			return;
		}
	}
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId)
	{
		if (!m_WorldDestructionBootstrap.Load_ValtanArena())
		{
			m_strStatus = m_WorldDestructionBootstrap.Get_Status();
			return;
		}
		if (!m_WorldDestructionRuntime.Initialize(
			m_WorldDestructionBootstrap.Get_DescriptorGraph(),
			m_strStatus, 1u))
		{
			return;
		}
		/* The stele slots are repeatable presentation state whose Deploy
		occurrences stay hidden on the Client until the state becomes INTACT.
		The authored set now carries the cover circle each raised slot owns, and
		its position is published from the same Deploy placement the Client
		renders, so the two can never describe different ground. */
		std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR> propSets;
		if (!Load_EncounterPropSets(m_eWorldId, propSets, m_strStatus))
			return;
		if (!propSets.empty())
		{
			if (propSets.size() != 1u ||
				!m_EncounterPropRuntime.Initialize(
					propSets.front(), m_strStatus, 1u))
			{
				m_strStatus =
					"World declares an encounter prop set this room cannot own";
				return;
			}
		}
		std::set<std::string> voidConditionIds;
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if ((!mutation.strCollisionStateId.empty() &&
				 !m_ServerCollisionSystem.Has_CollisionStateTarget(
					 mutation.strCollisionStateId)) ||
				(!mutation.strNavigationStateId.empty() &&
				 !m_ServerNavigation.Has_Condition(
					 mutation.strNavigationStateId)))
			{
				m_strStatus = "World destruction dynamic state reference is unknown: " +
					mutation.strMutationId;
				return;
			}
			/* The floor sectors are the only mutations that take ground away.
			Navigation needs their conditions before any of them can flip, so the
			set is handed over here and never rebuilt during combat. */
			if (mutation.bRemovesGround)
				voidConditionIds.insert(mutation.strNavigationStateId);
		}
		if (!m_ServerNavigation.Set_VoidConditions(
			voidConditionIds, m_strStatus))
		{
			return;
		}
	}
	if (!Initialize_WorldEntities())
		return;
	if (nullptr == Find_AvailablePlayerSpawn())
	{
		m_strStatus = "World bootstrap has no enabled player spawn";
		return;
	}

	m_isReady = true;
	m_strStatus = m_WorldBootstrap.Get_Status();
}

bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	return Is_AcceptedRoomCommandEnqueueResult(
		Enqueue_Detailed(std::move(command)));
}

LostArk::Server::ROOM_COMMAND_ENQUEUE_RESULT
LostArk::Server::CGameRoom::Enqueue_Detailed(ROOM_COMMAND command)
{
	if (command.iSessionId == INVALID_SESSION_ID)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	}

	std::scoped_lock lock{ m_CommandMutex };
	if (!m_isReady)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY;
	if (!m_acceptsCommands)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_SEALED;

	if (ROOM_COMMAND_TYPE::LEAVE == command.eType)
	{
		/* A disconnect must never compete with gameplay traffic for ingress
		   capacity. Keep one pending/in-flight cleanup per session and cancel
		   any commands that would otherwise run after that session leaves. */
		if (!m_QueuedCleanupSessionIds.insert(command.iSessionId).second)
		{
			++m_PerformanceMetrics.iDeduplicatedCleanupCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP;
		}
		const std::size_t oldCommandCount = m_InboundCommands.size();
		std::erase_if(
			m_InboundCommands,
			[sessionId = command.iSessionId](const ROOM_COMMAND& queued)
			{
				return queued.iSessionId == sessionId;
			});
		m_PerformanceMetrics.iCancelledCommandCountByCleanup +=
			oldCommandCount - m_InboundCommands.size();
		m_CleanupCommands.push_back(std::move(command));
		m_PerformanceMetrics.iCleanupIngressHighWatermark = (std::max)(
			m_PerformanceMetrics.iCleanupIngressHighWatermark,
			m_CleanupCommands.size());
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
	}
	if (m_QueuedCleanupSessionIds.contains(command.iSessionId))
	{
		// Do not let receive traffic reappear behind pending/in-flight cleanup.
		++m_PerformanceMetrics.iCancelledCommandCountByCleanup;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP;
	}

	if (Is_BestEffortCommand(command.eType))
	{
		if (Try_RemoveCoalescedCommand(m_InboundCommands, command))
		{
			if (ROOM_COMMAND_TYPE::MOVE == command.eType)
				++m_PerformanceMetrics.iCoalescedMoveCommandCount;
			else
				++m_PerformanceMetrics.iCoalescedAimCommandCount;
			m_InboundCommands.push_back(std::move(command));
			return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
		}
		if (m_InboundCommands.size() >= MAX_BEST_EFFORT_COMMAND_COUNT)
		{
			++m_PerformanceMetrics.iDroppedBestEffortCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT;
		}
	}
	else if (m_InboundCommands.size() >= MAX_RELIABLE_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedReliableCommandCount;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY;
	}

	m_InboundCommands.push_back(std::move(command));
	m_PerformanceMetrics.iIngressHighWatermark = (std::max)(
		m_PerformanceMetrics.iIngressHighWatermark,
		m_InboundCommands.size());
	return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
}

std::string LostArk::Server::CGameRoom::Describe_EnqueueResult(
	const ROOM_COMMAND_ENQUEUE_RESULT result) const
{
	std::scoped_lock lock{ m_CommandMutex };
	std::string description =
		"enqueueResult=" + std::string{ To_RoomCommandEnqueueResultName(result) } +
		" worldId=" + std::to_string(static_cast<std::uint16_t>(m_eWorldId)) +
		" ingressDepth=" + std::to_string(m_InboundCommands.size()) +
		" reliableCapacity=" + std::to_string(MAX_RELIABLE_COMMAND_COUNT) +
		" acceptsCommands=" + (m_acceptsCommands ? "true" : "false");
	if (!m_isReady)
		description += " roomReady=false";
	if (!m_RuntimeFailure.strSource.empty())
	{
		description +=
			" firstFailureTick=" +
			std::to_string(m_RuntimeFailure.iServerTick) +
			" firstFailureSource=" + m_RuntimeFailure.strSource +
			" firstFailureDetail=" + m_RuntimeFailure.strDetail;
	}
	return description;
}

bool LostArk::Server::CGameRoom::Try_GetRuntimeFailure(
	SERVER_ROOM_RUNTIME_FAILURE& outFailure) const
{
	std::scoped_lock lock{ m_CommandMutex };
	if (m_RuntimeFailure.strSource.empty())
		return false;
	outFailure = m_RuntimeFailure;
	return true;
}

void LostArk::Server::CGameRoom::Mark_RuntimeFailure(
	const std::string_view source)
{
	SERVER_ROOM_RUNTIME_FAILURE failure{};
	{
		std::scoped_lock lock{ m_CommandMutex };
		if (!m_RuntimeFailure.strSource.empty())
		{
			m_isReady = false;
			return;
		}
		m_RuntimeFailure.iServerTick = m_iServerTick;
		m_RuntimeFailure.strSource = source.empty() ?
			"unspecified-room-runtime-failure" : std::string{ source };
		m_RuntimeFailure.strDetail = m_strStatus.empty() ?
			m_RuntimeFailure.strSource : m_strStatus;
		m_isReady = false;
		failure = m_RuntimeFailure;
	}
	std::cerr << "[RoomRuntimeFailure] World=" <<
		static_cast<std::uint16_t>(m_eWorldId) <<
		" Tick=" << failure.iServerTick <<
		" Source=" << failure.strSource <<
		" Detail=" << failure.strDetail << '\n';
}

LostArk::Server::SERVER_ROOM_PERFORMANCE_METRICS
LostArk::Server::CGameRoom::Get_PerformanceMetrics() const
{
	std::scoped_lock lock{ m_CommandMutex };
	return m_PerformanceMetrics;
}

bool LostArk::Server::CGameRoom::Build_ValtanDecisionTraceResponse(
	const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
	LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
	std::string& status) const
{
	using namespace LostArk::Shared;
	S2C_VALTAN_DECISION_TRACE_RESPONSE staged{};
	staged.iRequestSequence = request.iRequestSequence;
	staged.strBossPlacementId = request.strBossPlacementId;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
	{
		staged.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	const auto bossIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				request.strBossPlacementId == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	if (m_WorldEntities.end() == bossIter)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_NO_BOSS;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	const VALTAN_DECISION_TRACE* trace =
		m_ValtanBrain.Get_LatestDecisionTrace();
	if (nullptr == trace)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::NO_TRACE;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->iTraceSequence <= request.iAfterTraceSequence)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->Candidates.size() > MAX_VALTAN_DECISION_TRACE_CANDIDATES ||
		m_ValtanDecisionTraceRevision.iBossEntityId != bossIter->iNetEntityId ||
		m_ValtanDecisionTraceRevision.strBossPlacementId !=
			bossIter->strPlacementId ||
		m_ValtanDecisionTraceRevision.iTraceSequence != trace->iTraceSequence ||
		!m_ValtanDecisionTraceRevision.DefinitionRevision.Is_Valid())
	{
		/* The trace is a self-contained immutable decision envelope. Its
		   DefinitionRevision is an identity for observability, not a live
		   gameplay lookup pin; keep the latest trace queryable after that old
		   generation is collected. */
		status = "Valtan decision trace revision metadata is invalid";
		return false;
	}

	const auto mapSource = [](const VALTAN_DECISION_SOURCE source,
		VALTAN_DECISION_TRACE_SOURCE& wire)
	{
		switch (source)
		{
		case VALTAN_DECISION_SOURCE::NONE:
			wire = VALTAN_DECISION_TRACE_SOURCE::NONE; return true;
		case VALTAN_DECISION_SOURCE::INTRO:
			wire = VALTAN_DECISION_TRACE_SOURCE::INTRO; return true;
		case VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_HEALTH_BAR; return true;
		case VALTAN_DECISION_SOURCE::FORCED_AUDITION:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_AUDITION; return true;
		case VALTAN_DECISION_SOURCE::ORDERED:
			wire = VALTAN_DECISION_TRACE_SOURCE::ORDERED; return true;
		case VALTAN_DECISION_SOURCE::WEIGHTED:
			wire = VALTAN_DECISION_TRACE_SOURCE::WEIGHTED; return true;
		case VALTAN_DECISION_SOURCE::GLOBAL:
			wire = VALTAN_DECISION_TRACE_SOURCE::GLOBAL; return true;
		default:
			return false;
		}
	};
	const auto mapResult = [](const VALTAN_DECISION_RESULT result,
		VALTAN_DECISION_TRACE_RESULT& wire)
	{
		switch (result)
		{
		case VALTAN_DECISION_RESULT::SELECTED:
			wire = VALTAN_DECISION_TRACE_RESULT::SELECTED; return true;
		case VALTAN_DECISION_RESULT::WAITING_FOR_INTRO_RANGE:
			wire = VALTAN_DECISION_TRACE_RESULT::WAITING_FOR_INTRO_RANGE;
			return true;
		case VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_ELIGIBLE_PATTERN;
			return true;
		case VALTAN_DECISION_RESULT::NO_VALID_TARGET:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_VALID_TARGET; return true;
		case VALTAN_DECISION_RESULT::CATALOG_UNAVAILABLE:
			wire = VALTAN_DECISION_TRACE_RESULT::CATALOG_UNAVAILABLE;
			return true;
		case VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED:
			wire = VALTAN_DECISION_TRACE_RESULT::MECHANIC_RESET_REQUIRED;
			return true;
		default:
			return false;
		}
	};
	const auto mapExclusions = [](const std::uint32_t source,
		std::uint32_t& wire)
	{
		struct BIT_MAP final { std::uint32_t Source; std::uint32_t Wire; };
		static constexpr BIT_MAP MAP[] =
		{
			{ VALTAN_EXCLUDE_WRONG_SELECTION_KIND,
				VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND },
			{ VALTAN_EXCLUDE_INTRO_ROW,
				VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW },
			{ VALTAN_EXCLUDE_NOT_IN_SELECTION_SET,
				VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET },
			{ VALTAN_EXCLUDE_ARMOR_MISMATCH,
				VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH },
			{ VALTAN_EXCLUDE_PHASE_REQUIREMENT,
				VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT },
			{ VALTAN_EXCLUDE_PHASE_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE },
			{ VALTAN_EXCLUDE_HEALTH_BAR_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE },
			{ VALTAN_EXCLUDE_NO_TARGET,
				VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET },
			{ VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE },
			{ VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE },
			{ VALTAN_EXCLUDE_COOLDOWN,
				VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN },
			{ VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED,
				VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED },
			{ VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED,
				VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED },
			{ VALTAN_EXCLUDE_DISABLED,
				VALTAN_DECISION_TRACE_EXCLUDE_DISABLED },
			{ VALTAN_EXCLUDE_UNRESOLVED_DEFINITION,
				VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION }
		};
		std::uint32_t known = 0u;
		wire = VALTAN_DECISION_TRACE_EXCLUDE_NONE;
		for (const BIT_MAP& bit : MAP)
		{
			known |= bit.Source;
			if (0u != (source & bit.Source))
				wire |= bit.Wire;
		}
		return 0u == (source & ~known) &&
			0u == (wire & ~VALTAN_DECISION_TRACE_KNOWN_EXCLUSION_MASK);
	};

	VALTAN_DECISION_TRACE_WIRE& wire = staged.Trace;
	wire.iTraceSequence = trace->iTraceSequence;
	wire.iServerTick = trace->iServerTick;
	wire.iPatternSequenceBeforeDecision =
		trace->iPatternSequenceBeforeDecision;
	wire.iExpectedPatternSequence = trace->iExpectedPatternSequence;
	wire.iCurrentHp = trace->iCurrentHp;
	wire.iMaximumHp = trace->iMaximumHp;
	wire.iHealthBar = trace->iHealthBar;
	wire.iGameplayPhase = trace->iGameplayPhase;
	wire.iTargetNetEntityId = trace->iTargetNetEntityId;
	wire.fTargetDistance = trace->fTargetDistance;
	wire.isIntroPatternConsumed = trace->bIntroPatternConsumed;
	wire.iRotationStepIndex = trace->iRotationStepIndex;
	wire.strRotationId = trace->strRotationId;
	wire.strPendingPatternId = trace->strPendingPatternId;
	wire.strSelectedPatternId = trace->strSelectedPatternId;
	wire.iRawRandomInput = trace->iRawRandomInput;
	wire.iMixedRandomValue = trace->iMixedRandomValue;
	wire.iTotalWeight = trace->iTotalWeight;
	wire.iRandomTicket = trace->iRandomTicket;
	wire.isMaximumConsecutiveRelaxed = trace->bMaximumConsecutiveRelaxed;
	wire.areCandidatesTruncated = trace->bCandidatesTruncated;
	if (!mapSource(trace->eSource, wire.eSource) ||
		!mapSource(trace->ePendingSource, wire.ePendingSource) ||
		!mapResult(trace->eResult, wire.eResult))
	{
		status = "Valtan decision trace contains an unknown selector enum";
		return false;
	}
	wire.Candidates.reserve(trace->Candidates.size());
	for (const VALTAN_DECISION_CANDIDATE_TRACE& source : trace->Candidates)
	{
		VALTAN_DECISION_TRACE_CANDIDATE_WIRE candidate{};
		candidate.strPatternId = source.strPatternId;
		if (!mapExclusions(source.iExclusionMask, candidate.iExclusionMask))
		{
			status = "Valtan decision trace contains an unknown exclusion bit";
			return false;
		}
		candidate.iAuthoredWeight = source.iAuthoredWeight;
		candidate.iEffectiveWeight = source.iEffectiveWeight;
		candidate.iCooldownRemainingTicks = source.iCooldownRemainingTicks;
		candidate.iConsecutiveUses = source.iConsecutiveUses;
		candidate.iMaximumConsecutiveUses = source.iMaximumConsecutiveUses;
		candidate.iWeightBeginInclusive = source.iWeightBeginInclusive;
		candidate.iWeightEndExclusive = source.iWeightEndExclusive;
		candidate.isSelected = source.bSelected;
		wire.Candidates.push_back(std::move(candidate));
	}
	staged.DefinitionRevision =
		m_ValtanDecisionTraceRevision.DefinitionRevision;
	staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE;
	outResponse = std::move(staged);
	status.clear();
	return true;
}

bool LostArk::Server::CGameRoom::Stage_GameplayGeneration(
	const std::uint32_t transactionSequence,
	const LostArk::Shared::GameplayDataRevision& baseRevision,
	const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
	std::string& status)
{
	std::vector<LostArk::Shared::GameplayDataRevision> livePins;
	if (!Build_RequiredPinnedGameplayRevisions(livePins))
	{
		status = "Room required gameplay revision pins are invalid";
		return false;
	}
	m_GameplayCatalog.Collect_Garbage(livePins);
	return m_GameplayCatalog.Stage(
		transactionSequence, baseRevision, candidateGeneration, status);
}

bool LostArk::Server::CGameRoom::Commit_GameplayGeneration(
	const std::uint32_t transactionSequence) noexcept
{
	if (!m_GameplayCatalog.Commit(transactionSequence))
		return false;
	const LostArk::Shared::GameplayDataRevision& activeRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		/* A boss keeps the last generation its brain evaluated until the first
		   tick that evaluates the new active catalog. That one-tick identity handoff
		   is what lets the brain reconcile a newly introduced or raised health
		   threshold after the boss is already below it. Running occurrences keep
		   the same pin for their full lifetime; definition-self-contained entities
		   can publish the new active identity immediately. */
		if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind)
		{
			entity.PinnedDefinitionRevision = activeRevision;
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Abort_GameplayGeneration(
	const std::uint32_t transactionSequence) noexcept
{
	m_GameplayCatalog.Abort(transactionSequence);
}

bool LostArk::Server::CGameRoom::Try_SealPrivateArenaForRetirement()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return false;

	// Gameplay containers are room-thread-owned. The mutex makes the empty
	// decision atomic against every receive-thread Enqueue call.
	std::scoped_lock lock{ m_CommandMutex };
	if (!m_acceptsCommands)
		return true;
	if (!m_InboundCommands.empty() ||
		!m_CleanupCommands.empty() ||
		!m_QueuedCleanupSessionIds.empty() ||
		!m_PendingWorldTransfers.empty() ||
		!m_Sessions.empty() ||
		!m_Players.empty() ||
		!m_PlayerIdBySessionId.empty() ||
		!m_PlayerIdByEntityId.empty())
	{
		return false;
	}
	m_acceptsCommands = false;
	return true;
}

bool LostArk::Server::CGameRoom::Commit_WorldTransferDeparture(
	const SESSION_ID sessionId)
{
	if (!m_isReady || !m_PlayerIdBySessionId.contains(sessionId))
		return false;

	// CServerApp invokes this on the room thread after staging the target
	// REGISTER/ENTER commands. The target cannot process them until this call
	// has cleared the old CClientSession player binding.
	Leave(
		sessionId,
		LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
	return !m_PlayerIdBySessionId.contains(sessionId);
}

bool LostArk::Server::CGameRoom::Try_DequeueWorldTransfer(
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	if (m_PendingWorldTransfers.empty())
		return false;
	outTransfer = std::move(m_PendingWorldTransfers.front());
	m_PendingWorldTransfers.pop_front();
	return true;
}

void LostArk::Server::CGameRoom::Tick(const float fixedDeltaSeconds)
{
	if (!m_isReady)
		return;
	const auto tickStart = std::chrono::steady_clock::now();
	const auto recordTickDuration = [this, tickStart]()
		{
			const std::uint64_t elapsedMicroseconds = To_Microseconds(
				std::chrono::steady_clock::now() - tickStart);
			const auto navigationMetrics = m_ServerNavigation.Get_PerformanceMetrics();
			std::scoped_lock lock{ m_CommandMutex };
			m_PerformanceMetrics.Navigation = navigationMetrics;
			++m_PerformanceMetrics.iTickCount;
			m_PerformanceMetrics.iLastTickMicroseconds = elapsedMicroseconds;
			m_PerformanceMetrics.iMaximumTickMicroseconds = (std::max)(
				m_PerformanceMetrics.iMaximumTickMicroseconds,
				elapsedMicroseconds);
		};

	std::deque<ROOM_COMMAND> cleanupCommands;
	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
		const std::size_t cleanupIngressDepth = m_CleanupCommands.size();
		cleanupCommands.swap(m_CleanupCommands);
		const std::size_t ingressDepth = m_InboundCommands.size();
		const std::size_t drainCount = (std::min)(
			ingressDepth, MAX_COMMANDS_DRAINED_PER_TICK);
		for (std::size_t index = 0u; index < drainCount; ++index)
		{
			commands.push_back(std::move(m_InboundCommands.front()));
			m_InboundCommands.pop_front();
		}
		m_PerformanceMetrics.iLastIngressDepth = ingressDepth;
		m_PerformanceMetrics.iLastDrainedCommandCount = drainCount;
		m_PerformanceMetrics.iLastRemainingCommandCount =
			m_InboundCommands.size();
		m_PerformanceMetrics.iLastCleanupIngressDepth = cleanupIngressDepth;
		m_PerformanceMetrics.iLastDrainedCleanupCommandCount =
			cleanupCommands.size();
		m_PerformanceMetrics.iLastRemainingCleanupCommandCount =
			m_CleanupCommands.size();
		if (!m_InboundCommands.empty())
			++m_PerformanceMetrics.iDrainLimitedTickCount;
	}

	// Cleanup is independent of and always precedes the bounded gameplay drain.
	for (ROOM_COMMAND& command : cleanupCommands)
	{
		Leave(command.iSessionId, command.eLeaveReason);
		std::scoped_lock lock{ m_CommandMutex };
		m_QueuedCleanupSessionIds.erase(command.iSessionId);
	}

	for (ROOM_COMMAND& command : commands)
	{
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld,
				command.strSpawnPlacementOverrideId, command.CarriedInventory);
			break;
		case ROOM_COMMAND_TYPE::MOVE:
			Handle_Move(command.iSessionId, command.Move);
			break;
		case ROOM_COMMAND_TYPE::USE_SKILL:
			Handle_UseSkill(command.iSessionId, command.UseSkill);
			break;
		case ROOM_COMMAND_TYPE::RELEASE_SKILL:
			Handle_ReleaseSkill(command.iSessionId, command.ReleaseSkill);
			break;
		case ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM:
			Handle_UpdateSkillAim(command.iSessionId, command.UpdateSkillAim);
			break;
		case ROOM_COMMAND_TYPE::USE_ESTHER_SKILL:
			Handle_UseEstherSkill(command.iSessionId, command.UseEstherSkill);
			break;
		case ROOM_COMMAND_TYPE::REVIVE_PLAYER:
			Handle_RevivePlayer(command.iSessionId, command.RevivePlayer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_KILL_SELF:
			Handle_DebugKillSelf(command.iSessionId, command.DebugKillSelf);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_ENTER_KAKULSAYDON_ARENA:
			Handle_DebugEnterKakulSaydonArena(
				command.iSessionId, command.DebugEnterKakulSaydonArena);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_PLACEMENT:
			Handle_DebugTeleportToPlacement(
				command.iSessionId, command.DebugTeleportToPlacement);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_POSITION:
			Handle_DebugTeleportToPosition(
				command.iSessionId, command.DebugTeleportToPosition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_MARIO_JUMP:
			Handle_DebugMarioJump(command.iSessionId, command.DebugMarioJump);
			break;
		case ROOM_COMMAND_TYPE::MARIO_MOVE:
			Handle_MarioMove(command.iSessionId, command.MarioMove);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_FILL:
			Handle_DebugBingoFill(command.iSessionId, command.DebugBingoFill);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_BOMB:
			Handle_DebugBingoBomb(command.iSessionId, command.DebugBingoBomb);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_HAMMER:
			Handle_DebugBingoHammer(command.iSessionId, command.DebugBingoHammer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM:
			Handle_DebugSetMadnessForm(
				command.iSessionId, command.DebugSetMadnessForm);
			break;
		case ROOM_COMMAND_TYPE::INTERACTION_SLOT:
			Handle_InteractionSlot(command.iSessionId, command.InteractionSlot);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_KOUKU_HUD_MODE:
			Handle_DebugSetKoukuHudMode(
				command.iSessionId, command.DebugSetKoukuHudMode);
			break;
		case ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS:
			Handle_ChangeCharacterClass(
				command.iSessionId, command.ChangeCharacterClass);
			break;
		case ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY:
			Handle_SpawnWorldEntity(
				command.iSessionId,
				command.SpawnWorldEntity);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_AUDITION:
			Handle_ValtanAudition(
				command.iSessionId,
				command.ValtanAudition);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_START:
			Handle_ValtanPatternFlowStart(
				command.iSessionId, command.ValtanPatternFlowStart);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT:
			Handle_ValtanPatternFlowStopAfterCurrent(
				command.iSessionId,
				command.ValtanPatternFlowStopAfterCurrent);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_PATTERN_AUDITION:
			Handle_KoukuSaydonPatternAudition(
				command.iSessionId, command.KoukuSaydonPatternAudition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM:
			Handle_DebugGiveItem(command.iSessionId, command.DebugGiveItem);
			break;
		case ROOM_COMMAND_TYPE::USE_ITEM:
			Handle_UseItem(command.iSessionId, command.UseItem);
			break;
		case ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES:
			Handle_DespawnAllWorldEntities(
				command.iSessionId, command.DespawnAllWorldEntities);
			break;
		case ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY:
			Handle_ConfirmNpcEntry(
				command.iSessionId, command.ConfirmNpcEntry);
			break;
		case ROOM_COMMAND_TYPE::INTERACT_TRIGGER:
			Handle_InteractTrigger(
				command.iSessionId, command.InteractTrigger);
			break;
		case ROOM_COMMAND_TYPE::RETURN_TO_BERN:
			Handle_ReturnToBern(
				command.iSessionId, command.ReturnToBern);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_WORLD_PLAYBACK:
			Handle_DebugWorldPlayback(command.iSessionId, command.DebugWorldPlayback);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE:
			Handle_PartyInvite(command.iSessionId, command.PartyInvite);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE_RESPOND:
			Handle_PartyInviteRespond(
				command.iSessionId, command.PartyInviteRespond);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_PROPOSE:
			Handle_RaidEntryPropose(
				command.iSessionId, command.RaidEntryPropose);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_RESPOND:
			Handle_RaidEntryRespond(
				command.iSessionId, command.RaidEntryRespond);
			break;
		case ROOM_COMMAND_TYPE::CHAT:
			Handle_Chat(command.iSessionId, command.Chat);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			// LEAVE is routed exclusively through m_CleanupCommands.
			break;
		}
	}

	Flush_PartyTransferResults();
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		recordTickDuration();
		return;
	}

	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
#ifdef _DEBUG
	// WORLD supports and their first pattern tick must exist before any walking query.
	Prepare_KoukuAuditionTick(updateTick);
	if (!Refresh_KoukuSupportSurfaces(updateTick)) { recordTickDuration(); return; }
#endif
	Refresh_PlayerBlockingBodies();
	for (const auto& [id, player] : m_Players)
		if (player.eCardMazeRole != LostArk::Shared::CARD_MAZE_ROLE::NONE)
			m_CardMazePreviousPositions[id] = {player.fPositionX, player.fPositionZ};
	Update_Players(fixedDeltaSeconds);
	Update_CardMaze(updateTick);
	Update_KoukuBingo(updateTick);
	m_CombatObjectRuntime.Update(
		m_Players, m_WorldEntities, m_GameplayCatalog,
		fixedDeltaSeconds, updateTick, m_TickDamageEvents);
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
	bool evaluatePlayerTriggers = true;
#ifdef _DEBUG
	evaluatePlayerTriggers = WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
			m_ValtanTimelineAudition.ePhase;
#endif
	if (evaluatePlayerTriggers)
	{
		m_ServerTriggerSystem.Evaluate_Entries(
		m_Players,
		updateTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind)
			{
				/* Presentation has no Server state to commit, so the entry
				   itself is the whole result: tell the room and report the
				   action handled. */
				Broadcast_WorldSequencePlay(targetId);
				return true;
			}
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
				return m_SpawnGroupRuntime.Activate(targetId);
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
			{
				const bool activated = Activate_Encounter(targetId);
#ifdef _DEBUG
				if (activated && WORLD_ID::VALTAN_ARENA == m_eWorldId &&
					"boss.valtan.center" == targetId)
				{
					SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
					if (nullptr == boss)
						return false;
					const std::uint32_t openingHp =
						CValtanBrain::Resolve_HealthBarHp(*boss, 159u);
					if (0u == openingHp)
						return false;
					/* The next normal brain tick observes only 160 -> 159 and
					queues the real opening wall charge. Nothing here plays a
					camera, breaks a wall or emits a Client cue directly. */
					boss->iCurrentHp = openingHp;
					boss->iLastEvaluatedHealthBar = 160u;
				}
#endif
				return activated;
			}
			return false;
		},
		promptEdges);
	}
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		Update_MarioControlState(player);
	}
	for (const SERVER_INTERACT_PROMPT_EDGE& edge : promptEdges)
		Send_InteractPrompt(edge);
	Cleanup_EmptyMarioStages();
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[sessionId = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sessionId;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	m_SpawnGroupRuntime.Update(
		fixedDeltaSeconds,
		m_SpawnGroupBootstrap,
		[this](const std::string& spawnGroupId)
		{
			return Count_SpawnGroupEntities(spawnGroupId);
		},
		[this](const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			const std::uint32_t ordinal)
		{
			return Spawn_Monster(
				spawnGroupId, entry, anchor, profile, ordinal);
		});
	m_EstherSkillSystem.Update(fixedDeltaSeconds, !m_Players.empty());
	Update_WorldEntities(fixedDeltaSeconds);
#ifdef _DEBUG
	// An owner may have completed or aborted during the boss update this tick.
	if (!Refresh_KoukuSupportSurfaces(updateTick)) { recordTickDuration(); return; }
#endif
	Update_KoukuPlayerModes(updateTick);
	if (!m_isReady)
	{
		recordTickDuration();
		return;
	}
	/* Boss root motion is resolved after the ordinary player update. Refresh a
	grabbed fallback once more at that committed pose so the snapshot never
	trails its attachment owner by one fixed tick. */
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == player.eAction)
			(void)Update_PlayerAttachment(player, updateTick);
	}
	if (!Commit_DueEncounterProps(updateTick) ||
		!Commit_DueWorldDestruction(updateTick))
	{
		Mark_RuntimeFailure("fixed-tick.due-world-transaction");
		recordTickDuration();
		return;
	}
	Drain_BossCombatEvents();
#ifdef _DEBUG
	if (!Flush_KoukuSaydonPatternAuditionLifecycle())
	{
		m_strStatus = "KoukuSaydon pattern audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.koukusaydon-audition-lifecycle");
		recordTickDuration();
		return;
	}
	// Current completion and Next promotion observe the final committed tick.
	// A promoted ID cannot reach BeginPattern until the next world update.
	(void)Refresh_ValtanPatternIdAuditionState();
	if (!Flush_ValtanPatternIdAuditionLifecycle())
	{
		m_strStatus = "Valtan audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-audition-lifecycle");
		recordTickDuration();
		return;
	}
	if (!Flush_ValtanPatternFlowLifecycle())
	{
		m_strStatus = "Valtan pattern-flow lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-flow-lifecycle");
		recordTickDuration();
		return;
	}
#endif
	if (!Broadcast_CombatObjectLifecycle())
	{
		Mark_RuntimeFailure("fixed-tick.combat-object-lifecycle");
		recordTickDuration();
		return;
	}
	m_iServerTick = updateTick;
	Expire_RaidEntryProposals();
	if (!m_Players.empty())
		Broadcast_WorldSnapshot();
	std::vector<LostArk::Shared::GameplayDataRevision> liveGenerationPins;
	if (!Build_RequiredPinnedGameplayRevisions(liveGenerationPins))
	{
		m_strStatus = "Gameplay generation pin set exceeded its wire bound";
		Mark_RuntimeFailure("fixed-tick.gameplay-generation-pins");
		recordTickDuration();
		return;
	}
	m_GameplayCatalog.Collect_Garbage(liveGenerationPins);
	recordTickDuration();
	if (!m_Players.empty() && 0u == (m_iServerTick % 300u))
	{
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			Get_PerformanceMetrics();
		std::size_t maximumCurrentOutboundFrames = 0u;
		std::size_t maximumOutboundFrameHighWatermark = 0u;
		std::uint64_t snapshotCoalescedCount = 0u;
		std::uint64_t snapshotDroppedCount = 0u;
		std::uint64_t reliableRejectedCount = 0u;
		std::uint64_t sendFailureCount = 0u;
		std::uint64_t maximumWireSendMicroseconds = 0u;
		for (const auto& [sessionId, weakSession] : m_Sessions)
		{
			(void)sessionId;
			const std::shared_ptr<CClientSession> session = weakSession.lock();
			if (nullptr == session)
				continue;
			const CLIENT_SESSION_OUTBOUND_METRICS sessionMetrics =
				session->Get_OutboundMetrics();
			maximumCurrentOutboundFrames = (std::max)(
				maximumCurrentOutboundFrames,
				sessionMetrics.iCurrentQueuedFrameCount);
			maximumOutboundFrameHighWatermark = (std::max)(
				maximumOutboundFrameHighWatermark,
				sessionMetrics.iQueuedFrameHighWatermark);
			snapshotCoalescedCount +=
				sessionMetrics.iSnapshotCoalescedFrameCount;
			snapshotDroppedCount +=
				sessionMetrics.iSnapshotDroppedFrameCount;
			reliableRejectedCount +=
				sessionMetrics.iReliableRejectedFrameCount;
			sendFailureCount += sessionMetrics.iSendFailureCount;
			maximumWireSendMicroseconds = (std::max)(
				maximumWireSendMicroseconds,
				sessionMetrics.iMaximumFrameSendMicroseconds);
		}
		const bool hasNewFailure =
			metrics.iDrainLimitedTickCount >
				m_LastRoomPerfLogSample.iDrainLimitedTickCount ||
			metrics.iDroppedBestEffortCommandCount >
				m_LastRoomPerfLogSample.iDroppedBestEffortCommandCount ||
			metrics.iRejectedReliableCommandCount >
				m_LastRoomPerfLogSample.iRejectedReliableCommandCount ||
			metrics.iRejectedCleanupCommandCount >
				m_LastRoomPerfLogSample.iRejectedCleanupCommandCount ||
			metrics.iSnapshotEncodeFailureCount >
				m_LastRoomPerfLogSample.iSnapshotEncodeFailureCount ||
			metrics.iSnapshotEnqueueFailureCount >
				m_LastRoomPerfLogSample.iSnapshotEnqueueFailureCount ||
			snapshotDroppedCount > m_iLastRoomPerfSnapshotDroppedCount ||
			reliableRejectedCount > m_iLastRoomPerfReliableRejectedCount ||
			sendFailureCount > m_iLastRoomPerfWireSendFailureCount;
		const bool hasCurrentPressure =
			metrics.iLastTickMicroseconds >= 33333u ||
			(metrics.iMaximumTickMicroseconds >= 33333u &&
				metrics.iMaximumTickMicroseconds >
					m_LastRoomPerfLogSample.iMaximumTickMicroseconds) ||
			0u != metrics.iLastRemainingCommandCount ||
			0u != metrics.iLastRemainingCleanupCommandCount ||
			maximumCurrentOutboundFrames >= 64u ||
			(maximumOutboundFrameHighWatermark >= 64u &&
				maximumOutboundFrameHighWatermark >
					m_iLastRoomPerfOutboundHighWatermark);
		const bool isHeartbeat = 0u == (m_iServerTick % 1800u);
		m_LastRoomPerfLogSample = metrics;
		m_iLastRoomPerfSnapshotDroppedCount = snapshotDroppedCount;
		m_iLastRoomPerfReliableRejectedCount = reliableRejectedCount;
		m_iLastRoomPerfWireSendFailureCount = sendFailureCount;
		m_iLastRoomPerfOutboundHighWatermark =
			maximumOutboundFrameHighWatermark;
		if (!hasNewFailure && !hasCurrentPressure && !isHeartbeat)
			return;

		std::cout << "[RoomPerf] Kind="
			<< (hasNewFailure || hasCurrentPressure ? "anomaly" : "heartbeat")
			<< " World=" << static_cast<unsigned>(m_eWorldId)
			<< " Tick=" << m_iServerTick
			<< " TickUs=" << metrics.iLastTickMicroseconds
			<< " TickMaxUs=" << metrics.iMaximumTickMicroseconds
			<< " Ingress=" << metrics.iLastIngressDepth
			<< " IngressHigh=" << metrics.iIngressHighWatermark
			<< " Drained=" << metrics.iLastDrainedCommandCount
			<< " Remaining=" << metrics.iLastRemainingCommandCount
			<< " CleanupIngress=" << metrics.iLastCleanupIngressDepth
			<< " CleanupIngressHigh=" << metrics.iCleanupIngressHighWatermark
			<< " CleanupDrained="
			<< metrics.iLastDrainedCleanupCommandCount
			<< " CleanupRemaining="
			<< metrics.iLastRemainingCleanupCommandCount
			<< " CleanupDeduplicated="
			<< metrics.iDeduplicatedCleanupCommandCount
			<< " CleanupCancelledCommands="
			<< metrics.iCancelledCommandCountByCleanup
			<< " MoveCoalesced=" << metrics.iCoalescedMoveCommandCount
			<< " AimCoalesced=" << metrics.iCoalescedAimCommandCount
			<< " BestEffortDropped="
			<< metrics.iDroppedBestEffortCommandCount
			<< " ReliableRejected="
			<< metrics.iRejectedReliableCommandCount
			<< " SnapshotEncodeUs="
			<< metrics.iLastSnapshotEncodeMicroseconds
			<< " SnapshotEnqueueUs="
			<< metrics.iLastSnapshotEnqueueMicroseconds
			<< " SessionEnqueueMaxUs="
			<< metrics.iMaximumSessionEnqueueMicroseconds
			<< " SnapshotEnqueueFailures="
			<< metrics.iSnapshotEnqueueFailureCount
			<< " OutboundQueuedMax=" << maximumCurrentOutboundFrames
			<< " OutboundHighMax=" << maximumOutboundFrameHighWatermark
			<< " SnapshotCoalesced=" << snapshotCoalescedCount
			<< " SnapshotDropped=" << snapshotDroppedCount
			<< " OutboundReliableRejected=" << reliableRejectedCount
			<< " WireSendMaxUs=" << maximumWireSendMicroseconds
			<< " WireSendFailures=" << sendFailureCount;
		const auto writeNavigation = [](const char* name,
			const SERVER_NAVIGATION_QUERY_METRICS& stage)
			{
				std::cout << " Nav" << name << "Calls=" << stage.iCalls
					<< " Nav" << name << "TotalUs=" << stage.iTotalNanoseconds / 1000u
					<< " Nav" << name << "MaxUs=" << stage.iMaximumNanoseconds / 1000u
					<< " Nav" << name << "Expanded=" << stage.iExpandedNodes
					<< " Nav" << name << "PathPoints=" << stage.iReturnedPathPoints;
			};
		writeNavigation("FindPath", metrics.Navigation.FindPath);
		writeNavigation("ReachablePath", metrics.Navigation.ReachablePath);
		writeNavigation("ProjectPoint", metrics.Navigation.ProjectPoint);
		writeNavigation("SmoothPath", metrics.Navigation.SmoothPath);
		writeNavigation("TraversalStep", metrics.Navigation.TraversalStep);
		writeNavigation("LineOfSight", metrics.Navigation.LineOfSight);
		std::cout << '\n';
	}
}
