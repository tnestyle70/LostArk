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

		void Release(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RELEASE_SKILL& command,
			const CGameplayCatalog& catalog);

		/* Every source of incoming player damage asks this first. A COUNTER skill
		guarding inside its window absorbs the hit and promotes to its counter
		stage, and the caller must then skip the damage it was about to apply. */
		[[nodiscard]] static bool Try_Counter(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);

		/* True while the player stands in a stance the identity gauge is paying
		for, which is any stance other than the class default on a class that has
		a gauge. Movement and the drain both key off this one answer. */
		[[nodiscard]] static bool Is_HoldingGaugedStance(
			const SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile);

		/* Runs every tick regardless of what the player is doing: the gauge is
		spent by standing, not by acting. */
		static void Update_Identity(
			SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile);

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
