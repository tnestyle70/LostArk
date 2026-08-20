#pragma once

#include "GameplayCatalog.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <map>

namespace LostArk::Server
{
	class CValtanBrain final
	{
	public:
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

		/* outDamageEvents collects the hits this pattern tick lands on players so
		the room can ship the amounts in the same tick's snapshot. */
		void Update(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
		bool Try_BuildImpactMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		bool Complete_ImpactStage(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;
	};
}
