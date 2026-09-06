#include "GameRoom.h"

#include "ClientSession.h"

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
	constexpr const char* RAID_CLEAR_TEST_MODE_ENV =
		"LOSTARK_RAID_CLEAR_TEST_MODE";

	bool Is_RaidClearTestModeEnabled()
	{
		char* value = nullptr;
		size_t valueLength = 0u;
		if (0 != _dupenv_s(&value, &valueLength, RAID_CLEAR_TEST_MODE_ENV))
			return false;
		const bool enabled = nullptr != value && 2u == valueLength &&
			'1' == value[0];
		std::free(value);
		return enabled;
	}

	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}

	bool Is_StablePatternFlowId(const std::string_view value)
	{
		return !value.empty() &&
			value.size() <= MAX_STABLE_NETWORK_ID_BYTES &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' ||
						character == '-' || character == '.';
				});
	}

	bool Is_PatternFlowRevision(const std::string_view value)
	{
		return VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES == value.size() &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return (character >= '0' && character <= '9') ||
						(character >= 'a' && character <= 'f');
				});
	}

	std::string Build_PatternFlowStartRequestIdentity(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request)
	{
		std::string identity =
			Format_GameplayDataRevision(request.ExpectedDefinitionRevision) + "|" +
			request.strBossPlacementId + "|" +
			request.strFlowId + "|" + request.strFlowRevision + "|" +
			request.strStartSlotId + "|" +
			std::to_string(request.iInterStepPursuitMs);
		for (const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot : request.Slots)
		{
			identity += "|" + slot.strSlotId + "=" + slot.strPatternId;
		}
		return identity;
	}

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

	bool Is_SameDestructionTransitionTarget(
		const WORLD_DESTRUCTION_STATE_TRANSITION& left,
		const WORLD_DESTRUCTION_STATE_TRANSITION& right)
	{
		return left.strGroupId == right.strGroupId &&
			left.strMutationId == right.strMutationId &&
			left.ePreviousState == right.ePreviousState &&
			left.eNextState == right.eNextState &&
			left.eFinalState == right.eFinalState &&
			left.iPreviousStateVersion == right.iPreviousStateVersion &&
			left.iNextStateVersion == right.iNextStateVersion &&
			left.iCommitTick == right.iCommitTick &&
			left.bApplyPersistentMutation ==
				right.bApplyPersistentMutation &&
			left.MemberPlacementIds == right.MemberPlacementIds &&
			left.strCollisionStateId == right.strCollisionStateId &&
			left.strNavigationStateId == right.strNavigationStateId;
	}

	template<typename PAIR>
	bool Append_UniqueDestructionTransitions(
		const WORLD_DESTRUCTION_TRANSACTION& transaction,
		const std::uint32_t expectedEncounterEpoch,
		const std::uint32_t expectedRequestTick,
		std::vector<PAIR>& stagedPairs,
		std::map<std::string, std::size_t>& stagedIndexByGroupId)
	{
		if (transaction.iEncounterEpoch != expectedEncounterEpoch ||
			transaction.iRequestTick != expectedRequestTick ||
			transaction.BindingApplications.size() !=
				transaction.Transitions.size() ||
			transaction.Transitions.empty())
		{
			return false;
		}
		for (std::size_t index = 0u;
			index < transaction.Transitions.size(); ++index)
		{
			const WORLD_DESTRUCTION_STATE_TRANSITION& transition =
				transaction.Transitions[index];
			const auto existing = stagedIndexByGroupId.find(
				transition.strGroupId);
			if (stagedIndexByGroupId.end() != existing)
			{
				if (!Is_SameDestructionTransitionTarget(
					stagedPairs[existing->second].Transition, transition))
				{
					return false;
				}
				/* Contact bindings are staged first. A later stage binding may own
				   the same mutation, but one transition must retain exactly one
				   application, so keep that first valid pair. */
				continue;
			}
			stagedIndexByGroupId.emplace(
				transition.strGroupId, stagedPairs.size());
			stagedPairs.push_back({
				transaction.BindingApplications[index], transition });
		}
		return true;
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

	std::string_view To_RoomCommandEnqueueResultName(
		const ROOM_COMMAND_ENQUEUE_RESULT result)
	{
		switch (result)
		{
		case ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED:
			return "ACCEPTED";
		case ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT:
			return "DROPPED_BEST_EFFORT";
		case ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP:
			return "DEDUPLICATED_CLEANUP";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND:
			return "REJECTED_INVALID_COMMAND";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY:
			return "REJECTED_ROOM_NOT_READY";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_SEALED:
			return "REJECTED_ROOM_SEALED";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP:
			return "REJECTED_PENDING_CLEANUP";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY:
			return "REJECTED_RELIABLE_CAPACITY";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING:
			return "REJECTED_BINDING_MISSING";
		default:
			return "UNKNOWN";
		}
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
	static_assert(static_cast<std::uint32_t>(
		SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN));
	static_assert(
		(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::SHIELDED) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GROGGY) |
		 static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN)) ==
		BOSS_COMBAT_STATE_KNOWN_FLAG_MASK);

	bool Has_ReachedServerTick(
		const std::uint32_t currentTick,
		const std::uint32_t targetTick)
	{
		return currentTick == targetTick ||
			static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	void Clear_ValtanGhostRelocationState(SERVER_WORLD_ENTITY& boss)
	{
		/* A relocation can overlap a different typed invulnerability owner only
		after future catalog expansion. Never clear a flag unless this exact
		transaction recorded that it introduced it. */
		if (boss.bGhostRelocationOwnsHiddenFlag)
		{
			(void)CBossCombatRuntime::Set_Flag(
				boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, false);
		}
		if (boss.bGhostRelocationOwnsInvulnerableFlag)
		{
			(void)CBossCombatRuntime::Set_Flag(
				boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, false);
		}
		boss.bGhostRepositionPending = false;
		boss.iGhostReappearTick = 0u;
		boss.bGhostRelocationRetryPending = false;
		boss.iGhostRelocationRetryTick = 0u;
		boss.bGhostRelocationOwnsHiddenFlag = false;
		boss.bGhostRelocationOwnsInvulnerableFlag = false;
	}

	std::uint32_t DurationMillisecondsToServerTicks(
		const std::uint32_t durationMs)
	{
		return static_cast<std::uint32_t>((
			static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ + 999u) /
			1000u);
	}

	void Cancel_PlayerActionForPatternStatus(SERVER_PLAYER& player)
	{
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iActionStartTick = 0u;
		player.Clear_Attachment();
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
		player.fKnockbackDirectionX = 0.f;
		player.fKnockbackDirectionZ = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.fKnockbackRemainingSeconds = 0.f;
		player.iKnockdownEndTick = 0u;
		player.iHitReactionGraceEndTick = 0u;
		player.isCombatReady = false;
	}
#ifdef _DEBUG
	constexpr const char* VALTAN_ARENA_AUDITION_PLACEMENT_ID =
		"boss.valtan.center";
	constexpr const char* CHARACTER_SELECT_AUDITION_PLACEMENT_ID =
		"boss.valtan.character-select.lazy";
	constexpr std::array<const char*, 8u>
		CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS{
			"VALTAN_ARMOR_BREAK_OPENING",
			"VALTAN_ENTRANCE_WHIRLWIND",
			"VALTAN_ARENA_BREAK_109",
			"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
			"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
			"VALTAN_FOUR_PILLARS_105",
			"VALTAN_SIX_PIZZA_106",
			"VALTAN_GHOST_PORTAL_ONCE" };

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

	bool Is_ValtanOutcomeFollowupInFlight(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence,
		const LostArk::Shared::GameplayDataRevision& definitionRevision)
	{
		if (0u == rootPatternSequence || !definitionRevision.Is_Valid())
			return false;
		if (boss.PendingPatternFollowup.Is_Pending())
		{
			return boss.PendingPatternFollowup.iRootPatternSequence ==
					rootPatternSequence &&
				boss.PendingPatternFollowup.PinnedDefinitionRevision ==
					definitionRevision;
		}
		return boss.iPatternFollowupDepth > 0u &&
			!boss.strPatternId.empty() &&
			boss.iPatternFollowupRootSequence == rootPatternSequence &&
			boss.PinnedDefinitionRevision == definitionRevision;
	}

	bool Has_ValtanOutcomeGroupCompleted(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence,
		const LostArk::Shared::GameplayDataRevision& definitionRevision)
	{
		return 0u != rootPatternSequence && definitionRevision.Is_Valid() &&
			boss.strPatternId.empty() &&
			!boss.PendingPatternFollowup.Is_Pending() &&
			boss.PatternTerminalReceipt.iRootPatternSequence ==
				rootPatternSequence &&
			LostArk::Server::SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
				boss.PatternTerminalReceipt.eResult &&
			boss.PinnedDefinitionRevision == definitionRevision;
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
		"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK";
	constexpr const char* FINAL_ARENA_FLOOR_A_STAGE_ID = "IMPACT";
	constexpr const char* FINAL_ARENA_FLOOR_A_ACTION_ID =
		"valtan.mechanic.terrain-destruction-3.impact";
	constexpr std::uint32_t FINAL_ARENA_FLOOR_A_STAGE_INDEX = 3u;
	constexpr const char* FINAL_ARENA_FLOOR_B_PATTERN_ID =
		"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK";
	constexpr const char* FINAL_ARENA_FLOOR_B_STAGE_ID = "IMPACT";
	constexpr const char* FINAL_ARENA_FLOOR_B_ACTION_ID =
		"valtan.mechanic.terrain-destruction-9.impact";
	constexpr std::uint32_t FINAL_ARENA_FLOOR_B_STAGE_INDEX = 3u;

	bool Resolve_ValtanArenaPreset(
		const std::uint32_t rawPreset,
		LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& outState,
		const char*& outLabel)
	{
		using LostArk::Shared::VALTAN_ARENA_PRESET;
		using LostArk::Server::VALTAN_TIMELINE_ARENA_STATE;
		switch (static_cast<VALTAN_ARENA_PRESET>(rawPreset))
		{
		case VALTAN_ARENA_PRESET::FRESH:
			outState = VALTAN_TIMELINE_ARENA_STATE::FRESH;
			outLabel = "Fresh / all walls intact";
			return true;
		case VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE:
			outState = VALTAN_TIMELINE_ARENA_STATE::ALL_WALLS_GONE;
			outLabel = "Circle / all walls gone";
			return true;
		case VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_GONE;
			outLabel = "3 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR30_GONE;
			outLabel = "9 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE;
			outLabel = "3 and 9 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::END:
		default:
			return false;
		}
	}

	struct VALTAN_FIGHT_PAGE_POLICY final
	{
		const char* pTimelineRowId;
		std::uint8_t iInitialGameplayPhase;
		bool bPlayEntrance;
	};

	/* Each key is an authored mechanic row, not a mutable list ordinal. The
	boundary mechanic is allowed to cross normally; all earlier mechanics are
	installed as completed ledger entries by Start_ValtanFightPage. */
	constexpr std::array<VALTAN_FIGHT_PAGE_POLICY, 4u>
		VALTAN_FIGHT_PAGE_POLICIES{
			VALTAN_FIGHT_PAGE_POLICY{
				"valtan.timeline.160-entrance-whirlwind", 1u, true },
			VALTAN_FIGHT_PAGE_POLICY{
				"valtan.timeline.109-arena-break", 1u, false },
			VALTAN_FIGHT_PAGE_POLICY{
				"valtan.timeline.62-center-grab-counter", 2u, false },
			VALTAN_FIGHT_PAGE_POLICY{
				"valtan.timeline.14-ghost-transition", 2u, false } };

	const VALTAN_FIGHT_PAGE_POLICY* Find_ValtanFightPagePolicy(
		const std::string& rowId)
	{
		const auto found = std::find_if(
			VALTAN_FIGHT_PAGE_POLICIES.begin(),
			VALTAN_FIGHT_PAGE_POLICIES.end(),
			[&rowId](const VALTAN_FIGHT_PAGE_POLICY& policy)
			{
				return rowId == policy.pTimelineRowId;
			});
		return VALTAN_FIGHT_PAGE_POLICIES.end() == found ? nullptr : &*found;
	}
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
		player.Clear_Attachment();
		player.Clear_PatternBindStatus();
		player.Clear_SilenceStatus();
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
			std::scoped_lock lock{ m_CommandMutex };
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
	Refresh_PlayerBlockingBodies();
	Update_Players(fixedDeltaSeconds);
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
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
	for (const SERVER_INTERACT_PROMPT_EDGE& edge : promptEdges)
		Send_InteractPrompt(edge);
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

bool LostArk::Server::CGameRoom::Stage_PlayerEntry(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
	STAGED_PLAYER_ENTRY& staged,
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory)
{
	using namespace LostArk::Shared;
	outReason = SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED;
	status.clear();
	const auto reject = [&outReason, &status](
		const SESSION_DIAGNOSTIC_REASON reason, const char* detail)
	{
		outReason = reason;
		status = detail;
		return false;
	};
	const std::size_t offset = precedingEntries.size();
	if (!m_isReady || nullptr == session || session->Is_Closing() ||
		INVALID_SESSION_ID == session->Get_SessionId() ||
		!Is_Valid_EnterWorld(enterWorld) || enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(session->Get_SessionId()) ||
		m_Players.size() + offset >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		INVALID_PLAYER_ID == m_iNextPlayerId ||
		INVALID_NET_ENTITY_ID == m_iNextNetEntityId ||
		offset > (std::numeric_limits<PLAYER_ID>::max)() - m_iNextPlayerId ||
		offset > (std::numeric_limits<NET_ENTITY_ID>::max)() - m_iNextNetEntityId)
	{
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"player entry room/session/identity validation failed");
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = nullptr;
	if (!spawnPlacementOverrideId.empty())
	{
		/* Not restricted to PLAYER_SPAWN kind or exclusivity -- an override names
		one specific placement (e.g. a guide NPC) directly, and several returning
		players landing at the same NPC concurrently is fine (unlike normal
		PLAYER_SPAWN slots, which are one-player-at-a-time). */
		spawn = Find_Placement(spawnPlacementOverrideId);
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				"spawn placement override id does not exist in this world's bootstrap");
	}
	else
	{
		for (const auto& candidate : m_WorldBootstrap.Get_Placements())
		{
			if (!candidate.isEnabled || WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != candidate.eKind)
				continue;
			const bool occupied = std::any_of(m_Players.begin(), m_Players.end(),
				[&candidate](const auto& value)
				{ return value.second.strSpawnPlacementId == candidate.strPlacementId; });
			const bool reserved = std::any_of(precedingEntries.begin(), precedingEntries.end(),
				[&candidate](const auto& value)
				{ return value.Player.strSpawnPlacementId == candidate.strPlacementId; });
			if (!occupied && !reserved) { spawn = &candidate; break; }
		}
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				"target room has fewer free player spawns than the transfer batch");
	}
	STAGED_PLAYER_ENTRY candidate{};
	candidate.pSession = session;
	SERVER_PLAYER& player = candidate.Player;
	player.iSessionId = session->Get_SessionId();
	player.iPlayerId = m_iNextPlayerId + static_cast<PLAYER_ID>(offset);
	player.iNetEntityId = m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(offset);
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.fPositionY = spawn->fPositionY;
	if (!spawnPlacementOverrideId.empty())
	{
		/* An override names an NPC's own placement, not an authored player-standing
		spot -- its exact point is often flush against a wall or counter (the NPC's
		back), so landing there directly can navigation-project to the wrong side
		of that geometry. Stand where a player who walked up to talk to it would:
		NPC_APPROACH_OFFSET_M out along its own forward direction, facing back
		toward it (matches this codebase's yaw convention, forward = (sin, cos),
		e.g. MonsterBrain.cpp's own movement step). */
		constexpr float NPC_APPROACH_OFFSET_M = 2.5f;
		const float yawRadians = spawn->fYawDegrees * DEGREES_TO_RADIANS;
		player.fPositionX = spawn->fPositionX + std::sin(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fPositionZ = spawn->fPositionZ + std::cos(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fYawDegrees = std::fmod(spawn->fYawDegrees + 180.f, 360.f);
	}
	else
	{
		player.fPositionX = spawn->fPositionX;
		player.fPositionZ = spawn->fPositionZ;
		player.fYawDegrees = spawn->fYawDegrees;
	}
	const PLAYER_RUNTIME_PROFILE* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_PROFILE_MISSING,
			"selected character class has no runtime profile");
	player.eStance = profile->eDefaultStance;
	player.iCurrentHp = player.iMaximumHp = profile->iMaximumHp;
	player.iCurrentResource = player.iMaximumResource = profile->iMaximumResource;
	player.fMoveSpeed = profile->fMoveSpeed;
	player.iCurrentIdentity = player.iMaximumIdentity = profile->iMaximumIdentity;
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, projected))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED,
				"player spawn could not project onto Server navigation");
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}
	if (!carriedInventory.empty())
	{
		// A world transfer carrying the departing player's own live inventory
		// (e.g. Handle_ReturnToBern) replaces the default fresh-entry grant
		// entirely -- Valtan clear rewards must survive the trip back to Bern.
		player.Inventory = carriedInventory;
	}
	else
	{
		for (const char* potionId : { "POTION_HP_SMALL", "POTION_HP_MEDIUM", "POTION_HP_LARGE" })
		{
			const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(potionId);
			if (nullptr == definition) continue;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = potionId;
			item.iQuantity = (std::min)(500u, definition->iMaxStack);
			player.Inventory.push_back(std::move(item));
		}
	}
	staged = std::move(candidate);
	return true;
}

bool LostArk::Server::CGameRoom::Build_PlayerEntryFrames(
	STAGED_PLAYER_ENTRY& entry, const std::span<const STAGED_PLAYER_ENTRY> batch,
	std::string& status)
{
	using namespace LostArk::Shared;
	std::vector<PACKET_FRAME> frames;
	const auto append = [&frames, &status](const PACKET_TYPE type, const auto& message)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
		{
			status = "initial entry payload failed validation, packet=" +
				std::to_string(static_cast<std::uint16_t>(type));
			return false;
		}
		frames.push_back({ type, writer.Get_Buffer() });
		return true;
	};
	S2C_ENTER_ACCEPTED accepted{};
	accepted.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	accepted.eWorldId = m_eWorldId;
	accepted.iPlayerId = entry.Player.iPlayerId;
	accepted.iNetEntityId = entry.Player.iNetEntityId;
	accepted.ActiveGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(accepted.RequiredPinnedGameplayRevisions))
	{
		status = "initial entry pinned gameplay revisions failed validation";
		return false;
	}
	if (!append(PACKET_TYPE::S2C_ENTER_ACCEPTED, accepted)) return false;
	S2C_INVENTORY_SNAPSHOT inventory{};
	inventory.Items = entry.Player.Inventory;
	if (!append(PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, inventory)) return false;
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_WorldDestructionRuntime.Is_Initialized())
		{
			status = "initial world destruction runtime is not initialized";
			return false;
		}
		S2C_WORLD_DESTRUCTION_FULL_SYNC fullSync{};
		fullSync.strCombatRuntimeRevision = m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
		fullSync.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		fullSync.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
		for (const auto& state : m_WorldDestructionRuntime.Get_GroupStates())
			fullSync.GroupStates.push_back(To_NetworkDestructionState(state));
		fullSync.Diagnostics = Build_WorldDestructionDiagnostics();
		if (!append(PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, fullSync)) return false;
	}
	if (m_EncounterPropRuntime.Is_Initialized())
	{
		S2C_ENCOUNTER_PROP_SYNC props{};
		props.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
		props.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		props.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
		for (const auto& slot : m_EncounterPropRuntime.Get_SlotStates())
		{
			ENCOUNTER_PROP_SLOT_WIRE wire{};
			wire.strSlotId = slot.strSlotId;
			wire.eState = slot.eState;
			wire.iStateVersion = slot.iStateVersion;
			wire.iStateStartTick = slot.iStateStartTick;
			wire.iOccurrenceSequence = slot.iOccurrenceSequence;
			props.Slots.push_back(std::move(wire));
		}
		if (!append(PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, props)) return false;
	}
	std::unordered_set<NET_ENTITY_ID> admittedWorldEntityIds;
	for (const bool dependentPass : { false, true })
	{
		for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
		{
			const bool isDependent = INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId;
			if (isDependent != dependentPass)
				continue;
			if (!admittedWorldEntityIds.insert(entity.iNetEntityId).second)
			{
				status = "Initial world entity ID is duplicated";
				return false;
			}
			if (isDependent)
			{
				const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
					[&entity](const SERVER_WORLD_ENTITY& candidate)
					{ return candidate.iNetEntityId == entity.iOwnerBossNetEntityId; });
				if (!admittedWorldEntityIds.contains(entity.iOwnerBossNetEntityId) ||
					owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
					INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
					owner->strEncounterId != entity.strEncounterId ||
					owner->PinnedDefinitionRevision !=
						entity.PinnedDefinitionRevision)
				{
					status = "Initial dependent boss has no preceding primary owner";
					return false;
				}
			}
			std::vector<std::uint8_t> payload;
			if (!Build_WorldEntitySpawnedPayload(entity, payload))
			{
				status = "World entity spawn payload preflight failed: " + entity.strPlacementId;
				return false;
			}
			frames.push_back({ PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, std::move(payload) });
		}
	}
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> combatObjects;
	m_CombatObjectRuntime.Build_LiveSpawnMessages(0u == m_iServerTick ? 1u : m_iServerTick, combatObjects);
	for (const auto& object : combatObjects)
		if (!append(PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, object)) return false;
	const auto appendPlayer = [&append](const SERVER_PLAYER& player)
	{
		S2C_PLAYER_SPAWNED message{};
		message.iPlayerId = player.iPlayerId;
		message.iNetEntityId = player.iNetEntityId;
		message.eCharacterClass = player.eCharacterClass;
		message.strNickName = player.strNickName;
		message.fPositionX = player.fPositionX;
		message.fPositionY = player.fPositionY;
		message.fPositionZ = player.fPositionZ;
		message.fYawDegrees = player.fYawDegrees;
		return append(PACKET_TYPE::S2C_PLAYER_SPAWNED, message);
	};
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (!appendPlayer(player)) return false;
	}
	for (const auto& staged : batch)
		if (!appendPlayer(staged.Player)) return false;
	entry.Frames = std::move(frames);
	return true;
}

void LostArk::Server::CGameRoom::Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry)
{
	const SERVER_PLAYER& player = entry.Player;
	m_Sessions.insert_or_assign(player.iSessionId, entry.pSession);
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	entry.pSession->Bind_PlayerId(player.iPlayerId);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory)
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
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				WSAEINVAL,
				"ENTER_WORLD failed room/session/id validation");
		}
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		const std::size_t enabledPlayerSpawnCount =
			static_cast<std::size_t>(std::count_if(
				m_WorldBootstrap.Get_Placements().begin(),
				m_WorldBootstrap.Get_Placements().end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
				}));
		const std::string roomFullCounts =
			"activePlayers=" + std::to_string(m_Players.size()) +
			", registeredSessionsIncludingCandidate=" +
			std::to_string(m_Sessions.size()) +
			", candidateSessionId=" + std::to_string(sessionId) +
			", candidateRegistered=" +
			(m_Sessions.contains(sessionId) ? "true" : "false") +
			", enabledPlayerSpawns=" +
			std::to_string(enabledPlayerSpawnCount);
		/* Leave room for the terminal-action prefix so the copied close context
		   remains bounded to roughly one KiB. */
		constexpr std::size_t MAX_ROOM_FULL_CONTEXT_BYTES = 960u;
		const std::uint64_t observedUnixMilliseconds =
			Current_UnixMilliseconds();
		std::string roomFullContext = roomFullCounts + ", activeRoster=[";
		bool isFirstRosterEntry = true;
		for (const auto& [playerId, player] : m_Players)
		{
			const std::shared_ptr<CClientSession> incumbent =
				Find_Session(player.iSessionId);
			std::string peer = "unavailable";
			std::uint64_t lastInboundUnixMilliseconds = 0u;
			if (nullptr != incumbent)
			{
				const CLIENT_SESSION_PEER_ENDPOINT& endpoint =
					incumbent->Get_PeerEndpoint();
				peer = endpoint.strAddress + ':' +
					std::to_string(endpoint.iPort);
				lastInboundUnixMilliseconds =
					incumbent->Get_LastInboundUnixMilliseconds();
			}
			const std::uint64_t lastInboundAgeMilliseconds =
				0u != lastInboundUnixMilliseconds &&
				observedUnixMilliseconds >= lastInboundUnixMilliseconds ?
				observedUnixMilliseconds - lastInboundUnixMilliseconds : 0u;
			const std::string rosterEntry =
				(isFirstRosterEntry ? "" : ", ") +
				std::string{ "{sessionId=" } +
				std::to_string(player.iSessionId) +
				", playerId=" + std::to_string(playerId) +
				", spawn=" + player.strSpawnPlacementId +
				", peer=" + peer +
				", lastInboundUnixMs=" +
				std::to_string(lastInboundUnixMilliseconds) +
				", lastInboundAgeMs=" +
				std::to_string(lastInboundAgeMilliseconds) + '}';
			if (roomFullContext.size() + rosterEntry.size() + 1u >
				MAX_ROOM_FULL_CONTEXT_BYTES)
			{
				constexpr std::string_view TRUNCATED =
					", {truncated=true}]";
				roomFullContext.resize((std::min)(
					roomFullContext.size(),
					MAX_ROOM_FULL_CONTEXT_BYTES - TRUNCATED.size()));
				roomFullContext.append(TRUNCATED);
				break;
			}
			roomFullContext += rosterEntry;
			isFirstRosterEntry = false;
		}
		if (roomFullContext.empty() || ']' != roomFullContext.back())
			roomFullContext += ']';
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"typed ROOM_FULL rejection flushed before close; " +
					roomFullContext);
		}
		else
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"ROOM_FULL rejection could not be queued; " +
					roomFullContext);
		}
		return false;
	}
	STAGED_PLAYER_ENTRY entry{};
	SESSION_DIAGNOSTIC_REASON reason{};
	std::string status;
	if (!Stage_PlayerEntry(session, enterWorld, {}, entry, reason, status,
			spawnPlacementOverrideId, carriedInventory))
	{
		session->Request_Close(reason, WSAEINVAL, status);
		return false;
	}
	if (!Build_PlayerEntryFrames(entry, std::span<const STAGED_PLAYER_ENTRY>{ &entry, 1u }, status))
	{
		m_strStatus = status;
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, 0, status);
		return false;
	}
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare({ { session, entry.Frames } }, status))
	{
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_INITIAL_SYNC_ENQUEUE_FAILED, 0, status);
		return false;
	}
	Commit_PlayerEntry(entry);
	outbound.Commit();
	Broadcast_Spawned(entry.Player, sessionId);
	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId << ", PlayerId=" << entry.Player.iPlayerId
		<< ", Spawn=" << entry.Player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason, const bool publishDeparture)
{
	using namespace LostArk::Shared;

#ifdef _DEBUG
	if (sessionId == m_ValtanPatternIdAudition.iOwnerSessionId)
	{
		Cancel_ValtanNextPatternReservation("owner left the room");
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase)
		{
			if (SERVER_WORLD_ENTITY* boss =
				Find_AuditionBoss(m_ValtanPatternIdAudition.strBossPlacementId))
			{
				std::erase(boss->PendingPatternIds, m_ValtanPatternIdAudition.strPatternId);
				boss->bAutomaticPatternSequenceAuditionOverride = true;
				boss->bAutomaticPatternSequenceAuditionHold = true;
			}
			Cancel_ValtanPatternIdAudition("owner left before the occurrence started");
		}
		else
		{
			// A already running with other players may finish normally; its
			// departed owner can no longer append to that terminal anchor.
			m_ValtanPatternIdAudition.iOwnerSessionId = INVALID_SESSION_ID;
		}
	}
	m_ValtanNextPatternReceiptBySessionId.erase(sessionId);
	if (sessionId == m_ValtanPatternFlowAudition.iOwnerSessionId &&
		Is_ValtanPatternFlowRunning())
	{
		Abort_ValtanPatternFlowForOwner(
			sessionId, "Valtan pattern-flow owner left the room");
		(void)Flush_ValtanPatternFlowLifecycle();
	}
	if (sessionId == m_ValtanTimelineAudition.iOwnerSessionId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}
	if (sessionId == m_KoukuSaydonPatternAudition.iOwnerSessionId)
	{
		if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING ==
			m_KoukuSaydonPatternAudition.ePhase)
		{
			Clear_KoukuSaydonPatternAudition();
		}
		else
		{
			// A running Server occurrence may finish for the remaining room. The
			// departed tool owner no longer receives or extends its lifecycle.
			m_KoukuSaydonPatternAudition.iOwnerSessionId = INVALID_SESSION_ID;
		}
	}
	std::erase_if(m_PendingKoukuSaydonPatternAuditionLifecycle,
		[sessionId](const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& edge)
		{
			return edge.iSessionId == sessionId;
		});
	m_KoukuSaydonPatternAuditionReceiptBySessionId.erase(sessionId);
#endif
	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternIdAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowStartSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowControlSequenceBySessionId.erase(sessionId);
	m_PendingPartyTransferResults.erase(sessionId);
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
	m_PendingPartyInviteByTargetPlayerId.erase(playerId);
	std::erase_if(m_PendingPartyInviteByTargetPlayerId,
		[playerId](const auto& invite) { return invite.second == playerId; });
	Cancel_RaidEntryProposalsInvolving(playerId);
	Remove_FromParty(playerId);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	if (publishDeparture)
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

void LostArk::Server::CGameRoom::Close_SessionForBindingFailure(
	const SESSION_ID sessionId,
	const std::string_view packetName,
	const std::string_view validation)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	session->Request_Close(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		WSAENOTCONN,
		"packet=" + std::string{ packetName } + " validation=" +
			std::string{ validation });
}

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
	if (player.bPatternBound ||
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
	if (playerIter->second.bPatternBound ||
		playerIter->second.fKnockbackRemainingSeconds > 0.f ||
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
	/* Always revives at the arena's own authored center (the Valtan boss's own
	placement, "boss.valtan.center") instead of wherever the player died --
	a fixed, known-safe spot regardless of how the death happened (fall,
	pattern hit, etc.), rather than reviving in place and only recovering when
	that exact spot turned out unwalkable. isEnabled/eKind aren't checked here
	(unlike a player's own strSpawnPlacementId lookup) since this is one
	specific, verified placement id, not caller-influenced data. */
	constexpr const char* VALTAN_ARENA_CENTER_PLACEMENT_ID = "boss.valtan.center";
	const WORLD_BOOTSTRAP_PLACEMENT* arenaCenter =
		Find_Placement(VALTAN_ARENA_CENTER_PLACEMENT_ID);
	if (nullptr == arenaCenter)
		return;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(
			arenaCenter->fPositionX, arenaCenter->fPositionZ, projected))
		{
			return;
		}
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}
	else
	{
		player.fPositionX = arenaCenter->fPositionX;
		player.fPositionY = arenaCenter->fPositionY;
		player.fPositionZ = arenaCenter->fPositionZ;
	}
	player.fYawDegrees = arenaCenter->fYawDegrees;
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
#endif
}

void LostArk::Server::CGameRoom::Reset_PlayerForDebugTeleport(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
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
	result.fPositionX = ground.x;
	result.fPositionY = ground.y;
	result.fPositionZ = ground.z;
	player.LastDebugTeleportResult = result;
	return result;
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

bool LostArk::Server::CGameRoom::Grant_Item(
	SERVER_PLAYER& player,
	const std::string& itemId,
	const std::uint32_t quantity)
{
	using namespace LostArk::Shared;
	const SERVER_ITEM_DEFINITION* itemDefinition =
		m_ItemCatalog.Find_Item(itemId);
	// An unknown item ID is rejected rather than silently dropped or
	// substituted; the inventory is only ever a replace-in-full send, so a
	// no-op reply carries no result to give a caller.
	if (nullptr == itemDefinition)
		return false;

	const auto existing = std::find_if(
		player.Inventory.begin(), player.Inventory.end(),
		[&itemId](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == itemId;
		});
	if (existing == player.Inventory.end())
	{
		if (player.Inventory.size() >= MAX_INVENTORY_ITEMS)
			return false;
		INVENTORY_ITEM_SNAPSHOT item{};
		item.strItemId = itemId;
		item.iQuantity = (std::min)(quantity, itemDefinition->iMaxStack);
		player.Inventory.push_back(std::move(item));
	}
	else
	{
		const std::uint64_t stacked =
			static_cast<std::uint64_t>(existing->iQuantity) +
			static_cast<std::uint64_t>(quantity);
		existing->iQuantity = static_cast<std::uint32_t>(
			(std::min)(stacked, static_cast<std::uint64_t>(
				itemDefinition->iMaxStack)));
	}
	return true;
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
	if (!Grant_Item(player, request.strItemId, request.iQuantity))
		return;

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
	const std::uint32_t despawnTick =
		0u == m_iServerTick ? 1u : m_iServerTick;
	for (auto entity = m_WorldEntities.rbegin(); entity != m_WorldEntities.rend(); ++entity)
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity->eKind)
		{
			(void)Release_PlayerAttachments(
				entity->iNetEntityId, 0.f, 0u, false, 0u, despawnTick);
		}
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
		{
			Mark_RuntimeFailure("despawn-all.combat-object-lifecycle");
			return;
		}
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
	}
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

	const auto isAlreadyStaged = [this](SESSION_ID sid)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sid](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sid ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sid) != pending.PartyBatchSessionIds.end();
			});
	};
	if (isAlreadyStaged(player.iSessionId))
		return;

	/* Solo by default; becomes the whole party's member list only when the
	   confirming player is a partied leader (members.front(), the original
	   inviter -- see "Same-room party state" in GameRoom.h). A non-leader
	   member's solo confirm is rejected instead of splitting the party across
	   two rooms, so a formed party never breaks on a Valtan entry. */
	std::vector<PLAYER_ID> batchMemberIds{ playerIter->first };
	const auto partyIdIter = m_PartyIdByPlayerId.find(playerIter->first);
	if (partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter =
			m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() > 1)
		{
			if (membersIter->second.front() != playerIter->first)
			{
				Notify_PartyTransferFailure(sessionId, request.iRequestSequence,
					guideIter->eTargetWorldId, PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER);
				return;
			}
			batchMemberIds = membersIter->second;
		}
	}

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = guideIter->eTargetWorldId;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	for (const PLAYER_ID memberId : batchMemberIds)
	{
		const auto memberIter = m_Players.find(memberId);
		if (memberIter == m_Players.end() ||
			CHARACTER_CLASS_ID::END == memberIter->second.eCharacterClass ||
			memberIter->second.strNickName.empty() ||
			isAlreadyStaged(memberIter->second.iSessionId))
		{
			Notify_PartyTransferFailure(sessionId, request.iRequestSequence,
				guideIter->eTargetWorldId, PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE);
			return;
		}
		if (batchMemberIds.size() > 1u)
			transfer.PartyBatchSessionIds.push_back(memberIter->second.iSessionId);
	}
	m_PendingWorldTransfers.push_back(std::move(transfer));
}

void LostArk::Server::CGameRoom::Handle_ReturnToBern(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RETURN_TO_BERN& request)
{
	using namespace LostArk::Shared;
	// Same guide NPC placement Handle_ConfirmNpcEntry's own
	// VALTAN_ENTRY_GUIDE_NPCS[0] leads out from -- either would do (both lead to
	// the same target), this just needs to be a real BERN placement id.
	constexpr const char* BERN_RETURN_PLACEMENT_ID = "npc.bern.beda.guide";

	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return;
	if (!m_bValtanRaidCleared && !Is_RaidClearTestModeEnabled())
		return;

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

	// Solo only -- unlike Handle_ConfirmNpcEntry, returning is never batched
	// across a party. Each player presses their own button independently.
	const bool isAlreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId;
		});
	if (isAlreadyStaged)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = WORLD_ID::BERN;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	transfer.strSpawnPlacementOverrideId = BERN_RETURN_PLACEMENT_ID;
	// Carries Valtan clear rewards (and anything else still held) across the
	// trip -- without this, Stage_PlayerEntry's default fresh-entry grant would
	// silently reset the player back to just 3 starting potions.
	transfer.CarriedInventory = player.Inventory;
	m_PendingWorldTransfers.push_back(std::move(transfer));
}

void LostArk::Server::CGameRoom::Handle_PartyInvite(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID inviterId = sessionIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	const SERVER_PLAYER& inviter = inviterIter->second;

	const auto targetPlayerIdIter =
		m_PlayerIdByEntityId.find(request.iTargetNetEntityId);
	if (targetPlayerIdIter == m_PlayerIdByEntityId.end() ||
		targetPlayerIdIter->second == inviterId)
	{
		return;
	}
	const PLAYER_ID targetId = targetPlayerIdIter->second;
	const auto targetIter = m_Players.find(targetId);
	if (targetIter == m_Players.end())
		return;
	const SERVER_PLAYER& target = targetIter->second;

	const auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	const std::uint32_t inviterPartyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	const auto targetPartyIter = m_PartyIdByPlayerId.find(targetId);
	if (targetPartyIter != m_PartyIdByPlayerId.end())
	{
		// Already partied together, or target belongs to a different party --
		// merging two existing parties is not supported yet either way.
		return;
	}
	if (0u != inviterPartyId)
	{
		const auto membersIter = m_PartyMembersByPartyId.find(inviterPartyId);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() >= MAX_PARTY_MEMBERS)
		{
			return;
		}
	}

	// A new invite silently replaces whatever this target's last unanswered
	// invite was -- only one can ever be outstanding per target.
	m_PendingPartyInviteByTargetPlayerId[targetId] = inviterId;

	const std::shared_ptr<CClientSession> targetSession =
		Find_Session(target.iSessionId);
	if (nullptr == targetSession)
		return;
	S2C_PARTY_INVITE_RECEIVED message{};
	message.iFromNetEntityId = inviter.iNetEntityId;
	message.strFromNickname = inviter.strNickName;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!targetSession->Send_Frame(
			PACKET_TYPE::S2C_PARTY_INVITE_RECEIVED, writer.Get_Buffer()))
	{
		targetSession->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_PartyInviteRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request)
{
	using namespace LostArk::Shared;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto pendingIter =
		m_PendingPartyInviteByTargetPlayerId.find(responderId);
	if (pendingIter == m_PendingPartyInviteByTargetPlayerId.end())
		return;
	const PLAYER_ID inviterId = pendingIter->second;
	const auto inviterIter = m_Players.find(inviterId);
	if (inviterIter == m_Players.end())
		return;
	if (inviterIter->second.iNetEntityId != request.iFromNetEntityId)
		return;
	// A response to a replaced invite must not consume the current invite,
	// regardless of whether that stale response accepts or declines.
	m_PendingPartyInviteByTargetPlayerId.erase(pendingIter);
	if (!request.bAccepted)
		return;
	if (m_Players.find(responderId) == m_Players.end())
		return;
	// Re-check both invariants Handle_PartyInvite validated -- state may have
	// changed while this invite was outstanding.
	if (m_PartyIdByPlayerId.find(responderId) != m_PartyIdByPlayerId.end())
		return;

	auto inviterPartyIter = m_PartyIdByPlayerId.find(inviterId);
	std::uint32_t partyId = inviterPartyIter != m_PartyIdByPlayerId.end() ?
		inviterPartyIter->second : 0u;
	if (0u == partyId)
	{
		partyId = m_iNextPartyId++;
		m_PartyMembersByPartyId[partyId] = { inviterId };
		m_PartyIdByPlayerId[inviterId] = partyId;
	}
	else if (m_PartyMembersByPartyId[partyId].size() >= MAX_PARTY_MEMBERS)
	{
		return;
	}
	m_PartyMembersByPartyId[partyId].push_back(responderId);
	m_PartyIdByPlayerId[responderId] = partyId;

	Broadcast_PartyRoster(partyId);
}

void LostArk::Server::CGameRoom::Broadcast_PartyRoster(
	const std::uint32_t partyId)
{
	using namespace LostArk::Shared;

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;

	S2C_PARTY_ROSTER message{};
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		PARTY_ROSTER_MEMBER member{};
		member.iNetEntityId = playerIter->second.iNetEntityId;
		member.strNickname = playerIter->second.strNickName;
		member.eCharacterClass = playerIter->second.eCharacterClass;
		message.Members.push_back(std::move(member));
	}
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : membersIter->second)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_PARTY_ROSTER, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

std::uint32_t LostArk::Server::CGameRoom::Place_PartyForCutscene(
	const std::string& instanceId)
{
	using namespace LostArk::Shared;
	/* Only the pop-up book cutscene stages the party; every other sequence is
	   presentation the players watch from where they already stand. */
	if ("world.sequence.instance.original_kouku" != instanceId)
		return 0u;

	/* Authored in front of the boss at (-0.29, 1.33, 737.63), three metres
	   apart, each checked walkable against the Area navigation. */
	struct CUTSCENE_STAND_SPOT final
	{
		float fX;
		float fY;
		float fZ;
	};
	static constexpr CUTSCENE_STAND_SPOT SPOTS[] = {
		{ -1.166f, 1.31f, 745.078f },
		{ -3.339f, 1.32f, 743.009f },
		{ -5.512f, 1.30f, 740.941f },
		{ -7.685f, 1.32f, 738.872f },
	};
	static constexpr float BOSS_X = -0.2883f;
	static constexpr float BOSS_Z = 737.6292f;

	std::uint32_t placed = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp)
			continue;
		const CUTSCENE_STAND_SPOT& spot =
			SPOTS[placed % (sizeof(SPOTS) / sizeof(SPOTS[0]))];
		/* A cutscene entrance is a placement, not a move: clear whatever the
		   player was doing so no queued path or skill drags them back off. */
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.fPositionX = spot.fX;
		player.fPositionY = spot.fY;
		player.fPositionZ = spot.fZ;
		/* Face the boss so the party watches the show. */
		player.fYawDegrees = std::atan2(BOSS_X - spot.fX, BOSS_Z - spot.fZ) *
			RADIANS_TO_DEGREES;
		++placed;
	}
	return placed;
}

void LostArk::Server::CGameRoom::Send_InteractPrompt(
	const SERVER_INTERACT_PROMPT_EDGE& edge)
{
	using namespace LostArk::Shared;

	const auto player = m_Players.find(edge.iPlayerId);
	if (m_Players.end() == player)
		return;
	const std::shared_ptr<CClientSession> session =
		Find_Session(player->second.iSessionId);
	if (nullptr == session)
		return;
	S2C_INTERACT_PROMPT message{};
	message.strTriggerPlacementId = edge.strTriggerPlacementId;
	message.bAvailable = edge.bAvailable;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!session->Send_Frame(
		PACKET_TYPE::S2C_INTERACT_PROMPT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_InteractTrigger(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_INTERACT_TRIGGER& request)
{
	using namespace LostArk::Shared;

	const auto playerId = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == playerId)
		return;
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	const std::uint32_t actionTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_ServerTriggerSystem.Activate_Interact(
		playerId->second,
		request.strTriggerPlacementId,
		m_Players,
		actionTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind)
			{
				Broadcast_WorldSequencePlay(targetId);
				return true;
			}
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
				return m_SpawnGroupRuntime.Activate(targetId);
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
				return Activate_Encounter(targetId);
			return false;
		}))
	{
		return;
	}
	/* A gated box can move worlds like any other, so its transfer is staged
	   through the same pending list the tick uses. */
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[staged = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == staged;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	/* The offer is spent: withdraw it so the Client stops drawing the key.
	   A repeatable box re-offers on the next entry edge. */
	Send_InteractPrompt({ playerId->second,
		request.strTriggerPlacementId, false });
}

void LostArk::Server::CGameRoom::Broadcast_WorldSequencePlay(
	const std::string& instanceId)
{
	using namespace LostArk::Shared;

	/* Place before the frame goes out so the snapshot that carries the
	   cutscene already carries the party on the arena. */
	(void)Place_PartyForCutscene(instanceId);

	S2C_WORLD_SEQUENCE_PLAY message{};
	message.strSequenceInstanceId = instanceId;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Remove_FromParty(
	const LostArk::Shared::PLAYER_ID playerId)
{
	const auto partyIdIter = m_PartyIdByPlayerId.find(playerId);
	if (partyIdIter == m_PartyIdByPlayerId.end())
		return;
	const std::uint32_t partyId = partyIdIter->second;
	m_PartyIdByPlayerId.erase(partyIdIter);

	const auto membersIter = m_PartyMembersByPartyId.find(partyId);
	if (membersIter == m_PartyMembersByPartyId.end())
		return;
	std::vector<LostArk::Shared::PLAYER_ID>& members = membersIter->second;
	members.erase(
		std::remove(members.begin(), members.end(), playerId),
		members.end());
	if (members.empty())
	{
		m_PartyMembersByPartyId.erase(membersIter);
		return;
	}
	Broadcast_PartyRoster(partyId);
}

bool LostArk::Server::CGameRoom::Is_PlayerNearValtanEntryNpc(
	const SERVER_PLAYER& player, const std::string& npcPlacementId) const
{
	using namespace LostArk::Shared;
	// Handle_ConfirmNpcEntry의 VALTAN_ENTRY_GUIDE_NPCS와 같은 placement 집합. 여기서는
	// proximity만 검증하고, target world는 NPC가 아니라 propose의 eTarget이 소유한다.
	static constexpr const char* GUIDE_NPC_PLACEMENT_IDS[] = {
		"npc.bern.beda.guide", "npc.bern.aylara" };
	constexpr float INTERACTION_RADIUS = 3.f;
	const bool isGuide = std::any_of(
		std::begin(GUIDE_NPC_PLACEMENT_IDS), std::end(GUIDE_NPC_PLACEMENT_IDS),
		[&npcPlacementId](const char* id) { return npcPlacementId == id; });
	if (!isGuide)
		return false;
	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&npcPlacementId](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == npcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
		return false;
	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	return deltaX * deltaX + deltaZ * deltaZ <=
		INTERACTION_RADIUS * INTERACTION_RADIUS;
}

bool LostArk::Server::CGameRoom::Stage_PartyWorldTransfer(
	const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	if (batchMemberIds.empty())
		return false;
	const auto leaderIter = m_Players.find(batchMemberIds.front());
	if (leaderIter == m_Players.end())
		return false;
	const SERVER_PLAYER& leader = leaderIter->second;

	const auto isAlreadyStaged = [this](SESSION_ID sid)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sid](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sid ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sid) !=
						pending.PartyBatchSessionIds.end();
			});
	};

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = leader.iSessionId;
	transfer.eTargetWorldId = targetWorldId;
	transfer.eCharacterClass = leader.eCharacterClass;
	transfer.strNickName = leader.strNickName;
	transfer.iPartyRequestSequence = requestSequence;
	for (const PLAYER_ID memberId : batchMemberIds)
	{
		const auto memberIter = m_Players.find(memberId);
		if (memberIter == m_Players.end() ||
			CHARACTER_CLASS_ID::END == memberIter->second.eCharacterClass ||
			memberIter->second.strNickName.empty() ||
			isAlreadyStaged(memberIter->second.iSessionId))
		{
			return false;
		}
		if (batchMemberIds.size() > 1u)
			transfer.PartyBatchSessionIds.push_back(memberIter->second.iSessionId);
	}
	m_PendingWorldTransfers.push_back(std::move(transfer));
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_RaidEntryVote(
	const RAID_ENTRY_PROPOSAL& proposal, const bool bClosed,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	S2C_RAID_ENTRY_VOTE message{};
	message.iProposalId = proposal.iProposalId;
	message.iAccepted = static_cast<std::uint8_t>(proposal.Accepted.size());
	message.iTotal = static_cast<std::uint8_t>(proposal.Voters.size());
	message.bClosed = bClosed;
	message.eResult = bClosed ? result : RAID_ENTRY_VOTE_RESULT::END;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto playerIter = m_Players.find(memberId);
		if (playerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(playerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_VOTE, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Handle_RaidEntryPropose(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request)
{
	using namespace LostArk::Shared;
	// 30Hz 기준 30초 미응답이면 tick 루프가 TIMEOUT으로 닫는다.
	constexpr std::uint32_t VOTE_TIMEOUT_TICKS = 30u * 30u;

	if (WORLD_ID::BERN != m_eWorldId || request.eTarget >= RAID_ENTRY_TARGET::END)
		return;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID proposerId = sessionIter->second;
	const auto playerIter = m_Players.find(proposerId);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& proposer = playerIter->second;
	if (0u == proposer.iCurrentHp || PLAYER_ACTION_STATE::NONE != proposer.eAction ||
		INVALID_SESSION_ID == proposer.iSessionId ||
		CHARACTER_CLASS_ID::END == proposer.eCharacterClass ||
		proposer.strNickName.empty())
	{
		return;
	}
	if (!Is_PlayerNearValtanEntryNpc(proposer, request.strNpcPlacementId))
		return;

	// 한 플레이어는 동시에 하나의 열린 proposal에만 속한다.
	const bool alreadyInVote = std::any_of(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[proposerId](const RAID_ENTRY_PROPOSAL& p)
		{
			return std::find(p.Voters.begin(), p.Voters.end(), proposerId) !=
				p.Voters.end();
		});
	if (alreadyInVote)
		return;

	std::uint32_t partyId = 0u;
	std::vector<PLAYER_ID> voters{ proposerId };
	const auto partyIdIter = m_PartyIdByPlayerId.find(proposerId);
	if (partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter = m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() > 1u)
		{
			// 파티 발의는 리더(members.front())만 가능. 비리더는 조용히 거절한다
			// (Client UI가 입장하기를 리더에게만 노출하므로 정상 경로에서 오지 않는다).
			if (membersIter->second.front() != proposerId)
				return;
			partyId = partyIdIter->second;
			voters = membersIter->second;
		}
	}

	RAID_ENTRY_PROPOSAL proposal{};
	proposal.iProposalId = m_iNextRaidEntryProposalId++;
	if (0u == m_iNextRaidEntryProposalId)
		m_iNextRaidEntryProposalId = 1u;
	proposal.iPartyId = partyId;
	proposal.iRequestSequence = request.iRequestSequence;
	proposal.eTarget = request.eTarget;
	proposal.strNpcPlacementId = request.strNpcPlacementId;
	proposal.Voters = voters;
	proposal.iDeadlineTick = m_iServerTick + VOTE_TIMEOUT_TICKS;

	S2C_RAID_ENTRY_PROMPT prompt{};
	prompt.iProposalId = proposal.iProposalId;
	prompt.iProposerNetEntityId = proposer.iNetEntityId;
	prompt.eTarget = proposal.eTarget;
	prompt.strProposerNickname = proposer.strNickName;
	CPacketWriter promptWriter;
	if (!Write_Message(promptWriter, prompt))
		return;
	for (const PLAYER_ID memberId : proposal.Voters)
	{
		const auto memberPlayerIter = m_Players.find(memberId);
		if (memberPlayerIter == m_Players.end())
			continue;
		const std::shared_ptr<CClientSession> session =
			Find_Session(memberPlayerIter->second.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(
				PACKET_TYPE::S2C_RAID_ENTRY_PROMPT, promptWriter.Get_Buffer()))
		{
			session->Request_Close();
		}
	}

	m_RaidEntryProposals.push_back(std::move(proposal));
	Broadcast_RaidEntryVote(
		m_RaidEntryProposals.back(), false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Handle_RaidEntryRespond(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID responderId = sessionIter->second;

	const auto proposalIter = std::find_if(
		m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
		[&request](const RAID_ENTRY_PROPOSAL& p)
		{
			return p.iProposalId == request.iProposalId;
		});
	if (proposalIter == m_RaidEntryProposals.end())
		return;
	if (std::find(proposalIter->Voters.begin(), proposalIter->Voters.end(),
			responderId) == proposalIter->Voters.end())
	{
		return;
	}
	if (!request.bAccepted)
	{
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::DECLINED);
		return;
	}
	if (std::find(proposalIter->Accepted.begin(), proposalIter->Accepted.end(),
			responderId) == proposalIter->Accepted.end())
	{
		proposalIter->Accepted.push_back(responderId);
	}
	if (proposalIter->Accepted.size() >= proposalIter->Voters.size())
		Close_RaidEntryVote(*proposalIter, RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED);
	else
		Broadcast_RaidEntryVote(*proposalIter, false, RAID_ENTRY_VOTE_RESULT::END);
}

void LostArk::Server::CGameRoom::Close_RaidEntryVote(
	RAID_ENTRY_PROPOSAL& proposal,
	const LostArk::Shared::RAID_ENTRY_VOTE_RESULT result)
{
	using namespace LostArk::Shared;
	RAID_ENTRY_VOTE_RESULT finalResult = result;
	if (RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED == result)
	{
		const WORLD_ID targetWorld =
			(RAID_ENTRY_TARGET::KAKULSAYDON == proposal.eTarget)
			? WORLD_ID::KAKULSAYDON_ARENA : WORLD_ID::VALTAN_ARENA;
		// 수락 완료와 실제 stage 사이에 멤버가 unavailable해졌으면 전송하지 않고
		// CANCELLED로 낮춰 전원이 Bern에 남게 한다(부분 이동 금지).
		if (!Stage_PartyWorldTransfer(
				proposal.Voters, targetWorld, proposal.iRequestSequence))
		{
			finalResult = RAID_ENTRY_VOTE_RESULT::CANCELLED;
		}
	}
	Broadcast_RaidEntryVote(proposal, true, finalResult);
	const std::uint32_t closedId = proposal.iProposalId;
	m_RaidEntryProposals.erase(
		std::remove_if(m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[closedId](const RAID_ENTRY_PROPOSAL& p)
			{
				return p.iProposalId == closedId;
			}),
		m_RaidEntryProposals.end());
}

void LostArk::Server::CGameRoom::Expire_RaidEntryProposals()
{
	using namespace LostArk::Shared;
	// Close_RaidEntryVote가 벡터를 수정하므로 만료 id를 먼저 모은 뒤 닫는다.
	std::vector<std::uint32_t> expiredIds;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (m_iServerTick >= p.iDeadlineTick)
			expiredIds.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : expiredIds)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::TIMEOUT);
	}
}

void LostArk::Server::CGameRoom::Cancel_RaidEntryProposalsInvolving(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	std::vector<std::uint32_t> ids;
	for (const RAID_ENTRY_PROPOSAL& p : m_RaidEntryProposals)
	{
		if (std::find(p.Voters.begin(), p.Voters.end(), playerId) != p.Voters.end())
			ids.push_back(p.iProposalId);
	}
	for (const std::uint32_t id : ids)
	{
		const auto it = std::find_if(
			m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(),
			[id](const RAID_ENTRY_PROPOSAL& p) { return p.iProposalId == id; });
		if (it != m_RaidEntryProposals.end())
			Close_RaidEntryVote(*it, RAID_ENTRY_VOTE_RESULT::CANCELLED);
	}
}

bool LostArk::Server::CGameRoom::Transfer_PartyTo(
	CGameRoom& target, const std::vector<SESSION_ID>& leaderFirstSessionIds,
	LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status)
{
	using namespace LostArk::Shared;
	outResult = PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
	const auto reject = [&outResult, &status](const PARTY_TRANSFER_RESULT reason, const char* detail)
	{
		outResult = reason;
		status = detail;
		return false;
	};
	if (!m_isReady || !target.m_isReady || WORLD_ID::BERN != m_eWorldId ||
		WORLD_ID::VALTAN_ARENA != target.m_eWorldId ||
		leaderFirstSessionIds.size() < 2u || leaderFirstSessionIds.size() > MAX_PARTY_MEMBERS)
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "invalid party transfer world/batch");
	const auto leader = m_PlayerIdBySessionId.find(leaderFirstSessionIds.front());
	if (leader == m_PlayerIdBySessionId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party leader is no longer present");
	const auto sourceParty = m_PartyIdByPlayerId.find(leader->second);
	if (sourceParty == m_PartyIdByPlayerId.end())
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party no longer exists");
	const auto sourceMembers = m_PartyMembersByPartyId.find(sourceParty->second);
	if (sourceMembers == m_PartyMembersByPartyId.end() ||
		sourceMembers->second.size() != leaderFirstSessionIds.size() ||
		sourceMembers->second.front() != leader->second)
		return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party changed before transfer");
	if (0u == target.m_iNextPartyId || target.m_PartyMembersByPartyId.contains(target.m_iNextPartyId))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party identity is exhausted");

	std::vector<STAGED_PLAYER_ENTRY> entries;
	std::vector<NET_ENTITY_ID> departingEntities;
	entries.reserve(leaderFirstSessionIds.size());
	departingEntities.reserve(leaderFirstSessionIds.size());
	for (std::size_t index = 0; index < leaderFirstSessionIds.size(); ++index)
	{
		const auto member = m_Players.find(sourceMembers->second[index]);
		if (member == m_Players.end() ||
			member->second.iSessionId != leaderFirstSessionIds[index] ||
			std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.begin() + index,
				leaderFirstSessionIds[index]) != leaderFirstSessionIds.begin() + index)
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "source party member identity changed");
		const auto session = Find_Session(member->second.iSessionId);
		if (nullptr == session || session->Is_Closing())
			return reject(PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE, "party member session is terminal");
		C2S_ENTER_WORLD enter{};
		enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enter.eWorldId = target.m_eWorldId;
		enter.eCharacterClass = member->second.eCharacterClass;
		enter.strNickName = member->second.strNickName;
		STAGED_PLAYER_ENTRY entry{};
		SESSION_DIAGNOSTIC_REASON reason{};
		if (!target.Stage_PlayerEntry(session, enter, entries, entry, reason, status))
		{
			outResult = SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL == reason ?
				PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL : PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entries.push_back(std::move(entry));
		departingEntities.push_back(member->second.iNetEntityId);
	}
	std::vector<CLIENT_SESSION_RELIABLE_BATCH> outboundBatches;
	S2C_PARTY_ROSTER roster{};
	for (const auto& entry : entries)
		roster.Members.push_back({ entry.Player.iNetEntityId, entry.Player.strNickName,
			entry.Player.eCharacterClass });
	CPacketWriter rosterWriter;
	if (!Write_Message(rosterWriter, roster))
		return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target party roster failed encoding");
	for (auto& entry : entries)
	{
		if (!target.Build_PlayerEntryFrames(entry, entries, status))
		{
			outResult = PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED;
			return false;
		}
		entry.Frames.push_back({ PACKET_TYPE::S2C_PARTY_ROSTER, rosterWriter.Get_Buffer() });
		outboundBatches.push_back({ entry.pSession, entry.Frames });
	}
	// Include observer notifications in the same bounded FIFO reservation;
	// neither a slow member nor a slow spectator can cause a partial commit.
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (std::find(leaderFirstSessionIds.begin(), leaderFirstSessionIds.end(),
			player.iSessionId) != leaderFirstSessionIds.end()) continue;
		CLIENT_SESSION_RELIABLE_BATCH observer{ Find_Session(player.iSessionId), {} };
		for (const NET_ENTITY_ID entityId : departingEntities)
		{
			S2C_PLAYER_DESPAWNED message{};
			message.iNetEntityId = entityId;
			message.eReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "source departure payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer() });
		}
		outboundBatches.push_back(std::move(observer));
	}
	for (const auto& [id, player] : target.m_Players)
	{
		(void)id;
		CLIENT_SESSION_RELIABLE_BATCH observer{ target.Find_Session(player.iSessionId), {} };
		for (const auto& entry : entries)
		{
			S2C_PLAYER_SPAWNED message{};
			message.iPlayerId = entry.Player.iPlayerId;
			message.iNetEntityId = entry.Player.iNetEntityId;
			message.eCharacterClass = entry.Player.eCharacterClass;
			message.strNickName = entry.Player.strNickName;
			message.fPositionX = entry.Player.fPositionX;
			message.fPositionY = entry.Player.fPositionY;
			message.fPositionZ = entry.Player.fPositionZ;
			message.fYawDegrees = entry.Player.fYawDegrees;
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return reject(PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, "target spawn payload failed encoding");
			observer.Frames.push_back({ PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer() });
		}
		outboundBatches.push_back(std::move(observer));
	}

	// All allocating membership work is staged before taking outbound locks.
	// Commit below contains only erases, swaps and atomic player-id stores.
	auto targetPlayers = target.m_Players;
	auto targetSessionPlayers = target.m_PlayerIdBySessionId;
	auto targetEntityPlayers = target.m_PlayerIdByEntityId;
	auto targetSessions = target.m_Sessions;
	auto targetPartyIds = target.m_PartyIdByPlayerId;
	auto targetParties = target.m_PartyMembersByPartyId;
	std::vector<PLAYER_ID> targetMembers;
	targetMembers.reserve(entries.size());
	for (const auto& entry : entries)
	{
		const SERVER_PLAYER& player = entry.Player;
		targetPlayers.emplace(player.iPlayerId, player);
		targetSessionPlayers.emplace(player.iSessionId, player.iPlayerId);
		targetEntityPlayers.emplace(player.iNetEntityId, player.iPlayerId);
		targetSessions.insert_or_assign(player.iSessionId, entry.pSession);
		targetPartyIds.emplace(player.iPlayerId, target.m_iNextPartyId);
		targetMembers.push_back(player.iPlayerId);
	}
	targetParties.emplace(target.m_iNextPartyId, std::move(targetMembers));
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare(outboundBatches, status))
	{
		outResult = PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY;
		return false;
	}
	// No callback here may send to the locked queues. Whole-party removal has
	// no intermediate roster; all departures/arrivals were staged above.
	for (const PLAYER_ID memberId : sourceMembers->second)
		m_PartyIdByPlayerId.erase(memberId);
	m_PartyMembersByPartyId.erase(sourceMembers);
	for (const SESSION_ID sessionId : leaderFirstSessionIds)
		Leave(sessionId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED, false);
	target.m_Players.swap(targetPlayers);
	target.m_PlayerIdBySessionId.swap(targetSessionPlayers);
	target.m_PlayerIdByEntityId.swap(targetEntityPlayers);
	target.m_Sessions.swap(targetSessions);
	target.m_PartyIdByPlayerId.swap(targetPartyIds);
	target.m_PartyMembersByPartyId.swap(targetParties);
	target.m_iNextPlayerId += static_cast<PLAYER_ID>(entries.size());
	target.m_iNextNetEntityId += static_cast<NET_ENTITY_ID>(entries.size());
	++target.m_iNextPartyId;
	for (const auto& entry : entries)
		entry.pSession->Bind_PlayerId(entry.Player.iPlayerId);
	outbound.Commit();
	status = "party transfer committed";
	return true;
}

void LostArk::Server::CGameRoom::Notify_PartyTransferFailure(
	const SESSION_ID sessionId, const std::uint32_t requestSequence,
	const LostArk::Shared::WORLD_ID targetWorldId,
	const LostArk::Shared::PARTY_TRANSFER_RESULT result)
{
	if (0u == requestSequence || !m_PlayerIdBySessionId.contains(sessionId)) return;
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT message{};
	message.iRequestSequence = requestSequence;
	message.eTargetWorldId = targetWorldId;
	message.eResult = result;
	m_PendingPartyTransferResults.insert_or_assign(sessionId, message);
	Flush_PartyTransferResults();
}

void LostArk::Server::CGameRoom::Flush_PartyTransferResults()
{
	using namespace LostArk::Shared;
	for (auto iter = m_PendingPartyTransferResults.begin(); iter != m_PendingPartyTransferResults.end();)
	{
		const auto session = Find_Session(iter->first);
		if (nullptr == session || session->Is_Closing())
		{
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CPacketWriter writer;
		if (!Write_Message(writer, iter->second))
		{
			m_strStatus = "party transfer failure notice failed validation";
			iter = m_PendingPartyTransferResults.erase(iter);
			continue;
		}
		CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
		std::string status;
		if (!outbound.Prepare({ { session, {
			{ PACKET_TYPE::S2C_PARTY_TRANSFER_RESULT, writer.Get_Buffer() } } } }, status))
		{
			++iter;
			continue;
		}
		outbound.Commit();
		iter = m_PendingPartyTransferResults.erase(iter);
	}
}

void LostArk::Server::CGameRoom::Handle_Chat(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHAT& request)
{
	using namespace LostArk::Shared;

	const auto senderPlayerIdIter = m_PlayerIdBySessionId.find(sessionId);
	if (senderPlayerIdIter == m_PlayerIdBySessionId.end())
		return;
	const auto senderIter = m_Players.find(senderPlayerIdIter->second);
	if (senderIter == m_Players.end())
		return;

	S2C_CHAT message{};
	message.iFromNetEntityId = senderIter->second.iNetEntityId;
	message.strFromNickname = senderIter->second.strNickName;
	message.strText = request.strText;

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;

	// Every current room member, sender included -- see Handle_Chat's own
	// header comment for why the sender reads its own bubble off this same
	// broadcast instead of a second local-only path.
	for (const auto& [playerId, player] : m_Players)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(player.iSessionId);
		if (nullptr != session &&
			!session->Send_Frame(PACKET_TYPE::S2C_CHAT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
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
LostArk::Server::CGameRoom::Find_KoukuSaydonAuditionBoss()
{
	const auto found = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this](const SERVER_WORLD_ENTITY& entity)
		{
			return CKoukuSaydonBrain::Is_GateOneBoss(m_eWorldId, entity);
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}

LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_KoukuSaydonPatternAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
	LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult)
{
	using namespace LostArk::Shared;
	outResult = {};
	outResult.iRequestSequence = request.iRequestSequence;
	outResult.eOperation = request.eOperation;
	outResult.Scope = request.Scope;
	outResult.strRequestedPatternId = request.strPatternId;
	outResult.PinnedGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	outResult.iPinnedSourceRevision =
		CKoukuSaydonBrain::Resolve_ProductSourceRevision(
			m_GameplayCatalog.Active());

	const auto reject = [&outResult](
		const KOUKUSAYDON_PATTERN_AUDITION_RESULT result,
		std::string reason)
	{
		outResult.eResult = result;
		outResult.iRoomAuditionEpoch = 0u;
		outResult.iBossNetEntityId = INVALID_NET_ENTITY_ID;
		outResult.strResolvedPatternId.clear();
		outResult.iPatternSequence = 0u;
		outResult.iStageIndex = 0u;
		outResult.strReason = std::move(reason);
		return result;
	};

#ifndef _DEBUG
	(void)sessionId;
	return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_RELEASE_BUILD,
		"KoukuSaydon pattern audition is available only in Debug builds");
#else
	const auto sameRequest = [](const auto& left, const auto& right)
	{
		return left.iRequestSequence == right.iRequestSequence &&
			left.eOperation == right.eOperation &&
			left.Scope.eWorldId == right.Scope.eWorldId &&
			left.Scope.strEncounterId == right.Scope.strEncounterId &&
			left.Scope.strBossPlacementId == right.Scope.strBossPlacementId &&
			left.Scope.strBossArchetypeId == right.Scope.strBossArchetypeId &&
			left.Scope.ExpectedGameplayRevision ==
				right.Scope.ExpectedGameplayRevision &&
			left.Scope.iExpectedSourceRevision ==
				right.Scope.iExpectedSourceRevision &&
			left.strPatternId == right.strPatternId;
	};
	const auto previous =
		m_KoukuSaydonPatternAuditionReceiptBySessionId.find(sessionId);
	if (m_KoukuSaydonPatternAuditionReceiptBySessionId.end() != previous &&
		request.iRequestSequence <= previous->second.Request.iRequestSequence)
	{
		if (sameRequest(request, previous->second.Request))
		{
			outResult = previous->second.Result;
			if (KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
				previous->second.Result.eResult)
			{
				outResult.eResult =
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED;
				if (previous->second.LastLifecycle)
				{
					const auto& lifecycle = *previous->second.LastLifecycle;
					outResult.strResolvedPatternId = lifecycle.strPatternId;
					outResult.iPatternSequence = lifecycle.iPatternSequence;
					outResult.iStageIndex = lifecycle.iStageIndex;
				}
			}
			return outResult.eResult;
		}
		return reject(
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST,
			"KoukuSaydon audition request sequence is stale or reused");
	}

	const bool exactScope =
		WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
		WORLD_ID::KAKULSAYDON_ARENA == request.Scope.eWorldId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == request.Scope.strEncounterId &&
		KOUKUSAYDON_G1_BOSS_PLACEMENT_ID ==
			request.Scope.strBossPlacementId &&
		KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID ==
			request.Scope.strBossArchetypeId;
	if (!exactScope)
		return reject(
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SCOPE_MISMATCH,
			"KoukuSaydon audition scope does not match the Gate 1 boss tuple");
	if (request.Scope.ExpectedGameplayRevision !=
		m_GameplayCatalog.Get_ActiveRevision())
	{
		return reject(
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_REVISION_MISMATCH,
			"KoukuSaydon audition expected gameplay revision is not active");
	}
	if (request.Scope.iExpectedSourceRevision !=
		outResult.iPinnedSourceRevision)
	{
		return reject(
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::
				REJECTED_SOURCE_REVISION_MISMATCH,
			"KoukuSaydon audition expected Product source revision is not active");
	}
	SERVER_WORLD_ENTITY* boss = Find_KoukuSaydonAuditionBoss();
	if (nullptr == boss)
		return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_NO_BOSS,
			"KoukuSaydon Gate 1 boss is not spawned");
	if (0u == boss->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss->eAction)
		return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_BOSS_DEAD,
			"KoukuSaydon Gate 1 boss is dead");
	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE !=
		m_KoukuSaydonPatternAudition.ePhase || !boss->strPatternId.empty())
	{
		return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_BUSY,
			"KoukuSaydon Gate 1 boss already owns a pattern occurrence");
	}

	std::vector<std::string> patternIds;
	std::vector<std::uint32_t> transitionTicks;
	std::string patternStatus;
	if (KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED ==
		request.eOperation)
	{
		const auto* definitions = m_GameplayCatalog.Active().Find_BossPatterns(
			std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
		const auto found = nullptr == definitions ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(definitions->begin(), definitions->end(),
				[&request](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return pattern.strPatternId == request.strPatternId;
				});
		if (nullptr == definitions || definitions->end() == found)
			return reject(
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNKNOWN_PATTERN,
				"KoukuSaydon selected pattern ID is not admitted");
		if (!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
			*found, patternStatus))
		{
			return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::
				REJECTED_UNSUPPORTED_PATTERN, std::move(patternStatus));
		}
		patternIds.push_back(request.strPatternId);
	}
	else
	{
		const BOSS_PATTERN_SEQUENCE_DEFINITION* sequence =
			m_GameplayCatalog.Active().Find_BossPatternSequence(
				std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
		if (nullptr == sequence ||
			KOUKUSAYDON_G1_PLAY_ALL_SEQUENCE_ID != sequence->strSequenceId ||
			sequence->PatternIds.empty() ||
			sequence->PatternIds.size() != sequence->iExpectedStepCount ||
			sequence->TransitionPursuitTicks.size() + 1u !=
				sequence->PatternIds.size())
		{
			return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::
				REJECTED_NO_PRODUCT_SEQUENCE,
				"KoukuSaydon Product pattern sequence is unavailable");
		}
		for (const std::string& patternId : sequence->PatternIds)
		{
			if (nullptr == CKoukuSaydonBrain::Find_AnimationOnlyPattern(
				m_GameplayCatalog.Active(), patternId, patternStatus))
			{
				return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_UNSUPPORTED_PATTERN, std::move(patternStatus));
			}
		}
		patternIds = sequence->PatternIds;
		transitionTicks = sequence->TransitionPursuitTicks;
	}

	m_KoukuSaydonPatternAudition = {};
	m_KoukuSaydonPatternAudition.ePhase =
		KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
	m_KoukuSaydonPatternAudition.iOwnerSessionId = sessionId;
	m_KoukuSaydonPatternAudition.Request = request;
	m_KoukuSaydonPatternAudition.iRoomAuditionEpoch =
		m_iNextKoukuSaydonPatternAuditionEpoch;
	m_iNextKoukuSaydonPatternAuditionEpoch =
		Add_ServerTicksSkippingReservedZero(
			m_iNextKoukuSaydonPatternAuditionEpoch, 1u);
	m_KoukuSaydonPatternAudition.iBossEntityId = boss->iNetEntityId;
	m_KoukuSaydonPatternAudition.PinnedGameplayRevision =
		request.Scope.ExpectedGameplayRevision;
	m_KoukuSaydonPatternAudition.iPinnedSourceRevision =
		request.Scope.iExpectedSourceRevision;
	m_KoukuSaydonPatternAudition.PatternIds = std::move(patternIds);
	m_KoukuSaydonPatternAudition.TransitionTicks = std::move(transitionTicks);
	m_KoukuSaydonPatternAudition.iNextStartTick =
		0u == m_iServerTick ? 1u :
		Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u);

	outResult.eResult = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
	outResult.iRoomAuditionEpoch =
		m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
	outResult.iBossNetEntityId = boss->iNetEntityId;
	outResult.strResolvedPatternId =
		m_KoukuSaydonPatternAudition.PatternIds.front();
	outResult.PinnedGameplayRevision =
		m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
	outResult.iPinnedSourceRevision =
		m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
	m_KoukuSaydonPatternAuditionReceiptBySessionId[sessionId] =
		{ request, outResult, std::nullopt };
	Queue_KoukuSaydonPatternAuditionLifecycle(
		outResult.strResolvedPatternId, 0u, 0u,
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING);
	return outResult.eResult;
#endif
}

void LostArk::Server::CGameRoom::Handle_KoukuSaydonPatternAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
	(void)Evaluate_KoukuSaydonPatternAudition(sessionId, request, result);
	if (!Send_KoukuSaydonPatternAuditionResult(session, result))
		session->Request_Close();
}

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_KoukuSaydonPatternAuditionLifecycle(
	const std::string& patternId,
	const std::uint32_t patternSequence,
	const std::uint32_t stageIndex,
	const LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (INVALID_SESSION_ID == m_KoukuSaydonPatternAudition.iOwnerSessionId)
		return;
	S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE message{};
	message.iRequestSequence =
		m_KoukuSaydonPatternAudition.Request.iRequestSequence;
	message.eOperation = m_KoukuSaydonPatternAudition.Request.eOperation;
	message.Scope = m_KoukuSaydonPatternAudition.Request.Scope;
	message.iRoomAuditionEpoch =
		m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
	message.iBossNetEntityId = m_KoukuSaydonPatternAudition.iBossEntityId;
	message.strPatternId = patternId;
	message.iPatternSequence = patternSequence;
	message.iStageIndex = stageIndex;
	message.eState = state;
	message.PinnedGameplayRevision =
		m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
	message.iPinnedSourceRevision =
		m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
	message.strReason = std::move(reason);
	m_PendingKoukuSaydonPatternAuditionLifecycle.push_back(
		{ m_KoukuSaydonPatternAudition.iOwnerSessionId, message });
	const auto receipt = m_KoukuSaydonPatternAuditionReceiptBySessionId.find(
		m_KoukuSaydonPatternAudition.iOwnerSessionId);
	if (m_KoukuSaydonPatternAuditionReceiptBySessionId.end() != receipt &&
		receipt->second.Request.iRequestSequence == message.iRequestSequence)
	{
		receipt->second.LastLifecycle = message;
	}
}

bool LostArk::Server::CGameRoom::Flush_KoukuSaydonPatternAuditionLifecycle()
{
	using namespace LostArk::Shared;
	auto pending = std::move(m_PendingKoukuSaydonPatternAuditionLifecycle);
	m_PendingKoukuSaydonPatternAuditionLifecycle.clear();
	for (const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& targeted : pending)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(targeted.iSessionId);
		if (nullptr == session)
			continue;
		CPacketWriter writer;
		if (!Write_Message(writer, targeted.Message))
			return false;
		if (!session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE,
			writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Clear_KoukuSaydonPatternAudition()
{
	m_KoukuSaydonPatternAudition = {};
}
#endif

bool LostArk::Server::CGameRoom::Update_KoukuSaydonBoss(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
#ifndef _DEBUG
	if (!boss.strPatternId.empty())
		m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick);
	boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
	return true;
#else
	using namespace LostArk::Shared;
	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
		m_KoukuSaydonPatternAudition.ePhase)
	{
		if (!boss.strPatternId.empty())
		{
			m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick);
			m_strStatus = "KoukuSaydon boss had an unowned pattern occurrence";
			return false;
		}
		boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
		return true;
	}
	if (m_KoukuSaydonPatternAudition.iBossEntityId != boss.iNetEntityId ||
		m_KoukuSaydonPatternAudition.iPatternIndex >=
			m_KoukuSaydonPatternAudition.PatternIds.size())
	{
		m_strStatus = "KoukuSaydon audition lost its exact boss or pattern slot";
		if (!boss.strPatternId.empty())
			m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick);
		Queue_KoukuSaydonPatternAuditionLifecycle(
			m_KoukuSaydonPatternAudition.PatternIds.empty() ?
				std::string{ "KAKULSAYDON_G1_INVALID" } :
				m_KoukuSaydonPatternAudition.PatternIds.front(),
			boss.iPatternSequence, boss.iPatternStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
			m_strStatus);
		Clear_KoukuSaydonPatternAudition();
		return true;
	}
	const CGameplayCatalog* pinnedCatalog = m_GameplayCatalog.Resolve(
		m_KoukuSaydonPatternAudition.PinnedGameplayRevision);
	const std::string& selectedPatternId =
		m_KoukuSaydonPatternAudition.PatternIds[
			m_KoukuSaydonPatternAudition.iPatternIndex];
	if (nullptr == pinnedCatalog ||
		m_KoukuSaydonPatternAudition.iPinnedSourceRevision !=
			CKoukuSaydonBrain::Resolve_ProductSourceRevision(*pinnedCatalog))
	{
		m_strStatus = nullptr == pinnedCatalog ?
			"KoukuSaydon pinned gameplay generation is unavailable" :
			"KoukuSaydon pinned Product source revision is unavailable";
		if (!boss.strPatternId.empty())
			m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick);
		Queue_KoukuSaydonPatternAuditionLifecycle(
			selectedPatternId, boss.iPatternSequence, boss.iPatternStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
			m_strStatus);
		Clear_KoukuSaydonPatternAudition();
		return true;
	}

	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING ==
			m_KoukuSaydonPatternAudition.ePhase &&
		Has_ReachedServerTick(
			serverTick, m_KoukuSaydonPatternAudition.iNextStartTick))
	{
		std::string status;
		const BOSS_PATTERN_DEFINITION* pattern =
			CKoukuSaydonBrain::Find_AnimationOnlyPattern(
				*pinnedCatalog, selectedPatternId, status);
		if (nullptr == pattern || !m_KoukuSaydonBrain.Begin_Pattern(
			boss, *pattern, m_KoukuSaydonPatternAudition.PinnedGameplayRevision,
			serverTick, status))
		{
			m_strStatus = status.empty() ?
				"KoukuSaydon pattern begin failed" : std::move(status);
			Queue_KoukuSaydonPatternAuditionLifecycle(
				selectedPatternId, boss.iPatternSequence, boss.iPatternStageIndex,
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
				m_strStatus);
			Clear_KoukuSaydonPatternAudition();
			return true;
		}
		m_KoukuSaydonPatternAudition.ePhase =
			KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
		Queue_KoukuSaydonPatternAuditionLifecycle(
			selectedPatternId, boss.iPatternSequence, boss.iPatternStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE);
	}
	if (KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING ==
		m_KoukuSaydonPatternAudition.ePhase)
	{
		return true;
	}

	const std::string completedPatternId = boss.strPatternId;
	const std::uint32_t occurrenceSequence = boss.iPatternSequence;
	const std::uint32_t previousStageIndex = boss.iPatternStageIndex;
	std::string status;
	const KOUKUSAYDON_BRAIN_UPDATE_RESULT update =
		m_KoukuSaydonBrain.Update(boss, *pinnedCatalog, serverTick, status);
	if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED == update)
	{
		Queue_KoukuSaydonPatternAuditionLifecycle(
			selectedPatternId, boss.iPatternSequence, boss.iPatternStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		return true;
	}
	if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION == update ||
		KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD == update)
	{
		m_strStatus = status.empty() ?
			"KoukuSaydon pattern occurrence aborted" : std::move(status);
		Queue_KoukuSaydonPatternAuditionLifecycle(
			selectedPatternId, occurrenceSequence, previousStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
			m_strStatus);
		Clear_KoukuSaydonPatternAudition();
		return true;
	}
	if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED != update)
		return true;

	Queue_KoukuSaydonPatternAuditionLifecycle(
		completedPatternId, occurrenceSequence, previousStageIndex,
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PATTERN_COMPLETED);
	const std::size_t completedIndex =
		m_KoukuSaydonPatternAudition.iPatternIndex;
	if (completedIndex + 1u <
		m_KoukuSaydonPatternAudition.PatternIds.size())
	{
		++m_KoukuSaydonPatternAudition.iPatternIndex;
		m_KoukuSaydonPatternAudition.ePhase =
			KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
		const std::uint32_t transitionTicks = (std::max)(1u,
			m_KoukuSaydonPatternAudition.TransitionTicks[completedIndex]);
		m_KoukuSaydonPatternAudition.iNextStartTick =
			Add_ServerTicksSkippingReservedZero(serverTick, transitionTicks);
		Queue_KoukuSaydonPatternAuditionLifecycle(
			m_KoukuSaydonPatternAudition.PatternIds[
				m_KoukuSaydonPatternAudition.iPatternIndex],
			0u, 0u,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING);
		return true;
	}
	Queue_KoukuSaydonPatternAuditionLifecycle(
		completedPatternId, occurrenceSequence, previousStageIndex,
		KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED);
	Clear_KoukuSaydonPatternAudition();
	boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
	return true;
#endif
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
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction ||
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING == player.eAction)
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
void LostArk::Server::CGameRoom::Queue_ValtanAuditionLifecycle(
	const SESSION_ID ownerSessionId,
	const std::uint32_t requestSequence,
	const std::uint32_t roomEpoch,
	const std::uint32_t patternSequence,
	const std::string& patternId,
	const LostArk::Shared::GameplayDataRevision& pinnedRevision,
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	if (INVALID_SESSION_ID == ownerSessionId)
		return;
	LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE message{};
	message.iRequestSequence = requestSequence;
	message.iRoomAuditionEpoch = roomEpoch;
	message.iPatternSequence = patternSequence;
	message.strPatternId = patternId;
	message.eState = state;
	message.PinnedDefinitionRevision = pinnedRevision;
	message.strReason = std::move(reason);
	m_PendingValtanAuditionLifecycle.push_back({ ownerSessionId, std::move(message) });
}

void LostArk::Server::CGameRoom::Queue_ValtanPatternIdAuditionLifecycle(
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
			m_ValtanPatternIdAudition.ePhase ||
		INVALID_SESSION_ID == m_ValtanPatternIdAudition.iOwnerSessionId ||
		m_ValtanPatternIdAudition.bAdoptedLivePredecessor)
	{
		return;
	}
	Queue_ValtanAuditionLifecycle(
		m_ValtanPatternIdAudition.iOwnerSessionId,
		m_ValtanPatternIdAudition.iRequestSequence,
		m_ValtanPatternIdAudition.iRoomAuditionEpoch,
		m_ValtanPatternIdAudition.iExpectedPatternSequence,
		m_ValtanPatternIdAudition.strPatternId,
		m_ValtanPatternIdAudition.PinnedDefinitionRevision, state, std::move(reason));
	const auto receipt = m_ValtanPatternIdAuditionSequenceBySessionId.find(
		m_ValtanPatternIdAudition.iOwnerSessionId);
	if (m_ValtanPatternIdAuditionSequenceBySessionId.end() != receipt &&
		receipt->second.Request.iRequestSequence ==
			m_ValtanPatternIdAudition.iRequestSequence &&
		!m_PendingValtanAuditionLifecycle.empty())
	{
		receipt->second.LastLifecycle =
			m_PendingValtanAuditionLifecycle.back().Message;
	}
}

void LostArk::Server::CGameRoom::Queue_ValtanNextPatternLifecycle(
	const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	Queue_ValtanAuditionLifecycle(
		reservation.iOwnerSessionId, reservation.iRequestSequence,
		reservation.iRoomAuditionEpoch, reservation.iExpectedPatternSequence,
		reservation.strPatternId, reservation.PinnedDefinitionRevision,
		state, std::move(reason));
}

bool LostArk::Server::CGameRoom::Is_ValtanPatternIdAuditionRunning() const noexcept
{
	return VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == m_ValtanPatternIdAudition.ePhase ||
		m_ValtanNextPattern.has_value();
}

void LostArk::Server::CGameRoom::Cancel_ValtanNextPatternReservation(std::string reason)
{
	if (!m_ValtanNextPattern)
		return;
	Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
		LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED, std::move(reason));
	m_ValtanNextPattern.reset();
}

void LostArk::Server::CGameRoom::Cancel_ValtanPatternIdAudition(std::string reason)
{
	Cancel_ValtanNextPatternReservation(reason);
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == m_ValtanPatternIdAudition.ePhase)
	{
		// Completed holds still own a Next CAS identity; invalidate it on reset.
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED, std::move(reason));
	}
	m_ValtanPatternIdAudition = {};
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

LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanNextPatternControl(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	std::uint32_t& outCurrentHealthBar)
{
	using namespace LostArk::Shared;
	outCurrentHealthBar = 0u;
	if ((WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		 WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId) ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
	}
	const auto previous = m_ValtanNextPatternReceiptBySessionId.find(sessionId);
	if (m_ValtanNextPatternReceiptBySessionId.end() != previous)
	{
		const C2S_VALTAN_AUDITION_REQUEST& prior = previous->second.Request;
		if (request.iRequestSequence == prior.iRequestSequence &&
			request.eOperation == prior.eOperation &&
			request.iTargetHealthBar == prior.iTargetHealthBar &&
			request.strBossPlacementId == prior.strBossPlacementId &&
			request.strPatternId == prior.strPatternId &&
			request.iPredecessorRoomAuditionEpoch == prior.iPredecessorRoomAuditionEpoch &&
			request.iPredecessorPatternSequence == prior.iPredecessorPatternSequence &&
			request.iExpectedNextRequestSequence == prior.iExpectedNextRequestSequence &&
			request.ExpectedDefinitionRevision == prior.ExpectedDefinitionRevision)
		{
			outCurrentHealthBar = previous->second.iCurrentHealthBar;
			// LIVE's result echoes epoch zero. Replay its still-owned lifecycle so
			// an exact retry can recover the authoritative epoch without restarting.
			if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == prior.eOperation &&
				VALTAN_AUDITION_RESULT::QUEUED == previous->second.Result)
			{
				if (m_ValtanNextPattern && m_ValtanNextPattern->iOwnerSessionId == sessionId &&
					m_ValtanNextPattern->iRequestSequence == prior.iRequestSequence)
				{
					Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
						m_ValtanNextPattern->bReportedWaitingForPlayer ?
						VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER :
						VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
				}
				else if (!m_ValtanPatternIdAudition.bAdoptedLivePredecessor &&
					m_ValtanPatternIdAudition.iOwnerSessionId == sessionId &&
					m_ValtanPatternIdAudition.iRequestSequence == prior.iRequestSequence)
				{
					const auto phase = m_ValtanPatternIdAudition.ePhase;
					if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == phase)
						Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
					else if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == phase)
						Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
					else if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == phase)
						Queue_ValtanPatternIdAuditionLifecycle(
							m_ValtanPatternIdAudition.bReportedWaitingForPlayer ?
							VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER :
							VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
				}
			}
			return previous->second.Result;
		}
		// Next command streams never wrap. A stale/different replay cannot replace
		// the receipt needed to answer an unconfirmed newer command exactly.
		if (request.iRequestSequence <= prior.iRequestSequence)
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
	}
	if (0u == request.iRequestSequence)
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;

	const auto evaluate = [&]() -> VALTAN_AUDITION_RESULT
	{
		CPacketWriter validation;
		if (!Write_Message(validation, request))
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		SERVER_WORLD_ENTITY* boss = Find_AuditionBoss(request.strBossPlacementId);
		if (nullptr == boss)
			return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;
		outCurrentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		if (0u == boss->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss->eAction)
			return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
		if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == request.eOperation)
			return Adopt_ValtanLiveNextPattern(sessionId, request, *boss);
		const auto& current = m_ValtanPatternIdAudition;
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE == current.ePhase ||
			request.iPredecessorRoomAuditionEpoch != current.iRoomAuditionEpoch ||
			request.iPredecessorPatternSequence != current.iExpectedPatternSequence ||
			request.strBossPlacementId != current.strBossPlacementId ||
			boss->iNetEntityId != current.iBossEntityId)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		if (sessionId != current.iOwnerSessionId)
			return VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER;
		/* Queue/replace/clear is an exact CAS on the predecessor's immutable
		   definition, not on whichever generation happens to be active now. */
		if (request.ExpectedDefinitionRevision !=
			current.PinnedDefinitionRevision)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		const bool liveOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == current.ePhase &&
			((boss->iPatternSequence == current.iExpectedPatternSequence &&
			  boss->strPatternId == current.strPatternId &&
			  boss->PinnedDefinitionRevision == current.PinnedDefinitionRevision) ||
			 Is_ValtanOutcomeFollowupInFlight(
				 *boss, current.iExpectedPatternSequence,
				 current.PinnedDefinitionRevision));
		const bool pendingOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == current.ePhase &&
			boss->iPatternSequence == current.iExpectedPatternSequence - 1u &&
			boss->PendingPatternIds.end() != std::find(
				boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(), current.strPatternId);
		const bool completedOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == current.ePhase &&
			Has_ValtanOutcomeGroupCompleted(
				*boss, current.iExpectedPatternSequence,
				current.PinnedDefinitionRevision);
		if ((!liveOccurrence && !pendingOccurrence && !completedOccurrence) ||
			boss->bMechanicLedgerRequiresReset)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		if (Is_ValtanPatternFlowRunning() || m_ValtanFightPageStart.Is_Active() ||
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		const std::uint32_t currentNextToken = m_ValtanNextPattern ?
			m_ValtanNextPattern->iRequestSequence : 0u;
		if (request.iExpectedNextRequestSequence != currentNextToken)
			return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
		if (VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID == request.eOperation)
		{
			if (!m_ValtanNextPattern ||
				request.strPatternId != m_ValtanNextPattern->strPatternId)
			{
				return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
			}
			Cancel_ValtanNextPatternReservation("cleared");
			return VALTAN_AUDITION_RESULT::CLEARED;
		}
		if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID != request.eOperation ||
			(std::numeric_limits<std::uint32_t>::max)() == current.iExpectedPatternSequence)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		const CGameplayCatalog* pinned =
			m_GameplayCatalog.Resolve(current.PinnedDefinitionRevision);
		const auto* patterns = nullptr == pinned ? nullptr :
			pinned->Find_BossPatterns(boss->strEncounterId);
		if (nullptr == patterns || patterns->end() == std::find_if(
			patterns->begin(), patterns->end(),
			[&request](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == request.strPatternId &&
					pattern.bAuthoringMasterManaged;
			}) ||
			(WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId &&
			 Is_CharacterSelectEnvironmentDependentPattern(request.strPatternId)))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		VALTAN_NEXT_PATTERN_RESERVATION reservation{};
		reservation.iOwnerSessionId = sessionId;
		reservation.iBossEntityId = boss->iNetEntityId;
		reservation.iRequestSequence = request.iRequestSequence;
		reservation.iRoomAuditionEpoch = current.iRoomAuditionEpoch;
		reservation.iPredecessorPatternSequence = current.iExpectedPatternSequence;
		reservation.iExpectedPatternSequence = current.iExpectedPatternSequence + 1u;
		reservation.strBossPlacementId = current.strBossPlacementId;
		reservation.strPatternId = request.strPatternId;
		reservation.PinnedDefinitionRevision = current.PinnedDefinitionRevision;
		// Validation is complete. Only the reservation and its two lifecycle edges
		// change; no boss, player, prop, combat object or navigation reset occurs.
		Cancel_ValtanNextPatternReservation("replaced");
		m_ValtanNextPattern = std::move(reservation);
		Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
			VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
		return VALTAN_AUDITION_RESULT::QUEUED;
	};
	const VALTAN_AUDITION_RESULT verdict = evaluate();
	m_ValtanNextPatternReceiptBySessionId.insert_or_assign(sessionId,
		VALTAN_NEXT_PATTERN_COMMAND_RECEIPT{ request, verdict, outCurrentHealthBar });
	return verdict;
}

LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Adopt_ValtanLiveNextPattern(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	if (request.iPredecessorPatternSequence != boss.iPatternSequence ||
		(0u == boss.iPatternSequence && !boss.strPatternId.empty()))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	}
	const bool hasFlow = Is_ValtanPatternFlowRunning();
	const auto& current = m_ValtanPatternIdAudition;
	if ((hasFlow && m_ValtanPatternFlowAudition.iOwnerSessionId != sessionId) ||
		(VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE != current.ePhase &&
		 INVALID_SESSION_ID != current.iOwnerSessionId && current.iOwnerSessionId != sessionId) ||
		(m_ValtanNextPattern && m_ValtanNextPattern->iOwnerSessionId != sessionId))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER;
	}
	if (m_ValtanNextPattern)
		return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == current.ePhase)
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	if (boss.PendingPatternFollowup.Is_Pending() ||
		boss.iPatternFollowupDepth > 0u)
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase ||
		m_ValtanFightPageStart.Is_Active() || 0u != m_iValtanAuditionArmedHealthBar ||
		boss.bMechanicLedgerRequiresReset || 0u == m_iNextValtanAuditionEpoch ||
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence)
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}
	const bool active = !boss.strPatternId.empty();
	if (hasFlow && (boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId ||
		boss.strPlacementId != m_ValtanPatternFlowAudition.strBossPlacementId ||
		(active && (!boss.bAutomaticPatternSequenceStepRunning ||
		 boss.strRotationId != m_ValtanPatternFlowAudition.Sequence.strSequenceId ||
		 boss.iRotationStepIndex >= m_ValtanPatternFlowAudition.Sequence.PatternIds.size() ||
		 boss.strPatternId != m_ValtanPatternFlowAudition.Sequence.PatternIds[boss.iRotationStepIndex]))))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	}
	const GameplayDataRevision pinnedRevision = hasFlow ?
		m_ValtanPatternFlowAudition.PinnedDefinitionRevision :
		(active ? boss.PinnedDefinitionRevision : m_GameplayCatalog.Get_ActiveRevision());
	if (request.ExpectedDefinitionRevision != pinnedRevision)
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	const CGameplayCatalog* pinned = m_GameplayCatalog.Resolve(pinnedRevision);
	const auto* patterns = nullptr == pinned ? nullptr :
		pinned->Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns || (active && boss.PinnedDefinitionRevision != pinnedRevision) ||
		patterns->end() == std::find_if(patterns->begin(), patterns->end(),
			[&request](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == request.strPatternId && pattern.bAuthoringMasterManaged;
			}) ||
		(active && patterns->end() == std::find_if(patterns->begin(), patterns->end(),
			[&boss](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == boss.strPatternId;
			})) ||
		(WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId &&
		 Is_CharacterSelectEnvironmentDependentPattern(request.strPatternId)))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}

	VALTAN_PATTERN_ID_AUDITION_STATE adopted{};
	adopted.ePhase = active ? VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE :
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD;
	adopted.iOwnerSessionId = sessionId;
	adopted.iBossEntityId = boss.iNetEntityId;
	adopted.iExpectedPatternSequence = boss.iPatternSequence;
	adopted.iRequestSequence = request.iRequestSequence;
	adopted.iRoomAuditionEpoch = m_iNextValtanAuditionEpoch;
	adopted.strBossPlacementId = boss.strPlacementId;
	adopted.strPatternId = boss.strPatternId;
	adopted.PinnedDefinitionRevision = pinnedRevision;
	adopted.bAdoptedLivePredecessor = true;
	if (hasFlow && active)
		adopted.AdoptedFlowSequence = m_ValtanPatternFlowAudition.Sequence;
	VALTAN_NEXT_PATTERN_RESERVATION next{};
	next.iOwnerSessionId = sessionId;
	next.iBossEntityId = boss.iNetEntityId;
	next.iRequestSequence = request.iRequestSequence;
	next.iRoomAuditionEpoch = adopted.iRoomAuditionEpoch;
	next.iPredecessorPatternSequence = boss.iPatternSequence;
	next.iExpectedPatternSequence = boss.iPatternSequence + 1u;
	next.strBossPlacementId = boss.strPlacementId;
	next.strPatternId = request.strPatternId;
	next.PinnedDefinitionRevision = pinnedRevision;

	// Admission is complete. Transfer only playback ownership: the active
	// occurrence, player, boss pose/HP and all arena runtimes remain untouched.
	if (hasFlow)
	{
		Queue_ValtanPatternFlowLifecycle(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
			&boss, "Flow remainder replaced by a live Next Pattern reservation");
		m_ValtanPatternFlowAudition = {};
	}
	m_ValtanPatternIdAudition = std::move(adopted);
	m_ValtanNextPattern = std::move(next);
	m_iNextValtanAuditionEpoch =
		(std::numeric_limits<std::uint32_t>::max)() == m_iNextValtanAuditionEpoch ?
		0u : m_iNextValtanAuditionEpoch + 1u;
	boss.bIntroPatternConsumed = true;
	boss.PendingPatternIds.clear();
	if (!active)
	{
		// Freeze the idle decision until this tick's world/prop commits finish.
		// IDLE_HOLD is admission evidence, never a fabricated COMPLETED receipt.
		boss.PinnedDefinitionRevision = pinnedRevision;
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		boss.bAutomaticPatternSequenceStepRunning = false;
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
		boss.MovePath.clear();
	}
	Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
		VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
	return VALTAN_AUDITION_RESULT::QUEUED;
}

void LostArk::Server::CGameRoom::Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	const bool idleAnchor = VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD ==
		m_ValtanPatternIdAudition.ePhase;
	if (!m_ValtanNextPattern ||
		(!idleAnchor && VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD !=
			m_ValtanPatternIdAudition.ePhase))
	{
		return;
	}
	auto& next = *m_ValtanNextPattern;
	const auto& current = m_ValtanPatternIdAudition;
	const bool completedOutcomeGroup = idleAnchor ||
		Has_ValtanOutcomeGroupCompleted(
			boss, current.iExpectedPatternSequence,
			current.PinnedDefinitionRevision);
	if (!m_PlayerIdBySessionId.contains(next.iOwnerSessionId) ||
		next.iOwnerSessionId != current.iOwnerSessionId ||
		next.iBossEntityId != boss.iNetEntityId ||
		next.strBossPlacementId != boss.strPlacementId ||
		next.iRoomAuditionEpoch != current.iRoomAuditionEpoch ||
		next.iPredecessorPatternSequence != current.iExpectedPatternSequence ||
		next.iExpectedPatternSequence <= next.iPredecessorPatternSequence ||
		next.iExpectedPatternSequence != next.iPredecessorPatternSequence + 1u ||
		next.PinnedDefinitionRevision != current.PinnedDefinitionRevision ||
		next.PinnedDefinitionRevision != boss.PinnedDefinitionRevision ||
		nullptr == m_GameplayCatalog.Resolve(next.PinnedDefinitionRevision) ||
		!boss.strPatternId.empty() || !boss.PendingPatternIds.empty() ||
		boss.bMechanicLedgerRequiresReset || !completedOutcomeGroup)
	{
		Cancel_ValtanNextPatternReservation("predecessor no longer owns the completed occurrence");
		return;
	}
	// An idle admission has no cancellable predecessor. Consume it at this
	// post-commit seam; the existing pending-Next guard waits for a player.
	if (!idleAnchor && !Has_EngagedAuditionPlayer(boss))
	{
		if (!next.bReportedWaitingForPlayer)
		{
			Queue_ValtanNextPatternLifecycle(next,
				VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
			next.bReportedWaitingForPlayer = true;
		}
		return;
	}
	/* A branch-owned child consumes its own PatternSequence values. Keep the
	   reservation's root CAS immutable while it is replaceable/waiting, then
	   rebase the consumed successor exactly once to the completed leaf. */
	next.iPredecessorPatternSequence = boss.iPatternSequence;
	next.iExpectedPatternSequence =
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence ?
			1u : boss.iPatternSequence + 1u;
	VALTAN_PATTERN_ID_AUDITION_STATE promoted{};
	promoted.ePhase = VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
	promoted.iOwnerSessionId = next.iOwnerSessionId;
	promoted.iBossEntityId = next.iBossEntityId;
	promoted.iExpectedPatternSequence = next.iExpectedPatternSequence;
	promoted.iRequestSequence = next.iRequestSequence;
	promoted.iRoomAuditionEpoch = next.iRoomAuditionEpoch;
	promoted.strBossPlacementId = next.strBossPlacementId;
	promoted.strPatternId = next.strPatternId;
	promoted.PinnedDefinitionRevision = next.PinnedDefinitionRevision;
	promoted.bResetlessContinuation = true;
	boss.PendingPatternIds.push_back(next.strPatternId);
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = false;
	m_ValtanPatternIdAudition = std::move(promoted);
	m_ValtanNextPattern.reset();
	Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
}

bool LostArk::Server::CGameRoom::Prepare_ValtanPatternIdAuditionBeforeBrain(
	SERVER_WORLD_ENTITY& boss)
{
	if (boss.iNetEntityId != m_ValtanPatternIdAudition.iBossEntityId ||
		boss.strPlacementId != m_ValtanPatternIdAudition.strBossPlacementId ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD == m_ValtanPatternIdAudition.ePhase)
		return false;
	const bool exactActiveOccurrence =
		boss.iPatternSequence ==
			m_ValtanPatternIdAudition.iExpectedPatternSequence &&
		boss.strPatternId == m_ValtanPatternIdAudition.strPatternId &&
		boss.PinnedDefinitionRevision ==
			m_ValtanPatternIdAudition.PinnedDefinitionRevision;
	const bool outcomeFollowupInFlight = Is_ValtanOutcomeFollowupInFlight(
		boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
		m_ValtanPatternIdAudition.PinnedDefinitionRevision);
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		!exactActiveOccurrence && !outcomeFollowupInFlight)
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("active audition identity was discarded before the tick");
		return false;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING != m_ValtanPatternIdAudition.ePhase ||
		!m_ValtanPatternIdAudition.bResetlessContinuation)
	{
		return true;
	}
	if (boss.iPatternSequence != m_ValtanPatternIdAudition.iExpectedPatternSequence - 1u ||
		!boss.strPatternId.empty() || boss.PendingPatternIds.end() == std::find(
			boss.PendingPatternIds.begin(), boss.PendingPatternIds.end(), m_ValtanPatternIdAudition.strPatternId))
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("pending Next identity was discarded before the tick");
		return false;
	}
	// The last target may disappear after promotion but before BeginPattern.
	// Retain the exact pending occurrence without invoking the no-target abort
	// path or allowing the ordinary Product selector to take this slot.
	if (!Has_EngagedAuditionPlayer(boss))
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		if (!m_ValtanPatternIdAudition.bReportedWaitingForPlayer)
		{
			Queue_ValtanPatternIdAuditionLifecycle(
				LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
			m_ValtanPatternIdAudition.bReportedWaitingForPlayer = true;
		}
		return false;
	}
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = false;
	m_ValtanPatternIdAudition.bReportedWaitingForPlayer = false;
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
		Cancel_ValtanPatternIdAudition("Valtan audition boss became unavailable");
		return false;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
			m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD ==
			m_ValtanPatternIdAudition.ePhase)
	{
		Try_PromoteValtanNextPattern(*boss);
		return Is_ValtanPatternIdAuditionRunning();
	}
	if (boss->bMechanicLedgerRequiresReset)
	{
		boss->bAutomaticPatternSequenceAuditionOverride = true;
		boss->bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("Valtan occurrence requires an authoritative reset");
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
				Cancel_ValtanPatternIdAudition(
					"Valtan audition definition revision changed before start");
				return false;
			}
			m_ValtanPatternIdAudition.ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE;
			Queue_ValtanPatternIdAuditionLifecycle(
				LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		}
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		Is_ValtanOutcomeFollowupInFlight(
			*boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
			m_ValtanPatternIdAudition.PinnedDefinitionRevision))
	{
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase &&
		boss->iPatternSequence == m_ValtanPatternIdAudition.iExpectedPatternSequence - 1u &&
		boss->PendingPatternIds.end() != std::find(
			boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(),
			m_ValtanPatternIdAudition.strPatternId))
	{
		m_ValtanPatternIdAudition.ePhase =
			VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
		return true;
	}

	// An empty pattern ID is not completion evidence: no-target, death and
	// authoritative discard also retire it. Only FinishPattern's exact receipt
	// may advance the chain after this tick's final world/prop/damage commits.
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		Has_ValtanOutcomeGroupCompleted(
			*boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
			m_ValtanPatternIdAudition.PinnedDefinitionRevision) &&
		!boss->bMechanicLedgerRequiresReset)
	{
		boss->iPatternFollowupDepth = 0u;
		boss->iPatternFollowupRootSequence = 0u;
		boss->bAutomaticPatternSequenceAuditionOverride = true;
		boss->bAutomaticPatternSequenceAuditionHold = true;
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
		m_ValtanPatternIdAudition.ePhase = VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD;
		Try_PromoteValtanNextPattern(*boss);
		return Is_ValtanPatternIdAuditionRunning();
	}
	boss->bAutomaticPatternSequenceAuditionOverride = true;
	boss->bAutomaticPatternSequenceAuditionHold = true;
	Cancel_ValtanPatternIdAudition("Valtan audition occurrence aborted, discarded or replaced");
	return false;
}

bool LostArk::Server::CGameRoom::Is_ValtanPatternFlowRunning() const noexcept
{
	return VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE !=
		m_ValtanPatternFlowAudition.ePhase;
}

const LostArk::Server::BOSS_PATTERN_SEQUENCE_DEFINITION*
LostArk::Server::CGameRoom::Resolve_ValtanPatternFlowSequence(
	const SERVER_WORLD_ENTITY& boss) const noexcept
{
	if (!Is_ValtanPatternFlowRunning() ||
		boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId)
	{
		const auto& current = m_ValtanPatternIdAudition;
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == current.ePhase &&
			current.bAdoptedLivePredecessor && current.AdoptedFlowSequence &&
			boss.iNetEntityId == current.iBossEntityId)
		{
			return &*current.AdoptedFlowSequence;
		}
		return nullptr;
	}
	return &m_ValtanPatternFlowAudition.Sequence;
}

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
	std::vector<std::uint8_t> payload;
	return nullptr != session &&
		Build_WorldEntitySpawnedPayload(entity, payload) &&
		session->Send_Frame(
			LostArk::Shared::PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, payload);
}

bool LostArk::Server::CGameRoom::Build_WorldEntitySpawnedPayload(
	const SERVER_WORLD_ENTITY& entity,
	std::vector<std::uint8_t>& outPayload)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.iOwnerBossNetEntityId = entity.iOwnerBossNetEntityId;
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
	message.PinnedDefinitionRevision = entity.PinnedDefinitionRevision;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	outPayload = writer.Get_Buffer();
	return true;
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
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
	message.iPredecessorRoomAuditionEpoch = request.iPredecessorRoomAuditionEpoch;
	message.iPredecessorPatternSequence = request.iPredecessorPatternSequence;
	message.iExpectedNextRequestSequence = request.iExpectedNextRequestSequence;
	message.ExpectedDefinitionRevision = request.ExpectedDefinitionRevision;
	message.ReplacementDefinitionRevision =
		request.ReplacementDefinitionRevision;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_KoukuSaydonPatternAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanPatternFlowResult(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t commandSequence,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
	const std::string& flowId,
	const std::string& flowRevision,
	const std::uint32_t roomFlowEpoch,
	const LostArk::Shared::GameplayDataRevision& pinnedRevision,
	const std::string& reason)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT message{};
	message.iCommandSequence = commandSequence;
	message.eCommand = command;
	message.eResult = result;
	message.strFlowId = flowId;
	message.strFlowRevision = flowRevision;
	message.iRoomFlowEpoch = roomFlowEpoch;
	message.PinnedDefinitionRevision = pinnedRevision;
	message.strReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT,
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
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId, reason))
		{
			session->Request_Close();
		}
	}
}

bool LostArk::Server::CGameRoom::Broadcast_CombatObjectLifecycle()
{
	using namespace LostArk::Shared;
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> presentationEvents;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	m_CombatObjectRuntime.Drain_Lifecycle(
		spawned, presentationEvents, despawned);
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
	for (const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& message :
		presentationEvents)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT,
				writer.Get_Buffer()))
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
		if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
		{
			snapshot.iAttachmentOwnerNetEntityId =
				player.iAttachmentOwnerNetEntityId;
			snapshot.eAttachmentSlot = player.eAttachmentSlot;
			snapshot.fAttachmentLocalOffsetX =
				player.fAttachmentLocalOffsetX;
			snapshot.fAttachmentLocalOffsetY =
				player.fAttachmentLocalOffsetY;
			snapshot.fAttachmentLocalOffsetZ =
				player.fAttachmentLocalOffsetZ;
			snapshot.fAttachmentYawOffsetDegrees =
				player.fAttachmentYawOffsetDegrees;
		}
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
		snapshot.isPatternBound = player.bPatternBound;
		snapshot.iPatternBindEndTick = player.iPatternBindEndTick;
		snapshot.iSilenceEndTick = player.iSilenceEndTick;
		snapshot.iSilenceDurationTicks = player.iSilenceDurationTicks;
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
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::NPC ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
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
			snapshot.iPatternTargetNetEntityId =
				entity.iPatternTargetEntityId;
			if (entity.bPortalMotionActive &&
				entity.bPortalRushTargetLocked &&
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					entity.ePatternStageMotionKind)
			{
				snapshot.PortalRushRoute.isValid = true;
				snapshot.PortalRushRoute.fStartX = entity.fPortalStartX;
				snapshot.PortalRushRoute.fStartY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fStartZ = entity.fPortalStartZ;
				snapshot.PortalRushRoute.fEndX = entity.fPortalEndX;
				snapshot.PortalRushRoute.fEndY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fEndZ = entity.fPortalEndZ;
			}
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
			if (BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
				entity.ePatternBossResponseKind)
			{
				snapshot.BossCombat.iResponseThreshold =
					entity.iPatternBossResponseThreshold;
				snapshot.BossCombat.iResponseProgress = (std::min)(
					entity.iPatternBossResponseAccumulatedHealthDamage,
					entity.iPatternBossResponseThreshold);
			}
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
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		m_Players.size() >= LostArk::Shared::MAX_VALTAN_RAID_PLAYERS)
	{
		return true;
	}
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
	SERVER_WORLD_ENTITY& outEntity,
	const CGameplayCatalog* definitionCatalog,
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId)
{
	const CGameplayCatalog& catalog = nullptr == definitionCatalog ?
		m_GameplayCatalog.Active() : *definitionCatalog;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind ||
		(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId &&
			(WORLD_BOOTSTRAP_KIND::BOSS != placement.eKind ||
			 ownerBossNetEntityId == netEntityId)))
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.iOwnerBossNetEntityId = ownerBossNetEntityId;
	staged.PinnedDefinitionRevision =
		catalog.Get_ActiveRevision();
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
	if (WORLD_BOOTSTRAP_KIND::NPC == staged.eKind)
	{
		/* NPCs are non-combat living bodies. Keep their liveness explicit so
		the body rebuild cannot silently drop them through a zero HP default.
		Their wire collision radius stays zero by Shared contract; the Server's
		blocking-body rebuild owns the separate player-sized NPC body. */
		staged.iCurrentHp = 1u;
		staged.iMaximumHp = 1u;
		staged.fCollisionRadius = 0.f;
		staged.strActionId = CNpcBehaviorRuntime::IDLE_ACTION_ID;
		staged.iActionStartTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		if (placement.bHasNpcBehavior &&
			!m_NpcBehaviorRuntime.Initialize(
				placement, m_ServerNavigation,
				staged.iActionStartTick, staged, m_strStatus))
		{
			return false;
		}
	}
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			catalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			catalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		const bool isDependentArchetype = std::any_of(patterns->begin(), patterns->end(),
			[&staged](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
					pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
			});
		if (isDependentArchetype !=
			(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId))
		{
			m_strStatus = "Boss archetype requires its declared primary/dependent spawn role";
			return false;
		}
		if (isDependentArchetype)
		{
			const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
				[ownerBossNetEntityId](const SERVER_WORLD_ENTITY& candidate)
				{ return candidate.iNetEntityId == ownerBossNetEntityId; });
			if (owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
				LostArk::Shared::INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
				0u == owner->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
				owner->bMechanicLedgerRequiresReset || owner->strEncounterId != staged.strEncounterId ||
				owner->PinnedDefinitionRevision != catalog.Get_ActiveRevision())
			{
				m_strStatus = "Dependent boss requires a live primary in its pinned encounter";
				return false;
			}
			const bool ownerRunsFinale = std::any_of(patterns->begin(), patterns->end(),
				[&owner, &staged](const BOSS_PATTERN_DEFINITION& pattern)
				{
					const bool directFinaleOccurrence =
						pattern.strPatternId == owner->strPatternId;
					const bool phaseThreeFinaleController =
						owner->bGhostPhasePatternLoopActive && 3u == owner->iPhase &&
						"BOSS_VALTAN" == owner->strArchetypeId &&
						"boss.valtan.center" == owner->strPlacementId &&
						"VALTAN_GHOST_FINALE" == pattern.strPatternId;
					return (directFinaleOccurrence || phaseThreeFinaleController) &&
						BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
						pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
				});
			if (!ownerRunsFinale)
			{
				m_strStatus = "Dependent boss owner is not running its declared finale";
				return false;
			}
		}
		const std::vector<BOSS_PART_DEFINITION>* bossParts =
			catalog.Find_BossParts(staged.strArchetypeId);
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
		WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId) || !m_Players.empty())
		return true;

	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.trigger-system");
		return false;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.spawn-groups");
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("empty-arena-reset.world-entities");
		return false;
	}
	m_CombatObjectRuntime.Reset();
	m_CombatObjectRuntime.Discard_PendingLifecycle();
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	m_iNextBossCombatEventSequence = 1u;
	m_ValtanPatternIdAuditionSequenceBySessionId.clear();
	m_ValtanPatternFlowStartSequenceBySessionId.clear();
	m_ValtanPatternFlowControlSequenceBySessionId.clear();
#ifdef _DEBUG
	Clear_KoukuSaydonPatternAudition();
	m_KoukuSaydonPatternAuditionReceiptBySessionId.clear();
	m_PendingKoukuSaydonPatternAuditionLifecycle.clear();
	Cancel_ValtanPatternIdAudition("room reset after the last player left");
	m_ValtanNextPatternReceiptBySessionId.clear();
	m_ValtanPatternFlowAudition = {};
	m_ValtanFightPageStart = {};
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
		Mark_RuntimeFailure("valtan-empty-reset.spawn-groups");
		return false;
	}
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
#ifdef _DEBUG
	m_ValtanTimelineAudition = {};
	m_ValtanFightPageStart = {};
#endif
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!m_EncounterPropRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.encounter-props");
		return false;
	}
	if (!m_WorldDestructionRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.world-destruction");
		return false;
	}
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iNextBossCombatEventSequence = 1u;
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("valtan-empty-reset.world-entities");
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
	m_bValtanRaidCleared = false;
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
				const ENCOUNTER_PROP_PREPARE_RESULT breakResult =
					m_EncounterPropRuntime.Prepare_BreakSlots(
						propBreakStage.PropBreakSlotIds,
						m_EncounterPropRuntime.Get_OccurrenceSequence(),
						serverTick, breakTransaction, breakStatus);
				if (ENCOUNTER_PROP_PREPARE_RESULT::READY == breakResult)
				{
					if (!m_EncounterPropRuntime.Commit(breakTransaction, breakStatus))
					{
						m_strStatus = std::move(breakStatus);
						return false;
					}
					Broadcast_EncounterPropSync();
				}
				else if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE != breakResult)
				{
					m_strStatus = std::move(breakStatus);
					return false;
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
		/* A new occurrence cannot claim slots that the previous one still owns.
		   Preserve the prop failure so this tick cannot publish completion or
		   promote Next after an uncommitted stage entry. */
		m_strStatus = std::move(status);
		return false;
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
	const std::uint32_t spawnWaveOrdinal,
	const bool scheduledSpawnWave)
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
		stagedCombat, stagedGameplayPhase, transaction, spawnWaveOrdinal,
		scheduledSpawnWave))
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
	if (!scheduledSpawnWave && !Commit_BossPatternPlayerStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger,
		serverTick, spawnWaveOrdinal))
	{
		m_strStatus = "Boss player stage action commit failed";
		return false;
	}
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
			(0u == action.Volley.iFirstSpawnOffsetMs &&
			 action.Volley.iSpawnCount <= 1u))
		{
			continue;
		}
		if (nullptr != scheduledAction &&
			(scheduledAction->Volley.iFirstSpawnOffsetMs !=
				action.Volley.iFirstSpawnOffsetMs ||
			 scheduledAction->Volley.iSpawnCount != action.Volley.iSpawnCount ||
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
	if (0u == boss.iAppliedPatternStageSpawnWaveCount &&
		0u == scheduledAction->Volley.iFirstSpawnOffsetMs)
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
		static_cast<std::uint64_t>(scheduledAction->Volley.iFirstSpawnOffsetMs) +
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
		serverTick, waveOrdinal, true))
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
	std::uint32_t nextStageAppliedSpawnWaveCount =
		nextActionId.empty() ? 0u : 1u;
	if (!nextActionId.empty())
	{
		const auto* nextPatterns = nextCatalog->Find_BossPatterns(
			boss.strEncounterId);
		if (nullptr == nextPatterns)
		{
			m_strStatus = "Boss next-stage scheduled volley encounter is missing";
			return false;
		}
		const auto nextPattern = std::find_if(
			nextPatterns->begin(), nextPatterns->end(),
			[&nextPatternId](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == nextPatternId; });
		if (nextPatterns->end() == nextPattern)
		{
			m_strStatus = "Boss next-stage scheduled volley pattern is missing";
			return false;
		}
		const auto nextStage = std::find_if(
			nextPattern->Stages.begin(), nextPattern->Stages.end(),
			[&nextActionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
			{ return candidate.strActionId == nextActionId; });
		if (nextPattern->Stages.end() == nextStage)
		{
			m_strStatus = "Boss next-stage scheduled volley owner is missing";
			return false;
		}
		for (const BOSS_PATTERN_STAGE_ACTION& action : nextStage->Actions)
		{
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
					action.eKind ||
				(0u == action.Volley.iFirstSpawnOffsetMs &&
				 action.Volley.iSpawnCount <= 1u))
			{
				continue;
			}
			nextStageAppliedSpawnWaveCount =
				0u == action.Volley.iFirstSpawnOffsetMs ? 1u : 0u;
			break;
		}
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
	/* A delayed first wave is intentionally absent from the ENTER transaction;
	   ordinal zero remains pending until its exact fixed-tick due time. */
	boss.iAppliedPatternStageSpawnWaveCount = nextStageAppliedSpawnWaveCount;
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
	if (!Commit_BossPatternPlayerStageActions(
			boss, *previousCatalog, previousPatternId, previousActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick) ||
		!Commit_BossPatternPlayerStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick))
	{
		m_strStatus = "Boss player stage action commit failed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
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
	const std::uint32_t spawnWaveOrdinal,
	const bool scheduledSpawnWave)
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
	if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		stage == pattern->Stages.begin() && pattern->Motion.bMoveToAnchorBeforeTakeoff)
	{
		SERVER_NAV_POINT anchor{};
		if (!m_ServerNavigation.Is_PointWalkableExact(
			pattern->Motion.fLandingX, pattern->Motion.fLandingZ) ||
			!m_ServerNavigation.Sample_Position(
				pattern->Motion.fLandingX, pattern->Motion.fLandingZ, anchor) ||
			std::fabs(anchor.y - pattern->Motion.fLandingY) > 1.5f)
		{
			m_strStatus = "Pre-takeoff arena anchor is not on the live arena deck";
			return false;
		}
	}
	const BOSS_PATTERN_STAGE_ACTION* scheduledVolleyClock = nullptr;
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
				action.eKind)
		{
			continue;
		}
		const std::uint64_t lastSpawnOffsetMs =
			static_cast<std::uint64_t>(action.Volley.iFirstSpawnOffsetMs) +
			static_cast<std::uint64_t>(0u == action.Volley.iSpawnCount ? 0u :
				action.Volley.iSpawnCount - 1u) *
				action.Volley.iSpawnIntervalMs;
		if (0u == action.Volley.iSpawnCount ||
			lastSpawnOffsetMs >= stage->iDurationMs)
		{
			m_strStatus = "Boss scheduled volley leaves its stage clock";
			return false;
		}
		if (0u == action.Volley.iFirstSpawnOffsetMs &&
			action.Volley.iSpawnCount <= 1u)
		{
			continue;
		}
		if (nullptr != scheduledVolleyClock &&
			(scheduledVolleyClock->Volley.iFirstSpawnOffsetMs !=
				action.Volley.iFirstSpawnOffsetMs ||
			 scheduledVolleyClock->Volley.iSpawnCount !=
				action.Volley.iSpawnCount ||
			 scheduledVolleyClock->Volley.iSpawnIntervalMs !=
				action.Volley.iSpawnIntervalMs))
		{
			m_strStatus = "Boss scheduled volleys do not share one clock";
			return false;
		}
		scheduledVolleyClock = &action;
	}
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (action.eTrigger != trigger)
			continue;
		const bool isTypedVolley =
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				action.eKind;
		const bool isScheduledVolley = isTypedVolley &&
			(0u != action.Volley.iFirstSpawnOffsetMs ||
			 action.Volley.iSpawnCount > 1u);
		if (scheduledSpawnWave &&
			(!isScheduledVolley ||
			 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
			 spawnWaveOrdinal >= action.Volley.iSpawnCount))
		{
			continue;
		}
		if (!scheduledSpawnWave && isTypedVolley &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
			0u != action.Volley.iFirstSpawnOffsetMs)
		{
			continue;
		}
		if (0u != action.iDurationMs &&
			BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS !=
				action.eKind &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND != action.eKind &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE != action.eKind)
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
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND:
		{
			const bool entering =
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger;
			if ("player.status.bind" != action.strTargetId ||
				(entering ?
					(5000u != action.iValue || action.iDurationMs < 100u ||
					 action.iDurationMs > 120000u) :
					(0u != action.iValue || 0u != action.iDurationMs)))
			{
				m_strStatus = "Boss player-bind stage action is invalid";
				return false;
			}
			if (!entering)
				break;
			const auto target = std::find_if(
				m_Players.begin(), m_Players.end(),
				[&boss](const auto& entry)
				{
					return entry.second.iNetEntityId ==
						boss.iPatternTargetEntityId;
				});
			if (m_Players.end() == target ||
				INVALID_NET_ENTITY_ID == boss.iPatternTargetEntityId ||
				0u == target->second.iCurrentHp ||
				!target->second.isCombatReady || target->second.bPatternBound ||
				PLAYER_ACTION_STATE::DEAD == target->second.eAction ||
				PLAYER_ACTION_STATE::FALLING == target->second.eAction ||
				PLAYER_ACTION_STATE::GRABBED == target->second.eAction ||
				!m_ServerNavigation.Is_Loaded() ||
				!m_ServerNavigation.Is_PointWalkableExact(
					target->second.fPositionX, target->second.fPositionZ))
			{
				m_strStatus = "Boss player-bind target is not an admitted alive target";
				return false;
			}
			SERVER_NAV_POINT ground{};
			if (!m_ServerNavigation.Sample_Position(
				target->second.fPositionX, target->second.fPositionZ, ground) ||
				std::abs(ground.y - target->second.fPositionY) > 1.5f ||
				!std::isfinite(target->second.fPositionY + 5.f))
			{
				m_strStatus = "Boss player-bind restore pose is not navigable";
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE:
		{
			if ("player.status.silence" != action.strTargetId ||
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				1u != action.iValue || action.iDurationMs < stage->iDurationMs ||
				action.iDurationMs < 100u || action.iDurationMs > 120000u)
			{
				m_strStatus = "Boss player-silence stage action is invalid";
				return false;
			}
			break;
		}
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
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
					!(("VALTAN_ARENA_BREAK_109" == patternId &&
					   "valtan.mechanic.arena-break-109.impact" == actionId &&
					   2u == action.iValue) ||
					  ("VALTAN_GHOST_RESPAWN_AUDITION" == patternId &&
					   "valtan.sequence.respawn.step-01" == actionId &&
					   3u == action.iValue))) ||
				(isValtan &&
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger) ||
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
			const bool perAlivePlayerVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
					action.Volley.ePolicy;
			const bool bossRelativeVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
					action.Volley.ePolicy;
			const bool arenaCenterVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER ==
					action.Volley.ePolicy;
			const bool navIndependentPortalVolley =
				"BOSS_VALTAN" == boss.strArchetypeId &&
				bossRelativeVolley &&
				"VALTAN_GHOST_PORTAL_ONCE" == patternId &&
				"valtan.ghost.portal-once.active" == actionId &&
				"combatobject.valtan.ghost.portal-charge" ==
					definition->strCombatObjectArchetypeId;
			if (isTypedVolley &&
				((!perAlivePlayerVolley && !bossRelativeVolley &&
				  !arenaCenterVolley) ||
				 (perAlivePlayerVolley &&
					BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
						LOCKED_TARGET_PER_ALIVE_PLAYER !=
						definition->eOriginPolicy) ||
				 ((bossRelativeVolley || arenaCenterVolley) &&
					BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
						definition->eOriginPolicy) ||
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
			if (bossRelativeVolley || arenaCenterVolley)
			{
				const std::uint32_t count =
					action.Volley.iCountPerResolvedTarget;
				/* ARENA_CENTER anchors on the boss spawn placement (the authored arena
				   centre) with world-absolute angles; BOSS_RELATIVE keeps the live boss
				   pose and yaw. Stage_BossCombatObject resolves the same origin. */
				const float volleyOriginX =
					arenaCenterVolley ? boss.fSpawnPositionX : boss.fPositionX;
				const float volleyOriginZ =
					arenaCenterVolley ? boss.fSpawnPositionZ : boss.fPositionZ;
				const float volleyYawBasisDegrees =
					arenaCenterVolley ? 0.f : boss.fYawDegrees;
				/* The four-rock presentation volleys may straddle the arena boundary when
				   an owner starts its authored radial set near an edge. Part Break is
				   expected to begin at the charge wall, while Six Pizza and Struggling
				   intentionally preserve their centered corner layouts. Keep the exception
				   tied to the exact owner IDs and an empty damage list; adding gameplay hits
				   makes navigation admission strict again. */
				const bool visualCardinalRocksMayStartOffNavigation =
					BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == definition->eKind &&
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
						definition->eDirectionPolicy && definition->Hits.empty() &&
					!definition->PresentationPulses.empty() &&
					(("combatobject.valtan.ground-roar.rock" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_GROUND_ROAR" == patternId &&
					  "valtan.sequence.sequence.400440.0.step-01" == actionId) ||
					 ("combatobject.valtan.part-break.rock" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_PART_BREAK" == patternId &&
					  "valtan.reaction.part-break.recovery" == actionId) ||
					 ("combatobject.valtan.six-pizza.rock-pillar" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_SIX_PIZZA_106" == patternId &&
					  "valtan.sequence.center-six-pizza-charge.step-01" == actionId) ||
					 ("combatobject.valtan.struggling.rock-pillar" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_STRUGGLING" == patternId &&
					  "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04" ==
						actionId)) &&
					4u == count;
				/* The exact ghost-portal owner is a world-transform rush rather than
				a walking actor. Unlike the presentation-only rock exception, it may
				own gameplay hits while its authored vertices cross missing nav. */
				const bool authoredVolleyMayStartOffNavigation =
					visualCardinalRocksMayStartOffNavigation ||
					navIndependentPortalVolley;
				const bool damagingCoverVolleyMayProject =
					BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == definition->eKind &&
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
						definition->eDirectionPolicy &&
					definition->fCoverRadiusM > 0.f && !definition->Hits.empty();
				if (count < 2u || count > 8u || action.iValue != count ||
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL !=
						action.Volley.eLayout ||
					action.Volley.fRadiusM <= 0.f ||
					action.Volley.bAllowOverlap ||
					1u != action.Volley.iSpawnCount ||
					0u != action.Volley.iArenaRandomCount ||
					(!navIndependentPortalVolley &&
					 !m_ServerNavigation.Is_Loaded()))
				{
					m_strStatus =
						"Boss-relative combat object volley is invalid";
					return false;
				}
				std::array<std::pair<float, float>, 8u> points{};
				std::array<SERVER_NAV_POINT, 8u> resolvedPoints{};
				bool skipVolley = false;
				for (std::uint32_t ordinal = 0u; ordinal < count; ++ordinal)
				{
					const float degrees = volleyYawBasisDegrees +
						action.Volley.fStartAngleDegrees +
						action.Volley.fAngleStepDegrees *
							static_cast<float>(ordinal);
					const float radians = degrees * DEGREES_TO_RADIANS;
					const float x = volleyOriginX +
						std::sin(radians) * action.Volley.fRadiusM;
					const float z = volleyOriginZ +
						std::cos(radians) * action.Volley.fRadiusM;
					SERVER_NAV_POINT resolvedPoint{
						x, arenaCenterVolley ? boss.fSpawnPositionY : boss.fPositionY, z };
					if (std::isfinite(x) && std::isfinite(z) &&
						damagingCoverVolleyMayProject &&
						!m_ServerNavigation.Is_PointWalkableExact(x, z))
					{
						SERVER_NAV_POINT projected{};
						const bool hasProjection =
							m_ServerNavigation.Project_PointOnSameLevel(x, z, projected);
						float projectionDistance = hasProjection ?
							std::hypot(projected.x - x, projected.z - z) :
							(std::numeric_limits<float>::max)();
						constexpr float MAX_COVER_PROJECTION_METERS = 2.f;
						/* Project_PointOnSameLevel returns a walkable cell centre. When
						   the centre is just beyond the authored two-metre cap, retain
						   the same cell but move toward the authored point until the
						   world displacement is exactly two metres. This removes grid
						   centre quantization without admitting a farther cell. */
						if (hasProjection &&
							projectionDistance > MAX_COVER_PROJECTION_METERS)
						{
							const float ratio = MAX_COVER_PROJECTION_METERS /
								projectionDistance;
							const float boundedX = x + (projected.x - x) * ratio;
							const float boundedZ = z + (projected.z - z) * ratio;
							if (m_ServerNavigation.Is_PointWalkableExact(boundedX, boundedZ))
							{
								projected.x = boundedX;
								projected.z = boundedZ;
								projectionDistance = MAX_COVER_PROJECTION_METERS;
							}
						}
						if (!hasProjection || projectionDistance > 2.f)
						{
							m_strStatus =
								"Damaging cover volley has no nearby navigation projection: patternId=" +
								patternId + " actionId=" + actionId + " combatObject=" +
								definition->strCombatObjectArchetypeId + " ordinal=" +
								std::to_string(ordinal) + " authoredX=" +
								std::to_string(x) + " authoredZ=" + std::to_string(z) +
								" projectedX=" + std::to_string(projected.x) +
								" projectedZ=" + std::to_string(projected.z) +
								" projectionDistance=" +
								std::to_string(projectionDistance);
							std::cerr << "[DamagingCoverVolleySkipped] " << m_strStatus
								<< '\n';
							skipVolley = true;
							break;
						}
						resolvedPoint = projected;
					}
					if (!std::isfinite(resolvedPoint.x) ||
						!std::isfinite(resolvedPoint.y) ||
						!std::isfinite(resolvedPoint.z) ||
						(!authoredVolleyMayStartOffNavigation &&
						 !m_ServerNavigation.Is_PointWalkableExact(
							 resolvedPoint.x, resolvedPoint.z)))
					{
						m_strStatus =
							"Boss-relative combat object leaves navigable arena: patternId=" +
							patternId + " actionId=" + actionId + " combatObject=" +
							definition->strCombatObjectArchetypeId + " ordinal=" +
							std::to_string(ordinal) + " bossX=" +
							std::to_string(boss.fPositionX) + " bossZ=" +
							std::to_string(boss.fPositionZ) + " spawnX=" +
							std::to_string(x) + " spawnZ=" + std::to_string(z);
						/* A presentation-only object (no gameplay hit, only pulses) never
						   owns damage or navigation authority, so one authored root that
						   lands inside a still-standing wall must not latch the whole room
						   into a runtime failure and close every session. The wave is
						   skipped without staging anything, the exact diagnostic stays in
						   m_strStatus and the Server log, and the wave counter still
						   advances so the skip is not retried every tick. Definitions with
						   any hit keep the strict room-failure path. */
						if (std::isfinite(resolvedPoint.x) &&
							std::isfinite(resolvedPoint.z) &&
							definition->Hits.empty() &&
							!definition->PresentationPulses.empty())
						{
							std::cerr << "[PresentationVolleySkipped] " << m_strStatus
								<< '\n';
							skipVolley = true;
							break;
						}
						return false;
					}
					for (std::uint32_t existingOrdinal = 0u;
						existingOrdinal < ordinal; ++existingOrdinal)
					{
						const auto& [existingX, existingZ] =
							points[existingOrdinal];
						const float deltaX = resolvedPoint.x - existingX;
						const float deltaZ = resolvedPoint.z - existingZ;
						if (deltaX * deltaX + deltaZ * deltaZ <=
							VOLLEY_SPACING_EPSILON)
						{
							m_strStatus =
								"Boss-relative combat object positions overlap";
							return false;
						}
					}
					points[ordinal] = { resolvedPoint.x, resolvedPoint.z };
					resolvedPoints[ordinal] = resolvedPoint;
				}
				if (skipVolley)
				{
					break;
				}
				const std::size_t firstStagedObject =
					combatObjectTransaction.Objects.size();
				const std::size_t firstStagedSpawn =
					combatObjectTransaction.Spawned.size();
				if (!m_CombatObjectRuntime.Stage_BossCombatObject(
					combatObjectTransaction, boss, nullptr, *definition,
					&action.Volley, catalog, count, serverTick, m_strStatus))
				{
					return false;
				}
				if (combatObjectTransaction.Objects.size() !=
						firstStagedObject + count ||
					combatObjectTransaction.Spawned.size() !=
						firstStagedSpawn + count)
				{
					m_strStatus = "Boss-relative combat object staging count is invalid";
					return false;
				}
				for (std::uint32_t ordinal = 0u; ordinal < count; ++ordinal)
				{
					SERVER_COMBAT_OBJECT& staged =
						combatObjectTransaction.Objects[firstStagedObject + ordinal];
					staged.LiveState.CurrentPose.fPositionX =
						resolvedPoints[ordinal].x;
					staged.LiveState.CurrentPose.fPositionY =
						resolvedPoints[ordinal].y;
					staged.LiveState.CurrentPose.fPositionZ =
						resolvedPoints[ordinal].z;
					staged.LiveState.PreviousPose = staged.LiveState.CurrentPose;
					auto& spawned =
						combatObjectTransaction.Spawned[firstStagedSpawn + ordinal];
					spawned.fPositionX = resolvedPoints[ordinal].x;
					spawned.fPositionY = resolvedPoints[ordinal].y;
					spawned.fPositionZ = resolvedPoints[ordinal].z;
				}
				break;
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
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER:
		{
			SERVER_NAV_POINT center{};
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				"boss.arena.center" != action.strTargetId || 1u != action.iValue ||
				!Resolve_ArenaCenter(boss, center))
				return false;
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE:
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				"boss.target.pattern" != action.strTargetId ||
				1u != action.iValue || 0u != action.iDurationMs ||
				BOSS_GRABBED_RELEASE_MODE::NONE != action.eReleaseMode ||
				0.f != action.fReleaseSpeedMps ||
				0.f != action.fReleaseYawOffsetDegrees)
			{
				m_strStatus = "Boss retarget stage action is invalid";
				return false;
			}
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT:
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT != trigger ||
				"boss.sequence.inter-step-pursuit" != action.strTargetId ||
				0u != action.iValue || 0u != action.iDurationMs ||
				"VALTAN_GHOST_DEATH_AUDITION" != patternId ||
				"valtan.sequence.dead.step-01" != actionId ||
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED !=
					boss.PatternTerminalReceipt.eResult ||
				0u == boss.iPatternSequence ||
				boss.PatternTerminalReceipt.iPatternSequence != boss.iPatternSequence)
			{
				m_strStatus =
					"Boss inter-step pursuit suppression edge is invalid";
				return false;
			}
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS:
		{
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			std::vector<LostArk::Shared::DAMAGE_EVENT> stagedDamageEvents;
			if (!Prepare_GrabbedPlayerImpact(
				boss, catalog, action, serverTick, stagedPlayers, stagedDamageEvents))
			{
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS:
		{
			const bool hold = BOSS_GRABBED_RELEASE_MODE::HOLD ==
				action.eReleaseMode && 0.f == action.fReleaseSpeedMps &&
				0u == action.iDurationMs &&
				0.f == action.fReleaseYawOffsetDegrees;
			const bool knockback =
				(BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK ==
					action.eReleaseMode ||
				 BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode) &&
					action.fReleaseSpeedMps > 0.f &&
					action.fReleaseSpeedMps <= 50.f &&
					action.iDurationMs > 0u && action.iDurationMs <= 5000u &&
					(BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION ==
						action.eReleaseMode || 0.f == action.fReleaseYawOffsetDegrees);
			if ("boss.attachment.left-hand" != action.strTargetId ||
				0u != action.iValue || !std::isfinite(action.fReleaseSpeedMps) ||
				!std::isfinite(action.fReleaseYawOffsetDegrees) ||
				std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
				(!hold && !knockback))
			{
				m_strStatus = "Boss grabbed-player release action is invalid";
				return false;
			}
			for (const auto& [playerId, player] : m_Players)
			{
				(void)playerId;
				if (player.iAttachmentOwnerNetEntityId != boss.iNetEntityId) continue;
				SERVER_PLAYER staged = player;
				const float distance = knockback ? action.fReleaseSpeedMps *
					(static_cast<float>(action.iDurationMs) / 1000.f) : 0.f;
				const bool prepared =
					BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode ?
					Prepare_ArenaEjection(staged, boss, action, serverTick) :
					Release_PlayerAttachment(staged, boss.iNetEntityId, distance,
						action.iDurationMs, false, 0u, serverTick);
				if (!prepared)
				{
					m_strStatus = "Boss grabbed-player release target is invalid";
					return false;
				}
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

bool LostArk::Server::CGameRoom::Prepare_GrabbedPlayerImpact(
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents)
{
	using namespace LostArk::Shared;
	stagedPlayers.clear();
	stagedDamageEvents.clear();
	const bool execution = BOSS_PATTERN_STAGE_ACTION_KIND::
		EXECUTE_GRABBED_PLAYERS == action.eKind;
	const bool damage = BOSS_PATTERN_STAGE_ACTION_KIND::
		DAMAGE_GRABBED_PLAYERS == action.eKind;
	if ((!execution && !damage) || 0u == serverTick ||
		0u == boss.iPatternSequence || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction ||
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
		0u != action.iValue || 0u != action.iDurationMs ||
		BOSS_GRABBED_RELEASE_MODE::NONE != action.eReleaseMode ||
		0.f != action.fReleaseSpeedMps ||
		0.f != action.fReleaseYawOffsetDegrees ||
		(execution && "boss.attachment.left-hand" != action.strTargetId))
	{
		m_strStatus = "Grabbed-player impact action or owner is invalid";
		return false;
	}
	const SERVER_BOSS_GRAB_ROSTER roster =
		CValtanBrain::Classify_GrabbedPlayers(boss, m_Players);
	if (execution && SERVER_BOSS_GRAB_CLASSIFICATION::ALL != roster.eClassification)
	{
		m_strStatus = "Grab execution requires every living participant in this occurrence";
		return false;
	}
	if (m_TickDamageEvents.size() > MAX_DAMAGE_EVENTS ||
		roster.iGrabbedCount > MAX_DAMAGE_EVENTS - m_TickDamageEvents.size())
	{
		m_strStatus = "Grabbed-player impact damage event capacity is exhausted";
		return false;
	}
	const BOSS_RUNTIME_PROFILE* bossProfile = catalog.Find_Boss(boss.strArchetypeId);
	const std::uint32_t rate = damage ?
		catalog.Find_DamageRatePercent(action.strTargetId) : 0u;
	if (damage && (nullptr == bossProfile || 0u == rate))
	{
		m_strStatus = "Grabbed-player impact damage definition is missing";
		return false;
	}
	const std::uint32_t rawDamage = damage ?
		CGameplayCatalog::Resolve_Damage(bossProfile->iAttackPower, rate) : 0u;
	for (const auto& [playerId, player] : m_Players)
	{
		if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
			player.iAttachmentOwnerNetEntityId != boss.iNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != player.eAttachmentSlot ||
			player.iAttachmentPatternSequence != boss.iPatternSequence ||
			0u == player.iCurrentHp)
		{
			continue;
		}
		const auto identity = m_PlayerIdByEntityId.find(player.iNetEntityId);
		const PLAYER_RUNTIME_PROFILE* profile = catalog.Find_Player(player.eCharacterClass);
		if (INVALID_NET_ENTITY_ID == player.iNetEntityId ||
			m_PlayerIdByEntityId.end() == identity || identity->second != playerId ||
			player.iPlayerId != playerId || nullptr == profile ||
			!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionY) ||
			!std::isfinite(player.fPositionZ) || !std::isfinite(player.fYawDegrees))
		{
			m_strStatus = "Grabbed-player impact target identity or transform is invalid";
			return false;
		}
		SERVER_PLAYER staged = player;
		const std::uint32_t amount = execution ? player.iCurrentHp :
			(std::min)(player.iCurrentHp,
				CGameplayCatalog::Apply_Defense(rawDamage, profile->iDefense));
		staged.iCurrentHp -= amount;
		if (!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
		{
			m_strStatus = "Grabbed-player impact detach could not be prepared";
			return false;
		}
		staged.iActionStartTick = 0u == staged.iCurrentHp ? serverTick : 0u;
		staged.Projectiles.clear();
		staged.iSpawnedProjectileMask = 0u;
		staged.iAppliedHitMask = 0u;
		staged.hasAppliedSkillDamage = false;
		staged.fFallVelocityY = 0.f;
		staged.iFallDeathTick = 0u;
		stagedPlayers.emplace(playerId, std::move(staged));
		DAMAGE_EVENT event{};
		event.iTargetNetEntityId = player.iNetEntityId;
		event.iAmount = amount;
		event.fPositionX = player.fPositionX;
		event.fPositionY = player.fPositionY;
		event.fPositionZ = player.fPositionZ;
		event.isOutgoing = false;
		stagedDamageEvents.push_back(event);
	}
	if (stagedPlayers.size() != roster.iGrabbedCount)
	{
		m_strStatus = "Grabbed-player impact roster changed during preparation";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Commit_BossPatternPlayerStageActions(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	const std::uint32_t spawnWaveOrdinal)
{
	using namespace LostArk::Shared;
	if (patternId.empty() || actionId.empty() || spawnWaveOrdinal > 0u)
		return true;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
		return false;
	const auto pattern = std::find_if(
		patterns->begin(), patterns->end(),
		[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
		{
			return candidate.strPatternId == patternId;
		});
	if (patterns->end() == pattern)
		return false;
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&actionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{
			return candidate.strActionId == actionId;
		});
	if (pattern->Stages.end() == stage)
		return false;

	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (action.eTrigger != trigger)
			continue;
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT ==
			action.eKind)
		{
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT != trigger ||
				"boss.sequence.inter-step-pursuit" != action.strTargetId ||
				0u != action.iValue || 0u != action.iDurationMs ||
				"VALTAN_GHOST_DEATH_AUDITION" != patternId ||
				"valtan.sequence.dead.step-01" != actionId ||
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED !=
					boss.PatternTerminalReceipt.eResult ||
				boss.PatternTerminalReceipt.iPatternSequence != boss.iPatternSequence)
			{
				return false;
			}
			/* FinishPattern already advanced the ordered cursor and reserved the
			default delay. Consume only that delay; Debug COMPLETED_HOLD and the
			pattern-flow lifecycle remain owned by their existing state machines. */
			boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER == action.eKind)
		{
			SERVER_NAV_POINT center{};
			if (!Resolve_ArenaCenter(boss, center))
				return false;
			boss.fPositionX = center.x;
			boss.fPositionY = center.y;
			boss.fPositionZ = center.z;
			boss.MovePath.clear();
			boss.PatternStageRootMotion.clear();
			boss.fPatternForcedMotionSpeed = 0.f;
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE ==
			action.eKind)
		{
			std::vector<SERVER_PLAYER*> candidates;
			candidates.reserve(m_Players.size());
			for (auto& [playerId, player] : m_Players)
			{
				(void)playerId;
				if (0u == player.iCurrentHp || !player.isCombatReady ||
					PLAYER_ACTION_STATE::GRABBED == player.eAction ||
					PLAYER_ACTION_STATE::DEAD == player.eAction ||
					PLAYER_ACTION_STATE::FALLING == player.eAction)
				{
					continue;
				}
				candidates.push_back(&player);
			}
			if (candidates.empty())
			{
				boss.iPatternTargetEntityId = INVALID_NET_ENTITY_ID;
				boss.bHasPatternTargetLastPosition = false;
				continue;
			}
			const std::uint64_t seed = Mix_DeterministicRandom(
				Hash_StableId(actionId) ^ Hash_StableId(action.strTargetId) ^
				(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
				static_cast<std::uint64_t>(boss.iPatternSequence) ^
				(static_cast<std::uint64_t>(serverTick) << 1u));
			SERVER_PLAYER& selected = *candidates[
				static_cast<std::size_t>(seed % candidates.size())];
			boss.iTargetEntityId = selected.iNetEntityId;
			boss.iPatternTargetEntityId = selected.iNetEntityId;
			boss.bHasPatternTargetLastPosition = true;
			boss.fPatternTargetLastPositionX = selected.fPositionX;
			boss.fPatternTargetLastPositionY = selected.fPositionY;
			boss.fPatternTargetLastPositionZ = selected.fPositionZ;
			if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
				stage->Motion.eKind)
			{
				const float deltaX = selected.fPositionX - boss.fPositionX;
				const float deltaZ = selected.fPositionZ - boss.fPositionZ;
				if (deltaX * deltaX + deltaZ * deltaZ > 0.000001f)
				{
					boss.fYawDegrees =
						std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
				}
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger)
			{
				const auto target = std::find_if(
					m_Players.begin(), m_Players.end(),
					[&boss](const auto& entry)
					{
						return entry.second.iNetEntityId ==
							boss.iPatternTargetEntityId;
					});
				if (m_Players.end() == target)
					return false;
				SERVER_PLAYER staged = target->second;
				const float restoreX = staged.fPositionX;
				const float restoreY = staged.fPositionY;
				const float restoreZ = staged.fPositionZ;
				const float restoreYaw = staged.fYawDegrees;
				const bool restoreCombatReady = staged.isCombatReady;
				Cancel_PlayerActionForPatternStatus(staged);
				staged.bPatternBound = true;
				staged.iPatternBindOwnerNetEntityId = boss.iNetEntityId;
				staged.iPatternBindSequence = boss.iPatternSequence;
				staged.iPatternBindEndTick = Add_ServerTicksSkippingReservedZero(
					0u == serverTick ? 1u : serverTick,
					DurationMillisecondsToServerTicks(action.iDurationMs));
				staged.fPatternBindRestoreX = restoreX;
				staged.fPatternBindRestoreY = restoreY;
				staged.fPatternBindRestoreZ = restoreZ;
				staged.fPatternBindRestoreYawDegrees = restoreYaw;
				staged.bPatternBindRestoreCombatReady = restoreCombatReady;
				staged.fPositionY = restoreY +
					static_cast<float>(action.iValue) / 1000.f;
				stagedPlayers.emplace(target->first, std::move(staged));
			}
			else
			{
				for (const auto& [playerId, player] : m_Players)
				{
					if (!player.bPatternBound ||
						player.iPatternBindOwnerNetEntityId != boss.iNetEntityId ||
						player.iPatternBindSequence != boss.iPatternSequence)
					{
						continue;
					}
					SERVER_PLAYER staged = player;
					if (!Restore_PatternBoundPlayer(staged))
						return false;
					stagedPlayers.emplace(playerId, std::move(staged));
				}
			}
			for (auto& [playerId, staged] : stagedPlayers)
			{
				m_CombatObjectRuntime.Cancel_Source(staged.iNetEntityId);
				m_Players.at(playerId) = std::move(staged);
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			for (const auto& [playerId, player] : m_Players)
			{
				if (0u == player.iCurrentHp ||
					PLAYER_ACTION_STATE::DEAD == player.eAction)
				{
					continue;
				}
				SERVER_PLAYER staged = player;
				staged.iSilenceOwnerNetEntityId = boss.iNetEntityId;
				staged.iSilencePatternSequence = boss.iPatternSequence;
				staged.iSilenceEndTick = Add_ServerTicksSkippingReservedZero(
					0u == serverTick ? 1u : serverTick,
					DurationMillisecondsToServerTicks(action.iDurationMs));
				staged.iSilenceDurationTicks =
					DurationMillisecondsToServerTicks(action.iDurationMs);
				if (PLAYER_PENDING_COMMAND_KIND::SKILL ==
					staged.PendingCommand.eKind)
				{
					staged.PendingCommand.Clear();
				}
				stagedPlayers.emplace(playerId, std::move(staged));
			}
			for (auto& [playerId, staged] : stagedPlayers)
				m_Players.at(playerId) = std::move(staged);
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
			BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			std::vector<DAMAGE_EVENT> stagedDamageEvents;
			if (!Prepare_GrabbedPlayerImpact(
				boss, catalog, action, serverTick, stagedPlayers, stagedDamageEvents))
			{
				return false;
			}
			/* Allocate before publishing any player copy. The remaining moves and
			POD event copies cannot expose a partially executed roster. */
			m_TickDamageEvents.reserve(m_TickDamageEvents.size() + stagedDamageEvents.size());
			for (auto& [playerId, staged] : stagedPlayers)
			{
				m_CombatObjectRuntime.Cancel_Source(staged.iNetEntityId);
				m_Players.at(playerId) = std::move(staged);
			}
			m_TickDamageEvents.insert(m_TickDamageEvents.end(),
				stagedDamageEvents.begin(), stagedDamageEvents.end());
			if (BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind)
			{
				boss.iGrabExecutionCommittedPatternSequence = boss.iPatternSequence;
				boss.iGrabExecutionCommittedStageIndex = boss.iPatternStageIndex;
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS ==
			action.eKind)
		{
			const float distance =
				BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK ==
					action.eReleaseMode ?
				action.fReleaseSpeedMps *
					(static_cast<float>(action.iDurationMs) / 1000.f) : 0.f;
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			for (const auto& [playerId, player] : m_Players)
			{
				if (player.iAttachmentOwnerNetEntityId != boss.iNetEntityId)
					continue;
				SERVER_PLAYER staged = player;
				const bool prepared =
					BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode ?
					Prepare_ArenaEjection(staged, boss, action, serverTick) :
					Release_PlayerAttachment(staged, boss.iNetEntityId, distance,
						action.iDurationMs, false, 0u, serverTick);
				if (!prepared)
					return false;
				stagedPlayers.emplace(playerId, std::move(staged));
			}
			for (auto& [playerId, staged] : stagedPlayers)
				m_Players.at(playerId) = std::move(staged);
		}
	}
	if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH == stage->Motion.eKind)
	{
		if (!CValtanBrain::Lock_PortalTargetRushAtStageStart(boss))
		{
			m_strStatus = "Portal target rush Stage entered without a valid locked route";
			return false;
		}
	}
	else if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA == stage->Motion.eKind)
		CValtanBrain::Configure_PortalMotion(boss, *stage);
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
	CEncounterPropRuntime staged = m_EncounterPropRuntime;
	bool broadcast = false;
	std::string status;
	const bool breakDue = 0u != m_iPillarAuditionBreakTick &&
		Has_ReachedServerTick(serverTick, m_iPillarAuditionBreakTick);
	if (breakDue)
	{
		ENCOUNTER_PROP_TRANSACTION breakTransaction{};
		const auto prepared = staged.Prepare_Break(
			staged.Get_OccurrenceSequence(), serverTick, breakTransaction, status);
		if (ENCOUNTER_PROP_PREPARE_RESULT::REJECTED == prepared ||
			(ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared &&
			 !staged.Commit(breakTransaction, status)))
		{
			m_strStatus = "Encounter prop break commit failed: " + status;
			return false;
		}
		if (ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared)
		{
			broadcast = true;
		}
	}
	ENCOUNTER_PROP_TRANSACTION transaction{};
	const auto prepared = staged.Prepare_DueRemoval(
		serverTick, PILLAR_BREAKING_TICKS, transaction, status);
	if (ENCOUNTER_PROP_PREPARE_RESULT::REJECTED == prepared ||
		(ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared &&
		 !staged.Commit(transaction, status)))
	{
		m_strStatus = "Encounter prop removal commit failed: " + status;
		return false;
	}
	broadcast = broadcast || ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared;
	if (breakDue)
		m_iPillarAuditionBreakTick = 0u;
	if (broadcast)
	{
		m_EncounterPropRuntime = std::move(staged);
		Broadcast_EncounterPropSync();
	}
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
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
			(WORLD_BOOTSTRAP_KIND::NPC != entity.eKind &&
			 0u == entity.iCurrentHp))
		{
			continue;
		}
		/* Same body the skill hit test uses: monsters carry their profile
		radius, the boss reads its profile, and town NPCs use the shared upright
		player-sized body until the catalog owns a dedicated gameplay radius. */
		float radius = entity.fCollisionRadius;
		float centerY = entity.fPositionY + radius;
		float halfHeight = radius;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			if (const BOSS_RUNTIME_PROFILE* bossProfile =
				m_GameplayCatalog.Find_Boss(entity.strArchetypeId))
			{
				radius = bossProfile->fCollisionRadius;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
		{
			using namespace LostArk::Shared::WorldCollision;
			radius = PLAYER_HALF_EXTENT_X;
			centerY = entity.fPositionY + PLAYER_CENTER_OFFSET_Y;
			halfHeight = PLAYER_HALF_EXTENT_Y;
		}
		else if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind)
		{
			continue;
		}
		if (radius <= 0.f)
			continue;
		if (WORLD_BOOTSTRAP_KIND::NPC != entity.eKind)
		{
			centerY = entity.fPositionY + radius;
			halfHeight = radius;
		}
		bodies.push_back(SERVER_BLOCKING_BODY{
			entity.fPositionX, entity.fPositionZ, radius,
			centerY, halfHeight, entity.iNetEntityId });
	}
	m_ServerCollisionSystem.Set_BlockingBodies(std::move(bodies));
}

void LostArk::Server::CGameRoom::Begin_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	player.iFallDeathTick = Add_ServerTicksSkippingReservedZero(
		player.iActionStartTick, FALL_DEATH_TICKS);
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
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.Clear_Attachment();
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
}

bool LostArk::Server::CGameRoom::Capture_PlayerAttachment(
	const LostArk::Shared::NET_ENTITY_ID playerEntityId,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == playerEntityId ||
		INVALID_NET_ENTITY_ID == ownerEntityId ||
		playerEntityId == ownerEntityId || 0u == serverTick ||
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot)
	{
		return false;
	}

	const auto playerId = m_PlayerIdByEntityId.find(playerEntityId);
	if (m_PlayerIdByEntityId.end() == playerId)
		return false;
	const auto playerIter = m_Players.find(playerId->second);
	if (m_Players.end() == playerIter)
		return false;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() == owner ||
		WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
		SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
		0u == owner->iCurrentHp || 0u == owner->iPatternSequence ||
		!std::isfinite(owner->fPositionX) ||
		!std::isfinite(owner->fPositionY) ||
		!std::isfinite(owner->fPositionZ) ||
		!std::isfinite(owner->fYawDegrees))
	{
		return false;
	}

	SERVER_PLAYER& player = playerIter->second;
	if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
	{
		return player.iAttachmentOwnerNetEntityId == ownerEntityId &&
			player.eAttachmentSlot == slot &&
			player.iAttachmentPatternSequence == owner->iPatternSequence;
	}
	if (0u == player.iCurrentHp || !player.isCombatReady ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		!std::isfinite(player.fPositionX) ||
		!std::isfinite(player.fPositionY) ||
		!std::isfinite(player.fPositionZ) ||
		!std::isfinite(player.fYawDegrees))
	{
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float deltaX = player.fPositionX - owner->fPositionX;
	const float deltaZ = player.fPositionZ - owner->fPositionZ;
	const float localX = deltaX * cosine - deltaZ * sine;
	const float localY = player.fPositionY - owner->fPositionY;
	const float localZ = deltaX * sine + deltaZ * cosine;
	const float localYaw = Wrap_Degrees(
		player.fYawDegrees - owner->fYawDegrees);
	if (!std::isfinite(localX) || !std::isfinite(localY) ||
		!std::isfinite(localZ) || !std::isfinite(localYaw))
	{
		return false;
	}

	/* Capture interrupts one complete action transaction. Projectiles and
	combat objects cannot remain owned by a body whose input is now frozen. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
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
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.Clear_Attachment();
	player.iAttachmentOwnerNetEntityId = ownerEntityId;
	player.eAttachmentSlot = slot;
	player.iAttachmentPatternSequence = owner->iPatternSequence;
	player.fAttachmentLocalOffsetX = localX;
	player.fAttachmentLocalOffsetY = localY;
	player.fAttachmentLocalOffsetZ = localZ;
	player.fAttachmentYawOffsetDegrees = localYaw;
	player.eAction = PLAYER_ACTION_STATE::GRABBED;
	player.iActionStartTick = serverTick;
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return true;
}

bool LostArk::Server::CGameRoom::Release_PlayerAttachment(
	SERVER_PLAYER& player,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
		player.iAttachmentOwnerNetEntityId != ownerEntityId ||
		0u == serverTick || !std::isfinite(pushRangeM))
	{
		return false;
	}

	float sourceX = player.fPositionX;
	float sourceZ = player.fPositionZ;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() != owner &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionZ))
	{
		sourceX = owner->fPositionX;
		sourceZ = owner->fPositionZ;
	}

	player.Clear_Attachment();
	player.eAction = 0u == player.iCurrentHp ?
		PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = 0u != player.iCurrentHp;
	if (0u != player.iCurrentHp)
	{
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, sourceX, sourceZ, pushRangeM, pushMs,
			knockdown, downMs, serverTick);
	}
	return true;
}

std::size_t LostArk::Server::CGameRoom::Release_PlayerAttachments(
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	std::size_t released = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (Release_PlayerAttachment(
			player, ownerEntityId, pushRangeM, pushMs,
			knockdown, downMs, serverTick))
		{
			++released;
		}
	}
	return released;
}

bool LostArk::Server::CGameRoom::Update_PlayerAttachment(
	SERVER_PLAYER& player,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction)
	{
		if (INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::NONE != player.eAttachmentSlot ||
			0u != player.iAttachmentPatternSequence)
		{
			player.Clear_Attachment();
		}
		return false;
	}

	const NET_ENTITY_ID ownerEntityId =
		player.iAttachmentOwnerNetEntityId;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	const bool validOwner = m_WorldEntities.end() != owner &&
		WORLD_BOOTSTRAP_KIND::BOSS == owner->eKind &&
		SERVER_ENTITY_ACTION::DEAD != owner->eAction &&
		0u != owner->iCurrentHp && 0u != owner->iPatternSequence &&
		player.iAttachmentPatternSequence == owner->iPatternSequence &&
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND == player.eAttachmentSlot &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionY) &&
		std::isfinite(owner->fPositionZ) &&
		std::isfinite(owner->fYawDegrees) &&
		std::isfinite(player.fAttachmentLocalOffsetX) &&
		std::isfinite(player.fAttachmentLocalOffsetY) &&
		std::isfinite(player.fAttachmentLocalOffsetZ) &&
		std::isfinite(player.fAttachmentYawOffsetDegrees);
	if (!validOwner)
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float nextX = owner->fPositionX +
		player.fAttachmentLocalOffsetX * cosine +
		player.fAttachmentLocalOffsetZ * sine;
	const float nextY = owner->fPositionY +
		player.fAttachmentLocalOffsetY;
	const float nextZ = owner->fPositionZ -
		player.fAttachmentLocalOffsetX * sine +
		player.fAttachmentLocalOffsetZ * cosine;
	const float nextYaw = Wrap_Degrees(
		owner->fYawDegrees + player.fAttachmentYawOffsetDegrees);
	if (!std::isfinite(nextX) || !std::isfinite(nextY) ||
		!std::isfinite(nextZ) || !std::isfinite(nextYaw))
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	player.fPositionX = nextX;
	player.fPositionY = nextY;
	player.fPositionZ = nextZ;
	player.fYawDegrees = nextYaw;
	player.isCombatReady = false;
	return true;
}

/* Owns the whole falling life cycle of one player inside one tick: it starts
a fall when the authored ground under the player is gone, advances a running
fall, and turns it into the ordinary death the revive path already
understands. Returning true is what keeps trigger motion, skills and movement
from running at all this tick. */
bool LostArk::Server::CGameRoom::Restore_PatternBoundPlayer(
	SERVER_PLAYER& player)
{
	float restoreX = player.fPatternBindRestoreX;
	float restoreY = player.fPatternBindRestoreY;
	float restoreZ = player.fPatternBindRestoreZ;
	bool resolved = std::isfinite(restoreX) && std::isfinite(restoreY) &&
		std::isfinite(restoreZ);
	if (m_ServerNavigation.Is_Loaded())
	{
		resolved = false;
		SERVER_NAV_POINT ground{};
		if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPatternBindRestoreX,
				player.fPatternBindRestoreZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPatternBindRestoreX;
			restoreY = ground.y;
			restoreZ = player.fPatternBindRestoreZ;
			resolved = true;
		}
		else if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			(m_ServerNavigation.Project_PointOnSameLevel(
			player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground) ||
			m_ServerNavigation.Project_Point(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground)) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = ground.x;
			restoreY = ground.y;
			restoreZ = ground.z;
			resolved = true;
		}
		if (!resolved && std::isfinite(player.fPositionX) &&
			std::isfinite(player.fPositionZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPositionX, player.fPositionZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPositionX, player.fPositionZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPositionX;
			restoreY = ground.y;
			restoreZ = player.fPositionZ;
			resolved = true;
		}
		if (!resolved && !player.strSpawnPlacementId.empty())
		{
			const WORLD_BOOTSTRAP_PLACEMENT* spawn =
				Find_Placement(player.strSpawnPlacementId);
			if (nullptr != spawn &&
				m_ServerNavigation.Is_PointWalkableExact(
					spawn->fPositionX, spawn->fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					spawn->fPositionX, spawn->fPositionZ, ground) &&
				std::isfinite(ground.x) && std::isfinite(ground.y) &&
				std::isfinite(ground.z))
			{
				restoreX = spawn->fPositionX;
				restoreY = ground.y;
				restoreZ = spawn->fPositionZ;
				resolved = true;
			}
		}
	}
	if (!resolved)
		return false;
	const float restoreYaw = std::isfinite(player.fPatternBindRestoreYawDegrees) ?
		player.fPatternBindRestoreYawDegrees :
		(std::isfinite(player.fYawDegrees) ? player.fYawDegrees : 0.f);
	player.fPositionX = restoreX;
	player.fPositionY = restoreY;
	player.fPositionZ = restoreZ;
	player.fYawDegrees = restoreYaw;
	player.eAction = 0u == player.iCurrentHp ?
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD :
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	player.isCombatReady = 0u != player.iCurrentHp &&
		player.bPatternBindRestoreCombatReady;
	player.Clear_PatternBindStatus();
	return true;
}

bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (player.bArenaEjectionActive && 0u != player.iCurrentHp)
		return false;
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

	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
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
		const auto ownsLivePatternOccurrence =
			[this](const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
				const std::uint32_t patternSequence)
			{
				return std::any_of(
					m_WorldEntities.begin(), m_WorldEntities.end(),
					[ownerEntityId, patternSequence](
						const SERVER_WORLD_ENTITY& entity)
					{
						return entity.iNetEntityId == ownerEntityId &&
							entity.iPatternSequence == patternSequence &&
							0u != entity.iCurrentHp &&
							SERVER_ENTITY_ACTION::DEAD != entity.eAction;
					});
			};
		if (0u == player.iCurrentHp ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			/* A lethal hit does not strand the replicated body five metres above
			the arena. Restore the admitted pose first, while preserving DEAD and
			combat-disabled state, then release both occurrence owners. */
			if (player.bPatternBound)
				(void)Restore_PatternBoundPlayer(player);
			player.Clear_SilenceStatus();
		}
		else
		{
			if (player.bPatternBound &&
				(Has_ReachedServerTick(updateTick, player.iPatternBindEndTick) ||
				 !ownsLivePatternOccurrence(
					player.iPatternBindOwnerNetEntityId,
					player.iPatternBindSequence)))
			{
				(void)Restore_PatternBoundPlayer(player);
			}
			if (0u != player.iSilenceEndTick &&
				Has_ReachedServerTick(updateTick, player.iSilenceEndTick))
			{
				player.Clear_SilenceStatus();
			}
		}
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
		if (player.bPatternBound)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			player.PendingCommand.Clear();
			player.isCombatReady = false;
			continue;
		}
		if (Update_PlayerAttachment(player, updateTick))
			continue;
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			continue;
		}
		const bool wasKnockbackActive =
			player.fKnockbackRemainingSeconds > 0.f;
		Advance_PlayerKnockback(player, fixedDeltaSeconds);
		if (wasKnockbackActive)
			continue;
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

bool LostArk::Server::CGameRoom::Resolve_ArenaCenter(
	const SERVER_WORLD_ENTITY& boss, SERVER_NAV_POINT& point)
{
	const bool exact = m_ServerNavigation.Is_PointWalkableExact(
		boss.fSpawnPositionX, boss.fSpawnPositionZ) &&
		m_ServerNavigation.Sample_Position(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point);
	if (!m_ServerNavigation.Is_Loaded() ||
		(!exact && !m_ServerNavigation.Project_PointOnSameLevel(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point)) ||
		!m_ServerNavigation.Is_PointWalkableExact(point.x, point.z) ||
		std::fabs(point.y - boss.fSpawnPositionY) > 1.5f ||
		std::hypot(point.x - boss.fSpawnPositionX,
			point.z - boss.fSpawnPositionZ) > 8.f)
	{
		m_strStatus = "Arena center has no nearby walkable same-level recovery point";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Prepare_ArenaEjection(
	SERVER_PLAYER& staged,
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick)
{
	if (BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION != action.eReleaseMode ||
		!m_ServerNavigation.Is_Loaded() ||
		!std::isfinite(action.fReleaseSpeedMps) || action.fReleaseSpeedMps <= 0.f ||
		action.fReleaseSpeedMps > 50.f || 0u == action.iDurationMs ||
		action.iDurationMs > 5000u ||
		!std::isfinite(action.fReleaseYawOffsetDegrees) ||
		std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
		!std::isfinite(boss.fYawDegrees))
	{
		m_strStatus = "Arena ejection policy or navigation is invalid";
		return false;
	}
	const float yaw = (boss.fYawDegrees + action.fReleaseYawOffsetDegrees) *
		DEGREES_TO_RADIANS;
	const float directionX = -std::sin(yaw);
	const float directionZ = -std::cos(yaw);
	constexpr float maximumDistance = 128.f;
	constexpr float outsideMargin = 2.f;
	const float sampleStep = std::clamp(m_ServerNavigation.Get_CellSize(), 0.1f, 0.5f);
	float lastArenaGround = 0.f;
	/* Scan past small holes and seams. An interior missing cell is not the arena
	   exterior; the endpoint lies beyond the last same-deck ground on this ray. */
	for (float distance = 0.f; distance <= maximumDistance; distance += sampleStep)
	{
		const float x = staged.fPositionX + directionX * distance;
		const float z = staged.fPositionZ + directionZ * distance;
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_PointWalkableExact(x, z) &&
			m_ServerNavigation.Sample_Position(x, z, ground) &&
			std::fabs(ground.y - boss.fSpawnPositionY) <= 1.5f)
			lastArenaGround = distance;
	}
	const float minimumDistance = action.fReleaseSpeedMps *
		(static_cast<float>(action.iDurationMs) / 1000.f);
	const float distance = (std::max)(minimumDistance, lastArenaGround + outsideMargin);
	if (!std::isfinite(distance) || distance > maximumDistance ||
		!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
	{
		m_strStatus = "Arena ejection has no bounded exterior destination";
		return false;
	}
	if (0u == staged.iCurrentHp)
		return true;
	staged.fKnockbackDirectionX = directionX;
	staged.fKnockbackDirectionZ = directionZ;
	staged.fKnockbackSpeed = action.fReleaseSpeedMps;
	staged.fKnockbackRemainingSeconds = distance / action.fReleaseSpeedMps;
	staged.bArenaEjectionActive = true;
	staged.iEjectionOwnerNetEntityId = boss.iNetEntityId;
	staged.isCombatReady = false;
	return true;
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
		player.Clear_Attachment();
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
	if (player.bArenaEjectionActive)
	{
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fKnockbackRemainingSeconds =
			(std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&player](const SERVER_WORLD_ENTITY& boss)
			{ return boss.iNetEntityId == player.iEjectionOwnerNetEntityId; });
		if (player.fKnockbackRemainingSeconds <= 0.00001f ||
			owner == m_WorldEntities.end() || 0u == owner->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == owner->eAction)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
				1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		}
		return;
	}
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
	constexpr float TRIANGLE_CIRCUMRADIUS_M = 9.f;
	constexpr float TRIANGLE_EDGE_LENGTH_M = 15.5884572681f;
	constexpr float PORTAL_RUNNER_SPEED_MPS = 11.9911209755f;
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
		if (INVALID_NET_ENTITY_ID == child->iOwnerBossNetEntityId)
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

void LostArk::Server::CGameRoom::Update_WorldEntities(
	const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
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
			CKoukuSaydonBrain::Is_GateOneBoss(m_eWorldId, entity))
		{
			if (!Update_KoukuSaydonBoss(entity, updateTick))
			{
				Mark_RuntimeFailure("world-update.koukusaydon-brain");
				return;
			}
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
#ifdef _DEBUG
			if (updateValtanBrain &&
				!Prepare_ValtanPatternIdAuditionBeforeBrain(entity))
				continue;
#endif
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
#ifdef _DEBUG
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
#endif
			if (updateValtanBrain)
			{
				std::vector<SERVER_PLAYER_CAPTURE_REQUEST> captureRequests;
				const BOSS_PATTERN_SEQUENCE_DEFINITION* patternFlowSequence =
					nullptr;
#ifdef _DEBUG
				patternFlowSequence =
					Resolve_ValtanPatternFlowSequence(entity);
#endif
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
#ifdef _DEBUG
				stopRequested = stopRequested ||
					(m_ValtanNextPattern &&
						m_ValtanNextPattern->iBossEntityId == entity.iNetEntityId) ||
					(Is_ValtanPatternFlowRunning() &&
						m_ValtanPatternFlowAudition.iBossEntityId == entity.iNetEntityId &&
						m_ValtanPatternFlowAudition.bStopAfterCurrent);
#endif
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
#ifdef _DEBUG
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
#endif
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
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			const auto playerIter = m_Players.find(playerId);
			if (playerIter == m_Players.end())
				continue;
			for (const std::string& itemId : m_ValtanClearRewards.Get_ItemIds())
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
		iter = m_WorldEntities.erase(iter);
	}
	if (!Update_DependentBosses(updateTick))
		Mark_RuntimeFailure("world-update.dependent-bosses-after-primary");
}
