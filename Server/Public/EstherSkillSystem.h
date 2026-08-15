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

	// Timeline the room drives on the summoned entity. The server only speaks
	// these semantic stage ids; the client maps them to clips through the
	// NpcCatalog actionClips contract.
	inline constexpr const char* ESTHER_ACTION_APPEAR = "esther.appear";
	inline constexpr const char* ESTHER_ACTION_STRIKE = "esther.strike";
	inline constexpr const char* ESTHER_ACTION_LEAVE = "esther.leave";

	inline constexpr std::uint32_t ESTHER_APPEAR_MS = 800u;
	// npc_att_battle_7_01 measures 94 frames at 30 fps.
	inline constexpr std::uint32_t ESTHER_STRIKE_MS = 3200u;
	inline constexpr std::uint32_t ESTHER_LEAVE_MS = 1500u;
	// World units per second the summon rises during the leave stage.
	inline constexpr float ESTHER_LEAVE_RISE_PER_SECOND = 12.f;

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
		On NONE the gauge has been consumed to zero and outArchetypeId names
		the summon the room must spawn. Never partially consumes. */
		[[nodiscard]] ESTHER_USE_REJECTION Try_Consume(
			std::uint8_t slotIndex,
			std::string& outArchetypeId);

	private:
		static constexpr std::uint32_t GAUGE_MAXIMUM = 1000u;
		static constexpr float REGEN_PER_SECOND = 200.f;

		bool m_isEnabled = false;
		std::uint32_t m_iGauge = 0u;
		// Sub-point remainder so a 30 Hz tick loses nothing to truncation.
		float m_fRegenRemainder = 0.f;
	};
}
