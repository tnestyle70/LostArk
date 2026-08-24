#pragma once

#include "GameplayCatalog.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <map>
#include <vector>

namespace LostArk::Server
{
	class CMonsterBrain final
	{
	public:
		/* worldEntities is the room's own live list, read only to push this
		monster out of a body it overlaps. The caller iterates that same list, so
		the brain must not spawn, despawn or reorder anything in it. */
		void Update(SERVER_WORLD_ENTITY& monster,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;

		/* One fixed tick of a player-hit knockback: slides the living monster
		along its stored direction, clamped to walkable floor, and burns the
		remaining window. Returns true when the tick was consumed, in which
		case the caller skips Update so the chase step cannot cancel the push.
		A clamp against a wall ends the window early. */
		static bool Advance_Knockback(
			SERVER_WORLD_ENTITY& monster,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds);
	};
}
