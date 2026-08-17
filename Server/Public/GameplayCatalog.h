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
		std::uint32_t iAreaType = 0;
		float fRange = 0.f;
		float fAngleDegrees = 0.f;
		float fHeight = 0.f;
		float fOffset = 0.f;
		float fInner = 0.f;
		std::uint32_t iMaxTargets = 0;
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

		std::unordered_map<LostArk::Shared::SKILL_ID, PLAYER_SKILL_DEFINITION>
			m_Skills;
		std::unordered_map<std::string, BOSS_RUNTIME_PROFILE> m_Bosses;
		std::unordered_map<std::string, std::vector<BOSS_PATTERN_DEFINITION>>
			m_BossPatterns;
		std::unordered_map<std::string, std::string> m_IntroPatternIdByEncounter;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_RUNTIME_PROFILE> m_Players;
		std::unordered_map<std::string, std::uint32_t>
			m_DamageRatePercentByProfileId;
		std::string m_strStatus;
	};
}
