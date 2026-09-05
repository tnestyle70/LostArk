#pragma once

#include "ServerWorldEntity.h"

#include <cstdint>
#include <string>
#include <string_view>

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
		[[nodiscard]] static bool Validate_AnimationOnlyPattern(
			const BOSS_PATTERN_DEFINITION& pattern,
			std::string& status);
		[[nodiscard]] static const BOSS_PATTERN_DEFINITION*
			Find_AnimationOnlyPattern(
				const CGameplayCatalog& catalog,
				std::string_view patternId,
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
