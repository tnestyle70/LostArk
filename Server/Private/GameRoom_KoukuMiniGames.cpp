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

bool LostArk::Server::CGameRoom::Begin_CardMaze(
	const LostArk::Shared::PLAYER_ID claimantId)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId)
		return false;
	if (CKoukuCardMazeRuntime::PHASE::HUNTING == m_KoukuCardMaze.Get_Phase())
	{
		const auto p = m_Players.find(claimantId);
		return p != m_Players.end() && m_KoukuCardMaze.Toggle_Telescope(p->second);
	}
	else if (CKoukuCardMazeRuntime::PHASE::COMPLETE == m_KoukuCardMaze.Get_Phase())
	{
		return false;
	}
	if (m_WorldBootstrap.Get_CardMazeLanes().size() != 36u)
	{ m_strStatus = "Publish all 36 card maze lanes before starting"; return false; }
	std::vector<CKoukuCardMazeRuntime::SPAWN_REQUEST> spawns;
	std::string status;
	if (!m_KoukuCardMaze.Plan(claimantId, m_Players, m_ServerNavigation,
		m_iServerTick, spawns, status))
	{
		m_strStatus = status;
		return false;
	}
	std::uint32_t ordinal = 0u;
	for (const CKoukuCardMazeRuntime::SPAWN_REQUEST& request : spawns)
	{
		const MONSTER_RUNTIME_PROFILE* profile = m_SpawnGroupBootstrap.Find_Profile(
			CKoukuCardMazeRuntime::Archetype_ForSuit(request.eSuit));
		SPAWN_GROUP_ANCHOR anchor{};
		anchor.strAnchorId = "cardmaze.random";
		anchor.fPositionX = request.fPositionX;
		anchor.fPositionY = request.fPositionY;
		anchor.fPositionZ = request.fPositionZ;
		anchor.fYawDegrees = request.fYawDegrees;
		SPAWN_GROUP_ENTRY entry{};
		entry.strAnchorId = anchor.strAnchorId;
		entry.iCount = 1u;
		if (nullptr != profile)
			entry.strArchetypeId = profile->strArchetypeId;
		const NET_ENTITY_ID spawnedId = m_iNextNetEntityId;
		if (nullptr == profile ||
			!Spawn_Monster(CKoukuCardMazeRuntime::SPAWN_GROUP_TAG, entry, anchor, *profile, ordinal++))
		{
			/* Fail closed: nothing of a half-raised run stays behind. */
			const std::string reason = nullptr == profile ?
				"Card maze monster profile is not published: " +
					std::string(CKoukuCardMazeRuntime::Archetype_ForSuit(request.eSuit)) :
				"Card maze target could not be spawned: " + m_strStatus;
			Despawn_CardMazeTargets();
			m_KoukuCardMaze.Abort();
			m_strStatus = reason;
			return false;
		}
		m_KoukuCardMaze.Register_Target(spawnedId, request.eSuit);
	}
	m_KoukuCardMaze.Commit(m_Players);
	/* A Debug start can bypass the box; a run never keeps one standing. */
	Clear_CardMazeClownBox();
	m_strStatus = "Card maze started with " + std::to_string(spawns.size()) + " targets";
	return true;
}

void LostArk::Server::CGameRoom::Reset_CardMaze()
{
	Despawn_CardMazeTargets();
	Clear_CardMazeClownBox();
	m_KoukuCardMaze.Reset(m_Players);
	m_iCardMazeMarchStartTick = m_iCardMazeCycleMs = 0u;
	m_CardMazePreviousPositions.clear();
	m_CardMazeContactTicks.clear();
}

void LostArk::Server::CGameRoom::Despawn_CardMazeTargets()
{
	for (const auto& [entityId, suit] : m_KoukuCardMaze.Get_Targets())
	{
		(void)suit;
		const auto entity = std::find_if(
			m_WorldEntities.begin(), m_WorldEntities.end(),
			[removedId = entityId](const SERVER_WORLD_ENTITY& candidate)
			{
				return candidate.iNetEntityId == removedId;
			});
		if (m_WorldEntities.end() == entity)
			continue;
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
			Mark_RuntimeFailure("card-maze.combat-object-lifecycle");
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		m_WorldEntities.erase(entity);
	}
}

void LostArk::Server::CGameRoom::Resolve_MarioHammerHit(
	SERVER_PLAYER& player, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (!player.iMarioStage || !player.iCurrentHp || !player.isCombatReady || player.TriggerMove.isActive)
		return;
	const float yaw = player.fYawDegrees * DEGREES_TO_RADIANS;
	const float forwardX = std::sin(yaw), forwardZ = std::cos(yaw);
	for (auto& entity : m_WorldEntities)
	{
		if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind ||
			entity.iMarioPatrolStage != player.iMarioStage || !entity.iCurrentHp ||
			std::abs(entity.fPositionY - player.fPositionY) > .8f)
			continue;
		const float dx = entity.fPositionX - player.fPositionX;
		const float dz = entity.fPositionZ - player.fPositionZ;
		const float distance = std::hypot(dx, dz);
		// The Mario Q uses the same authored clown hammer as the maze Q.
		if (distance > CKoukuCardMazeRuntime::HAMMER_RANGE_M + entity.fCollisionRadius ||
			(distance > .01f && (dx * forwardX + dz * forwardZ) / distance <
				CKoukuCardMazeRuntime::HAMMER_HALF_ANGLE_COS))
			continue;
		SERVER_PLAYER_TO_WORLD_HIT hit{};
		hit.iSourcePlayerId = player.iPlayerId;
		hit.iSkillId = CKoukuCardMazeRuntime::HAMMER_SKILL_ID;
		hit.iRawDamage = CKoukuCardMazeRuntime::HAMMER_RAW_DAMAGE;
		hit.fSourceX = player.fPositionX; hit.fSourceZ = player.fPositionZ;
		hit.fFallbackDirectionX = forwardX; hit.fFallbackDirectionZ = forwardZ;
		hit.iServerTick = updateTick;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(entity, hit, m_TickDamageEvents);
	}
	/* Source balls are Client placements with no entity. The bootstrap carries
	the stage layout's slots; a popped slot bit is what the Client hides. The
	ball pivot sits at its base, so the height window is under one floor step. */
	constexpr float BALL_RADIUS_M = .47f, BALL_HEIGHT_WINDOW_M = 1.2f;
	if (player.iMarioStage > 4u) return;
	for (const auto& ball : m_WorldBootstrap.Get_MarioBalls())
	{
		const std::uint16_t bit = static_cast<std::uint16_t>(1u << ball.slot);
		if (ball.stage != player.iMarioStage || ball.layout != player.iMarioLayoutVariant ||
			(m_MarioPoppedBalls[player.iMarioStage] & bit) ||
			std::abs(ball.y - player.fPositionY) > BALL_HEIGHT_WINDOW_M)
			continue;
		const float dx = ball.x - player.fPositionX;
		const float dz = ball.z - player.fPositionZ;
		const float distance = std::hypot(dx, dz);
		if (distance > CKoukuCardMazeRuntime::HAMMER_RANGE_M + BALL_RADIUS_M ||
			(distance > .01f && (dx * forwardX + dz * forwardZ) / distance <
				CKoukuCardMazeRuntime::HAMMER_HALF_ANGLE_COS))
			continue;
		m_MarioPoppedBalls[player.iMarioStage] |= bit;
	}
}

std::uint8_t LostArk::Server::CGameRoom::Mario_CurseReleasedMask(
	const std::uint8_t stage, const std::uint8_t layout) const
{
	if (stage < 1u || stage > 4u) return 0u;
	std::uint8_t present = 0u, remaining = 0u;
	for (const auto& ball : m_WorldBootstrap.Get_MarioBalls())
	{
		if (ball.stage != stage || ball.layout != layout) continue;
		present |= static_cast<std::uint8_t>(1u << ball.color);
		if (!(m_MarioPoppedBalls[stage] & (1u << ball.slot)))
			remaining |= static_cast<std::uint8_t>(1u << ball.color);
	}
	return static_cast<std::uint8_t>(present & ~remaining);
}

void LostArk::Server::CGameRoom::Resolve_CardMazeHammerHit(
	SERVER_PLAYER& player, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	// Q is the maze jump slam; LMB and Mario keep the generic hammer amount.
	const std::uint32_t rawDamage = 0u == player.iCurrentSkillId ?
		500u : CKoukuCardMazeRuntime::HAMMER_RAW_DAMAGE;
	const float yawRadians = player.fYawDegrees * DEGREES_TO_RADIANS;
	const float forwardX = std::sin(yawRadians);
	const float forwardZ = std::cos(yawRadians);
	/* Before the run the clown box stands on the telescope. The swing damages
	it like any monster, and the telescope stays shut until it is destroyed. */
	if (CKoukuCardMazeRuntime::PHASE::INACTIVE == m_KoukuCardMaze.Get_Phase() &&
		!m_bCardMazeClownBoxDestroyed)
	{
		const auto box = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[id = m_iCardMazeClownBoxId](const SERVER_WORLD_ENTITY& entity)
			{
				return INVALID_NET_ENTITY_ID != id && entity.iNetEntityId == id;
			});
		if (m_WorldEntities.end() == box || 0u == box->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == box->eAction ||
			std::abs(box->fPositionY - player.fPositionY) > .8f)
		{
			return;
		}
		const float deltaX = box->fPositionX - player.fPositionX;
		const float deltaZ = box->fPositionZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		// Inside the body any facing lands, as it does inside the telescope box.
		if (distance > box->fCollisionRadius &&
			(distance > CKoukuCardMazeRuntime::HAMMER_RANGE_M + box->fCollisionRadius ||
			 (deltaX * forwardX + deltaZ * forwardZ) / distance <
				CKoukuCardMazeRuntime::HAMMER_HALF_ANGLE_COS))
		{
			return;
		}
		SERVER_PLAYER_TO_WORLD_HIT hit{};
		hit.iSourcePlayerId = player.iPlayerId;
		hit.iSkillId = CKoukuCardMazeRuntime::HAMMER_SKILL_ID;
		hit.iRawDamage = rawDamage;
		hit.fSourceX = player.fPositionX;
		hit.fSourceZ = player.fPositionZ;
		hit.fFallbackDirectionX = forwardX;
		hit.fFallbackDirectionZ = forwardZ;
		hit.iServerTick = updateTick;
		if (SERVER_COMBAT_HIT_RESULT::KILLED ==
			CServerCombatHitRuntime::Apply_PlayerToWorld(*box, hit, m_TickDamageEvents))
		{
			m_bCardMazeClownBoxDestroyed = true;
			m_iCardMazeClownBoxId = INVALID_NET_ENTITY_ID;
			m_strStatus = "Card maze clown box destroyed; strike the telescope with Q to begin";
		}
		return;
	}
	// Q strikes the authored box; subsequent owner hits toggle observation.
	const bool telescopeOpen = !player.CardMaze.transferStartTick;
	const WORLD_BOOTSTRAP_PLACEMENT* telescope = telescopeOpen ?
		Find_Placement(CKoukuCardMazeRuntime::TELESCOPE_PLACEMENT_ID) : nullptr;
	if (nullptr != telescope)
	{
		const float deltaX = telescope->fPositionX - player.fPositionX;
		const float deltaZ = telescope->fPositionZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const float reach = CKoukuCardMazeRuntime::HAMMER_RANGE_M +
			(std::max)(telescope->fHalfExtentX, telescope->fHalfExtentZ);
		const float boxYaw = telescope->fYawDegrees * DEGREES_TO_RADIANS;
		const float localX = deltaX * std::cos(boxYaw) - deltaZ * std::sin(boxYaw);
		const float localZ = deltaX * std::sin(boxYaw) + deltaZ * std::cos(boxYaw);
		const bool insideBox = std::abs(localX) <= telescope->fHalfExtentX &&
			std::abs(localZ) <= telescope->fHalfExtentZ;
		if (telescope->isEnabled && std::abs(player.fPositionY - telescope->fPositionY) <= telescope->fHalfExtentY &&
			(insideBox || (distance <= reach && (distance <= 0.01f ||
				(deltaX * forwardX + deltaZ * forwardZ) / distance >=
					CKoukuCardMazeRuntime::HAMMER_HALF_ANGLE_COS))) &&
			Begin_CardMaze(player.iPlayerId))
		{
			return;
		}
	}
	if (CKoukuCardMazeRuntime::PHASE::HUNTING != m_KoukuCardMaze.Get_Phase())
		return;
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (!m_KoukuCardMaze.Can_Hit(player, entity.iNetEntityId) ||
			WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind ||
			0u == entity.iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction)
		{
			continue;
		}
		const float deltaX = entity.fPositionX - player.fPositionX;
		const float deltaZ = entity.fPositionZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		if (distance > CKoukuCardMazeRuntime::HAMMER_RANGE_M + entity.fCollisionRadius)
			continue;
		if (distance > 0.01f &&
			(deltaX * forwardX + deltaZ * forwardZ) / distance <
				CKoukuCardMazeRuntime::HAMMER_HALF_ANGLE_COS)
		{
			continue;
		}
		SERVER_PLAYER_TO_WORLD_HIT hit{};
		hit.iSourcePlayerId = player.iPlayerId;
		hit.iSkillId = CKoukuCardMazeRuntime::HAMMER_SKILL_ID;
		hit.iRawDamage = rawDamage;
		hit.fSourceX = player.fPositionX;
		hit.fSourceZ = player.fPositionZ;
		hit.fFallbackDirectionX = forwardX;
		hit.fFallbackDirectionZ = forwardZ;
		hit.iServerTick = updateTick;
		const SERVER_COMBAT_HIT_RESULT result =
			CServerCombatHitRuntime::Apply_PlayerToWorld(entity, hit, m_TickDamageEvents);
		if (SERVER_COMBAT_HIT_RESULT::LANDED != result &&
			SERVER_COMBAT_HIT_RESULT::KILLED != result)
		{
			continue;
		}
		const CKoukuCardMazeRuntime::HIT_OUTCOME outcome = m_KoukuCardMaze.On_TargetHit(
			player, entity, SERVER_COMBAT_HIT_RESULT::KILLED == result);
		if (outcome.bKillCounted)
		{
			/* The shard rises where the suppressed damage number used to: the
			felled soldier's position, carrying the hunter's running count. */
			DAMAGE_EVENT shard{};
			shard.iTargetNetEntityId = entity.iNetEntityId;
			shard.iAmount = player.iCardMazeKills;
			shard.fPositionX = entity.fPositionX;
			shard.fPositionY = entity.fPositionY;
			shard.fPositionZ = entity.fPositionZ;
			shard.isOutgoing = true;
			shard.eCardMazeSuit = player.eCardMazeSuit;
			m_TickDamageEvents.push_back(shard);
		}
		if (outcome.bStartMarch)
		{
			m_iCardMazeMarchStartTick = updateTick;
			m_iCardMazeCycleMs = 0u;
			for (const auto& lane : m_WorldBootstrap.Get_CardMazeLanes())
				m_iCardMazeCycleMs = (std::max)(m_iCardMazeCycleMs, lane.delayMs + lane.durationMs);
			// Durable snapshot clock drives all 36 presentations and contact sweeps.
			m_strStatus = "Card maze march started by the first hit";
		}
		if (outcome.bAllComplete)
			m_strStatus = "Card maze complete: every hunter defeated their matching suit";
		else if (outcome.bHunterComplete)
			m_strStatus = "Card maze hunter completed their suit";
	}
	// Do not invalidate the entity range while the hammer iterates it.
	std::vector<NET_ENTITY_ID> removed;
	for (const auto& entity : m_WorldEntities)
		if (!entity.iCurrentHp && m_KoukuCardMaze.Is_Target(entity.iNetEntityId)) removed.push_back(entity.iNetEntityId);
	for (const auto id : removed) Remove_CardMazeTarget(id);
}

bool LostArk::Server::CGameRoom::Spawn_KoukuCardRainSoldiers(
    LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick, const BOSS_PATTERN_MECHANIC_TRIGGER* tuning)
{
    using namespace LostArk::Shared;
    const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
        [ownerId](const auto& entity) { return entity.iNetEntityId == ownerId; });
    if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || owner == m_WorldEntities.end() ||
        owner->eKind != WORLD_BOOTSTRAP_KIND::BOSS || !owner->iCurrentHp || owner->strPatternId.empty() ||
        !m_ServerNavigation.Is_Loaded() || !m_GameplayCatalog.Get_ActiveRevision().Is_Valid())
    { m_strStatus = "Card rain soldiers preserved: owner or navigation unavailable"; return false; }
    const auto sequence = owner->iPatternSequence;
    if (std::any_of(m_KoukuCardRainSoldiers.begin(), m_KoukuCardRainSoldiers.end(),
        [&](const auto& entry) { return entry.second.ownerId == ownerId && entry.second.patternSequence == sequence; }))
        return true;
    // Source summons use the same three PPCH model families as the maze.
    // Encounter-lived soldiers keep their normal Server monster brain across patterns.
    // A room cap bounds repeated summons; no maze membership is added.
    const std::array<std::uint32_t, 3u> counts = tuning ? tuning->SoldierCounts : std::array<std::uint32_t, 3u>{1u, 1u, 1u};
    const float radiusMin = tuning ? tuning->fSoldierSpawnRadiusMinM : 3.f;
    const float radiusMax = tuning ? tuning->fSoldierSpawnRadiusMaxM : 6.f;
    const std::size_t count = counts[0] + counts[1] + counts[2];
    if (!count || count > 64u || std::any_of(counts.begin(), counts.end(), [](auto n) { return n > 32u; }) ||
        !std::isfinite(radiusMin) || !std::isfinite(radiusMax) || radiusMin < 0.f || radiusMax > 100.f || radiusMin > radiusMax)
    { m_strStatus = "Card rain soldier tuning is invalid"; return false; }
    constexpr std::size_t maximumSoldiers = 128u;
    if (m_KoukuCardRainSoldiers.size() + count > maximumSoldiers)
    { m_strStatus = "Card rain soldier capacity reached; existing soldiers preserved"; return false; }
    constexpr std::array archetypes{ "MONSTER_KOUKU_CARD_CLUB", "MONSTER_KOUKU_CARD_HEART", "MONSTER_KOUKU_CARD_DIAMOND" };
    std::vector<const MONSTER_RUNTIME_PROFILE*> profiles(count);
    std::vector<SPAWN_GROUP_ANCHOR> anchors(count);
    std::vector<std::size_t> families;
    for (std::size_t family = 0u; family < counts.size(); ++family)
        families.insert(families.end(), counts[family], family);
    const float x = owner->fPositionX, y = owner->fPositionY, z = owner->fPositionZ;
    const std::string group = "kouku.cardrain." + std::to_string(ownerId) + "." + std::to_string(sequence);
    std::uint32_t random = ownerId ^ (sequence * 747796405u) ^ tick;
    const auto unit = [&]() { random = random * 1664525u + 1013904223u; return float(random >> 8u) / 16777216.f; };
    for (std::size_t index = 0u; index < count; ++index)
    {
        profiles[index] = m_SpawnGroupBootstrap.Find_Profile(archetypes[families[index]]);
        if (!profiles[index]) { m_strStatus = "Card rain soldier profile unavailable"; return false; }
        bool found = false;
        for (std::uint32_t attempt = 0u; attempt < 48u && !found; ++attempt)
        {
            const float angle = unit() * 6.28318530718f, radius = radiusMin + unit() * (radiusMax - radiusMin);
            SERVER_NAV_POINT position{};
            if (!m_ServerNavigation.Sample_Position(x + std::cos(angle) * radius, z + std::sin(angle) * radius, position) ||
                !std::isfinite(position.y) || std::abs(position.y - y) > 1.f ||
                !m_ServerNavigation.Is_PointWalkableExact(position.x, position.z)) continue;
            bool separated = true;
            for (std::size_t previous = 0u; previous < index; ++previous)
            {
                const float dx = anchors[previous].fPositionX - position.x, dz = anchors[previous].fPositionZ - position.z;
                separated = separated && dx * dx + dz * dz >= 2.25f;
            }
            if (!separated) continue;
            auto& anchor = anchors[index];
            anchor.strAnchorId = group + ".anchor." + std::to_string(index);
            anchor.fPositionX = position.x; anchor.fPositionY = position.y; anchor.fPositionZ = position.z;
            anchor.fYawDegrees = std::atan2(x - position.x, z - position.z) * 57.2957795131f;
            found = true;
        }
        if (!found) { m_strStatus = "Card rain soldiers preserved: not enough separated navigation points"; return false; }
    }
    if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID ||
        m_iNextNetEntityId > (std::numeric_limits<NET_ENTITY_ID>::max)() - count) return false;
    const auto firstId = m_iNextNetEntityId;
    m_WorldEntities.reserve(m_WorldEntities.size() + count);
    for (std::uint32_t index = 0u; index < count; ++index)
    {
        SPAWN_GROUP_ENTRY entry{};
        entry.strArchetypeId = archetypes[families[index]]; entry.strAnchorId = anchors[index].strAnchorId; entry.iCount = 1u;
        const auto id = m_iNextNetEntityId;
        if (!Spawn_Monster(group, entry, anchors[index], *profiles[index], index))
        {
            for (auto it = m_WorldEntities.begin(); it != m_WorldEntities.end();)
            {
                if (it->strSpawnGroupId != group) { ++it; continue; }
                Broadcast_WorldEntityDespawned(it->iNetEntityId);
                m_KoukuCardRainSoldiers.erase(it->iNetEntityId);
                it = m_WorldEntities.erase(it);
            }
            m_iNextNetEntityId = firstId;
            m_strStatus = "Card rain soldier batch rolled back";
            return false;
        }
        m_KoukuCardRainSoldiers.emplace(id, KOUKU_CARD_RAIN_SOLDIER_STATE{
            ownerId, sequence, 0u });
    }
    m_strStatus = "Card rain spawned " + std::to_string(count) + " authoritative card soldiers";
    return true;
}

void LostArk::Server::CGameRoom::Update_KoukuCardRainSoldiers(std::uint32_t tick)
{
    (void)tick;
    for (auto state = m_KoukuCardRainSoldiers.begin(); state != m_KoukuCardRainSoldiers.end();)
    {
        auto entity = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
            [&](const auto& row) { return row.iNetEntityId == state->first; });
        const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
            [&](const auto& row) { return row.iNetEntityId == state->second.ownerId; });
        const bool expired = owner == m_WorldEntities.end() || !owner->iCurrentHp ||
            (entity != m_WorldEntities.end() && !entity->iCurrentHp);
        if (entity == m_WorldEntities.end()) { state = m_KoukuCardRainSoldiers.erase(state); continue; }
        if (!expired) { ++state; continue; }
        m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
        if (!Broadcast_CombatObjectLifecycle()) Mark_RuntimeFailure("card-rain.despawn");
        Broadcast_WorldEntityDespawned(entity->iNetEntityId);
        m_WorldEntities.erase(entity);
        state = m_KoukuCardRainSoldiers.erase(state);
    }
}

bool LostArk::Server::CGameRoom::Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request)
{
	const auto* profile = m_SpawnGroupBootstrap.Find_Profile(CKoukuCardMazeRuntime::Archetype_ForSuit(request.eSuit));
	if (!profile) { m_strStatus = "Card maze target profile missing"; return false; }
	SPAWN_GROUP_ANCHOR anchor{};
	anchor.strAnchorId = "cardmaze.random";
	anchor.fPositionX = request.fPositionX; anchor.fPositionY = request.fPositionY; anchor.fPositionZ = request.fPositionZ;
	anchor.fYawDegrees = request.fYawDegrees;
	SPAWN_GROUP_ENTRY entry{};
	entry.strArchetypeId = profile->strArchetypeId; entry.strAnchorId = anchor.strAnchorId; entry.iCount = 1u;
	const auto id = m_iNextNetEntityId;
	if (!Spawn_Monster(CKoukuCardMazeRuntime::SPAWN_GROUP_TAG, entry, anchor, *profile, 0u)) return false;
	m_KoukuCardMaze.Register_Target(id, request.eSuit);
	return true;
}

void LostArk::Server::CGameRoom::Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id)
{
	const auto e = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
		[id](const SERVER_WORLD_ENTITY& entity) { return entity.iNetEntityId == id; });
	if (e != m_WorldEntities.end())
	{
		m_CombatObjectRuntime.Cancel_Source(id);
		if (!Broadcast_CombatObjectLifecycle()) Mark_RuntimeFailure("card-maze.despawn");
		Broadcast_WorldEntityDespawned(id);
		m_WorldEntities.erase(e);
	}
	m_KoukuCardMaze.Retire_Target(id);
}

bool LostArk::Server::CGameRoom::Begin_CardMazeTransfer(SERVER_PLAYER& player,
	float x, float y, float z, std::uint32_t tick, bool leaving)
{
	SERVER_NAV_POINT ground{};
	if (!m_ServerNavigation.Sample_Position(x, z, ground) || std::abs(ground.y - y) > 1.f)
	{ m_strStatus = "Card maze transfer destination has no matching navigation"; return false; }
	player.fCardMazeTransferX = x; player.fCardMazeTransferY = ground.y; player.fCardMazeTransferZ = z;
	player.bCardMazeTransferCommitted = false;
	player.CardMaze.transferStartTick = tick;
	player.CardMaze.flags = static_cast<std::uint8_t>((player.CardMaze.flags & ~1u) | (leaving ? 8u : 0u));
	player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
	player.PendingCommand.Clear(); player.Clear_SkillTarget(); player.Projectiles.clear();
	player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
	player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION;
	player.iActionStartTick = tick;
	return true;
}

void LostArk::Server::CGameRoom::Update_CardMaze(std::uint32_t tick)
{
	using namespace LostArk::Shared;
	using Maze = CKoukuCardMazeRuntime;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA) return;
	if (m_KoukuCardMaze.Get_Phase() == Maze::PHASE::INACTIVE) { Update_CardMazeClownBox(tick); return; }
	const bool anyLiving = std::any_of(m_Players.begin(), m_Players.end(), [](const auto& entry) {
		return entry.second.eCardMazeRole != CARD_MAZE_ROLE::NONE && entry.second.iCurrentHp > 0u;
	});
	if (!anyLiving) { Reset_CardMaze(); return; }
	const auto distanceSquared = [](float x, float z) { return x * x + z * z; };
	bool pendingTransfer = false;
	for (auto& [id, player] : m_Players)
	{
		if (player.eCardMazeRole == CARD_MAZE_ROLE::NONE) continue;
		player.CardMaze.marchStartTick = m_iCardMazeMarchStartTick;
		player.CardMaze.marchCycleMs = m_iCardMazeCycleMs;
		if (!player.iCurrentHp)
		{
			player.CardMaze.flags = 0u; player.CardMaze.transferStartTick = 0u;
			continue;
		}
		if (player.CardMaze.transferStartTick)
		{
			const auto elapsed = tick - player.CardMaze.transferStartTick;
			pendingTransfer = true;
			if (elapsed >= 18u && !player.bCardMazeTransferCommitted)
			{
				player.fPositionX = player.fCardMazeTransferX;
				player.fPositionY = player.fCardMazeTransferY;
				player.fPositionZ = player.fCardMazeTransferZ;
				player.bCardMazeTransferCommitted = true;
				if (!(player.CardMaze.flags & 8u)) m_KoukuCardMaze.Mark_Escaped(player);
			}
			if (elapsed >= 36u)
			{
				player.CardMaze.transferStartTick = 0u;
				player.eAction = PLAYER_ACTION_STATE::NONE;
				player.fActionElapsedSeconds = 0.f;
			}
			continue;
		}
		if (m_KoukuCardMaze.Get_Phase() != Maze::PHASE::HUNTING) continue;
		if (Maze::In_SafeZone(player.fPositionX, player.fPositionZ))
		{
			// The central lane may cross the telescope: physical overlap is harmless here.
		}
		else
		{
			if (!m_KoukuCardMaze.Is_SoloHunter(id))
				player.CardMaze.flags &= static_cast<std::uint8_t>(~1u);
			bool contact = false;
			const auto previous = m_CardMazePreviousPositions.find(id);
			const auto cooldown = m_CardMazeContactTicks.find(id);
			if (m_iCardMazeMarchStartTick && m_iCardMazeCycleMs &&
				(cooldown == m_CardMazeContactTicks.end() || tick - cooldown->second >= 30u))
			{
				const double absoluteMs = double(tick - m_iCardMazeMarchStartTick) * 1000.0 / 30.0;
				const double beforeMs = (std::max)(0.0, absoluteMs - 1000.0 / 30.0);
				const auto firstCycle = static_cast<std::uint64_t>(beforeMs / m_iCardMazeCycleMs);
				const auto lastCycle = static_cast<std::uint64_t>(absoluteMs / m_iCardMazeCycleMs);
				for (std::uint64_t cycle = firstCycle; cycle <= lastCycle && !contact; ++cycle)
				for (const auto& lane : m_WorldBootstrap.Get_CardMazeLanes())
				{
					const double startMs = double(cycle * m_iCardMazeCycleMs) + lane.delayMs;
					const double endMs = startMs + lane.durationMs;
					if (absoluteMs < startMs || beforeMs >= endMs) continue;
					const float a = static_cast<float>(std::clamp((beforeMs - startMs) / lane.durationMs, 0.0, 1.0));
					const float b = static_cast<float>(std::clamp((absoluteMs - startMs) / lane.durationMs, 0.0, 1.0));
					const float px = previous == m_CardMazePreviousPositions.end() ? player.fPositionX : previous->second.first;
					const float pz = previous == m_CardMazePreviousPositions.end() ? player.fPositionZ : previous->second.second;
					const float ax = px - (lane.startX + (lane.endX - lane.startX) * a);
					const float az = pz - (lane.startZ + (lane.endZ - lane.startZ) * a);
					const float bx = player.fPositionX - (lane.startX + (lane.endX - lane.startX) * b);
					const float bz = player.fPositionZ - (lane.startZ + (lane.endZ - lane.startZ) * b);
					const float vx = bx - ax, vz = bz - az;
					const float len = distanceSquared(vx, vz);
					const float t = len > .000001f ? std::clamp(-(ax * vx + az * vz) / len, 0.f, 1.f) : 0.f;
					// Initial tuned contact radius (Seto + player); independent of model scale.
					if (std::abs(player.fPositionY - lane.startY) < 2.f && distanceSquared(ax + vx * t, az + vz * t) <= 1.44f)
					{ contact = true; break; }
				}
			}
			if (contact && player.eCardMazeSuit != MECHANIC_CARD_SYMBOL::NONE)
			{
				m_CardMazeContactTicks[id] = tick;
				m_KoukuCardMaze.Reset_Progress(player);
				std::vector<NET_ENTITY_ID> replaced;
				for (const auto& [entity, suit] : m_KoukuCardMaze.Get_Targets())
					if (suit == player.eCardMazeSuit) replaced.push_back(entity);
				for (auto entity : replaced) Remove_CardMazeTarget(entity);
			}
		}
		if (player.eCardMazeSuit == MECHANIC_CARD_SYMBOL::NONE || (player.CardMaze.flags & 2u)) continue;
		if (player.iCardMazeKills >= Maze::KILL_TARGET)
		{
			if (!(player.CardMaze.flags & 4u))
			{
				// Only the successful matching kill owns a portal position.
				m_strStatus = "Card maze exit is waiting for its defeated suit position";
			}
			if ((player.CardMaze.flags & 4u) && std::abs(player.fPositionY - player.CardMaze.exitY) < 1.f &&
				distanceSquared(player.fPositionX - player.CardMaze.exitX, player.fPositionZ - player.CardMaze.exitZ) <= 1.f)
				(void)Begin_CardMazeTransfer(player, Maze::CENTER_X, -.01f, Maze::CENTER_Z, tick, false);
		}
		else
		{
			const bool exists = std::any_of(m_KoukuCardMaze.Get_Targets().begin(), m_KoukuCardMaze.Get_Targets().end(),
				[&](const auto& target) { return target.second == player.eCardMazeSuit; });
			if (!exists)
			{
				Maze::SPAWN_REQUEST target{};
				if (m_KoukuCardMaze.Sample_Corridor(player.eCardMazeSuit, m_Players, m_WorldEntities,
					m_ServerNavigation, tick ^ id, target)) (void)Spawn_CardMazeTarget(target);
				else m_strStatus = "Card maze target waiting for a free corridor";
			}
		}
	}
	if (m_KoukuCardMaze.Get_Phase() == Maze::PHASE::COMPLETE)
	{
		if (!pendingTransfer)
		{
			for (auto& [id, player] : m_Players)
				if (player.eCardMazeRole != CARD_MAZE_ROLE::NONE)
				{ (void)id; player.Clear_KoukuInteractionState(); }
			Reset_CardMaze();
		}
		return;
	}
	if (m_KoukuCardMaze.All_LivingCentral(m_Players))
	{
		const auto* destination = Find_Placement("cardmaze.return");
		if (!destination || destination->TriggerActions.size() != 1u)
		{ m_strStatus = "Set the cardmaze.return movePlayer destination in MapTool"; return; }
		const auto& move = destination->TriggerActions.front();
		SERVER_NAV_POINT ground{};
		if (move.eKind != WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER ||
			!m_ServerNavigation.Sample_Position(move.fTargetX, move.fTargetZ, ground) || std::abs(ground.y - move.fTargetY) > 1.f)
		{ m_strStatus = "Card maze return destination is not walkable"; return; }
		m_KoukuCardMaze.Complete();
		m_iCardMazeMarchStartTick = m_iCardMazeCycleMs = 0u;
		Despawn_CardMazeTargets();
		for (auto& [id, player] : m_Players)
			if (player.eCardMazeRole != CARD_MAZE_ROLE::NONE && player.iCurrentHp)
			{ (void)id; (void)Begin_CardMazeTransfer(player, move.fTargetX, move.fTargetY, move.fTargetZ, tick, true); }
	}
}

void LostArk::Server::CGameRoom::Update_CardMazeClownBox(const std::uint32_t tick)
{
	using namespace LostArk::Shared;
	using Maze = CKoukuCardMazeRuntime;
	const bool anyEntered = std::any_of(m_Players.begin(), m_Players.end(), [](const auto& entry) {
		const SERVER_PLAYER& player = entry.second;
		return KOUKU_HUD_MODE::MAZE == player.eKoukuHudMode && player.iCurrentHp > 0u &&
			player.fPositionX >= Maze::MAZE_MIN_X && player.fPositionX <= Maze::MAZE_MAX_X &&
			player.fPositionZ >= Maze::MAZE_MIN_Z && player.fPositionZ <= Maze::MAZE_MAX_Z;
	});
	if (!anyEntered)
	{
		Clear_CardMazeClownBox();
		return;
	}
	if (m_bCardMazeClownBoxDestroyed)
		return;
	if (INVALID_NET_ENTITY_ID != m_iCardMazeClownBoxId)
	{
		const auto box = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[id = m_iCardMazeClownBoxId](const SERVER_WORLD_ENTITY& entity) { return entity.iNetEntityId == id; });
		if (m_WorldEntities.end() == box)
		{
			/* Removed without dying (a Debug despawn): raise it again. */
			m_iCardMazeClownBoxId = INVALID_NET_ENTITY_ID;
			m_iCardMazeClownBoxDueTick = 0u;
		}
		else if (0u == box->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == box->eAction)
		{
			/* Any killing blow opens the telescope; the world update keeps the
			body for its death clip and removes it afterwards. */
			m_bCardMazeClownBoxDestroyed = true;
			m_iCardMazeClownBoxId = INVALID_NET_ENTITY_ID;
			m_strStatus = "Card maze clown box destroyed; strike the telescope with Q to begin";
		}
		return;
	}
	if (0u == m_iCardMazeClownBoxDueTick)
	{
		m_iCardMazeClownBoxDueTick = (std::max)(1u, tick + Maze::CLOWN_BOX_SPAWN_DELAY_TICKS);
		return;
	}
	if (static_cast<std::int32_t>(tick - m_iCardMazeClownBoxDueTick) < 0)
		return;
	const WORLD_BOOTSTRAP_PLACEMENT* telescope = Find_Placement(Maze::TELESCOPE_PLACEMENT_ID);
	const MONSTER_RUNTIME_PROFILE* profile = m_SpawnGroupBootstrap.Find_Profile(Maze::CLOWN_BOX_ARCHETYPE_ID);
	// A failed raise retries once a second and never opens the telescope.
	m_iCardMazeClownBoxDueTick = (std::max)(1u, tick + Maze::CLOWN_BOX_SPAWN_DELAY_TICKS);
	if (nullptr == telescope || nullptr == profile)
	{
		m_strStatus = nullptr == profile ?
			"Card maze clown box profile is not published: " + std::string(Maze::CLOWN_BOX_ARCHETYPE_ID) :
			"Card maze clown box needs the cardmaze.telescope placement";
		return;
	}
	SPAWN_GROUP_ANCHOR anchor{};
	anchor.strAnchorId = Maze::CLOWN_BOX_SPAWN_GROUP_TAG;
	anchor.fPositionX = telescope->fPositionX;
	anchor.fPositionY = telescope->fPositionY;
	anchor.fPositionZ = telescope->fPositionZ;
	anchor.fYawDegrees = telescope->fYawDegrees;
	SPAWN_GROUP_ENTRY entry{};
	entry.strArchetypeId = profile->strArchetypeId;
	entry.strAnchorId = anchor.strAnchorId;
	entry.iCount = 1u;
	const NET_ENTITY_ID spawnedId = m_iNextNetEntityId;
	if (!Spawn_Monster(Maze::CLOWN_BOX_SPAWN_GROUP_TAG, entry, anchor, *profile, 0u))
	{
		m_strStatus = "Card maze clown box could not be spawned at the telescope placement";
		return;
	}
	m_iCardMazeClownBoxId = spawnedId;
	m_iCardMazeClownBoxDueTick = 0u;
	m_strStatus = "Card maze clown box raised; break it with Q to open the telescope";
}

void LostArk::Server::CGameRoom::Clear_CardMazeClownBox()
{
	const LostArk::Shared::NET_ENTITY_ID id = m_iCardMazeClownBoxId;
	m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_iCardMazeClownBoxDueTick = 0u;
	m_bCardMazeClownBoxDestroyed = false;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == id)
		return;
	const auto box = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
		[id](const SERVER_WORLD_ENTITY& entity) { return entity.iNetEntityId == id; });
	/* A living box leaves with the run; a dead one finishes its death clip
	under the generic world update. */
	if (m_WorldEntities.end() == box || 0u == box->iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == box->eAction)
		return;
	m_CombatObjectRuntime.Cancel_Source(id);
	if (!Broadcast_CombatObjectLifecycle())
		Mark_RuntimeFailure("card-maze.clown-box-despawn");
	Broadcast_WorldEntityDespawned(id);
	m_WorldEntities.erase(box);
}
