#pragma once

#include "GameplayCatalog.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	class CPlayerSkillSystem final
	{
	public:
		bool Try_Start(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick) const;

		/* outDamageEvents collects every hit this call resolves so the room can
		ship the amounts in the same tick's snapshot. The room owns the vector's
		lifetime; a combo emits once per stage that lands. */
		void Update(
			SERVER_PLAYER& player,
			std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			const CServerNavigation* navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
	};
}
