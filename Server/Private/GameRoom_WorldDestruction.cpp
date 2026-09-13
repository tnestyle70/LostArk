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

void LostArk::Server::CGameRoom::Resolve_NavigableStep(
	const CServerNavigation& navigation,
	const float fromX,
	const float fromZ,
	const float targetX,
	const float targetZ,
	float& outX,
	float& outZ)
{
	if (!navigation.Is_Loaded() ||
		!navigation.Is_PointWalkableExact(fromX, fromZ))
	{
		outX = targetX;
		outZ = targetZ;
		return;
	}
	/* Eight samples resolve a stride finer than one navigation cell at the
	   fastest authored charge speed, so the stop lands against the face rather
	   than a whole stride short of it. */
	constexpr std::uint32_t SAMPLE_COUNT = 8u;
	float reachedX = fromX;
	float reachedZ = fromZ;
	for (std::uint32_t sample = 1u; sample <= SAMPLE_COUNT; ++sample)
	{
		const float ratio = static_cast<float>(sample) /
			static_cast<float>(SAMPLE_COUNT);
		const float sampleX = fromX + (targetX - fromX) * ratio;
		const float sampleZ = fromZ + (targetZ - fromZ) * ratio;
		if (!navigation.Is_PointWalkableExact(sampleX, sampleZ))
			break;
		reachedX = sampleX;
		reachedZ = sampleZ;
	}
	outX = reachedX;
	outZ = reachedZ;
}

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Break_EveryWallForAudition(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t resetTick,
	std::string& status)
{
	struct WALL_PAIR final
	{
		WORLD_DESTRUCTION_BINDING_APPLICATION Application;
		WORLD_DESTRUCTION_STATE_TRANSITION Transition;
	};
	const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& graph =
		m_WorldDestructionBootstrap.Get_DescriptorGraph();
	const std::uint32_t epoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	std::vector<WALL_PAIR> stagedPairs;
	std::map<std::string, std::size_t> stagedIndexByGroupId;
	const auto append =
		[&stagedPairs, &stagedIndexByGroupId, epoch, resetTick](
			const WORLD_DESTRUCTION_TRANSACTION& transaction)
		{
			return Append_UniqueDestructionTransitions(
				transaction, epoch, resetTick,
				stagedPairs, stagedIndexByGroupId);
		};

	for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding : graph.Bindings)
	{
		if (WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT !=
			binding.eTriggerKind)
		{
			continue;
		}
		WORLD_DESTRUCTION_TRANSACTION contactTransaction{};
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY !=
				m_WorldDestructionRuntime.Prepare_ContactTrigger(
					binding.strImpactReceiverId, boss.iNetEntityId,
					resetTick, resetTick, contactTransaction, status) ||
			!append(contactTransaction))
		{
			status = "Audition wall clear failed on an ordinary wall: " + status;
			return false;
		}
	}

	WORLD_DESTRUCTION_ACTION_TUPLE outerAction{};
	outerAction.strPatternId = FINAL_ARENA_PATTERN_ID;
	outerAction.strStageId = FINAL_ARENA_STAGE_ID;
	outerAction.strActionId = FINAL_ARENA_ACTION_ID;
	outerAction.iStageIndex = 2u;
	WORLD_DESTRUCTION_TRANSACTION outerTransaction{};
	const std::uint32_t outerPatternSequence =
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence ?
		1u : boss.iPatternSequence + 1u;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY !=
			m_WorldDestructionRuntime.Prepare_StageTrigger(
				outerAction, boss.iNetEntityId, outerPatternSequence,
				resetTick, outerTransaction, status) ||
		!append(outerTransaction))
	{
		status = "Audition wall clear failed on the outer ring: " + status;
		return false;
	}

	std::sort(
		stagedPairs.begin(), stagedPairs.end(),
		[](const WALL_PAIR& left, const WALL_PAIR& right)
		{
			return left.Transition.strGroupId < right.Transition.strGroupId;
		});
	WORLD_DESTRUCTION_TRANSACTION wallTransaction{};
	wallTransaction.iEncounterEpoch = epoch;
	wallTransaction.iRequestTick = resetTick;
	for (WALL_PAIR& pair : stagedPairs)
	{
		wallTransaction.BindingApplications.push_back(
			std::move(pair.Application));
		wallTransaction.Transitions.push_back(std::move(pair.Transition));
	}
	return Commit_WorldDestructionTransaction(
		wallTransaction, {}, resetTick, status);
}
#endif

bool LostArk::Server::CGameRoom::Apply_WorldDestructionStageEntry(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		0u == boss.iNetEntityId || 0u == boss.iPatternSequence ||
		boss.strPatternId.empty() || boss.strPatternStageId.empty() ||
		boss.strActionId.empty())
	{
		return true;
	}

	WORLD_DESTRUCTION_ACTION_TUPLE action{};
	action.strPatternId = boss.strPatternId;
	action.strStageId = boss.strPatternStageId;
	action.strActionId = boss.strActionId;
	action.iStageIndex = boss.iPatternStageIndex;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_StageTrigger(
			action, boss.iNetEntityId, boss.iPatternSequence, serverTick,
			transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
	{
		return true;
	}
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == result &&
		!Build_WorldDestructionLiveEvents(
			transaction, boss, liveEvents, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Commit_WorldDestructionTransaction(
			transaction, liveEvents, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (!liveEvents.empty())
	{
		const std::uint64_t lastSequence = liveEvents.back().iEventSequence;
		m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
			0u : lastSequence + 1u;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionBodyContact(
	SERVER_WORLD_ENTITY& boss,
	const float previousX,
	const float previousY,
	const float previousZ,
	const std::uint32_t serverTick)
{
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		0u == boss.iNetEntityId || boss.fCollisionRadius <= 0.f ||
		0u == serverTick)
	{
		return true;
	}
	std::vector<std::string> contacts;
	m_ServerCollisionSystem.Collect_BossCircleContacts(
		previousX, previousY, previousZ,
		boss.fPositionX, boss.fPositionY, boss.fPositionZ,
		boss.fCollisionRadius, contacts);
	return Apply_WorldDestructionContacts(boss, contacts, serverTick);
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionPatternHitContact(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		!boss.bPatternWallContact || 0u == boss.iNetEntityId ||
		boss.fCollisionRadius <= 0.f || 0u == serverTick)
	{
		return true;
	}
	std::vector<std::string> contacts;
	m_ServerCollisionSystem.Collect_BossPatternHitContacts(
		boss.ePatternHitShape,
		boss.fPositionX, boss.fPositionY, boss.fPositionZ,
		boss.fYawDegrees, boss.fCollisionRadius,
		boss.fPatternHitOuterRadius, boss.fPatternHitInnerRadius,
		boss.fPatternHitAngleDegrees, boss.fPatternHitLength,
		boss.fPatternHitHalfWidth, contacts);
	return Apply_WorldDestructionContacts(boss, contacts, serverTick);
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionContacts(
	SERVER_WORLD_ENTITY& boss,
	const std::vector<std::string>& contactPlacementIds,
	const std::uint32_t serverTick)
{
	if (contactPlacementIds.empty())
		return true;

	for (const std::string& contactId : contactPlacementIds)
	{
		WORLD_DESTRUCTION_TRANSACTION transaction{};
		std::string status;
		/* The tick is the contact sequence. Destruction is one-way, so a wall
		   that is already breaking answers NO_CHANGE instead of accumulating a
		   ledger entry every tick the body stays against it. */
		const WORLD_DESTRUCTION_PREPARE_RESULT result =
			m_WorldDestructionRuntime.Prepare_ContactTrigger(
				contactId, boss.iNetEntityId, serverTick, serverTick,
				transaction, status);
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result)
			continue;
		std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
		if (!Build_WorldDestructionLiveEvents(
			transaction, boss, liveEvents, status) ||
			!Commit_WorldDestructionTransaction(
				transaction, liveEvents, serverTick, status))
		{
			/* A refused contact is isolated. Geometry fires this every tick the
			   body or axe touches something, so one rejection must not take the room
			   down with it the way an authored stage edge would. */
			m_strStatus = std::move(status);
			continue;
		}
		if (!liveEvents.empty())
		{
			const std::uint64_t lastSequence = liveEvents.back().iEventSequence;
			m_iNextWorldDestructionEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
				lastSequence : lastSequence + 1u;
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_WorldDestructionImpact(
	SERVER_WORLD_ENTITY& boss,
	const std::string& receiverPlacementId,
	const std::uint32_t serverTick,
	bool& outTriggered)
{
	outTriggered = false;
	if (!m_WorldDestructionRuntime.Is_Initialized() ||
		receiverPlacementId.empty() || 0u == boss.iNetEntityId ||
		0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strPatternStageId.empty() || boss.strActionId.empty())
	{
		return true;
	}
	WORLD_DESTRUCTION_ACTION_TUPLE action{};
	action.strPatternId = boss.strPatternId;
	action.strStageId = boss.strPatternStageId;
	action.strActionId = boss.strActionId;
	action.iStageIndex = boss.iPatternStageIndex;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_ImpactTrigger(
			action, receiverPlacementId, boss.iNetEntityId,
			boss.iPatternSequence, serverTick, transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST == result ||
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
	{
		return true;
	}
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Build_WorldDestructionLiveEvents(
			transaction, boss, liveEvents, status) ||
		!Commit_WorldDestructionTransaction(
			transaction, liveEvents, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	if (!liveEvents.empty())
	{
		const std::uint64_t lastSequence = liveEvents.back().iEventSequence;
		m_iNextWorldDestructionEventSequence =
			(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
			0u : lastSequence + 1u;
	}
	if (!CBossCombatRuntime::Publish_PatternOutcome(
		boss, BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT, serverTick))
	{
		m_strStatus = "Valtan impact could not publish WALL_CONTACT";
		return false;
	}
	outTriggered = true;
	return true;
}

bool LostArk::Server::CGameRoom::Commit_WorldDestructionTransaction(
	const WORLD_DESTRUCTION_TRANSACTION& transaction,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick,
	std::string& status)
{
	std::vector<SERVER_COLLISION_STATE_CHANGE> collisionChanges;
	std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> navigationChanges;
	Build_WorldDestructionStateChanges(
		transaction, collisionChanges, navigationChanges);

	SERVER_COLLISION_STATE_STAGE collisionStage{};
	SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
	if (!m_ServerCollisionSystem.Prepare_StateChanges(
			collisionChanges, collisionStage, status) ||
		!m_ServerNavigation.Prepare_ConditionChanges(
			navigationChanges, navigationStage, status) ||
		!m_WorldDestructionRuntime.Commit(transaction, status))
	{
		return false;
	}
	m_ServerCollisionSystem.Commit_StateChanges(std::move(collisionStage));
	m_ServerNavigation.Commit_ConditionChanges(std::move(navigationStage));
	if (!navigationChanges.empty())
		Invalidate_DynamicNavigationPaths();
	if (!Broadcast_WorldDestructionDelta(
		transaction.Transitions, liveEvents, serverTick))
	{
		status = "World destruction delta broadcast failed after commit";
		return false;
	}
	return true;
}

void LostArk::Server::CGameRoom::Invalidate_DynamicNavigationPaths()
{
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		if (!player.hasMoveGoal)
			continue;
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX, player.fPositionZ,
			player.fMoveGoalX, player.fMoveGoalZ, player.MovePath))
		{
			player.hasMoveGoal = false;
			continue;
		}
		const SERVER_NAV_POINT& goal = player.MovePath.back();
		player.fMoveGoalX = goal.x;
		player.fMoveGoalZ = goal.z;
	}
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		entity.MovePath.clear();
		entity.iMovePathIndex = 0u;
		entity.iNextPathReplanTick = m_iServerTick;
	}
}

bool LostArk::Server::CGameRoom::Build_WorldDestructionLiveEvents(
	const WORLD_DESTRUCTION_TRANSACTION& transaction,
	const SERVER_WORLD_ENTITY& boss,
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	std::string& status) const
{
	using namespace LostArk::Shared;
	liveEvents.clear();
	if (transaction.iEncounterEpoch !=
			m_WorldDestructionRuntime.Get_EncounterEpoch() ||
		0u == transaction.iRequestTick ||
		transaction.BindingApplications.size() != transaction.Transitions.size() ||
		0u == boss.iNetEntityId || !std::isfinite(boss.fPositionX) ||
		!std::isfinite(boss.fPositionY) || !std::isfinite(boss.fPositionZ) ||
		!std::isfinite(boss.fYawDegrees))
	{
		status = "World destruction live-event source is invalid";
		return false;
	}

	std::size_t eventCount = 0u;
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
		transaction.Transitions)
	{
		if (WORLD_DESTRUCTION_STATE::INTACT == transition.ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState)
		{
			++eventCount;
		}
	}
	if (eventCount > MAX_WORLD_DESTRUCTION_EVENTS ||
		(0u < eventCount &&
			(0u == m_iNextWorldDestructionEventSequence ||
			 static_cast<std::uint64_t>(eventCount - 1u) >
				(std::numeric_limits<std::uint64_t>::max)() -
				m_iNextWorldDestructionEventSequence)))
	{
		status = "World destruction live-event sequence is exhausted";
		return false;
	}

	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	const float forwardX = std::sin(yawRadians);
	const float forwardZ = std::cos(yawRadians);
	if (!std::isfinite(forwardX) || !std::isfinite(forwardZ))
	{
		status = "World destruction live-event direction is invalid";
		return false;
	}

	liveEvents.reserve(eventCount);
	std::uint64_t sequence = m_iNextWorldDestructionEventSequence;
	for (std::size_t index = 0u; index < transaction.Transitions.size(); ++index)
	{
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition =
			transaction.Transitions[index];
		const WORLD_DESTRUCTION_BINDING_APPLICATION& application =
			transaction.BindingApplications[index];
		const bool isContact =
			WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT ==
				application.eTriggerKind;
		if (application.strMutationId != transition.strMutationId ||
			application.iSourceNetEntityId != boss.iNetEntityId ||
			(isContact ?
				application.iPatternSequence != transaction.iRequestTick :
				application.iPatternSequence != boss.iPatternSequence))
		{
			liveEvents.clear();
			status = "World destruction live-event transaction is inconsistent";
			return false;
		}
		if (WORLD_DESTRUCTION_STATE::INTACT != transition.ePreviousState ||
			WORLD_DESTRUCTION_STATE::BREAKING != transition.eNextState)
		{
			continue;
		}

		WORLD_DESTRUCTION_EVENT_WIRE event{};
		event.iEventSequence = sequence++;
		event.strGroupId = transition.strGroupId;
		event.strMutationId = transition.strMutationId;
		event.strBindingId = application.strBindingId;
		event.iPatternSequence = application.iPatternSequence;
		event.iSourceNetEntityId = application.iSourceNetEntityId;
		event.iServerTick = transaction.iRequestTick;
		event.fImpactOriginX = boss.fPositionX;
		event.fImpactOriginY = boss.fPositionY;
		event.fImpactOriginZ = boss.fPositionZ;
		event.fImpactDirectionX = forwardX;
		event.fImpactDirectionY = 0.f;
		event.fImpactDirectionZ = forwardZ;
		event.iRandomSeed = Hash_DestructionEventIdentity(
			transaction.iEncounterEpoch, event.iEventSequence,
			transition, application, transaction.iRequestTick);
		liveEvents.push_back(std::move(event));
	}
	status = liveEvents.empty() ?
		"World destruction transition has no one-shot live event" :
		"World destruction one-shot live events staged";
	return true;
}

bool LostArk::Server::CGameRoom::Commit_DueWorldDestruction(
	const std::uint32_t serverTick)
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	WORLD_DESTRUCTION_TRANSACTION transaction{};
	std::string status;
	const WORLD_DESTRUCTION_PREPARE_RESULT result =
		m_WorldDestructionRuntime.Prepare_DueStateCommits(
			serverTick, transaction, status);
	if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE == result)
		return true;
	if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != result ||
		!Commit_WorldDestructionTransaction(
			transaction, {}, serverTick, status))
	{
		m_strStatus = std::move(status);
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Activate_Encounter(
	const std::string& placementId)
{
	const WORLD_BOOTSTRAP_PLACEMENT* placement = Find_Placement(placementId);
	if (nullptr == placement ||
		placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == placementId;
		});
	if (m_WorldEntities.end() != existing)
		return false;

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
		return false;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		Apply_KoukuGateEntryCard(player, m_WorldEntities.back());
	}
	return true;
}

bool LostArk::Server::CGameRoom::Spawn_Monster(
	const std::string& spawnGroupId,
	const SPAWN_GROUP_ENTRY& entry,
	const SPAWN_GROUP_ANCHOR& anchor,
	const MONSTER_RUNTIME_PROFILE& profile,
	const std::uint32_t ordinal)
{
	if (spawnGroupId.empty() ||
		entry.strArchetypeId != profile.strArchetypeId ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	SERVER_NAV_POINT projected{
		anchor.fPositionX, anchor.fPositionY, anchor.fPositionZ };
	const bool marioSource = entry.strAnchorId.starts_with("anchor.mario") &&
		spawnGroupId.starts_with("spawn.mario") && spawnGroupId.ends_with(".source");
	if (marioSource)
	{
		if (!m_ServerNavigation.Sample_Position(anchor.fPositionX, anchor.fPositionZ, projected) ||
			std::abs(projected.y - anchor.fPositionY) > .25f)
		{
			m_strStatus = "Mario source anchor floor mismatch: " + entry.strAnchorId;
			return false;
		}
	}
	else if (m_ServerNavigation.Is_Loaded() &&
		!m_ServerNavigation.Project_Point(
			anchor.fPositionX, anchor.fPositionZ, projected))
	{
		/* An anchor placed off the walkable floor used to swallow the whole
		entry: the spawn returned false, the wave never finished scheduling, and
		nothing anywhere said which anchor was at fault. */
		m_strStatus = "Spawn anchor is not on walkable ground: " +
			entry.strAnchorId + " for " + entry.strArchetypeId;
#ifdef _DEBUG
		OutputDebugStringA(("[GameRoom] " + m_strStatus + "\n").c_str());
#endif
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = m_iNextNetEntityId;
	staged.strPlacementId = spawnGroupId + "." +
		std::to_string(staged.iNetEntityId) + "." + std::to_string(ordinal);
	staged.strArchetypeId = profile.strArchetypeId;
	staged.strSpawnGroupId = spawnGroupId;
	staged.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
	staged.eAction = SERVER_ENTITY_ACTION::IDLE;
	staged.fPositionX = projected.x;
	staged.fPositionY = projected.y;
	staged.fPositionZ = projected.z;
	/* The projected anchor, not the raw authored one, so the post is a point the
	monster can actually stand on. */
	staged.fSpawnPositionX = projected.x;
	staged.fSpawnPositionY = projected.y;
	staged.fSpawnPositionZ = projected.z;
	staged.fYawDegrees = anchor.fYawDegrees;
	if (marioSource)
	{
		float bestDistance = (std::numeric_limits<float>::max)();
		for (const auto& lane : MARIO_LANES)
		{
			if (spawnGroupId != "spawn.mario" + std::to_string(lane.stage) + ".source") continue;
			const auto* arrival = Find_Placement(lane.arrival);
			const auto* exit = Find_Placement(lane.exit);
			if (!arrival || !exit || arrival->TriggerActions.size() != 1u ||
				arrival->TriggerActions.front().eKind != WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER) continue;
			const auto& start = arrival->TriggerActions.front();
			if (std::abs(start.fTargetY - projected.y) > 1.f) continue;
			const float dx = exit->fPositionX - start.fTargetX;
			const float dz = exit->fPositionZ - start.fTargetZ;
			const float length = std::hypot(dx, dz);
			if (length < .1f) continue;
			const float axisX = dx / length, axisZ = dz / length;
			const float along = (projected.x - start.fTargetX) * axisX +
				(projected.z - start.fTargetZ) * axisZ;
			const float clamped = (std::clamp)(along, 0.f, length);
			const float distance = std::hypot(projected.x - start.fTargetX - axisX * clamped,
				projected.z - start.fTargetZ - axisZ * clamped);
			if (distance >= bestDistance) continue;
			bestDistance = distance;
			staged.iMarioPatrolStage = lane.stage;
			staged.fMarioPatrolAxisX = axisX;
			staged.fMarioPatrolAxisZ = axisZ;
			staged.fMarioPatrolMinimum = (std::min)(0.f, -along);
			staged.fMarioPatrolMaximum = (std::max)(0.f, length - along);
		}
		if (!staged.iMarioPatrolStage)
		{
			m_strStatus = "Mario monster has no same-floor authored patrol lane: " + entry.strAnchorId;
			return false;
		}
		const float yaw = anchor.fYawDegrees * .0174532925f;
		staged.bMarioPatrolForward = std::sin(yaw) * staged.fMarioPatrolAxisX +
			std::cos(yaw) * staged.fMarioPatrolAxisZ >= 0.f;
	}
	staged.iCurrentHp = profile.iMaxHp;
	staged.iMaximumHp = profile.iMaxHp;
	staged.iAttackPower = profile.iAttackPower;
	staged.iDefense = profile.iDefense;
	staged.fCollisionRadius = profile.fCollisionRadius;
	staged.fEngageDistance = profile.fEngageRange;
	staged.fTargetReleaseDistance = profile.fTargetReleaseRange;
	staged.fMoveSpeed = profile.fMoveSpeed;
	staged.fTurnSpeedDegreesPerSecond =
		profile.fTurnSpeedDegreesPerSecond;
	staged.fMoveAcceleration = profile.fAcceleration;
	staged.fMoveDeceleration = profile.fDeceleration;
	staged.fArrivalSlowRadius = profile.fArrivalSlowRadius;
	staged.fAttackRange = profile.fAttackRange;
	staged.iPatternTelegraphMs = profile.iAttackWindupMs;
	staged.iPatternActiveMs = profile.iAttackActiveMs;
	staged.iPatternRecoveryMs = profile.iAttackRecoveryMs;
	staged.iDeadDespawnMs = profile.iDeadDespawnMs;
	staged.fHitKnockbackScale = profile.fHitKnockbackScale;
	staged.fAttackPushRangeM = profile.fAttackPushRangeM;
	staged.iAttackPushMs = profile.iAttackPushMs;
	staged.bAttackKnockdown = profile.bAttackKnockdown;
	staged.iAttackDownMs = profile.iAttackDownMs;
	staged.PinnedDefinitionRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!staged.PinnedDefinitionRevision.Is_Valid())
		return false;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

std::uint32_t LostArk::Server::CGameRoom::Count_SpawnGroupEntities(
	const std::string& spawnGroupId) const
{
	return static_cast<std::uint32_t>(std::count_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&spawnGroupId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
				entity.strSpawnGroupId == spawnGroupId;
		}));
}
