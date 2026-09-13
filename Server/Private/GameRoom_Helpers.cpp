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

namespace GameRoomDetail
{

bool Is_KoukuBossMotionNavigable(const BOSS_PATTERN_DEFINITION& pattern,
		const CServerNavigation& navigation)
	{
		if (!pattern.BossMotion) return true;
		const auto& motion = *pattern.BossMotion;
		return navigation.Is_PointWalkableExact(motion.StartPosition[0], motion.StartPosition[2]) &&
			navigation.Is_PointWalkableExact(motion.EndPosition[0], motion.EndPosition[2]) &&
			navigation.Has_LineOfSight(motion.StartPosition[0], motion.StartPosition[2],
				motion.EndPosition[0], motion.EndPosition[2]);
	}

/* The hammer keeps its whole authored swing, because the Client scrubs
	that clip by the action's age and snaps back to idle as soon as the Server
	drops the action. What the hunter gets instead is a cancel: once the head
	has landed, their own next input ends the recovery early. Returns true
	when a recovery was actually cancelled. An escape teleport borrows the
	same lock, so a pending transfer is never cancelled here. */
	bool Cancel_MazeHammerRecovery(
		LostArk::Server::SERVER_PLAYER& player, const std::uint32_t tick)
	{
		using namespace LostArk::Shared;
		if (PLAYER_ACTION_STATE::INTERACTION != player.eAction ||
			KOUKU_HUD_MODE::MAZE != player.eKoukuHudMode ||
			0u != player.CardMaze.transferStartTick ||
			0u == player.iActionStartTick ||
			static_cast<std::int32_t>(tick - (player.iActionStartTick +
				LostArk::Server::CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET)) < 0)
		{
			return false;
		}
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u;
		player.fActionElapsedSeconds = 0.f;
		player.PendingCommand.Clear();
		return true;
	}

void Project_MarioRailPoint(const SERVER_PLAYER& player, float& x, float& z)
	{
		if (0u == player.iMarioStage || !player.bMarioRailReady)
			return;
		const double axisX = player.fMarioRailRightX;
		const double axisZ = player.fMarioRailRightZ;
		const double lengthSquared = axisX * axisX + axisZ * axisZ;
		if (lengthSquared < 0.5)
			return;
		const double along = ((static_cast<double>(x) - player.fMarioRailOriginX) * axisX +
			(static_cast<double>(z) - player.fMarioRailOriginZ) * axisZ) / lengthSquared;
		x = static_cast<float>(player.fMarioRailOriginX + axisX * along);
		z = static_cast<float>(player.fMarioRailOriginZ + axisZ * along);
	}

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
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
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
#endif

#ifdef _DEBUG
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
		player.iCurrentMadness = 0u;
		player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
		player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		player.Clear_KoukuInteractionState();
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
