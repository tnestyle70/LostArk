#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct PLAYER_ROOT_MOTION_SAMPLE
	{
		std::uint32_t iTimeMs = 0;
		float fForward = 0.f;
		float fLateral = 0.f;
	};

	struct PLAYER_SKILL_HIT final
	{
		std::uint32_t iTimeMs = 0;
		std::uint32_t iRepeatCount = 1;
		std::uint32_t iRepeatMs = 0;
		/* Official AreaType: 1 circle/ring (fRange, fInner), 2 forward box
		(fRange length, fWidth), 3 fan (fRange, fAngleDegrees sweep, fInner). */
		std::uint32_t iAreaType = 0;
		float fRange = 0.f;
		float fAngleDegrees = 0.f;
		float fWidth = 0.f;
		float fHeight = 0.f;
		float fOffset = 0.f;
		float fInner = 0.f;
		std::uint32_t iMaxTargets = 0;
		/* Official push on this hit: 0 ms means the target is not moved; the
		range is metres away from the caster, negative pulling it closer. */
		std::uint32_t iPushMs = 0;
		float fPushRange = 0.f;
	};

	enum class PLAYER_PROJECTILE_KIND : std::uint8_t
	{
		MISSILE,
		FIXAREA,
		GRENADE,
		TRACE
	};

	/* One hit an object applies itself. A contact hit fires on each target the
	shape overlaps while the object lives (iRepeatCount times per target,
	iRepeatMs apart); a timed hit fires at iTimeMs after spawn (again
	iRepeatCount/iRepeatMs) on everything the shape overlaps at that moment. The
	shape origin and forward are the object's, not the caster's. */
	struct PLAYER_PROJECTILE_HIT final
	{
		bool isContact = false;
		PLAYER_SKILL_HIT Hit;
	};

	/* Where an object appears: on the caster, pushed fOffsetForward/fOffsetRight
	metres along the facing (official AreaOrigin 0), or where the aim points,
	clamped to fMaxDistance from the caster (AreaOrigin 1). */
	enum class PLAYER_PROJECTILE_ORIGIN : std::uint8_t
	{
		CASTER,
		AIM
	};

	/* An object a skill spawns at iTimeMs of its action, from the official
	Projectile definition: a MISSILE flies along the aim at fSpeed until it has
	travelled the aim distance clamped to [fMinDistance, fMaxDistance] (no cap
	when the max is 0) or iLifeMs is spent; a FIXAREA stands at its origin for
	iLifeMs. GRENADE and TRACE move like a MISSILE until their own motion is
	modelled. */
	struct PLAYER_SKILL_PROJECTILE final
	{
		std::uint32_t iTimeMs = 0;
		PLAYER_PROJECTILE_KIND eKind = PLAYER_PROJECTILE_KIND::MISSILE;
		PLAYER_PROJECTILE_ORIGIN eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
		float fOffsetForward = 0.f;
		float fOffsetRight = 0.f;
		float fSpeed = 0.f;
		float fMinDistance = 0.f;
		float fMaxDistance = 0.f;
		std::uint32_t iLifeMs = 0;
		float fRadius = 0.f;
		std::vector<PLAYER_PROJECTILE_HIT> Hits;
	};

	struct PLAYER_COMBO_STAGE final
	{
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iInputOpenMs = 0;
		std::uint32_t iInputCloseMs = 0;
		/* A stage advance resets the action clock, so a staged skill owns its
		movement per stage instead of on one action-long curve. */
		std::vector<PLAYER_ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
	};

	struct PLAYER_SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strInputSlot;
		std::string strActionId;
		std::string strDamageProfileId;
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iResourceCost = 0;
		/* Charged once on a successful Try_Start, same as iResourceCost, but out
		of the class identity gauge instead (Artist's moon/sun orbs). 0 for every
		skill that does not spend it. Falling short leaves the skill unusable
		rather than starting it for free. */
		std::uint32_t iIdentityCost = 0;
		float fMovementDistance = 0.f;
		float fMaximumRange = 0.f;
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eSetsStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		std::vector<PLAYER_COMBO_STAGE> ComboStages;
		std::vector<PLAYER_ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
	};

	struct BOSS_RUNTIME_PROFILE
	{
		std::string strArchetypeId;
		std::string strEncounterId;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumHealthBars = 0;
		/* Multiplicand for every damage rate this boss casts. */
		std::uint32_t iAttackPower = 0;
		/* Added to a skill's reach because official ranges stop at the target's
		edge while the server measures centre to centre. */
		float fCollisionRadius = 0.f;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		std::uint32_t iPhaseTwoHpPercent = 0;
	};

	enum class BOSS_PATTERN_SELECTION
	{
		NORMAL,
		HEALTH_BAR
	};

	enum class BOSS_PATTERN_HIT_SHAPE
	{
		NONE,
		CIRCLE,
		RING,
		CONE,
		BOX,
		CROSS,
		SIX_DIRECTIONS
	};

	enum class BOSS_PATTERN_STAGE_KIND
	{
		WINDUP,
		ACTIVE,
		RECOVERY
	};

	struct BOSS_PATTERN_STAGE_DEFINITION
	{
		std::string strStageId;
		std::string strActionId;
		std::string strDamageProfileId;
		BOSS_PATTERN_STAGE_KIND eStageKind =
			BOSS_PATTERN_STAGE_KIND::WINDUP;
		BOSS_PATTERN_HIT_SHAPE eHitShape =
			BOSS_PATTERN_HIT_SHAPE::NONE;
		std::uint32_t iDurationMs = 0;
		float fHitOuterRadius = 0.f;
		float fHitInnerRadius = 0.f;
		float fHitAngleDegrees = 0.f;
		float fHitLength = 0.f;
		float fHitHalfWidth = 0.f;
		std::uint32_t iHitCount = 0;
		std::uint32_t iHitIntervalMs = 0;
		/* Official player push of this stage's hit: metres over iPushMs, a
		negative range pulls the player toward the boss. */
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0;
		/* This ACTIVE hit is a physical axe contact candidate. Damage hits that
		are roars, magic, waves or floor mechanics deliberately leave this false. */
		bool bWallContact = false;
	};

	enum class BOSS_PATTERN_MOTION_KIND : std::uint8_t
	{
		NONE,
		LEAP_TO_ANCHOR
	};

	/* A pattern whose boss motion the Server computes itself carries one
	compiled anchor. The leap landing, the cinematic camera lookAt and the radial
	wall launch directions all read this one position, so none of them can drift
	into a private copy of the coordinate. */
	struct BOSS_PATTERN_MOTION
	{
		BOSS_PATTERN_MOTION_KIND eKind = BOSS_PATTERN_MOTION_KIND::NONE;
		std::string strAnchorId;
		float fLandingX = 0.f;
		float fLandingY = 0.f;
		float fLandingZ = 0.f;
		float fApexHeight = 0.f;
	};

	struct BOSS_PATTERN_DEFINITION
	{
		std::string strEncounterId;
		std::string strPatternId;
		std::string strActionId;
		BOSS_PATTERN_MOTION Motion;
		BOSS_PATTERN_SELECTION eSelection = BOSS_PATTERN_SELECTION::NORMAL;
		std::uint32_t iMinimumHealthBar = 0;
		std::uint32_t iMaximumHealthBar = 0;
		std::uint32_t iTriggerHealthBar = 0;
		std::uint32_t iTriggerOrder = 0;
		std::uint32_t iSelectionWeight = 0;
		std::uint32_t iMaximumConsecutiveUses = 0;
		float fMinimumRange = 0.f;
		float fMaximumRange = 0.f;
		/* Read from the first sourceActionId in Valtan.skilltiming. These raw
		values preserve the original skill metadata; gameplay metre ranges above
		remain the separately tuned Server selection contract. */
		std::uint32_t iSourcePrimaryActionId = 0;
		std::uint32_t iSourceShapeCount = 0;
		std::uint32_t iSourceCooldownMs = 0;
		std::uint32_t iSourceCooldownTicks = 0;
		std::uint32_t iSourceRangeUnits = 0;
		std::uint32_t iSourceApproachUnits = 0;
		std::uint32_t iSourceTurnDegrees = 0;
		std::uint32_t iExpectedStageCount = 0;
		std::vector<BOSS_PATTERN_STAGE_DEFINITION> Stages;
	};

	enum class VALTAN_DEBUG_AUDITION_MAPPING : std::uint8_t
	{
		PRODUCT_DIRECT,
		PRODUCT_CANDIDATE,
		PRODUCT_PARTIAL,
		MARKER,
		UNRESOLVED
	};

	/* One preserved recording occurrence. Executable rows name a stable product
	pattern and are still labelled by evidence strength; marker/unresolved rows
	own only a bounded idle pause and never substitute an unrelated attack. */
	struct VALTAN_DEBUG_AUDITION_STEP final
	{
		std::string strOccurrenceId;
		std::string strPatternId;
		VALTAN_DEBUG_AUDITION_MAPPING eMapping =
			VALTAN_DEBUG_AUDITION_MAPPING::UNRESOLVED;
		std::uint32_t iOrdinal = 0;
		std::uint32_t iRepeat = 0;
		std::uint32_t iTargetHealthBar = 0;
		std::uint32_t iPauseAfterMs = 0;
	};

	struct VALTAN_DEBUG_AUDITION_DEFINITION final
	{
		std::string strEncounterId;
		std::string strSequenceId;
		std::vector<VALTAN_DEBUG_AUDITION_STEP> Steps;
	};

	struct PLAYER_RUNTIME_PROFILE
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumResource = 0;
		/* Resource restored per wall-clock second while not casting. The pool is
		sized to official CostMp, so the regen rate has to be data too. */
		std::uint32_t iResourceRegenPerSecond = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		float fMoveSpeed = 0.f;
		/* Multiplies fMoveSpeed while the player holds a defensive stance. 1
		leaves the class unchanged, which is what every class without one uses. */
		float fDefenseStanceMoveSpeedScale = 1.f;
		/* The class identity gauge. 0 means the class has none, and then the
		rates are 0 too and no gauge is ever tracked. */
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityRegenPerSecond = 0;
		/* Spent per second while a stance the gauge pays for is held. Emptying
		the gauge drops the stance. Only one of this and
		iIdentityStanceSwitchCost is nonzero for a given class. */
		std::uint32_t iIdentityDrainPerSecond = 0;
		/* Charged once, only if already banked, the moment a stance-setting skill
		lands (LanceMaster's spear swap). A short fall leaves the switch free and
		does not touch the gauge. */
		std::uint32_t iIdentityStanceSwitchCost = 0;
		/* Non-zero turns the regen fill into a clock hand: reaching the maximum
		wraps back to 0 and keeps regenerating instead of holding at full
		(DimensionMaster). No stance or skill ever spends it -- the wrap itself
		is the spend. What happens at a full lap is not decided yet. */
		std::uint32_t iIdentityCyclic = 0;
		LostArk::Shared::PLAYER_STANCE_ID eDefaultStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
	};

	class CGameplayCatalog final
	{
	public:
		bool Load();

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const VALTAN_DEBUG_AUDITION_DEFINITION* Find_ValtanDebugAudition(
			const std::string& encounterId) const;
		/* Pattern the encounter plays exactly once when the boss first engages,
		before any health-bar or weighted selection runs. Empty when unknown. */
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		/* Percent of the caster's attack power, straight from the official
		EFTable_SkillEffect rate. Zero means the profile is unknown. */
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		/* The one place a rate becomes a number, so player skills and boss
		patterns cannot drift apart. Always at least 1 for a known profile: a hit
		that connects should never read as a miss. */
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, std::uint32_t damageRatePercent);
		/* The official client payload exposes defense coefficients but not the
		server mitigation formula.  This PROJECT_TUNED curve is centralized here
		so every incoming hit uses the same deterministic rule. */
		static std::uint32_t Apply_Defense(
			std::uint32_t rawDamage, std::uint32_t defense);

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		/* Shared by the per-skill and per-stage rows so both read one packed
		encoding and one ordering rule. Reports its own failure into m_strStatus. */
		bool Parse_RootMotionSamples(
			std::string_view packed,
			std::uint32_t sampleCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_ROOT_MOTION_SAMPLE>& outSamples);
		bool Parse_SkillHits(
			std::string_view packed,
			std::uint32_t hitCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_HIT>& outHits);
		/* One "areaType:range:angle:width:height:offset:inner:maxTargets:pushMs:
		pushRange" run, shared by caster hits and projectile hits. */
		bool Parse_HitShapeExtent(
			const std::string_view* fields,
			PLAYER_SKILL_HIT& outHit);
		/* One SKILLPROJ / SKILLSTAGEPROJ tail: index, spawn time, kind, motion and
		the packed hit list. Appends in row order and rejects an out-of-order
		spawn or index. */
		bool Parse_SkillProjectile(
			const std::vector<std::string_view>& fields,
			std::size_t firstField,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_PROJECTILE>& outProjectiles);

		std::unordered_map<LostArk::Shared::SKILL_ID, PLAYER_SKILL_DEFINITION>
			m_Skills;
		std::unordered_map<std::string, BOSS_RUNTIME_PROFILE> m_Bosses;
		std::unordered_map<std::string, std::vector<BOSS_PATTERN_DEFINITION>>
			m_BossPatterns;
		std::unordered_map<std::string, VALTAN_DEBUG_AUDITION_DEFINITION>
			m_ValtanDebugAuditions;
		std::unordered_map<std::string, std::string> m_IntroPatternIdByEncounter;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_RUNTIME_PROFILE> m_Players;
		std::unordered_map<std::string, std::uint32_t>
			m_DamageRatePercentByProfileId;
		std::string m_strStatus;
	};
}
