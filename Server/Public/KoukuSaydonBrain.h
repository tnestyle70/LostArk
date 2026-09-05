#pragma once

#include "ServerWorldEntity.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace LostArk::Server
{
	inline constexpr std::string_view KOUKUSAYDON_G1_ENCOUNTER_ID =
		"ENCOUNTER_KAKULSAYDON_G1";
	inline constexpr std::string_view KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	inline constexpr std::string_view KOUKUSAYDON_G1_BOSS_PLACEMENT_ID =
		"boss.kakulsaydon.g1.kouku";
	inline constexpr std::string_view KOUKUSAYDON_G1_PLAY_ALL_SEQUENCE_ID =
		"KAKULSAYDON_G1_PLAY_ALL";
	/* Every boss archetype the KoukuSaydon arena may host shares this prefix.
	The Debug gate buttons activate disabled placements of these archetypes;
	they all join the Gate 1 encounter for profile and pattern lookup. */
	inline constexpr std::string_view KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX =
		"BOSS_KAKULSAYDON_";

	enum class KOUKUSAYDON_BRAIN_UPDATE_RESULT : std::uint8_t
	{
		IDLE,
		RUNNING,
		STAGE_CHANGED,
		PATTERN_COMPLETED,
		ABORTED_INVALID_DEFINITION,
		ABORTED_BOSS_DEAD
	};

	// Animation-only Gate 1 runtime. It consumes the generic pattern/stage
	// schema but owns no Valtan selection, combat, movement, or arena state.
	class CKoukuSaydonBrain final
	{
	public:
		[[nodiscard]] static bool Is_GateOneBoss(
			LostArk::Shared::WORLD_ID worldId,
			const SERVER_WORLD_ENTITY& boss) noexcept;
		/* Any primary boss of the KoukuSaydon arena, the Gate 1 Kouku included.
		A boss that is an arena boss but not the Gate 1 boss owns no brain: it
		waits in place until a later audition contract names it, so activating
		it from the Debug gate buttons can never start a pattern by itself. */
		[[nodiscard]] static bool Is_ArenaBoss(
			LostArk::Shared::WORLD_ID worldId,
			const SERVER_WORLD_ENTITY& boss) noexcept;
		/* Placement form of Is_ArenaBoss for the Debug spawn command: a disabled
		BOSS placement of the Gate 1 encounter with an arena boss archetype. The
		statically enabled Gate 1 Kouku is deliberately not one. */
		[[nodiscard]] static bool Is_ArenaBossPlacement(
			LostArk::Shared::WORLD_ID worldId,
			const WORLD_BOOTSTRAP_PLACEMENT& placement) noexcept;
		[[nodiscard]] static bool Validate_AnimationOnlyPattern(
			const BOSS_PATTERN_DEFINITION& pattern,
			std::string& status);
		[[nodiscard]] static const BOSS_PATTERN_DEFINITION*
			Find_AnimationOnlyPattern(
				const CGameplayCatalog& catalog,
				std::string_view patternId,
				std::string& status);
		/* Keep the authored order for this body. The transition after a retained
		pattern stays with that pattern; skipped bodies contribute no playback.
		Failure preserves the caller's previous selection. */
		[[nodiscard]] static bool Select_AnimationOnlySequence(
			const std::vector<BOSS_PATTERN_DEFINITION>& definitions,
			const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
			std::string_view bossArchetypeId,
			std::vector<std::string>& outPatternIds,
			std::vector<std::uint32_t>& outTransitionTicks,
			std::string& status);
		[[nodiscard]] static std::uint32_t Resolve_ProductSourceRevision(
			const CGameplayCatalog& catalog) noexcept;
		[[nodiscard]] bool Begin_Pattern(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			const LostArk::Shared::GameplayDataRevision& revision,
			std::uint32_t serverTick,
			std::string& status) const;
		[[nodiscard]] KOUKUSAYDON_BRAIN_UPDATE_RESULT Update(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick,
			std::string& status) const;
		void Abort_Pattern(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick) const;

	private:
		static void Enter_Stage(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			std::uint32_t stageIndex,
			std::uint32_t serverTick,
			bool evaluatesOnEntryTick);
		static void Finish_Pattern(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick,
			SERVER_BOSS_PATTERN_TERMINAL_RESULT result);
	};
}
