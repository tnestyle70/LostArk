#pragma once

#include "ServerIds.h"

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"
#include "Network/PacketMessages.h"
#include "ServerNavigation.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_PLAYER
	{
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;

		std::string strNickName;
		std::string strSpawnPlacementId;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		std::uint32_t iLastMoveSequence = 0;
		float fMoveGoalX = 0.f;
		float fMoveGoalZ = 0.f;
		float fMoveSpeed = 6.f;
		bool hasMoveGoal = false;
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;

		std::uint32_t iCurrentHp = 1000;
		std::uint32_t iMaximumHp = 1000;
		std::uint32_t iCurrentResource = 100;
		std::uint32_t iMaximumResource = 100;
		// Fixed-point regen carry in ticks: gains profile regen per tick and pays
		// out one resource per SERVER_TICK_HZ accumulated, so a second restores
		// exactly resourceRegenPerSecond with integers only.
		std::uint32_t iResourceAccumulator = 0;
		LostArk::Shared::PLAYER_ACTION_STATE eAction =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		LostArk::Shared::SKILL_ID iCurrentSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iLastSkillSequence = 0;
		float fActionElapsedSeconds = 0.f;
		float fSkillAimDirectionX = 0.f;
		float fSkillAimDirectionZ = 1.f;
		bool hasAppliedSkillDamage = false;
		// 1-based while a combo action runs, 0 otherwise.
		std::uint8_t iComboStage = 0;
		// Set by a press inside the open window, consumed when the stage ends.
		bool hasBufferedComboInput = false;
		std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t>
			CooldownEndTickBySkillId;
	};
}
