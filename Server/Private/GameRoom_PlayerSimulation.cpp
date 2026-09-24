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

namespace
{
	constexpr float KOUKU_FALL_DEPTH_M = 5.f;

	enum class FORCED_SURFACE_RESULT
	{
		SUPPORTED,
		BLOCKED,
		FALL
	};

	/* Forced motion follows physical support, not the walking graph. A
	   non-walkable surface can support a pushed body, while a lower deck must
	   never become an instantaneous landing. Collision has already resolved
	   this straight segment; no nearest-cell projection is permitted here. */
	FORCED_SURFACE_RESULT Trace_ForcedSurface(
		const LostArk::Server::CServerNavigation& navigation,
		const LostArk::Server::SERVER_NAV_POINT& from,
		const float toX, const float toZ,
		LostArk::Server::SERVER_NAV_POINT& outPoint)
	{
		outPoint = from;
		const float distance = std::hypot(toX - from.x, toZ - from.z);
		const float sampleStep = std::clamp(navigation.Get_CellSize() * 0.5f, 0.01f, 0.25f);
		if (!std::isfinite(distance) || !std::isfinite(from.y) ||
			!std::isfinite(sampleStep) || distance / sampleStep > 4096.f)
		{
			return FORCED_SURFACE_RESULT::BLOCKED;
		}
		/* Keep forced support bounded even if an older navigation policy uses
		   zero to permit arbitrary walking height changes. */
		const float authoredStep = navigation.Get_MaximumTraversalStepHeight();
		const float maximumStep = authoredStep > 0.f ? (std::min)(authoredStep, 1.f) : 1.f;
		const auto count = static_cast<std::uint32_t>((std::max)(1.f, std::ceil(distance / sampleStep)));
		for (std::uint32_t sample = 0u; sample <= count; ++sample)
		{
			const float ratio = static_cast<float>(sample) / static_cast<float>(count);
			const float x = from.x + (toX - from.x) * ratio;
			const float z = from.z + (toZ - from.z) * ratio;
			LostArk::Server::SERVER_NAV_POINT ground{};
			if (!navigation.Sample_SurfacePosition(x, z, ground) ||
				!std::isfinite(ground.y) || ground.y < outPoint.y - maximumStep)
			{
				outPoint.x = x;
				outPoint.z = z;
				return FORCED_SURFACE_RESULT::FALL;
			}
			if (ground.y > outPoint.y + maximumStep)
				return FORCED_SURFACE_RESULT::BLOCKED;
			outPoint = ground;
		}
		return FORCED_SURFACE_RESULT::SUPPORTED;
	}
}

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
	player.fFallDeathPlaneY = (player.bKnockbackBallistic ? player.fKnockbackSupportY : player.fPositionY) - KOUKU_FALL_DEPTH_M;
	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.bKoukuFallDeath = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA && !player.iMarioStage;
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
	/* The ejection/ballistic phase ends at this boundary.  Keep the ordinary
	   FALLING integrator authoritative after the edge crossing; leaving either
	   typed flight flag set would make Update_PlayerFall return early forever
	   and the player could never reach the dead-zone deadline. */
	player.bArenaEjectionActive = false;
	player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
	player.bKnockbackCanLeaveArena = false;
	player.bKnockbackBallistic = false;
	player.fKnockbackVelocityY = 0.f;
	player.fKnockbackLaunchY = 0.f;
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
	const std::uint32_t serverTick, const std::uint32_t holdEndTick, const std::uint32_t sourcePatternSequence)
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
	const auto liveOwner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	auto* owner = sourcePatternSequence && m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, sourcePatternSequence) :
		(liveOwner == m_WorldEntities.end() ? nullptr : &*liveOwner);
	if (nullptr == owner ||
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
	const auto body = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	const auto* owner = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, player.iAttachmentPatternSequence) :
		(body == m_WorldEntities.end() ? nullptr : &*body);
	const bool liveOwner = nullptr != owner &&
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
				player, ownerEntityId, 0.f, 0u, false, 0u,
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

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const
{
	if (m_eWorldId != LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA || player.iMarioStage ||
		player.eKoukuAreaHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE) return nullptr;
	// Refinement grid rectangles are only bake coverage, not arena bounds.
	// The casino continues onto the base grid. Classify the separated authored
	// stages by their spawn anchors, not by Gate2Fine's small rectangle.
	const WORLD_BOOTSTRAP_PLACEMENT* nearest = nullptr;
	float distance = (std::numeric_limits<float>::max)();
	for (const char* id : { "stage.kakul.sl01", "stage.kakul.sl02", "stage.kakul.sl03",
		"stage.kakul.sl04", "stage.kakul.sl05" })
	{
		const auto* marker = Find_Placement(id);
		if (!marker || marker->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN) continue;
		const float dx = player.fPositionX - marker->fPositionX;
		const float dz = player.fPositionZ - marker->fPositionZ;
		const float candidate = dx * dx + dz * dz;
		if (candidate < distance) { nearest = marker; distance = candidate; }
	}
	return nearest && nearest->strPlacementId == "stage.kakul.sl03" ? nearest : nullptr;
}

bool LostArk::Server::CGameRoom::Try_KoukuWalkOffFloor(
	SERVER_PLAYER& player, const float x, const float z,
	const float fixedDeltaSeconds, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || !m_ServerNavigation.Is_Loaded() ||
		!player.iCurrentHp || player.TriggerMove.isActive || player.bArenaEjectionActive) return false;
	const auto* gate2 = Resolve_KoukuFallCenter(player);
	const bool casino = nullptr != gate2;
	if (!player.iMarioStage && !casino) return false;
	SERVER_NAV_POINT surface{};
	// Obstacles keep their physical support. A blocked walking cell alone
	// must never be interpreted as a hole.
	const auto supported = [&](const float px, const float pz) {
		return m_ServerNavigation.Sample_SurfacePosition(px, pz, surface) &&
			surface.y >= player.fPositionY - m_ServerNavigation.Get_MaximumTraversalStepHeight();
	};
	if (supported(x, z)) return false;
	float resolvedX{}, resolvedY{}, resolvedZ{};
	bool blocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(player, x, player.fPositionY, z,
		resolvedX, resolvedY, resolvedZ, blocked) || blocked ||
		supported(resolvedX, resolvedZ)) return false;
	if (casino && !player.iMarioStage)
	{
		SERVER_NAV_POINT center{};
		if (!m_ServerNavigation.Project_Point(gate2->fPositionX, gate2->fPositionZ, center, gate2->fPositionY)) return false;
		player.KoukuFallRevivePosition = std::array<float, 3u>{ center.x, center.y, center.z };
	}
	player.fPositionX = resolvedX;
	player.fPositionZ = resolvedZ;
	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if ((player.bArenaEjectionActive ||
		(player.bKnockbackBallistic && player.fKnockbackRemainingSeconds > 0.f)) && 0u != player.iCurrentHp)
		return false;
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence && player.iCurrentHp && player.eAction == PLAYER_ACTION_STATE::FALLING)
	{
		SERVER_NAV_POINT start{}, ground{}; float yaw = 0.f;
		if (Resolve_KoukuRevivePosition(player, start, yaw) &&
			(!m_ServerNavigation.Is_Loaded() || m_ServerNavigation.Project_Point(start.x, start.z, ground, start.y)))
		{
			if (!m_ServerNavigation.Is_Loaded()) ground = start;
			player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
			player.fFallVelocityY = 0.f; player.iFallDeathTick = 0u;
			player.eAction = PLAYER_ACTION_STATE::NONE; player.isCombatReady = true;
		}
		return true;
	}
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		const bool reachedDeath = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
			(!std::isfinite(player.fFallDeathPlaneY) || player.fPositionY <= player.fFallDeathPlaneY) : sinceDeadline >= 0;
		if (!std::isfinite(player.fPositionY) || reachedDeath)
		{
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
			Update_MarioControlState(player);
		}
		return true;
	}
	// Authored jumps and entry/exit transfers own their airborne trajectory.
	if (player.TriggerMove.isActive) return false;
	if (gateFence || !m_ServerNavigation.Is_Loaded() ||
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
		if (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA && player.iCurrentHp &&
			!player.iMarioStage && !player.TriggerMove.isActive &&
			player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FALLING)
		{
			const auto* center = Resolve_KoukuFallCenter(player);
			if (center)
			{
				SERVER_NAV_POINT ground{};
				if (m_ServerNavigation.Project_Point(center->fPositionX, center->fPositionZ, ground, center->fPositionY))
					player.KoukuFallRevivePosition = std::array<float, 3u>{ground.x, ground.y, ground.z};
			}
			else if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::NONE)
				player.KoukuFallRevivePosition.reset();
		}
		/* A song that ended any way but its own timeout (a hit, a bind, death) never
		lands the player later. */
		if (0u != player.iSquareHoleId &&
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG != player.eAction)
		{
			player.iSquareHoleId = 0u;
		}
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
			if (!player.TriggerMove.isActive && !authoredMoveSource.empty())
				Complete_KoukuMarioReturn(player, authoredMoveSource, updateTick);
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
			player.iHitReactionGraceEndTick = player.bPushOnlyHitReaction ? 0u :
				updateTick + PLAYER_HIT_REACTION_GRACE_TICKS;
			player.bPushOnlyHitReaction = false;
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
		/* The square-hole lock is the song plus a black hold: the Client screen is fully
		black when the song ticks run out, and the player lands inside the hold. */
		const bool squareHoleSongElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + SQUAREHOLE_LOCK_TICKS)) >= 0;
		if (squareHoleSongElapsed)
			Finish_SquareHoleSong(player);
		/* An escape teleport borrows the same INTERACTION lock and start tick,
		so a swing is judged only for a real hammer press. */
		const bool mazeHammerPress =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MAZE == player.eKoukuHudMode &&
			0u == player.CardMaze.transferStartTick;
		/* The maze hammer lands part-way through its press: judge the swing
		once, on that tick, against the run's targets in front of the player. */
		if (mazeHammerPress &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::Hammer_HitTickOffset(player.iCurrentSkillId))
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
		if (estherCastElapsed || squareHoleSongElapsed || interactionElapsed)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.PendingCommand.Clear();
		}
		CServerBuffRuntime::Expire(player.ActiveBuffs, updateTick);
		CServerBuffRuntime::Settle_Shield(m_GameplayCatalog.Active(), player);
		Update_VehicleSkill(player, fixedDeltaSeconds);
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
		/* The route was pulled taut from where the player stood when it was
		built, and the player has moved since. Pull it again from here and aim
		at the farthest waypoint still in sight. Without this the player keeps
		facing the first waypoint - a neighbouring cell centre whose bearing has
		little to do with the goal's - and every rebuild puts that cell on the
		other side, which is what made a held right mouse shake. */
		if (!player.MovePath.empty() && m_ServerNavigation.Is_Loaded())
		{
			for (std::size_t candidate = player.MovePath.size();
				candidate > player.iMovePathIndex + 1u; --candidate)
			{
				const SERVER_NAV_POINT& ahead = player.MovePath[candidate - 1u];
				if (m_ServerNavigation.Has_LineOfSight(
					player.fPositionX, player.fPositionZ, ahead.x, ahead.z,
					player.fPositionY))
				{
					player.iMovePathIndex = candidate - 1u;
					break;
				}
			}
		}
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
		/* A smoothed path's intermediate points are corners, not arrivals. Only
		the last one is where the player stops and has to end up facing. */
		const bool targetIsDestination =
			player.iMovePathIndex + 1u >= player.MovePath.size();
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
				(LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId ?
					VEHICLE_TURN_DEGREES_PER_SECOND :
					PLAYER_TURN_DEGREES_PER_SECOND) * fixedDeltaSeconds;
			/* Close to the destination the turn radius no longer fits, so facing
			snaps rather than orbiting the point. A corner is not a destination:
			snapping there made every path bend read as an instant pivot. */
			if (0u != player.iMarioStage ||
				(targetIsDestination && distance <= DIRECT_BEARING_DISTANCE) ||
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
				Resolve_PlayerMoveSpeed(player) *
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
		if (Try_KoukuWalkOffFloor(player, proposedX, proposedZ, fixedDeltaSeconds, updateTick))
			continue;
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
				proposedGround,
				player.fPositionY))
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
				resolvedGround,
				player.fPositionY))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			resolvedY = resolvedGround.y;
		}
		// Successful tangent slides clear wasBlocked; a deflected step still
		// reached the body and must finish a goal inside that same body.
		const bool reachedGoalBody =
			(wasBlocked || resolvedX != proposedX || resolvedZ != proposedZ) &&
			m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(player,
				proposedX, proposedY, proposedZ,
				player.MovePath.empty() ? player.fPositionY : player.MovePath.back().y);
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (reachedGoalBody)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			continue;
		}
		if (wasBlocked)
		{
			/* The body sweep met something the navigation grid does not carry:
			a collisionBox, or another body. Route around it and keep the goal.
			Dropping the goal here is what made a held right mouse shake next
			to an obstacle: every brush cancelled the move, and the re-send
			50 ms later started a fresh search from a start cell that had
			moved, so the first waypoint jumped from side to side.

			The rebuild is rate limited because brushing reports blocked on
			many ticks in a row. Between rebuilds the move is left alone and
			the existing slide carries it along the obstacle. */
			const bool mayReroute = m_ServerNavigation.Is_Loaded() &&
				updateTick - player.iMoveRerouteTick >= MOVE_REROUTE_MIN_TICKS;
			if (!mayReroute)
				continue;
			player.iMoveRerouteTick = updateTick;
			std::vector<SERVER_NAV_POINT> reroute;
			if (m_ServerNavigation.Find_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY))
			{
				m_ServerNavigation.Smooth_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY);
				player.MovePath = std::move(reroute);
				player.iMovePathIndex = 0;
				continue;
			}
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
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence) player.bKnockbackCanLeaveArena = false;
	if (player.bKnockbackBallistic)
	{
		// A bounded launch keeps the authored Y arc while its ground footprint
		// obeys the same navigation and fence collision as ordinary movement.
		const bool bounded = !player.bKnockbackCanLeaveArena;
		if (bounded)
		{
			SERVER_NAV_POINT reachable{desiredX, player.fKnockbackSupportY, desiredZ};
			bool clamped = false;
			if (m_ServerNavigation.Is_Loaded())
				CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation, player.fPositionX, player.fPositionZ,
					desiredX, desiredZ, reachable, clamped, player.fKnockbackSupportY);
			SERVER_PLAYER groundBody = player;
			groundBody.fPositionY = player.fKnockbackSupportY;
			float x = player.fPositionX, y = player.fKnockbackSupportY, z = player.fPositionZ;
			bool blocked = false;
			if (!m_ServerCollisionSystem.Resolve_PlayerMove(groundBody, reachable.x, reachable.y, reachable.z, x, y, z, blocked))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT valid{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Has_LineOfSight(player.fPositionX, player.fPositionZ, x, z) ||
				 !m_ServerNavigation.Resolve_TraversalStep(player.fPositionX, player.fPositionZ, x, z, valid)))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT support{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Sample_SurfacePosition(x, z, support) ||
				 std::abs(support.y - player.fKnockbackSupportY) > 1.f))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			desiredX = x; desiredZ = z;
			if (clamped || blocked) player.fKnockbackSpeed = 0.f;
		}
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fPositionY += player.fKnockbackVelocityY * step -
			0.5f * player.fKnockbackGravityMps2 * step * step;
		player.fKnockbackVelocityY -= player.fKnockbackGravityMps2 * step;
		player.fKnockbackRemainingSeconds = (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		if (!bounded && m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
			player.fPositionY <= player.fKnockbackSupportY - KOUKU_FALL_DEPTH_M)
		{
			const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, 0.f, tick);
			(void)Update_PlayerFall(player, 0.f, tick);
			return;
		}
		SERVER_NAV_POINT floor{ desiredX, player.fKnockbackLaunchY, desiredZ };
		bool hasFloor = !m_ServerNavigation.Is_Loaded() ||
			m_ServerNavigation.Sample_SurfacePosition(desiredX, desiredZ, floor);
		/* A ballistic player may cross an overlapping upper deck while leaving an
		   arena.  That deck is not a landing surface for a flight launched from
		   below it: accepting it would snap Y upward and turn the dead-zone fall
		   into a nav teleport.  Lower floors remain valid and are handled by the
		   normal gravity/dead-zone path. */
		constexpr float maximumLandingRiseM = 0.01f;
		if (hasFloor && std::isfinite(floor.y) &&
			floor.y > player.fKnockbackLaunchY + maximumLandingRiseM)
			hasFloor = false;
		if (bounded && (!hasFloor || std::abs(floor.y - player.fKnockbackSupportY) > 1.f))
		{
			floor = {desiredX, player.fKnockbackSupportY, desiredZ};
			hasFloor = true;
		}
		if (player.fKnockbackVelocityY <= 0.f && hasFloor && player.fPositionY <= floor.y + 0.0001f)
		{
			player.fPositionY = floor.y;
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = player.fKnockbackVelocityY = 0.f;
			player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = false;
			const auto landingTick = Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u);
			const auto recoveryEnd = Add_ServerTicksSkippingReservedZero(landingTick,
				CKoukuSaydonLogicRuntime::Ticks_FromMs(PLAYER_HIT_LANDING_RECOVERY_MS));
			if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN &&
				static_cast<std::int32_t>(recoveryEnd - player.iKnockdownEndTick) > 0)
				player.iKnockdownEndTick = recoveryEnd;
		}
		else if (player.fKnockbackRemainingSeconds <= 0.00001f)
		{
			if (hasFloor)
			{
				// A lower deck takes longer than the nominal same-height flight.
				// Keep descending at the endpoint without snapping down to that deck.
				player.fKnockbackSpeed = 0.f;
				player.fKnockbackRemainingSeconds = fixedDeltaSeconds;
			}
			else
			{
				const float velocityY = player.fKnockbackVelocityY;
				const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
				Begin_PlayerFall(player, 0.f, tick);
				player.fFallVelocityY = velocityY;
			}
		}
		return;
	}
	Project_MarioRailPoint(player, desiredX, desiredZ);
	// Ordinary/arena pushes must reach their existing bounded or swept-surface
	// mover. Only an authored Mario exit uses the rail's walking-floor check.
	if (player.iMarioStage && player.bKnockbackCanLeaveArena &&
		Try_KoukuWalkOffFloor(player, desiredX, desiredZ, fixedDeltaSeconds,
			Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u))) return;
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
	if (m_ServerNavigation.Is_Loaded() &&
		(m_eWorldId == LostArk::Shared::WORLD_ID::VALTAN_ARENA ||
		 (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
		  player.bKnockbackCanLeaveArena && !player.iMarioStage)))
	{
		/* Valtan hits and authored Kouku arena-exit hits cross walking boundaries. Keep actual walls
		   and bodies authoritative, but do not route or clamp the displacement
		   to walkable cells. A push is a straight sweep, not walking avoidance
		   around another body; this also gives support one exact segment. */
		using namespace LostArk::Shared::WorldCollision;
		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_CircleMove(
			player.fPositionX, player.fPositionY, player.fPositionZ,
			desiredX, player.fPositionY, desiredZ,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y, PLAYER_CENTER_OFFSET_Y,
			resolvedX, resolvedY, resolvedZ, wasBlocked,
			player.iNetEntityId, false))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		SERVER_NAV_POINT supported{};
		const FORCED_SURFACE_RESULT support = Trace_ForcedSurface(
			m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ},
			resolvedX, resolvedZ, supported);
		player.fPositionX = supported.x;
		player.fPositionY = supported.y;
		player.fPositionZ = supported.z;
		if (FORCED_SURFACE_RESULT::FALL == support)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
			return;
		}
		player.fKnockbackRemainingSeconds = wasBlocked || FORCED_SURFACE_RESULT::BLOCKED == support ?
			0.f : (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		if (player.fKnockbackRemainingSeconds <= 0.f)
		{
			player.fKnockbackSpeed = 0.f;
			player.bKnockbackCanLeaveArena = false;
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
			wasClamped,
			player.fPositionY);
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
	if (gateFence && m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT supported{};
		const auto support = Trace_ForcedSurface(m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ}, resolvedX, resolvedZ, supported);
		if (support != FORCED_SURFACE_RESULT::SUPPORTED)
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = supported.y;
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
		player.bKnockbackCanLeaveArena = false;
	}
}
