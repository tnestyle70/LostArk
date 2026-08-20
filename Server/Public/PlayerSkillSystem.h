#pragma once

#include "GameplayCatalog.h"
#include "ServerCollisionSystem.h"
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

		/* A different skill pressed during a COMBO consumes its sequence and is
		copied as the one pending explicit intent. Costs/cooldown are rechecked only
		when the stage boundary commits it. */
		bool Try_StagePendingSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog) const;

		bool Try_StartPending(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick) const;

		void Release(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RELEASE_SKILL& command,
			const CGameplayCatalog& catalog);

		/* A HOLD skill keeps accepting a new aim while it charges (stage 1 and 2,
		before the key is released); the firing stage keeps the last direction. */
		void Update_Aim(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& command,
			const CGameplayCatalog& catalog) const;

		/* Every source of incoming player damage asks this first. A COUNTER skill
		guarding inside its window absorbs the hit and promotes to its counter
		stage, and the caller must then skip the damage it was about to apply. */
		[[nodiscard]] static bool Try_Counter(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);

		/* Every source of landed player damage arms the authored hit reaction
		here after applying its damage: a push window away from (or, for a
		negative range, toward) the hit source, and optionally KNOCKDOWN until
		downMs expires. A window or knockdown already running keeps the new hit
		from re-arming, and DEAD or TRIGGER_MOVE players are never armed. */
		static void Arm_PlayerHitReaction(
			SERVER_PLAYER& player,
			float sourceX,
			float sourceZ,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
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
			const CServerCollisionSystem* collision,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const;

		/* Advances every object the player's skills spawned: moves it, fires
		its contact and timed hits, and drops it once its distance or life is
		spent. Runs even when the player has no action, since a missile outlives
		the clip that threw it. */
		static void Update_Projectiles(
			SERVER_PLAYER& player,
			std::vector<SERVER_WORLD_ENTITY>& worldEntities,
			const CGameplayCatalog& catalog,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);

		/* Root motion advances by clip delta, so it has to answer the same
		question a walk step does: how far along this tick's displacement can the
		player actually stand. Returns the reachable point, which is the start
		itself when the very first sample off the start is already blocked. */
		static void Clamp_StepToWalkable(
			const CServerNavigation& navigation,
			float startX,
			float startZ,
			float desiredX,
			float desiredZ,
			SERVER_NAV_POINT& outPoint,
			bool& outWasClamped);

	private:
		bool Try_StartInternal(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			bool sequenceAlreadyConsumed) const;
	};
}
