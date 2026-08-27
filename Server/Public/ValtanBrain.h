#pragma once

#include "GameplayCatalog.h"
#include "Gameplay/CombatCollisionContract.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
	enum class VALTAN_DECISION_SOURCE : std::uint8_t
	{
		NONE,
		INTRO,
		FORCED_HEALTH_BAR,
		FORCED_AUDITION,
		ORDERED,
		WEIGHTED,
		GLOBAL,
		/* Source aliases keep old Debug-only call sites source compatible while
		all newly recorded decisions use the explicit names above. */
		FORCED_MECHANIC = FORCED_HEALTH_BAR,
		ROTATION_ORDERED = ORDERED,
		ROTATION_WEIGHTED = WEIGHTED,
		GLOBAL_WEIGHTED = GLOBAL
	};

	enum class VALTAN_DECISION_RESULT : std::uint8_t
	{
		SELECTED,
		WAITING_FOR_INTRO_RANGE,
		NO_ELIGIBLE_PATTERN,
		NO_VALID_TARGET,
		CATALOG_UNAVAILABLE,
		MECHANIC_RESET_REQUIRED
	};

	/* A candidate can carry more than one reason. SOFT_REPEAT_BLOCKED is not a
	permanent rejection: when every otherwise-eligible row is blocked, the same
	selector marks those rows SOFT_REPEAT_RELAXED and rolls over their original
	weight intervals. */
	enum VALTAN_DECISION_EXCLUSION : std::uint32_t
	{
		VALTAN_EXCLUDE_NONE = 0u,
		VALTAN_EXCLUDE_WRONG_SELECTION_KIND = 1u << 0u,
		VALTAN_EXCLUDE_INTRO_ROW = 1u << 1u,
		VALTAN_EXCLUDE_NOT_IN_SELECTION_SET = 1u << 2u,
		VALTAN_EXCLUDE_ARMOR_MISMATCH = 1u << 3u,
		VALTAN_EXCLUDE_PHASE_REQUIREMENT = 1u << 4u,
		VALTAN_EXCLUDE_PHASE_RANGE = 1u << 5u,
		VALTAN_EXCLUDE_HEALTH_BAR_RANGE = 1u << 6u,
		VALTAN_EXCLUDE_NO_TARGET = 1u << 7u,
		VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE = 1u << 8u,
		VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE = 1u << 9u,
		VALTAN_EXCLUDE_COOLDOWN = 1u << 10u,
		VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED = 1u << 11u,
		VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED = 1u << 12u,
		VALTAN_EXCLUDE_DISABLED = 1u << 13u,
		VALTAN_EXCLUDE_UNRESOLVED_DEFINITION = 1u << 14u,
		/* Compatibility aliases for the first local observatory draft. */
		VALTAN_EXCLUDE_NOT_NORMAL = VALTAN_EXCLUDE_WRONG_SELECTION_KIND,
		VALTAN_EXCLUDE_INTRO = VALTAN_EXCLUDE_INTRO_ROW,
		VALTAN_EXCLUDE_ROTATION = VALTAN_EXCLUDE_NOT_IN_SELECTION_SET,
		VALTAN_EXCLUDE_ARMOR = VALTAN_EXCLUDE_ARMOR_MISMATCH,
		VALTAN_EXCLUDE_HEALTH_BAR = VALTAN_EXCLUDE_HEALTH_BAR_RANGE,
		VALTAN_EXCLUDE_RANGE = VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE |
			VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE,
		VALTAN_EXCLUDE_MAXIMUM_CONSECUTIVE =
			VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED,
		VALTAN_EXCLUDE_MISSING_DEFINITION =
			VALTAN_EXCLUDE_UNRESOLVED_DEFINITION
	};

	struct VALTAN_DECISION_CANDIDATE_TRACE final
	{
		std::string strPatternId;
		std::uint32_t iExclusionMask = VALTAN_EXCLUDE_NONE;
		std::uint32_t iAuthoredWeight = 0u;
		std::uint32_t iEffectiveWeight = 0u;
		std::uint32_t iCooldownRemainingTicks = 0u;
		std::uint32_t iConsecutiveUses = 0u;
		std::uint32_t iMaximumConsecutiveUses = 0u;
		std::uint64_t iWeightBeginInclusive = 0u;
		std::uint64_t iWeightEndExclusive = 0u;
		bool bSoftRepeatBlocked = false;
		bool bSoftRepeatRelaxed = false;
		bool bSelected = false;
	};

	struct VALTAN_DECISION_TRACE final
	{
		std::uint64_t iTraceSequence = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequenceBeforeDecision = 0u;
		std::uint32_t iExpectedPatternSequence = 0u;
		std::uint32_t iCurrentHp = 0u;
		std::uint32_t iMaximumHp = 0u;
		std::uint32_t iHealthBar = 0u;
		std::uint8_t iGameplayPhase = 1u;
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fTargetDistance = 0.f;
		bool bIntroPatternConsumed = false;
		std::uint32_t iRotationStepIndex = 0u;
		VALTAN_DECISION_SOURCE eSource = VALTAN_DECISION_SOURCE::NONE;
		VALTAN_DECISION_RESULT eResult =
			VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
		std::string strRotationId;
		std::string strPendingPatternId;
		VALTAN_DECISION_SOURCE ePendingSource =
			VALTAN_DECISION_SOURCE::NONE;
		std::string strSelectedPatternId;
		std::uint64_t iRawRandomInput = 0u;
		std::uint64_t iMixedRandomValue = 0u;
		std::uint64_t iTotalWeight = 0u;
		std::uint64_t iRandomTicket = 0u;
		bool bMaximumConsecutiveRelaxed = false;
		bool bCandidatesTruncated = false;
		std::vector<VALTAN_DECISION_CANDIDATE_TRACE> Candidates;
	};

	/* The brain owns hit admission, while the room owns interruption and
	attachment lifecycle. This fixed-tick request bridges those two owners only
	for a surviving CAPTURE hit; it carries stable network identities, never a
	SERVER_PLAYER pointer. */
	struct SERVER_PLAYER_CAPTURE_REQUEST final
	{
		LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	};

	class CValtanBrain final
	{
	public:
		static constexpr std::size_t MAX_DECISION_TRACE_COUNT = 32u;
		static constexpr std::size_t MAX_DECISION_CANDIDATE_COUNT = 64u;
		static constexpr std::size_t MAX_MECHANIC_OCCURRENCE_COUNT = 64u;

		/* The bar the boss currently sits on, counting down from
		iMaximumHealthBars. Rounded up so surviving HP always shows a bar, which
		is what makes a threshold pattern fire on the tick the bar changes. */
		[[nodiscard]] static std::uint32_t Calculate_HealthBar(
			const SERVER_WORLD_ENTITY& boss);
		/* Inverse of the above: the HP that reads as healthBar, so a Debug
		audition can park a boss on a chosen threshold without restating the
		rounding. 0 when the bar is outside the boss's authored range. */
		[[nodiscard]] static std::uint32_t Resolve_HealthBarHp(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t healthBar);

		/* Whether a weighted pattern is offered in this boss's current armour
		state. A boss with no authored plates reads as stripped, so an encounter
		that never had armour keeps offering exactly what it did before. */
		[[nodiscard]] static bool Is_ArmorRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_ARMOR_REQUIREMENT requirement);

		/* Whether a weighted pattern is offered in this boss's current phase.
		The phase itself is advanced from the authored HP threshold, so this only
		reads the state and never decides when it changes. */
		[[nodiscard]] static bool Is_PhaseRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_PHASE_REQUIREMENT requirement);

		/* GameRoom owns stage-action preflight/commit. If that transaction cannot
		finish, the running health mechanic is closed explicitly instead of being
		silently re-queued from its crossed HP bar. */
		static void Fail_ActiveMechanic(
			SERVER_WORLD_ENTITY& boss,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);
		static void Fail_Mechanic(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);

		[[nodiscard]] std::size_t Get_DecisionTraceCount() const noexcept
		{
			return m_iDecisionTraceCount;
		}
		/* age zero is the oldest retained trace. Returns null when out of range. */
		[[nodiscard]] const VALTAN_DECISION_TRACE* Get_DecisionTrace(
			std::size_t age) const noexcept;
		[[nodiscard]] const VALTAN_DECISION_TRACE*
			Get_LatestDecisionTrace() const noexcept;

		/* outDamageEvents collects the hits this pattern tick lands on players so
		the room can ship the amounts in the same tick's snapshot. */
		/* coverCircles are the raised encounter props standing this tick. A boss
		   attack whose straight line to a player passes through one is answered
		   by that prop, so the player takes nothing. An empty list is the normal
		   case and costs one branch. */
		void Update(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
				coverCircles,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const CGameplayCatalog* activeThresholdCatalog = nullptr,
			std::uint16_t activeThresholdGenerationEpoch = 1u,
			std::vector<SERVER_PLAYER_CAPTURE_REQUEST>* outCaptureRequests =
				nullptr,
			const BOSS_PATTERN_SEQUENCE_DEFINITION*
				automaticSequenceOverride = nullptr) const;
		bool Try_BuildStageMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		/* The armour charge loop: a charge drives forward until it meets an
		authoritative wall, and the collision -- not the clock -- ends the stage. */
		bool Try_BuildImpactMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		bool Complete_ImpactStage(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;

	private:
		void Record_DecisionTrace(VALTAN_DECISION_TRACE&& trace) const;

		mutable std::array<VALTAN_DECISION_TRACE, MAX_DECISION_TRACE_COUNT>
			m_DecisionTraces{};
		mutable std::size_t m_iDecisionTraceWriteIndex = 0u;
		mutable std::size_t m_iDecisionTraceCount = 0u;
		mutable std::uint64_t m_iNextDecisionTraceSequence = 1u;
	};
}
