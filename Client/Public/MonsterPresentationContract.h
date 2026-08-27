#pragma once

#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>

namespace Client
{
	enum class MONSTER_PRESENTATION_ACTION_KIND : std::uint8_t
	{
		IDLE,
		CHASE,
		ATTACK,
		DEAD
	};

	struct MONSTER_PRESENTATION_ACTION_STATE final
	{
		MONSTER_PRESENTATION_ACTION_KIND eLastKind =
			MONSTER_PRESENTATION_ACTION_KIND::IDLE;
		std::uint32_t iLastOccurrenceStartTick = 0u;
		std::uint32_t iAttackOccurrenceStartTick = 0u;
		bool hasProjectedAction = false;
	};

	struct MONSTER_PRESENTATION_ACTION_FRAME final
	{
		MONSTER_PRESENTATION_ACTION_KIND eKind =
			MONSTER_PRESENTATION_ACTION_KIND::IDLE;
		std::uint32_t iOccurrenceStartTick = 0u;
		bool shouldRestartClip = false;
		bool isLoop = true;
	};

	/* Server exposes WINDUP/ACTIVE/RECOVERY as separate simulation states, but
	all three present one authored attack clip. WINDUP owns the occurrence edge;
	ACTIVE and RECOVERY keep that token so snapshots cannot restart the clip.
	A late join that first observes ACTIVE still starts the clip exactly once. */
	class CMonsterPresentationContract final
	{
	public:
		static std::size_t Select_AttackPresentation(
			LostArk::Shared::NET_ENTITY_ID entityId,
			std::uint32_t occurrenceStartTick,
			std::size_t presentationCount) noexcept
		{
			if (0u == presentationCount)
				return 0u;
			const std::uint64_t seed =
				static_cast<std::uint64_t>(entityId) * 0x9E3779B185EBCA87ull +
				static_cast<std::uint64_t>(occurrenceStartTick);
			return static_cast<std::size_t>(seed % presentationCount);
		}

		static MONSTER_PRESENTATION_ACTION_FRAME Project(
			LostArk::Shared::WORLD_ENTITY_ACTION action,
			std::uint32_t actionStartTick,
			MONSTER_PRESENTATION_ACTION_STATE& state) noexcept
		{
			MONSTER_PRESENTATION_ACTION_FRAME frame{};
			switch (action)
			{
			case LostArk::Shared::WORLD_ENTITY_ACTION::CHASE:
				frame.eKind = MONSTER_PRESENTATION_ACTION_KIND::CHASE;
				break;
			case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_WINDUP:
			case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_ACTIVE:
			case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_RECOVERY:
				frame.eKind = MONSTER_PRESENTATION_ACTION_KIND::ATTACK;
				frame.isLoop = false;
				break;
			case LostArk::Shared::WORLD_ENTITY_ACTION::DEAD:
				frame.eKind = MONSTER_PRESENTATION_ACTION_KIND::DEAD;
				frame.isLoop = false;
				break;
			default:
				frame.eKind = MONSTER_PRESENTATION_ACTION_KIND::IDLE;
				break;
			}

			if (MONSTER_PRESENTATION_ACTION_KIND::ATTACK == frame.eKind)
			{
				const bool beginsOccurrence =
					LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_WINDUP == action ||
					!state.hasProjectedAction ||
					MONSTER_PRESENTATION_ACTION_KIND::ATTACK != state.eLastKind ||
					0u == state.iAttackOccurrenceStartTick;
				if (beginsOccurrence)
					state.iAttackOccurrenceStartTick = actionStartTick;
				frame.iOccurrenceStartTick = state.iAttackOccurrenceStartTick;
			}
			else
			{
				state.iAttackOccurrenceStartTick = 0u;
				frame.iOccurrenceStartTick = actionStartTick;
			}

			frame.shouldRestartClip = !state.hasProjectedAction ||
				state.eLastKind != frame.eKind ||
				state.iLastOccurrenceStartTick != frame.iOccurrenceStartTick;
			state.eLastKind = frame.eKind;
			state.iLastOccurrenceStartTick = frame.iOccurrenceStartTick;
			state.hasProjectedAction = true;
			return frame;
		}
	};
}
