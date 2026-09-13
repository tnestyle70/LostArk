#pragma once

#include "GameRoom.h"
#include "ClientSession.h"
#include <set>
#include <cmath>
#include <cstdlib>

namespace GameRoomDetail
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

bool Is_KoukuBossMotionNavigable(const BOSS_PATTERN_DEFINITION& pattern,
		const CServerNavigation& navigation);

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

// One KoukuSaydon interaction HUD press converted to fixed 30 Hz room ticks.
	constexpr std::uint32_t KOUKU_INTERACTION_TICKS =
		(LostArk::Shared::KOUKU_INTERACTION_ACTION_MS * 30u + 999u) / 1000u;

/* The hammer keeps its whole authored swing, because the Client scrubs
	that clip by the action's age and snaps back to idle as soon as the Server
	drops the action. What the hunter gets instead is a cancel: once the head
	has landed, their own next input ends the recovery early. Returns true
	when a recovery was actually cancelled. An escape teleport borrows the
	same lock, so a pending transfer is never cancelled here. */
	bool Cancel_MazeHammerRecovery(
		LostArk::Server::SERVER_PLAYER& player, const std::uint32_t tick);

constexpr const char* RAID_CLEAR_TEST_MODE_ENV =
		"LOSTARK_RAID_CLEAR_TEST_MODE";

/* Project-owned 2D lanes: coordinates remain in the existing published
	Gameplay placements. Source camera rails established only the fixed screen
	right sign; neither runtime cameras nor camera edits supply Server input. */
	struct MARIO_LANE_BINDING
	{
		std::uint8_t stage;
		const char* arrival;
		const char* exit;
		float rightSign;
	};

constexpr std::array<MARIO_LANE_BINDING, 18u> MARIO_LANES{{
		{1u, "Mario1_go", "Mario1_Trigger_1", 1.f},
		{1u, "Mario1_Trigger_1", "Mario1_Trigger_3", 1.f},
		{1u, "Mario1_Trigger_3", "Mario1_Trigger_5", 1.f},
		{2u, "Mario2_go", "Mario2_Trigger_2", 1.f},
		{2u, "Mario2_Trigger_2", "Mario2_Trigger_4", -1.f},
		{2u, "Mario2_Trigger_4", "Mario2_Trigger_7", 1.f},
		{3u, "Mario3_go", "Mario3_Trigger_4", 1.f},
		{3u, "Mario3_Trigger_4", "Mario3_Trigger_5", -1.f},
		{3u, "Mario3_Trigger_5", "Mario3_Trigger_6", 1.f},
		{3u, "Mario3_Trigger_6", "Mario3_Trigger_8", 1.f},
		{3u, "Mario3_Trigger_8", "Mario3_Trigger_10", -1.f},
		{3u, "Mario3_Trigger_10", "Mario3_Trigger_12", 1.f},
		{4u, "Mario4_go", "Mario4_Tigger_2", 1.f},
		{4u, "Mario4_Tigger_2", "Mario4_Tigger_5", 1.f},
		{4u, "Mario4_Tigger_3", "Mario4_Tigger_6", 1.f},
		{4u, "Mario4_Tigger_6", "Mario4_Tigger_7", -1.f},
		{4u, "Mario4_Tigger_7", "Mario4_Tigger_13", 1.f},
		{4u, "Mario4_Tigger_5", "Mario4_Tigger_7", 1.f}
	}};

void Project_MarioRailPoint(const SERVER_PLAYER& player, float& x, float& z);

bool Is_RaidClearTestModeEnabled();

std::uint64_t Current_UnixMilliseconds();

bool Is_StablePatternFlowId(const std::string_view value);

bool Is_PatternFlowRevision(const std::string_view value);

std::string Build_PatternFlowStartRequestIdentity(
		const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);

float Resolve_VolleyMinimumSpacing(
		const BOSS_COMBAT_OBJECT_DEFINITION& definition);

std::uint64_t Mix_DeterministicRandom(std::uint64_t value);

std::uint64_t Hash_StableId(const std::string& value);

float DeterministicUnitFloat(const std::uint64_t value);

void Build_WorldDestructionStateChanges(
		const WORLD_DESTRUCTION_TRANSACTION& transaction,
		std::vector<SERVER_COLLISION_STATE_CHANGE>& collisionChanges,
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& navigationChanges);

bool Is_SameDestructionTransitionTarget(
		const WORLD_DESTRUCTION_STATE_TRANSITION& left,
		const WORLD_DESTRUCTION_STATE_TRANSITION& right);

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
		const std::chrono::steady_clock::duration duration);

bool Is_BestEffortCommand(const ROOM_COMMAND_TYPE type);

std::string_view To_RoomCommandEnqueueResultName(
		const ROOM_COMMAND_ENQUEUE_RESULT result);

bool Is_SameBestEffortStream(
		const ROOM_COMMAND& queued,
		const ROOM_COMMAND& incoming);

bool Try_RemoveCoalescedCommand(
		std::deque<ROOM_COMMAND>& commands,
		const ROOM_COMMAND& incoming);

float Wrap_Degrees(float degrees);

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
		const std::uint32_t currentTick);

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
		const std::uint32_t targetTick);

void Clear_ValtanGhostRelocationState(SERVER_WORLD_ENTITY& boss);

std::uint32_t DurationMillisecondsToServerTicks(
		const std::uint32_t durationMs);

void Cancel_PlayerActionForPatternStatus(SERVER_PLAYER& player);

#ifdef _DEBUG
constexpr const char* VALTAN_ARENA_AUDITION_PLACEMENT_ID =
		"boss.valtan.center";
#endif

#ifdef _DEBUG
constexpr const char* CHARACTER_SELECT_AUDITION_PLACEMENT_ID =
		"boss.valtan.character-select.lazy";
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
bool Is_CharacterSelectEnvironmentDependentPattern(
		const std::string& patternId);
#endif

#ifdef _DEBUG
bool Is_ValtanOutcomeFollowupInFlight(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence,
		const LostArk::Shared::GameplayDataRevision& definitionRevision);
#endif

#ifdef _DEBUG
bool Has_ValtanOutcomeGroupCompleted(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence,
		const LostArk::Shared::GameplayDataRevision& definitionRevision);
#endif

#ifdef _DEBUG
constexpr const char* WALL_ATTACK_PATTERN_ID = "VALTAN_DOWN_SMASH";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_PATTERN_ID = "VALTAN_ARENA_BREAK_109";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_STAGE_ID = "IMPACT";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_ACTION_ID =
		"valtan.mechanic.arena-break-109.impact";
#endif

#ifdef _DEBUG
/* The final arena is the arena after the whole shrink chain, so the Debug
	   view also stages the two floor collapses. Stage A drops the outer rail at
	   84 bars and stage B drops the brick ring at the 30-bar landing. */
	constexpr const char* FINAL_ARENA_FLOOR_A_PATTERN_ID =
		"VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_FLOOR_A_STAGE_ID = "IMPACT";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_FLOOR_A_ACTION_ID =
		"valtan.mechanic.terrain-destruction-3.impact";
#endif

#ifdef _DEBUG
constexpr std::uint32_t FINAL_ARENA_FLOOR_A_STAGE_INDEX = 3u;
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_FLOOR_B_PATTERN_ID =
		"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_FLOOR_B_STAGE_ID = "IMPACT";
#endif

#ifdef _DEBUG
constexpr const char* FINAL_ARENA_FLOOR_B_ACTION_ID =
		"valtan.mechanic.terrain-destruction-9.impact";
#endif

#ifdef _DEBUG
constexpr std::uint32_t FINAL_ARENA_FLOOR_B_STAGE_INDEX = 3u;
#endif

#ifdef _DEBUG
bool Resolve_ValtanArenaPreset(
		const std::uint32_t rawPreset,
		LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& outState,
		const char*& outLabel);
#endif

#ifdef _DEBUG
struct VALTAN_FIGHT_PAGE_POLICY final
	{
		const char* pTimelineRowId;
		std::uint8_t iInitialGameplayPhase;
		bool bPlayEntrance;
	};
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
const VALTAN_FIGHT_PAGE_POLICY* Find_ValtanFightPagePolicy(
		const std::string& rowId);
#endif

#ifdef _DEBUG
/* A known ordinary wall used only by the Debug attack audition. The boss is
	   kept six metres outside its centre so body contact cannot pre-empt the
	   DOWN_SMASH hit pulse, while the player is projected onto the inner side. */
	constexpr float WALL_ATTACK_CENTER_X = 161.402061f;
#endif

#ifdef _DEBUG
constexpr float WALL_ATTACK_CENTER_Y = 23.04f;
#endif

#ifdef _DEBUG
constexpr float WALL_ATTACK_CENTER_Z = -133.312236f;
#endif

#ifdef _DEBUG
constexpr float WALL_ATTACK_BOSS_OFFSET_Z = -6.f;
#endif

#ifdef _DEBUG
constexpr float WALL_ATTACK_PLAYER_OFFSET_Z = 8.f;
#endif

#ifdef _DEBUG
void Freeze_TimelineAuditionPlayer(SERVER_PLAYER& player);
#endif

#ifdef _DEBUG
void Prepare_TimelineAuditionPlayer(
		SERVER_PLAYER& player,
		const std::uint32_t actionTick);
#endif

bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message);

bool Is_NewerSequence(
		const std::uint32_t candidate,
		const std::uint32_t previous);

WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind);

WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action);

WORLD_DESTRUCTION_RUNTIME_STATE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_STATE state);

WORLD_DESTRUCTION_STATE_WIRE To_NetworkDestructionState(
		const WORLD_DESTRUCTION_GROUP_STATE& state);

std::uint32_t Hash_DestructionEventIdentity(
		const std::uint32_t encounterEpoch,
		const std::uint64_t eventSequence,
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition,
		const WORLD_DESTRUCTION_BINDING_APPLICATION& application,
		const std::uint32_t serverTick);

}
