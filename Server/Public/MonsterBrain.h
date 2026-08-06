#pragma once

#include "GameplayCatalog.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <map>

namespace LostArk::Server
{
	class CMonsterBrain final
	{
	public:
		void Update(SERVER_WORLD_ENTITY& monster,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;
	};
}
