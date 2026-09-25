#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <cstdint>
#include <vector>

namespace LostArk::Server
{
	enum class SERVER_COMBAT_HIT_RESULT : std::uint8_t
	{
		NOT_ADMITTED,
		ABSORBED,
		LANDED,
		KILLED
	};

	struct SERVER_PLAYER_TO_WORLD_HIT final
	{
		LostArk::Shared::PLAYER_ID iSourcePlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iRawDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iCounterPower = 0u;
		/* The caster rolled a critical hit for this one hit. iRawDamage already
		carries the critical multiplier; this only colours the damage event. */
		bool bCritical = false;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fFallbackDirectionX = 0.f;
		float fFallbackDirectionZ = 1.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		std::uint32_t iServerTick = 0u;
		// A typed boss health-bar amount bypasses armor, but not shields or immunity.
		bool bHealthDamagePreResolved = false;
	};

	struct SERVER_WORLD_TO_PLAYER_HIT final
	{
		std::uint32_t iRawDamage = 0u;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		bool bUsePushDirection = false;
		bool bForcePush = false;
		bool bPushCanLeaveArena = false;
		bool bPushBallistic = false;
		float fPushHeightM = 0.f;
		float fPushDirectionX = 0.f, fPushDirectionZ = 1.f;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0u;
		std::uint32_t iServerTick = 0u;
		/* Typed mechanic verdicts (instant death, max-HP percent) are authored
		as the final amount, so neither defense nor a counter skill may soften
		them. Ordinary boss hits leave both false. */
		bool bIgnoreDefense = false;
		bool bIgnoreCounter = false;
		// Server encounter failure verdict; bypasses all personal damage protection.
		bool bEncounterWipe = false;
	};

	// Mario hazards share the normal authoritative knockdown/landing path.
	inline void Configure_MarioHazardLaunch(SERVER_WORLD_TO_PLAYER_HIT& hit)
	{
		if (hit.fPushRangeM <= 0.f || !hit.iPushMs) return;
		hit.bKnockdown = hit.bForcePush = hit.bPushBallistic = true;
		hit.bPushCanLeaveArena = false;
		hit.fPushHeightM = 2.f;
		if (hit.iDownMs < hit.iPushMs + 600u) hit.iDownMs = hit.iPushMs + 600u;
	}

	/* Buffs live next to the two damage directions because that is where they are
	read: a holder's buffs scale the damage it deals and the damage it takes. */
	class CServerBuffRuntime final
	{
	public:
		/* Grants every buff the skill authors, to the caster, its allies or the
		entity it hit, and replaces an older cast of the same buff. */
		static void Apply_SkillBuffs(
			const CGameplayCatalog& catalog,
			std::uint32_t skillId,
			SERVER_PLAYER& caster,
			std::vector<SERVER_PLAYER*>& allies,
			std::vector<SERVER_WORLD_ENTITY*>& enemies,
			std::uint32_t serverTick);
		/* Drops what has run out. Called once per tick for every holder. */
		static void Expire(
			std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs,
			std::uint32_t serverTick);
		/* Percent the holder's outgoing damage is raised by, summed over buffs. */
		[[nodiscard]] static std::int32_t Damage_DealtPercent(
			const CGameplayCatalog& catalog,
			const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs);
		/* Percent the holder's incoming damage changes by: negative mitigates. */
		[[nodiscard]] static std::int32_t Damage_TakenPercent(
			const CGameplayCatalog& catalog,
			const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs);
		/* Applies a summed percent to one damage amount, never below 1. */
		[[nodiscard]] static std::uint32_t Scale_Damage(
			std::uint32_t damage, std::int32_t percent);
		/* Drops the shield pool once no buff grants one any more. */
		static void Settle_Shield(
			const CGameplayCatalog& catalog, SERVER_PLAYER& player);
	};

	/* The two combat directions share event/death/reaction ownership here.
	   Shape tests and repeat/contact policy stay in their callers. */
	class CServerCombatHitRuntime final
	{
	public:
		[[nodiscard]] static bool Is_PlayerDamageableWorldTarget(
			const SERVER_WORLD_ENTITY& target) noexcept;
		static void Add_MadnessGauge(SERVER_PLAYER& target, double gain);
		static SERVER_COMBAT_HIT_RESULT Apply_PlayerToWorld(
			SERVER_WORLD_ENTITY& target,
			const SERVER_PLAYER_TO_WORLD_HIT& hit,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);
		static SERVER_COMBAT_HIT_RESULT Apply_WorldToPlayer(
			SERVER_PLAYER& target,
			const SERVER_WORLD_TO_PLAYER_HIT& hit,
			const CGameplayCatalog& catalog,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);
	};
}
