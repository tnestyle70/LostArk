#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
}

void LostArk::Server::CGameRoom::Refresh_PlayerBlockingBodies()
{
	std::vector<SERVER_BLOCKING_BODY> bodies;
	bodies.reserve(m_WorldEntities.size());
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon ||
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
			(WORLD_BOOTSTRAP_KIND::NPC != entity.eKind &&
			 0u == entity.iCurrentHp))
		{
			continue;
		}
		/* Same body the skill hit test uses: monsters carry their profile
		radius, the boss reads its profile, and town NPCs use the shared upright
		player-sized body until the catalog owns a dedicated gameplay radius. */
		float radius = entity.fCollisionRadius;
		float centerY = entity.fPositionY + radius;
		float halfHeight = radius;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			if (const BOSS_RUNTIME_PROFILE* bossProfile =
				m_GameplayCatalog.Find_Boss(entity.strArchetypeId))
			{
				radius = bossProfile->fCollisionRadius;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
		{
			using namespace LostArk::Shared::WorldCollision;
			radius = PLAYER_HALF_EXTENT_X;
			centerY = entity.fPositionY + PLAYER_CENTER_OFFSET_Y;
			halfHeight = PLAYER_HALF_EXTENT_Y;
		}
		else if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind)
		{
			continue;
		}
		if (radius <= 0.f)
			continue;
		if (WORLD_BOOTSTRAP_KIND::NPC != entity.eKind)
		{
			centerY = entity.fPositionY + radius;
			halfHeight = radius;
		}
		bodies.push_back(SERVER_BLOCKING_BODY{
			entity.fPositionX, entity.fPositionZ, radius,
			centerY, halfHeight, entity.iNetEntityId });
	}
	m_ServerCollisionSystem.Set_BlockingBodies(std::move(bodies));
}

void LostArk::Server::CGameRoom::Begin_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	player.iFallDeathTick = Add_ServerTicksSkippingReservedZero(
		player.iActionStartTick, FALL_DEATH_TICKS);
	player.fFallVelocityY = 0.f;
	/* Everything the fall interrupts is cleared here instead of inside each
	system, so no half-finished action can resume when the body lands dead. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.Clear_Attachment();
	/* Every boss and monster gate already refuses a player that is not combat
	ready, so this one flag removes the falling body from acquisition and from
	area damage without editing four separate target filters. */
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	/* The edge that opens the hole is also the first tick of the descent, so
	the body integrates here instead of hanging one tick at the old height and
	broadcasting a FALLING snapshot that has not moved. The deadline was just
	set a full FALL_DEATH_TICKS away, so it cannot be due on this tick. */
	player.fFallVelocityY -=
		FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
	player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
}

bool LostArk::Server::CGameRoom::Capture_PlayerAttachment(
	const LostArk::Shared::NET_ENTITY_ID playerEntityId,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	const std::uint32_t serverTick, const std::uint32_t holdEndTick)
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == playerEntityId ||
		INVALID_NET_ENTITY_ID == ownerEntityId ||
		playerEntityId == ownerEntityId || 0u == serverTick ||
        (holdEndTick && (Has_ReachedServerTick(serverTick, holdEndTick) || holdEndTick - serverTick > 18001u)) ||
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot)
	{
		return false;
	}

	const auto playerId = m_PlayerIdByEntityId.find(playerEntityId);
	if (m_PlayerIdByEntityId.end() == playerId)
		return false;
	const auto playerIter = m_Players.find(playerId->second);
	if (m_Players.end() == playerIter)
		return false;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() == owner ||
		WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
		SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
		0u == owner->iCurrentHp || 0u == owner->iPatternSequence ||
		!std::isfinite(owner->fPositionX) ||
		!std::isfinite(owner->fPositionY) ||
		!std::isfinite(owner->fPositionZ) ||
		!std::isfinite(owner->fYawDegrees))
	{
		return false;
	}

	SERVER_PLAYER& player = playerIter->second;
	if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
	{
		return player.iAttachmentOwnerNetEntityId == ownerEntityId &&
			player.eAttachmentSlot == slot &&
			player.iAttachmentPatternSequence == owner->iPatternSequence &&
            player.iAttachmentEndTick == holdEndTick;
	}
	if (0u == player.iCurrentHp || !player.isCombatReady ||
        PLAYER_ACTION_STATE::FEAR == player.eAction ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		!std::isfinite(player.fPositionX) ||
		!std::isfinite(player.fPositionY) ||
		!std::isfinite(player.fPositionZ) ||
		!std::isfinite(player.fYawDegrees))
	{
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float deltaX = player.fPositionX - owner->fPositionX;
	const float deltaZ = player.fPositionZ - owner->fPositionZ;
	const float localX = deltaX * cosine - deltaZ * sine;
	const float localY = player.fPositionY - owner->fPositionY;
	const float localZ = deltaX * sine + deltaZ * cosine;
	const float localYaw = Wrap_Degrees(
		player.fYawDegrees - owner->fYawDegrees);
	if (!std::isfinite(localX) || !std::isfinite(localY) ||
		!std::isfinite(localZ) || !std::isfinite(localYaw))
	{
		return false;
	}

	/* Capture interrupts one complete action transaction. Projectiles and
	combat objects cannot remain owned by a body whose input is now frozen. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0u;
	player.iSpawnedProjectileMask = 0u;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.Clear_Attachment();
	player.iAttachmentOwnerNetEntityId = ownerEntityId;
	player.eAttachmentSlot = slot;
	player.iAttachmentPatternSequence = owner->iPatternSequence;
    player.iAttachmentEndTick = holdEndTick;
	player.fAttachmentLocalOffsetX = localX;
	player.fAttachmentLocalOffsetY = localY;
	player.fAttachmentLocalOffsetZ = localZ;
	player.fAttachmentYawOffsetDegrees = localYaw;
	player.eAction = PLAYER_ACTION_STATE::GRABBED;
	player.iActionStartTick = serverTick;
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return true;
}

bool LostArk::Server::CGameRoom::Release_PlayerAttachment(
	SERVER_PLAYER& player,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
		player.iAttachmentOwnerNetEntityId != ownerEntityId ||
		0u == serverTick || !std::isfinite(pushRangeM))
	{
		return false;
	}

	float sourceX = player.fPositionX;
	float sourceZ = player.fPositionZ;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() != owner &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionZ))
	{
		sourceX = owner->fPositionX;
		sourceZ = owner->fPositionZ;
	}

	player.Clear_Attachment();
	player.eAction = 0u == player.iCurrentHp ?
		PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = 0u != player.iCurrentHp;
	if (0u != player.iCurrentHp)
	{
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, sourceX, sourceZ, pushRangeM, pushMs,
			knockdown, downMs, serverTick);
	}
	return true;
}

std::size_t LostArk::Server::CGameRoom::Release_PlayerAttachments(
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	std::size_t released = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (Release_PlayerAttachment(
			player, ownerEntityId, pushRangeM, pushMs,
			knockdown, downMs, serverTick))
		{
			++released;
		}
	}
	return released;
}

bool LostArk::Server::CGameRoom::Update_PlayerAttachment(
	SERVER_PLAYER& player,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction)
	{
		if (INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::NONE != player.eAttachmentSlot ||
			(0u != player.iAttachmentPatternSequence || 0u != player.iAttachmentEndTick))
		{
			player.Clear_Attachment();
		}
		return false;
	}

	const NET_ENTITY_ID ownerEntityId =
		player.iAttachmentOwnerNetEntityId;
    if (player.iCurrentHp == 0u || (player.iAttachmentEndTick && Has_ReachedServerTick(serverTick, player.iAttachmentEndTick)))
    {
        (void)Release_PlayerAttachment(player, ownerEntityId, 0.f, 0u, false, 0u, serverTick ? serverTick : 1u);
        return false;
    }
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	const bool liveOwner = m_WorldEntities.end() != owner &&
		WORLD_BOOTSTRAP_KIND::BOSS == owner->eKind &&
		SERVER_ENTITY_ACTION::DEAD != owner->eAction &&
		0u != owner->iCurrentHp && 0u != owner->iPatternSequence &&
		player.iAttachmentPatternSequence == owner->iPatternSequence;
	/* A World Object carries this player, not a boss bone. The KoukuSaydon logic
	runtime writes the transform from the authored region every tick it runs, so
	the pose it wrote stands; all that is owned here is the deadline that region
	gave us and the ordinary release once it passes. */
	if (PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP == player.eAttachmentSlot)
	{
		if (!liveOwner || 0u == player.iAttachmentReleaseTick ||
			CKoukuSaydonLogicRuntime::Has_ReachedTick(
				serverTick, player.iAttachmentReleaseTick))
		{
			(void)Release_PlayerAttachment(
				player, ownerEntityId, 0.f, 0u, liveOwner, liveOwner ? 1500u : 0u,
				0u == serverTick ? 1u : serverTick);
			return false;
		}
		player.isCombatReady = false;
		return true;
	}
	const bool validOwner = liveOwner &&
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND == player.eAttachmentSlot &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionY) &&
		std::isfinite(owner->fPositionZ) &&
		std::isfinite(owner->fYawDegrees) &&
		std::isfinite(player.fAttachmentLocalOffsetX) &&
		std::isfinite(player.fAttachmentLocalOffsetY) &&
		std::isfinite(player.fAttachmentLocalOffsetZ) &&
		std::isfinite(player.fAttachmentYawOffsetDegrees);
	if (!validOwner)
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float nextX = owner->fPositionX +
		player.fAttachmentLocalOffsetX * cosine +
		player.fAttachmentLocalOffsetZ * sine;
	const float nextY = owner->fPositionY +
		player.fAttachmentLocalOffsetY;
	const float nextZ = owner->fPositionZ -
		player.fAttachmentLocalOffsetX * sine +
		player.fAttachmentLocalOffsetZ * cosine;
	const float nextYaw = Wrap_Degrees(
		owner->fYawDegrees + player.fAttachmentYawOffsetDegrees);
	if (!std::isfinite(nextX) || !std::isfinite(nextY) ||
		!std::isfinite(nextZ) || !std::isfinite(nextYaw))
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	player.fPositionX = nextX;
	player.fPositionY = nextY;
	player.fPositionZ = nextZ;
	player.fYawDegrees = nextYaw;
	player.isCombatReady = false;
	return true;
}

/* Owns the whole falling life cycle of one player inside one tick: it starts
a fall when the authored ground under the player is gone, advances a running
fall, and turns it into the ordinary death the revive path already
understands. Returning true is what keeps trigger motion, skills and movement
from running at all this tick. */
bool LostArk::Server::CGameRoom::Restore_PatternBoundPlayer(
	SERVER_PLAYER& player)
{
	float restoreX = player.fPatternBindRestoreX;
	float restoreY = player.fPatternBindRestoreY;
	float restoreZ = player.fPatternBindRestoreZ;
	bool resolved = std::isfinite(restoreX) && std::isfinite(restoreY) &&
		std::isfinite(restoreZ);
	if (m_ServerNavigation.Is_Loaded())
	{
		resolved = false;
		SERVER_NAV_POINT ground{};
		if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPatternBindRestoreX,
				player.fPatternBindRestoreZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPatternBindRestoreX;
			restoreY = ground.y;
			restoreZ = player.fPatternBindRestoreZ;
			resolved = true;
		}
		else if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			(m_ServerNavigation.Project_PointOnSameLevel(
			player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground) ||
			m_ServerNavigation.Project_Point(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground)) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = ground.x;
			restoreY = ground.y;
			restoreZ = ground.z;
			resolved = true;
		}
		if (!resolved && std::isfinite(player.fPositionX) &&
			std::isfinite(player.fPositionZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPositionX, player.fPositionZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPositionX, player.fPositionZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPositionX;
			restoreY = ground.y;
			restoreZ = player.fPositionZ;
			resolved = true;
		}
		if (!resolved && !player.strSpawnPlacementId.empty())
		{
			const WORLD_BOOTSTRAP_PLACEMENT* spawn =
				Find_Placement(player.strSpawnPlacementId);
			if (nullptr != spawn &&
				m_ServerNavigation.Is_PointWalkableExact(
					spawn->fPositionX, spawn->fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					spawn->fPositionX, spawn->fPositionZ, ground) &&
				std::isfinite(ground.x) && std::isfinite(ground.y) &&
				std::isfinite(ground.z))
			{
				restoreX = spawn->fPositionX;
				restoreY = ground.y;
				restoreZ = spawn->fPositionZ;
				resolved = true;
			}
		}
	}
	if (!resolved)
		return false;
	const float restoreYaw = std::isfinite(player.fPatternBindRestoreYawDegrees) ?
		player.fPatternBindRestoreYawDegrees :
		(std::isfinite(player.fYawDegrees) ? player.fYawDegrees : 0.f);
	player.fPositionX = restoreX;
	player.fPositionY = restoreY;
	player.fPositionZ = restoreZ;
	player.fYawDegrees = restoreYaw;
	player.eAction = 0u == player.iCurrentHp ?
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD :
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	player.isCombatReady = 0u != player.iCurrentHp &&
		player.bPatternBindRestoreCombatReady;
	player.Clear_PatternBindStatus();
	return true;
}

bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (player.bArenaEjectionActive && 0u != player.iCurrentHp)
		return false;
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		if (!std::isfinite(player.fPositionY) || sinceDeadline >= 0)
		{
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
		}
		return true;
	}
	if (!m_ServerNavigation.Is_Loaded() ||
		0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		!m_ServerNavigation.Is_PointInVoidRegion(
			player.fPositionX, player.fPositionZ))
	{
		return false;
	}

	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (player.CardMaze.transferStartTick && player.iCurrentHp) continue;
		const auto ownsLivePatternOccurrence =
			[this](const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
				const std::uint32_t patternSequence)
			{
				return std::any_of(
					m_WorldEntities.begin(), m_WorldEntities.end(),
					[ownerEntityId, patternSequence](
						const SERVER_WORLD_ENTITY& entity)
					{
						return entity.iNetEntityId == ownerEntityId &&
							entity.iPatternSequence == patternSequence &&
							0u != entity.iCurrentHp &&
							SERVER_ENTITY_ACTION::DEAD != entity.eAction;
					});
			};
        (void)CKoukuSaydonLogicRuntime::Update_PlayerFear(player, updateTick);
		Update_MarioControlState(player);
		if (0u == player.iCurrentHp ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			/* A lethal hit does not strand the replicated body five metres above
			the arena. Restore the admitted pose first, while preserving DEAD and
			combat-disabled state, then release both occurrence owners. */
			if (player.bPatternBound)
				(void)Restore_PatternBoundPlayer(player);
			player.Clear_SilenceStatus();
		}
		else
		{
			if (player.bPatternBound &&
				(Has_ReachedServerTick(updateTick, player.iPatternBindEndTick) ||
				 !ownsLivePatternOccurrence(
					player.iPatternBindOwnerNetEntityId,
					player.iPatternBindSequence)))
			{
				(void)Restore_PatternBoundPlayer(player);
			}
			if (0u != player.iSilenceEndTick &&
				Has_ReachedServerTick(updateTick, player.iSilenceEndTick))
			{
				player.Clear_SilenceStatus();
			}
		}
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
		{
			const bool driverMayPlay =
				player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId &&
				VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
					m_ValtanTimelineAudition.ePhase;
			if (!driverMayPlay)
			{
				m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
				Freeze_TimelineAuditionPlayer(player);
				continue;
			}
		}
#endif
		if (player.bPatternBound)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			player.PendingCommand.Clear();
			player.isCombatReady = false;
			continue;
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::FEAR == player.eAction) continue;
		if (Update_PlayerAttachment(player, updateTick))
			continue;
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
		const std::string authoredMoveSource = player.TriggerMove.strSourcePlacementId;
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			if (authoredMoveSource.empty())
				Project_MarioRailPoint(player, player.fPositionX, player.fPositionZ);
			if (0u != player.iMarioStage && !player.TriggerMove.isActive && !authoredMoveSource.empty())
				(void)Configure_MarioRail(player, authoredMoveSource);
			Update_MarioControlState(player);
			continue;
		}
		const bool wasKnockbackActive =
			player.fKnockbackRemainingSeconds > 0.f;
		Advance_PlayerKnockback(player, fixedDeltaSeconds);
		if (wasKnockbackActive)
			continue;
		if (LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction &&
			static_cast<std::int32_t>(
				updateTick - player.iKnockdownEndTick) >= 0)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.iKnockdownEndTick = 0u;
			player.iHitReactionGraceEndTick =
				updateTick + PLAYER_HIT_REACTION_GRACE_TICKS;
			player.PendingCommand.Clear();
		}
		/* The Esther call is a fixed-length lock, not a balance skill: the
		roster owns the summon, this block only releases the caster once the
		call clip has run out. Signed difference keeps ordering across a
		wrapped tick counter. */
		const bool estherCastElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + ESTHER_CAST_TICKS)) >= 0;
		/* An escape teleport borrows the same INTERACTION lock and start tick,
		so a swing is judged only for a real hammer press. */
		const bool mazeHammerPress =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MAZE == player.eKoukuHudMode &&
			0u == player.CardMaze.transferStartTick;
		/* The maze hammer lands part-way through its press: judge the swing
		once, on that tick, against the run's targets in front of the player. */
		if (mazeHammerPress &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET)
		{
			Resolve_CardMazeHammerHit(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MARIO == player.eKoukuHudMode &&
			0u == player.iCurrentSkillId &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET)
		{
			Resolve_MarioHammerHit(player, updateTick);
		}
		/* A KoukuSaydon interaction press is the same kind of lock, but it ends
		with the clip the pressed slot was authored on so the Client is never
		left holding a frozen last frame. An escape teleport borrows this action
		with no slot of its own, and INVALID_SKILL_ID is 0 -- the same value as
		slot 0 -- so it is separated by its transfer tick, not by the skill id. */
		const std::uint32_t interactionTicks =
			0u == player.CardMaze.transferStartTick ?
			CKoukuSaydonLogicRuntime::Ticks_FromMs(
				LostArk::Shared::Kouku_InteractionActionMs(
					player.eKoukuHudMode, player.iCurrentSkillId)) :
			KOUKU_INTERACTION_TICKS;
		const bool interactionElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + interactionTicks)) >= 0;
		if (estherCastElapsed || interactionElapsed)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.PendingCommand.Clear();
		}
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			&m_ServerCollisionSystem,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction &&
			PLAYER_PENDING_COMMAND_KIND::NONE != player.PendingCommand.eKind)
		{
			Commit_PendingPlayerCommand(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		Update_MarioMoveGoal(player, updateTick);
		if (!player.hasMoveGoal)
			continue;
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				PLAYER_TURN_DEGREES_PER_SECOND * fixedDeltaSeconds;
			if (0u != player.iMarioStage || distance <= DIRECT_BEARING_DISTANCE ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			// Movement follows the requested path immediately; facing catches up
			// independently so an opposite click does not first walk sideways.
			const float stepX = deltaX * moveRatio;
			const float stepZ = deltaZ * moveRatio;
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
		}
		Project_MarioRailPoint(player, proposedX, proposedZ);
		/* A smoothed path can skip many authored cells. Never interpolate Y toward
		the distant waypoint: doing so raises the player while XZ is still on the
		lower deck and lets a later height check see an already-raised player.
		Resolve both XZ positions against navigation and take only its ground Y. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT proposedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				proposedX,
				proposedZ,
				proposedGround))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			proposedY = proposedGround.y;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		Project_MarioRailPoint(player, resolvedX, resolvedZ);
		/* Body collision may slide XZ away from the point checked above. Validate
		the final slide destination too and ground it before committing any
		authoritative coordinate. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT resolvedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				resolvedX,
				resolvedZ,
				resolvedGround))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			resolvedY = resolvedGround.y;
		}
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (wasBlocked)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

bool LostArk::Server::CGameRoom::Resolve_ArenaCenter(
	const SERVER_WORLD_ENTITY& boss, SERVER_NAV_POINT& point)
{
	const bool exact = m_ServerNavigation.Is_PointWalkableExact(
		boss.fSpawnPositionX, boss.fSpawnPositionZ) &&
		m_ServerNavigation.Sample_Position(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point);
	if (!m_ServerNavigation.Is_Loaded() ||
		(!exact && !m_ServerNavigation.Project_PointOnSameLevel(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point)) ||
		!m_ServerNavigation.Is_PointWalkableExact(point.x, point.z) ||
		std::fabs(point.y - boss.fSpawnPositionY) > 1.5f ||
		std::hypot(point.x - boss.fSpawnPositionX,
			point.z - boss.fSpawnPositionZ) > 8.f)
	{
		m_strStatus = "Arena center has no nearby walkable same-level recovery point";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Prepare_ArenaEjection(
	SERVER_PLAYER& staged,
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick)
{
	if (BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION != action.eReleaseMode ||
		!m_ServerNavigation.Is_Loaded() ||
		!std::isfinite(action.fReleaseSpeedMps) || action.fReleaseSpeedMps <= 0.f ||
		action.fReleaseSpeedMps > 50.f || 0u == action.iDurationMs ||
		action.iDurationMs > 5000u ||
		!std::isfinite(action.fReleaseYawOffsetDegrees) ||
		std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
		!std::isfinite(boss.fYawDegrees))
	{
		m_strStatus = "Arena ejection policy or navigation is invalid";
		return false;
	}
	const float yaw = (boss.fYawDegrees + action.fReleaseYawOffsetDegrees) *
		DEGREES_TO_RADIANS;
	const float directionX = -std::sin(yaw);
	const float directionZ = -std::cos(yaw);
	constexpr float maximumDistance = 128.f;
	constexpr float outsideMargin = 2.f;
	const float sampleStep = std::clamp(m_ServerNavigation.Get_CellSize(), 0.1f, 0.5f);
	float lastArenaGround = 0.f;
	/* Scan past small holes and seams. An interior missing cell is not the arena
	   exterior; the endpoint lies beyond the last same-deck ground on this ray. */
	for (float distance = 0.f; distance <= maximumDistance; distance += sampleStep)
	{
		const float x = staged.fPositionX + directionX * distance;
		const float z = staged.fPositionZ + directionZ * distance;
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_PointWalkableExact(x, z) &&
			m_ServerNavigation.Sample_Position(x, z, ground) &&
			std::fabs(ground.y - boss.fSpawnPositionY) <= 1.5f)
			lastArenaGround = distance;
	}
	const float minimumDistance = action.fReleaseSpeedMps *
		(static_cast<float>(action.iDurationMs) / 1000.f);
	const float distance = (std::max)(minimumDistance, lastArenaGround + outsideMargin);
	if (!std::isfinite(distance) || distance > maximumDistance ||
		!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
	{
		m_strStatus = "Arena ejection has no bounded exterior destination";
		return false;
	}
	if (0u == staged.iCurrentHp)
		return true;
	staged.fKnockbackDirectionX = directionX;
	staged.fKnockbackDirectionZ = directionZ;
	staged.fKnockbackSpeed = action.fReleaseSpeedMps;
	staged.fKnockbackRemainingSeconds = distance / action.fReleaseSpeedMps;
	staged.bArenaEjectionActive = true;
	staged.iEjectionOwnerNetEntityId = boss.iNetEntityId;
	staged.isCombatReady = false;
	return true;
}

void LostArk::Server::CGameRoom::Advance_PlayerKnockback(
	SERVER_PLAYER& player, const float fixedDeltaSeconds)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		player.fKnockbackRemainingSeconds <= 0.f)
	{
		return;
	}
	if (0u == player.iCurrentHp ||
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
	{
		player.Clear_Attachment();
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	const float step = (std::min)(
		fixedDeltaSeconds, player.fKnockbackRemainingSeconds);
	float desiredX = player.fPositionX +
		player.fKnockbackDirectionX * player.fKnockbackSpeed * step;
	float desiredZ = player.fPositionZ +
		player.fKnockbackDirectionZ * player.fKnockbackSpeed * step;
	Project_MarioRailPoint(player, desiredX, desiredZ);
	if (player.bArenaEjectionActive)
	{
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fKnockbackRemainingSeconds =
			(std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&player](const SERVER_WORLD_ENTITY& boss)
			{ return boss.iNetEntityId == player.iEjectionOwnerNetEntityId; });
		if (player.fKnockbackRemainingSeconds <= 0.00001f ||
			owner == m_WorldEntities.end() || 0u == owner->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == owner->eAction)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
				1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		}
		return;
	}
	SERVER_NAV_POINT reachable{ desiredX, player.fPositionY, desiredZ };
	bool wasClamped = false;
	if (m_ServerNavigation.Is_Loaded())
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			desiredX,
			desiredZ,
			reachable,
			wasClamped);
	}
	Project_MarioRailPoint(player, reachable.x, reachable.z);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!player.bMarioRailReady || !m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, reachable.x, reachable.z, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		reachable.y = railGround.y;
	}
	float resolvedX = player.fPositionX;
	float resolvedY = player.fPositionY;
	float resolvedZ = player.fPositionZ;
	bool wasBlocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(
		player,
		reachable.x,
		reachable.y,
		reachable.z,
		resolvedX,
		resolvedY,
		resolvedZ,
		wasBlocked))
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	Project_MarioRailPoint(player, resolvedX, resolvedZ);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, resolvedX, resolvedZ, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = railGround.y;
	}
	player.fPositionX = resolvedX;
	player.fPositionY = resolvedY;
	player.fPositionZ = resolvedZ;
	player.fKnockbackRemainingSeconds = (wasClamped || wasBlocked) ?
		0.f : player.fKnockbackRemainingSeconds - step;
	if (player.fKnockbackRemainingSeconds <= 0.f)
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
	}
}
