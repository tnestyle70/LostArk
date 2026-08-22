#pragma once

#include "GameplayCatalog.h"
#include "Network/NetworkIds.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_WORLD_ENTITY;

	enum class SERVER_BOSS_COMBAT_FLAG : std::uint32_t
	{
		INVULNERABLE = 1u << 0u,
		SHIELDED = 1u << 1u,
		COUNTERABLE = 1u << 2u,
		GROGGY = 1u << 3u
	};

	struct SERVER_BOSS_PART_STATE final
	{
		std::string strPartId;
		std::uint32_t iStateMask = 0u;
		std::uint32_t iCurrentDurability = 0u;
		std::uint32_t iMaximumDurability = 0u;
		std::uint32_t iDamageReductionPercent = 0u;
		BOSS_PART_DAMAGE_CONDITION eDamageCondition =
			BOSS_PART_DAMAGE_CONDITION::ALWAYS;
	};

	struct SERVER_BOSS_PATTERN_OUTCOME_SIGNAL final
	{
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iServerTick = 0u;
		BOSS_PATTERN_STAGE_OUTCOME eOutcome =
			BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
	};

	/* A part break is persistent state plus a one-shot presentation edge. The
	Room drains this queue into a bounded snapshot vector only after assigning
	its room-global event sequence, so a hit never loses the edge merely because
	the snapshot is assembled by a later system. */
	struct SERVER_BOSS_PART_BREAK_EDGE final
	{
		std::uint32_t iPartMask = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iStateRevision = 0u;
	};

	struct SERVER_BOSS_COMBAT_STATE final
	{
		std::uint32_t iStateRevision = 0u;
		std::uint32_t iFlags = 0u;
		std::uint32_t iAlivePartMask = 0u;
		std::uint32_t iStaggerCurrent = 0u;
		std::uint32_t iStaggerMaximum = 0u;
		std::uint32_t iShieldCurrent = 0u;
		std::uint32_t iShieldMaximum = 0u;
		std::vector<SERVER_BOSS_PART_STATE> Parts;
		std::vector<SERVER_BOSS_PATTERN_OUTCOME_SIGNAL> PendingOutcomes;
		std::vector<SERVER_BOSS_PART_BREAK_EDGE> PendingPartBreakEdges;
	};

	struct BOSS_INCOMING_HIT final
	{
		LostArk::Shared::PLAYER_ID iSourcePlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iRawDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iCounterPower = 0u;
		std::uint32_t iServerTick = 0u;
		float fSourceX = 0.f;
		float fSourceZ = 0.f;
	};

	struct BOSS_HIT_RESULT final
	{
		std::uint32_t iHealthDamage = 0u;
		std::uint32_t iShieldDamage = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iDestroyedPartMask = 0u;
		std::uint32_t iAlivePartMask = 0u;
		std::string strDestroyedPartId;
		bool bBlockedByInvulnerability = false;
		bool bCounterTriggered = false;
		bool bStaggerBroken = false;
		bool bPartDestroyed = false;
	};

	class CBossCombatRuntime final
	{
	public:
		static bool Initialize(
			SERVER_BOSS_COMBAT_STATE& state,
			const std::vector<BOSS_PART_DEFINITION>& definitions,
			std::string& status);
		static BOSS_HIT_RESULT Apply_PlayerHit(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_INCOMING_HIT& hit);
		static bool Publish_PatternOutcome(
			SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_STAGE_OUTCOME outcome,
			std::uint32_t serverTick);
		static bool Consume_PatternOutcome(
			SERVER_WORLD_ENTITY& boss,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_OUTCOME outcome);
		static void Discard_PatternOutcomes(
			SERVER_WORLD_ENTITY& boss,
			const std::string& actionId);
		static void Clear_PatternOutcomes(SERVER_WORLD_ENTITY& boss);
		static bool Has_Flag(
			const SERVER_BOSS_COMBAT_STATE& state,
			SERVER_BOSS_COMBAT_FLAG flag) noexcept;
		static bool Set_Flag(
			SERVER_BOSS_COMBAT_STATE& state,
			SERVER_BOSS_COMBAT_FLAG flag,
			bool enabled) noexcept;
		/* A nonzero value opens a fresh gauge/window at zero or full respectively;
		zero closes it. These functions own current/maximum/flag invariants so a
		stage action cannot expose a half-configured snapshot. */
		static bool Set_StaggerGauge(
			SERVER_BOSS_COMBAT_STATE& state,
			std::uint32_t maximum) noexcept;
		static bool Set_Shield(
			SERVER_BOSS_COMBAT_STATE& state,
			std::uint32_t maximum) noexcept;
		/* The gameplay phase lives on the entity but ships inside the boss
		combat snapshot, so the revision that guards that snapshot has to move
		with it. A client that sees the same revision is entitled to assume the
		same contents and rejects the frame otherwise. */
		static bool Set_GameplayPhase(
			SERVER_WORLD_ENTITY& boss,
			std::uint8_t phase) noexcept;
	};
}
