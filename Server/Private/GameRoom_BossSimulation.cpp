#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/KoukuTargetTracking.h"

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

bool LostArk::Server::CGameRoom::Activate_ValtanGhostPhaseLoop(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
		INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId ||
		"BOSS_VALTAN" != boss.strArchetypeId ||
		"boss.valtan.center" != boss.strPlacementId ||
		3u != boss.iPhase || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		m_strStatus = "Valtan ghost phase activation owner is invalid";
		return false;
	}
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Valtan ghost phase loop definition is unavailable";
		return false;
	}
	const auto finale = std::find_if(
		patterns->begin(), patterns->end(),
		[](const BOSS_PATTERN_DEFINITION& definition)
		{ return "VALTAN_GHOST_FINALE" == definition.strPatternId; });
	if (patterns->end() == finale ||
		BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != finale->Finale.eKind ||
		6u != finale->Finale.GhostPatternIds.size())
	{
		m_strStatus = "Valtan ghost phase loop definition is unavailable";
		return false;
	}
	BOSS_PATTERN_SEQUENCE_DEFINITION sequence{};
	sequence.strEncounterId = boss.strEncounterId;
	sequence.strSequenceId = "sequence.valtan.ghost-phase.primary-loop";
	sequence.eMode = BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
	sequence.iExpectedStepCount =
		static_cast<std::uint32_t>(finale->Finale.GhostPatternIds.size());
	sequence.PatternIds = finale->Finale.GhostPatternIds;
	boss.GhostPhasePatternSequence = std::move(sequence);
	boss.bGhostPhasePatternLoopActive = true;
	boss.iGhostAuxiliaryOccurrenceSequence = 0u;
	boss.iGhostAuxiliaryNextSpawnTick = 0u;
	boss.strRotationId.clear();
	boss.iRotationStepIndex = 0u;
	boss.bAutomaticPatternSequenceStepRunning = false;
	boss.iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
	boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
	boss.iGhostPortalLastSpawnTick = 0u;
	boss.iGhostPortalOccurrenceSequence = 0u;
	Clear_ValtanGhostRelocationState(boss);
	boss.iGhostRelocationSequence = 0u;
	return true;
}

bool LostArk::Server::CGameRoom::Begin_ValtanGhostRelocation(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (!boss.bGhostPhasePatternLoopActive || boss.bGhostRepositionPending ||
		WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
		INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId ||
		"BOSS_VALTAN" != boss.strArchetypeId ||
		"boss.valtan.center" != boss.strPlacementId ||
		3u != boss.iPhase || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction ||
		0u == serverTick || !boss.strPatternId.empty() ||
		(boss.bGhostRelocationRetryPending &&
		 !Has_ReachedServerTick(serverTick, boss.iGhostRelocationRetryTick)))
	{
		m_strStatus = "Valtan ghost relocation owner is invalid";
		return false;
	}
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	const auto finale = nullptr == patterns ? nullptr :
		[patterns]() -> const BOSS_PATTERN_DEFINITION*
		{
			const auto found = std::find_if(
				patterns->begin(), patterns->end(),
				[](const BOSS_PATTERN_DEFINITION& definition)
				{ return "VALTAN_GHOST_FINALE" == definition.strPatternId; });
			return found == patterns->end() ? nullptr : &*found;
		}();
	const BOSS_RUNTIME_PROFILE* profile = catalog.Find_Boss(boss.strArchetypeId);
	if (nullptr == finale ||
		BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != finale->Finale.eKind ||
		6u != finale->Finale.GhostPatternIds.size() || nullptr == profile ||
		!std::isfinite(finale->Finale.fSpawnHalfExtentsX) ||
		!std::isfinite(finale->Finale.fSpawnHalfExtentsZ) ||
		finale->Finale.fSpawnHalfExtentsX <= 0.f ||
		finale->Finale.fSpawnHalfExtentsZ <= 0.f ||
		!std::isfinite(profile->fCollisionRadius) ||
		profile->fCollisionRadius <= 0.f)
	{
		m_strStatus = "Valtan ghost relocation definition is unavailable";
		return false;
	}
	if (CBossCombatRuntime::Has_Flag(
			boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) ||
		CBossCombatRuntime::Has_Flag(
			boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE))
	{
		m_strStatus = "Valtan ghost relocation inherited an unclosed combat flag";
		return false;
	}

	const auto hasSpawnClearance = [this, &boss, profile](
		const SERVER_NAV_POINT& center)
	{
		/* A random centre is admitted only when the entire boss footprint remains
		on the same walkable deck. This is the same half-cell conservative sampling
		used by the former dependent-ghost path. */
		const float radius = profile->fCollisionRadius;
		const float spacing = (std::max)(
			0.05f, m_ServerNavigation.Get_CellSize() * 0.5f);
		const std::uint32_t segments = (std::max)(1u,
			static_cast<std::uint32_t>(std::ceil(2.f * radius / spacing)));
		for (std::uint32_t row = 0u; row <= segments; ++row)
		{
			for (std::uint32_t column = 0u; column <= segments; ++column)
			{
				const float x = center.x - radius +
					2.f * radius * column / segments;
				const float z = center.z - radius +
					2.f * radius * row / segments;
				SERVER_NAV_POINT ground{};
				if (!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
					!m_ServerNavigation.Sample_Position(x, z, ground) ||
					std::fabs(ground.y - boss.fSpawnPositionY) > 1.5f)
				{
					return false;
				}
			}
		}
		return true;
	};

	std::uint32_t relocationSequence =
		(std::numeric_limits<std::uint32_t>::max)() ==
			boss.iGhostRelocationSequence ?
			1u : boss.iGhostRelocationSequence + 1u;
	if (0u == relocationSequence)
		relocationSequence = 1u;
	const std::uint64_t seed = Mix_DeterministicRandom(
		(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
		static_cast<std::uint64_t>(relocationSequence) ^
		static_cast<std::uint64_t>(serverTick));
	SERVER_NAV_POINT spawn{};
	bool foundSpawn = false;
	for (std::uint32_t attempt = 0u; attempt < 128u && !foundSpawn; ++attempt)
	{
		const float x = boss.fSpawnPositionX +
			(2.f * DeterministicUnitFloat(seed + attempt * 2u) - 1.f) *
				finale->Finale.fSpawnHalfExtentsX;
		const float z = boss.fSpawnPositionZ +
			(2.f * DeterministicUnitFloat(seed + attempt * 2u + 1u) - 1.f) *
				finale->Finale.fSpawnHalfExtentsZ;
		foundSpawn = m_ServerNavigation.Is_PointWalkableExact(x, z) &&
			m_ServerNavigation.Sample_Position(x, z, spawn) &&
			std::fabs(spawn.y - boss.fSpawnPositionY) <= 1.5f &&
			hasSpawnClearance(spawn);
	}
	if (!foundSpawn)
	{
		/* The arena can be changing on the same fixed tick as an attack finishes.
		A bounded random miss is therefore not room corruption. Preserve a valid
		current footprint, or move to the immutable encounter anchor only when that
		anchor validates, then retry with the next tick in the deterministic seed. */
		const SERVER_NAV_POINT currentPose{
			boss.fPositionX, boss.fPositionY, boss.fPositionZ };
		SERVER_NAV_POINT currentGround{};
		const bool currentPoseValid = std::isfinite(currentPose.x) &&
			std::isfinite(currentPose.y) && std::isfinite(currentPose.z) &&
			m_ServerNavigation.Is_PointWalkableExact(
				currentPose.x, currentPose.z) &&
			m_ServerNavigation.Sample_Position(
				currentPose.x, currentPose.z, currentGround) &&
			std::fabs(currentPose.y - currentGround.y) <= 1.5f &&
			std::fabs(currentGround.y - boss.fSpawnPositionY) <= 1.5f &&
			hasSpawnClearance(currentGround);
		SERVER_NAV_POINT fallback = currentPose;
		bool fallbackValid = currentPoseValid;
		bool fallbackRequiresCommit = false;
		if (!fallbackValid)
		{
			fallback = {
				boss.fSpawnPositionX, boss.fSpawnPositionY,
				boss.fSpawnPositionZ };
			fallbackValid = std::isfinite(fallback.x) &&
				std::isfinite(fallback.y) && std::isfinite(fallback.z) &&
				m_ServerNavigation.Is_PointWalkableExact(fallback.x, fallback.z) &&
				m_ServerNavigation.Sample_Position(
					fallback.x, fallback.z, fallback) &&
				std::fabs(fallback.y - boss.fSpawnPositionY) <= 1.5f &&
				hasSpawnClearance(fallback);
			fallbackRequiresCommit = fallbackValid;
		}
		if (fallbackRequiresCommit)
		{
			boss.fPositionX = fallback.x;
			boss.fPositionY = fallback.y;
			boss.fPositionZ = fallback.z;
		}
		boss.bGhostRelocationRetryPending = true;
		boss.iGhostRelocationRetryTick =
			Add_ServerTicksSkippingReservedZero(serverTick, 1u);
		boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
		m_strStatus = fallbackValid ?
			"Valtan ghost relocation retry pending from a validated pose" :
			"Valtan ghost relocation retry pending while preserving its current pose";
		return true;
	}

	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	if (!CBossCombatRuntime::Set_Flag(
			stagedCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true) ||
		!CBossCombatRuntime::Set_Flag(
			stagedCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, true))
	{
		m_strStatus = "Valtan ghost relocation flags could not be staged";
		return false;
	}

	/* Commit pose, replication flags, and the one-tick latch together. The
	immutable spawn pose remains the portal-triangle centre. */
	boss.fPositionX = spawn.x;
	boss.fPositionY = spawn.y;
	boss.fPositionZ = spawn.z;
	boss.BossCombat = std::move(stagedCombat);
	boss.bGhostRepositionPending = true;
	boss.iGhostReappearTick = Add_ServerTicksSkippingReservedZero(serverTick, 1u);
	boss.iGhostRelocationSequence = relocationSequence;
	boss.bGhostRelocationRetryPending = false;
	boss.iGhostRelocationRetryTick = 0u;
	boss.bGhostRelocationOwnsHiddenFlag = true;
	boss.bGhostRelocationOwnsInvulnerableFlag = true;
	boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
	return true;
}

bool LostArk::Server::CGameRoom::Update_ValtanGhostPortalScheduler(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	constexpr std::uint32_t PORTAL_OCCURRENCE_INTERVAL_MS = 7900u;
	constexpr std::uint32_t PORTAL_RUNNER_START_DELAY_MS = 300u;
	constexpr float TRIANGLE_CIRCUMRADIUS_M = 13.5f;
	constexpr float TRIANGLE_EDGE_LENGTH_M = 23.3826859022f;
	constexpr float PORTAL_RUNNER_SPEED_MPS = 17.9866814632f;
	constexpr float TRIANGLE_START_ANGLE_DEGREES = 30.f;
	constexpr float TRIANGLE_ANGLE_STEP_DEGREES = 120.f;
	if (!boss.bGhostPhasePatternLoopActive)
		return true;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
		INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId ||
		"BOSS_VALTAN" != boss.strArchetypeId || 3u != boss.iPhase ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		return true;
	}
	if (0u != boss.iGhostPortalLastSpawnTick &&
		Elapsed_ServerTicksSkippingReservedZero(
			boss.iGhostPortalLastSpawnTick, serverTick) <
			DurationMillisecondsToServerTicks(PORTAL_OCCURRENCE_INTERVAL_MS))
	{
		return true;
	}
	const bool runnerStillActive = std::any_of(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&boss](const SERVER_WORLD_ENTITY& candidate)
		{
			return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
					candidate.eDependentBossRole &&
				candidate.iOwnerBossNetEntityId == boss.iNetEntityId;
		});
	if (runnerStillActive)
		return true;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Valtan ghost portal occurrence definition is unavailable";
		return false;
	}
	const auto portal = std::find_if(
		patterns->begin(), patterns->end(),
		[](const BOSS_PATTERN_DEFINITION& definition)
		{ return "VALTAN_GHOST_PORTAL_ONCE" == definition.strPatternId; });
	if (patterns->end() == portal ||
		1u != portal->Stages.size() ||
		"valtan.ghost.portal-once.active" != portal->Stages.front().strActionId ||
		1900u != portal->Stages.front().iDurationMs)
	{
		m_strStatus = "Valtan ghost portal occurrence definition is unavailable";
		return false;
	}
	std::uint32_t occurrenceSequence =
		(std::numeric_limits<std::uint32_t>::max)() ==
			boss.iGhostPortalOccurrenceSequence ?
			1u : boss.iGhostPortalOccurrenceSequence + 1u;
	if (0u == occurrenceSequence)
		occurrenceSequence = 1u;
	/* Validate and stage the authored world-space geometry before opening the
	combat-object transaction. Like VALTAN_WARP, neither the proxy nor its visible
	runner is a navigation-walking actor: an exact triangle edge may cross missing
	navigation. All three retain the immutable encounter-anchor Y. */
	std::array<SERVER_NAV_POINT, 3u> vertices{};
	for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
	{
		const float degrees = TRIANGLE_START_ANGLE_DEGREES +
			TRIANGLE_ANGLE_STEP_DEGREES * static_cast<float>(ordinal);
		const float radians = degrees * DEGREES_TO_RADIANS;
		const float x = boss.fSpawnPositionX +
			std::sin(radians) * TRIANGLE_CIRCUMRADIUS_M;
		const float z = boss.fSpawnPositionZ +
			std::cos(radians) * TRIANGLE_CIRCUMRADIUS_M;
		if (!std::isfinite(x) || !std::isfinite(boss.fSpawnPositionY) ||
			!std::isfinite(z))
		{
			m_strStatus = "Valtan ghost portal triangle vertex is not finite";
			return false;
		}
		vertices[ordinal] = { x, boss.fSpawnPositionY, z };
	}
	for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
	{
		const SERVER_NAV_POINT& start = vertices[ordinal];
		const SERVER_NAV_POINT& end =
			vertices[(ordinal + 1u) % vertices.size()];
		const float routeLength = std::hypot(end.x - start.x, end.z - start.z);
		if (!std::isfinite(routeLength) ||
			std::fabs(routeLength - TRIANGLE_EDGE_LENGTH_M) > 0.001f)
		{
			m_strStatus = "Valtan ghost portal triangle edge geometry is invalid";
			return false;
		}
	}
	std::vector<SERVER_WORLD_ENTITY> stagedRunners;
	stagedRunners.reserve(vertices.size());
	NET_ENTITY_ID nextId = m_iNextNetEntityId;
	for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
	{
		if (INVALID_NET_ENTITY_ID == nextId)
		{
			m_strStatus = "Portal runner entity ID space is exhausted";
			return false;
		}
		const SERVER_NAV_POINT& start = vertices[ordinal];
		const SERVER_NAV_POINT& end =
			vertices[(ordinal + 1u) % vertices.size()];
		const float yawDegrees =
			std::atan2(end.x - start.x, end.z - start.z) *
			RADIANS_TO_DEGREES;
		WORLD_BOOTSTRAP_PLACEMENT placement{};
		placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		placement.strPlacementId = boss.strPlacementId + ".portal-runner." +
			std::to_string(occurrenceSequence) + "." +
			std::to_string(ordinal);
		placement.strArchetypeId = "BOSS_VALTAN_GHOST";
		placement.strEncounterId = boss.strEncounterId;
		/* Build at the admitted encounter anchor, then commit the exact edge pose.
		This prevents generic boss construction from projecting a runner vertex
		back onto navigation. */
		placement.fPositionX = boss.fSpawnPositionX;
		placement.fPositionY = boss.fSpawnPositionY;
		placement.fPositionZ = boss.fSpawnPositionZ;
		placement.fYawDegrees = yawDegrees;
		SERVER_WORLD_ENTITY runner{};
		if (!Build_WorldEntity(
			placement, nextId, runner, &catalog, boss.iNetEntityId))
		{
			return false;
		}
		runner.eDependentBossRole =
			SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER;
		runner.fPositionX = runner.fSpawnPositionX = start.x;
		runner.fPositionY = runner.fSpawnPositionY = start.y;
		runner.fPositionZ = runner.fSpawnPositionZ = start.z;
		runner.fYawDegrees = yawDegrees;
		runner.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		runner.strPatternId = portal->strPatternId;
		runner.strPatternStageId = portal->Stages.front().strStageId;
		runner.strActionId = portal->Stages.front().strActionId;
		runner.iActionStartTick = serverTick;
		runner.iPatternSequence = occurrenceSequence;
		runner.iPatternStageIndex = 0u;
		runner.iPatternStageDurationMs = portal->Stages.front().iDurationMs;
		runner.iPatternStageFirstEvaluationTick = serverTick;
		runner.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
		runner.ProductSequencePinnedDefinitionRevision =
			runner.PinnedDefinitionRevision;
		runner.bIntroPatternConsumed = true;
		runner.iPhase = boss.iPhase;
		runner.bPortalMotionActive = true;
		runner.bPortalRushTargetLocked = true;
		runner.iPortalRushRetargetDelayMs =
			PORTAL_RUNNER_START_DELAY_MS;
		runner.fPortalRushSpeedMps = PORTAL_RUNNER_SPEED_MPS;
		runner.fPortalRushDistanceM = TRIANGLE_EDGE_LENGTH_M;
		runner.fPortalStartX = start.x;
		runner.fPortalStartZ = start.z;
		runner.fPortalEndX = end.x;
		runner.fPortalEndZ = end.z;
		runner.fPortalLastHitSampleX = start.x;
		runner.fPortalLastHitSampleZ = start.z;
		runner.ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
		std::vector<std::uint8_t> payload;
		if (!Build_WorldEntitySpawnedPayload(runner, payload))
		{
			m_strStatus = "Portal runner spawn wire admission failed";
			return false;
		}
		stagedRunners.push_back(std::move(runner));
		++nextId;
	}
	const NET_ENTITY_ID ownerId = boss.iNetEntityId;
	SERVER_WORLD_ENTITY synthetic = boss;
	synthetic.strPatternId = portal->strPatternId;
	synthetic.strPatternStageId = portal->Stages.front().strStageId;
	synthetic.strActionId = portal->Stages.front().strActionId;
	synthetic.iPatternSequence = occurrenceSequence;
	synthetic.iPatternStageIndex = 0u;
	synthetic.iActionStartTick = serverTick;
	synthetic.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
	synthetic.ProductSequencePinnedDefinitionRevision = catalog.Get_ActiveRevision();
	/* One atomic radial volley owns the three simultaneous edge damage proxies.
	The primary and auxiliary ghost action clocks remain independent. */
	synthetic.fPositionX = boss.fSpawnPositionX;
	synthetic.fPositionY = boss.fSpawnPositionY;
	synthetic.fPositionZ = boss.fSpawnPositionZ;
	synthetic.fYawDegrees = 0.f;
	/* Reserve before the central volley is committed. The owner is re-resolved
	by stable ID after these three preflighted appends, so vector relocation
	cannot turn one occurrence into a portal-only partial spawn. */
	m_WorldEntities.reserve(m_WorldEntities.size() + stagedRunners.size());
	if (!Apply_BossPatternStageActions(
		synthetic, portal->strPatternId, portal->Stages.front().strActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick))
	{
		return false;
	}
	for (SERVER_WORLD_ENTITY& runner : stagedRunners)
	{
		m_WorldEntities.push_back(std::move(runner));
		Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	}
	m_iNextNetEntityId = nextId;
	const auto committedOwner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerId](const SERVER_WORLD_ENTITY& candidate)
		{ return candidate.iNetEntityId == ownerId; });
	if (m_WorldEntities.end() == committedOwner)
	{
		m_strStatus = "Portal runner owner disappeared during commit";
		return false;
	}
	committedOwner->iGhostPortalOccurrenceSequence = occurrenceSequence;
	committedOwner->iGhostPortalLastSpawnTick = serverTick;
	return true;
}

bool LostArk::Server::CGameRoom::Update_DependentBosses(const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	constexpr std::uint32_t PORTAL_RUNNER_START_DELAY_MS = 300u;
	constexpr std::uint32_t PORTAL_RUNNER_TRAVEL_MS = 1300u;
	constexpr std::uint32_t PORTAL_RUNNER_DESPAWN_MS = 1600u;
	const auto finaleOf = [this](const SERVER_WORLD_ENTITY& owner)
		-> const BOSS_PATTERN_FINALE*
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS != owner.eKind ||
			INVALID_NET_ENTITY_ID != owner.iOwnerBossNetEntityId ||
			0u == owner.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == owner.eAction ||
			owner.bMechanicLedgerRequiresReset)
			return nullptr;
		const bool phaseThreeFinaleController =
			owner.bGhostPhasePatternLoopActive && 3u == owner.iPhase &&
			"BOSS_VALTAN" == owner.strArchetypeId &&
			"boss.valtan.center" == owner.strPlacementId;
		if (!phaseThreeFinaleController && owner.strPatternId.empty())
			return nullptr;
		const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(owner);
		const auto* patterns = nullptr == catalog ? nullptr :
			catalog->Find_BossPatterns(owner.strEncounterId);
		if (nullptr == patterns)
			return nullptr;
		const std::string finalePatternId = phaseThreeFinaleController ?
			std::string("VALTAN_GHOST_FINALE") : owner.strPatternId;
		const auto pattern = std::find_if(patterns->begin(), patterns->end(),
			[&finalePatternId](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == finalePatternId; });
		return pattern != patterns->end() &&
			BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern->Finale.eKind ?
			&pattern->Finale : nullptr;
	};
	for (auto child = m_WorldEntities.begin(); child != m_WorldEntities.end();)
	{
		if (INVALID_NET_ENTITY_ID == child->iOwnerBossNetEntityId || child->bKoukuGazeClone)
		{
			++child;
			continue;
		}
		auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&child](const SERVER_WORLD_ENTITY& candidate)
			{ return candidate.iNetEntityId == child->iOwnerBossNetEntityId; });
		const bool ownerLive = owner != m_WorldEntities.end() &&
			nullptr != finaleOf(*owner) && owner->strEncounterId == child->strEncounterId;
		if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
			child->eDependentBossRole)
		{
			const std::uint64_t elapsedTicks =
				Elapsed_ServerTicksSkippingReservedZero(
					child->iActionStartTick, serverTick);
			/* Keep the exact 1600 ms endpoint alive for one authoritative snapshot.
			The Client's portal-route presentation hides the runner at that same
			visual boundary; the following fixed tick only retires wire identity. */
			if (ownerLive &&
				elapsedTicks <=
					DurationMillisecondsToServerTicks(PORTAL_RUNNER_DESPAWN_MS))
			{
				const float elapsedMs = static_cast<float>(elapsedTicks) *
					1000.f / static_cast<float>(SERVER_TICK_HZ);
				const float routeRatio = std::clamp(
					(elapsedMs - static_cast<float>(PORTAL_RUNNER_START_DELAY_MS)) /
						static_cast<float>(PORTAL_RUNNER_TRAVEL_MS),
					0.f, 1.f);
				child->fPositionX = child->fPortalStartX +
					(child->fPortalEndX - child->fPortalStartX) * routeRatio;
				child->fPositionY = child->fSpawnPositionY;
				child->fPositionZ = child->fPortalStartZ +
					(child->fPortalEndZ - child->fPortalStartZ) * routeRatio;
				child->fActionElapsedSeconds = elapsedMs / 1000.f;
				++child;
				continue;
			}
			m_CombatObjectRuntime.Cancel_Source(child->iNetEntityId);
			if (!Broadcast_CombatObjectLifecycle())
				return false;
			Broadcast_WorldEntityDespawned(child->iNetEntityId);
			child = m_WorldEntities.erase(child);
			continue;
		}
		if (SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY !=
			child->eDependentBossRole)
		{
			m_strStatus = "Dependent boss runtime role is invalid";
			return false;
		}
		const bool finished = child->strPatternId.empty() &&
			child->iRotationStepIndex >= child->DependentPatternSequence.PatternIds.size();
		if (ownerLive && child->bMechanicLedgerRequiresReset)
		{
			m_strStatus = "Dependent boss attack failed and requires an encounter reset";
			return false;
		}
		if (ownerLive && !finished && 0u != child->iCurrentHp &&
			!child->bMechanicLedgerRequiresReset)
		{
			++child;
			continue;
		}
		if (ownerLive)
		{
			/* Make despawn and replacement distinct fixed-tick edges. The due tick
			belongs only to the auxiliary lane and never stalls the primary loop. */
			owner->iGhostAuxiliaryNextSpawnTick =
				Add_ServerTicksSkippingReservedZero(serverTick, 1u);
		}
		m_CombatObjectRuntime.Cancel_Source(child->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
			return false;
		Broadcast_WorldEntityDespawned(child->iNetEntityId);
		child = m_WorldEntities.erase(child);
	}
	/* Portal occurrences append three runners, so schedule them only from this
	iterator-free seam. Re-resolve the owner by stable ID after every call because
	the scheduler reserves and may relocate the world-entity vector. */
	std::vector<NET_ENTITY_ID> portalOwnerIds;
	for (const SERVER_WORLD_ENTITY& candidate : m_WorldEntities)
	{
		if (candidate.bGhostPhasePatternLoopActive &&
			INVALID_NET_ENTITY_ID == candidate.iOwnerBossNetEntityId)
		{
			portalOwnerIds.push_back(candidate.iNetEntityId);
		}
	}
	for (const NET_ENTITY_ID ownerId : portalOwnerIds)
	{
		auto owner = std::find_if(
			m_WorldEntities.begin(), m_WorldEntities.end(),
			[ownerId](const SERVER_WORLD_ENTITY& candidate)
			{ return candidate.iNetEntityId == ownerId; });
		if (m_WorldEntities.end() == owner)
			continue;
		const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(*owner);
		if (nullptr == catalog ||
			!Update_ValtanGhostPortalScheduler(*owner, *catalog, serverTick))
		{
			return false;
		}
	}
	std::vector<SERVER_WORLD_ENTITY> stagedGhosts;
	NET_ENTITY_ID nextId = m_iNextNetEntityId;
	for (SERVER_WORLD_ENTITY& owner : m_WorldEntities)
	{
		const BOSS_PATTERN_FINALE* finale = finaleOf(owner);
		if (nullptr == finale || m_Players.empty())
			continue;
		if (0u != owner.iGhostAuxiliaryNextSpawnTick &&
			!Has_ReachedServerTick(serverTick,
				owner.iGhostAuxiliaryNextSpawnTick))
		{
			continue;
		}
		const auto activeCount = std::count_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&owner](const SERVER_WORLD_ENTITY& child)
			{
				return SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY ==
						child.eDependentBossRole &&
					child.iOwnerBossNetEntityId == owner.iNetEntityId;
			});
		if (activeCount >= finale->iMaximumActiveGhosts)
			continue;
		if (INVALID_NET_ENTITY_ID == nextId)
		{
			m_strStatus = "Dependent boss entity ID space is exhausted";
			return false;
		}
		const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(owner);
		if (nullptr == catalog)
			return false;
		const BOSS_RUNTIME_PROFILE* ghostProfile =
			catalog->Find_Boss(finale->strGhostArchetypeId);
		if (nullptr == ghostProfile)
		{
			m_strStatus = "Ghost finale profile disappeared from its pinned generation";
			return false;
		}
		const std::uint32_t auxiliaryOccurrenceSequence =
			(std::numeric_limits<std::uint32_t>::max)() ==
				owner.iGhostAuxiliaryOccurrenceSequence ?
			1u : owner.iGhostAuxiliaryOccurrenceSequence + 1u;
		const std::uint64_t auxiliaryIdentity =
			(static_cast<std::uint64_t>(owner.iNetEntityId) << 32u) ^
			static_cast<std::uint64_t>(auxiliaryOccurrenceSequence);
		const std::uint64_t skillSeed = Mix_DeterministicRandom(
			auxiliaryIdentity ^ Hash_StableId(
				"valtan.ghost-phase.auxiliary.skill"));
		const std::string& selectedPatternId = finale->GhostPatternIds[
			static_cast<std::size_t>(skillSeed % finale->GhostPatternIds.size())];
		const auto hasSpawnClearance = [this, &owner, ghostProfile](const SERVER_NAV_POINT& center)
		{
			/* Conservatively cover the entire body square, including the circle
			boundary. Half-cell spacing cannot skip a one-cell navigation hole. */
			const float radius = ghostProfile->fCollisionRadius;
			const float spacing = (std::max)(0.05f, m_ServerNavigation.Get_CellSize() * 0.5f);
			const std::uint32_t segments = (std::max)(1u,
				static_cast<std::uint32_t>(std::ceil(2.f * radius / spacing)));
			for (std::uint32_t row = 0u; row <= segments; ++row)
			{
				for (std::uint32_t column = 0u; column <= segments; ++column)
				{
					const float x = center.x - radius + 2.f * radius * column / segments;
					const float z = center.z - radius + 2.f * radius * row / segments;
					SERVER_NAV_POINT ground{};
					if (!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
						!m_ServerNavigation.Sample_Position(x, z, ground) ||
						std::fabs(ground.y - owner.fSpawnPositionY) > 1.5f)
						return false;
				}
			}
			return true;
		};
		SERVER_NAV_POINT spawn{};
		bool foundSpawn = false;
		const std::uint64_t seed = Mix_DeterministicRandom(
			auxiliaryIdentity ^ Hash_StableId(
				"valtan.ghost-phase.auxiliary.spawn"));
		for (std::uint32_t attempt = 0u; attempt < 128u && !foundSpawn; ++attempt)
		{
			const float x = owner.fSpawnPositionX +
				(2.f * DeterministicUnitFloat(seed + attempt * 2u) - 1.f) *
					finale->fSpawnHalfExtentsX;
			const float z = owner.fSpawnPositionZ +
				(2.f * DeterministicUnitFloat(seed + attempt * 2u + 1u) - 1.f) *
					finale->fSpawnHalfExtentsZ;
			foundSpawn = m_ServerNavigation.Is_PointWalkableExact(x, z) &&
				m_ServerNavigation.Sample_Position(x, z, spawn) &&
				std::fabs(spawn.y - owner.fSpawnPositionY) <= 1.5f &&
				hasSpawnClearance(spawn);
		}
		if (!foundSpawn)
		{
			/* Dynamic destruction can temporarily remove every candidate. Preserve
			the occurrence identity and entity ID, then retry the same deterministic
			candidate set on the next fixed tick without failing the room. */
			owner.iGhostAuxiliaryNextSpawnTick =
				Add_ServerTicksSkippingReservedZero(serverTick, 1u);
			m_strStatus =
				"Ghost finale auxiliary spawn retry deferred: no live same-deck footprint";
			continue;
		}
		WORLD_BOOTSTRAP_PLACEMENT placement{};
		placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		placement.strPlacementId = owner.strPlacementId + ".ghost." + std::to_string(nextId);
		placement.strArchetypeId = finale->strGhostArchetypeId;
		placement.strEncounterId = owner.strEncounterId;
		placement.fPositionX = spawn.x;
		placement.fPositionY = spawn.y;
		placement.fPositionZ = spawn.z;
		placement.fYawDegrees = owner.fYawDegrees;
		SERVER_WORLD_ENTITY child{};
		if (!Build_WorldEntity(placement, nextId, child, catalog, owner.iNetEntityId))
			return false;
		child.eDependentBossRole = SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY;
		/* Build_WorldEntity projects static placements to a cell centre. This
		candidate already passed exact live clearance; retain that admitted pose. */
		child.fPositionX = child.fSpawnPositionX = spawn.x;
		child.fPositionY = child.fSpawnPositionY = spawn.y;
		child.fPositionZ = child.fSpawnPositionZ = spawn.z;
		child.PinnedDefinitionRevision = catalog->Get_ActiveRevision();
		child.ProductSequencePinnedDefinitionRevision = child.PinnedDefinitionRevision;
		child.bIntroPatternConsumed = true;
		child.iPhase = owner.iPhase;
		child.DependentPatternSequence.strEncounterId = owner.strEncounterId;
		child.DependentPatternSequence.strSequenceId =
			"sequence.valtan.ghost-phase.auxiliary";
		child.DependentPatternSequence.PatternIds = { selectedPatternId };
		child.DependentPatternSequence.iExpectedStepCount = 1u;
		std::vector<std::uint8_t> payload;
		if (!Build_WorldEntitySpawnedPayload(child, payload))
		{
			m_strStatus = "Dependent boss spawn wire admission failed";
			return false;
		}
		stagedGhosts.push_back(std::move(child));
		/* Commit the random occurrence only after every spawn precondition and
		wire admission has succeeded. */
		owner.iGhostAuxiliaryOccurrenceSequence = auxiliaryOccurrenceSequence;
		owner.iGhostAuxiliaryNextSpawnTick = 0u;
		++nextId;
	}
	/* Appending only after iteration preserves every primary and child reference. */
	m_WorldEntities.reserve(m_WorldEntities.size() + stagedGhosts.size());
	for (SERVER_WORLD_ENTITY& child : stagedGhosts)
	{
		m_WorldEntities.push_back(std::move(child));
		Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	}
	m_iNextNetEntityId = nextId;
	return true;
}

void LostArk::Server::CGameRoom::Clear_KoukuPlayerTargets(
	SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger)
{
	for (auto& window : ledger.PlayerTargetWindows)
	{
		for (const auto& [playerId, objectId] : window.TrackingObjects)
			m_CombatObjectRuntime.Cancel_OwnedVisualObject(objectId, boss.iNetEntityId, ledger.iPatternSequence);
		window.TrackingObjects.clear();
		window.bClosed = true;
	}
}

void LostArk::Server::CGameRoom::Update_KoukuRandomVolley(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
	KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog,
	const std::uint32_t serverTick, const bool hasAlivePlayers)
{
	using Clock = CKoukuSaydonLogicRuntime;
	if (trigger.RandomVolleys.empty() || !Clock::Has_ReachedTick(serverTick, window.iNextRandomTick)) return;
	if (trigger.iRandomSpawnIntervalMs == 0u || trigger.iRandomSpawnIntervalMs > 600000u) return;
	const auto advanceDeadline = [&] {
		do { window.iNextRandomTick = Clock::Add_Ticks(window.iNextRandomTick, Clock::Ticks_FromMs(trigger.iRandomSpawnIntervalMs)); }
		while (Clock::Has_ReachedTick(serverTick, window.iNextRandomTick));
	};
	// Empty rooms consume schedule time, but keep the next authored volley and
	// never accumulate a burst to replay when a player becomes eligible again.
	if (!hasAlivePlayers) { advanceDeadline(); return; }
	const auto& selected = trigger.RandomVolleys[window.iRandomWaveOrdinal % trigger.RandomVolleys.size()];
	BOSS_COMBAT_OBJECT_DEFINITION definition;
	definition.strEncounterId = boss.strEncounterId; definition.strOwnerPatternId = boss.strPatternId;
	definition.strOwnerStageActionId = trigger.strTriggerId;
	definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
	definition.strClientVisualId = selected.strClientVisualId;
	definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
	definition.iLifeMs = selected.iLifetimeMs;
	definition.AttackTemplates = selected.Hits;
    std::uint32_t scaleSeed = boss.iPatternSequence * 747796405u ^ boss.iNetEntityId ^ (window.iRandomWaveOrdinal * 2891336453u);
    for (const unsigned char ch : trigger.strTriggerId) scaleSeed = (scaleSeed ^ ch) * 16777619u;
    scaleSeed ^= scaleSeed >> 16u; scaleSeed *= 2246822519u; scaleSeed ^= scaleSeed >> 13u;
    const float uniformScale = trigger.fRandomScaleMin + (trigger.fRandomScaleMax - trigger.fRandomScaleMin) * float(scaleSeed & 0x00ffffffu) / 16777215.f;
    for (auto& hit : definition.AttackTemplates)
    {
        hit.fRadiusM *= uniformScale; hit.fInnerRadiusM *= uniformScale;
        hit.fLengthM *= uniformScale; hit.fHalfWidthM *= uniformScale;
        hit.fOffsetForwardM *= uniformScale; hit.fOffsetRightM *= uniformScale;
    }
	definition.PresentationPulses.push_back({ "combatpresentation.kouku.showtime.started", 0u });
	BOSS_PATTERN_STAGE_ACTION randomAction;
	randomAction.strTargetId = trigger.strTriggerId + ".random";
	randomAction.Volley.iArenaRandomCount = 1u;
	randomAction.Volley.eArenaAnchorPolicy = BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
	randomAction.Volley.fArenaRandomRadiusM = trigger.fRandomArenaRadiusM;
	randomAction.Volley.fArenaHeightToleranceM = trigger.fRandomArenaHeightToleranceM;
	std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> origins;
	// The existing sampler admits an exact walkable centre on the arena deck.
	// One centre has no intra-wave spacing constraint; no player overlap is a hit here.
    SERVER_NAV_POINT currentAnchor{};
    const SERVER_NAV_POINT* anchorOverride = nullptr;
    if (trigger.strRandomAnchorKind == "BOSS")
    {
        if (!m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, currentAnchor)) return;
        anchorOverride = &currentAnchor;
    }
    if (!Resolve_ArenaRandomVolleyOrigins(boss, randomAction, definition, window.iRandomWaveOrdinal, origins, 1.f, anchorOverride)) return;
	auto transaction = m_CombatObjectRuntime.Begin_Transaction();
	std::string status;
	if (origins.size() != 1u || !m_CombatObjectRuntime.Stage_BossCombatObject(
		transaction, boss, &origins.front(), definition, nullptr, catalog, 1u, serverTick, status))
	{ m_strStatus = "Showtime random volley preserved its interval: " + status; return; }
	auto& pose = transaction.Objects.back().LiveState.CurrentPose;
	pose.fYawDegrees = 0.f; pose.fDirectionX = 0.f; pose.fDirectionZ = 1.f;
	transaction.Objects.back().LiveState.PreviousPose = pose;
	transaction.Spawned.back().fYawDegrees = 0.f;
	transaction.Objects.back().fUniformScale = uniformScale;
	transaction.Spawned.back().fUniformScale = uniformScale;
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{ m_strStatus = "Showtime random volley preserved its interval: transaction changed"; return; }
	++window.iRandomWaveOrdinal;
	advanceDeadline();
}

void LostArk::Server::CGameRoom::Update_KoukuPursuitProjectiles(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
	KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	using Clock = CKoukuSaydonLogicRuntime;
	if ((trigger.iSpawnIntervalMs == 0u && window.iRandomWaveOrdinal != 0u) ||
		!Clock::Has_ReachedTick(serverTick, window.iNextFixedTick)) return;
	if (trigger.ProjectileVisualIds.empty() || trigger.ProjectileVisualIds.size() > 4u ||
		!trigger.iProjectileCountPerWave || trigger.iProjectileCountPerWave > 16u) return;
	SERVER_PLAYER* target = nullptr;
	NET_ENTITY_ID requiredTarget = boss.iPatternTargetEntityId;
	bool diceBinding = false;
	if (const auto* member = Find_KoukuAuditionMember(boss.iNetEntityId, boss.iPatternSequence))
		for (const auto& binding : member->LogicLedger.Windows)
			if (binding.bOpened && !binding.bClosed && binding.iFreePlayerNetEntityId != INVALID_NET_ENTITY_ID)
			{ requiredTarget = binding.iFreePlayerNetEntityId; diceBinding = true; break; }
	for (auto& [id, player] : m_Players)
		if (player.iNetEntityId == requiredTarget && (!diceBinding || !player.bPatternBound) && player.isCombatReady && player.iCurrentHp &&
			player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING &&
			player.eAction != PLAYER_ACTION_STATE::GRABBED) { target = &player; break; }
	if (!target && !diceBinding) target = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "pursuit.target", serverTick);
	if (!target) return;
	const auto sample = [&](std::uint32_t ordinal) {
		std::uint32_t value = boss.iPatternSequence * 747796405u ^ boss.iNetEntityId ^
			(window.iRandomWaveOrdinal * 2891336453u) ^ (ordinal * 277803737u);
		for (const unsigned char ch : trigger.strTriggerId) value = (value ^ ch) * 16777619u;
		value ^= value >> 16u; value *= 2246822519u; value ^= value >> 13u;
		return float(value & 0x00ffffffu) / 16777216.f;
	};
	auto transaction = m_CombatObjectRuntime.Begin_Transaction();
	std::string status;
	for (std::uint32_t ordinal = 0u; ordinal < trigger.iProjectileCountPerWave; ++ordinal)
	{
		BOSS_COMBAT_OBJECT_DEFINITION definition;
		definition.strEncounterId = boss.strEncounterId;
		definition.strOwnerPatternId = boss.strPatternId;
		definition.strOwnerStageActionId = trigger.strTriggerId;
		definition.strCombatObjectArchetypeId = "combatobject.kouku.pursuit";
		definition.strClientVisualId = trigger.ProjectileVisualIds[(window.iRandomWaveOrdinal * trigger.iProjectileCountPerWave + ordinal) % trigger.ProjectileVisualIds.size()];
		definition.iLifeMs = trigger.iProjectileLifetimeMs ? trigger.iProjectileLifetimeMs : 600000u;
		definition.AttackTemplates = trigger.ProjectileHits;
		definition.PresentationPulses.push_back({ "combatpresentation.kouku.pursuit.started", 0u });
		if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction, boss, nullptr, definition, nullptr, catalog, 1u, serverTick, status))
		{ m_strStatus = "Pursuit volley preserved existing objects: " + status; return; }
		auto& object = transaction.Objects.back();
		object.iLockedTargetNetEntityId = target->iNetEntityId;
		object.bPersistentLifetime = trigger.iProjectileLifetimeMs == 0u;
		// The staging lifetime bounds authoring; a full-lifetime contact hit follows
		// the persistent card itself. Explicitly shorter authored hit windows stay finite.
		if (object.bPersistentLifetime)
			for (auto& hit : object.Hits)
				if (hit.eTrigger == SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT && hit.iEndMs == definition.iLifeMs)
					hit.iEndMs = 0u;
		object.bRoomOwnedTracking = true;
		object.bHoming = trigger.bProjectileHoming;
		object.bExpireOnDistanceEnd = trigger.fProjectileMaxDistanceM > 0.f;
		object.fRemainingDistanceM = trigger.fProjectileSpeedMps * definition.iLifeMs / 1000.f;
		if (object.bExpireOnDistanceEnd)
			object.fRemainingDistanceM = (std::min)(object.fRemainingDistanceM, trigger.fProjectileMaxDistanceM);
		object.fSpeedMps = trigger.fProjectileSpeedMps;
		object.fContactPresentationRadiusM = trigger.fProjectileContactRadiusM;
		object.strContactPresentationId = trigger.strContactVisualId;
        if (trigger.ProjectileCardSymbols.size() == trigger.ProjectileVisualIds.size())
            object.eDamageImmuneCardSymbol = trigger.ProjectileCardSymbols[
                (window.iRandomWaveOrdinal * trigger.iProjectileCountPerWave + ordinal) % trigger.ProjectileVisualIds.size()];
		/* Saydon faces model +X: slot zero is one quarter turn from the yaw basis,
		180 degrees opposite the old back-facing spawn. Keep this in step with the Tool preview
		copy in Client/Private/KoukuSaydonPresentationPlayer_LogicPreview.cpp. */
		constexpr float PURSUIT_BODY_FRONT_YAW_DEGREES = 90.f;
		const float angle = trigger.bProjectileHoming ? (boss.fYawDegrees + PURSUIT_BODY_FRONT_YAW_DEGREES + 360.f * ordinal / trigger.iProjectileCountPerWave) : sample(ordinal) * 360.f;
		const float radians = angle * .0174532925f;
		auto& pose = object.LiveState.CurrentPose;
		pose.fDirectionX = std::sin(radians); pose.fDirectionZ = std::cos(radians); pose.fYawDegrees = angle;
		pose.fPositionX += pose.fDirectionX * trigger.fProjectileSpawnRadiusM;
		pose.fPositionZ += pose.fDirectionZ * trigger.fProjectileSpawnRadiusM;
		object.LiveState.PreviousPose = pose;
		auto& message = transaction.Spawned.back();
		message.fPositionX = pose.fPositionX; message.fPositionZ = pose.fPositionZ; message.fYawDegrees = pose.fYawDegrees;
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{ m_strStatus = "Pursuit volley preserved existing objects: transaction changed"; return; }
	++window.iRandomWaveOrdinal;
	if (trigger.iSpawnIntervalMs)
		do { window.iNextFixedTick = Clock::Add_Ticks(window.iNextFixedTick, Clock::Ticks_FromMs(trigger.iSpawnIntervalMs)); }
		while (Clock::Has_ReachedTick(serverTick, window.iNextFixedTick));
}

void LostArk::Server::CGameRoom::Update_KoukuPlayerTargets(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	KOUKUSAYDON_LOGIC_LEDGER& ledger, const CGameplayCatalog& catalog, const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	using Clock = CKoukuSaydonLogicRuntime;
	if (ledger.strPatternId != pattern.strPatternId || ledger.iPatternSequence != boss.iPatternSequence) return;
	const auto eligible = [](const SERVER_PLAYER& player) {
		return player.isCombatReady && player.iCurrentHp != 0u &&
			player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING;
	};
	for (auto& window : ledger.PlayerTargetWindows)
	{
		if (window.bClosed || window.iLastUpdateTick == serverTick || window.iTriggerIndex >= pattern.MechanicTriggers.size()) continue;
		if (!Clock::Has_ReachedTick(serverTick, window.iStartTick) ||
			(window.iLastUpdateTick != 0u && !Clock::Has_ReachedTick(serverTick, window.iLastUpdateTick))) continue;
		const std::uint32_t previousUpdateTick = window.iLastUpdateTick;
		window.iLastUpdateTick = serverTick;
		const auto& trigger = pattern.MechanicTriggers[window.iTriggerIndex];
		if (Clock::Has_ReachedTick(serverTick, window.iEndTick))
		{
			for (const auto& [playerId, objectId] : window.TrackingObjects)
				m_CombatObjectRuntime.Cancel_OwnedVisualObject(objectId, boss.iNetEntityId, ledger.iPatternSequence);
			window.TrackingObjects.clear(); window.bClosed = true;
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES)
		{
			Update_KoukuPursuitProjectiles(boss, trigger, window, catalog, serverTick);
			continue;
		}
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
            trigger.strFixedVisualId.empty() && trigger.strTrackingVisualId.empty() && !trigger.RandomVolleys.empty())
        {
            const bool hasAlivePlayers = std::any_of(m_Players.begin(), m_Players.end(), [&](const auto& entry) { return eligible(entry.second); });
            Update_KoukuRandomVolley(boss, trigger, window, catalog, serverTick, hasAlivePlayers);
            continue;
        }
		const bool rotateOnly = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
		// RPCT_05 and RPCT_06 face model +X in every tracking mode. The same
		// basis must aim the stationary Showtime body and moving pursuit.
		const bool saydon = boss.strArchetypeId == "BOSS_KAKULSAYDON_G1_SAYDON" ||
			boss.strArchetypeId == "BOSS_KAKULSAYDON_G3_SAYDON" ||
			boss.strArchetypeId == "BOSS_KAKULSAYDON_BINGO_SAYDON" ||
			boss.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
		const float forwardYawOffset = saydon ? 90.f : 0.f;
		if (!rotateOnly && (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
			trigger.iSpawnIntervalMs == 0u || trigger.strFixedVisualId == trigger.strTrackingVisualId ||
			trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) ||
			!std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
		{ m_strStatus = "Showtime target window lost its validated definition"; continue; }
		// Fixed drops capture each player without turning the authored boss motion.
		// Tracking visuals and rotate-only windows keep the existing facing policy.
		const bool tracksFacing = rotateOnly || trigger.strFixedVisualId.empty() || !trigger.strTrackingVisualId.empty();
		const auto findFacingTarget = [&](const NET_ENTITY_ID id) -> SERVER_PLAYER* {
			for (auto& [playerId, player] : m_Players)
				if (player.iNetEntityId == id && eligible(player) && player.eAction != PLAYER_ACTION_STATE::GRABBED)
					return &player;
			return nullptr;
		};
		auto* facingTarget = tracksFacing ? findFacingTarget(boss.iPatternTargetEntityId) : nullptr;
		if (tracksFacing && !facingTarget) facingTarget = findFacingTarget(boss.iTargetEntityId);
		if (tracksFacing && !facingTarget) facingTarget = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "boss.target.pattern", serverTick);
		if (facingTarget)
		{
			boss.iTargetEntityId = boss.iPatternTargetEntityId = facingTarget->iNetEntityId;
			boss.bHasPatternTargetLastPosition = true;
			boss.fPatternTargetLastPositionX = facingTarget->fPositionX;
			boss.fPatternTargetLastPositionY = facingTarget->fPositionY;
			boss.fPatternTargetLastPositionZ = facingTarget->fPositionZ;
			const float dx = facingTarget->fPositionX - boss.fPositionX, dz = facingTarget->fPositionZ - boss.fPositionZ;
			if (std::isfinite(dx) && std::isfinite(dz) && dx * dx + dz * dz > .000001f)
			{
				float targetYaw = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
				targetYaw -= forwardYawOffset;
				// Both modes consume the same elapsed fixed ticks. Rotate-only windows
				// use the shared tenfold remaining-arc response; moving pursuit has
				// an independent 180-degree/second turn speed.
				const auto totalTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, window.iEndTick);
				const auto previousTicks = previousUpdateTick == 0u ? 0u :
					Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, previousUpdateTick) + 1u;
				const auto currentTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, serverTick) + 1u;
				if (!rotateOnly) boss.fYawDegrees = targetYaw;
				else if (std::isfinite(boss.fYawDegrees) && totalTicks > previousTicks && currentTicks > previousTicks)
				{
					const double turn = std::remainder(double(targetYaw) - boss.fYawDegrees, 360.0);
					// Moving pursuit has a turn-speed limit independent of the window lifetime.
					// Rotate-only response is identical for grouped and individual fixed ticks.
					const double yawStep = trigger.fFollowSpeedScale > 0.f ?
						(std::clamp)(turn, -6.0 * (currentTicks - previousTicks), 6.0 * (currentTicks - previousTicks)) :
						turn * KoukuTargetTracking::RotateOnlyFraction(currentTicks - previousTicks, totalTicks - previousTicks);
					boss.fYawDegrees = static_cast<float>(std::remainder(double(boss.fYawDegrees) + yawStep, 360.0));
				}
			}
		}
		else if (tracksFacing)
		{
			boss.iTargetEntityId = boss.iPatternTargetEntityId = INVALID_NET_ENTITY_ID;
			boss.bHasPatternTargetLastPosition = false;
		}
		if (rotateOnly)
		{
			/* A positive authored scale walks the body along the yaw this tick just wrote,
			   at the tracked player's own move speed, so BOSS-anchored Effects travel and
			   turn with it. Zero preserves the original rotate-only window exactly. */
			SERVER_NAV_POINT startGround{};
			const float followStep = facingTarget && trigger.fFollowSpeedScale > 0.f ?
				Resolve_PlayerMoveSpeed(*facingTarget) * trigger.fFollowSpeedScale / 30.f : 0.f;
			if (followStep <= 0.f || !std::isfinite(followStep) || !std::isfinite(boss.fYawDegrees)) continue;
			if (!m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, startGround))
			{ m_strStatus = "Boss tracking follow lost its current ground"; continue; }
			// Terrain owns the base; the source clip owns the height above it.
			const float followHeight = boss.fPositionY - startGround.y;
			if (!std::isfinite(followHeight)) continue;
			const float followRadians = (boss.fYawDegrees + forwardYawOffset) * .0174532925f;
			SERVER_NAV_POINT destination{ boss.fPositionX + std::sin(followRadians) * followStep,
				boss.fPositionY, boss.fPositionZ + std::cos(followRadians) * followStep };
			if (!m_ServerNavigation.Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ,
				destination.x, destination.z, destination))
			{ m_strStatus = "Boss tracking follow stopped at the navigation boundary"; continue; }
			destination.y += followHeight;
			bool followBlocked = false;
			SERVER_NAV_POINT followGround{};
			if (!m_ServerCollisionSystem.Resolve_CircleMove(boss.fPositionX, boss.fPositionY, boss.fPositionZ,
				destination.x, destination.y, destination.z, boss.fCollisionRadius, boss.fCollisionRadius,
				boss.fCollisionRadius, destination.x, destination.y, destination.z, followBlocked, boss.iNetEntityId, false) ||
				!m_ServerNavigation.Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ,
					destination.x, destination.z, followGround))
			{ m_strStatus = "Boss tracking follow preserved its pose at the collision boundary"; continue; }
			const float followY = followGround.y + followHeight;
			if (!std::isfinite(destination.x) || !std::isfinite(followY) || !std::isfinite(destination.z)) continue;
			/* The stage clock still owns its own baked root motion from a captured origin.
			   Move that origin and its ground base by the same delta, which keeps
			   originY - rootGroundY invariant so the next sample continues from here. */
			if (boss.bPatternStageRootOriginCaptured)
			{
				boss.fPatternStageOriginX += destination.x - boss.fPositionX;
				boss.fPatternStageOriginY += followY - boss.fPositionY;
				boss.fPatternStageOriginZ += destination.z - boss.fPositionZ;
				boss.fPatternStageRootGroundY += followGround.y - startGround.y;
			}
			boss.fPositionX = destination.x; boss.fPositionY = followY; boss.fPositionZ = destination.z;
			if (std::any_of(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& body) { return &body == &boss; }))
				(void)m_ServerCollisionSystem.Update_BlockingBody(boss.iNetEntityId,
					boss.fPositionX, boss.fPositionY + boss.fCollisionRadius, boss.fPositionZ);
			if (followBlocked) m_strStatus = "Boss tracking follow stopped at the collision boundary";
			continue;
		}
		for (auto it = window.TrackingObjects.begin(); it != window.TrackingObjects.end();)
		{
			const auto player = m_Players.find(it->first);
			const auto& live = m_CombatObjectRuntime.Get_LiveObjects();
			const auto object = std::find_if(live.begin(), live.end(), [&](const auto& row) { return row.iCombatObjectId == it->second; });
			if (player == m_Players.end() || !eligible(player->second) || object == live.end() ||
				object->iLockedTargetNetEntityId != player->second.iNetEntityId)
			{
				m_CombatObjectRuntime.Cancel_OwnedVisualObject(it->second, boss.iNetEntityId, ledger.iPatternSequence);
				it = window.TrackingObjects.erase(it); continue;
			}
			const auto& pose = object->LiveState.CurrentPose;
			const float dx = player->second.fPositionX - pose.fPositionX, dz = player->second.fPositionZ - pose.fPositionZ;
			const float distance = std::hypot(dx, dz);
			const float step = Resolve_PlayerMoveSpeed(player->second) * trigger.fFollowSpeedScale / 30.f;
			if (!std::isfinite(distance) || !std::isfinite(step) || step < 0.f)
			{ m_strStatus = "Showtime tracking preserved its pose: non-finite target or speed"; ++it; continue; }
			const float ratio = distance > 0.f ? (std::min)(1.f, step / distance) : 0.f;
			const float x = pose.fPositionX + dx * ratio, z = pose.fPositionZ + dz * ratio;
			SERVER_NAV_POINT ground{};
			if (m_ServerNavigation.Sample_Position(x, z, ground) && std::isfinite(ground.y))
				m_CombatObjectRuntime.Set_OwnedVisualPosition(it->second, boss.iNetEntityId, ledger.iPatternSequence, x, ground.y, z);
			else m_strStatus = "Showtime tracking preserved its pose: ground is unavailable";
			++it;
		}
		const bool fixedDue = !trigger.strFixedVisualId.empty() && Clock::Has_ReachedTick(serverTick, window.iNextFixedTick);
		std::size_t playerCount = 0u;
		for (const auto& [playerId, player] : m_Players) if (eligible(player)) ++playerCount;
		if (playerCount > 4u) { m_strStatus = "Showtime target window exceeds four living players"; continue; }
		Update_KoukuRandomVolley(boss, trigger, window, catalog, serverTick, playerCount != 0u);
		auto transaction = m_CombatObjectRuntime.Begin_Transaction();
		std::vector<std::pair<PLAYER_ID, COMBAT_OBJECT_ID>> createdTrackers;
		bool admitted = true;
		std::string failure;
		for (const auto& [playerId, player] : m_Players)
		{
			if (!eligible(player)) continue;
			const bool newTracker = !trigger.strTrackingVisualId.empty() && !window.TrackingObjects.contains(playerId);
			if (!fixedDue && !newTracker) continue;
			for (const bool tracking : { false, true })
			{
				if ((tracking && !newTracker) || (!tracking && !fixedDue)) continue;
				// Each living player's tracker begins at the authored arena centre,
				// while its exact locked entity remains the independent pursuit target.
				const float originX = tracking ? boss.fSpawnPositionX : player.fPositionX;
				const float originZ = tracking ? boss.fSpawnPositionZ : player.fPositionZ;
				SERVER_NAV_POINT ground{};
				if (!m_ServerNavigation.Sample_Position(originX, originZ, ground) || !std::isfinite(ground.y))
				{ admitted = false; failure = "the selected player or arena-centre ground is unavailable"; break; }
				SERVER_COMBAT_OBJECT_LOCKED_TARGET target;
				target.iNetEntityId = player.iNetEntityId;
				target.fPositionX = originX; target.fPositionY = ground.y; target.fPositionZ = originZ;
				BOSS_COMBAT_OBJECT_DEFINITION definition;
				definition.strEncounterId = boss.strEncounterId;
				definition.strOwnerPatternId = pattern.strPatternId;
				definition.strOwnerStageActionId = trigger.strTriggerId;
				definition.strCombatObjectArchetypeId = tracking ? "combatobject.kouku.showtime.tracking" : "combatobject.kouku.showtime.fixed";
				definition.strClientVisualId = tracking ? trigger.strTrackingVisualId : trigger.strFixedVisualId;
				definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
				// Exact window cleanup owns tracking; its float lifetime has one spare tick.
				const auto remainingTicks = static_cast<std::uint32_t>(window.iEndTick - serverTick);
				definition.iLifeMs = tracking ? static_cast<std::uint32_t>((static_cast<std::uint64_t>(remainingTicks) * 1000u + 29u) / 30u + 34u) : trigger.iFixedLifetimeMs;
				definition.AttackTemplates = tracking ? trigger.TrackingHits : trigger.FixedHits;
				if (tracking)
				{
					std::erase_if(definition.AttackTemplates, [&](const auto& hit) { return hit.iAtMs >= definition.iLifeMs; });
					for (auto& hit : definition.AttackTemplates) if (hit.strTrigger == "CONTACT") hit.iEndMs = (std::min)(hit.iEndMs, definition.iLifeMs);
				}
				definition.PresentationPulses.push_back({ "combatpresentation.kouku.showtime.started", 0u });
				if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction, boss, &target, definition, nullptr, catalog, 1u, serverTick, failure))
				{ admitted = false; break; }
				// Group TRS is in map axes; the boss's current facing must not rotate it.
				auto& object = transaction.Objects.back();
				object.bRoomOwnedTracking = true;
				object.LiveState.CurrentPose.fYawDegrees = 0.f;
				object.LiveState.CurrentPose.fDirectionX = 0.f;
				object.LiveState.CurrentPose.fDirectionZ = 1.f;
				object.LiveState.PreviousPose = object.LiveState.CurrentPose;
				transaction.Spawned.back().fYawDegrees = 0.f;
				if (tracking) createdTrackers.emplace_back(playerId, object.iCombatObjectId);
			}
			if (!admitted) break;
		}
		if (!admitted) { m_strStatus = "Showtime targets preserved existing objects: " + failure; continue; }
		if (!transaction.Objects.empty())
		{
			if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
			{ m_strStatus = "Showtime targets preserved existing objects: transaction changed"; continue; }
			for (const auto& [playerId, objectId] : createdTrackers) window.TrackingObjects.emplace(playerId, objectId);
		}
		if (fixedDue)
		{
			// Only consume an admitted interval; never replay missed ticks as a burst.
			do { window.iNextFixedTick = Clock::Add_Ticks(window.iNextFixedTick, Clock::Ticks_FromMs(trigger.iSpawnIntervalMs)); }
			while (Clock::Has_ReachedTick(serverTick, window.iNextFixedTick));
		}
	}
}

bool LostArk::Server::CGameRoom::Commit_KoukuAlbionAirborne(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, const std::uint32_t serverTick)
{
	using Phase = ALBION_AIRBORNE_PHASE;
	using namespace LostArk::Shared;
	const auto reject = [&](const char* reason) { m_strStatus = std::string("Albion airborne preserved its pose: ") + reason; return false; };
	if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE || boss.strPatternId != pattern.strPatternId ||
	 boss.iPatternStageIndex >= pattern.Stages.size() || !serverTick) return reject("owner or stage is unavailable");
	auto staged = std::make_unique<SERVER_WORLD_ENTITY>(boss);
	auto& state = staged->AlbionAirborne;
	if (state.iPatternSequence != boss.iPatternSequence) { state = {}; state.iPatternSequence = boss.iPatternSequence; }
	const auto eligible = [](const SERVER_PLAYER& player) {
	 return player.isCombatReady && player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
	  player.eAction != PLAYER_ACTION_STATE::FALLING && player.eAction != PLAYER_ACTION_STATE::GRABBED;
	};
	const auto selectedPlayer = [&]() -> SERVER_PLAYER* {
	 for (auto& [id, player] : m_Players) if (player.iNetEntityId == state.iSelectedPlayer && eligible(player)) return &player;
	 return nullptr;
	};
	if (trigger.eAirbornePhase == Phase::SELECT_PLAYER)
	{
	 auto* selected = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "albion.airborne.player", serverTick);
	 if (!selected) return reject("no living selectable player");
	 state.iSelectedPlayer = selected->iNetEntityId;
	 state.bHasSelectedGround = false;
	 if (trigger.bCaptureAirborneTargetPosition)
	 {
	  if (!m_ServerNavigation.Is_Loaded() || !m_ServerNavigation.Is_PointWalkableExact(selected->fPositionX, selected->fPositionZ) ||
	   !m_ServerNavigation.Sample_Position(selected->fPositionX, selected->fPositionZ, state.SelectedGround) ||
	   !std::isfinite(state.SelectedGround.y)) return reject("selected navigation ground is unavailable");
	  state.SelectedGround.x = selected->fPositionX; state.SelectedGround.z = selected->fPositionZ;
	  state.bHasSelectedGround = true;
	  if (!trigger.strSelectedEffectVisualId.empty())
	  {
	   const auto* catalog = Resolve_KoukuProductCatalog();
	   if (!catalog) return reject("selected Effect catalog is unavailable");
	   SERVER_COMBAT_OBJECT_LOCKED_TARGET target;
	   target.iNetEntityId = selected->iNetEntityId;
	   target.fPositionX = state.SelectedGround.x; target.fPositionY = state.SelectedGround.y; target.fPositionZ = state.SelectedGround.z;
	   BOSS_COMBAT_OBJECT_DEFINITION definition;
	   definition.strEncounterId = boss.strEncounterId; definition.strOwnerPatternId = pattern.strPatternId;
	   definition.strOwnerStageActionId = trigger.strTriggerId;
	   definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
	   definition.strClientVisualId = trigger.strSelectedEffectVisualId;
	   definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
	   definition.iLifeMs = trigger.iSelectedEffectLifetimeMs;
	   definition.PresentationPulses.push_back({ "combatpresentation.kouku.showtime.started", 0u });
	   auto transaction = m_CombatObjectRuntime.Begin_Transaction(); std::string status;
	   if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction, boss, &target, definition, nullptr, *catalog, 1u, serverTick, status))
	   { m_strStatus = "Selected airborne Effect preserved capture: " + status; return false; }
	   auto& object = transaction.Objects.back();
	   object.LiveState.CurrentPose.fYawDegrees = 0.f;
	   object.LiveState.CurrentPose.fDirectionX = 0.f; object.LiveState.CurrentPose.fDirectionZ = 1.f;
	   object.LiveState.PreviousPose = object.LiveState.CurrentPose;
	   transaction.Spawned.back().fYawDegrees = 0.f;
	   if (!m_CombatObjectRuntime.Commit(std::move(transaction))) return reject("selected Effect transaction changed");
	  }
	 }
	 boss.AlbionAirborne = state; boss.iTargetEntityId = boss.iPatternTargetEntityId = selected->iNetEntityId;
	 return true;
	}
	SERVER_NAV_POINT oldGround{}, newGround{};
	if (!m_ServerNavigation.Is_Loaded() || !m_ServerNavigation.Sample_Position(boss.fPositionX,boss.fPositionZ,oldGround))
	 return reject("current navigation ground is unavailable");
	const double patternMs = CKoukuSaydonBrain::Pattern_ElapsedMs(boss, serverTick);
	const double stageMs = CKoukuSaydonBrain::Stage_RootTimeMs(boss, pattern, serverTick);
	const float sourceUp = CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion, stageMs).fUp;
	state.ePhase = trigger.eAirbornePhase; state.iStartMs = trigger.iStartMs;
	state.iDurationMs = trigger.iAirborneDurationMs; state.iSourceStageIndex = boss.iPatternStageIndex;
	state.fStartHeightM = boss.fPositionY - oldGround.y; state.fPhaseHeightM = trigger.fAirborneHeightM; state.fLandingProgress = 0.f;
	float x = boss.fPositionX, z = boss.fPositionZ;
	if (state.ePhase == Phase::JUMP)
	{
	 if (!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM <= 0.f || trigger.fAirborneHeightM > 100000.f ||
	  trigger.iAirborneDurationMs > 600000u) return reject("jump height or duration is invalid");
	 state.fJumpHeightM = trigger.fAirborneHeightM;
	 // Reuse the actual navigation/body sweep. The same source clock still owns XZ.
	 staged->iPatternStageRootLastTick = 0u;
	 std::string status;
	 if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,pattern,serverTick,m_ServerNavigation,m_ServerCollisionSystem,status))
	 { m_strStatus = "Albion jump preserved its pose: " + status; return false; }
	}
	else
	{
	 if (!std::isfinite(state.fJumpHeightM) || state.fJumpHeightM <= 0.f) return reject("initial jump has not committed");
	 if (state.ePhase == Phase::APPEAR_PLAYER)
	 {
	  if (state.fPhaseHeightM <= 0.f || !std::isfinite(state.fPhaseHeightM)) return reject("appearance height is invalid");
	  if (state.bHasSelectedGround)
	  { x = state.SelectedGround.x; z = state.SelectedGround.z; }
	  else
	  {
	   auto* selected = selectedPlayer();
	   if (!selected) selected = Select_BossRandomAliveTarget(boss,trigger.strTriggerId,"albion.airborne.player",serverTick);
	   if (!selected) return reject("selected player is unavailable and no replacement is alive");
	   state.iSelectedPlayer = selected->iNetEntityId; x = selected->fPositionX; z = selected->fPositionZ;
	  }
	 }
	 else if (state.ePhase == Phase::CENTER) { x = trigger.fTeleportX; z = trigger.fTeleportZ; }
	 else if (state.ePhase == Phase::SLAM)
	 {
	  std::uint64_t stageStart = 0u;
	  for (std::uint32_t index=0u; index<boss.iPatternStageIndex; ++index) stageStart += pattern.Stages[index].iDurationMs;
	  const double triggerAge = (std::max)(0.0,double(trigger.iStartMs)-double(stageStart));
	  state.fSourceUpAtStart = CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion,triggerAge).fUp;
	  state.fSourceUpMinimum = state.fSourceUpAtStart;
	  for (const auto& sample : boss.PatternStageRootMotion) if (sample.iTimeMs >= triggerAge)
	   state.fSourceUpMinimum = (std::min)(state.fSourceUpMinimum,sample.fUp);
	  if (state.fSourceUpAtStart-state.fSourceUpMinimum <= .000001f) return reject("source animation has no remaining descent");
	 }
	 else if (state.ePhase != Phase::DISAPPEAR) return reject("unknown motion phase");
	 if (state.ePhase == Phase::SLAM)
	 {
	  staged->iPatternStageRootLastTick = 0u;
	  std::string status;
	  if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,pattern,serverTick,m_ServerNavigation,m_ServerCollisionSystem,status))
	  { m_strStatus = "Albion slam preserved its pose: " + status; return false; }
	 }
	 else
	 {
	 if (!std::isfinite(x) || !std::isfinite(z) || !m_ServerNavigation.Is_PointWalkableExact(x,z) ||
	  !m_ServerNavigation.Sample_Position(x,z,newGround)) return reject("destination navigation is unavailable");
	 if (state.ePhase == Phase::CENTER && (!std::isfinite(trigger.fTeleportY) || std::abs(trigger.fTeleportY-newGround.y)>1.f))
	  return reject("center reference height does not match navigation");
	 float height = 0.f;
	 if (!CKoukuSaydonBrain::Sample_AlbionAirborneHeight(state,patternMs,boss.iPatternStageIndex,sourceUp,height))
	  return reject("phase height is invalid");
	 // A captured ground point owns the complete anchor, including terrain height.
	 if (state.ePhase == Phase::APPEAR_PLAYER && state.bHasSelectedGround) newGround.y = state.SelectedGround.y;
	 const float y = newGround.y + height;
	 if (!std::isfinite(y) || !m_ServerCollisionSystem.Is_CirclePositionClear(x,y,z,boss.fCollisionRadius,
	  boss.fCollisionRadius,boss.fCollisionRadius,boss.iNetEntityId)) return reject("destination body overlap is invalid");
	 // Rebase the existing stage origin, including terrain, so its next sample
	 // cannot undo the teleport or introduce a second vertical displacement.
	 if (staged->bPatternStageRootOriginCaptured)
	 {
	  staged->fPatternStageOriginX += x-boss.fPositionX; staged->fPatternStageOriginY += y-boss.fPositionY;
	  staged->fPatternStageOriginZ += z-boss.fPositionZ;
	  staged->fPatternStageRootGroundY += newGround.y-oldGround.y;
	 }
	 staged->fPositionX=x; staged->fPositionY=y; staged->fPositionZ=z; staged->iPatternStageRootLastTick=serverTick;
	 }
	}
	if (state.iSelectedPlayer) staged->iTargetEntityId=staged->iPatternTargetEntityId=state.iSelectedPlayer;
	boss = std::move(*staged);
	if (std::any_of(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& body) { return &body == &boss; }))
		(void)m_ServerCollisionSystem.Update_BlockingBody(boss.iNetEntityId,boss.fPositionX,boss.fPositionY+boss.fCollisionRadius,boss.fPositionZ);
	return true;
}

void LostArk::Server::CGameRoom::Commit_KoukuMechanicTriggers(const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	// Equal-clock player selection precedes appearance regardless of authored row order.
	std::stable_sort(m_PendingKoukuMechanicTriggers.begin(), m_PendingKoukuMechanicTriggers.end(), [](const auto& a, const auto& b) {
		if (a.iBossEntityId != b.iBossEntityId) return a.iBossEntityId < b.iBossEntityId;
		if (a.iPatternSequence != b.iPatternSequence) return a.iPatternSequence < b.iPatternSequence;
		if (a.Trigger.iStartMs != b.Trigger.iStartMs) return a.Trigger.iStartMs < b.Trigger.iStartMs;
		const auto priority = [](const auto& t) { return t.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
			t.eAirbornePhase == ALBION_AIRBORNE_PHASE::SELECT_PLAYER ? 0 : 1; };
		return priority(a.Trigger) < priority(b.Trigger);
	});
	// This executes outside the world iteration: vector growth cannot invalidate a boss reference.
	for (const auto& pending : m_PendingKoukuMechanicTriggers)
	{
		auto liveOwner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&pending](const SERVER_WORLD_ENTITY& entity) { return entity.iNetEntityId == pending.iBossEntityId; });
		auto* owner = Find_KoukuOccurrenceOwner(pending.iBossEntityId, pending.iPatternSequence);
		if (!owner || liveOwner == m_WorldEntities.end()) continue;
		const bool detached = owner != &*liveOwner;
		const auto& trigger = pending.Trigger;
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD)
        { Begin_KoukuBingoDuration(*owner, trigger, serverTick); continue; }
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
		{
			const auto* catalog = Resolve_KoukuProductCatalog(); std::string status;
			const auto* pattern = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
			if (!pattern) { m_strStatus = "Albion airborne preserved its pose: pinned pattern is unavailable: " + status; continue; }
			(void)Commit_KoukuAlbionAirborne(*owner,*pattern,trigger,serverTick);
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS)
		{
			(void)Commit_KoukuMarioPhasePlayers(*owner, trigger, serverTick);
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
		{
			SERVER_NAV_POINT oldGround{}, newGround{};
			const bool faceCenter = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER;
			const bool grounded = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED;
            // Match the existing FaceCenter/Albion center deck admission tolerance.
            constexpr float destinationHeightToleranceM = 1.f;
            const float referenceY = (faceCenter || grounded) ? trigger.fTeleportY : owner->fPositionY;
			if (!std::isfinite(referenceY) || !std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportZ) ||
				!m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ) ||
				!m_ServerNavigation.Sample_Position(owner->fPositionX, owner->fPositionZ, oldGround) ||
				!m_ServerNavigation.Sample_Position(trigger.fTeleportX, trigger.fTeleportZ, newGround) ||
				((faceCenter || grounded) && std::abs(referenceY - newGround.y) > destinationHeightToleranceM))
            { m_strStatus = "Boss teleport preserved its pose: destination navigation or height is invalid"; continue; }
            const float destinationY = grounded ? newGround.y : referenceY;
            if (!m_ServerCollisionSystem.Is_CirclePositionClear(trigger.fTeleportX, destinationY + owner->fCollisionRadius, trigger.fTeleportZ,
					owner->fCollisionRadius, owner->fCollisionRadius, owner->fCollisionRadius, owner->iNetEntityId))
			{ m_strStatus = "Boss XZ teleport preserved its pose: destination navigation or body overlap is invalid"; continue; }
			const float rootX = owner->fPatternStageOriginX + trigger.fTeleportX - owner->fPositionX;
			const float rootZ = owner->fPatternStageOriginZ + trigger.fTeleportZ - owner->fPositionZ;
			const float rootGroundY = owner->fPatternStageRootGroundY + newGround.y - oldGround.y;
            const float rootY = owner->fPatternStageOriginY + (grounded ? destinationY - owner->fPositionY : 0.f);
			if (owner->bPatternStageRootOriginCaptured &&
				(!std::isfinite(rootX) || !std::isfinite(rootZ) || !std::isfinite(rootY) || !std::isfinite(rootGroundY)))
			{ m_strStatus = "Boss XZ teleport preserved its pose: root origin is invalid"; continue; }
			// XZ keeps root-up; grounded rebases Y by the same committed floor delta.
            // Both preserve stage/action clocks and continue the existing root curve.
			if (owner->bPatternStageRootOriginCaptured)
			{
				owner->fPatternStageOriginX = rootX; owner->fPatternStageOriginZ = rootZ;
				owner->fPatternStageRootGroundY = rootGroundY;
                if (grounded) owner->fPatternStageOriginY = rootY;
			}
            if (grounded) owner->bPatternRootGrounded = true;
			owner->fPositionX = trigger.fTeleportX; owner->fPositionZ = trigger.fTeleportZ;
            if (grounded) owner->fPositionY = destinationY;
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
			{
				owner->fPositionY = trigger.fTeleportY;
				const bool bingo = owner->strArchetypeId == "BOSS_KAKULSAYDON_BINGO_SAYDON";
				const float dx = owner->fSpawnPositionX - owner->fPositionX, dz = owner->fSpawnPositionZ - owner->fPositionZ;
				if (dx * dx + dz * dz > .000001f)
					owner->fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES -
						(bingo || owner->strArchetypeId == "BOSS_KAKULSAYDON_G1_SAYDON" ||
						 owner->strArchetypeId == "BOSS_KAKULSAYDON_G3_SAYDON" ||
						 owner->strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON" ? 90.f : 0.f);
				owner->fPatternStageOriginYawDegrees = owner->fYawDegrees;
			}

			if (!detached) m_ServerCollisionSystem.Update_BlockingBody(owner->iNetEntityId, owner->fPositionX,
				owner->fPositionY + owner->fCollisionRadius, owner->fPositionZ);
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
		{
			const auto* catalog = Resolve_KoukuProductCatalog();
			const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
			std::string status;
			const auto* parent = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
			std::size_t selected = 0u;
			std::array<std::uint32_t, 4u> cloneDurations{};
			if (!parent || !baseCatalog || owner->KoukuDirectionPlayback || owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID ||
				!m_ServerNavigation.Is_Loaded() || !CKoukuSaydonBrain::Select_CrossDirection(*owner, *parent, trigger,
					*catalog, selected, cloneDurations, status))
			{ m_strStatus = "Cross direction preserved all actors: " + status; continue; }
			auto real = std::make_shared<SERVER_WORLD_ENTITY>(*owner);
			m_KoukuSaydonBrain.Abort_Pattern(*real, serverTick);
			const auto* realPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[selected], status);
			if (!realPattern || !m_KoukuSaydonBrain.Begin_Pattern(*real, *realPattern, owner->PinnedDefinitionRevision, serverTick, status))
			{ m_strStatus = "Cross direction preserved all actors: " + status; continue; }
			std::vector<SERVER_WORLD_ENTITY> clones;
			NET_ENTITY_ID nextId = m_iNextNetEntityId;
			for (std::size_t index = 0u; index < 4u; ++index)
			{
				if (index == selected) continue;
				const auto* child = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[index], status);
				WORLD_BOOTSTRAP_PLACEMENT placement{};
				placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
				placement.strPlacementId = owner->strPlacementId + ".cross." + std::to_string(owner->iPatternSequence) + "." + std::to_string(index);
				placement.strArchetypeId = owner->strArchetypeId; placement.strEncounterId = owner->strEncounterId;
				placement.fPositionX = owner->fPositionX; placement.fPositionY = owner->fPositionY; placement.fPositionZ = owner->fPositionZ;
				placement.fYawDegrees = owner->fYawDegrees;
				SERVER_WORLD_ENTITY clone{};
				if (!child || nextId == INVALID_NET_ENTITY_ID ||
					!Build_WorldEntity(placement, nextId, clone, baseCatalog, owner->iNetEntityId, owner->iPatternSequence))
				{ status = "a cross direction clone failed dependent-owner admission"; break; }
				clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
				clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
				clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
				clone.fYawDegrees = placement.fYawDegrees;
				clone.bKoukuGazeClone = clone.bKoukuSummonClone = true;
				clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
				clone.iKoukuCloneEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
					CKoukuSaydonLogicRuntime::Ticks_FromMs(cloneDurations[index]));
				if (!m_KoukuSaydonBrain.Begin_Pattern(clone, *child, owner->PinnedDefinitionRevision, serverTick, status)) break;
				std::vector<std::uint8_t> payload;
				if (!Build_WorldEntitySpawnedPayload(clone, payload)) { status = "a cross direction clone failed spawn serialization"; break; }
				clones.push_back(std::move(clone)); ++nextId;
			}
			if (clones.size() != 3u)
			{ m_strStatus = "Cross direction preserved all actors: " + status; continue; }
			const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
			m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
			auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
			committedOwner.KoukuDirectionPlayback = std::move(real);
			committedOwner.bKoukuDirectionPlaybackComplete = false;
			committedOwner.iKoukuDirectionEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(committedOwner.iPatternStartTick,
				CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
			for (auto& clone : clones)
			{ m_WorldEntities.push_back(std::move(clone)); Broadcast_WorldEntitySpawned(m_WorldEntities.back()); }
			m_iNextNetEntityId = nextId;
			m_strStatus = "Cross direction selected real Pattern " + trigger.DirectionPatternIds[selected] + "; three clones end at " + trigger.strCloneEndStageId;
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS)
		{
			if (!detached) (void)Spawn_KoukuCardRainSoldiers(owner->iNetEntityId, serverTick);
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
		{
			const auto* catalog = Resolve_KoukuProductCatalog();
			const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
			std::string status;
			const auto* ownerPattern = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
			if (!ownerPattern || !baseCatalog || owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID ||
				!m_ServerNavigation.Is_Loaded() || trigger.PatternSpawns.empty() || trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
			{ m_strStatus = "Summon Patterns preserved all actors: owner, catalog or spawn list is unavailable: " + status; continue; }
			const auto authoredTrigger = std::find_if(ownerPattern->MechanicTriggers.begin(), ownerPattern->MechanicTriggers.end(),
				[&](const auto& row) { return row.strTriggerId == trigger.strTriggerId && row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS; });
			if (authoredTrigger == ownerPattern->MechanicTriggers.end())
			{ m_strStatus = "Summon Patterns preserved all actors: the owning occurrence is unavailable"; continue; }
			const float yaw = owner->fYawDegrees * 0.01745329251994329577f;
			std::vector<SERVER_WORLD_ENTITY> clones;
			NET_ENTITY_ID nextId = m_iNextNetEntityId;
			bool admitted = true;
			for (const auto& spawn : trigger.PatternSpawns)
			{
				const auto* childPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, spawn.strPatternId, status);
				if (!childPattern || !CKoukuSaydonBrain::Validate_SummonedPattern(*ownerPattern, *childPattern, status, true))
				{ admitted = false; break; }
				std::uint64_t childDurationMs = 0u;
				for (const auto& stage : childPattern->Stages) childDurationMs += stage.iDurationMs;
				// A fixed Parent clock shortens pending duration to its absolute deadline.
				// Admission compares against the original authored window, before tick rounding.
				if (childDurationMs > authoredTrigger->iDurationMs)
				{ status = "a summoned Pattern exceeds its occurrence lifetime"; admitted = false; break; }
				WORLD_BOOTSTRAP_PLACEMENT placement{};
				placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
				placement.strPlacementId = owner->strPlacementId + ".summon." + std::to_string(owner->iPatternSequence) + "." + trigger.strTriggerId + "." + spawn.strSpawnId;
				placement.strArchetypeId = owner->strArchetypeId;
				placement.strEncounterId = owner->strEncounterId;
				const bool mapAnchor = spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP;
				placement.fPositionX = mapAnchor ? spawn.PositionOffset[0] : owner->fPositionX + spawn.PositionOffset[0] * std::cos(yaw) + spawn.PositionOffset[2] * std::sin(yaw);
				placement.fPositionY = mapAnchor ? spawn.PositionOffset[1] : owner->fPositionY + spawn.PositionOffset[1];
				placement.fPositionZ = mapAnchor ? spawn.PositionOffset[2] : owner->fPositionZ - spawn.PositionOffset[0] * std::sin(yaw) + spawn.PositionOffset[2] * std::cos(yaw);
				placement.fYawDegrees = std::remainder((mapAnchor ? 0.f : owner->fYawDegrees) + spawn.fYawOffsetDegrees, 360.f);
				SERVER_NAV_POINT mapGround{};
				if (mapAnchor && (!m_ServerNavigation.Sample_Position(placement.fPositionX, placement.fPositionZ, mapGround) ||
					!std::isfinite(mapGround.y) || std::abs(mapGround.y - placement.fPositionY) > 1.f))
				{ status = "a MAP summon position does not match its navigation floor"; admitted = false; break; }
				SERVER_WORLD_ENTITY clone{};
				if (nextId == INVALID_NET_ENTITY_ID || !std::isfinite(placement.fPositionX) || !std::isfinite(placement.fPositionY) ||
					!std::isfinite(placement.fPositionZ) || !std::isfinite(placement.fYawDegrees) ||
					!m_ServerNavigation.Is_PointWalkableExact(placement.fPositionX, placement.fPositionZ) ||
					!Build_WorldEntity(placement, nextId, clone, baseCatalog, owner->iNetEntityId, owner->iPatternSequence))
				{ status = "a summoned actor failed navigation or dependent-owner admission"; admitted = false; break; }
				clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
				clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
				clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
				clone.fYawDegrees = placement.fYawDegrees;
				// Reuse the existing dependent clone lifetime and primary-loop exclusion.
				clone.bKoukuGazeClone = true;
				clone.bKoukuSummonClone = true;
				clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
				clone.iKoukuCloneEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
					CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
				if (!m_KoukuSaydonBrain.Begin_Pattern(clone, *childPattern, owner->PinnedDefinitionRevision, serverTick, status) ||
					!CKoukuSaydonBrain::Apply_StageRootMotion(clone, *childPattern, serverTick, m_ServerNavigation, m_ServerCollisionSystem, status) ||
					!Update_KoukuSummonTriggers(clone, *childPattern, serverTick))
				{ if (status.empty()) status = m_strStatus; admitted = false; break; }
				std::vector<std::uint8_t> payload;
				if (!Build_WorldEntitySpawnedPayload(clone, payload))
				{ status = "a summoned actor failed spawn wire validation"; admitted = false; break; }
				clones.push_back(std::move(clone));
				++nextId;
			}
			if (!admitted || clones.size() != trigger.PatternSpawns.size())
			{ m_strStatus = "Summon Patterns preserved all actors: " + status; continue; }
			m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
			for (auto& clone : clones)
			{
				m_WorldEntities.push_back(std::move(clone));
				Broadcast_WorldEntitySpawned(m_WorldEntities.back());
			}
			m_iNextNetEntityId = nextId;
			m_strStatus = "Summon Patterns started " + std::to_string(trigger.PatternSpawns.size()) + " independently animated clones";
			continue;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE)
		{
			const CGameplayCatalog* catalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
			if (!catalog || !m_ServerNavigation.Is_Loaded() || trigger.iCountPerPlayer < 1u || trigger.iCountPerPlayer > 8u ||
				!std::isfinite(trigger.fPlayerEffectRadiusM) || trigger.fPlayerEffectRadiusM < 0.f || trigger.fPlayerEffectRadiusM > 20.f ||
				((trigger.iCountPerPlayer == 1u) != (trigger.fPlayerEffectRadiusM == 0.f)) ||
				(trigger.bRandomPlayerOnly && trigger.iCountPerPlayer != 1u) || trigger.iArenaRandomCount > 32u ||
				!std::isfinite(trigger.fArenaRandomRadiusM) || !std::isfinite(trigger.fArenaHeightToleranceM) ||
				!std::isfinite(trigger.fArenaMinimumSpacingM) ||
				(trigger.iArenaRandomCount == 0u ?
					(trigger.fArenaRandomRadiusM != 0.f || trigger.fArenaHeightToleranceM != 0.f || trigger.fArenaMinimumSpacingM != 0.f) :
					(trigger.fArenaRandomRadiusM <= 0.f || trigger.fArenaRandomRadiusM > 100.f ||
					 trigger.fArenaHeightToleranceM <= 0.f || trigger.fArenaHeightToleranceM > 10.f ||
					 trigger.fArenaMinimumSpacingM <= 0.f || trigger.fArenaMinimumSpacingM > 20.f)) ||
				trigger.iEffectLifetimeMs < 1u || trigger.iEffectLifetimeMs > 600000u)
			{ m_strStatus = "Albion volley preserved existing objects: invalid catalog, navigation or layout"; continue; }
			BOSS_COMBAT_OBJECT_DEFINITION definition{};
			definition.strEncounterId = owner->strEncounterId;
			definition.strOwnerPatternId = owner->strPatternId;
			definition.strOwnerStageActionId = trigger.strTriggerId;
			definition.strCombatObjectArchetypeId = "combatobject.kouku.albion.bluecircle";
			definition.strClientVisualId = "combatvisual.kouku.albion.bluecircle";
			definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
			definition.iLifeMs = trigger.iEffectLifetimeMs;
			// Hit templates share each authoritative circle instance and its warning clock.
			definition.AttackTemplates = trigger.FixedHits;
			definition.PresentationPulses.push_back({ "combatpresentation.kouku.albion.started", 0u });
			BOSS_COMBAT_OBJECT_VOLLEY volley{};
			volley.ePolicy = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
			volley.iCountPerResolvedTarget = trigger.iCountPerPlayer;
			volley.iMaximumTotalObjects = 64u;
			volley.fRadiusM = trigger.fPlayerEffectRadiusM;
			volley.iArenaRandomCount = trigger.iArenaRandomCount;
			volley.fArenaRandomRadiusM = trigger.fArenaRandomRadiusM;
			volley.fArenaHeightToleranceM = trigger.fArenaHeightToleranceM;
			if (trigger.iArenaRandomCount != 0u)
				volley.eArenaAnchorPolicy = BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
			if (trigger.iCountPerPlayer > 1u)
			{
				volley.eLayout = BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
				volley.fAngleStepDegrees = 360.f / static_cast<float>(trigger.iCountPerPlayer);
			}
			const auto eligiblePlayer = [](const SERVER_PLAYER& player)
			{
				return player.isCombatReady && player.iCurrentHp != 0u && player.iMarioStage == 0u &&
					player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING;
			};
			PLAYER_ID selectedPlayerId = INVALID_PLAYER_ID;
			if (trigger.bRandomPlayerOnly)
			{
				const std::uint64_t seed = Mix_DeterministicRandom(Hash_StableId(trigger.strTriggerId) ^
					(static_cast<std::uint64_t>(owner->iNetEntityId) << 32u) ^ owner->iPatternSequence);
				std::uint64_t selectedRank = (std::numeric_limits<std::uint64_t>::max)();
				for (const auto& [playerId, player] : m_Players)
				{
					if (!eligiblePlayer(player)) continue;
					const std::uint64_t rank = Mix_DeterministicRandom(seed ^ static_cast<std::uint64_t>(playerId));
					if (selectedPlayerId == INVALID_PLAYER_ID || rank < selectedRank)
					{ selectedPlayerId = playerId; selectedRank = rank; }
				}
				if (selectedPlayerId == INVALID_PLAYER_ID)
				{ m_strStatus = "Albion volley preserved existing objects: no eligible arena player"; continue; }
			}
			BOSS_PATTERN_STAGE_ACTION arenaAction{};
			arenaAction.strTargetId = trigger.strTriggerId;
			arenaAction.Volley = volley;
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
			if (!Resolve_ArenaRandomVolleyOrigins(*owner, arenaAction, definition, 0u, arenaOrigins,
				trigger.fArenaMinimumSpacingM))
			{ m_strStatus = "Albion volley preserved existing objects: " + m_strStatus; continue; }
			auto transaction = m_CombatObjectRuntime.Begin_Transaction();
			std::string failure;
			bool admitted = true;
			for (const auto& [playerId, player] : m_Players)
			{
				if (!eligiblePlayer(player) || (trigger.bRandomPlayerOnly && playerId != selectedPlayerId))
					continue;
				SERVER_COMBAT_OBJECT_LOCKED_TARGET target{};
				target.iNetEntityId = player.iNetEntityId;
				target.fPositionX = player.fPositionX; target.fPositionY = player.fPositionY; target.fPositionZ = player.fPositionZ;
				target.bTrackUntilFirstPulse = false;
				if (transaction.Objects.size() + trigger.iCountPerPlayer + arenaOrigins.size() > volley.iMaximumTotalObjects)
				{ failure = "the resolved volley exceeds the object limit"; admitted = false; break; }
				if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction, *owner, &target, definition, &volley,
						*catalog, trigger.iCountPerPlayer, serverTick, failure))
				{ admitted = false; break; }
			}
			for (const auto& origin : arenaOrigins)
			{
				if (!admitted) break;
				if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction, *owner, &origin, definition,
					nullptr, *catalog, 1u, serverTick, failure))
				{ admitted = false; break; }
			}
			// Validate the actual staged radial poses, then keep spawn/reconnect snapshots on the same floor.
			for (std::size_t index = 0u; admitted && index < transaction.Objects.size(); ++index)
			{
				auto& object = transaction.Objects[index];
				auto& pose = object.LiveState.CurrentPose;
				SERVER_NAV_POINT ground{};
				if (!m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) ||
					!m_ServerNavigation.Sample_Position(pose.fPositionX, pose.fPositionZ, ground) || !std::isfinite(ground.y))
				{ failure = "a staged point is outside navigable ground"; admitted = false; break; }
				pose.fPositionY = ground.y;
				object.LiveState.PreviousPose = pose;
				transaction.Spawned[index].fPositionY = ground.y;
			}
			if (!admitted)
			{ m_strStatus = "Albion volley preserved existing objects: " + failure; continue; }
			if (!transaction.Objects.empty() && !m_CombatObjectRuntime.Commit(std::move(transaction)))
				m_strStatus = "Albion volley preserved existing objects: transaction revision changed";
			continue;
		}
		const CGameplayCatalog* catalog = Resolve_KoukuProductCatalog();
		const CGameplayCatalog* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
		std::string status;
		const BOSS_PATTERN_DEFINITION* clonePattern = nullptr == catalog ? nullptr :
			CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.strClonePatternId, status);
		if (nullptr == clonePattern || nullptr == baseCatalog || trigger.ClockHours.size() != 3u ||
			std::find(clonePattern->AuditionBossArchetypeIds.begin(), clonePattern->AuditionBossArchetypeIds.end(),
				owner->strArchetypeId) == clonePattern->AuditionBossArchetypeIds.end())
		{
			m_strStatus = "KoukuSaydon teleport preserved the boss: clone pattern/body is unavailable: " + status;
			continue;
		}
		const float dx = trigger.fTeleportX - owner->fSpawnPositionX;
		const float dz = trigger.fTeleportZ - owner->fSpawnPositionZ;
		const float radius = std::sqrt(dx * dx + dz * dz);
		if (!std::isfinite(radius) || radius <= 0.01f ||
			(m_ServerNavigation.Is_Loaded() &&
			 !m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ)))
		{
			m_strStatus = "KoukuSaydon teleport preserved the boss: target is not walkable";
			continue;
		}
		const float anchor = std::atan2(dx, dz);
		std::vector<SERVER_WORLD_ENTITY> clones;
		NET_ENTITY_ID nextId = m_iNextNetEntityId;
		bool admitted = true;
		for (const std::uint32_t hour : trigger.ClockHours)
		{
			const float angle = anchor + static_cast<float>(hour - 1u) * 0.52359877559829887f;
			WORLD_BOOTSTRAP_PLACEMENT placement{};
			placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			placement.strPlacementId = owner->strPlacementId + ".gaze." +
				std::to_string(owner->iPatternSequence) + "." + std::to_string(hour);
			placement.strArchetypeId = owner->strArchetypeId;
			placement.strEncounterId = owner->strEncounterId;
			placement.fPositionX = owner->fSpawnPositionX + std::sin(angle) * radius;
			placement.fPositionY = trigger.fTeleportY;
			placement.fPositionZ = owner->fSpawnPositionZ + std::cos(angle) * radius;
			placement.fYawDegrees = std::atan2(owner->fSpawnPositionX - placement.fPositionX,
				owner->fSpawnPositionZ - placement.fPositionZ) * 57.29577951308232f + trigger.fFaceCenterYawOffsetDegrees;
			SERVER_WORLD_ENTITY clone{};
			if (INVALID_NET_ENTITY_ID == nextId ||
				(m_ServerNavigation.Is_Loaded() && !m_ServerNavigation.Is_PointWalkableExact(
					placement.fPositionX, placement.fPositionZ)) ||
				!Build_WorldEntity(placement, nextId, clone, baseCatalog, owner->iNetEntityId, owner->iPatternSequence))
			{
				admitted = false;
				break;
			}
			// Static placement admission projects to cell centres; retain this admitted exact circle.
			clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
			clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
			clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
			clone.fYawDegrees = placement.fYawDegrees;
			clone.bKoukuGazeClone = true;
			clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
			clone.iKoukuCloneEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
				CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
			clone.PinnedDefinitionRevision = owner->PinnedDefinitionRevision;
			if (!m_KoukuSaydonBrain.Begin_Pattern(clone, *clonePattern,
				owner->PinnedDefinitionRevision, serverTick, status))
			{
				admitted = false;
				break;
			}
			std::vector<std::uint8_t> payload;
			if (!Build_WorldEntitySpawnedPayload(clone, payload))
			{
				admitted = false;
				break;
			}
			clones.push_back(std::move(clone));
			++nextId;
		}
		if (!admitted || clones.size() != 3u)
		{
			m_strStatus = "KoukuSaydon teleport preserved all actors: clone admission failed: " + status;
			continue;
		}
		const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
		m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
		auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
		committedOwner.fPositionX = trigger.fTeleportX;
		committedOwner.fPositionY = trigger.fTeleportY;
		committedOwner.fPositionZ = trigger.fTeleportZ;
		committedOwner.fYawDegrees = std::atan2(-dx, -dz) * 57.29577951308232f + trigger.fFaceCenterYawOffsetDegrees;
		for (auto& clone : clones)
		{
			m_WorldEntities.push_back(std::move(clone));
			Broadcast_WorldEntitySpawned(m_WorldEntities.back());
		}
		m_iNextNetEntityId = nextId;
		m_strStatus = "KoukuSaydon real boss teleported; three inward-facing clones started";
	}
	m_PendingKoukuMechanicTriggers.clear();
	Commit_KoukuMarioEntries();
}

bool LostArk::Server::CGameRoom::Update_KoukuSummonTriggers(
	SERVER_WORLD_ENTITY& clone, const BOSS_PATTERN_DEFINITION& pattern, const std::uint32_t serverTick)
{
	using Clock = CKoukuSaydonLogicRuntime;
	std::vector<std::size_t> due;
	for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
	{
		const auto& trigger = pattern.MechanicTriggers[index];
		if (std::find(clone.KoukuSummonStartedTriggers.begin(), clone.KoukuSummonStartedTriggers.end(), index) == clone.KoukuSummonStartedTriggers.end() &&
			Clock::Has_ReachedTick(serverTick, Clock::Add_Ticks(clone.iPatternStartTick, Clock::Ticks_FromMs(trigger.iStartMs))))
			due.push_back(index);
	}
	std::stable_sort(due.begin(), due.end(), [&](const auto a, const auto b) {
		const auto& left = pattern.MechanicTriggers[a]; const auto& right = pattern.MechanicTriggers[b];
		if (left.iStartMs != right.iStartMs) return left.iStartMs < right.iStartMs;
		return left.eAirbornePhase == ALBION_AIRBORNE_PHASE::JUMP && right.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP;
	});
	for (const auto index : due)
	{
		const auto& trigger = pattern.MechanicTriggers[index];
		if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
			(trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP && trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
			!Commit_KoukuAlbionAirborne(clone, pattern, trigger, serverTick)) return false;
		clone.KoukuSummonStartedTriggers.push_back(index);
	}
	return true;
}

void LostArk::Server::CGameRoom::Update_KoukuGazeClones(const std::uint32_t serverTick)
{
	for (auto clone = m_WorldEntities.begin(); clone != m_WorldEntities.end();)
	{
		if (!clone->bKoukuGazeClone)
		{
			++clone;
			continue;
		}
		const auto* owner = Find_KoukuOccurrenceOwner(clone->iOwnerBossNetEntityId, clone->iKoukuCloneOwnerSequence);
		bool live = owner != nullptr && owner->iCurrentHp > 0u &&
			!CKoukuSaydonLogicRuntime::Has_ReachedTick(serverTick, clone->iKoukuCloneEndTick);
		const auto* catalog = Resolve_KoukuProductCatalog();
		if (live && nullptr != catalog && !clone->strPatternId.empty())
		{
			std::string status;
			if (clone->bKoukuSummonClone)
			{
				const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
				live = pattern && CKoukuSaydonBrain::Apply_StageRootMotion(*clone, *pattern, serverTick,
					m_ServerNavigation, m_ServerCollisionSystem, status);
			}
			if (live)
			{
				const auto result = m_KoukuSaydonBrain.Update(*clone, *catalog, serverTick, status);
				live = KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION != result &&
					KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD != result;
				if (live && clone->bKoukuSummonClone && !clone->strPatternId.empty())
				{
					const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
					live = pattern && Update_KoukuSummonTriggers(*clone, *pattern, serverTick);
					if (!live && status.empty()) status = m_strStatus;
				}
			}
			if (!live) m_strStatus = "KoukuSaydon clone playback failed: " + status;
		}
		if (live)
		{
			++clone;
			continue;
		}
		Broadcast_WorldEntityDespawned(clone->iNetEntityId);
		clone = m_WorldEntities.erase(clone);
	}
}

void LostArk::Server::CGameRoom::Update_WorldEntities(
	const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	// Idempotent for direct simulation callers; normal ticks already prepared before players.
	Prepare_KoukuAuditionTick(updateTick);
	Update_KoukuPatternTails(updateTick);
	Update_KoukuGazeClones(updateTick);
	if (!Update_DependentBosses(updateTick))
	{
		Mark_RuntimeFailure("world-update.dependent-bosses-before-primary");
		return;
	}
	const auto releaseBossAttachments =
		[this, updateTick](const SERVER_WORLD_ENTITY& boss)
		{
			if (WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
				LostArk::Shared::INVALID_NET_ENTITY_ID != boss.iNetEntityId)
			{
				(void)Release_PlayerAttachments(
					boss.iNetEntityId, 0.f, 0u, false, 0u, updateTick);
			}
		};
	Update_PendingEstherSummons(fixedDeltaSeconds);
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		CServerBuffRuntime::Expire(entity.ActiveBuffs, updateTick);
		if (entity.bKoukuGazeClone || entity.eKind == WORLD_BOOTSTRAP_KIND::WORLD_OBJECT)
			continue;
		if (entity.isEstherSummon)
		{
			/* The clip carries its own entrance and exit; the room only clocks
			the strike so the sweep below despawns it the moment it ends. */
			entity.fActionElapsedSeconds += fixedDeltaSeconds;
			continue;
		}
		if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER ==
			entity.eDependentBossRole)
		{
			/* The dependent scheduler owns this actor's exact world transform and
			lifetime. It never enters target selection, navigation or boss combat. */
			continue;
		}
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::NPC)
		{
			const WORLD_BOOTSTRAP_PLACEMENT* placement =
				Find_Placement(entity.strPlacementId);
			if (nullptr == placement)
			{
				m_strStatus = "NPC runtime placement disappeared: " +
					entity.strPlacementId;
				Mark_RuntimeFailure("world-update.npc-placement");
				return;
			}
			if (!placement->bHasNpcBehavior)
				continue;
			const SERVER_WORLD_ENTITY* lookTarget = nullptr;
			if (!placement->NpcBehavior.strLookTargetPlacementId.empty())
			{
				const auto target = std::find_if(
					m_WorldEntities.begin(), m_WorldEntities.end(),
					[placement](const SERVER_WORLD_ENTITY& candidate)
					{
						return candidate.strPlacementId ==
							placement->NpcBehavior.strLookTargetPlacementId;
					});
				if (target != m_WorldEntities.end())
					lookTarget = &*target;
			}
			if (!m_NpcBehaviorRuntime.Update(
				*placement, lookTarget, m_ServerNavigation,
				m_ServerCollisionSystem,
				fixedDeltaSeconds, updateTick, entity, m_strStatus))
			{
				Mark_RuntimeFailure("world-update.npc-behavior");
				return;
			}
			if (!m_ServerCollisionSystem.Update_BlockingBody(
				entity.iNetEntityId,
				entity.fPositionX,
				entity.fPositionY +
					LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y,
				entity.fPositionZ))
			{
				m_strStatus = "NPC blocking body disappeared: " +
					entity.strPlacementId;
				Mark_RuntimeFailure("world-update.npc-blocking-body");
				return;
			}
			continue;
		}
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
			m_ServerNavigation.Is_Loaded() &&
			CKoukuSaydonBrain::Is_ArenaBoss(m_eWorldId, entity))
		{
			/* Every arena boss, including the enabled starting Kouku, waits
			where it spawned until an audition names it. It never enters the Valtan brain, so the gate
			button that activated it cannot start a pattern by itself. While an
			audition owns another arena boss this one only keeps its pin. */
			const bool auditionOwnsAnotherBoss =
				KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE !=
					m_KoukuSaydonPatternAudition.ePhase &&
				nullptr == Find_KoukuAuditionMember(entity.iNetEntityId);
			if (auditionOwnsAnotherBoss)
			{
				entity.PinnedDefinitionRevision =
					m_GameplayCatalog.Get_ActiveRevision();
				continue;
			}
			if (!Update_KoukuSaydonBoss(entity, updateTick))
			{
				Mark_RuntimeFailure("world-update.koukusaydon-arena-boss");
				return;
			}
            (void)m_ServerCollisionSystem.Update_BlockingBody(entity.iNetEntityId,
                entity.fPositionX, entity.fPositionY + entity.fCollisionRadius, entity.fPositionZ);
			continue;
		}
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
			m_ServerNavigation.Is_Loaded())
		{
			const CGameplayCatalog* occurrenceCatalog =
				Resolve_ValtanGameplayCatalog(entity);
			if (nullptr == occurrenceCatalog)
			{
				m_strStatus =
					"Valtan occurrence pinned gameplay generation is missing";
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.valtan-pinned-generation");
				return;
			}
			bool updateValtanBrain = true;
			const bool ghostRelocationOwnerAlive =
				entity.bGhostPhasePatternLoopActive &&
				LostArk::Shared::INVALID_NET_ENTITY_ID ==
					entity.iOwnerBossNetEntityId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				3u == entity.iPhase && 0u != entity.iCurrentHp &&
				SERVER_ENTITY_ACTION::DEAD != entity.eAction &&
				entity.strPatternId.empty();
			if ((entity.bGhostRepositionPending ||
				 entity.bGhostRelocationRetryPending) &&
				!ghostRelocationOwnerAlive)
			{
				/* Death, reset and identity replacement are ordinary cancellation
				edges. Cleanup is ownership-aware and never makes the room fatal. */
				Clear_ValtanGhostRelocationState(entity);
			}
			if (entity.bGhostRelocationRetryPending)
			{
				updateValtanBrain = false;
				if (Has_ReachedServerTick(
						updateTick, entity.iGhostRelocationRetryTick) &&
					!Begin_ValtanGhostRelocation(
						entity, *occurrenceCatalog, updateTick))
				{
					/* A transient definition/flag conflict retains the visible pose and
					retries. Unknown state does not clear another mechanic's flag. */
					entity.bGhostRelocationRetryPending = true;
					entity.iGhostRelocationRetryTick =
						Add_ServerTicksSkippingReservedZero(updateTick, 1u);
					m_strStatus = "Valtan ghost relocation retry deferred";
				}
			}
			if (entity.bGhostRepositionPending)
			{
				const bool relocationLatchValid = ghostRelocationOwnerAlive &&
					entity.bGhostRelocationOwnsInvulnerableFlag &&
					entity.bGhostRelocationOwnsHiddenFlag &&
					CBossCombatRuntime::Has_Flag(
						entity.BossCombat,
						SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) &&
					CBossCombatRuntime::Has_Flag(
						entity.BossCombat,
						SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) &&
					0u != entity.iGhostReappearTick;
				if (!relocationLatchValid)
				{
					Clear_ValtanGhostRelocationState(entity);
					if (ghostRelocationOwnerAlive)
					{
						entity.bGhostRelocationRetryPending = true;
						entity.iGhostRelocationRetryTick =
							Add_ServerTicksSkippingReservedZero(updateTick, 1u);
						updateValtanBrain = false;
					}
					m_strStatus = "Valtan ghost relocation latch was safely cancelled";
				}
				else if (Has_ReachedServerTick(
					updateTick, entity.iGhostReappearTick))
				{
					Clear_ValtanGhostRelocationState(entity);
					updateValtanBrain = true;
				}
				else
				{
					/* The completion tick already published the hidden pose. Do not let
					the ordered selector start another pattern until the next tick clears
					the render/invulnerability edge. */
					updateValtanBrain = false;
				}
			}
			if (updateValtanBrain &&
				!Prepare_ValtanPatternIdAuditionBeforeBrain(entity))
				continue;
			const std::uint32_t previousPatternSequence =
				entity.iPatternSequence;
			const std::uint32_t previousStageIndex =
				entity.iPatternStageIndex;
			const std::uint32_t previousActionStartTick =
				entity.iActionStartTick;
			const std::uint32_t previousAppliedPatternHitCount =
				entity.iAppliedPatternHitCount;
			const std::string previousPatternId = entity.strPatternId;
			const std::string previousStageId = entity.strPatternStageId;
			const std::string previousActionId = entity.strActionId;
			const LostArk::Shared::GameplayDataRevision
				previousDefinitionRevision = entity.PinnedDefinitionRevision;
			/* The Brain resolves the next stage into the boss value first. Keep the
			old value detached until every EXIT/ENTER action has passed preflight;
			on failure the stage graph rolls back while the typed mechanic ledger
			retains FAILED_REQUIRES_RESET. */
			const SERVER_WORLD_ENTITY bossBeforeBrain = entity;
			/* Where the body was before the brain moved it. The segment between
			the two is what actually touched a wall this tick, in any pattern and
			while idle, so a fast charge cannot step over a slab. */
			const float contactStartX = entity.fPositionX;
			const float contactStartY = entity.fPositionY;
			const float contactStartZ = entity.fPositionZ;
			if (updateValtanBrain)
			{
				updateValtanBrain = Prepare_ValtanFightPageBeforeBrain(
					entity, updateTick);
			}
			if (updateValtanBrain)
			{
				updateValtanBrain = Prepare_ValtanTimelineRowBeforeBrain(
					entity, updateTick);
			}
			if (updateValtanBrain)
			{
				std::vector<SERVER_PLAYER_CAPTURE_REQUEST> captureRequests;
				const BOSS_PATTERN_SEQUENCE_DEFINITION* patternFlowSequence =
					nullptr;
				patternFlowSequence =
					Resolve_ValtanPatternFlowSequence(entity);
				if (LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId)
					patternFlowSequence = &entity.DependentPatternSequence;
				else if (nullptr == patternFlowSequence &&
					entity.bGhostPhasePatternLoopActive)
					patternFlowSequence = &entity.GhostPhasePatternSequence;
				/* Only a stele that is standing right now is cover. A slot that
				   is breaking or hidden stops answering the blow on the same
				   tick the Server retired it. */
				std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>
					coverCircles;
				if (m_EncounterPropRuntime.Is_Initialized())
				{
					const float coverRadius =
						m_EncounterPropRuntime.Get_CoverRadiusMeters();
					for (const ENCOUNTER_PROP_SLOT_STATE& slot :
						m_EncounterPropRuntime.Get_SlotStates())
					{
						if (LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT !=
							slot.eState)
						{
							continue;
						}
						coverCircles.push_back({
							slot.fPositionX, slot.fPositionZ, coverRadius });
					}
				}
				/* Authored fixed combat objects become cover only until their first
				   timed damage pulse. Restrict the lookup to this boss occurrence so
				   another encounter entity cannot contribute foreign cover. */
				for (const SERVER_COMBAT_OBJECT& combatObject :
					m_CombatObjectRuntime.Get_LiveObjects())
				{
					if (combatObject.eSourceKind !=
							SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY ||
						combatObject.iSourceNetEntityId != entity.iNetEntityId ||
						!std::isfinite(combatObject.fCoverRadiusM) ||
						combatObject.fCoverRadiusM <= 0.f)
					{
						continue;
					}
					bool pendingTimedDamage = false;
					for (const SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit :
						combatObject.Hits)
					{
						if (SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED == hit.eTrigger &&
							0u == hit.iAppliedTimedCount &&
							combatObject.fElapsedMilliseconds <
								static_cast<float>(hit.iAtMs))
						{
							pendingTimedDamage = true;
							break;
						}
					}
					if (pendingTimedDamage)
					{
						coverCircles.push_back({
							combatObject.LiveState.CurrentPose.fPositionX,
							combatObject.LiveState.CurrentPose.fPositionZ,
							combatObject.fCoverRadiusM });
					}
				}
				CValtanBrain& brain = LostArk::Shared::INVALID_NET_ENTITY_ID ==
					entity.iOwnerBossNetEntityId ? m_ValtanBrain : *m_DependentValtanBrain;
				brain.Update(
					entity,
					m_Players,
					*occurrenceCatalog,
					m_ServerNavigation,
					fixedDeltaSeconds,
					updateTick,
					coverCircles,
					m_TickDamageEvents,
					&m_GameplayCatalog.Active(),
					m_GameplayCatalog.Get_ActiveGenerationEpoch(),
					&captureRequests,
					patternFlowSequence);
				for (const SERVER_PLAYER_CAPTURE_REQUEST& request : captureRequests)
				{
					(void)Capture_PlayerAttachment(
						request.iPlayerNetEntityId, entity.iNetEntityId,
						request.eAttachmentSlot, updateTick);
				}
				if (SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
					0u == entity.iCurrentHp)
				{
					Clear_ValtanGhostRelocationState(entity);
					(void)Release_PlayerAttachments(
						entity.iNetEntityId, 0.f, 0u, false, 0u, updateTick);
				}
				const VALTAN_DECISION_TRACE* latestTrace =
					m_ValtanBrain.Get_LatestDecisionTrace();
				if (LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
					nullptr != latestTrace &&
					latestTrace->iTraceSequence !=
						m_ValtanDecisionTraceRevision.iTraceSequence)
				{
					m_ValtanDecisionTraceRevision.iBossEntityId =
						entity.iNetEntityId;
					m_ValtanDecisionTraceRevision.strBossPlacementId =
						entity.strPlacementId;
					m_ValtanDecisionTraceRevision.iTraceSequence =
						latestTrace->iTraceSequence;
					m_ValtanDecisionTraceRevision.DefinitionRevision =
						occurrenceCatalog->Get_ActiveRevision();
				}
			}
			if (!previousPatternId.empty() && entity.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult)
			{
				const auto& objects = m_CombatObjectRuntime.Get_LiveObjects();
				const bool independentEffectStillRunning = std::any_of(
					objects.begin(), objects.end(),
					[&entity, &previousPatternId, previousPatternSequence](
						const SERVER_COMBAT_OBJECT& object)
					{
						return object.iSourceNetEntityId == entity.iNetEntityId &&
							object.LiveState.strOwnerPatternId == previousPatternId &&
							object.LiveState.iOwnerPatternSequence == previousPatternSequence &&
							object.fRemainingMilliseconds > 0.f;
					});
				/* The object owns its tail clock and immutable birth pose. A completed
				foreground must not insert a pursuit hold before its successor. */
				if (independentEffectStillRunning &&
					!entity.PendingPatternFollowup.Is_Pending() &&
					0u == entity.iPatternFollowupDepth)
				{
					entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
				}
			}
			bool finaleCycleRestarted = false;
			if (updateValtanBrain &&
				LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
				!previousPatternId.empty() && entity.strPatternId.empty() &&
				0u != entity.iCurrentHp && !entity.bMechanicLedgerRequiresReset &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult &&
				entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence)
			{
				const auto* definitions =
					occurrenceCatalog->Find_BossPatterns(entity.strEncounterId);
				const auto finale = nullptr == definitions ? nullptr :
					[&]() -> const BOSS_PATTERN_DEFINITION*
					{
						const auto found = std::find_if(definitions->begin(), definitions->end(),
							[&previousPatternId](const BOSS_PATTERN_DEFINITION& definition)
							{ return definition.strPatternId == previousPatternId; });
						return found == definitions->end() ? nullptr : &*found;
					}();
				bool stopRequested = entity.bAutomaticPatternSequenceAuditionHold;
				stopRequested = stopRequested ||
					(m_ValtanNextPattern &&
						m_ValtanNextPattern->iBossEntityId == entity.iNetEntityId) ||
					(Is_ValtanPatternFlowRunning() &&
						m_ValtanPatternFlowAudition.iBossEntityId == entity.iNetEntityId &&
						m_ValtanPatternFlowAudition.bStopAfterCurrent);
				if (!stopRequested && nullptr != finale &&
					BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == finale->Finale.eKind)
				{
					entity.iRotationStepIndex = bossBeforeBrain.iRotationStepIndex;
					entity.bAutomaticPatternSequenceStepRunning =
						bossBeforeBrain.bAutomaticPatternSequenceStepRunning;
					entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
					if (!m_ValtanBrain.Restart_FinaleCycle(
						entity, m_Players, *finale, updateTick))
					{
						entity = bossBeforeBrain;
						m_strStatus = "Finale cycle identity space is exhausted";
						Mark_RuntimeFailure("world-update.finale-cycle-restart");
						return;
					}
					for (auto& mechanic : entity.MechanicOccurrences)
					{
						if (mechanic.strPatternId != previousPatternId)
							continue;
						mechanic.eState = SERVER_BOSS_MECHANIC_STATE::ACTIVE;
						mechanic.iPatternSequence = entity.iPatternSequence;
						mechanic.iFinishedTick = 0u;
					}
					finaleCycleRestarted = true;
				}
			}
			(void)finaleCycleRestarted;
			const bool stageIdentityChanged =
				previousPatternSequence != entity.iPatternSequence ||
				previousStageIndex != entity.iPatternStageIndex ||
				previousPatternId != entity.strPatternId ||
				previousActionId != entity.strActionId;
			const bool pauseClockOnlyChanged =
				!stageIdentityChanged &&
				previousStageId == entity.strPatternStageId &&
				previousActionStartTick != entity.iActionStartTick &&
				(bossBeforeBrain.bAutomaticPatternSequencePausedForRevive ||
				 entity.bAutomaticPatternSequencePausedForRevive);
			const bool stageChanged = stageIdentityChanged ||
				previousStageId != entity.strPatternStageId ||
				(previousActionStartTick != entity.iActionStartTick &&
				 !pauseClockOnlyChanged);
			const auto hasGrabbedPlayerStageAction = [this, &entity](
				const std::string& patternId, const std::string& actionId,
				const LostArk::Shared::GameplayDataRevision& revision,
				const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger)
			{
				if (patternId.empty() || actionId.empty()) return false;
				const CGameplayCatalog* ownerCatalog = m_GameplayCatalog.Resolve(revision);
				const auto* definitions = nullptr == ownerCatalog ? nullptr :
					ownerCatalog->Find_BossPatterns(entity.strEncounterId);
				if (nullptr == definitions) return false;
				for (const auto& definition : *definitions)
				{
					if (definition.strPatternId != patternId) continue;
					for (const auto& stage : definition.Stages)
					{
						if (stage.strActionId != actionId) continue;
						return std::any_of(stage.Actions.begin(), stage.Actions.end(),
							[trigger](const BOSS_PATTERN_STAGE_ACTION& action)
							{
								return action.eTrigger == trigger &&
									(BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS == action.eKind ||
									 BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
									 BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind);
							});
					}
				}
				return false;
			};
			/* Both sides use the same pinned catalogs and trigger boundary as the
			transaction. Failed EXIT release must preserve attachments even when
			FinishPattern has already cleared the proposed next identity. */
			const bool preserveGrabbedPlayersOnFailure = stageIdentityChanged &&
				(hasGrabbedPlayerStageAction(previousPatternId, previousActionId,
					previousDefinitionRevision, BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT) ||
				 hasGrabbedPlayerStageAction(entity.strPatternId, entity.strActionId,
					entity.PinnedDefinitionRevision, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER));
			if (stageIdentityChanged &&
				!Apply_BossPatternStageTransition(
					entity, previousPatternId, previousActionId,
					entity.strPatternId, entity.strActionId,
					previousDefinitionRevision,
					entity.PinnedDefinitionRevision, updateTick))
			{
				auto mechanicOccurrences = std::move(entity.MechanicOccurrences);
				auto pendingPatternIds = std::move(entity.PendingPatternIds);
				auto triggeredPatternIds = std::move(entity.TriggeredPatternIds);
				const bool mechanicResetRequired =
					entity.bMechanicLedgerRequiresReset;
				const std::uint32_t lastEvaluatedHealthBar =
					entity.iLastEvaluatedHealthBar;
				const bool restorePatternVerticalOffsetAfterRollback =
					bossBeforeBrain.bPatternVerticalOffsetApplied &&
					std::isfinite(bossBeforeBrain.fPatternVerticalBaseY);
				const bool restoreStageVerticalOffsetAfterRollback =
					bossBeforeBrain.bPatternStageVerticalOffsetApplied &&
					std::isfinite(bossBeforeBrain.fPatternStageVerticalBaseY);
				const bool restoreVerticalOffsetAfterRollback =
					restorePatternVerticalOffsetAfterRollback ||
					restoreStageVerticalOffsetAfterRollback;
				const float restoredVerticalBaseY =
					restorePatternVerticalOffsetAfterRollback ?
					bossBeforeBrain.fPatternVerticalBaseY :
					bossBeforeBrain.fPatternStageVerticalBaseY;
				if (!preserveGrabbedPlayersOnFailure)
					releaseBossAttachments(entity);
				entity = bossBeforeBrain;
				/* Apply_BossPatternStageTransition already classified the occurrence as
				   aborted and restored its typed vertical offset. Do not let this
				   transaction rollback resurrect the preflight pose at Y+offset. */
				if (restoreVerticalOffsetAfterRollback)
				{
					entity.fPositionY = restoredVerticalBaseY;
					entity.bPatternVerticalOffsetApplied = false;
					entity.fPatternVerticalBaseY = 0.f;
					entity.bPatternStageVerticalOffsetApplied = false;
					entity.fPatternStageVerticalBaseY = 0.f;
				}
				entity.MechanicOccurrences = std::move(mechanicOccurrences);
				entity.PendingPatternIds = std::move(pendingPatternIds);
				entity.TriggeredPatternIds = std::move(triggeredPatternIds);
				entity.bMechanicLedgerRequiresReset = mechanicResetRequired;
				entity.iLastEvaluatedHealthBar = lastEvaluatedHealthBar;
				Mark_RuntimeFailure("world-update.pattern-stage-transition");
				return;
			}
			const bool completedRespawn = stageIdentityChanged &&
				LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
				LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"boss.valtan.center" == entity.strPlacementId &&
				"VALTAN_GHOST_RESPAWN_AUDITION" == previousPatternId &&
				entity.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult &&
				entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence;
			if (completedRespawn &&
				!Activate_ValtanGhostPhaseLoop(entity, *occurrenceCatalog))
			{
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.ghost-phase-loop-activation");
				return;
			}
			const bool completedDirectGhostLoopStep =
				entity.bGhostPhasePatternLoopActive &&
				!previousPatternId.empty() && entity.strPatternId.empty() &&
				!entity.PendingPatternFollowup.Is_Pending() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult &&
				entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence &&
				std::find(entity.GhostPhasePatternSequence.PatternIds.begin(),
					entity.GhostPhasePatternSequence.PatternIds.end(),
					previousPatternId) !=
					entity.GhostPhasePatternSequence.PatternIds.end();
			const bool completedGhostLoopOutcomeGroup =
				entity.bGhostPhasePatternLoopActive &&
				!previousPatternId.empty() &&
				entity.strPatternId.empty() &&
				!entity.PendingPatternFollowup.Is_Pending() &&
				bossBeforeBrain.iPatternFollowupDepth > 0u &&
				0u != bossBeforeBrain.iPatternFollowupRootSequence &&
				bossBeforeBrain.iRotationStepIndex > 0u &&
				bossBeforeBrain.iRotationStepIndex <=
					bossBeforeBrain.GhostPhasePatternSequence.PatternIds.size() &&
				!bossBeforeBrain.bAutomaticPatternSequenceStepRunning &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult &&
				entity.PatternTerminalReceipt.iPatternSequence ==
					previousPatternSequence &&
				entity.PatternTerminalReceipt.iRootPatternSequence ==
					bossBeforeBrain.iPatternFollowupRootSequence;
			const bool completedGhostLoopStep =
				completedDirectGhostLoopStep || completedGhostLoopOutcomeGroup;
			if (completedGhostLoopStep)
			{
				entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
				if (entity.iRotationStepIndex >=
					entity.GhostPhasePatternSequence.PatternIds.size())
				{
					entity.iRotationStepIndex = 0u;
				}
			}
			if (!previousPatternId.empty() &&
				previousPatternId != entity.strPatternId)
			{
				/* A pattern owns every attachment it captured. Even a catalog-corrupt
				exit without its authored release action cannot leak that ownership
				into the next pattern. */
				releaseBossAttachments(entity);
			}
			if (!previousPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE != bossBeforeBrain.eAction &&
				SERVER_ENTITY_ACTION::IDLE == entity.eAction)
			{
				/* A mechanic-reset latch can abort motion without changing the stage
				identity. Attachment cleanup follows the action abort edge as well. */
				releaseBossAttachments(entity);
			}
			if (!stageChanged && updateValtanBrain &&
				!Apply_BossPatternScheduledSpawnWave(entity, updateTick))
			{
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.pattern-scheduled-spawn-wave");
				return;
			}
			if (stageChanged && !Apply_EncounterPropStageEntry(entity, updateTick))
			{
				CValtanBrain::Fail_ActiveMechanic(entity,
					SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, updateTick);
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.encounter-prop-stage-entry");
				return;
			}
			if (stageChanged && !Apply_WorldDestructionStageEntry(
				entity, updateTick))
			{
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.world-destruction-stage-entry");
				return;
			}
			/* A charge owns one swept wall transaction below. Letting the stationary
			   body-contact pass run first can break every overlapping wall box before
			   the first surface chooses its single impact/contact mutation. */
			if (!entity.bPatternChargeImpact && !entity.bPortalMotionActive &&
				!bossBeforeBrain.bPortalMotionActive &&
				!Apply_WorldDestructionBodyContact(
				entity, contactStartX, contactStartY, contactStartZ,
				updateTick))
			{
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.world-destruction-body-contact");
				return;
			}
			/* A damage pulse is evaluated by the Brain at the same fixed tick as
			   the axe proxy. Only stages compiled in the wall-contact allowlist set
			   bPatternWallContact, so roars, waves and magic never reach here. */
			if (entity.bPatternWallContact &&
				entity.iAppliedPatternHitCount > previousAppliedPatternHitCount &&
				!Apply_WorldDestructionPatternHitContact(entity, updateTick))
			{
				releaseBossAttachments(entity);
				Mark_RuntimeFailure("world-update.world-destruction-pattern-hit");
				return;
			}
			float proposedX = 0.f;
			float proposedZ = 0.f;
			if (!entity.bAutomaticPatternSequencePausedForRevive &&
				m_ValtanBrain.Try_BuildStageMotion(
				entity, fixedDeltaSeconds, proposedX, proposedZ))
			{
				const bool portalTargetRush =
					BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
						entity.ePatternStageMotionKind;
				const auto Take_MotionStep =
					[this, &entity, portalTargetRush](
						const float targetX, const float targetZ)
					{
						if (portalTargetRush)
						{
							entity.fPositionX = targetX;
							entity.fPositionZ = targetZ;
							return;
						}
						Resolve_NavigableStep(
							m_ServerNavigation,
							entity.fPositionX, entity.fPositionZ,
							targetX, targetZ,
							entity.fPositionX, entity.fPositionZ);
					};
				/* Dash Charge stops on the first authoritative collisionBox, including
				ordinary walls. Other authored charge-impact mechanics retain their
				exact receiver-only contract. */
				const bool dashStopsOnEveryWall =
					"VALTAN_DASH_CHARGE" == entity.strPatternId &&
					"CHARGE" == entity.strPatternStageId &&
					"valtan.attack.dash-charge.active" == entity.strActionId;
				SERVER_BOSS_WALL_HIT hit{};
				bool foundChargeWall = false;
				if (entity.bPatternChargeImpact && dashStopsOnEveryWall)
				{
					foundChargeWall =
						m_ServerCollisionSystem.Sweep_BossCircleAgainstWalls(
						entity.fPositionX, entity.fPositionY, entity.fPositionZ,
						proposedX, entity.fPositionY, proposedZ,
						entity.fCollisionRadius, hit);
				}
				else if (entity.bPatternChargeImpact)
				{
					SERVER_BOSS_RECEIVER_HIT receiverHit{};
					foundChargeWall =
						m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
							entity.fPositionX, entity.fPositionY,
							entity.fPositionZ, proposedX, entity.fPositionY,
							proposedZ, entity.fCollisionRadius, receiverHit);
					if (foundChargeWall)
					{
						hit.strCollisionPlacementId =
							receiverHit.strReceiverPlacementId;
						hit.strImpactReceiverPlacementId =
							receiverHit.strReceiverPlacementId;
						hit.fHitRatio = receiverHit.fHitRatio;
					}
				}
				if (foundChargeWall)
				{
					const float deltaX = proposedX - entity.fPositionX;
					const float deltaZ = proposedZ - entity.fPositionZ;
					const float distance = std::sqrt(
						deltaX * deltaX + deltaZ * deltaZ);
					const float marginRatio = distance > 0.000001f ?
						0.001f / distance : 0.f;
					const float safeRatio = (std::max)(
						0.f, hit.fHitRatio - marginRatio);
					Take_MotionStep(
						entity.fPositionX + deltaX * safeRatio,
						entity.fPositionZ + deltaZ * safeRatio);
					const SERVER_WORLD_ENTITY bossBeforeImpactTransition = entity;
					const std::string impactPreviousPatternId = entity.strPatternId;
					const std::string impactPreviousActionId = entity.strActionId;
					bool triggered = false;
					if (!hit.strImpactReceiverPlacementId.empty() &&
						!Apply_WorldDestructionImpact(
							entity, hit.strImpactReceiverPlacementId,
							updateTick, triggered))
					{
						Mark_RuntimeFailure("world-update.world-destruction-impact");
						return;
					}
					if (dashStopsOnEveryWall && !triggered &&
						!Apply_WorldDestructionContacts(
						entity, { hit.strCollisionPlacementId }, updateTick))
					{
						Mark_RuntimeFailure("world-update.world-destruction-contact");
						return;
					}
					if (!dashStopsOnEveryWall && !triggered)
					{
						entity.fPatternForcedMotionSpeed = 0.f;
					}
					/* A fresh receiver publishes WALL_CONTACT as part of its exact
					   mutation. Ordinary walls and already-consumed receiver surfaces
					   publish the same geometry outcome without manufacturing another
					   destruction transition. */
					const bool impactOutcomePublished = !dashStopsOnEveryWall ||
						triggered ||
						CBossCombatRuntime::Publish_PatternOutcome(
							entity, BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
							updateTick);
					const CGameplayCatalog* impactCatalog =
						m_GameplayCatalog.Resolve(entity.PinnedDefinitionRevision);
					if ((dashStopsOnEveryWall || triggered) &&
						(!impactOutcomePublished || nullptr == impactCatalog ||
						!m_ValtanBrain.Complete_ImpactStage(
							entity, *impactCatalog, updateTick) ||
						!Apply_BossPatternStageTransition(
							entity, impactPreviousPatternId,
							impactPreviousActionId, entity.strPatternId,
							entity.strActionId,
							bossBeforeImpactTransition.PinnedDefinitionRevision,
							entity.PinnedDefinitionRevision, updateTick) ||
						!Apply_WorldDestructionStageEntry(entity, updateTick)))
					{
						auto mechanicOccurrences =
							std::move(entity.MechanicOccurrences);
						const bool mechanicResetRequired =
							entity.bMechanicLedgerRequiresReset;
						releaseBossAttachments(entity);
						entity = bossBeforeImpactTransition;
						entity.MechanicOccurrences =
							std::move(mechanicOccurrences);
						entity.bMechanicLedgerRequiresReset =
							mechanicResetRequired;
						m_strStatus = "Valtan wall-contact stage transition failed";
						Mark_RuntimeFailure("world-update.wall-contact-stage-transition");
						return;
					}
					/* Complete_ImpactStage resolves the authored WALL_CONTACT branch;
					   GameRoom owns the shared wall-stop/destruction transaction order. */
				}
				else
				{
					Take_MotionStep(proposedX, proposedZ);
					const float yaw = entity.fYawDegrees * DEGREES_TO_RADIANS;
					const float probeDistance = (std::max)(0.1f,
						m_ServerNavigation.Get_CellSize());
					const bool blocked = !portalTargetRush && (
						std::fabs(entity.fPositionX - proposedX) > 0.001f ||
						std::fabs(entity.fPositionZ - proposedZ) > 0.001f ||
						!m_ServerNavigation.Is_PointWalkableExact(
							entity.fPositionX + std::sin(yaw) * probeDistance,
							entity.fPositionZ + std::cos(yaw) * probeDistance));
					if (blocked &&
						BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == entity.ePatternPlayerResponse)
					{
						const SERVER_WORLD_ENTITY beforeBlocked = entity;
						if (m_ValtanBrain.Complete_NavigationBlockedStage(
							entity, m_Players, *occurrenceCatalog, updateTick) &&
							(!Apply_BossPatternStageTransition(entity,
								beforeBlocked.strPatternId, beforeBlocked.strActionId,
								entity.strPatternId, entity.strActionId,
								beforeBlocked.PinnedDefinitionRevision,
								entity.PinnedDefinitionRevision, updateTick) ||
							 !Apply_EncounterPropStageEntry(entity, updateTick) ||
							 !Apply_WorldDestructionStageEntry(entity, updateTick)))
						{
							entity = beforeBlocked;
							entity.bMechanicLedgerRequiresReset = true;
							m_strStatus = "Capture charge navigation outcome could not commit";
							Mark_RuntimeFailure("world-update.capture-charge-navigation-outcome");
							return;
						}
					}
				}
			}
			if (finaleCycleRestarted &&
				m_ValtanPatternIdAudition.iBossEntityId == entity.iNetEntityId &&
				VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
					m_ValtanPatternIdAudition.ePhase &&
				m_ValtanPatternIdAudition.iExpectedPatternSequence == previousPatternSequence)
			{
				m_ValtanPatternIdAudition.iExpectedPatternSequence = entity.iPatternSequence;
				Queue_ValtanPatternIdAuditionLifecycle(
					LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
			}
			if (updateValtanBrain)
				Restore_ValtanTimelineRowAfterBrain(entity, updateTick);
			if (updateValtanBrain)
				Refresh_ValtanPatternFlowState(entity);
			/* Debug controllers consume the root-group receipt above. Product has no
			   controller, so retire the leaf-only bookkeeping at the same post-brain
			   seam; the terminal receipt keeps the immutable root identity. */
			if (entity.strPatternId.empty() &&
				!entity.PendingPatternFollowup.Is_Pending() &&
				entity.iPatternFollowupDepth > 0u &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					entity.PatternTerminalReceipt.eResult &&
				entity.PatternTerminalReceipt.iPatternSequence ==
					previousPatternSequence &&
				entity.PatternTerminalReceipt.iRootPatternSequence ==
					entity.iPatternFollowupRootSequence)
			{
				entity.iPatternFollowupDepth = 0u;
				entity.iPatternFollowupRootSequence = 0u;
			}
			if (LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
				entity.strPatternId.empty() &&
				occurrenceCatalog->Get_ActiveRevision() ==
					m_GameplayCatalog.Get_ActiveRevision())
			{
				/* Publish the active identity only after the brain has observed it.
				If an old occurrence finished on this tick, retaining its pin for one
				more boundary makes the next active-catalog evaluation detectable. */
				entity.PinnedDefinitionRevision =
					m_GameplayCatalog.Get_ActiveRevision();
			}
		}
		else if (entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
			m_ServerNavigation.Is_Loaded())
		{
			if (CMonsterBrain::Advance_Knockback(
				entity, m_ServerNavigation, m_ServerCollisionSystem,
				fixedDeltaSeconds))
			{
				(void)m_ServerCollisionSystem.Update_BlockingBody(
					entity.iNetEntityId,
					entity.fPositionX,
					entity.fPositionY + entity.fCollisionRadius,
					entity.fPositionZ);
				continue;
			}
			m_MonsterBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				m_ServerCollisionSystem,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
			/* Later entities in this deterministic vector order collide against
			the position accepted earlier in the same fixed tick. */
			(void)m_ServerCollisionSystem.Update_BlockingBody(
				entity.iNetEntityId,
				entity.fPositionX,
				entity.fPositionY + entity.fCollisionRadius,
				entity.fPositionZ);
		}
	}

	Tick_MvpLedgers();

	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		// Only the Product arena's primary Valtan completes the raid. Dependent
		// ghost bosses share BOSS runtime state, and Character Select can spawn a
		// Valtan audition, but neither is a reward authority. The entity latch
		// prevents the retained DEAD presentation from granting again.
		if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId ||
			WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind ||
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			"BOSS_VALTAN" != entity.strArchetypeId ||
			"boss.valtan.center" != entity.strPlacementId ||
			SERVER_ENTITY_ACTION::DEAD != entity.eAction ||
			entity.bLootGranted)
		{
			continue;
		}
		entity.bLootGranted = true;
		m_bValtanRaidCleared = true;
		m_GateMvpLedger.clear();
		Merge_MvpLedger(entity);
		Broadcast_RaidMvpResult(1u);
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			const auto playerIter = m_Players.find(playerId);
			if (playerIter == m_Players.end())
				continue;
			for (const std::string& itemId :
				m_ValtanClearRewards.Get_ItemIds(playerIter->second.eCharacterClass))
				(void)Grant_Item(playerIter->second, itemId, 1u);
			const std::shared_ptr<CClientSession> session =
				Find_Session(sessionId);
			if (nullptr != session &&
				!Send_InventorySnapshot(
					session, 0u, playerIter->second.Inventory))
			{
				session->Request_Close();
			}
		}
	}

	for (auto iter = m_WorldEntities.begin(); iter != m_WorldEntities.end();)
	{
		const bool shouldDespawn =
			(WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind &&
				(0u == iter->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == iter->eAction)) ||
			(WORLD_BOOTSTRAP_KIND::MONSTER == iter->eKind &&
				SERVER_ENTITY_ACTION::DEAD == iter->eAction &&
				iter->fActionElapsedSeconds * 1000.f >=
					static_cast<float>(iter->iDeadDespawnMs)) ||
			(iter->isEstherSummon &&
				iter->fActionElapsedSeconds * 1000.f >=
					static_cast<float>(iter->iEstherStrikeMs));
		if (!shouldDespawn)
		{
			++iter;
			continue;
		}
		if (WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind)
		{
			/* A gate boss's award ledger goes to the room before the clear is decided. */
			if (Gate_IndexOfPlacement(iter->strPlacementId) >= 0)
				Merge_MvpLedger(*iter);
			Notify_KoukuRaidBossDeath(*iter, updateTick);
			if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
				"ENCOUNTER_VALTAN" == iter->strEncounterId &&
				"BOSS_VALTAN" == iter->strArchetypeId)
			{
				Clear_ValtanGhostRelocationState(*iter);
			}
			(void)Release_PlayerAttachments(
				iter->iNetEntityId, 0.f, 0u, false, 0u, updateTick);
		}
		m_CombatObjectRuntime.Cancel_Source(iter->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
		{
			Mark_RuntimeFailure("world-update.dead-entity-combat-object-lifecycle");
			return;
		}
		Broadcast_WorldEntityDespawned(iter->iNetEntityId,
			WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind ?
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::DEAD :
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		/* Gate progress: the last primary boss of a gate dying is that gate's clear. */
		if (WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind)
			Notify_GateBossDeath(*iter);
		iter = m_WorldEntities.erase(iter);
	}
	Commit_KoukuMechanicTriggers(updateTick);
	if (!Update_DependentBosses(updateTick))
		Mark_RuntimeFailure("world-update.dependent-bosses-after-primary");
}
