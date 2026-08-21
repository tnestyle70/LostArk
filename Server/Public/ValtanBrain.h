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
		bool Try_BuildStageMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
	};
}
