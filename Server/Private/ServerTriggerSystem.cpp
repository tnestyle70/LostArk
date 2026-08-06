#include "ServerTriggerSystem.h"

#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
}

bool LostArk::Server::CServerTriggerSystem::Initialize(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	std::string& outStatus)
{
	std::vector<RUNTIME_TRIGGER> staged;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != placement.eKind ||
			!placement.isEnabled)
		{
			continue;
		}
		if (1u != placement.TriggerActions.size() ||
			(WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL !=
				placement.TriggerActions.front().eKind))
		{
			outStatus = "Enabled trigger requires one supported action: " +
				placement.strPlacementId;
			return false;
		}
		staged.push_back({ placement });
	}
	m_Triggers = std::move(staged);
	outStatus = "Initialized server triggers: " +
		std::to_string(m_Triggers.size());
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Update_PlayerMotion(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction)
	{
		return false;
	}
	if (!player.TriggerMove.isActive ||
		player.TriggerMove.fDurationSeconds <= 0.f)
	{
		player.TriggerMove = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
		return false;
	}
	if (0u == player.iCurrentHp)
	{
		player.TriggerMove = {};
		return false;
	}

	SERVER_TRIGGER_MOVE& move = player.TriggerMove;
	move.fElapsedSeconds = (std::min)(
		move.fDurationSeconds,
		move.fElapsedSeconds + fixedDeltaSeconds);
	const float ratio = move.fElapsedSeconds / move.fDurationSeconds;
	player.fPositionX = move.fStartX +
		(move.fTargetX - move.fStartX) * ratio;
	player.fPositionY = move.fStartY +
		(move.fTargetY - move.fStartY) * ratio +
		4.f * move.fArcHeight * ratio * (1.f - ratio);
	player.fPositionZ = move.fStartZ +
		(move.fTargetZ - move.fStartZ) * ratio;

	if (ratio >= 1.f)
	{
		player.fPositionX = move.fTargetX;
		player.fPositionY = move.fTargetY;
		player.fPositionZ = move.fTargetZ;
		move = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
	}
	return true;
}

void LostArk::Server::CServerTriggerSystem::Evaluate_Entries(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers)
{
	outTransfers.clear();
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		std::unordered_set<LostArk::Shared::PLAYER_ID> currentInside;
		for (auto& [playerId, player] : players)
		{
			if (0u == player.iCurrentHp || !Contains(trigger, player))
				continue;
			currentInside.insert(playerId);
			if (trigger.PlayersInside.contains(playerId) ||
				(trigger.Definition.isTriggerOnce && trigger.hasFired))
			{
				continue;
			}
			const WORLD_TRIGGER_ACTION& action =
				trigger.Definition.TriggerActions.front();
			bool fired = false;
			if (WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == action.eKind)
			{
				fired = Begin_MovePlayer(player, action, actionStartTick);
			}
			else if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL == action.eKind)
			{
				SERVER_WORLD_TRANSFER_REQUEST transfer{};
				fired = Build_WorldTransfer(player, action, transfer);
				if (fired)
					outTransfers.push_back(std::move(transfer));
			}
			if (fired && trigger.Definition.isTriggerOnce)
			{
				trigger.hasFired = true;
			}
		}
		trigger.PlayersInside = std::move(currentInside);
	}
}

void LostArk::Server::CServerTriggerSystem::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
		trigger.PlayersInside.erase(playerId);
}

bool LostArk::Server::CServerTriggerSystem::Contains(
	const RUNTIME_TRIGGER& trigger,
	const SERVER_PLAYER& player)
{
	const WORLD_BOOTSTRAP_PLACEMENT& box = trigger.Definition;
	const float deltaX = player.fPositionX - box.fPositionX;
	const float deltaZ = player.fPositionZ - box.fPositionZ;
	const float yaw = box.fYawDegrees * DEGREES_TO_RADIANS;
	const float cosine = std::cos(yaw);
	const float sine = std::sin(yaw);
	const float localX = cosine * deltaX - sine * deltaZ;
	const float localZ = sine * deltaX + cosine * deltaZ;
	const float playerCenterY = player.fPositionY +
		LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
	return std::abs(localX) <= box.fHalfExtentX +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X &&
		std::abs(playerCenterY - box.fPositionY) <= box.fHalfExtentY +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y &&
		std::abs(localZ) <= box.fHalfExtentZ +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Z;
}

bool LostArk::Server::CServerTriggerSystem::Begin_MovePlayer(
	SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	const std::uint32_t actionStartTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp || 0u == actionStartTick ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != action.eKind)
	{
		return false;
	}

	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0;
	player.hasBufferedComboInput = false;
	player.TriggerMove.fStartX = player.fPositionX;
	player.TriggerMove.fStartY = player.fPositionY;
	player.TriggerMove.fStartZ = player.fPositionZ;
	player.TriggerMove.fTargetX = action.fTargetX;
	player.TriggerMove.fTargetY = action.fTargetY;
	player.TriggerMove.fTargetZ = action.fTargetZ;
	player.TriggerMove.fDurationSeconds = action.fDurationSeconds;
	player.TriggerMove.fElapsedSeconds = 0.f;
	player.TriggerMove.fArcHeight = action.fArcHeight;
	player.TriggerMove.isActive = true;
	const float deltaX = action.fTargetX - player.fPositionX;
	const float deltaZ = action.fTargetZ - player.fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ > 0.000001f)
		player.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
	player.iActionStartTick = actionStartTick;
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Build_WorldTransfer(
	const SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	using namespace LostArk::Shared;
	if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL != action.eKind ||
		(WORLD_ID::BERN != action.eTargetWorldId &&
			WORLD_ID::VALTAN_ARENA != action.eTargetWorldId) ||
		INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty() || 0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		return false;
	}

	outTransfer.iSessionId = player.iSessionId;
	outTransfer.eTargetWorldId = action.eTargetWorldId;
	outTransfer.eCharacterClass = player.eCharacterClass;
	outTransfer.strNickName = player.strNickName;
	return true;
}
