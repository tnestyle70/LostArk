#pragma once

#include "Network/PacketType.h"

#include <cstdint>
#include <string>

namespace LostArk::Server
{
	/* The raid Esther gauge and roster. The gauge is room-shared state: every
	party member sees the same value and any of them may spend it (the original
	game's party-leader restriction is intentionally absent while this is a test
	contract). Numbers live here as constants until the real charge mechanic is
	decided and promoted into a Data/Balance contract. */

	// The one semantic action the room drives on the summoned entity. The
	// server only speaks this id; the client maps it to clips through the
	// NpcCatalog actionClips contract.
	inline constexpr const char* ESTHER_ACTION_STRIKE = "esther.strike";

	/* The gauge drains the moment the use is accepted, but the summon itself
	lands after this delay, this far along the caster's aim. The landing spot
	is sampled on the navigation grid; an unwalkable target falls back to the
	caster's feet so a cliff edge never loses the summon. */
	inline constexpr float ESTHER_SUMMON_DELAY_SECONDS = 1.f;
	inline constexpr float ESTHER_SUMMON_FORWARD_METERS = 2.f;

	/* The caster's call animation (act_estherskill_1, 45 frames at 30 fps) is
	the same length for every class, so the ESTHER_CAST action holds exactly
	this long before the room returns the player to NONE. */
	inline constexpr std::uint32_t ESTHER_CAST_DURATION_MS = 1500u;

	/* Valtan roster order is Sillian, Wei, Bahuntur. Every summon's authored
	clip carries its own entrance and exit (Sillian
	npc_evt1_sk_swordofchampion_bk 157 frames at 30 fps, Wei npc_sk_dochul 213,
	Bahuntur npc_sk_breathofarcturus 121), so the summon spawns straight into
	the strike and despawns the moment its clip ends. */
	struct ESTHER_ROSTER_ENTRY
	{
		std::uint8_t iSlotIndex;
		const char* pArchetypeId;
		std::uint32_t iStrikeMs;
	};
	inline constexpr ESTHER_ROSTER_ENTRY ESTHER_ROSTER[] =
	{
		{ 1u, "NPC_59030", 5300u },
		{ 2u, "NPC_58700", 7100u },
		{ 3u, "NPC_59060", 4100u },
	};

	enum class ESTHER_USE_REJECTION : std::uint8_t
	{
		NONE,
		DISABLED_WORLD,
		UNSUPPORTED_SLOT,
		GAUGE_NOT_FULL
	};

	class CEstherSkillSystem final
	{
	public:
		// Only a raid room owns an Esther roster. Every other world keeps the
		// maximum at 0, which the snapshot contract reads as "no Esther here".
		void Initialize(LostArk::Shared::WORLD_ID worldId);

		[[nodiscard]] bool Is_Enabled() const { return m_isEnabled; }
		[[nodiscard]] std::uint32_t Get_Gauge() const { return m_iGauge; }
		[[nodiscard]] std::uint32_t Get_GaugeMaximum() const
		{
			return m_isEnabled ? GAUGE_MAXIMUM : 0u;
		}

		// Test-rate regeneration. Only ticks while the room has players so an
		// empty arena re-arms from zero for the next party.
		void Update(float fixedDeltaSeconds, bool hasPlayers);
		void Reset();

		/* Validates a slot against the world roster and the full-gauge rule.
		On NONE the gauge has been consumed to zero and outEntry names the
		roster row (summon archetype and stage timeline) the room must spawn.
		Never partially consumes. */
		[[nodiscard]] ESTHER_USE_REJECTION Try_Consume(
			std::uint8_t slotIndex,
			const ESTHER_ROSTER_ENTRY*& outEntry);

	private:
		static constexpr std::uint32_t GAUGE_MAXIMUM = 1000u;
		static constexpr float REGEN_PER_SECOND = 200.f;

		bool m_isEnabled = false;
		std::uint32_t m_iGauge = 0u;
		// Sub-point remainder so a 30 Hz tick loses nothing to truncation.
		float m_fRegenRemainder = 0.f;
	};
}
