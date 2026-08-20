#pragma once

#include "Network/NetworkIds.h"
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
		std::uint32_t iPatternTelegraphMs = 0;
		std::uint32_t iPatternActiveMs = 0;
		std::uint32_t iPatternRecoveryMs = 0;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
		std::uint32_t iPatternStageDurationMs = 0;
		BOSS_PATTERN_HIT_SHAPE ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		float fPatternHitOuterRadius = 0.f;
		float fPatternHitInnerRadius = 0.f;
		float fPatternHitAngleDegrees = 0.f;
		float fPatternHitLength = 0.f;
		float fPatternHitHalfWidth = 0.f;
		std::uint32_t iPatternHitCount = 0;
		std::uint32_t iPatternHitIntervalMs = 0;
		std::uint32_t iAppliedPatternHitCount = 0;
		bool bPatternWallContact = false;
		/* Player push of the running pattern stage's hit; negative pulls toward
		the boss. */
		float fPatternPushRangeM = 0.f;
		std::uint32_t iPatternPushMs = 0;
		bool bPatternKnockdown = false;
		std::uint32_t iPatternDownMs = 0;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint32_t iMaximumHealthBars = 1;
		std::uint32_t iLastEvaluatedHealthBar = 1;
		std::uint8_t iPhase = 1;
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
		bool hasAppliedPatternDamage = false;
		std::string strLastPatternId;
		std::uint32_t iConsecutivePatternUses = 0;
		std::vector<SERVER_BOSS_PATTERN_COOLDOWN> PatternCooldowns;
		std::vector<std::string> PendingPatternIds;
		std::vector<std::string> TriggeredPatternIds;
		LostArk::Shared::NET_ENTITY_ID iTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fLastPathGoalX = 0.f;
		float fLastPathGoalZ = 0.f;
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;
	};
}
