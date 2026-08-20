#include "ServerTriggerSystem.h"

#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace
{
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
}

bool LostArk::Server::CServerTriggerSystem::Initialize(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	std::string& outStatus,
	const bool enableDebugValtanStageBypass)
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
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER !=
				placement.TriggerActions.front().eKind))
		{
			outStatus = "Enabled trigger requires one supported action: " +
				placement.strPlacementId;
			return false;
		}
		staged.push_back({ placement });
	}
	m_Triggers = std::move(staged);
#ifdef _DEBUG
	m_bDebugValtanStageBypass = enableDebugValtanStageBypass;
#else
	(void)enableDebugValtanStageBypass;
	m_bDebugValtanStageBypass = false;
#endif
	outStatus = "Initialized server triggers: " +
		std::to_string(m_Triggers.size()) +
		(m_bDebugValtanStageBypass ? ", ValtanStageBypass=1" : "");
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
		player.PendingCommand.Clear();
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
		player.PendingCommand.Clear();
	}
	return true;
}

void LostArk::Server::CServerTriggerSystem::Evaluate_Entries(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget)
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
#ifdef _DEBUG
			WORLD_TRIGGER_ACTION bypassMove{};
			if (m_bDebugValtanStageBypass &&
				"Stage_Boss" != trigger.Definition.strPlacementId &&
				Build_ValtanStageBypassMove(
					trigger.Definition.strPlacementId, bypassMove))
			{
				fired = Begin_MovePlayer(player, bypassMove, actionStartTick);
			}
			else
#endif
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
			else if ((WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == action.eKind ||
				WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == action.eKind) &&
				activateTarget)
			{
				fired = activateTarget(action.eKind, action.strTargetId);
			}
#ifdef _DEBUG
			/* Stage_Boss keeps its real activateEncounter action, then places the
			player at the authored 159-bar wall-charge bait point before the room's
			boss update runs in this same tick. */
			if (fired && m_bDebugValtanStageBypass &&
				"Stage_Boss" == trigger.Definition.strPlacementId &&
				Place_PlayerAtValtanAuditionBait(player, actionStartTick))
			{
				/* Placement is completed inside the typed helper. */
			}
#endif
			if (fired && trigger.Definition.isTriggerOnce)
			{
				trigger.hasFired = true;
			}
		}
		trigger.PlayersInside = std::move(currentInside);
	}
}

#ifdef _DEBUG
bool LostArk::Server::CServerTriggerSystem::Place_PlayerAtValtanAuditionBait(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick) const
{
	WORLD_TRIGGER_ACTION move{};
	if (!Build_ValtanStageBypassMove("Stage_Boss", move) ||
		!Begin_MovePlayer(player, move, actionStartTick))
	{
		return false;
	}
	return Update_PlayerMotion(player, move.fDurationSeconds) &&
		LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction;
}

bool LostArk::Server::CServerTriggerSystem::Build_ValtanStageBypassMove(
	const std::string& triggerPlacementId,
	WORLD_TRIGGER_ACTION& outAction)
{
	/* These destinations stop just before the next authored trigger. The player
	still walks into every next stage deliberately, while the long blocked route
	and its unkillable audition monsters no longer prevent reaching Valtan. */
	struct BYPASS_DESTINATION final
	{
		const char* pTriggerPlacementId;
		float x;
		float y;
		float z;
		float duration;
		float arcHeight;
	};
	static constexpr BYPASS_DESTINATION DESTINATIONS[] =
	{
		{ "Stage_1", 46.741f, 10.060f, -61.417f, 0.65f, 1.0f },
		{ "Stage_MiniBoss", 86.110f, 14.627f, -93.033f, 0.75f, 1.2f },
		{ "Stage_2", 94.762f, 15.511f, -90.633f, 0.55f, 0.8f },
		{ "Stage_3", 126.450f, 23.061f, -94.750f, 0.90f, 2.0f },
		{ "Stage_Boss", 154.296f, 22.970f, -125.219f, 0.01f, 0.f }
	};
	const auto found = std::find_if(
		std::begin(DESTINATIONS), std::end(DESTINATIONS),
		[&triggerPlacementId](const BYPASS_DESTINATION& destination)
		{
			return triggerPlacementId == destination.pTriggerPlacementId;
		});
	if (std::end(DESTINATIONS) == found)
		return false;

	WORLD_TRIGGER_ACTION staged{};
	staged.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
	staged.fTargetX = found->x;
	staged.fTargetY = found->y;
	staged.fTargetZ = found->z;
	staged.fDurationSeconds = found->duration;
	staged.fArcHeight = found->arcHeight;
	outAction = staged;
	return true;
}
#endif

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
	player.PendingCommand.Clear();
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
