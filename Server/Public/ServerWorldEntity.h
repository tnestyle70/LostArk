#pragma once

#include "Network/NetworkIds.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "WorldBootstrap.h"
#include "ServerNavigation.h"
#include "NpcBehaviorRuntime.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_BOSS_PATTERN_COOLDOWN final
	{
		/* Source-derived patterns that share the same primary action share one
		cooldown family. Synthetic patterns have source action 0 and fall back to
		their stable pattern ID. */
		std::uint32_t iSourcePrimaryActionId = 0;
		std::string strPatternId;
		std::uint32_t iReadyTick = 0;
	};

	enum class SERVER_BOSS_MECHANIC_STATE : std::uint8_t
	{
		QUEUED,
		ACTIVE,
		COMPLETED,
		FAILED_REQUIRES_RESET
	};

	enum class SERVER_BOSS_MECHANIC_FAILURE : std::uint8_t
	{
		NONE,
		MISSING_PATTERN_DEFINITION,
		NO_VALID_TARGET,
		INVALID_RUNNING_DEFINITION,
		STAGE_TRANSITION_PREFLIGHT,
		STAGE_TRANSITION_COMMIT,
		BOSS_DIED
	};

	/* One health-bar mechanic is armed exactly once by stable pattern ID. The
	legacy PendingPatternIds queue remains the execution queue, while this
	bounded ledger explains whether that occurrence is waiting, running, done,
	or unsafe to retry without resetting the encounter. */
	struct SERVER_BOSS_MECHANIC_OCCURRENCE final
	{
		std::string strPatternId;
		/* A forced mechanic is an occurrence when the threshold is crossed, not
		an ID to reinterpret against whichever catalog happens to be active when
		the current pattern finishes. Keep that immutable definition generation
		until this queued/running occurrence reaches a terminal state. */
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision;
		SERVER_BOSS_MECHANIC_STATE eState =
			SERVER_BOSS_MECHANIC_STATE::QUEUED;
		SERVER_BOSS_MECHANIC_FAILURE eFailure =
			SERVER_BOSS_MECHANIC_FAILURE::NONE;
		std::uint32_t iTriggerHealthBar = 0u;
		std::uint32_t iQueuedTick = 0u;
		std::uint32_t iStartedTick = 0u;
		std::uint32_t iFinishedTick = 0u;
		std::uint32_t iPatternSequence = 0u;
	};

	/* The live state of one authored boss armour plate. iPlateIndex is copied
	from the profile and never changes, so it stays the slot a broken plate is
	named by. iRemainingDurability only falls inside a GROGGY stage and stops at
	zero; iDefense stops counting toward mitigation at that moment. */
	struct SERVER_BOSS_ARMOR_PLATE_STATE final
	{
		std::uint32_t iPlateIndex = 0;
		std::uint32_t iRemainingDurability = 0;
		std::uint32_t iDefense = 0;
	};

	enum class SERVER_ENTITY_ACTION
	{
		IDLE,
		CHASE,
		PATTERN_WINDUP,
		PATTERN_ACTIVE,
		PATTERN_RECOVERY,
		DEAD
	};

	enum class SERVER_BOSS_PATTERN_TERMINAL_RESULT : std::uint8_t
	{
		NONE,
		COMPLETED,
		ABORTED
	};

	struct SERVER_BOSS_PATTERN_TERMINAL_RECEIPT final
	{
		std::uint32_t iPatternSequence = 0u;
		SERVER_BOSS_PATTERN_TERMINAL_RESULT eResult =
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::NONE;
	};

	struct SERVER_WORLD_ENTITY
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		/* Dependent bosses share the existing boss runtime, never the primary HUD
		identity or damage target set. This relation outlives a pattern cycle. */
		LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		BOSS_PATTERN_SEQUENCE_DEFINITION DependentPatternSequence;
		std::string strPlacementId;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPatternId;
		std::string strPatternStageId;
		std::string strActionId;
		std::string strDamageProfileId;
		std::string strSpawnGroupId;
		WORLD_BOOTSTRAP_KIND eKind = WORLD_BOOTSTRAP_KIND::END;
		SERVER_ENTITY_ACTION eAction = SERVER_ENTITY_ACTION::IDLE;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		float fActionElapsedSeconds = 0.f;
		float fPatternMinimumRange = 0.f;
		float fPatternMaximumRange = 0.f;
		float fPatternForcedMotionSpeed = 0.f;
		/* The travel the running stage baked into its clip, copied on stage entry
		so the step never reaches back into the catalog. A stage that has one is
		stepped along it and ignores fPatternForcedMotionSpeed. */
		std::vector<ROOT_MOTION_SAMPLE> PatternStageRootMotion;
		bool bPortalMotionActive = false;
		/* PORTAL_TARGET_RUSH captures its target exactly once when the authored
		   retarget delay expires. Until then no yaw, endpoint, movement, or hit
		   sweep is committed for the leg. */
		bool bPortalRushTargetLocked = false;
		std::uint32_t iPortalRushRetargetDelayMs = 0u;
		float fPortalRushSpeedMps = 0.f;
		float fPortalRushDistanceM = 0.f;
		std::vector<LostArk::Shared::NET_ENTITY_ID> PortalStageHitTargets;
		float fPortalStartX = 0.f;
		float fPortalStartZ = 0.f;
		float fPortalEndX = 0.f;
		float fPortalEndZ = 0.f;
		/* Portal contact is swept from the last authored pulse, rather than the
		current render pose, so a fast root-motion step cannot tunnel through a
		player between two 50 ms evaluations. */
		float fPortalLastHitSampleX = 0.f;
		float fPortalLastHitSampleZ = 0.f;
		/* The authored placement this entity spawned at. The 109 phase
		transition lands Valtan back on it, so the landing point stays authored
		data rather than a constant recomputed inside the brain. */
		float fSpawnPositionX = 0.f;
		float fSpawnPositionY = 0.f;
		float fSpawnPositionZ = 0.f;
		/* Where the current authored leap started. No jump clip exists in the
		converted Valtan model, so the Server owns the whole arc and the Client
		only presents the replicated transform. */
		float fLeapOriginX = 0.f;
		float fLeapOriginY = 0.f;
		float fLeapOriginZ = 0.f;
		float fLeapApexHeight = 0.f;
		/* Where the current authored leap lands. Copied from the pattern's one
		compiled anchor, never from the boss placement, so the landing, the
		cinematic lookAt and the radial wall directions stay on one point. */
		float fLeapLandingX = 0.f;
		float fLeapLandingY = 0.f;
		float fLeapLandingZ = 0.f;
		/* Apex the current pattern authored, zero when it owns no leap. */
		float fPatternLeapApexHeight = 0.f;
		/* Compiled serverMotion travel stage. A leap may hold at its apex in
		   authored stages between TAKEOFF and this descent stage. */
		std::uint32_t iPatternLeapTravelStageIndex = 1u;
		/* The clip may spend most of a stage anticipating or recovering. These
		   authored subwindows keep the Server transform on the actual lift/drop
		   interval instead of stretching the leap across the whole stage. */
		std::uint32_t iPatternLeapTakeoffStartMs = 0u;
		std::uint32_t iPatternLeapTakeoffEndMs = 0u;
		std::uint32_t iPatternLeapTravelStartMs = 0u;
		std::uint32_t iPatternLeapTravelEndMs = 0u;
		bool bPatternMoveToAnchorBeforeTakeoff = false;
		/* The encounter's intro pattern runs once per encounter epoch, on the
		first engage. A late joiner never replays it, and only a room-empty or
		Debug reset clears the ledger. */
		bool bIntroPatternConsumed = false;
		/* The Debug ordered audition replays an authored 1-67 list and requires an
		empty queue between steps, so product pattern follow-ups must not be
		queued while it drives the boss. */
		bool bScriptedPatternPlayback = false;
		/* Debug entrance/health-bar audition temporarily yields the Product
		automatic sequence without changing the timeline playback contract. The
		brain holds the boss while an armed health-bar audition waits to cross. */
		bool bAutomaticPatternSequenceAuditionOverride = false;
		bool bAutomaticPatternSequenceAuditionHold = false;
		/* Set only after the Product automatic sequence selects its current step.
		The cursor advances in FinishPattern, never at selection, so an aborted
		step cannot be silently skipped. */
		bool bAutomaticPatternSequenceStepRunning = false;
		/* A non-empty party with no engageable player pauses an already-running
		Product sequence step until Debug revive restores a target. This is not a
		mechanic failure: the stage, hit cursor, and ordered cursor remain owned by
		the same occurrence. The last tick drives exact clock compensation while
		the pause is replicated through the unchanged action identity. */
		bool bAutomaticPatternSequencePausedForRevive = false;
		std::uint32_t iAutomaticPatternSequencePauseLastTick = 0u;
		/* Every successful non-terminal step yields a fixed authored pursuit
		window. During this countdown the normal CHASE path follows the nearest
		player, but no selector condition may replace or reorder the next step. */
		std::uint32_t iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
		std::uint32_t iAutomaticPatternSequencePursuitTicksRemaining = 0u;
		std::uint32_t iPatternTelegraphMs = 0;
		std::uint32_t iPatternActiveMs = 0;
		std::uint32_t iPatternRecoveryMs = 0;
		std::uint32_t iPatternSequence = 0;
		SERVER_BOSS_PATTERN_TERMINAL_RECEIPT PatternTerminalReceipt;
		/* Only a committed typed grab execution authorizes the current stage to
		finish its animation clock after the last living target was executed. */
		std::uint32_t iGrabExecutionCommittedPatternSequence = 0u;
		std::uint32_t iGrabExecutionCommittedStageIndex = 0u;
		std::uint32_t iPatternStageIndex = 0;
		/* Immutable gameplay bootstrap identity pinned when this entity is
		created and refreshed at each boss pattern occurrence boundary. */
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		/* Product owns one immutable sequence from its first selected step through
		terminal idle. Hot reload cannot combine an old ordinal with a new list;
		only a fresh encounter/reset releases this pin. Debug Flow owns its own pin. */
		LostArk::Shared::GameplayDataRevision ProductSequencePinnedDefinitionRevision{};
		std::uint32_t iPatternStageDurationMs = 0;
		/* First nonzero Server tick on which this stage is evaluated. Tick zero is
		the process-wide reserved sentinel, so wrap advances UINT32_MAX -> 1. */
		std::uint32_t iPatternStageFirstEvaluationTick = 0;
		/* Number of stage-owned spawn waves committed for the current action.
		The ENTER edge commits wave zero and later fixed ticks advance this only
		after the whole mixed volley transaction succeeds. */
		std::uint32_t iAppliedPatternStageSpawnWaveCount = 0;
		BOSS_PATTERN_STAGE_MOTION_KIND ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		BOSS_PATTERN_HIT_SHAPE ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		BOSS_PATTERN_PLAYER_RESPONSE ePatternPlayerResponse =
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT ePatternAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		BOSS_PATTERN_PART_DAMAGE_POLICY ePatternPartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		bool bPatternHasCounterProxy = false;
		float fPatternCounterProxyForwardOffsetM = 0.f;
		float fPatternCounterProxyRightOffsetM = 0.f;
		float fPatternCounterProxyRadiusM = 0.f;
		float fPatternHitOuterRadius = 0.f;
		float fPatternHitInnerRadius = 0.f;
		float fPatternHitAngleDegrees = 0.f;
		float fPatternHitLength = 0.f;
		float fPatternHitHalfWidth = 0.f;
		std::uint32_t iPatternHitCount = 0;
		std::uint32_t iPatternHitIntervalMs = 0;
		std::uint32_t iPatternHitDelayMs = 0;
		std::uint32_t iAppliedPatternHitCount = 0;
		bool bPatternWallContact = false;
		/* The running stage reaches through a raised encounter prop, so cover
		does not answer this blow. */
		bool bPatternPiercesCover = false;
		/* Player push of the running pattern stage's hit; negative pulls toward
		the boss. */
		float fPatternPushRangeM = 0.f;
		std::uint32_t iPatternPushMs = 0;
		bool bPatternKnockdown = false;
		std::uint32_t iPatternDownMs = 0;
		/* True only while a GROGGY stage runs. Set by the stage transition, so
		it clears itself when the pattern moves on or is interrupted. */
		bool bPatternGroggy = false;
		/* True while a charge stage drives the boss forward. Meeting either an
		ordinary authoritative wall or a destructible impact receiver ends that
		stage early into the GROGGY stage behind it. */
		bool bPatternChargeImpact = false;
		/* True for as long as an authored invulnerable pattern runs. Player hits
		resolve to nothing while it is set, so the raid answers the mechanic
		instead of outracing it. Cleared when the pattern ends. */
		bool bPatternInvulnerable = false;
		/* Raised by the hit that took a plate to zero durability. The brain owns
		stage transitions, so it consumes this on its next tick and enters the
		PART_BREAK stage; the damage path never moves the boss itself. */
		bool bPendingArmorBreakReaction = false;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint32_t iMaximumHealthBars = 1;
		std::uint32_t iLastEvaluatedHealthBar = 1;
		std::uint8_t iPhase = 1;
		/* Room-local immutable-generation epoch last used to evaluate health
		thresholds. It is distinct from PinnedDefinitionRevision: a running A
		occurrence executes from A while new crossings are evaluated from active B.
		The room fails closed before this compact epoch can wrap, and placing it
		here consumes the phase field's existing alignment gap. */
		std::uint16_t iLastHealthMechanicGenerationEpoch = 0u;
		SERVER_BOSS_COMBAT_STATE BossCombat;
		float fEngageDistance = 0.f;
		float fTargetReleaseDistance = 0.f;
		float fMoveSpeed = 0.f;
		float fTurnSpeedDegreesPerSecond = 0.f;
		float fMoveAcceleration = 0.f;
		float fMoveDeceleration = 0.f;
		float fArrivalSlowRadius = 0.f;
		float fCurrentMoveSpeed = 0.f;
		float fCollisionRadius = 0.f;
		float fAttackRange = 0.f;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		std::uint32_t iDeadDespawnMs = 0;
		/* Latches once this BOSS entity's Valtan clear rewards have been granted
		to the room's players, so a boss that stays DEAD without despawning (see
		Valtan) is never re-looted on a later tick. */
		bool bLootGranted = false;
		/* Multiplier on the authored push range of each player hit, from the
		monster profile; 0 never moves. */
		float fHitKnockbackScale = 0.f;
		/* Player push of this monster's landed attack, from the monster profile;
		negative pulls the player toward the monster. */
		float fAttackPushRangeM = 0.f;
		std::uint32_t iAttackPushMs = 0;
		bool bAttackKnockdown = false;
		std::uint32_t iAttackDownMs = 0;
		/* The knockback in flight: unit XZ direction away from the attacker,
		metres per second (negative pulls closer), and how much of the authored
		push window is left. A new hit restarts the window with its own
		direction instead of stacking. */
		float fKnockbackDirectionX = 0.f;
		float fKnockbackDirectionZ = 0.f;
		float fKnockbackSpeed = 0.f;
		float fKnockbackRemainingSeconds = 0.f;
		/* A raid Esther summon plays one roster-owned strike for iEstherStrikeMs
		instead of running a brain, and despawns the moment it ends rather than
		through the MONSTER dead sweep. */
		bool isEstherSummon = false;
		std::uint32_t iEstherStrikeMs = 0;
		std::uint32_t iNextPathReplanTick = 0;
		BOSS_PHASE_POLICY PhasePolicy;
		/* Deprecated fixture mirror retained until callers have migrated to the
		typed PhasePolicy. Product runtime never reads this field. */
		std::uint32_t iPhaseTwoHpPercent = 0;
		/* Staged from the boss profile at spawn, in authored plate order. Empty
		for every entity that wears no armour. */
		std::vector<SERVER_BOSS_ARMOR_PLATE_STATE> ArmorPlates;
		bool hasAppliedPatternDamage = false;
		std::string strLastPatternId;
		/* Active automatic program or normal-selection span plus its ordered
		cursor. Product sequence steps advance only after successful completion;
		WEIGHTED_POOL spans keep the cursor at zero and use only the id for
		diagnostics. */
		std::string strRotationId;
		std::uint32_t iRotationStepIndex = 0;
		std::uint32_t iConsecutivePatternUses = 0;
		std::vector<SERVER_BOSS_PATTERN_COOLDOWN> PatternCooldowns;
		std::vector<std::string> PendingPatternIds;
		std::vector<std::string> TriggeredPatternIds;
		std::vector<SERVER_BOSS_MECHANIC_OCCURRENCE> MechanicOccurrences;
		bool bMechanicLedgerRequiresReset = false;
		LostArk::Shared::NET_ENTITY_ID iTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		/* The authored pattern target is distinct from the nearest entity used to
		keep the encounter engaged. LOCK policies preserve this ID until the
		pattern ends; TRACK is the only policy allowed to replace it each tick. */
		LostArk::Shared::NET_ENTITY_ID iPatternTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		/* LOCK target policies keep the last valid server position for stage
		actions that occur after the selected player leaves or becomes invalid. */
		bool bHasPatternTargetLastPosition = false;
		float fPatternTargetLastPositionX = 0.f;
		float fPatternTargetLastPositionY = 0.f;
		float fPatternTargetLastPositionZ = 0.f;
		float fLastPathGoalX = 0.f;
		float fLastPathGoalZ = 0.f;
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;
		SERVER_NPC_BEHAVIOR_STATE NpcBehavior;
	};
}
