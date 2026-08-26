#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <utility>

namespace
{
	using namespace LostArk::Shared;
	using namespace LostArk::Server;

	/* The pillars rise at the tail of the one pillar pattern the encounter
	   already owns. The video shows three further cycles, but no product
	   pattern, stage or binding is identified for them and none for the shatter
	   either, so the product path binds the raise alone and the Debug audition
	   drives a whole cycle for verification. */
	constexpr const char* PILLAR_PATTERN_ID = "VALTAN_FOUR_PILLARS_105";
	constexpr const char* PILLAR_SPAWN_STAGE_ID = "RECOVERY";
	/* 250ms at the fixed 30Hz tick, the same shatter window the walls use. */
	constexpr std::uint32_t PILLAR_BREAKING_TICKS = 8u;
	/* Debug audition only. The recording raises the pillars at 16:42.74 and
	   starts the shatter at 16:58.56, so one audition cycle reproduces that
	   15.82s dwell. No product trigger reads this. */
	constexpr std::uint32_t PILLAR_AUDITION_DWELL_TICKS = 475u;

	constexpr float MAX_ABS_MOVE_GOAL = 10000.f;
	constexpr float MOVE_STOP_DISTANCE = 0.05f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float DEGREES_TO_RADIANS = 0.01745329251994329577f;
	constexpr float PLAYER_TURN_DEGREES_PER_SECOND = 540.f;
	constexpr float DIRECT_BEARING_DISTANCE = 1.5f;
	constexpr float VOLLEY_SPACING_EPSILON = 0.0001f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;
	constexpr std::uint32_t ARENA_RANDOM_MAXIMUM_ATTEMPTS = 512u;
	constexpr float TWO_PI = 6.28318530717958647692f;
	// The caster's Esther call converted to fixed 30 Hz room ticks.
	constexpr std::uint32_t ESTHER_CAST_TICKS =
		(ESTHER_CAST_DURATION_MS * 30u + 999u) / 1000u;

	float Resolve_VolleyMinimumSpacing(
		const BOSS_COMBAT_OBJECT_DEFINITION& definition)
	{
		float maximumExtent = 0.f;
		for (const BOSS_COMBAT_OBJECT_HIT& hit : definition.Hits)
		{
			float extent = 0.f;
			switch (hit.eHitShape)
			{
			case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			case BOSS_PATTERN_HIT_SHAPE::RING:
				extent = hit.fHitOuterRadius;
				break;
			case BOSS_PATTERN_HIT_SHAPE::CONE:
				extent = hit.fHitLength;
				break;
			case BOSS_PATTERN_HIT_SHAPE::BOX:
			case BOSS_PATTERN_HIT_SHAPE::CROSS:
			case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
				extent = std::sqrt(
					hit.fHitLength * hit.fHitLength +
					hit.fHitHalfWidth * hit.fHitHalfWidth);
				break;
			default:
				break;
			}
			maximumExtent = (std::max)(maximumExtent, extent);
		}
		return maximumExtent * 2.f;
	}

	std::uint64_t Mix_DeterministicRandom(std::uint64_t value)
	{
		value += 0x9e3779b97f4a7c15ull;
		value = (value ^ (value >> 30u)) * 0xbf58476d1ce4e5b9ull;
		value = (value ^ (value >> 27u)) * 0x94d049bb133111ebull;
		return value ^ (value >> 31u);
	}

	std::uint64_t Hash_StableId(const std::string& value)
	{
		std::uint64_t hash = 1469598103934665603ull;
		for (const unsigned char character : value)
		{
			hash ^= character;
			hash *= 1099511628211ull;
		}
		return hash;
	}

	float DeterministicUnitFloat(const std::uint64_t value)
	{
		return static_cast<float>((Mix_DeterministicRandom(value) >> 40u) &
			0xffffffull) / 16777216.f;
	}

	void Build_WorldDestructionStateChanges(
		const WORLD_DESTRUCTION_TRANSACTION& transaction,
		std::vector<SERVER_COLLISION_STATE_CHANGE>& collisionChanges,
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& navigationChanges)
	{
		collisionChanges.clear();
		navigationChanges.clear();
		for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition :
			transaction.Transitions)
		{
			if (!transition.strCollisionStateId.empty())
			{
				SERVER_COLLISION_STATE_CHANGE change{};
				change.strPlacementId = transition.strCollisionStateId;
				change.bPlayerBlocking =
					WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState ||
					WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState;
				change.bImpactReceiverEnabled =
					WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState;
				collisionChanges.push_back(std::move(change));
			}
			if (transition.bApplyPersistentMutation &&
				!transition.strNavigationStateId.empty())
			{
				SERVER_NAVIGATION_CONDITION_CHANGE change{};
				change.strConditionId = transition.strNavigationStateId;
				change.bValue =
					WORLD_DESTRUCTION_STATE::FRACTURED == transition.eNextState ||
					WORLD_DESTRUCTION_STATE::DESPAWNED == transition.eNextState;
				navigationChanges.push_back(std::move(change));
			}
		}
	}

	std::uint64_t To_Microseconds(
		const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				duration).count());
	}

	bool Is_BestEffortCommand(const ROOM_COMMAND_TYPE type)
	{
		return ROOM_COMMAND_TYPE::MOVE == type ||
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == type;
	}

	bool Is_SameBestEffortStream(
		const ROOM_COMMAND& queued,
		const ROOM_COMMAND& incoming)
	{
		if (queued.iSessionId != incoming.iSessionId ||
			queued.eType != incoming.eType ||
			!Is_BestEffortCommand(incoming.eType))
		{
			return false;
		}
		if (ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == incoming.eType)
		{
			return queued.UpdateSkillAim.iSkillId ==
				incoming.UpdateSkillAim.iSkillId;
		}
		return true;
	}

	bool Try_RemoveCoalescedCommand(
		std::deque<ROOM_COMMAND>& commands,
		const ROOM_COMMAND& incoming)
	{
		// A command from the same session is an ordering barrier. This keeps a
		// newer move/aim from jumping across skill, release, or lifecycle intent.
		for (std::size_t index = commands.size(); index > 0u; --index)
		{
			const ROOM_COMMAND& queued = commands[index - 1u];
			if (queued.iSessionId != incoming.iSessionId)
				continue;
			if (!Is_SameBestEffortStream(queued, incoming))
				return false;
			commands.erase(commands.begin() +
				static_cast<std::ptrdiff_t>(index - 1u));
			return true;
		}
		return false;
	}

	float Wrap_Degrees(float degrees)
	{
		while (degrees > 180.f)
			degrees -= 360.f;
		while (degrees < -180.f)
			degrees += 360.f;
		return degrees;
	}

	/* An arena floor collapse drops the player for one and a half seconds and
	then kills. The room steps at a fixed 30 Hz, so the deadline is 45 ticks
	after the fall begins. Gravity is the plain metric constant: the descent
	only presents a death the collapse already decided. */
	constexpr float FALL_GRAVITY_METERS_PER_SECOND_SQUARED = 9.8f;
	constexpr std::uint32_t FALL_DEATH_TICKS = 45u;

	constexpr std::uint32_t Add_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick,
		const std::uint32_t elapsedTicks)
	{
		constexpr std::uint64_t SERVER_TICK_CARDINALITY =
			(static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)()));
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(startTick - 1u) + elapsedTicks) %
				SERVER_TICK_CARDINALITY) + 1u);
	}

	std::uint64_t Elapsed_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick,
		const std::uint32_t currentTick)
	{
		if (0u == startTick || 0u == currentTick)
			return 0u;
		return currentTick >= startTick ?
			static_cast<std::uint64_t>(currentTick - startTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - startTick) +
				currentTick;
	}

	static_assert(91u == Add_ServerTicksSkippingReservedZero(1u, 90u));
	static_assert(90u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)(), 90u));
	static_assert(1u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)() - 89u, 90u));
	static_assert(static_cast<std::uint32_t>(
		SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::INVULNERABLE));
	static_assert(static_cast<std::uint32_t>(
		SERVER_BOSS_COMBAT_FLAG::SHIELDED) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::SHIELDED));
	static_assert(static_cast<std::uint32_t>(
		SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::COUNTERABLE));
	static_assert(static_cast<std::uint32_t>(
		SERVER_BOSS_COMBAT_FLAG::GROGGY) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GROGGY));
	static_assert(
		(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::SHIELDED) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GROGGY)) ==
		BOSS_COMBAT_STATE_KNOWN_FLAG_MASK);

	bool Has_ReachedServerTick(
		const std::uint32_t currentTick,
		const std::uint32_t targetTick)
	{
		return currentTick == targetTick ||
			static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}
#ifdef _DEBUG
	constexpr std::uint32_t CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS = 90u;
	constexpr const char* VALTAN_ARENA_AUDITION_PLACEMENT_ID =
		"boss.valtan.center";
	constexpr const char* CHARACTER_SELECT_AUDITION_PLACEMENT_ID =
		"boss.valtan.character-select.lazy";
	constexpr std::array<const char*, 6u>
		CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS{
			"VALTAN_ARMOR_BREAK_OPENING",
			"VALTAN_ENTRANCE_WHIRLWIND",
			"VALTAN_ARENA_BREAK_109",
			"VALTAN_ARENA_BREAK_84",
			"VALTAN_ARENA_BREAK_33",
			"VALTAN_FOUR_PILLARS_105" };

	bool Is_CharacterSelectEnvironmentDependentPattern(
		const std::string& patternId)
	{
		return std::any_of(
			CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS.begin(),
			CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS.end(),
			[&patternId](const char* blockedPatternId)
			{
				return patternId == blockedPatternId;
			});
	}

	constexpr const char* WALL_ATTACK_PATTERN_ID = "VALTAN_DOWN_SMASH";
	constexpr const char* FINAL_ARENA_PATTERN_ID = "VALTAN_ARENA_BREAK_109";
	constexpr const char* FINAL_ARENA_STAGE_ID = "IMPACT";
	constexpr const char* FINAL_ARENA_ACTION_ID =
		"valtan.mechanic.arena-break-109.impact";
	/* The final arena is the arena after the whole shrink chain, so the Debug
	   view also stages the two floor collapses. Stage A drops the outer rail at
	   84 bars and stage B drops the brick ring at the 30-bar landing. */
	constexpr const char* FINAL_ARENA_FLOOR_A_PATTERN_ID =
		"VALTAN_ARENA_BREAK_84";
	constexpr const char* FINAL_ARENA_FLOOR_A_STAGE_ID = "IMPACT";
	constexpr const char* FINAL_ARENA_FLOOR_A_ACTION_ID =
		"valtan.mechanic.arena-floor-84.impact";
	constexpr std::uint32_t FINAL_ARENA_FLOOR_A_STAGE_INDEX = 1u;
	constexpr const char* FINAL_ARENA_FLOOR_B_PATTERN_ID =
		"VALTAN_ARENA_BREAK_33";
	constexpr const char* FINAL_ARENA_FLOOR_B_STAGE_ID = "LANDING";
	constexpr const char* FINAL_ARENA_FLOOR_B_ACTION_ID =
		"valtan.mechanic.arena-break-33.landing";
	constexpr std::uint32_t FINAL_ARENA_FLOOR_B_STAGE_INDEX = 1u;
	/* A known ordinary wall used only by the Debug attack audition. The boss is
	   kept six metres outside its centre so body contact cannot pre-empt the
	   DOWN_SMASH hit pulse, while the player is projected onto the inner side. */
	constexpr float WALL_ATTACK_CENTER_X = 161.402061f;
	constexpr float WALL_ATTACK_CENTER_Y = 23.04f;
	constexpr float WALL_ATTACK_CENTER_Z = -133.312236f;
	constexpr float WALL_ATTACK_BOSS_OFFSET_Z = -6.f;
	constexpr float WALL_ATTACK_PLAYER_OFFSET_Z = 8.f;

	void Freeze_TimelineAuditionPlayer(SERVER_PLAYER& player)
	{
		player.eAction = 0u == player.iCurrentHp ?
			PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iActionStartTick = 0u;
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;
		player.fActionElapsedSeconds = 0.f;
		player.fSkillAimDirectionX = 0.f;
		player.fSkillAimDirectionZ = 1.f;
		player.fSkillAimDistance = 0.f;
		player.hasAppliedSkillDamage = false;
		player.iAppliedHitMask = 0u;
		player.iSpawnedProjectileMask = 0u;
		player.Projectiles.clear();
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.fBufferedComboAimX = 0.f;
		player.fBufferedComboAimZ = 1.f;
		player.fBufferedComboAimDistance = 0.f;
		player.hasReleasedHold = false;
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.isCombatReady = false;
	}

	void Prepare_TimelineAuditionPlayer(
		SERVER_PLAYER& player,
		const std::uint32_t actionTick)
	{
		Freeze_TimelineAuditionPlayer(player);
		player.iCurrentHp = player.iMaximumHp;
		player.iCurrentResource = player.iMaximumResource;
		player.iResourceAccumulator = 0u;
		player.iCurrentIdentity = player.iMaximumIdentity;
		player.iIdentityAccumulator = 0u;
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = actionTick;
		player.isCombatReady = true;
	}
#endif

	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message)
	{
		return message.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			Is_Known_World_Id(message.eWorldId) &&
			Is_Supported_Playable_Character_Class(message.eCharacterClass) &&
			Is_Valid_PlayerNickname(message.strNickName);
	}

	bool Is_NewerSequence(
		const std::uint32_t candidate,
		const std::uint32_t previous)
	{
		return 0u != candidate &&
			static_cast<std::int32_t>(candidate - previous) > 0;
	}

	void Smooth_MovePath(
		const CServerNavigation& navigation,
		const float startX,
		const float startZ,
		const float goalX,
		const float goalZ,
		std::vector<SERVER_NAV_POINT>& path)
	{
		if (path.empty())
			return;
		SERVER_NAV_POINT exactGoal{};
		if (navigation.Sample_Position(goalX, goalZ, exactGoal) &&
			navigation.Has_LineOfSight(
				path.back().x, path.back().z, goalX, goalZ))
		{
			path.back() = exactGoal;
		}
		std::vector<SERVER_NAV_POINT> smoothed;
		smoothed.reserve(path.size());
		float fromX = startX;
		float fromZ = startZ;
		std::size_t index = 0;
		while (index < path.size())
		{
			std::size_t visible = index;
			for (std::size_t candidate = path.size();
				candidate > index + 1; --candidate)
			{
				const SERVER_NAV_POINT& point = path[candidate - 1];
				if (navigation.Has_LineOfSight(fromX, fromZ, point.x, point.z))
				{
					visible = candidate - 1;
					break;
				}
			}
			smoothed.push_back(path[visible]);
			fromX = path[visible].x;
			fromZ = path[visible].z;
			index = visible + 1;
		}
		path = std::move(smoothed);
	}

	WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind)
	{
		switch (kind)
		{
		case WORLD_BOOTSTRAP_KIND::NPC: return WORLD_ENTITY_KIND::NPC;
		case WORLD_BOOTSTRAP_KIND::BOSS: return WORLD_ENTITY_KIND::BOSS;
		case WORLD_BOOTSTRAP_KIND::MONSTER: return WORLD_ENTITY_KIND::MONSTER;
		default: return WORLD_ENTITY_KIND::END;
		}
	}

	WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action)
	{
		switch (action)
		{
		case SERVER_ENTITY_ACTION::IDLE: return WORLD_ENTITY_ACTION::IDLE;
		case SERVER_ENTITY_ACTION::CHASE: return WORLD_ENTITY_ACTION::CHASE;
		case SERVER_ENTITY_ACTION::PATTERN_WINDUP: return WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		case SERVER_ENTITY_ACTION::PATTERN_ACTIVE: return WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		case SERVER_ENTITY_ACTION::PATTERN_RECOVERY: return WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		case SERVER_ENTITY_ACTION::DEAD: return WORLD_ENTITY_ACTION::DEAD;
		default: return WORLD_ENTITY_ACTION::END;
		}
	}

	WORLD_DESTRUCTION_RUNTIME_STATE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_STATE state)
	{
		switch (state)
		{
		case WORLD_DESTRUCTION_STATE::INTACT:
			return WORLD_DESTRUCTION_RUNTIME_STATE::INTACT;
		case WORLD_DESTRUCTION_STATE::BREAKING:
			return WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING;
		case WORLD_DESTRUCTION_STATE::FRACTURED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED;
		case WORLD_DESTRUCTION_STATE::DESPAWNED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED;
		default:
			return WORLD_DESTRUCTION_RUNTIME_STATE::END;
		}
	}

	WORLD_DESTRUCTION_STATE_WIRE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_GROUP_STATE& state)
	{
		WORLD_DESTRUCTION_STATE_WIRE wire{};
		wire.strGroupId = state.strGroupId;
		wire.eState = To_NetworkDestructionState(state.eState);
		wire.iStateVersion = state.iStateVersion;
		wire.iStateStartTick = state.iStateStartTick;
		wire.iCommitTick = state.iCommitTick;
		return wire;
	}

	std::uint32_t Hash_DestructionEventIdentity(
		const std::uint32_t encounterEpoch,
		const std::uint64_t eventSequence,
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition,
		const WORLD_DESTRUCTION_BINDING_APPLICATION& application,
		const std::uint32_t serverTick)
	{
		std::uint32_t hash = 2166136261u;
		const auto appendByte = [&hash](const std::uint8_t value)
		{
			hash ^= value;
			hash *= 16777619u;
		};
		const auto appendU64 = [&appendByte](const std::uint64_t value)
		{
			for (std::uint32_t shift = 0u; shift < 64u; shift += 8u)
				appendByte(static_cast<std::uint8_t>(value >> shift));
		};
		const auto appendString = [&appendByte](const std::string& value)
		{
			for (const unsigned char character : value)
				appendByte(character);
			appendByte(0xffu);
		};

		appendU64(encounterEpoch);
		appendU64(eventSequence);
		appendString(transition.strGroupId);
		appendString(transition.strMutationId);
		appendString(application.strBindingId);
		appendU64(application.iPatternSequence);
		appendU64(application.iSourceNetEntityId);
		appendU64(serverTick);
		return 0u == hash ? 1u : hash;
	}
}

LostArk::Server::CGameplayCatalogGenerations::CGameplayCatalogGenerations()
{
	m_Generations.reserve(MAX_GENERATION_COUNT);
}

bool LostArk::Server::CGameplayCatalogGenerations::Load()
{
	auto initial = std::make_shared<CGameplayCatalog>();
	if (nullptr == initial || !initial->Load())
	{
		m_strStatus = nullptr == initial ?
			"Gameplay catalog allocation failed" : initial->Get_Status();
		return false;
	}
	return Initialize(std::move(initial));
}

bool LostArk::Server::CGameplayCatalogGenerations::Initialize(
	const std::shared_ptr<const CGameplayCatalog>& initialGeneration)
{
	if (nullptr == initialGeneration ||
		!initialGeneration->Get_ActiveRevision().Is_Valid())
	{
		m_strStatus = "Initial gameplay generation is invalid";
		return false;
	}
	m_Generations.clear();
	m_Generations.push_back(initialGeneration);
	m_pActiveGeneration = initialGeneration;
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
	m_iActiveGenerationEpoch = 1u;
	m_strStatus = "Initialized immutable gameplay generation";
	return true;
}

bool LostArk::Server::CGameplayCatalogGenerations::Stage(
	const std::uint32_t transactionSequence,
	const LostArk::Shared::GameplayDataRevision& baseRevision,
	const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
	std::string& status)
{
	if (0u == transactionSequence || nullptr == m_pActiveGeneration ||
		(std::numeric_limits<std::uint16_t>::max)() ==
			m_iActiveGenerationEpoch ||
		m_pActiveGeneration->Get_ActiveRevision() != baseRevision ||
		nullptr == candidateGeneration ||
		!candidateGeneration->Get_ActiveRevision().Is_Valid() ||
		candidateGeneration->Get_ActiveRevision() == baseRevision)
	{
		status = "Room gameplay generation stage identity is invalid";
		return false;
	}
	if (nullptr != m_pStagedGeneration)
	{
		if (m_iStagedTransactionSequence == transactionSequence &&
			m_pStagedGeneration->Get_ActiveRevision() ==
				candidateGeneration->Get_ActiveRevision())
		{
			return true;
		}
		status = "Room already owns a different staged gameplay generation";
		return false;
	}
	const bool alreadyRetained = nullptr != Resolve(
		candidateGeneration->Get_ActiveRevision());
	if (!alreadyRetained && m_Generations.size() >= MAX_GENERATION_COUNT)
	{
		status = "Room gameplay generation capacity is exhausted";
		return false;
	}
	m_pStagedGeneration = alreadyRetained ?
		std::shared_ptr<const CGameplayCatalog>{} : candidateGeneration;
	if (alreadyRetained)
	{
		for (const auto& generation : m_Generations)
		{
			if (nullptr != generation && generation->Get_ActiveRevision() ==
				candidateGeneration->Get_ActiveRevision())
			{
				m_pStagedGeneration = generation;
				break;
			}
		}
	}
	m_iStagedTransactionSequence = transactionSequence;
	status.clear();
	return nullptr != m_pStagedGeneration;
}

bool LostArk::Server::CGameplayCatalogGenerations::Commit(
	const std::uint32_t transactionSequence) noexcept
{
	if (0u == transactionSequence ||
		transactionSequence != m_iStagedTransactionSequence ||
		nullptr == m_pStagedGeneration)
	{
		return false;
	}
	const bool alreadyRetained = std::any_of(
		m_Generations.begin(), m_Generations.end(),
		[this](const std::shared_ptr<const CGameplayCatalog>& generation)
		{
			return nullptr != generation &&
				generation->Get_ActiveRevision() ==
				m_pStagedGeneration->Get_ActiveRevision();
		});
	if (!alreadyRetained)
	{
		if (m_Generations.size() >= m_Generations.capacity())
			return false;
		m_Generations.push_back(m_pStagedGeneration);
	}
	m_pActiveGeneration = std::move(m_pStagedGeneration);
	m_iStagedTransactionSequence = 0u;
	++m_iActiveGenerationEpoch;
	m_strStatus = "Committed immutable gameplay generation";
	return true;
}

void LostArk::Server::CGameplayCatalogGenerations::Abort(
	const std::uint32_t transactionSequence) noexcept
{
	if (0u != transactionSequence &&
		transactionSequence != m_iStagedTransactionSequence)
	{
		return;
	}
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
}

void LostArk::Server::CGameplayCatalogGenerations::Collect_Garbage(
	const std::vector<LostArk::Shared::GameplayDataRevision>& livePins)
{
	m_Generations.erase(
		std::remove_if(
			m_Generations.begin(), m_Generations.end(),
			[this, &livePins](
				const std::shared_ptr<const CGameplayCatalog>& generation)
			{
				if (nullptr == generation || generation == m_pActiveGeneration ||
					generation == m_pStagedGeneration)
				{
					return false;
				}
				return livePins.end() == std::find(
					livePins.begin(), livePins.end(),
					generation->Get_ActiveRevision());
			}),
		m_Generations.end());
}

const LostArk::Server::CGameplayCatalog*
LostArk::Server::CGameplayCatalogGenerations::Resolve(
	const LostArk::Shared::GameplayDataRevision& revision) const noexcept
{
	if (!revision.Is_Valid())
		return nullptr;
	for (const auto& generation : m_Generations)
	{
		if (nullptr != generation &&
			generation->Get_ActiveRevision() == revision)
		{
			return generation.get();
		}
	}
	if (nullptr != m_pStagedGeneration &&
		m_pStagedGeneration->Get_ActiveRevision() == revision)
	{
		return m_pStagedGeneration.get();
	}
	return nullptr;
}

const LostArk::Server::CGameplayCatalog&
LostArk::Server::CGameplayCatalogGenerations::Active() const noexcept
{
	static const CGameplayCatalog EMPTY_CATALOG{};
	return nullptr == m_pActiveGeneration ? EMPTY_CATALOG : *m_pActiveGeneration;
}

const LostArk::Server::PLAYER_SKILL_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_Skill(
	const LostArk::Shared::SKILL_ID skillId) const
{
	return Active().Find_Skill(skillId);
}

const LostArk::Server::BOSS_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalogGenerations::Find_Boss(
	const std::string& archetypeId) const
{
	return Active().Find_Boss(archetypeId);
}

const std::vector<LostArk::Server::BOSS_PART_DEFINITION>*
LostArk::Server::CGameplayCatalogGenerations::Find_BossParts(
	const std::string& archetypeId) const
{
	return Active().Find_BossParts(archetypeId);
}

const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>*
LostArk::Server::CGameplayCatalogGenerations::Find_BossPatterns(
	const std::string& encounterId) const
{
	return Active().Find_BossPatterns(encounterId);
}

const LostArk::Server::BOSS_COMBAT_OBJECT_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_BossCombatObject(
	const std::string& archetypeId) const
{
	return Active().Find_BossCombatObject(archetypeId);
}

const LostArk::Server::VALTAN_TIMELINE_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimeline(
	const std::string& encounterId) const
{
	return Active().Find_ValtanTimeline(encounterId);
}

const LostArk::Server::VALTAN_TIMELINE_ROW*
LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimelineRow(
	const std::string& encounterId, const std::uint32_t commandId) const
{
	return Active().Find_ValtanTimelineRow(encounterId, commandId);
}

const LostArk::Server::BOSS_PATTERN_ROTATION_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_BossPatternRotation(
	const std::string& encounterId, const std::uint32_t gameplayPhase,
	const std::uint32_t healthBar) const
{
	return Active().Find_BossPatternRotation(
		encounterId, gameplayPhase, healthBar);
}

const std::string&
LostArk::Server::CGameplayCatalogGenerations::Find_IntroPatternId(
	const std::string& encounterId) const
{
	return Active().Find_IntroPatternId(encounterId);
}

const LostArk::Server::PLAYER_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalogGenerations::Find_Player(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	return Active().Find_Player(characterClass);
}

std::uint32_t
LostArk::Server::CGameplayCatalogGenerations::Find_DamageRatePercent(
	const std::string& damageProfileId) const
{
	return Active().Find_DamageRatePercent(damageProfileId);
}

const LostArk::Shared::GameplayDataRevision&
LostArk::Server::CGameplayCatalogGenerations::Get_ActiveRevision() const noexcept
{
	return Active().Get_ActiveRevision();
}

const std::string&
LostArk::Server::CGameplayCatalogGenerations::Get_Status() const noexcept
{
	return m_strStatus.empty() ? Active().Get_Status() : m_strStatus;
}

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
		LostArk::Shared::WORLD_ID::BERN == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
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
	if (!m_isReady || command.iSessionId == INVALID_SESSION_ID)
		return false;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return false;
	}

	std::scoped_lock lock{ m_CommandMutex };
	if (!m_acceptsCommands)
		return false;

	if (Is_BestEffortCommand(command.eType))
	{
		if (Try_RemoveCoalescedCommand(m_InboundCommands, command))
		{
			if (ROOM_COMMAND_TYPE::MOVE == command.eType)
				++m_PerformanceMetrics.iCoalescedMoveCommandCount;
			else
				++m_PerformanceMetrics.iCoalescedAimCommandCount;
			m_InboundCommands.push_back(std::move(command));
			return true;
		}
		if (m_InboundCommands.size() >= MAX_BEST_EFFORT_COMMAND_COUNT)
		{
			++m_PerformanceMetrics.iDroppedBestEffortCommandCount;
			return true;
		}
	}
	else if (ROOM_COMMAND_TYPE::LEAVE != command.eType &&
		m_InboundCommands.size() >= MAX_RELIABLE_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedReliableCommandCount;
		return false;
	}
	else if (ROOM_COMMAND_TYPE::LEAVE == command.eType &&
		m_InboundCommands.size() >= MAX_INBOUND_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedCleanupCommandCount;
		return false;
	}

	m_InboundCommands.push_back(std::move(command));
	m_PerformanceMetrics.iIngressHighWatermark = (std::max)(
		m_PerformanceMetrics.iIngressHighWatermark,
		m_InboundCommands.size());
	return true;
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
			std::scoped_lock lock{ m_CommandMutex };
			++m_PerformanceMetrics.iTickCount;
			m_PerformanceMetrics.iLastTickMicroseconds = elapsedMicroseconds;
			m_PerformanceMetrics.iMaximumTickMicroseconds = (std::max)(
				m_PerformanceMetrics.iMaximumTickMicroseconds,
				elapsedMicroseconds);
		};

	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
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
		if (!m_InboundCommands.empty())
			++m_PerformanceMetrics.iDrainLimitedTickCount;
	}

	for (ROOM_COMMAND& command : commands)
	{
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld);
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
		case ROOM_COMMAND_TYPE::LEAVE:
			Leave(command.iSessionId, command.eLeaveReason);
			break;
		}
	}

	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		recordTickDuration();
		return;
	}

	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	Refresh_PlayerBlockingBodies();
	Update_Players(fixedDeltaSeconds);
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	m_CombatObjectRuntime.Update(
		m_Players, m_WorldEntities, m_GameplayCatalog,
		fixedDeltaSeconds, updateTick, m_TickDamageEvents);
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
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
		});
	}
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
	if (!m_isReady)
	{
		recordTickDuration();
		return;
	}
	(void)Commit_DueEncounterProps(updateTick);
	if (!Commit_DueWorldDestruction(updateTick))
	{
		m_isReady = false;
		recordTickDuration();
		return;
	}
	Drain_BossCombatEvents();
#ifdef _DEBUG
	if (!Flush_ValtanPatternIdAuditionLifecycle())
	{
		m_strStatus = "Valtan audition lifecycle serialization failed";
		m_isReady = false;
		recordTickDuration();
		return;
	}
#endif
	if (!Broadcast_CombatObjectLifecycle())
	{
		m_isReady = false;
		recordTickDuration();
		return;
	}
	m_iServerTick = updateTick;
	if (!m_Players.empty())
		Broadcast_WorldSnapshot();
	std::vector<LostArk::Shared::GameplayDataRevision> liveGenerationPins;
	if (!Build_RequiredPinnedGameplayRevisions(liveGenerationPins))
	{
		m_strStatus = "Gameplay generation pin set exceeded its wire bound";
		m_isReady = false;
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
			<< " WireSendFailures=" << sendFailureCount << '\n';
	}
}

void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;
	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld)
{
	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
			session->Request_Close();
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush();
		}
		else
		{
			session->Request_Close();
		}
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = Find_AvailablePlayerSpawn();
	if (nullptr == spawn)
	{
		session->Request_Close();
		return false;
	}

	SERVER_PLAYER player{};
	player.iSessionId = sessionId;
	player.iPlayerId = m_iNextPlayerId;
	player.iNetEntityId = m_iNextNetEntityId;
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.fPositionX = spawn->fPositionX;
	player.fPositionY = spawn->fPositionY;
	player.fPositionZ = spawn->fPositionZ;
	player.fYawDegrees = spawn->fYawDegrees;
	const PLAYER_RUNTIME_PROFILE* playerProfile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == playerProfile)
	{
		session->Request_Close();
		return false;
	}
	player.eStance = playerProfile->eDefaultStance;
	player.iCurrentHp = playerProfile->iMaximumHp;
	player.iMaximumHp = playerProfile->iMaximumHp;
	player.iCurrentResource = playerProfile->iMaximumResource;
	player.iMaximumResource = playerProfile->iMaximumResource;
	player.fMoveSpeed = playerProfile->fMoveSpeed;
	// A gauge starts full: the stance it pays for is available on entry, and a
	// class without one keeps both at 0 so nothing ever drains.
	player.iMaximumIdentity = playerProfile->iMaximumIdentity;
	player.iCurrentIdentity = playerProfile->iMaximumIdentity;
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(
			player.fPositionX, player.fPositionZ, projected))
		{
			session->Request_Close();
			return false;
		}
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}

	// Debug/testing seed: every entering player starts with 500 of each HP
	// potion tier, per explicit request, so trying the heal-on-use path
	// doesn't need 500 separate F1 Give Item clicks first.
	for (const char* potionId :
		{ "POTION_HP_SMALL", "POTION_HP_MEDIUM", "POTION_HP_LARGE" })
	{
		const SERVER_ITEM_DEFINITION* potionDefinition =
			m_ItemCatalog.Find_Item(potionId);
		if (nullptr == potionDefinition)
			continue;
		INVENTORY_ITEM_SNAPSHOT seeded{};
		seeded.strItemId = potionId;
		seeded.iQuantity = (std::min)(500u, potionDefinition->iMaxStack);
		player.Inventory.push_back(std::move(seeded));
	}

	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	session->Bind_PlayerId(player.iPlayerId);

	if (!Send_Accepted(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	// A re-entering session sees its existing inventory without needing to
	// give itself an item first. iRequestSequence 0 marks this as the
	// unsolicited entry snapshot rather than an answer to a give request.
	if (!Send_InventorySnapshot(session, 0u, player.Inventory))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	if (!Send_WorldDestructionFullSync(session) ||
		!Send_EncounterPropSync(session))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (!Send_WorldEntitySpawned(session, entity))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> liveCombatObjects;
	m_CombatObjectRuntime.Build_LiveSpawnMessages(
		0u == m_iServerTick ? 1u : m_iServerTick, liveCombatObjects);
	for (const S2C_COMBAT_OBJECT_SPAWNED& combatObject : liveCombatObjects)
	{
		if (!Send_CombatObjectSpawned(session, combatObject))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	for (const auto& [existingPlayerId, existingPlayer] : m_Players)
	{
		(void)existingPlayerId;
		if (existingPlayer.iSessionId == sessionId)
			continue;
		if (!Send_Spawned(session, existingPlayer))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	if (!Send_Spawned(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	Broadcast_Spawned(player, sessionId);

	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", PlayerId=" << player.iPlayerId
		<< ", Spawn=" << player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;

#ifdef _DEBUG
	if (sessionId == m_ValtanTimelineAudition.iOwnerSessionId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}
#endif
	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternIdAuditionSequenceBySessionId.erase(sessionId);
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;
	m_CombatObjectRuntime.Cancel_Source(netEntityId);
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	Broadcast_Despawned(netEntityId, reason);

	std::cout << "Player left. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", RoomPlayers=" << m_Players.size() << '\n';

	if (!Reset_ReplayableArenaWhenEmpty())
	{
		std::cerr << "Replayable arena reset failed. World="
			<< static_cast<unsigned>(m_eWorldId) << ", Status="
			<< m_strStatus << '\n';
	}
	if (!Reset_ValtanArenaWhenEmpty())
	{
		std::cerr << "Valtan arena reset failed: "
			<< m_strStatus << '\n';
	}
}

void LostArk::Server::CGameRoom::Handle_Move(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MOVE& move)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(move.iClientSequence, player.iLastMoveSequence) ||
		!std::isfinite(move.fGoalX) || !std::isfinite(move.fGoalZ) ||
		std::abs(move.fGoalX) > MAX_ABS_MOVE_GOAL ||
		std::abs(move.fGoalZ) > MAX_ABS_MOVE_GOAL)
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}

	player.iLastMoveSequence = move.iClientSequence;
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
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp)
	{
		if (0u != player.iCurrentHp && Is_BufferableComboAction(player))
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
		Smooth_MovePath(
			m_ServerNavigation,
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
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp)
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
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
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
	/* Character Select Server Arena is the presentation audition room.  Keep
	its retries Server-authoritative with a fixed three-second audition cooldown
	and full resources; action-running, sequence, class, aim and snapshot gates
	remain in CPlayerSkillSystem::Try_Start.  Release rooms retain authored
	balance. */
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
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
		{
			playerIter->second.CooldownEndTickBySkillId.insert_or_assign(
				useSkill.iSkillId,
				Add_ServerTicksSkippingReservedZero(
					actionStartTick,
					CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS));
		}
#endif
		playerIter->second.isCombatReady = true;
	}
}

void LostArk::Server::CGameRoom::Handle_RevivePlayer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
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
	/* A fall kills the player over a hole. Reviving in place would drop them
	again on the next tick, so a revive whose current cell is no longer walkable
	steps out to the nearest ground of the same deck that is still standing.
	The arena progression triggers are room-wide triggerOnce, so returning the
	body to the entry spawn would strand it outside a fight it can never
	re-enter, and a plain XZ projection would drop it onto a lower deck. The
	entry spawn stays as the last resort for a body with no reachable ground on
	its own deck. Every other death still revives exactly where it happened. */
	if (m_ServerNavigation.Is_Loaded() &&
		!m_ServerNavigation.Is_PointWalkableExact(
			player.fPositionX, player.fPositionZ))
	{
		SERVER_NAV_POINT projected{};
		if (m_ServerNavigation.Project_PointOnSameLevel(
			player.fPositionX, player.fPositionZ, projected))
		{
			player.fPositionX = projected.x;
			player.fPositionY = projected.y;
			player.fPositionZ = projected.z;
		}
		else
		{
			const WORLD_BOOTSTRAP_PLACEMENT* spawn =
				Find_Placement(player.strSpawnPlacementId);
			if (nullptr == spawn || !spawn->isEnabled ||
				WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn->eKind ||
				!m_ServerNavigation.Project_Point(
					spawn->fPositionX, spawn->fPositionZ, projected))
			{
				return;
			}
			player.fPositionX = projected.x;
			player.fPositionY = projected.y;
			player.fPositionZ = projected.z;
			player.fYawDegrees = spawn->fYawDegrees;
		}
	}
	player.iCurrentHp = player.iMaximumHp;
	player.iCurrentResource = player.iMaximumResource;
	player.iResourceAccumulator = 0u;
	player.iCurrentIdentity = player.iMaximumIdentity;
	player.iIdentityAccumulator = 0u;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.eStance = profile->eDefaultStance;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
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
	player.isCombatReady = false;
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

void LostArk::Server::CGameRoom::Handle_ReleaseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
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
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
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
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
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
	staged.eAction = PLAYER_ACTION_STATE::NONE;
	staged.eStance = profile->eDefaultStance;
	staged.iCurrentSkillId = INVALID_SKILL_ID;
	staged.Clear_SkillTarget();
	staged.iActionStartTick = 0u;
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

void LostArk::Server::CGameRoom::Handle_DebugGiveItem(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request)
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
	const SERVER_ITEM_DEFINITION* itemDefinition =
		m_ItemCatalog.Find_Item(request.strItemId);
	// An unknown item ID is rejected rather than silently dropped or
	// substituted; the inventory is only ever a replace-in-full send, so a
	// no-op reply carries no result to give a caller.
	if (nullptr == itemDefinition)
		return;

	const auto existing = std::find_if(
		player.Inventory.begin(), player.Inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == request.strItemId;
		});
	if (existing == player.Inventory.end())
	{
		INVENTORY_ITEM_SNAPSHOT item{};
		item.strItemId = request.strItemId;
		item.iQuantity = (std::min)(request.iQuantity, itemDefinition->iMaxStack);
		if (player.Inventory.size() >= MAX_INVENTORY_ITEMS)
			return;
		player.Inventory.push_back(std::move(item));
	}
	else
	{
		const std::uint64_t stacked =
			static_cast<std::uint64_t>(existing->iQuantity) +
			static_cast<std::uint64_t>(request.iQuantity);
		existing->iQuantity = static_cast<std::uint32_t>(
			(std::min)(stacked, static_cast<std::uint64_t>(
				itemDefinition->iMaxStack)));
	}

	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, player.Inventory))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_UseItem(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_ITEM& request)
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
	const SERVER_ITEM_DEFINITION* itemDefinition =
		m_ItemCatalog.Find_Item(request.strItemId);
	// Not owned, not a consumable, or already dead: no-op. A dead player using
	// a heal potion would just resurrect them for free outside the real
	// revive path, so this is deliberately excluded too.
	if (nullptr == itemDefinition || 0u == itemDefinition->iHealPercent ||
		0u == player.iCurrentHp)
	{
		return;
	}

	const auto existing = std::find_if(
		player.Inventory.begin(), player.Inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == request.strItemId;
		});
	if (existing == player.Inventory.end() || 0u == existing->iQuantity)
		return;

	const std::uint64_t healAmount =
		(static_cast<std::uint64_t>(player.iMaximumHp) *
			static_cast<std::uint64_t>(itemDefinition->iHealPercent)) / 100u;
	player.iCurrentHp = static_cast<std::uint32_t>((std::min)(
		static_cast<std::uint64_t>(player.iMaximumHp),
		static_cast<std::uint64_t>(player.iCurrentHp) + healAmount));

	--existing->iQuantity;
	if (0u == existing->iQuantity)
		player.Inventory.erase(existing);

	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, player.Inventory))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_DespawnAllWorldEntities(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request)
{
	using namespace LostArk::Shared;
	(void)request;
	// Same room gating as Handle_SpawnWorldEntity -- this debug revert only makes
	// sense for the Character Select Arena's own spawn buttons.
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		return;
	}

	// Character Select Arena's own placements have no statically-enabled
	// MONSTER/BOSS entries (confirmed: only 4 disabled playerSpawn + one disabled
	// BOSS_VALTAN), so everything currently in m_WorldEntities here was created by
	// the debug spawn buttons -- safe to despawn all of it unconditionally.
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
		Broadcast_WorldEntityDespawned(entity.iNetEntityId);
	m_WorldEntities.clear();

	// Reset spawn group state (DORMANT) too, so the same group can be activated
	// again -- Handle_SpawnWorldEntity's Is_ActiveOrCompleted check would otherwise
	// keep refusing a re-spawn after this revert. Same call
	// Reset_ReplayableArenaWhenEmpty already uses for the equivalent "room is
	// empty" reset, just without that gate.
	std::string resetStatus;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
		m_strStatus = std::move(resetStatus);
}

void LostArk::Server::CGameRoom::Handle_ConfirmNpcEntry(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request)
{
	using namespace LostArk::Shared;
	// Mirrors the two disabled changeLevel triggerBox placements
	// (trigger.bern.to-valtan / valtan) in Data/Worlds/LV_BER_BERNCASTLE/
	// Gameplay.world.json -- both guide NPCs currently lead to the same target.
	struct VALTAN_ENTRY_GUIDE_NPC
	{
		const char* pNpcPlacementId;
		WORLD_ID eTargetWorldId;
	};
	static constexpr VALTAN_ENTRY_GUIDE_NPC VALTAN_ENTRY_GUIDE_NPCS[] =
	{
		{ "npc.bern.beda.guide", WORLD_ID::VALTAN_ARENA },
		{ "npc.bern.aylara", WORLD_ID::VALTAN_ARENA },
	};
	// Same footprint as the old trigger boxes' largest half extent (2m), so
	// standing where the box used to be still reaches the NPC.
	constexpr float INTERACTION_RADIUS = 3.f;

	if (WORLD_ID::BERN != m_eWorldId)
		return;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& player = playerIter->second;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::NONE != player.eAction)
		return;

	const auto guideIter = std::find_if(
		std::begin(VALTAN_ENTRY_GUIDE_NPCS), std::end(VALTAN_ENTRY_GUIDE_NPCS),
		[&request](const VALTAN_ENTRY_GUIDE_NPC& guide)
		{
			return request.strNpcPlacementId == guide.pNpcPlacementId;
		});
	if (std::end(VALTAN_ENTRY_GUIDE_NPCS) == guideIter)
		return;

	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == request.strNpcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
		return;

	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ >
		INTERACTION_RADIUS * INTERACTION_RADIUS)
	{
		return;
	}

	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return;
	}

	const bool alreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId;
		});
	if (alreadyStaged)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = guideIter->eTargetWorldId;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	m_PendingWorldTransfers.push_back(std::move(transfer));
}

void LostArk::Server::CGameRoom::Handle_SpawnWorldEntity(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId) || nullptr == session)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(request.strPlacementId);
	if (nullptr == placement)
	{
		const auto group = std::find_if(
			m_SpawnGroupBootstrap.Get_Groups().begin(),
			m_SpawnGroupBootstrap.Get_Groups().end(),
			[&request](const SPAWN_GROUP_DEFINITION& definition)
			{
				return definition.strSpawnGroupId == request.strPlacementId;
			});
		if (m_SpawnGroupBootstrap.Get_Groups().end() == group)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		if (!m_SpawnGroupRuntime.Is_ActiveOrCompleted(request.strPlacementId) &&
			!m_SpawnGroupRuntime.Activate_Immediate(
				request.strPlacementId,
				m_SpawnGroupBootstrap,
				[this](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				}))
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}

		if (!Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::ACTIVATED,
			INVALID_NET_ENTITY_ID))
		{
			session->Request_Close();
		}
		return;
	}

	if (placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		placement->strArchetypeId != "BOSS_VALTAN")
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}
	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == request.strPlacementId;
		});
	if (m_WorldEntities.end() != existing)
	{
		if (!Send_WorldEntitySpawned(session, *existing) ||
			!Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS,
				existing->iNetEntityId))
		{
			session->Request_Close();
		}
		return;
	}
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	if (!Send_WorldEntitySpawnResult(
		session,
		request.strPlacementId,
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED,
		m_WorldEntities.back().iNetEntityId))
	{
		session->Request_Close();
	}
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_AuditionBoss()
{
	return Find_AuditionBoss("boss.valtan.center");
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_AuditionBoss(
	const std::string& placementId)
{
	if (placementId.empty())
		return nullptr;
	const auto found = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				placementId == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}

bool LostArk::Server::CGameRoom::Has_EngagedAuditionPlayer(
	const SERVER_WORLD_ENTITY& boss) const
{
	/* CValtanBrain::Update drops its target and calls FinishPattern when no
	combat-ready living player is inside the engage distance, so a pattern
	queued while that is true would be discarded before it ever started. The
	brain widens the radius to the longest authored pattern range; this checks
	only the boss's own engage distance, which is the narrower of the two, so an
	accepted audition is always one the brain will also act on. */
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			continue;
		}
		const float deltaX = player.fPositionX - boss.fPositionX;
		const float deltaZ = player.fPositionZ - boss.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <=
			boss.fEngageDistance * boss.fEngageDistance)
		{
			return true;
		}
	}
	return false;
}

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_ValtanPatternIdAuditionLifecycle(
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
			m_ValtanPatternIdAudition.ePhase ||
		INVALID_SESSION_ID == m_ValtanPatternIdAudition.iOwnerSessionId)
	{
		return;
	}
	S2C_VALTAN_AUDITION_LIFECYCLE message{};
	message.iRequestSequence =
		m_ValtanPatternIdAudition.iRequestSequence;
	message.iRoomAuditionEpoch =
		m_ValtanPatternIdAudition.iRoomAuditionEpoch;
	message.iPatternSequence =
		m_ValtanPatternIdAudition.iExpectedPatternSequence;
	message.strPatternId = m_ValtanPatternIdAudition.strPatternId;
	message.eState = state;
	message.PinnedDefinitionRevision =
		m_ValtanPatternIdAudition.PinnedDefinitionRevision;
	message.strReason = std::move(reason);
	m_PendingValtanAuditionLifecycle.push_back({
		m_ValtanPatternIdAudition.iOwnerSessionId, std::move(message) });
}

bool LostArk::Server::CGameRoom::Flush_ValtanPatternIdAuditionLifecycle()
{
	using namespace LostArk::Shared;
	auto pending = std::move(m_PendingValtanAuditionLifecycle);
	m_PendingValtanAuditionLifecycle.clear();
	for (const TARGETED_VALTAN_AUDITION_LIFECYCLE& targeted : pending)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(targeted.iSessionId);
		if (nullptr == session)
			continue;
		CPacketWriter writer;
		if (!Write_Message(writer, targeted.Message))
			return false;
		if (!session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE,
			writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Refresh_ValtanPatternIdAuditionState()
{
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
		m_ValtanPatternIdAudition.ePhase)
	{
		return false;
	}

	const auto boss = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId ==
					m_ValtanPatternIdAudition.iBossEntityId &&
				entity.strPlacementId ==
					m_ValtanPatternIdAudition.strBossPlacementId &&
				WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	if (m_WorldEntities.end() == boss || 0u == boss->iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss->eAction)
	{
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED,
			"Valtan audition boss became unavailable");
		m_ValtanPatternIdAudition = {};
		return false;
	}

	if (boss->iPatternSequence ==
			m_ValtanPatternIdAudition.iExpectedPatternSequence &&
		boss->strPatternId == m_ValtanPatternIdAudition.strPatternId)
	{
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE !=
			m_ValtanPatternIdAudition.ePhase)
		{
			if (!boss->PinnedDefinitionRevision.Is_Valid() ||
				boss->PinnedDefinitionRevision !=
					m_ValtanPatternIdAudition.PinnedDefinitionRevision)
			{
				Queue_ValtanPatternIdAuditionLifecycle(
					LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED,
					"Valtan audition definition revision changed before start");
				m_ValtanPatternIdAudition = {};
				return false;
			}
			m_ValtanPatternIdAudition.ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE;
			Queue_ValtanPatternIdAuditionLifecycle(
				LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		}
		return true;
	}
	if (boss->PendingPatternIds.end() != std::find(
			boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(),
			m_ValtanPatternIdAudition.strPatternId))
	{
		m_ValtanPatternIdAudition.ePhase =
			VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
		return true;
	}

	/* The selected occurrence finished, was removed by another authoritative
	   reset, or no longer owns the expected sequence. A naturally running boss
	   pattern was never registered here, so it cannot block the first request. */
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		boss->iPatternSequence ==
			m_ValtanPatternIdAudition.iExpectedPatternSequence &&
		boss->strPatternId.empty())
	{
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
	}
	else
	{
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED,
			"Valtan audition occurrence was discarded or replaced");
	}
	m_ValtanPatternIdAudition = {};
	return false;
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

	boss = std::move(stagedBoss);
	/* Character Select has no Valtan wall, floor or prop runtime. Cancel only
	   objects emitted by this boss; the private player's active combat objects
	   belong to a different source and survive the audition reset. */
	m_CombatObjectRuntime.Cancel_Source(boss.iNetEntityId);
	m_TickBossCombatEvents.clear();
	m_iValtanAuditionArmedHealthBar = 0u;
#ifdef _DEBUG
	m_ValtanPatternIdAudition = {};
	m_ValtanTimelineAudition = {};
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
	/* All preflight work above succeeded. Combat objects belong to the same
	encounter epoch, so reset them only at this commit edge and keep reliable
	despawns for the players who are still observing the audition. */
	m_CombatObjectRuntime.Reset();
	m_iValtanAuditionArmedHealthBar = 0u;
#ifdef _DEBUG
	m_ValtanPatternIdAudition = {};
	m_ValtanTimelineAudition = {};
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
	std::set<std::string> stagedGroupIds;
	const auto append =
		[&](const WORLD_DESTRUCTION_TRANSACTION& transaction) -> bool
		{
			if (transaction.iEncounterEpoch != runtime.Get_EncounterEpoch() ||
				transaction.iRequestTick != requestTick ||
				transaction.BindingApplications.size() !=
					transaction.Transitions.size() ||
				transaction.Transitions.empty())
			{
				return false;
			}
			for (std::size_t index = 0u;
				index < transaction.Transitions.size(); ++index)
			{
				const std::string& groupId =
					transaction.Transitions[index].strGroupId;
				if (!stagedGroupIds.insert(groupId).second)
					return false;
				staged.push_back({ transaction.BindingApplications[index],
					transaction.Transitions[index] });
			}
			return true;
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

bool LostArk::Server::CGameRoom::Stop_ValtanTimelineRow(
	const bool resetEncounter)
{
	SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
	if (nullptr != boss &&
		boss->iNetEntityId == m_ValtanTimelineAudition.iBossEntityId)
	{
		boss->PendingPatternIds.clear();
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
	if (!expectedSequence || !boss.strPatternId.empty())
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
	std::uint32_t& currentHealthBar = outCurrentHealthBar;
	const auto evaluate = [&]() -> VALTAN_AUDITION_RESULT
	{
		const bool isPatternIdPlay =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == request.eOperation;
		const bool isValtanArena = WORLD_ID::VALTAN_ARENA == m_eWorldId;
		const bool isCharacterSelectArena =
			WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId;
		if ((!isPatternIdPlay && !isValtanArena) ||
			(isPatternIdPlay && !isValtanArena && !isCharacterSelectArena) ||
			!m_PlayerIdBySessionId.contains(sessionId))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
		}
		auto& handledSequences = isPatternIdPlay ?
			m_ValtanPatternIdAuditionSequenceBySessionId :
			m_ValtanAuditionSequenceBySessionId;
		const auto handled = handledSequences.find(sessionId);
		if (0u == request.iRequestSequence ||
			(handledSequences.end() != handled &&
			 !Is_NewerSequence(request.iRequestSequence, handled->second)))
		{
			return VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED;
		}
		/* A verdict finishes this request identity, including a rejection. The
		Client may retry the same sequence while its response is in flight; it
		must never become executable later merely because room state changed. */
		handledSequences.insert_or_assign(
			sessionId, request.iRequestSequence);
		if (Refresh_ValtanPatternIdAuditionState())
		{
			m_strStatus = "Valtan stable-ID pattern audition is already "
				"pending or active: " +
				m_ValtanPatternIdAudition.strPatternId;
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (isPatternIdPlay)
		{
			if (0u != request.iTargetHealthBar ||
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
			const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
				m_GameplayCatalog.Find_BossPatterns(boss->strEncounterId);
			if (nullptr == patterns || patterns->end() == std::find_if(
					patterns->begin(), patterns->end(),
					[&request](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == request.strPatternId;
					}))
			{
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			if (isCharacterSelectArena &&
				Is_CharacterSelectEnvironmentDependentPattern(
					request.strPatternId))
			{
				m_strStatus =
					"Character Select cannot audition an arena-environment pattern";
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
			{
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			/* Valtan Arena keeps its existing one-click bait placement. Character
			   Select has no raid trigger/bait contract and therefore requires its
			   private-room player to already be engaged with the spawned boss. */
			if (isValtanArena)
			{
				const auto playerId = m_PlayerIdBySessionId.find(sessionId);
				if (m_PlayerIdBySessionId.end() == playerId)
					return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
				const auto player = m_Players.find(playerId->second);
				if (m_Players.end() == player || 0u == player->second.iCurrentHp)
					return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
				player->second.isCombatReady = true;
				if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
						player->second,
						0u == m_iServerTick ? 1u : m_iServerTick))
				{
					return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
				}
			}
			if (!Has_EngagedAuditionPlayer(*boss))
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;

			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			std::string resetStatus;
			if (isCharacterSelectArena)
			{
				/* Preflight the spawn reset before mutating the live boss. This also
				   proves the private-room player remains engaged at the canonical
				   placement after a previous moving pattern displaced the boss. */
				SERVER_WORLD_ENTITY resetProbe{};
				if (!Build_ValtanBossOnlyAuditionReset(
						*boss, resetTick, resetProbe, resetStatus))
				{
					m_strStatus = std::move(resetStatus);
					return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
				}
				if (!Has_EngagedAuditionPlayer(resetProbe))
					return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
				if (!Reset_ValtanBossOnlyAuditionState(
						*boss, resetTick, resetStatus))
				{
					m_strStatus = std::move(resetStatus);
					return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
				}
			}
			else if (!Reset_ValtanAuditionState(
					*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			if (!Has_EngagedAuditionPlayer(*boss))
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;

			boss->bIntroPatternConsumed = true;
			boss->PendingPatternIds.push_back(request.strPatternId);
			m_ValtanPatternIdAudition.ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
			m_ValtanPatternIdAudition.iOwnerSessionId = sessionId;
			m_ValtanPatternIdAudition.iBossEntityId = boss->iNetEntityId;
			m_ValtanPatternIdAudition.iExpectedPatternSequence =
				(std::numeric_limits<std::uint32_t>::max)() ==
					boss->iPatternSequence ?
				1u : boss->iPatternSequence + 1u;
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
				1u : m_iNextValtanAuditionEpoch + 1u;
			m_ValtanPatternIdAudition.PinnedDefinitionRevision =
				m_GameplayCatalog.Get_ActiveRevision();
			Queue_ValtanPatternIdAuditionLifecycle(
				VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
			m_iValtanAuditionArmedHealthBar = 0u;
			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			m_strStatus = "Valtan pattern ID audition queued: " +
				request.strPatternId;
			return VALTAN_AUDITION_RESULT::QUEUED;
		}
		const bool isPillarCyclePlay =
			VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE == request.eOperation;
		const bool isWallAttackPlay =
			VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK == request.eOperation;
		/* The Debug pattern browser names an authored pattern by its index in
		   the encounter document instead of a health bar, so the NORMAL
		   patterns no bar owns can be inspected without fighting for them. */
		const bool isPatternPlay =
			VALTAN_AUDITION_OPERATION::PLAY_PATTERN == request.eOperation;
		const bool isFinalArenaView =
			VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA == request.eOperation;
		/* The same staging without the floor stages, so the 84 and 30 collapses
		   can afterwards be auditioned with every wall already gone. */
		const bool isOpenArenaView =
			VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL == request.eOperation;
		const bool isArenaStagingView = isFinalArenaView || isOpenArenaView;
		const bool isTimelinePlay =
			VALTAN_AUDITION_OPERATION::PLAY_TIMELINE_ROW == request.eOperation;
		const bool isTimelineStop =
			VALTAN_AUDITION_OPERATION::STOP_TIMELINE_ROW == request.eOperation;
		if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase &&
			!isTimelineStop && !isTimelinePlay)
		{
			m_strStatus =
				"Stop the active Valtan timeline row before another operation";
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		if (isTimelineStop)
		{
			return Stop_ValtanTimelineRow(true) ?
				VALTAN_AUDITION_RESULT::QUEUED :
				VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
		if (nullptr == boss && isTimelinePlay)
		{
			const WORLD_BOOTSTRAP_PLACEMENT* placement =
				Find_Placement("boss.valtan.center");
			SERVER_WORLD_ENTITY stagedBoss{};
			SERVER_PLAYER stagedOwner{};
			std::string preflightStatus;
			const std::uint32_t startTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (nullptr == placement || placement->isEnabled ||
				WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
				!Build_WorldEntity(
					*placement, m_iNextNetEntityId, stagedBoss) ||
				!Stage_ValtanTimelineRowStart(
					sessionId, stagedBoss, request.iTargetHealthBar, startTick,
					stagedOwner, preflightStatus))
			{
				if (!preflightStatus.empty())
					m_strStatus = std::move(preflightStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
		}
		if (nullptr == boss &&
			(VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation ||
				 VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE == request.eOperation ||
				 isPillarCyclePlay || isWallAttackPlay || isArenaStagingView ||
				 isPatternPlay || isTimelinePlay))
		{
			(void)Activate_Encounter("boss.valtan.center");
			boss = Find_AuditionBoss();
		}
		if (nullptr == boss)
			return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;
		if (isTimelinePlay)
		{
			std::string startStatus;
			const std::uint32_t startTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Start_ValtanTimelineRow(
				sessionId, *boss, request.iTargetHealthBar,
				startTick, startStatus))
			{
				m_strStatus = std::move(startStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			m_strStatus = std::move(startStatus);
			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		if (0u == boss->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == boss->eAction)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
		}

		const std::vector<BOSS_PATTERN_DEFINITION>* patterns =
			m_GameplayCatalog.Find_BossPatterns(boss->strEncounterId);
		if (nullptr == patterns)
			return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;
		/* The entrance sweep is the encounter's intro pattern, not a health-bar
		crossing, so it is looked up by its own allowlisted identity. */
		const bool isEntrancePlay =
			VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE == request.eOperation;
		if (isEntrancePlay)
		{
			const std::string& introPatternId =
				m_GameplayCatalog.Find_IntroPatternId(boss->strEncounterId);
			if (introPatternId.empty() ||
				patterns->end() == std::find_if(
					patterns->begin(), patterns->end(),
					[&introPatternId](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == introPatternId;
					}))
			{
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;
			}
		}
		std::string auditionPatternId;
		/* The pillar cycle names the mechanic, not a bar, so its own authored
		pattern decides which crossing reproduces it. */
		std::uint32_t targetHealthBar = request.iTargetHealthBar;
		if (isPatternPlay)
		{
			/* One-based so the wire never carries the zero that means "no bar",
			   and bounded by the catalog the publisher wrote in the same
			   authored order the Client lists. */
			if (0u == targetHealthBar || targetHealthBar > patterns->size())
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;
			auditionPatternId = (*patterns)[targetHealthBar - 1u].strPatternId;
			targetHealthBar = 0u;
		}
		else if (!isEntrancePlay && !isArenaStagingView)
		{
			const auto authored = std::find_if(
				patterns->begin(),
				patterns->end(),
				[isPillarCyclePlay, isWallAttackPlay, targetHealthBar]
				(const BOSS_PATTERN_DEFINITION& pattern)
				{
					if (isWallAttackPlay)
						return pattern.strPatternId == WALL_ATTACK_PATTERN_ID;
					if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection)
						return false;
					return isPillarCyclePlay
						? pattern.strPatternId == PILLAR_PATTERN_ID
						: pattern.iTriggerHealthBar == targetHealthBar;
				});
			if (patterns->end() == authored)
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;
			auditionPatternId = authored->strPatternId;
			if (!isWallAttackPlay)
				targetHealthBar = authored->iTriggerHealthBar;
		}

		const bool isOneClickPlay =
			VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation ||
			isEntrancePlay || isPillarCyclePlay || isWallAttackPlay ||
			isArenaStagingView || isPatternPlay;
		/* ARM/CROSS are diagnostics over the current encounter. PLAY is the
		   repeatable user-facing audition: it resets the authoritative boss and
		   destruction generation first, so a previous 159/80 run cannot make the
		   button silently unavailable. */
		if (!isOneClickPlay &&
			(boss->TriggeredPatternIds.end() != std::find(
			boss->TriggeredPatternIds.begin(),
			boss->TriggeredPatternIds.end(),
			auditionPatternId) ||
			!boss->PendingPatternIds.empty() ||
			!boss->strPatternId.empty()))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		if (isArenaStagingView)
		{
			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			/* Build one Server transaction containing the 69 ordinary contact
			   walls and the 30 pattern-only outer walls. There are deliberately no
			   one-shot debris events in this diagnostic view: it exists to inspect
			   the final persistent arena, collision and navigation state. */
			struct FINAL_TRANSITION_PAIR final
			{
				WORLD_DESTRUCTION_BINDING_APPLICATION Application;
				WORLD_DESTRUCTION_STATE_TRANSITION Transition;
			};
			std::vector<FINAL_TRANSITION_PAIR> stagedPairs;
			const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& graph =
				m_WorldDestructionBootstrap.Get_DescriptorGraph();
			const auto appendTransaction =
				[&stagedPairs, resetTick, this](
					const WORLD_DESTRUCTION_TRANSACTION& transaction) -> bool
				{
					if (transaction.iEncounterEpoch !=
							m_WorldDestructionRuntime.Get_EncounterEpoch() ||
						transaction.iRequestTick != resetTick ||
						transaction.BindingApplications.size() !=
							transaction.Transitions.size())
					{
						return false;
					}
					for (std::size_t index = 0u;
						index < transaction.Transitions.size(); ++index)
					{
						stagedPairs.push_back({
							transaction.BindingApplications[index],
							transaction.Transitions[index] });
					}
					return true;
				};

			for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding :
				graph.Bindings)
			{
				if (WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT !=
					binding.eTriggerKind)
				{
					continue;
				}
				WORLD_DESTRUCTION_TRANSACTION contactTransaction{};
				const WORLD_DESTRUCTION_PREPARE_RESULT contactResult =
					m_WorldDestructionRuntime.Prepare_ContactTrigger(
						binding.strImpactReceiverId, boss->iNetEntityId,
						resetTick, resetTick, contactTransaction, resetStatus);
				if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != contactResult ||
					!appendTransaction(contactTransaction))
				{
					m_strStatus = "Final arena ordinary-wall staging failed: " +
						resetStatus;
					return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
				}
			}

			WORLD_DESTRUCTION_ACTION_TUPLE outerAction{};
			outerAction.strPatternId = FINAL_ARENA_PATTERN_ID;
			outerAction.strStageId = FINAL_ARENA_STAGE_ID;
			outerAction.strActionId = FINAL_ARENA_ACTION_ID;
			outerAction.iStageIndex = 2u;
			WORLD_DESTRUCTION_TRANSACTION outerTransaction{};
			const std::uint32_t outerPatternSequence =
				(std::numeric_limits<std::uint32_t>::max)() ==
					boss->iPatternSequence ? 1u : boss->iPatternSequence + 1u;
			const WORLD_DESTRUCTION_PREPARE_RESULT outerResult =
				m_WorldDestructionRuntime.Prepare_StageTrigger(
					outerAction, boss->iNetEntityId, outerPatternSequence,
					resetTick, outerTransaction, resetStatus);
			if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != outerResult ||
				!appendTransaction(outerTransaction))
			{
				m_strStatus = "Final arena outer-wall staging failed: " +
					resetStatus;
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			/* The floor sectors carry their own health-bar patterns, so each
			   stage is prepared on its own pattern sequence. The final arena is
			   only complete when every authored group has been staged once. */
			const std::array<WORLD_DESTRUCTION_ACTION_TUPLE, 2u> floorActions{
				WORLD_DESTRUCTION_ACTION_TUPLE{
					FINAL_ARENA_FLOOR_A_PATTERN_ID,
					FINAL_ARENA_FLOOR_A_STAGE_ID,
					FINAL_ARENA_FLOOR_A_ACTION_ID,
					FINAL_ARENA_FLOOR_A_STAGE_INDEX },
				WORLD_DESTRUCTION_ACTION_TUPLE{
					FINAL_ARENA_FLOOR_B_PATTERN_ID,
					FINAL_ARENA_FLOOR_B_STAGE_ID,
					FINAL_ARENA_FLOOR_B_ACTION_ID,
					FINAL_ARENA_FLOOR_B_STAGE_INDEX } };
			std::uint32_t floorPatternSequence = outerPatternSequence;
			/* The open-arena view prepares the same two stages only to learn how
			   many groups the floor owns. It never appends them, so the sectors stay
			   INTACT and their own bars can still collapse them afterwards. */
			std::size_t floorGroupCount = 0u;
			for (const WORLD_DESTRUCTION_ACTION_TUPLE& floorAction : floorActions)
			{
				floorPatternSequence =
					(std::numeric_limits<std::uint32_t>::max)() ==
						floorPatternSequence ? 1u : floorPatternSequence + 1u;
				WORLD_DESTRUCTION_TRANSACTION floorTransaction{};
				const WORLD_DESTRUCTION_PREPARE_RESULT floorResult =
					m_WorldDestructionRuntime.Prepare_StageTrigger(
						floorAction, boss->iNetEntityId, floorPatternSequence,
						resetTick, floorTransaction, resetStatus);
				if (WORLD_DESTRUCTION_PREPARE_RESULT::READY != floorResult ||
					(isFinalArenaView && !appendTransaction(floorTransaction)))
				{
					m_strStatus = "Final arena floor staging failed: " +
						resetStatus;
					return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
				}
				floorGroupCount += floorTransaction.Transitions.size();
			}

			/* Checked before the subtraction so an unresolved floor can never wrap
			   the unsigned coverage target into a passing value. */
			if (0u == floorGroupCount ||
				floorGroupCount >= graph.Groups.size())
			{
				m_strStatus = "Arena staging could not resolve the floor sectors.";
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			const std::size_t expectedGroupCoverage = isFinalArenaView ?
				graph.Groups.size() : graph.Groups.size() - floorGroupCount;
			if (stagedPairs.size() != expectedGroupCoverage)
			{
				m_strStatus = isFinalArenaView ?
					"Final arena staging did not cover every group." :
					"Open arena staging did not cover every wall group.";
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			std::sort(
				stagedPairs.begin(), stagedPairs.end(),
				[](const FINAL_TRANSITION_PAIR& left,
					const FINAL_TRANSITION_PAIR& right)
				{
					return left.Transition.strGroupId <
						right.Transition.strGroupId;
				});
			WORLD_DESTRUCTION_TRANSACTION finalTransaction{};
			finalTransaction.iEncounterEpoch =
				m_WorldDestructionRuntime.Get_EncounterEpoch();
			finalTransaction.iRequestTick = resetTick;
			for (FINAL_TRANSITION_PAIR& pair : stagedPairs)
			{
				finalTransaction.BindingApplications.push_back(
					std::move(pair.Application));
				finalTransaction.Transitions.push_back(
					std::move(pair.Transition));
			}
			if (!Commit_WorldDestructionTransaction(
				finalTransaction, {}, resetTick, resetStatus))
			{
				m_strStatus = "Final arena transaction failed: " + resetStatus;
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (isWallAttackPlay)
		{
			const auto playerId = m_PlayerIdBySessionId.find(sessionId);
			if (m_PlayerIdBySessionId.end() == playerId)
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
			auto player = m_Players.find(playerId->second);
			if (m_Players.end() == player || 0u == player->second.iCurrentHp)
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
			SERVER_NAV_POINT attackTarget{};
			if (!m_ServerNavigation.Project_Point(
				WALL_ATTACK_CENTER_X,
				WALL_ATTACK_CENTER_Z + WALL_ATTACK_PLAYER_OFFSET_Z,
				attackTarget))
			{
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
			}

			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			boss->fPositionX = WALL_ATTACK_CENTER_X;
			boss->fPositionY = WALL_ATTACK_CENTER_Y;
			boss->fPositionZ =
				WALL_ATTACK_CENTER_Z + WALL_ATTACK_BOSS_OFFSET_Z;
			boss->fYawDegrees = 0.f;
			boss->MovePath.clear();
			/* A single-pattern audition replays the pattern it names, not the
			entrance, so the intro is spent before the request is queued. */
			boss->bIntroPatternConsumed = true;
			boss->PendingPatternIds.push_back(auditionPatternId);

			player = m_Players.find(playerId->second);
			if (m_Players.end() == player)
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
			player->second.fPositionX = attackTarget.x;
			player->second.fPositionY = attackTarget.y;
			player->second.fPositionZ = attackTarget.z;
			player->second.isCombatReady = true;
			player->second.eAction = PLAYER_ACTION_STATE::NONE;
			player->second.iCurrentSkillId = INVALID_SKILL_ID;
			player->second.Clear_SkillTarget();
			player->second.iActionStartTick = resetTick;
			player->second.fActionElapsedSeconds = 0.f;
			player->second.iComboStage = 0u;
			player->second.hasBufferedComboInput = false;
			player->second.PendingCommand.Clear();
			player->second.TriggerMove = {};
			player->second.MovePath.clear();
			player->second.iMovePathIndex = 0u;
			player->second.hasMoveGoal = false;
			player->second.fKnockbackRemainingSeconds = 0.f;
			player->second.fKnockbackSpeed = 0.f;
			player->second.iKnockdownEndTick = 0u;
			player->second.iHitReactionGraceEndTick = 0u;
			if (!Has_EngagedAuditionPlayer(*boss))
			{
				boss->PendingPatternIds.clear();
				return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
			}

			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			m_iValtanAuditionArmedHealthBar = 0u;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (isOneClickPlay)
		{
			const auto playerId = m_PlayerIdBySessionId.find(sessionId);
			if (m_PlayerIdBySessionId.end() != playerId)
			{
				const auto player = m_Players.find(playerId->second);
				if (m_Players.end() != player && 0u != player->second.iCurrentHp)
				{
					player->second.isCombatReady = true;
					if (!m_ServerTriggerSystem.Place_PlayerAtValtanAuditionBait(
						player->second,
						0u == m_iServerTick ? 1u : m_iServerTick))
					{
						return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;
					}
				}
			}
		}
		if (!Has_EngagedAuditionPlayer(*boss))
			return VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED;

		if (VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR == request.eOperation)
		{
			const std::uint32_t armedBar = request.iTargetHealthBar + 1u;
			const std::uint32_t armedHp =
				CValtanBrain::Resolve_HealthBarHp(*boss, armedBar);
			if (0u == armedHp)
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

			boss->iCurrentHp = armedHp;
			/* The brain compares this against the bar it recomputes, so parking
			it one above the target is what limits the next crossing to the one
			authored threshold. */
			boss->iLastEvaluatedHealthBar = armedBar;
			m_iValtanAuditionArmedHealthBar = request.iTargetHealthBar;
			currentHealthBar = armedBar;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::ARMED;
		}

		if (VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE == request.eOperation)
		{
			/* The only operation that hands the intro ledger back, so the boss
			replays its one first-appearance sweep against the two front walls
			through the same Brain path a fresh encounter uses. */
			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			boss->bIntroPatternConsumed = false;
			boss->bScriptedPatternPlayback = false;
			m_iValtanAuditionArmedHealthBar = 0u;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (isPatternPlay)
		{
			/* The named pattern is queued the way a crossed health bar queues its
			mechanic, so the Brain starts it on the next tick and it runs the same
			product stage, camera and destruction path instead of a second
			Debug-only playback. */
			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			boss->bIntroPatternConsumed = true;
			boss->PendingPatternIds.push_back(auditionPatternId);
			m_iValtanAuditionArmedHealthBar = 0u;
			currentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR == request.eOperation ||
			isPillarCyclePlay)
		{
			std::string resetStatus;
			const std::uint32_t resetTick =
				0u == m_iServerTick ? 1u : m_iServerTick;
			if (!Reset_ValtanAuditionState(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}
			const std::uint32_t previousBar = targetHealthBar + 1u;
			const std::uint32_t targetHp =
				CValtanBrain::Resolve_HealthBarHp(*boss, targetHealthBar);
			if (previousBar > boss->iMaximumHealthBars || 0u == targetHp)
				return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

			/* A bar whose pattern takes the ground away is only ever reached after
			the fight has already broken every wall, so auditioning it inside a
			walled arena hides the collapse behind them. The walls come down in
			this same request because a second one would give the boss a tick to
			start an ordinary pattern and the bar would then be rejected. */
			const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& auditionGraph =
				m_WorldDestructionBootstrap.Get_DescriptorGraph();
			const bool patternRemovesGround = std::any_of(
				auditionGraph.Bindings.begin(), auditionGraph.Bindings.end(),
				[&auditionGraph, &auditionPatternId](
					const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding)
				{
					if (WORLD_DESTRUCTION_TRIGGER_KIND::STAGE !=
						binding.eTriggerKind ||
						binding.strPatternId != auditionPatternId)
					{
						return false;
					}
					const auto mutation = std::find_if(
						auditionGraph.Mutations.begin(),
						auditionGraph.Mutations.end(),
						[&binding](
							const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& candidate)
						{
							return candidate.strMutationId == binding.strMutationId;
						});
					return auditionGraph.Mutations.end() != mutation &&
						mutation->bRemovesGround;
				});
			if (patternRemovesGround &&
				!Break_EveryWallForAudition(*boss, resetTick, resetStatus))
			{
				m_strStatus = std::move(resetStatus);
				return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
			}

			/* PLAY is one atomic Debug command. Unlike two UI clicks, there is no
			brain tick between priming the previous bar and crossing the target,
			so a normal pattern cannot steal the audition window. */
			boss->iCurrentHp = targetHp;
			boss->iLastEvaluatedHealthBar = previousBar;
			m_bPillarAuditionCycleArmed = isPillarCyclePlay;
			m_iValtanAuditionArmedHealthBar = 0u;
			currentHealthBar = targetHealthBar;
			m_ValtanAuditionSequenceBySessionId.insert_or_assign(
				sessionId, request.iRequestSequence);
			return VALTAN_AUDITION_RESULT::QUEUED;
		}

		if (m_iValtanAuditionArmedHealthBar != request.iTargetHealthBar)
			return VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED;
		const std::uint32_t targetHp =
			CValtanBrain::Resolve_HealthBarHp(*boss, request.iTargetHealthBar);
		if (0u == targetHp)
			return VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR;

		/* iLastEvaluatedHealthBar is deliberately left where ARM put it. The
		next CValtanBrain tick is what observes the crossing and queues the
		pattern, so the camera cue, the wall binding and the debris all run
		through the product path rather than being started from here. */
		boss->iCurrentHp = targetHp;
		m_iValtanAuditionArmedHealthBar = 0u;
		currentHealthBar = request.iTargetHealthBar;
		m_ValtanAuditionSequenceBySessionId.insert_or_assign(
			sessionId, request.iRequestSequence);
		return VALTAN_AUDITION_RESULT::QUEUED;
	};
	return evaluate();
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
		if (!append(entity.PinnedDefinitionRevision))
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
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE !=
			m_ValtanPatternIdAudition.ePhase &&
		!append(m_ValtanPatternIdAudition.PinnedDefinitionRevision))
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
	/* An idle boss makes its next decision from the process-active generation.
	   Once BeginPattern publishes a stable pattern id, every later stage lookup
	   resolves the occurrence pin until that pattern is completely retired. */
	#ifdef _DEBUG
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

bool LostArk::Server::CGameRoom::Send_Accepted(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_ENTER_ACCEPTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		return false;
	}
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_EnterRejected(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_REJECTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_SPAWNED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.eCharacterClass = player.eCharacterClass;
	message.strNickName = player.strNickName;
	message.fPositionX = player.fPositionX;
	message.fPositionY = player.fPositionY;
	message.fPositionZ = player.fPositionZ;
	message.fYawDegrees = player.fYawDegrees;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_WORLD_ENTITY& entity)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.eKind = To_NetworkKind(entity.eKind);
	message.strArchetypeId = entity.strArchetypeId;
	message.strEncounterId = entity.strEncounterId;
	message.strPlacementId = entity.strPlacementId;
	message.strActionId = entity.strActionId;
	message.fPositionX = entity.fPositionX;
	message.fPositionY = entity.fPositionY;
	message.fPositionZ = entity.fPositionZ;
	message.fYawDegrees = entity.fYawDegrees;
	message.fCollisionRadius = entity.fCollisionRadius;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CombatObjectSpawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, spawned) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Despawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldDestructionFullSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	if (nullptr == session || !m_WorldDestructionRuntime.Is_Initialized())
		return false;

	S2C_WORLD_DESTRUCTION_FULL_SYNC message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_GROUP_STATE& state :
		m_WorldDestructionRuntime.Get_GroupStates())
	{
		message.GroupStates.push_back(To_NetworkDestructionState(state));
	}
	message.Diagnostics = Build_WorldDestructionDiagnostics();
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, writer.Get_Buffer());
}

LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
LostArk::Server::CGameRoom::Build_WorldDestructionDiagnostics() const
{
	LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS diagnostics{};
	diagnostics.iActiveWallCollisionCount = static_cast<std::uint32_t>(
		m_ServerCollisionSystem.Get_ActivePlayerBlockingCount());
	diagnostics.iActiveNavBlockerRegionCount = static_cast<std::uint32_t>(
		m_ServerNavigation.Get_ActiveBlockerRegionCount());
	diagnostics.iNavigationRevision = m_ServerNavigation.Get_Revision();
	diagnostics.iLastEventSequence =
		0u == m_iNextWorldDestructionEventSequence ?
		0u : m_iNextWorldDestructionEventSequence - 1u;
	return diagnostics;
}

void LostArk::Server::CGameRoom::Broadcast_Spawned(
	const SERVER_PLAYER& player,
	const SESSION_ID exceptSessionId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (sessionId == exceptSessionId)
			continue;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Despawned(session, netEntityId, reason))
			session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawnResult(
	const std::shared_ptr<CClientSession>& session,
	const std::string& placementId,
	const LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWN_RESULT message{};
	message.strPlacementId = placementId;
	message.eResult = result;
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_InventorySnapshot(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t requestSequence,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& inventory)
{
	using namespace LostArk::Shared;
	S2C_INVENTORY_SNAPSHOT message{};
	message.iRequestSequence = requestSequence;
	message.Items = inventory;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	const LostArk::Shared::VALTAN_AUDITION_RESULT result,
	const std::uint32_t currentHealthBar)
{
	using namespace LostArk::Shared;
	S2C_VALTAN_AUDITION_RESULT message{};
	message.iRequestSequence = request.iRequestSequence;
	message.eOperation = request.eOperation;
	message.iTargetHealthBar = request.iTargetHealthBar;
	message.eResult = result;
	message.iCurrentHealthBar = currentHealthBar;
	message.strBossPlacementId = request.strBossPlacementId;
	message.strPatternId = request.strPatternId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CharacterClassChangeResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
	const LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
	const LostArk::Shared::CHARACTER_CLASS_ID activeClass)
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT message{};
	message.iClientSequence = request.iClientSequence;
	message.eResult = result;
	message.eRequestedClass = request.eCharacterClass;
	message.eActiveClass = activeClass;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT,
			writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntitySpawned(
	const SERVER_WORLD_ENTITY& entity)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldEntitySpawned(session, entity))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntityDespawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId))
		{
			session->Request_Close();
		}
	}
}

bool LostArk::Server::CGameRoom::Broadcast_CombatObjectLifecycle()
{
	using namespace LostArk::Shared;
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	m_CombatObjectRuntime.Drain_Lifecycle(spawned, despawned);
	for (const S2C_COMBAT_OBJECT_SPAWNED& message : spawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	for (const S2C_COMBAT_OBJECT_DESPAWNED& message : despawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Broadcast_WorldDestructionDelta(
	const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (transitions.empty() && liveEvents.empty())
		return true;

	S2C_WORLD_DESTRUCTION_DELTA message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = serverTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition : transitions)
	{
		WORLD_DESTRUCTION_GROUP_STATE state{};
		if (!m_WorldDestructionRuntime.Find_GroupState(
			transition.strGroupId, state))
		{
			return false;
		}
		message.ChangedStates.push_back(To_NetworkDestructionState(state));
	}
	std::sort(message.ChangedStates.begin(), message.ChangedStates.end(),
		[](const WORLD_DESTRUCTION_STATE_WIRE& left,
			const WORLD_DESTRUCTION_STATE_WIRE& right)
		{
			return left.strGroupId < right.strGroupId;
		});
	message.LiveEvents = liveEvents;
	message.Diagnostics = Build_WorldDestructionDiagnostics();

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.iEstherGauge = m_EstherSkillSystem.Get_Gauge();
	message.iEstherGaugeMaximum = m_EstherSkillSystem.Get_GaugeMaximum();
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		m_strStatus = "Live gameplay revision pins are invalid or exceed wire bounds";
		return;
	}
	message.Players.reserve(m_Players.size());
	message.Entities.reserve(m_WorldEntities.size());
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.eCharacterClass = player.eCharacterClass;
		snapshot.fPositionX = player.fPositionX;
		snapshot.fPositionY = player.fPositionY;
		snapshot.fPositionZ = player.fPositionZ;
		snapshot.fYawDegrees = player.fYawDegrees;
		snapshot.eLocomotionState =
			(player.hasMoveGoal || player.TriggerMove.isActive) ?
			PLAYER_LOCOMOTION_STATE::MOVING : PLAYER_LOCOMOTION_STATE::IDLE;
		snapshot.eAction = player.eAction;
		snapshot.eStance = player.eStance;
		snapshot.iSkillId = player.iCurrentSkillId;
		snapshot.iActionStartTick = player.iActionStartTick;
		if (PLAYER_ACTION_STATE::SKILL == player.eAction &&
			player.hasSkillTarget)
		{
			snapshot.hasSkillTarget = true;
			snapshot.fSkillTargetX = player.fSkillTargetX;
			snapshot.fSkillTargetY = player.fSkillTargetY;
			snapshot.fSkillTargetZ = player.fSkillTargetZ;
		}
		snapshot.iCurrentHp = player.iCurrentHp;
		snapshot.iMaximumHp = player.iMaximumHp;
		snapshot.iCurrentResource = player.iCurrentResource;
		snapshot.iMaximumResource = player.iMaximumResource;
		snapshot.iCurrentIdentity = player.iCurrentIdentity;
		snapshot.iMaximumIdentity = player.iMaximumIdentity;
		snapshot.isCombatReady = player.isCombatReady;
		snapshot.iComboStage = player.iComboStage;
		/* Collect, sort, then truncate: cutting during unordered_map iteration
		made the surviving cooldowns depend on hash order. Signed difference keeps
		ordering across a wrapped tick counter. */
		for (const auto& [skillId, cooldownEndTick] :
			player.CooldownEndTickBySkillId)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) > 0)
				snapshot.Cooldowns.push_back({ skillId, cooldownEndTick });
		}
		std::sort(snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[](const SKILL_COOLDOWN_SNAPSHOT& left,
				const SKILL_COOLDOWN_SNAPSHOT& right)
			{
				return left.iSkillId < right.iSkillId;
			});
		if (snapshot.Cooldowns.size() > MAX_PLAYER_COOLDOWNS)
			snapshot.Cooldowns.resize(MAX_PLAYER_COOLDOWNS);
		message.Players.push_back(snapshot);
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		WORLD_ENTITY_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = entity.iNetEntityId;
		snapshot.eAction = To_NetworkAction(entity.eAction);
		snapshot.strPatternId = entity.strPatternId;
		if (entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY)
		{
			snapshot.strActionId = entity.strActionId;
		}
		snapshot.fPositionX = entity.fPositionX;
		snapshot.fPositionY = entity.fPositionY;
		snapshot.fPositionZ = entity.fPositionZ;
		snapshot.fYawDegrees = entity.fYawDegrees;
		snapshot.iActionStartTick = entity.iActionStartTick;
		snapshot.iPatternSequence = entity.iPatternSequence;
		snapshot.iPatternStageIndex = entity.iPatternStageIndex;
		snapshot.iCurrentHp = entity.iCurrentHp;
		snapshot.iMaximumHp = entity.iMaximumHp;
		snapshot.iPhase = entity.iPhase;
		snapshot.PinnedDefinitionRevision =
			entity.PinnedDefinitionRevision;
		/* Presentation only needs to know which plates came off, not how much
		durability is left, so the wire carries one bit per authored plate. */
		for (const SERVER_BOSS_ARMOR_PLATE_STATE& plate : entity.ArmorPlates)
		{
			if (0u == plate.iRemainingDurability &&
				plate.iPlateIndex <
					LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES)
			{
				snapshot.iBrokenArmorMask |= static_cast<std::uint8_t>(
					1u << plate.iPlateIndex);
			}
		}
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			snapshot.hasBossCombatState = true;
			snapshot.BossCombat.iStateRevision =
				entity.BossCombat.iStateRevision;
			snapshot.BossCombat.iAlivePartMask =
				entity.BossCombat.iAlivePartMask;
			snapshot.BossCombat.iFlags = static_cast<std::uint16_t>(
				entity.BossCombat.iFlags) &
				BOSS_COMBAT_STATE_KNOWN_FLAG_MASK;
			snapshot.BossCombat.iCurrentStagger =
				entity.BossCombat.iStaggerCurrent;
			snapshot.BossCombat.iMaximumStagger =
				entity.BossCombat.iStaggerMaximum;
			snapshot.BossCombat.iCurrentShield =
				entity.BossCombat.iShieldCurrent;
			snapshot.BossCombat.iMaximumShield =
				entity.BossCombat.iShieldMaximum;
			/* The existing gameplay phase remains the one authority. The boss
			payload mirrors it rather than introducing a second phase clock. */
			snapshot.BossCombat.iGameplayPhase = entity.iPhase;
		}
		message.Entities.push_back(std::move(snapshot));
	}
	message.DamageEvents = m_TickDamageEvents;
	message.BossCombatEvents = m_TickBossCombatEvents;
	if (!m_CombatObjectRuntime.Build_Snapshots(message.CombatObjects))
		return;

	const auto encodeStart = std::chrono::steady_clock::now();
	CPacketWriter writer;
	const bool encoded = Write_Message(writer, message);
	const std::uint64_t encodeMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - encodeStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEncodeCount;
		m_PerformanceMetrics.iLastSnapshotEncodeMicroseconds =
			encodeMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds,
			encodeMicroseconds);
		if (!encoded)
			++m_PerformanceMetrics.iSnapshotEncodeFailureCount;
	}
	if (!encoded)
		return;

	const auto enqueueBatchStart = std::chrono::steady_clock::now();
	std::uint64_t maximumSessionEnqueueMicroseconds = 0u;
	std::uint64_t recipientCount = 0u;
	std::uint64_t enqueueFailureCount = 0u;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr == session)
			continue;
		++recipientCount;
		const auto sessionEnqueueStart = std::chrono::steady_clock::now();
		const bool enqueued = session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, writer.Get_Buffer());
		const std::uint64_t sessionEnqueueMicroseconds = To_Microseconds(
			std::chrono::steady_clock::now() - sessionEnqueueStart);
		maximumSessionEnqueueMicroseconds = (std::max)(
			maximumSessionEnqueueMicroseconds, sessionEnqueueMicroseconds);
		if (!enqueued)
		{
			++enqueueFailureCount;
			session->Request_Close();
		}
	}
	const std::uint64_t enqueueBatchMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - enqueueBatchStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEnqueueBatchCount;
		m_PerformanceMetrics.iSnapshotRecipientCount += recipientCount;
		m_PerformanceMetrics.iSnapshotEnqueueFailureCount += enqueueFailureCount;
		m_PerformanceMetrics.iLastSnapshotEnqueueMicroseconds =
			enqueueBatchMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds,
			enqueueBatchMicroseconds);
		m_PerformanceMetrics.iLastMaximumSessionEnqueueMicroseconds =
			maximumSessionEnqueueMicroseconds;
		m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds,
			maximumSessionEnqueueMicroseconds);
	}
}

std::shared_ptr<LostArk::Server::CClientSession>
LostArk::Server::CGameRoom::Find_Session(const SESSION_ID sessionId) const
{
	const auto iter = m_Sessions.find(sessionId);
	return iter == m_Sessions.end() ? nullptr : iter->second.lock();
}

void LostArk::Server::CGameRoom::Rollback_Join(const SESSION_ID sessionId)
{
	using namespace LostArk::Shared;
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
}

bool LostArk::Server::CGameRoom::Is_PlayerAdmissionFull() const
{
	return nullptr == Find_AvailablePlayerSpawn();
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_AvailablePlayerSpawn() const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN)
		{
			continue;
		}
		const bool isUsed = std::any_of(
			m_Players.begin(), m_Players.end(),
			[&placement](const auto& playerEntry)
			{
				return playerEntry.second.strSpawnPlacementId ==
					placement.strPlacementId;
			});
		if (!isUsed)
			return &placement;
	}
	return nullptr;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_Placement(
	const std::string& placementId) const
{
	const auto& placements = m_WorldBootstrap.Get_Placements();
	const auto iter = std::find_if(
		placements.begin(),
		placements.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == placementId;
		});
	return placements.end() != iter ? &*iter : nullptr;
}

bool LostArk::Server::CGameRoom::Build_WorldEntity(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	SERVER_WORLD_ENTITY& outEntity)
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind)
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.PinnedDefinitionRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!staged.PinnedDefinitionRevision.Is_Valid())
	{
		m_strStatus = "Active gameplay revision is unavailable";
		return false;
	}
	staged.strPlacementId = placement.strPlacementId;
	staged.strArchetypeId = placement.strArchetypeId;
	staged.strEncounterId = placement.strEncounterId;
	staged.eKind = placement.eKind;
	staged.fPositionX = placement.fPositionX;
	staged.fPositionY = placement.fPositionY;
	staged.fPositionZ = placement.fPositionZ;
	staged.fYawDegrees = placement.fYawDegrees;
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			m_GameplayCatalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			m_GameplayCatalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		const std::vector<BOSS_PART_DEFINITION>* bossParts =
			m_GameplayCatalog.Find_BossParts(staged.strArchetypeId);
		const std::vector<BOSS_PART_DEFINITION> noBossParts;
		std::string combatStatus;
		if (!CBossCombatRuntime::Initialize(
			staged.BossCombat,
			nullptr == bossParts ? noBossParts : *bossParts,
			combatStatus))
		{
			m_strStatus = std::move(combatStatus);
			return false;
		}
		staged.iCurrentHp = profile->iMaximumHp;
		staged.iMaximumHp = profile->iMaximumHp;
		staged.iMaximumHealthBars = profile->iMaximumHealthBars;
		staged.iLastEvaluatedHealthBar = profile->iMaximumHealthBars;
		staged.iAttackPower = profile->iAttackPower;
		staged.fCollisionRadius = profile->fCollisionRadius;
		staged.fEngageDistance = profile->fEngageDistance;
		staged.fMoveSpeed = profile->fMoveSpeed;
		staged.PhasePolicy = profile->PhasePolicy;
		staged.iPhaseTwoHpPercent = profile->PhasePolicy.iThresholdPercent;
		/* Every plate starts intact. A boss with no authored plates keeps an
		empty list and therefore no mitigation, which is the pre-armour rule. */
		staged.ArmorPlates.clear();
		staged.ArmorPlates.reserve(profile->ArmorPlates.size());
		for (const BOSS_ARMOR_PLATE& plate : profile->ArmorPlates)
		{
			SERVER_BOSS_ARMOR_PLATE_STATE state{};
			state.iPlateIndex = plate.iPlateIndex;
			state.iRemainingDurability = plate.iDurability;
			state.iDefense = plate.iDefense;
			staged.ArmorPlates.push_back(state);
		}
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX,
				staged.fPositionZ,
				projected))
			{
				m_strStatus = "Boss placement is outside server navigation";
				return false;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}
	staged.fSpawnPositionX = staged.fPositionX;
	staged.fSpawnPositionY = staged.fPositionY;
	staged.fSpawnPositionZ = staged.fPositionZ;
	outEntity = std::move(staged);
	return true;
}

bool LostArk::Server::CGameRoom::Initialize_WorldEntities()
{
	m_WorldEntities.clear();
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::COLLISION_BOX)
		{
			continue;
		}
		if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		{
			m_strStatus = "World entity ID space exhausted";
			return false;
		}
		SERVER_WORLD_ENTITY entity{};
		if (!Build_WorldEntity(placement, m_iNextNetEntityId, entity))
			return false;
		++m_iNextNetEntityId;
		m_WorldEntities.push_back(std::move(entity));
	}
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ReplayableArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId &&
		WORLD_ID::VALTAN_ARENA != m_eWorldId) || !m_Players.empty())
		return true;

	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		m_isReady = false;
		return false;
	}
	m_CombatObjectRuntime.Reset();
	m_CombatObjectRuntime.Discard_PendingLifecycle();
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	m_iNextBossCombatEventSequence = 1u;
	m_ValtanPatternIdAuditionSequenceBySessionId.clear();
#ifdef _DEBUG
	m_ValtanPatternIdAudition = {};
#endif
	m_strStatus = "Replayable arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId || !m_Players.empty())
		return true;

	std::string resetStatus;
	const std::uint32_t resetTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
#ifdef _DEBUG
	m_ValtanTimelineAudition = {};
#endif
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!m_EncounterPropRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!m_WorldDestructionRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iNextBossCombatEventSequence = 1u;
	if (!Initialize_WorldEntities())
	{
		m_isReady = false;
		return false;
	}
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	/* The encounter is fresh, so a bar armed by the previous occupants no
	longer describes any live boss. Leave already dropped their sequences. */
	m_iValtanAuditionArmedHealthBar = 0u;
	// The next party charges its Esther gauge from zero; any live summon was
	// already discarded with the entity rebuild above, and a summon still in
	// its landing delay has no party left to land for.
	m_PendingEstherSummons.clear();
	m_EstherSkillSystem.Reset();
	m_strStatus = "Valtan arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Apply_EncounterPropStageEntry(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_EncounterPropRuntime.Is_Initialized() ||
		0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strPatternStageId.empty())
	{
		return true;
	}
	/* A later pattern owns each shatter, because the stele set outlives the
	   pattern that raised it. The stage edge names the pair it breaks, so the
	   pair that goes leaves the opposite pair standing as the cover the raid
	   moves to. */
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Encounter prop pinned gameplay generation is missing";
		return false;
	}
	if (const std::vector<BOSS_PATTERN_DEFINITION>* propBreakPatterns =
		occurrenceCatalog->Find_BossPatterns(boss.strEncounterId))
	{
		const auto propBreakPattern = std::find_if(
			propBreakPatterns->begin(), propBreakPatterns->end(),
			[&boss](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == boss.strPatternId; });
		if (propBreakPatterns->end() != propBreakPattern &&
			boss.iPatternStageIndex < propBreakPattern->Stages.size())
		{
			const BOSS_PATTERN_STAGE_DEFINITION& propBreakStage =
				propBreakPattern->Stages[boss.iPatternStageIndex];
			const auto& propSlots = m_EncounterPropRuntime.Get_SlotStates();
			bool allowScriptedPropBreak = false;
#ifdef _DEBUG
			allowScriptedPropBreak = boss.bScriptedPatternPlayback &&
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
					m_ValtanTimelineAudition.ePhase &&
				boss.iNetEntityId == m_ValtanTimelineAudition.iBossEntityId &&
				m_ValtanTimelineAudition.bAllowProductPropBreak;
#endif
			/* Only a raised pair can shatter. The wave is an ordinary rotation
			   pattern that also runs when no stele stands, and asking to break a
			   hidden slot is a rejection, not a no-op. */
			const bool everyNamedSlotRaised =
				!propBreakStage.PropBreakSlotIds.empty() &&
				/* Generic scripted playback suppresses incidental prop breaks. A
				   timeline row that explicitly prepared four intact pillars is the
				   one exception: its real red-blade stages own the two pairs. */
				(!boss.bScriptedPatternPlayback || allowScriptedPropBreak) &&
				propBreakStage.strPropBreakSetId ==
					m_EncounterPropRuntime.Get_PropSetId() &&
				propBreakStage.strActionId == boss.strActionId &&
				std::all_of(
					propBreakStage.PropBreakSlotIds.begin(),
					propBreakStage.PropBreakSlotIds.end(),
					[&propSlots](const std::string& slotId)
					{
						const auto found = std::find_if(
							propSlots.begin(), propSlots.end(),
							[&slotId](const ENCOUNTER_PROP_SLOT_STATE& candidate)
							{ return candidate.strSlotId == slotId; });
						return propSlots.end() != found &&
							LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT ==
								found->eState;
					});
			if (everyNamedSlotRaised)
			{
				ENCOUNTER_PROP_TRANSACTION breakTransaction{};
				std::string breakStatus;
				if (ENCOUNTER_PROP_PREPARE_RESULT::READY ==
					m_EncounterPropRuntime.Prepare_BreakSlots(
						propBreakStage.PropBreakSlotIds,
						m_EncounterPropRuntime.Get_OccurrenceSequence(),
						serverTick, breakTransaction, breakStatus) &&
					m_EncounterPropRuntime.Commit(breakTransaction, breakStatus))
				{
					Broadcast_EncounterPropSync();
				}
				else if (!breakStatus.empty())
				{
					/* A refused shatter is isolated the same way a refused raise is. */
					m_strStatus = std::move(breakStatus);
				}
			}
		}
	}
	if (PILLAR_PATTERN_ID != boss.strPatternId ||
		PILLAR_SPAWN_STAGE_ID != boss.strPatternStageId)
	{
		return true;
	}

	ENCOUNTER_PROP_TRANSACTION transaction{};
	std::string status;
	const ENCOUNTER_PROP_PREPARE_RESULT result =
		m_EncounterPropRuntime.Prepare_Spawn(
			boss.iPatternSequence, serverTick, transaction, status);
	if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE == result)
		return true;
	if (ENCOUNTER_PROP_PREPARE_RESULT::READY != result ||
		!m_EncounterPropRuntime.Commit(transaction, status))
	{
		/* A rejected prop edge is isolated: it never fails the room tick and
		   never touches wall, collision or navigation state. */
		m_strStatus = std::move(status);
		return true;
	}
	/* The Debug audition asked for a whole cycle, so the raise it just observed
	   schedules the shatter the product path has no owner for yet. */
	if (m_bPillarAuditionCycleArmed)
	{
		m_iPillarAuditionBreakTick =
			Add_ServerTicksSkippingReservedZero(
				serverTick, PILLAR_AUDITION_DWELL_TICKS);
		m_bPillarAuditionCycleArmed = false;
	}
	Broadcast_EncounterPropSync();
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageActions(
	SERVER_WORLD_ENTITY& boss,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	const std::uint32_t spawnWaveOrdinal)
{
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Boss stage action pinned gameplay generation is missing";
		return false;
	}
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	if (!Stage_BossPatternStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger, serverTick,
		stagedCombat, stagedGameplayPhase, transaction, spawnWaveOrdinal))
	{
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	return true;
}

bool LostArk::Server::CGameRoom::Resolve_ArenaRandomVolleyOrigins(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const BOSS_COMBAT_OBJECT_DEFINITION& definition,
	const std::uint32_t spawnWaveOrdinal,
	std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins)
{
	outOrigins.clear();
	const BOSS_COMBAT_OBJECT_VOLLEY& volley = action.Volley;
	if (0u == volley.iArenaRandomCount)
		return true;
	if (!m_ServerNavigation.Is_Loaded() ||
		BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION !=
			volley.eArenaAnchorPolicy ||
		0u == boss.iNetEntityId || 0u == boss.iPatternSequence ||
		!std::isfinite(boss.fSpawnPositionX) ||
		!std::isfinite(boss.fSpawnPositionY) ||
		!std::isfinite(boss.fSpawnPositionZ) ||
		!std::isfinite(volley.fArenaRandomRadiusM) ||
		volley.fArenaRandomRadiusM <= 0.f ||
		!std::isfinite(volley.fArenaHeightToleranceM) ||
		volley.fArenaHeightToleranceM <= 0.f)
	{
		m_strStatus = "Boss arena-random volley contract is invalid";
		return false;
	}

	const float minimumSpacing = Resolve_VolleyMinimumSpacing(definition);
	if (!std::isfinite(minimumSpacing) || minimumSpacing <= 0.f)
	{
		m_strStatus = "Boss arena-random volley spacing is invalid";
		return false;
	}
	const float minimumSpacingSquared = minimumSpacing * minimumSpacing;
	/* Arena-random authoring defines a valid origin, not a guarantee that every
	   cell under the 3.5m damage circle is walkable. The Valtan nav paint has
	   intentional seams inside the arena, so admission pins the exact centre to
	   authoritative navigation/height and keeps whole circles apart by their
	   damage diameter. */
	const auto IsSpawnPointWalkable =
		[this, &boss, &volley](
			const float centerX, const float centerZ, float& outY)
		{
			SERVER_NAV_POINT center{};
			if (!m_ServerNavigation.Is_PointWalkableExact(centerX, centerZ) ||
				!m_ServerNavigation.Sample_Position(centerX, centerZ, center) ||
				std::abs(center.y - boss.fSpawnPositionY) >
					volley.fArenaHeightToleranceM)
			{
				return false;
			}
			outY = center.y;
			return true;
		};

	try
	{
		outOrigins.reserve(volley.iArenaRandomCount);
	}
	catch (const std::bad_alloc&)
	{
		m_strStatus = "Boss arena-random volley allocation failed";
		return false;
	}
	const std::uint64_t baseSeed = Mix_DeterministicRandom(
		Hash_StableId(action.strTargetId) ^
		(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
		static_cast<std::uint64_t>(boss.iPatternSequence) ^
		(static_cast<std::uint64_t>(spawnWaveOrdinal) << 48u));
	for (std::uint32_t attempt = 0u;
		attempt < ARENA_RANDOM_MAXIMUM_ATTEMPTS &&
		outOrigins.size() < volley.iArenaRandomCount;
		++attempt)
	{
		const std::uint64_t attemptSeed = baseSeed ^
			(static_cast<std::uint64_t>(attempt + 1u) *
				0xd6e8feb86659fd93ull);
		const float radius = std::sqrt(
			DeterministicUnitFloat(attemptSeed)) *
			volley.fArenaRandomRadiusM;
		const float angle = DeterministicUnitFloat(
			attemptSeed ^ 0xa0761d6478bd642full) * TWO_PI;
		const float x = boss.fSpawnPositionX + std::cos(angle) * radius;
		const float z = boss.fSpawnPositionZ + std::sin(angle) * radius;
		float y = 0.f;
		if (!IsSpawnPointWalkable(x, z, y))
			continue;
		const bool overlaps = std::any_of(
			outOrigins.begin(), outOrigins.end(),
			[x, z, minimumSpacingSquared](
				const SERVER_COMBAT_OBJECT_LOCKED_TARGET& existing)
			{
				const float deltaX = x - existing.fPositionX;
				const float deltaZ = z - existing.fPositionZ;
				return deltaX * deltaX + deltaZ * deltaZ +
					VOLLEY_SPACING_EPSILON < minimumSpacingSquared;
			});
		if (overlaps)
			continue;
		SERVER_COMBAT_OBJECT_LOCKED_TARGET origin{};
		origin.fPositionX = x;
		origin.fPositionY = y;
		origin.fPositionZ = z;
		origin.bTrackUntilFirstPulse = false;
		outOrigins.push_back(origin);
	}
	if (outOrigins.size() != volley.iArenaRandomCount)
	{
		outOrigins.clear();
		m_strStatus = "Boss arena-random volley has no valid point set";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternScheduledSpawnWave(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (boss.strPatternId.empty() || boss.strActionId.empty() ||
		0u == boss.iActionStartTick)
	{
		return true;
	}
	const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == catalog)
	{
		m_strStatus = "Boss scheduled volley pinned gameplay generation is missing";
		return false;
	}
	const auto* patterns = catalog->Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Boss scheduled volley encounter is missing";
		return false;
	}
	const auto pattern = std::find_if(
		patterns->begin(), patterns->end(),
		[&boss](const BOSS_PATTERN_DEFINITION& candidate)
		{ return candidate.strPatternId == boss.strPatternId; });
	if (patterns->end() == pattern)
	{
		m_strStatus = "Boss scheduled volley pattern is missing";
		return false;
	}
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&boss](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{ return candidate.strActionId == boss.strActionId; });
	if (pattern->Stages.end() == stage)
	{
		m_strStatus = "Boss scheduled volley stage is missing";
		return false;
	}
	const BOSS_PATTERN_STAGE_ACTION* scheduledAction = nullptr;
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
				action.eKind ||
			action.Volley.iSpawnCount <= 1u)
		{
			continue;
		}
		if (nullptr != scheduledAction &&
			(scheduledAction->Volley.iSpawnCount != action.Volley.iSpawnCount ||
			 scheduledAction->Volley.iSpawnIntervalMs !=
				action.Volley.iSpawnIntervalMs))
		{
			m_strStatus = "Boss scheduled volleys do not share one clock";
			return false;
		}
		scheduledAction = &action;
	}
	if (nullptr == scheduledAction)
		return true;
	if (0u == boss.iAppliedPatternStageSpawnWaveCount)
	{
		m_strStatus = "Boss scheduled volley ENTER wave was not committed";
		return false;
	}
	if (boss.iAppliedPatternStageSpawnWaveCount >=
		scheduledAction->Volley.iSpawnCount)
	{
		return true;
	}
	const std::uint32_t waveOrdinal =
		boss.iAppliedPatternStageSpawnWaveCount;
	const std::uint64_t dueMilliseconds =
		static_cast<std::uint64_t>(waveOrdinal) *
		scheduledAction->Volley.iSpawnIntervalMs;
	const std::uint64_t elapsedTicks =
		Elapsed_ServerTicksSkippingReservedZero(
			boss.iActionStartTick, serverTick);
	if (elapsedTicks * 1000ull <
		dueMilliseconds * static_cast<std::uint64_t>(SERVER_TICK_HZ))
	{
		return true;
	}
	if (!Apply_BossPatternStageActions(
		boss, boss.strPatternId, boss.strActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
		serverTick, waveOrdinal))
	{
		return false;
	}
	++boss.iAppliedPatternStageSpawnWaveCount;
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageTransition(
	SERVER_WORLD_ENTITY& boss,
	const std::string& previousPatternId,
	const std::string& previousActionId,
	const std::string& nextPatternId,
	const std::string& nextActionId,
	const LostArk::Shared::GameplayDataRevision& previousDefinitionRevision,
	const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
	const std::uint32_t serverTick)
{
	const std::string& failurePatternId =
		nextPatternId.empty() ? previousPatternId : nextPatternId;
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	const CGameplayCatalog* previousCatalog = previousPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(previousDefinitionRevision);
	const CGameplayCatalog* nextCatalog = nextPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(nextDefinitionRevision);
	if (nullptr == previousCatalog || nullptr == nextCatalog)
	{
		m_strStatus = "Boss stage transition pinned gameplay generation is missing";
		return false;
	}
	if (!Stage_BossPatternStageActions(
		boss, *previousCatalog, previousPatternId, previousActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick,
		stagedCombat, stagedGameplayPhase, transaction) ||
		!Stage_BossPatternStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick,
			stagedCombat, stagedGameplayPhase, transaction))
	{
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	WORLD_DESTRUCTION_TRANSACTION worldTransaction{};
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> worldEvents;
	bool hasWorldTransaction = false;
	if (m_WorldDestructionRuntime.Is_Initialized() &&
		0u != boss.iNetEntityId && 0u != boss.iPatternSequence &&
		!boss.strPatternId.empty() && !boss.strPatternStageId.empty() &&
		!boss.strActionId.empty())
	{
		WORLD_DESTRUCTION_ACTION_TUPLE worldAction{};
		worldAction.strPatternId = boss.strPatternId;
		worldAction.strStageId = boss.strPatternStageId;
		worldAction.strActionId = boss.strActionId;
		worldAction.iStageIndex = boss.iPatternStageIndex;
		std::string worldStatus;
		const WORLD_DESTRUCTION_PREPARE_RESULT worldResult =
			m_WorldDestructionRuntime.Prepare_StageTrigger(
				worldAction, boss.iNetEntityId, boss.iPatternSequence,
				serverTick, worldTransaction, worldStatus);
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == worldResult)
		{
			std::vector<SERVER_COLLISION_STATE_CHANGE> collisionChanges;
			std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> navigationChanges;
			SERVER_COLLISION_STATE_STAGE collisionStage{};
			SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
			Build_WorldDestructionStateChanges(
				worldTransaction, collisionChanges, navigationChanges);
			if (!Build_WorldDestructionLiveEvents(
					worldTransaction, boss, worldEvents, worldStatus) ||
				!m_ServerCollisionSystem.Prepare_StateChanges(
					collisionChanges, collisionStage, worldStatus) ||
				!m_ServerNavigation.Prepare_ConditionChanges(
					navigationChanges, navigationStage, worldStatus))
			{
				m_strStatus = std::move(worldStatus);
				CValtanBrain::Fail_Mechanic(
					boss, failurePatternId,
					SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
					serverTick);
				return false;
			}
			hasWorldTransaction = true;
		}
		else if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE != worldResult)
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
				serverTick);
			return false;
		}
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage transition combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	/* EnterPatternStage reset the counter before this transaction. A non-empty
	next stage has now committed its ENTER actions atomically, which is wave zero
	for every scheduled volley owned by that stage. */
	boss.iAppliedPatternStageSpawnWaveCount =
		nextActionId.empty() ? 0u : 1u;
	if (hasWorldTransaction)
	{
		std::string worldStatus;
		if (!Commit_WorldDestructionTransaction(
			worldTransaction, worldEvents, serverTick, worldStatus))
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT,
				serverTick);
			return false;
		}
		if (!worldEvents.empty())
		{
			const std::uint64_t lastSequence =
				worldEvents.back().iEventSequence;
			m_iNextWorldDestructionEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
					0u : lastSequence + 1u;
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Stage_BossPatternStageActions(
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	SERVER_BOSS_COMBAT_STATE& stagedCombat,
	std::uint8_t& stagedGameplayPhase,
	SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
	const std::uint32_t spawnWaveOrdinal)
{
	if (patternId.empty() || actionId.empty())
		return true;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Boss stage action encounter is missing";
		return false;
	}
	const auto pattern = std::find_if(patterns->begin(), patterns->end(),
		[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
		{
			return candidate.strPatternId == patternId;
		});
	if (patterns->end() == pattern)
	{
		m_strStatus = "Boss stage action pattern is missing: " + patternId;
		return false;
	}
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&actionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{
			return candidate.strActionId == actionId;
		});
	if (pattern->Stages.end() == stage)
	{
		m_strStatus = "Boss stage action owner is missing: " + actionId;
		return false;
	}
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (action.eTrigger != trigger)
			continue;
		const bool isScheduledVolley =
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				action.eKind;
		if (spawnWaveOrdinal > 0u &&
			(!isScheduledVolley ||
			 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
			 spawnWaveOrdinal >= action.Volley.iSpawnCount))
		{
			continue;
		}
		if (0u != action.iDurationMs)
		{
			m_strStatus = "Unsupported boss stage action: " + action.strTargetId;
			return false;
		}
		switch (action.eKind)
		{
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG:
		{
			if (action.iValue > 1u)
			{
				m_strStatus = "Boss flag stage action value is invalid: " +
					action.strTargetId;
				return false;
			}
			SERVER_BOSS_COMBAT_FLAG flag{};
			if ("boss.flag.groggy" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::GROGGY;
			else if ("boss.flag.invulnerable" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::INVULNERABLE;
			else if ("boss.flag.counterable" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::COUNTERABLE;
			else
			{
				m_strStatus = "Unknown boss flag stage action: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_Flag(
				stagedCombat, flag, 0u != action.iValue);
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE:
			if ("boss.gauge.stagger" != action.strTargetId)
			{
				m_strStatus = "Unknown boss stagger gauge target: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_StaggerGauge(
				stagedCombat, action.iValue);
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD:
			if ("boss.gauge.shield" != action.strTargetId)
			{
				m_strStatus = "Unknown boss shield gauge target: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_Shield(
				stagedCombat, action.iValue);
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE:
		{
			const BOSS_RUNTIME_PROFILE* profile =
				catalog.Find_Boss(boss.strArchetypeId);
			const bool isValtan = "BOSS_VALTAN" == boss.strArchetypeId &&
				"ENCOUNTER_VALTAN" == boss.strEncounterId;
			if (nullptr == profile ||
				BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT !=
					profile->PhasePolicy.eKind ||
				"boss.phase.gameplay" != action.strTargetId ||
				action.iValue < 2u || action.iValue > 3u ||
				(isValtan &&
					("VALTAN_ARENA_BREAK_109" != patternId ||
					 "valtan.mechanic.arena-break-109.impact" != actionId ||
					 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
					 2u != action.iValue)) ||
				action.iValue < stagedGameplayPhase)
			{
				m_strStatus = "Boss gameplay phase stage action is invalid";
				return false;
			}
			SERVER_WORLD_ENTITY stagedPhaseBoss{};
			stagedPhaseBoss.iPhase = stagedGameplayPhase;
			stagedPhaseBoss.BossCombat = stagedCombat;
			(void)CBossCombatRuntime::Set_GameplayPhase(
				stagedPhaseBoss, static_cast<std::uint8_t>(action.iValue));
			stagedGameplayPhase = stagedPhaseBoss.iPhase;
			stagedCombat = std::move(stagedPhaseBoss.BossCombat);
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT:
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY:
		{
			const bool isTypedVolley =
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
					action.eKind;
			const BOSS_COMBAT_OBJECT_DEFINITION* definition =
				catalog.Find_BossCombatObject(action.strTargetId);
			if (nullptr == definition ||
				definition->strEncounterId != boss.strEncounterId ||
				definition->strOwnerPatternId != patternId ||
				definition->strOwnerStageActionId != actionId)
			{
				m_strStatus = "Boss combat object owner is invalid: " +
					action.strTargetId;
				return false;
			}
			if (isTypedVolley &&
				(BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER !=
					definition->eOriginPolicy ||
				 BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER !=
					action.Volley.ePolicy ||
				 action.Volley.iSpawnCount < 1u ||
				 spawnWaveOrdinal >= action.Volley.iSpawnCount ||
				 action.Volley.iMaximumTotalObjects > 64u))
			{
				m_strStatus = "Boss combat object volley policy is invalid: " +
					action.strTargetId;
				return false;
			}
			SERVER_COMBAT_OBJECT_LOCKED_TARGET lockedTarget{};
			const SERVER_COMBAT_OBJECT_LOCKED_TARGET* resolvedLockedTarget =
				nullptr;
			if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE ==
				definition->eOriginPolicy)
			{
				for (const auto& [playerId, player] : m_Players)
				{
					(void)playerId;
					if (player.iNetEntityId == boss.iPatternTargetEntityId)
					{
						if (0u != player.iCurrentHp && player.isCombatReady &&
							LostArk::Shared::PLAYER_ACTION_STATE::DEAD !=
								player.eAction &&
							LostArk::Shared::PLAYER_ACTION_STATE::FALLING !=
								player.eAction)
						{
							lockedTarget.iNetEntityId = player.iNetEntityId;
							lockedTarget.fPositionX = player.fPositionX;
							lockedTarget.fPositionY = player.fPositionY;
							lockedTarget.fPositionZ = player.fPositionZ;
							resolvedLockedTarget = &lockedTarget;
						}
						break;
					}
				}
				if (nullptr == resolvedLockedTarget &&
					boss.bHasPatternTargetLastPosition &&
					LostArk::Shared::INVALID_NET_ENTITY_ID !=
						boss.iPatternTargetEntityId)
				{
					lockedTarget.iNetEntityId = boss.iPatternTargetEntityId;
					lockedTarget.fPositionX =
						boss.fPatternTargetLastPositionX;
					lockedTarget.fPositionY =
						boss.fPatternTargetLastPositionY;
					lockedTarget.fPositionZ =
						boss.fPatternTargetLastPositionZ;
					resolvedLockedTarget = &lockedTarget;
				}
				/* A pattern that never acquired a valid target still enters without
				creating an object. A vanished locked target uses the cached pose. */
				if (nullptr == resolvedLockedTarget)
				{
					break;
				}
			}
			if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
				definition->eOriginPolicy)
			{
				if (!isTypedVolley && action.iValue > 1u)
				{
					m_strStatus =
						"Multi-object per-player spawn requires typed volley layout";
					return false;
				}
				/* The volley is dealt once: every player alive at this edge gets
				one object locked to where they stand, and the whole set shares the
				staging transaction so a single failure drops all of it. */
				std::vector<const SERVER_PLAYER*> aliveTargets;
				for (const auto& [volleyPlayerId, volleyPlayer] : m_Players)
				{
					(void)volleyPlayerId;
					if (0u == volleyPlayer.iCurrentHp ||
						!volleyPlayer.isCombatReady ||
						LostArk::Shared::PLAYER_ACTION_STATE::DEAD ==
							volleyPlayer.eAction ||
						LostArk::Shared::PLAYER_ACTION_STATE::FALLING ==
							volleyPlayer.eAction)
					{
						continue;
					}
					aliveTargets.push_back(&volleyPlayer);
				}
				const std::uint32_t countPerTarget = isTypedVolley ?
					action.Volley.iCountPerResolvedTarget : action.iValue;
				if (0u == countPerTarget || countPerTarget > 8u ||
					(isTypedVolley && action.iValue != countPerTarget))
				{
					m_strStatus =
						"Boss combat object volley count is out of range";
					return false;
				}
				const std::uint32_t maximumTotal = isTypedVolley ?
					action.Volley.iMaximumTotalObjects : 32u;
				const std::uint64_t requestedTotal =
					static_cast<std::uint64_t>(aliveTargets.size()) * countPerTarget +
					(isTypedVolley ? action.Volley.iArenaRandomCount : 0u);
				if (maximumTotal < countPerTarget ||
					requestedTotal > maximumTotal || requestedTotal > 64u)
				{
					m_strStatus = "Boss combat object volley total is out of range";
					return false;
				}
				struct VOLLEY_POINT final
				{
					float fX = 0.f;
					float fZ = 0.f;
				};
				std::vector<VOLLEY_POINT> resolvedPoints;
				resolvedPoints.reserve(static_cast<std::size_t>(requestedTotal));
				if (isTypedVolley && !m_ServerNavigation.Is_Loaded())
				{
					m_strStatus =
						"Boss combat object volley navigation is unavailable";
					return false;
				}
				if (isTypedVolley)
				{
					for (const SERVER_PLAYER* volleyPlayer : aliveTargets)
					{
						const std::size_t targetPointBegin = resolvedPoints.size();
						for (std::uint32_t ordinal = 0u;
							ordinal < countPerTarget; ++ordinal)
						{
							float x = volleyPlayer->fPositionX;
							float z = volleyPlayer->fPositionZ;
							if (BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
								action.Volley.eLayout)
							{
								const float degrees =
									action.Volley.fStartAngleDegrees +
									action.Volley.fAngleStepDegrees *
										static_cast<float>(ordinal);
								const float radians = degrees * DEGREES_TO_RADIANS;
								x += std::sin(radians) * action.Volley.fRadiusM;
								z += std::cos(radians) * action.Volley.fRadiusM;
							}
							if (!std::isfinite(x) || !std::isfinite(z) ||
								!m_ServerNavigation.Is_PointWalkableExact(x, z))
							{
								m_strStatus =
									"Boss combat object volley leaves navigable arena";
								return false;
							}
							resolvedPoints.push_back({ x, z });
						}
						/* Layout belongs to one resolved target. Players may legitimately
						stack, so overlap admission compares only the ordinals dealt around
						that same target. */
						if (!action.Volley.bAllowOverlap)
						{
							const float minimumSpacing =
								Resolve_VolleyMinimumSpacing(*definition);
							const float minimumSpacingSquared =
								minimumSpacing * minimumSpacing;
							for (std::size_t left = targetPointBegin;
								left < resolvedPoints.size(); ++left)
							{
								for (std::size_t right = left + 1u;
								right < resolvedPoints.size(); ++right)
								{
									const float deltaX = resolvedPoints[left].fX -
										resolvedPoints[right].fX;
									const float deltaZ = resolvedPoints[left].fZ -
										resolvedPoints[right].fZ;
									if (deltaX * deltaX + deltaZ * deltaZ +
										VOLLEY_SPACING_EPSILON < minimumSpacingSquared)
									{
										m_strStatus =
											"Boss combat object volley spacing is invalid";
										return false;
									}
								}
							}
						}
					}
				}
				std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
				if (isTypedVolley &&
					!Resolve_ArenaRandomVolleyOrigins(
						boss, action, *definition, spawnWaveOrdinal,
						arenaOrigins))
				{
					return false;
				}
				for (const SERVER_PLAYER* volleyPlayer : aliveTargets)
				{
					SERVER_COMBAT_OBJECT_LOCKED_TARGET volleyTarget{};
					volleyTarget.iNetEntityId = volleyPlayer->iNetEntityId;
					volleyTarget.fPositionX = volleyPlayer->fPositionX;
					volleyTarget.fPositionY = volleyPlayer->fPositionY;
					volleyTarget.fPositionZ = volleyPlayer->fPositionZ;
					volleyTarget.bTrackUntilFirstPulse = false;
					if (!m_CombatObjectRuntime.Stage_BossCombatObject(
						combatObjectTransaction, boss, &volleyTarget, *definition,
						isTypedVolley ? &action.Volley : nullptr,
						catalog, countPerTarget, serverTick, m_strStatus))
					{
						return false;
					}
				}
				for (const SERVER_COMBAT_OBJECT_LOCKED_TARGET& arenaOrigin :
					arenaOrigins)
				{
					if (!m_CombatObjectRuntime.Stage_BossCombatObject(
						combatObjectTransaction, boss, &arenaOrigin, *definition,
						nullptr, catalog, 1u, serverTick, m_strStatus))
					{
						return false;
					}
				}
				/* An empty arena spawns nothing rather than falling back to the
				boss position. The authored arena supplement remains independent of
				the player count. */
				break;
			}
			if (!m_CombatObjectRuntime.Stage_BossCombatObject(
				combatObjectTransaction, boss, resolvedLockedTarget, *definition,
				nullptr, catalog, action.iValue, serverTick, m_strStatus))
			{
				return false;
			}
			break;
		}
		default:
			m_strStatus = "Unsupported boss stage action kind";
			return false;
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Drain_BossCombatEvents()
{
	using namespace LostArk::Shared;
	for (SERVER_WORLD_ENTITY& boss : m_WorldEntities)
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind)
			continue;
		auto& pending = boss.BossCombat.PendingPartBreakEdges;
		while (!pending.empty() &&
			m_TickBossCombatEvents.size() < MAX_BOSS_COMBAT_EVENTS)
		{
			const SERVER_BOSS_PART_BREAK_EDGE edge = pending.front();
			BOSS_COMBAT_EVENT event{};
			event.iEventSequence = m_iNextBossCombatEventSequence;
			event.iEventTick = edge.iServerTick;
			event.iBossNetEntityId = boss.iNetEntityId;
			event.eKind = BOSS_COMBAT_EVENT_KIND::PART_BROKEN;
			event.iPartMask = edge.iPartMask;
			m_TickBossCombatEvents.push_back(event);
			pending.erase(pending.begin());
			m_iNextBossCombatEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() ==
					m_iNextBossCombatEventSequence ?
				1u : m_iNextBossCombatEventSequence + 1u;
		}
	}
}

bool LostArk::Server::CGameRoom::Commit_DueEncounterProps(
	const std::uint32_t serverTick)
{
	if (!m_EncounterPropRuntime.Is_Initialized() || 0u == serverTick)
		return true;
	bool broadcast = false;
	std::string status;
	if (0u != m_iPillarAuditionBreakTick &&
		Has_ReachedServerTick(serverTick, m_iPillarAuditionBreakTick))
	{
		m_iPillarAuditionBreakTick = 0u;
		ENCOUNTER_PROP_TRANSACTION breakTransaction{};
		if (ENCOUNTER_PROP_PREPARE_RESULT::READY ==
			m_EncounterPropRuntime.Prepare_Break(
				m_EncounterPropRuntime.Get_OccurrenceSequence(), serverTick,
				breakTransaction, status) &&
			m_EncounterPropRuntime.Commit(breakTransaction, status))
		{
			broadcast = true;
		}
	}
	ENCOUNTER_PROP_TRANSACTION transaction{};
	if (ENCOUNTER_PROP_PREPARE_RESULT::READY ==
		m_EncounterPropRuntime.Prepare_DueRemoval(
			serverTick, PILLAR_BREAKING_TICKS, transaction, status) &&
		m_EncounterPropRuntime.Commit(transaction, status))
	{
		broadcast = true;
	}
	if (broadcast)
		Broadcast_EncounterPropSync();
	return true;
}

bool LostArk::Server::CGameRoom::Send_EncounterPropSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	/* A late joiner is told the live slot states and nothing else. The raise and
	   shatter one-shots already happened for the players who were here. */
	if (!m_EncounterPropRuntime.Is_Initialized() || nullptr == session)
		return true;
	S2C_ENCOUNTER_PROP_SYNC message{};
	message.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
	for (const ENCOUNTER_PROP_SLOT_STATE& slot :
		m_EncounterPropRuntime.Get_SlotStates())
	{
		ENCOUNTER_PROP_SLOT_WIRE wire{};
		wire.strSlotId = slot.strSlotId;
		wire.eState = slot.eState;
		wire.iStateVersion = slot.iStateVersion;
		wire.iStateStartTick = slot.iStateStartTick;
		wire.iOccurrenceSequence = slot.iOccurrenceSequence;
		message.Slots.push_back(std::move(wire));
	}
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	return session->Send_Frame(
		PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_EncounterPropSync()
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_EncounterPropSync(session))
			session->Request_Close();
	}
}

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
	const auto append =
		[&stagedPairs, epoch](const WORLD_DESTRUCTION_TRANSACTION& transaction)
		{
			if (transaction.iEncounterEpoch != epoch ||
				transaction.BindingApplications.size() !=
					transaction.Transitions.size())
			{
				return false;
			}
			for (std::size_t index = 0u;
				index < transaction.Transitions.size(); ++index)
			{
				stagedPairs.push_back({
					transaction.BindingApplications[index],
					transaction.Transitions[index] });
			}
			return true;
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
	if (m_ServerNavigation.Is_Loaded() &&
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
	staged.iCurrentHp = profile.iMaxHp;
	staged.iMaximumHp = profile.iMaxHp;
	staged.iAttackPower = profile.iAttackPower;
	staged.iDefense = profile.iDefense;
	staged.fCollisionRadius = profile.fCollisionRadius;
	staged.fEngageDistance = profile.fEngageRange;
	staged.fMoveSpeed = profile.fMoveSpeed;
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

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
}

void LostArk::Server::CGameRoom::Refresh_PlayerBlockingBodies()
{
	std::vector<SERVER_BLOCKING_BODY> bodies;
	bodies.reserve(m_WorldEntities.size());
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
			0u == entity.iCurrentHp)
		{
			continue;
		}
		/* Same body the skill hit test uses: monsters carry their profile
		radius, the boss reads its profile. */
		float radius = entity.fCollisionRadius;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			if (const BOSS_RUNTIME_PROFILE* bossProfile =
				m_GameplayCatalog.Find_Boss(entity.strArchetypeId))
			{
				radius = bossProfile->fCollisionRadius;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind)
		{
			continue;
		}
		if (radius <= 0.f)
			continue;
		bodies.push_back(SERVER_BLOCKING_BODY{
			entity.fPositionX, entity.fPositionZ, radius });
	}
	m_ServerCollisionSystem.Set_BlockingBodies(std::move(bodies));
}

/* Owns the whole falling life cycle of one player inside one tick: it starts
a fall when the authored ground under the player is gone, advances a running
fall, and turns it into the ordinary death the revive path already
understands. Returning true is what keeps trigger motion, skills and movement
from running at all this tick. */
bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		if (!std::isfinite(player.fPositionY) || sinceDeadline >= 0)
		{
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
		}
		return true;
	}
	if (!m_ServerNavigation.Is_Loaded() ||
		0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		!m_ServerNavigation.Is_PointInVoidRegion(
			player.fPositionX, player.fPositionZ))
	{
		return false;
	}

	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	const std::uint32_t deadline = player.iActionStartTick + FALL_DEATH_TICKS;
	player.iFallDeathTick = 0u == deadline ? 1u : deadline;
	player.fFallVelocityY = 0.f;
	/* Everything the fall interrupts is cleared here instead of inside each
	system, so no half-finished action can resume when the body lands dead. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
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
	/* Every boss and monster gate already refuses a player that is not combat
	ready, so this one flag removes the falling body from acquisition and from
	area damage without editing four separate target filters. */
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	/* The edge that opens the hole is also the first tick of the descent, so
	the body integrates here instead of hanging one tick at the old height and
	broadcasting a FALLING snapshot that has not moved. The deadline was just
	set a full FALL_DEATH_TICKS away, so it cannot be due on this tick. */
	player.fFallVelocityY -=
		FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
	player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
	return true;
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
		{
			const bool driverMayPlay =
				player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId &&
				VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
					m_ValtanTimelineAudition.ePhase;
			if (!driverMayPlay)
			{
				m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
				Freeze_TimelineAuditionPlayer(player);
				continue;
			}
		}
#endif
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			continue;
		}
		Advance_PlayerKnockback(player, fixedDeltaSeconds);
		if (LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction &&
			static_cast<std::int32_t>(
				updateTick - player.iKnockdownEndTick) >= 0)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.iKnockdownEndTick = 0u;
			player.iHitReactionGraceEndTick =
				updateTick + PLAYER_HIT_REACTION_GRACE_TICKS;
			player.PendingCommand.Clear();
		}
		/* The Esther call is a fixed-length lock, not a balance skill: the
		roster owns the summon, this block only releases the caster once the
		call clip has run out. Signed difference keeps ordering across a
		wrapped tick counter. */
		if (LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST ==
				player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + ESTHER_CAST_TICKS)) >= 0)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.PendingCommand.Clear();
		}
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			&m_ServerCollisionSystem,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction &&
			PLAYER_PENDING_COMMAND_KIND::NONE != player.PendingCommand.eKind)
		{
			Commit_PendingPlayerCommand(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		if (!player.hasMoveGoal)
			continue;
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				PLAYER_TURN_DEGREES_PER_SECOND * fixedDeltaSeconds;
			if (distance <= DIRECT_BEARING_DISTANCE ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			float stepX = deltaX * moveRatio;
			float stepZ = deltaZ * moveRatio;
			if (player.fYawDegrees != desiredYaw)
			{
				const float headingRadians =
					player.fYawDegrees / RADIANS_TO_DEGREES;
				const float headingStepX =
					std::sin(headingRadians) * moveDistance;
				const float headingStepZ =
					std::cos(headingRadians) * moveDistance;
				SERVER_NAV_POINT walkable{};
				if (!m_ServerNavigation.Is_Loaded() ||
					m_ServerNavigation.Sample_Position(
						player.fPositionX + headingStepX,
						player.fPositionZ + headingStepZ,
						walkable))
				{
					stepX = headingStepX;
					stepZ = headingStepZ;
				}
				else
				{
					player.fYawDegrees = desiredYaw;
				}
			}
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
		}
		/* A smoothed path can skip many authored cells. Never interpolate Y toward
		the distant waypoint: doing so raises the player while XZ is still on the
		lower deck and lets a later height check see an already-raised player.
		Resolve both XZ positions against navigation and take only its ground Y. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT proposedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				proposedX,
				proposedZ,
				proposedGround))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			proposedY = proposedGround.y;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		/* Body collision may slide XZ away from the point checked above. Validate
		the final slide destination too and ground it before committing any
		authoritative coordinate. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT resolvedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				resolvedX,
				resolvedZ,
				resolvedGround))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			resolvedY = resolvedGround.y;
		}
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (wasBlocked)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

void LostArk::Server::CGameRoom::Advance_PlayerKnockback(
	SERVER_PLAYER& player, const float fixedDeltaSeconds)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		player.fKnockbackRemainingSeconds <= 0.f)
	{
		return;
	}
	if (0u == player.iCurrentHp ||
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	const float step = (std::min)(
		fixedDeltaSeconds, player.fKnockbackRemainingSeconds);
	const float desiredX = player.fPositionX +
		player.fKnockbackDirectionX * player.fKnockbackSpeed * step;
	const float desiredZ = player.fPositionZ +
		player.fKnockbackDirectionZ * player.fKnockbackSpeed * step;
	SERVER_NAV_POINT reachable{ desiredX, player.fPositionY, desiredZ };
	bool wasClamped = false;
	if (m_ServerNavigation.Is_Loaded())
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			desiredX,
			desiredZ,
			reachable,
			wasClamped);
	}
	float resolvedX = player.fPositionX;
	float resolvedY = player.fPositionY;
	float resolvedZ = player.fPositionZ;
	bool wasBlocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(
		player,
		reachable.x,
		reachable.y,
		reachable.z,
		resolvedX,
		resolvedY,
		resolvedZ,
		wasBlocked))
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	player.fPositionX = resolvedX;
	player.fPositionY = resolvedY;
	player.fPositionZ = resolvedZ;
	player.fKnockbackRemainingSeconds = (wasClamped || wasBlocked) ?
		0.f : player.fKnockbackRemainingSeconds - step;
	if (player.fKnockbackRemainingSeconds <= 0.f)
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
	}
}

void LostArk::Server::CGameRoom::Update_WorldEntities(
	const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	Update_PendingEstherSummons(fixedDeltaSeconds);
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon)
		{
			/* The clip carries its own entrance and exit; the room only clocks
			the strike so the sweep below despawns it the moment it ends. */
			entity.fActionElapsedSeconds += fixedDeltaSeconds;
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
				m_isReady = false;
				return;
			}
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
			bool updateValtanBrain = true;
#ifdef _DEBUG
			updateValtanBrain = Prepare_ValtanTimelineRowBeforeBrain(
				entity, updateTick);
#endif
			if (updateValtanBrain)
			{
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
				m_ValtanBrain.Update(
					entity,
					m_Players,
					*occurrenceCatalog,
					m_ServerNavigation,
					fixedDeltaSeconds,
					updateTick,
					coverCircles,
					m_TickDamageEvents,
					&m_GameplayCatalog.Active(),
					m_GameplayCatalog.Get_ActiveGenerationEpoch());
				const VALTAN_DECISION_TRACE* latestTrace =
					m_ValtanBrain.Get_LatestDecisionTrace();
				if (nullptr != latestTrace &&
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
			const bool stageChanged =
				previousPatternSequence != entity.iPatternSequence ||
				previousStageIndex != entity.iPatternStageIndex ||
				previousActionStartTick != entity.iActionStartTick ||
				previousPatternId != entity.strPatternId ||
				previousStageId != entity.strPatternStageId ||
				previousActionId != entity.strActionId;
			const bool stageIdentityChanged =
				previousPatternSequence != entity.iPatternSequence ||
				previousStageIndex != entity.iPatternStageIndex ||
				previousPatternId != entity.strPatternId ||
				previousActionId != entity.strActionId;
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
				entity = bossBeforeBrain;
				entity.MechanicOccurrences = std::move(mechanicOccurrences);
				entity.PendingPatternIds = std::move(pendingPatternIds);
				entity.TriggeredPatternIds = std::move(triggeredPatternIds);
				entity.bMechanicLedgerRequiresReset = mechanicResetRequired;
				entity.iLastEvaluatedHealthBar = lastEvaluatedHealthBar;
				m_isReady = false;
				return;
			}
			if (!stageChanged && updateValtanBrain &&
				!Apply_BossPatternScheduledSpawnWave(entity, updateTick))
			{
				m_isReady = false;
				return;
			}
			if (stageChanged)
				(void)Apply_EncounterPropStageEntry(entity, updateTick);
			if (stageChanged && !Apply_WorldDestructionStageEntry(
				entity, updateTick))
			{
				m_isReady = false;
				return;
			}
			/* A charge owns one swept impact receiver transaction below. Letting the
			   stationary body-contact pass run first can break every overlapping wall
			   box before the exact receiver chooses its single mutation. */
			if (!entity.bPatternChargeImpact &&
				!Apply_WorldDestructionBodyContact(
				entity, contactStartX, contactStartY, contactStartZ,
				updateTick))
			{
				m_isReady = false;
				return;
			}
			/* A damage pulse is evaluated by the Brain at the same fixed tick as
			   the axe proxy. Only stages compiled in the wall-contact allowlist set
			   bPatternWallContact, so roars, waves and magic never reach here. */
			if (entity.bPatternWallContact &&
				entity.iAppliedPatternHitCount > previousAppliedPatternHitCount &&
				!Apply_WorldDestructionPatternHitContact(entity, updateTick))
			{
				m_isReady = false;
				return;
			}
			float proposedX = 0.f;
			float proposedZ = 0.f;
			if (m_ValtanBrain.Try_BuildStageMotion(
				entity, fixedDeltaSeconds, proposedX, proposedZ))
			{
				const auto Take_MotionStep =
					[this, &entity](const float targetX, const float targetZ)
					{
						Resolve_NavigableStep(
							m_ServerNavigation,
							entity.fPositionX, entity.fPositionZ,
							targetX, targetZ,
							entity.fPositionX, entity.fPositionZ);
					};
				/* Only a charge may end its stage on a wall. Ordinary stages now
				carry the small travel their clip bakes, and sweeping those against
				impact receivers would let a recovery shuffle smash a wall and then
				fail the impact transition the stage never declared. */
				SERVER_BOSS_RECEIVER_HIT hit{};
				if (entity.bPatternChargeImpact &&
					m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(
						entity.fPositionX, entity.fPositionY, entity.fPositionZ,
						proposedX, entity.fPositionY, proposedZ,
						entity.fCollisionRadius, hit))
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
					bool triggered = false;
					if (!Apply_WorldDestructionImpact(
						entity, hit.strReceiverPlacementId, updateTick, triggered))
					{
						m_isReady = false;
						return;
					}
					/* Geometry always stops the body, but only an exact, newly applied
					   destruction binding owns WALL_CONTACT. An unbound or consumed
					   receiver must not manufacture a GROGGY transition. */
					if (!triggered)
					{
						entity.fPatternForcedMotionSpeed = 0.f;
					}
					else
					{
						const SERVER_WORLD_ENTITY bossBeforeImpactTransition = entity;
						const std::string impactPreviousPatternId = entity.strPatternId;
						const std::string impactPreviousActionId = entity.strActionId;
						const CGameplayCatalog* impactCatalog =
							m_GameplayCatalog.Resolve(entity.PinnedDefinitionRevision);
						if (nullptr == impactCatalog ||
							!m_ValtanBrain.Complete_ImpactStage(
								entity, *impactCatalog, updateTick) ||
							!Apply_BossPatternStageTransition(
								entity, impactPreviousPatternId,
								impactPreviousActionId, entity.strPatternId,
								entity.strActionId,
								bossBeforeImpactTransition.PinnedDefinitionRevision,
								entity.PinnedDefinitionRevision, updateTick) ||
							!Apply_WorldDestructionStageEntry(entity, updateTick))
						{
							auto mechanicOccurrences =
								std::move(entity.MechanicOccurrences);
							const bool mechanicResetRequired =
								entity.bMechanicLedgerRequiresReset;
							entity = bossBeforeImpactTransition;
							entity.MechanicOccurrences =
								std::move(mechanicOccurrences);
							entity.bMechanicLedgerRequiresReset =
								mechanicResetRequired;
							m_strStatus =
								"Valtan impact stage transition failed";
							m_isReady = false;
							return;
						}
						/* Complete_ImpactStage resolves the authored WALL_CONTACT branch;
						   GameRoom only owns collision/world-destruction transaction order. */
					}
				}
				else
				{
					Take_MotionStep(proposedX, proposedZ);
				}
			}
#ifdef _DEBUG
			if (updateValtanBrain)
				Restore_ValtanTimelineRowAfterBrain(entity, updateTick);
#endif
			if (entity.strPatternId.empty() &&
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
				entity, m_ServerNavigation, fixedDeltaSeconds))
			{
				continue;
			}
			/* The brain reads m_WorldEntities to push this monster out of a body
			it overlaps. It only moves the entity handed to it, so passing the
			list this loop is walking neither spawns, despawns nor reorders it. */
			m_MonsterBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
		}
	}

	for (auto iter = m_WorldEntities.begin(); iter != m_WorldEntities.end();)
	{
		const bool shouldDespawn =
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
		Broadcast_WorldEntityDespawned(iter->iNetEntityId);
		iter = m_WorldEntities.erase(iter);
	}
#ifdef _DEBUG
	(void)Refresh_ValtanPatternIdAuditionState();
#endif
}
