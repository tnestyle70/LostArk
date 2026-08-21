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
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fFallbackDirectionX = 0.f;
		float fFallbackDirectionZ = 1.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		std::uint32_t iServerTick = 0u;
	};

	struct SERVER_WORLD_TO_PLAYER_HIT final
	{
		std::uint32_t iRawDamage = 0u;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0u;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0u;
		std::uint32_t iServerTick = 0u;
	};

	/* The two combat directions share event/death/reaction ownership here.
	   Shape tests and repeat/contact policy stay in their callers. */
	class CServerCombatHitRuntime final
	{
	public:
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
