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
			std::uint32_t actionStartTick,
			const CServerNavigation* navigation = nullptr) const;

		/* A different skill pressed during a COMBO consumes its sequence and is
		copied as the one pending explicit intent. Costs/cooldown are rechecked only
		when a manual stage or the complete automatic chain commits it. */
		bool Try_StagePendingSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			const CServerNavigation* navigation = nullptr) const;

		bool Try_StartPending(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			const CServerNavigation* navigation = nullptr) const;

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
		static bool Can_ArmPlayerHitReaction(const SERVER_PLAYER& player, std::uint32_t serverTick, bool forcePush = false);
		static void Arm_PlayerHitReaction(
			SERVER_PLAYER& player,
			float sourceX,
			float sourceZ,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick,
			bool forcePush = false, bool pushCanLeaveArena = false, bool pushBallistic = false);

		/* A stance-setting skill's authored cancel windows open after the swap has
		already happened on screen, so every exit from its action commits the
		stance: the natural end, a move cancel and a skill cancel alike. Without
		this the last stretch of the action silently loses the swap. Skills that
		set no stance are a no-op here. */
		static void Commit_StanceChange(
			SERVER_PLAYER& player,
			const PLAYER_SKILL_DEFINITION& skill,
			const CGameplayCatalog& catalog);

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

		/* Guardian Knight orb gauge in place of Update_Identity: it never
		regenerates on its own, only landed hits fill it (Gain_EmberGauge), and
		the dragon stance runs it from full to empty over the profile duration
		and drops the stance at zero. */
		static void Update_EmberGauge(
			SERVER_PLAYER& player,
			const PLAYER_RUNTIME_PROFILE& profile,
			const GUARDIAN_EMBER_PROFILE& ember);

		/* Spawn, revive and class change put every gauge where the class starts:
		an ember class starts empty with a full, unlocked ember pool, every other
		class starts with its identity gauge full. */
		static void Reset_Gauges(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog);

		/* One landed hit of a default-stance action adds the profile gain. */
		static void Gain_EmberGauge(
			SERVER_PLAYER& player,
			const CGameplayCatalog& catalog);

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
		/* The ember side of a successful Try_Start: spend what the skill asks
		and the player holds, lock a socket for a human-form expression skill,
		then refill. Skills of other classes leave the player untouched. */
		static void Apply_EmberOnStart(
			SERVER_PLAYER& player,
			const PLAYER_SKILL_DEFINITION& skill,
			const CGameplayCatalog& catalog);
		bool Try_StartInternal(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command,
			const CGameplayCatalog& catalog,
			std::uint32_t actionStartTick,
			bool sequenceAlreadyConsumed,
			const CServerNavigation* navigation) const;
	};
}
