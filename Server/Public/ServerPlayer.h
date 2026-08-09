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
	struct SERVER_TRIGGER_MOVE
	{
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		float fArcHeight = 0.f;
		bool isActive = false;
	};

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
		std::uint32_t iLastReviveSequence = 0;
		std::uint32_t iLastClassChangeSequence = 0;
		float fMoveGoalX = 0.f;
		float fMoveGoalZ = 0.f;
		float fMoveSpeed = 6.f;
		bool hasMoveGoal = false;
		// Valtan cannot acquire or damage this player until the server accepts the
		// first valid move/skill intent after entry or revive.
		bool isCombatReady = true;
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
		// The class identity gauge, and the same fixed-point carry the resource
		// pool uses. Both stay 0 for a class whose profile has no gauge.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityAccumulator = 0;
		LostArk::Shared::PLAYER_ACTION_STATE eAction =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		LostArk::Shared::SKILL_ID iCurrentSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;
		SERVER_TRIGGER_MOVE TriggerMove;
		std::uint32_t iLastSkillSequence = 0;
		float fActionElapsedSeconds = 0.f;
		float fSkillAimDirectionX = 0.f;
		float fSkillAimDirectionZ = 1.f;
		bool hasAppliedSkillDamage = false;
		// 1-based while a combo action runs, 0 otherwise.
		std::uint8_t iComboStage = 0;
		// Set by a press inside the open window, consumed when the stage ends.
		bool hasBufferedComboInput = false;
		// The aim that press carried. The next stage turns to it, so a combo
		// follows the cursor instead of repeating the first stage's facing.
		float fBufferedComboAimX = 0.f;
		float fBufferedComboAimZ = 1.f;
		// Set when a HOLD skill's key is let go, consumed when its loop ends.
		bool hasReleasedHold = false;
		std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t>
			CooldownEndTickBySkillId;
	};
}
