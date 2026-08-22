#pragma once

#include "Network/NetworkIds.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "WorldBootstrap.h"
#include "ServerNavigation.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_BOSS_PATTERN_COOLDOWN final
	{
		std::string strPatternId;
		std::uint32_t iReadyTick = 0;
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

	struct SERVER_WORLD_ENTITY
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
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
		/* The encounter's intro pattern runs once per encounter epoch, on the
		first engage. A late joiner never replays it, and only a room-empty or
		Debug reset clears the ledger. */
		bool bIntroPatternConsumed = false;
		/* The Debug ordered audition replays an authored 1-67 list and requires an
		empty queue between steps, so product pattern follow-ups must not be
		queued while it drives the boss. */
		bool bScriptedPatternPlayback = false;
		std::uint32_t iPatternTelegraphMs = 0;
		std::uint32_t iPatternActiveMs = 0;
		std::uint32_t iPatternRecoveryMs = 0;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
		std::uint32_t iPatternStageDurationMs = 0;
		BOSS_PATTERN_STAGE_MOTION_KIND ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		BOSS_PATTERN_HIT_SHAPE ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
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
		/* True while a charge stage drives the boss forward. Meeting an impact
		receiver ends that stage early into the GROGGY stage behind it. */
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
		SERVER_BOSS_COMBAT_STATE BossCombat;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		float fCollisionRadius = 0.f;
		float fAttackRange = 0.f;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		std::uint32_t iDeadDespawnMs = 0;
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
		std::uint32_t iPhaseTwoHpPercent = 0;
		/* Staged from the boss profile at spawn, in authored plate order. Empty
		for every entity that wears no armour. */
		std::vector<SERVER_BOSS_ARMOR_PLATE_STATE> ArmorPlates;
		bool hasAppliedPatternDamage = false;
		std::string strLastPatternId;
		/* Cursor into the authored rotation of the span the boss is in. It is
		kept per span so a scripted mechanic that interrupts the stretch does
		not restart the list, and it resets when the boss enters a new span. */
		std::string strRotationId;
		std::uint32_t iRotationStepIndex = 0;
		std::uint32_t iConsecutivePatternUses = 0;
		std::vector<SERVER_BOSS_PATTERN_COOLDOWN> PatternCooldowns;
		std::vector<std::string> PendingPatternIds;
		std::vector<std::string> TriggeredPatternIds;
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
	};
}
