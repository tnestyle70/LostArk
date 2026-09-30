#include "CombatObjectRuntime.h"
#include "ValtanBrain.h"
#include "ItemCatalog.h"

#include "ServerCombatHitRuntime.h"
#include "PlayerSkillSystem.h"
#include "ColosseumCombatPolicy.h"

#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <new>
#include <utility>

namespace
{
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float SECONDS_TO_MILLISECONDS = 1000.f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;
	constexpr std::size_t MAX_SERVER_COMBAT_OBJECTS = 1024u;

	std::uint32_t DamageOfSubHit(
		const std::uint64_t totalDamage,
		const std::uint32_t subHitTotal,
		const std::uint32_t index)
	{
		const std::uint64_t total = (std::max)(1u, subHitTotal);
		const std::uint64_t share =
			totalDamage * (index + 1u) / total - totalDamage * index / total;
		return static_cast<std::uint32_t>(share < 1u ? 1u : share);
	}

	LostArk::Server::SERVER_COMBAT_SHAPE_XZ ToShape(
		const LostArk::Server::PLAYER_SKILL_HIT& hit)
	{
		using namespace LostArk::Server;
		SERVER_COMBAT_SHAPE_XZ shape{};
		shape.fOffset = hit.fOffset;
		shape.iMaximumTargets = hit.iMaxTargets;
		switch (hit.iAreaType)
		{
		case 1u:
			shape.eKind = hit.fInner > 0.f ?
				SERVER_COMBAT_SHAPE_KIND::RING : SERVER_COMBAT_SHAPE_KIND::CIRCLE;
			shape.fOuterRadius = hit.fRange;
			shape.fInnerRadius = hit.fInner;
			break;
		case 2u:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX;
			shape.fLength = hit.fRange;
			shape.fHalfWidth = hit.fWidth * 0.5f;
			break;
		case 3u:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::CONE;
			shape.fLength = hit.fRange;
			shape.fInnerRadius = hit.fInner;
			shape.fAngleDegrees = hit.fAngleDegrees;
			break;
		default:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::END;
			break;
		}
		return shape;
	}

	LostArk::Server::SERVER_COMBAT_SHAPE_XZ ToShape(
		const LostArk::Server::BOSS_COMBAT_OBJECT_HIT& hit)
	{
		using namespace LostArk::Server;
		SERVER_COMBAT_SHAPE_XZ shape{};
		shape.fOuterRadius = hit.fHitOuterRadius;
		shape.fInnerRadius = hit.fHitInnerRadius;
		shape.fLength = hit.fHitLength;
		shape.fHalfWidth = hit.fHitHalfWidth;
		shape.fAngleDegrees = hit.fHitAngleDegrees;
		switch (hit.eHitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::CIRCLE;
			break;
		case BOSS_PATTERN_HIT_SHAPE::RING:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::RING;
			break;
		case BOSS_PATTERN_HIT_SHAPE::BOX:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX;
			break;
		case BOSS_PATTERN_HIT_SHAPE::CONE:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::CONE;
			break;
		default:
			shape.eKind = SERVER_COMBAT_SHAPE_KIND::END;
			break;
		}
		return shape;
	}

	LostArk::Server::SERVER_PLAYER* FindPlayerByEntityId(
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Server::SERVER_PLAYER>& players,
		const LostArk::Shared::NET_ENTITY_ID netEntityId)
	{
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.iNetEntityId == netEntityId)
				return &player;
		}
		return nullptr;
	}

	LostArk::Server::SERVER_WORLD_ENTITY* FindWorldEntityById(
		std::vector<LostArk::Server::SERVER_WORLD_ENTITY>& entities,
		const LostArk::Shared::NET_ENTITY_ID netEntityId)
	{
		for (LostArk::Server::SERVER_WORLD_ENTITY& entity : entities)
		{
			if (entity.iNetEntityId == netEntityId)
				return &entity;
		}
		return nullptr;
	}

	bool IsDamageable(const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		return LostArk::Server::CServerCombatHitRuntime::Is_PlayerDamageableWorldTarget(entity);
	}

	bool IsDamageable(const LostArk::Server::SERVER_PLAYER& player)
	{
		return 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != player.eAction;
	}

	LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ BodyOf(
		const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		return { entity.fPositionX, entity.fPositionZ, entity.fCollisionRadius };
	}

	LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ BodyOf(
		const LostArk::Server::SERVER_PLAYER& player)
	{
		return {
			player.fPositionX,
			player.fPositionZ,
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X
		};
	}

	LostArk::Server::SERVER_COMBAT_OBJECT_POSE AttackPose(
		LostArk::Server::SERVER_COMBAT_OBJECT_POSE pose,
		const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit)
	{
		pose.fPositionX += pose.fDirectionX * hit.fOffsetForwardM + pose.fDirectionZ * hit.fOffsetRightM;
		pose.fPositionZ += pose.fDirectionZ * hit.fOffsetForwardM - pose.fDirectionX * hit.fOffsetRightM;
		pose.fYawDegrees += hit.fYawOffsetDegrees;
		const float angle = pose.fYawDegrees * DEGREES_TO_RADIANS;
		pose.fDirectionX = std::sin(angle); pose.fDirectionZ = std::cos(angle);
		return pose;
	}

	std::uint32_t AttackDamage(const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit,
		const LostArk::Server::SERVER_PLAYER& target, const std::uint32_t raw)
	{
		if (hit.bInstantDeath) return (std::max)(1u, target.iCurrentHp);
		if (hit.iDamagePercent) return (std::max)(1u, static_cast<std::uint32_t>(
			std::uint64_t(target.iMaximumHp) * hit.iDamagePercent / 100u));
		return raw;
	}

	bool ContactOverlaps(
		const LostArk::Server::SERVER_COMBAT_OBJECT& object,
		const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit,
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target)
	{
		using namespace LostArk::Server;
		if (object.fElapsedMilliseconds < float(hit.iAtMs) ||
			(hit.iEndMs && object.fPreviousElapsedMilliseconds >= float(hit.iEndMs))) return false;
		auto start = object.LiveState.PreviousPose, end = object.LiveState.CurrentPose;
		const float delta = object.fElapsedMilliseconds - object.fPreviousElapsedMilliseconds;
		if (delta > 0.f)
		{
			const auto at = [&](float time) {
				const float alpha = (std::clamp)((time - object.fPreviousElapsedMilliseconds) / delta, 0.f, 1.f);
				auto pose = end;
				pose.fPositionX = start.fPositionX + (end.fPositionX - start.fPositionX) * alpha;
				pose.fPositionZ = start.fPositionZ + (end.fPositionZ - start.fPositionZ) * alpha;
				return pose;
			};
			const auto clippedStart = at(float(hit.iAtMs));
			if (hit.iEndMs) end = at(float(hit.iEndMs));
			start = clippedStart;
		}
		start = AttackPose(start, hit); end = AttackPose(end, hit);
		if (CServerCombatGeometry::Overlaps_Pose(hit.Shape, end.fPositionX, end.fPositionZ,
			end.fDirectionX, end.fDirectionZ, target)) return true;
		return SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT == hit.eContactSampling &&
			SERVER_COMBAT_SHAPE_KIND::CIRCLE == hit.Shape.eKind &&
			CServerCombatGeometry::SweptCircle_Overlaps(start.fPositionX, start.fPositionZ,
				end.fPositionX, end.fPositionZ, hit.Shape.fOuterRadius, target);
	}

	LostArk::Server::SERVER_COMBAT_OBJECT_CONTACT_MARK* FindContactMark(
		LostArk::Server::SERVER_COMBAT_OBJECT& object,
		const LostArk::Shared::NET_ENTITY_ID targetId,
		const std::size_t hitIndex)
	{
		for (auto& mark : object.ContactMarks)
		{
			if (mark.iTargetNetEntityId == targetId &&
				mark.iHitIndex == hitIndex)
			{
				return &mark;
			}
		}
		LostArk::Server::SERVER_COMBAT_OBJECT_CONTACT_MARK mark{};
		mark.iTargetNetEntityId = targetId;
		mark.iHitIndex = static_cast<std::uint16_t>(hitIndex);
		object.ContactMarks.push_back(mark);
		return &object.ContactMarks.back();
	}
}

LostArk::Server::SERVER_COMBAT_OBJECT_TRANSACTION
LostArk::Server::CCombatObjectRuntime::Begin_Transaction() const
{
	SERVER_COMBAT_OBJECT_TRANSACTION transaction{};
	transaction.iExpectedRevision = m_iRevision;
	transaction.iNextCombatObjectId = m_iNextCombatObjectId;
	return transaction;
}

bool LostArk::Server::CCombatObjectRuntime::Allocate_Id(
	SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
	LostArk::Shared::COMBAT_OBJECT_ID& outId) const
{
	using namespace LostArk::Shared;
	if (m_Objects.size() + transaction.Objects.size() >= MAX_SERVER_COMBAT_OBJECTS)
		return false;
	COMBAT_OBJECT_ID candidate = transaction.iNextCombatObjectId;
	if (INVALID_COMBAT_OBJECT_ID == candidate)
		candidate = 1u;
	const std::size_t maximumAttempts = m_Objects.size() +
		transaction.Objects.size() + 1u;
	for (std::size_t attempt = 0u; attempt < maximumAttempts; ++attempt)
	{
		const bool usedLive = std::any_of(
			m_Objects.begin(), m_Objects.end(),
			[candidate](const SERVER_COMBAT_OBJECT& object)
			{
				return object.iCombatObjectId == candidate;
			});
		const bool usedStaged = std::any_of(
			transaction.Objects.begin(), transaction.Objects.end(),
			[candidate](const SERVER_COMBAT_OBJECT& object)
			{
				return object.iCombatObjectId == candidate;
			});
		if (!usedLive && !usedStaged)
		{
			outId = candidate;
			transaction.iNextCombatObjectId =
				(std::numeric_limits<COMBAT_OBJECT_ID>::max)() == candidate ?
				1u : candidate + 1u;
			return true;
		}
		candidate = (std::numeric_limits<COMBAT_OBJECT_ID>::max)() == candidate ?
			1u : candidate + 1u;
	}
	return false;
}

bool LostArk::Server::CCombatObjectRuntime::Stage_WaterGunPresentation(
    SERVER_COMBAT_OBJECT_TRANSACTION& transaction, const SERVER_PLAYER& source,
    const std::uint32_t skillId, const CGameplayCatalog& catalog,
    const std::uint32_t spawnTick, std::string& status, const std::uint32_t projectileIndex) const
{
    const char* archetype = LostArk::Shared::MaharakaWaterGunProjectileArchetype(skillId);
    const auto* skill = LostArk::Shared::Find_MaharakaWaterGunSkill(skillId);
    if (!archetype || !skill || projectileIndex >= skill->iProjectileCount || !source.iNetEntityId || !spawnTick || !catalog.Get_ActiveRevision().Is_Valid() ||
        !std::isfinite(source.fPositionX) || !std::isfinite(source.fPositionY) ||
        !std::isfinite(source.fPositionZ) || !std::isfinite(source.fYawDegrees))
    { status = "Waterpang projectile presentation source is invalid"; return false; }
    SERVER_COMBAT_OBJECT object;
    if (!Allocate_Id(transaction, object.iCombatObjectId))
    { status = "Waterpang projectile presentation capacity is exhausted"; return false; }
    object.eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
    object.iSourcePlayerId = source.iPlayerId;
    object.iSourceNetEntityId = source.iNetEntityId;
    object.iSourceSkillId = skillId;
    object.iSpawnTick = spawnTick;
    object.strCombatObjectArchetypeId = object.strClientVisualId = archetype;
    object.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
    object.bReplicated = object.bPersistentLifetime = object.bRoomOwnedTracking = true;
    object.LiveState.iOwnerPatternSequence = spawnTick;
    object.LiveState.strOwnerPatternId = "maharaka.watergun";
    object.LiveState.strOwnerStageActionId = "maharaka.watergun.shot";
    auto& pose = object.LiveState.CurrentPose;
    const auto launch = LostArk::Shared::Sample_MaharakaWaterGunLaunch(*skill, projectileIndex,
        source.fPositionX, source.fPositionY, source.fPositionZ, source.fYawDegrees);
    pose.fPositionX = launch.fX; pose.fPositionY = launch.fY; pose.fPositionZ = launch.fZ;
    pose.fYawDegrees = launch.fYawDegrees;
    pose.fDirectionX = launch.fDirX;
    pose.fDirectionZ = launch.fDirZ;
    object.LiveState.PreviousPose = pose;
    transaction.Spawned.push_back(To_SpawnedMessage(object));
    transaction.Objects.push_back(std::move(object));
    return true;
}

bool LostArk::Server::CCombatObjectRuntime::Finish_WaterGunPresentation(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId, const LostArk::Shared::NET_ENTITY_ID sourceId,
    const std::uint32_t spawnTick, const std::uint32_t serverTick, const bool impact)
{
    using namespace LostArk::Shared;
    for (std::size_t index = 0u; index < m_Objects.size(); ++index)
    {
        const auto& object = m_Objects[index];
        if (object.iCombatObjectId != objectId) continue;
        if (!serverTick || object.iSourceNetEntityId != sourceId || object.iSpawnTick != spawnTick ||
            !object.bRoomOwnedTracking || !object.Hits.empty() ||
            !Is_MaharakaWaterGunProjectile(object.strCombatObjectArchetypeId)) return false;
        if (impact)
        {
            S2C_COMBAT_OBJECT_PRESENTATION_EVENT event;
            event.iEventSequence = m_iNextPresentationEventSequence++;
            if (!m_iNextPresentationEventSequence) m_iNextPresentationEventSequence = 1u;
            event.iServerTick = serverTick;
            event.iCombatObjectId = objectId; event.iSourceNetEntityId = sourceId;
            event.eKind = COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE;
            event.strCombatObjectArchetypeId = object.strCombatObjectArchetypeId;
            event.strOwnerPatternId = object.LiveState.strOwnerPatternId;
            event.strOwnerStageActionId = object.LiveState.strOwnerStageActionId;
            event.strHitId = "maharaka.watergun.impact";
            event.fPositionX = object.LiveState.CurrentPose.fPositionX;
            event.fPositionY = object.LiveState.CurrentPose.fPositionY;
            event.fPositionZ = object.LiveState.CurrentPose.fPositionZ;
            event.fYawDegrees = object.LiveState.CurrentPose.fYawDegrees;
            event.PinnedDefinitionRevision = object.PinnedDefinitionRevision;
            m_PendingPresentationEvents.push_back(std::move(event));
        }
        Despawn_At(index);
        return true;
    }
    return false;
}

bool LostArk::Server::CCombatObjectRuntime::Stage_BattleItemProjectile(
	SERVER_COMBAT_OBJECT_TRANSACTION& transaction, const SERVER_PLAYER& source,
	const SERVER_BATTLE_ITEM_USE& use, const float targetX, const float targetY, const float targetZ,
	const CGameplayCatalog& catalog, const std::uint32_t serverTick, std::string& status) const
{
	const auto* profile = catalog.Find_Player(source.eCharacterClass);
	const float dx = targetX - source.fPositionX, dz = targetZ - source.fPositionZ;
	const float distance = std::hypot(dx, dz);
	if (!profile || !serverTick || !std::isfinite(distance) || !std::isfinite(targetY) ||
		!source.iNetEntityId || !use.iProjectileSpeedCmPerSecond || !use.iRadiusCm ||
		(use.eKind != BATTLE_ITEM_KIND::DESTRUCTION && use.eKind != BATTLE_ITEM_KIND::WHIRLWIND))
	{ status = "Battle projectile definition is invalid"; return false; }
	SERVER_COMBAT_OBJECT object;
	if (!Allocate_Id(transaction, object.iCombatObjectId))
	{ status = "Battle projectile capacity is exhausted"; return false; }
	object.eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
	object.iSourcePlayerId = source.iPlayerId;
	object.iSourceNetEntityId = source.iNetEntityId;
	object.iSourceSkillId = use.iSkillId;
	object.iSpawnTick = serverTick;
	object.strCombatObjectArchetypeId = use.eKind == BATTLE_ITEM_KIND::DESTRUCTION ?
		"battle.item.destruction_bomb" : "battle.item.whirlwind_grenade";
	object.strClientVisualId = object.strCombatObjectArchetypeId;
	object.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
	object.bReplicated = object.bDetonateOnContact = true;
	object.LiveState.strOwnerPatternId = "battle.item";
	object.LiveState.strOwnerStageActionId = "battle.item.throw";
	object.fSpeedMps = static_cast<float>(use.iProjectileSpeedCmPerSecond) * .01f;
	object.fRemainingDistanceM = object.fProjectileDistanceM = distance;
	object.fProjectileStartY = source.fPositionY + static_cast<float>(use.iProjectileLaunchHeightCm) * .01f;
	object.fProjectileTargetY = targetY;
	object.fProjectileArcHeightM = static_cast<float>(use.iProjectileArcHeightCm) * .01f;
	object.fRemainingMilliseconds = (distance / object.fSpeedMps) * 1000.f + 100.f;
	auto& pose = object.LiveState.CurrentPose;
	pose.fPositionX = source.fPositionX; pose.fPositionY = object.fProjectileStartY; pose.fPositionZ = source.fPositionZ;
	pose.fDirectionX = distance > .0001f ? dx / distance : 0.f;
	pose.fDirectionZ = distance > .0001f ? dz / distance : 1.f;
	pose.fYawDegrees = std::atan2(pose.fDirectionX, pose.fDirectionZ) * RADIANS_TO_DEGREES;
	object.LiveState.PreviousPose = pose;
	object.strContactPresentationId = "battle.item.impact";
	object.fContactPresentationRadiusM = .15f;
	SERVER_COMBAT_OBJECT_HIT_RUNTIME hit;
	hit.strHitId = object.strContactPresentationId;
	hit.eTrigger = SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
	hit.Shape.eKind = SERVER_COMBAT_SHAPE_KIND::CIRCLE;
	hit.Shape.fOuterRadius = static_cast<float>(use.iRadiusCm) * .01f;
	hit.iPartDamage = use.iPartDamage; hit.iStaggerDamage = use.iStaggerDamage;
	hit.iStaggerMaximumDivisor = use.iStaggerMaximumDivisor;
	hit.RepeatRawDamage.push_back(CGameplayCatalog::Resolve_Damage(profile->iAttackPower, use.iDamageRatePercent));
	object.Hits.push_back(std::move(hit));
	transaction.Spawned.push_back(To_SpawnedMessage(object));
	transaction.Objects.push_back(std::move(object));
	return true;
}

bool LostArk::Server::CCombatObjectRuntime::Stage_PlayerProjectile(
	SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
	const SERVER_PLAYER& source,
	const PLAYER_SKILL_DEFINITION& skill,
	const PLAYER_SKILL_PROJECTILE& definition,
	const std::uint32_t stageIndex,
	const std::uint32_t projectileIndex,
	const std::uint64_t totalDamage,
	const std::uint32_t subHitTotal,
	const std::uint32_t subHitBase,
	const std::uint32_t serverTick,
	std::string& status) const
{
	(void)stageIndex;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == source.iNetEntityId ||
		LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId ||
		definition.Hits.empty() || 0u == definition.iLifeMs ||
		projectileIndex > (std::numeric_limits<std::uint8_t>::max)())
	{
		status = "Player combat object spawn is invalid";
		return false;
	}
	SERVER_COMBAT_OBJECT object{};
	if (!Allocate_Id(transaction, object.iCombatObjectId))
	{
		status = "Combat object capacity is exhausted";
		return false;
	}
	object.eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
	object.iSourcePlayerId = source.iPlayerId;
	object.iSourceSkillId = skill.iSkillId;
	object.iSourceNetEntityId = source.iNetEntityId;
	object.iSpawnTick = serverTick;
	/* Player objects must never inherit boss occurrence metadata. */
	object.LiveState.iOwnerPatternSequence = 0u;
	object.LiveState.strOwnerPatternId.clear();
	object.LiveState.strOwnerStageActionId.clear();
	object.LiveState.CurrentPose.fDirectionX = source.fSkillAimDirectionX;
	object.LiveState.CurrentPose.fDirectionZ = source.fSkillAimDirectionZ;
	object.LiveState.CurrentPose.fYawDegrees = std::atan2(
		object.LiveState.CurrentPose.fDirectionX,
		object.LiveState.CurrentPose.fDirectionZ) * RADIANS_TO_DEGREES;
	object.fSpeedMps = definition.fSpeed;
	object.fRemainingMilliseconds = static_cast<float>(definition.iLifeMs);
	object.LiveState.CurrentPose.fPositionX = source.fPositionX;
	object.LiveState.CurrentPose.fPositionY = source.fPositionY;
	object.LiveState.CurrentPose.fPositionZ = source.fPositionZ;
	if (PLAYER_PROJECTILE_ORIGIN::AIM == definition.eOrigin &&
		PLAYER_PROJECTILE_KIND::FIXAREA == definition.eKind)
	{
		const float distance = definition.fMaxDistance > 0.f ?
			(std::min)(source.fSkillAimDistance, definition.fMaxDistance) :
			source.fSkillAimDistance;
		object.LiveState.CurrentPose.fPositionX +=
			object.LiveState.CurrentPose.fDirectionX * distance;
		object.LiveState.CurrentPose.fPositionZ +=
			object.LiveState.CurrentPose.fDirectionZ * distance;
	}
	else
	{
		const float rightX = object.LiveState.CurrentPose.fDirectionZ;
		const float rightZ = -object.LiveState.CurrentPose.fDirectionX;
		object.LiveState.CurrentPose.fPositionX +=
			object.LiveState.CurrentPose.fDirectionX * definition.fOffsetForward +
			rightX * definition.fOffsetRight;
		object.LiveState.CurrentPose.fPositionZ +=
			object.LiveState.CurrentPose.fDirectionZ * definition.fOffsetForward +
			rightZ * definition.fOffsetRight;
	}
	if (PLAYER_PROJECTILE_KIND::FIXAREA == definition.eKind)
		object.fRemainingDistanceM = 0.f;
	else
	{
		object.fRemainingDistanceM = definition.fMaxDistance > 0.f ?
			(std::min)((std::max)(source.fSkillAimDistance,
				definition.fMinDistance), definition.fMaxDistance) : -1.f;
	}

	std::uint32_t resolvedSubHit = subHitBase;
	for (const PLAYER_PROJECTILE_HIT& authored : definition.Hits)
	{
		SERVER_COMBAT_OBJECT_HIT_RUNTIME hit{};
		hit.eTrigger = authored.isContact ?
			SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT :
			SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		hit.iAtMs = authored.Hit.iTimeMs;
		hit.iRepeatIntervalMs = authored.Hit.iRepeatMs;
		hit.Shape = ToShape(authored.Hit);
		hit.iStaggerDamage = skill.iStaggerDamage;
		hit.iPartDamage = skill.iPartDamage;
		hit.iCounterPower = skill.iCounterPower;
		hit.bCounterFromPrimarySlot = Is_PrimaryCounterSkill(skill);
		hit.fPushRangeM = authored.Hit.fPushRange;
		hit.iPushMs = authored.Hit.iPushMs;
        hit.iPlayerWholeCastDamage = totalDamage;
        hit.iPlayerSubHitTotal = subHitTotal;
        hit.iPlayerSubHitBase = resolvedSubHit;
        hit.iPlayerResultKind = authored.Hit.iResultKind;
		for (std::uint32_t repeat = 0u;
			repeat < authored.Hit.iRepeatCount; ++repeat, ++resolvedSubHit)
		{
			hit.RepeatRawDamage.push_back(
				DamageOfSubHit(totalDamage, subHitTotal, resolvedSubHit));
		}
		if (!CServerCombatGeometry::Is_Valid(hit.Shape) ||
			hit.RepeatRawDamage.empty())
		{
			status = "Player combat object hit is invalid";
			return false;
		}
		object.Hits.push_back(std::move(hit));
	}
	object.LiveState.PreviousPose = object.LiveState.CurrentPose;
	transaction.Objects.push_back(std::move(object));
	return true;
}

bool LostArk::Server::CCombatObjectRuntime::Stage_BossCombatObject(
	SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_COMBAT_OBJECT_LOCKED_TARGET* lockedTarget,
	const BOSS_COMBAT_OBJECT_DEFINITION& definition,
	const BOSS_COMBAT_OBJECT_VOLLEY* volley,
	const CGameplayCatalog& catalog,
	const std::uint32_t count,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (0u == count ||
		(definition.Hits.empty() && definition.AttackTemplates.empty() && definition.PresentationPulses.empty()) ||
		0u == definition.iLifeMs ||
		!std::isfinite(definition.fCoverRadiusM) ||
		definition.fCoverRadiusM < 0.f || definition.fCoverRadiusM > 100.f ||
		(definition.fCoverRadiusM > 0.f &&
			(BOSS_COMBAT_OBJECT_KIND::FIXED_AREA != definition.eKind ||
			 definition.Hits.empty() ||
			 std::any_of(definition.Hits.begin(), definition.Hits.end(),
				[](const BOSS_COMBAT_OBJECT_HIT& hit)
				{
					return BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED != hit.eTrigger;
				}))) ||
		definition.strCombatObjectArchetypeId.empty() ||
		definition.strClientVisualId.empty() ||
		definition.strOwnerPatternId.empty() ||
		definition.strOwnerStageActionId.empty() ||
		0u == boss.iPatternSequence ||
		definition.strEncounterId != boss.strEncounterId)
	{
		status = "Boss combat object spawn is invalid";
		return false;
	}
	if (!LostArk::Shared::Validate_AttackHitTemplates(definition.AttackTemplates, definition.iLifeMs))
	{ status = "Boss attack templates exceed their bounded shape, damage or lifetime contract"; return false; }
	if (nullptr != volley)
	{
		const bool radial = BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
			volley->eLayout;
		const bool single = BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE ==
			volley->eLayout;
		const bool perAlivePlayer =
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
				volley->ePolicy;
		const bool bossRelative =
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
				volley->ePolicy;
		const bool arenaCenter =
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER ==
				volley->ePolicy;
		if ((!perAlivePlayer && !bossRelative && !arenaCenter) ||
			count != volley->iCountPerResolvedTarget || count < 1u || count > 8u ||
			static_cast<std::uint64_t>(volley->iMaximumTotalObjects) <
				static_cast<std::uint64_t>(count) +
				static_cast<std::uint64_t>(volley->iArenaRandomCount) ||
			volley->iMaximumTotalObjects > 64u ||
			volley->iSpawnCount < 1u || volley->iSpawnCount > 8u ||
			(1u == volley->iSpawnCount ?
				0u != volley->iSpawnIntervalMs :
				0u == volley->iSpawnIntervalMs) ||
			volley->iArenaRandomCount > 32u ||
			!std::isfinite(volley->fRadiusM) || volley->fRadiusM < 0.f ||
			!std::isfinite(volley->fStartAngleDegrees) ||
			!std::isfinite(volley->fAngleStepDegrees) ||
			!std::isfinite(volley->fArenaRandomRadiusM) ||
			!std::isfinite(volley->fArenaHeightToleranceM) ||
			(0u == volley->iArenaRandomCount ?
				(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE !=
					volley->eArenaAnchorPolicy ||
				 volley->fArenaRandomRadiusM != 0.f ||
				 volley->fArenaHeightToleranceM != 0.f) :
				(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION !=
					volley->eArenaAnchorPolicy ||
				 volley->fArenaRandomRadiusM <= 0.f ||
				 volley->fArenaHeightToleranceM <= 0.f)) ||
			(count > 1u && (!radial || volley->fRadiusM <= 0.f ||
				0.f == volley->fAngleStepDegrees || volley->bAllowOverlap)) ||
			(1u == count && (!single || 0.f != volley->fRadiusM ||
				0.f != volley->fStartAngleDegrees ||
				0.f != volley->fAngleStepDegrees || volley->bAllowOverlap)) ||
			(perAlivePlayer &&
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
					LOCKED_TARGET_PER_ALIVE_PLAYER != definition.eOriginPolicy) ||
			(bossRelative &&
				(BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
						definition.eOriginPolicy || nullptr != lockedTarget ||
				 1u != volley->iSpawnCount || 0u != volley->iArenaRandomCount)))
		{
			status = "Boss combat object volley layout is invalid";
			return false;
		}
	}
	std::size_t replicatedCount = static_cast<std::size_t>(std::count_if(
		transaction.Objects.begin(), transaction.Objects.end(),
		[](const SERVER_COMBAT_OBJECT& object) { return object.bReplicated; }));
	for (const SERVER_COMBAT_OBJECT& existing : m_Objects)
		replicatedCount += existing.bReplicated ? 1u : 0u;
	if (replicatedCount + count > LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT)
	{
		status = "Replicated combat object capacity is exhausted";
		return false;
	}
	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	const float directionX = std::sin(yawRadians);
	const float directionZ = std::cos(yawRadians);
	for (std::uint32_t ordinal = 0u; ordinal < count; ++ordinal)
	{
		SERVER_COMBAT_OBJECT object{};
		if (!Allocate_Id(transaction, object.iCombatObjectId))
		{
			status = "Combat object capacity is exhausted";
			return false;
		}
		object.eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY;
		object.iSourceNetEntityId = boss.iNetEntityId;
		object.strSourceArchetypeId = boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ? "BOSS_VALTAN_GHOST" : boss.strArchetypeId;
		object.iSpawnTick = serverTick;
		object.strCombatObjectArchetypeId =
			definition.strCombatObjectArchetypeId;
		object.strClientVisualId = definition.strClientVisualId;
		object.PinnedDefinitionRevision = boss.PinnedDefinitionRevision;
		if (!object.PinnedDefinitionRevision.Is_Valid())
		{
			status = "Boss combat object definition revision is unavailable";
			return false;
		}
		object.bReplicated = true;
		object.LiveState.iOwnerPatternSequence = boss.iPatternSequence;
		object.LiveState.strOwnerPatternId = definition.strOwnerPatternId;
		object.LiveState.strOwnerStageActionId =
			definition.strOwnerStageActionId;
		object.LiveState.CurrentPose.fDirectionX = directionX;
		object.LiveState.CurrentPose.fDirectionZ = directionZ;
		object.LiveState.CurrentPose.fYawDegrees = boss.fYawDegrees;
		object.fSpeedMps = definition.fSpeedMps;
		object.iMovementStartDelayMs = definition.iMovementStartDelayMs;
		object.bExpireOnDistanceEnd = definition.bExpireOnDistanceEnd;
		object.fCoverRadiusM = definition.fCoverRadiusM;
		object.OwnerHitChain = definition.OwnerHitChain;
		object.fRemainingDistanceM =
			BOSS_COMBAT_OBJECT_KIND::MISSILE == definition.eKind ?
			definition.fMaximumDistanceM : 0.f;
		object.fRemainingMilliseconds = static_cast<float>(definition.iLifeMs);
		/* Both resolved-origin policies start at the pose supplied by the room.
		Player instances optionally follow their entity until the first timed
		pulse; arena-random instances deliberately keep the same pose. */
		if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE ==
				definition.eOriginPolicy ||
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
				definition.eOriginPolicy)
		{
			const bool shouldTrackUntilFirstPulse =
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE ==
					definition.eOriginPolicy ||
				(nullptr != lockedTarget && lockedTarget->bTrackUntilFirstPulse);
			if (nullptr == lockedTarget ||
				!std::isfinite(lockedTarget->fPositionX) ||
				!std::isfinite(lockedTarget->fPositionY) ||
				!std::isfinite(lockedTarget->fPositionZ) ||
				(shouldTrackUntilFirstPulse &&
				 LostArk::Shared::INVALID_NET_ENTITY_ID ==
					lockedTarget->iNetEntityId))
			{
				status = "Boss combat object resolved origin is unavailable";
				return false;
			}
			object.iLockedTargetNetEntityId = lockedTarget->iNetEntityId;
			object.bTrackLockedTargetUntilFirstPulse =
				shouldTrackUntilFirstPulse;
			if (nullptr != volley &&
				BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL == volley->eLayout)
			{
				const float radialDegrees = volley->fStartAngleDegrees +
					volley->fAngleStepDegrees * static_cast<float>(ordinal);
				const float radialRadians = radialDegrees * DEGREES_TO_RADIANS;
				object.fLockedTargetOffsetX =
					std::sin(radialRadians) * volley->fRadiusM;
				object.fLockedTargetOffsetZ =
					std::cos(radialRadians) * volley->fRadiusM;
			}
			object.LiveState.CurrentPose.fPositionX = lockedTarget->fPositionX +
				object.fLockedTargetOffsetX;
			object.LiveState.CurrentPose.fPositionY = lockedTarget->fPositionY;
			object.LiveState.CurrentPose.fPositionZ = lockedTarget->fPositionZ +
				object.fLockedTargetOffsetZ;
		}
		else
		{
			/* ARENA_CENTER anchors on the boss spawn placement (the authored arena
			centre) with world-absolute angles; every other policy keeps the live
			boss pose and yaw. */
			const bool arenaCenterVolley = nullptr != volley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER == volley->ePolicy;
			const float originX =
				arenaCenterVolley ? boss.fSpawnPositionX : boss.fPositionX;
			const float originY =
				arenaCenterVolley ? boss.fSpawnPositionY : boss.fPositionY;
			const float originZ =
				arenaCenterVolley ? boss.fSpawnPositionZ : boss.fPositionZ;
			const float yawBasisDegrees =
				arenaCenterVolley ? 0.f : boss.fYawDegrees;
			const float rightX = directionZ;
			const float rightZ = -directionX;
			object.LiveState.CurrentPose.fPositionX = originX +
				directionX * definition.fOffsetForwardM +
				rightX * definition.fOffsetRightM;
			object.LiveState.CurrentPose.fPositionY = originY;
			object.LiveState.CurrentPose.fPositionZ = originZ +
				directionZ * definition.fOffsetForwardM +
				rightZ * definition.fOffsetRightM;
			if (nullptr != volley &&
				(BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
					volley->ePolicy || arenaCenterVolley))
			{
				const float relativeDegrees = volley->fStartAngleDegrees +
					volley->fAngleStepDegrees * static_cast<float>(ordinal);
				const float worldDegrees = yawBasisDegrees + relativeDegrees;
				const float worldRadians = worldDegrees * DEGREES_TO_RADIANS;
				const float radialDirectionX = std::sin(worldRadians);
				const float radialDirectionZ = std::cos(worldRadians);
				object.LiveState.CurrentPose.fPositionX +=
					radialDirectionX * volley->fRadiusM;
				object.LiveState.CurrentPose.fPositionZ +=
					radialDirectionZ * volley->fRadiusM;
				const bool nextRadialSlot =
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT ==
						definition.eDirectionPolicy;
				const bool radialInward =
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD ==
						definition.eDirectionPolicy;
				if (nextRadialSlot)
				{
					/* The spawn remains the current radial slot. Its movement vector is
					the exact chord to the following authored slot, including the final
					ordinal's natural angular wrap back to slot zero. */
					const std::uint32_t nextOrdinal = (ordinal + 1u) % count;
					const float nextWorldDegrees = yawBasisDegrees +
						volley->fStartAngleDegrees +
						volley->fAngleStepDegrees *
							static_cast<float>(nextOrdinal);
					const float nextWorldRadians =
						nextWorldDegrees * DEGREES_TO_RADIANS;
					const float routeX =
						(std::sin(nextWorldRadians) - radialDirectionX) *
						volley->fRadiusM;
					const float routeZ =
						(std::cos(nextWorldRadians) - radialDirectionZ) *
						volley->fRadiusM;
					const float routeLengthSquared = routeX * routeX + routeZ * routeZ;
					if (!std::isfinite(routeLengthSquared) ||
						routeLengthSquared <= 0.000001f)
					{
						status = "Boss combat object next radial slot is invalid";
						return false;
					}
					const float inverseRouteLength =
						1.f / std::sqrt(routeLengthSquared);
					object.LiveState.CurrentPose.fDirectionX =
						routeX * inverseRouteLength;
					object.LiveState.CurrentPose.fDirectionZ =
						routeZ * inverseRouteLength;
					object.LiveState.CurrentPose.fYawDegrees = std::atan2(
						object.LiveState.CurrentPose.fDirectionX,
						object.LiveState.CurrentPose.fDirectionZ) *
						RADIANS_TO_DEGREES;
				}
				else
				{
					/* Preserve the existing radial inward/outward behavior for every
					pre-existing direction policy. */
					object.LiveState.CurrentPose.fDirectionX = radialInward ?
						-radialDirectionX : radialDirectionX;
					object.LiveState.CurrentPose.fDirectionZ = radialInward ?
						-radialDirectionZ : radialDirectionZ;
					object.LiveState.CurrentPose.fYawDegrees = radialInward ?
						worldDegrees + 180.f : worldDegrees;
				}
			}
		}
		for (const BOSS_COMBAT_OBJECT_HIT& authored : definition.Hits)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME hit{};
			hit.strHitId = authored.strHitId;
			hit.strDamageProfileId = authored.strDamageProfileId;
			hit.eTrigger =
				BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT == authored.eTrigger ?
				SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT :
				SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
			hit.eContactSampling =
				BOSS_COMBAT_OBJECT_KIND::MISSILE == definition.eKind ?
				SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT :
				SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::POSE;
			hit.iAtMs = authored.iAtMs;
			hit.iRepeatIntervalMs = authored.iRepeatIntervalMs;
			hit.Shape = ToShape(authored);
			hit.fPushRangeM = authored.fPushRangeM;
			hit.iPushMs = authored.iPushMs;
			hit.bKnockdown = authored.bKnockdown;
			hit.iDownMs = authored.iDownMs;
			const std::uint32_t rawDamage = CGameplayCatalog::Resolve_Damage(
				boss.iAttackPower,
				catalog.Find_DamageRatePercent(authored.strDamageProfileId));
			for (std::uint32_t repeat = 0u;
				repeat < authored.iRepeatCount; ++repeat)
			{
				hit.RepeatRawDamage.push_back(rawDamage);
			}
			if (!CServerCombatGeometry::Is_Valid(hit.Shape) ||
				hit.RepeatRawDamage.empty() || 0u == rawDamage)
			{
				status = "Boss combat object hit is invalid";
				return false;
			}
			object.Hits.push_back(std::move(hit));
		}
		for (const auto& authored : definition.AttackTemplates)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME hit;
			hit.strHitId = authored.strHitId;
			hit.strDamageProfileId = authored.strDamageProfileId;
			hit.eTrigger = authored.strTrigger == "CONTACT" ? SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT : SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
			hit.eContactSampling = SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT;
			hit.iAtMs = authored.iAtMs; hit.iEndMs = authored.iEndMs;
			hit.iRepeatIntervalMs = authored.iRepeatIntervalMs;
			hit.fOffsetForwardM = float(authored.fOffsetForwardM); hit.fOffsetRightM = float(authored.fOffsetRightM);
			hit.fYawOffsetDegrees = float(authored.fYawOffsetDegrees);
			hit.Shape.eKind = authored.strShape == "CIRCLE" ? SERVER_COMBAT_SHAPE_KIND::CIRCLE :
				authored.strShape == "RING" ? SERVER_COMBAT_SHAPE_KIND::RING :
				authored.strShape == "BOX" ? SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX : SERVER_COMBAT_SHAPE_KIND::CONE;
			hit.Shape.fOuterRadius = float(authored.fRadiusM); hit.Shape.fInnerRadius = float(authored.fInnerRadiusM);
			hit.Shape.fLength = float(authored.fLengthM); hit.Shape.fHalfWidth = float(authored.fHalfWidthM);
			hit.Shape.fAngleDegrees = float(authored.fAngleDegrees);
			hit.bInstantDeath = authored.strDamageKind == "INSTANT_DEATH" ||
				(authored.strDamageKind == "MAX_HP_PERCENT" && authored.iDamagePercent >= 100u);
			hit.bIgnoreDefense = authored.strDamageKind != "PROFILE";
			hit.iDamagePercent = authored.iDamagePercent;
			hit.strNumericBalanceId = authored.strNumericBalanceId;
			hit.fRiseHeightM = static_cast<float>(authored.fRiseHeightM);
			hit.fPushRangeM = static_cast<float>(authored.fPushRangeM);
			hit.bForcePush = authored.ForcePush.value_or(authored.fRiseHeightM > 0.0);
			hit.bPushFromBoss = authored.strPushDirection == "AWAY_FROM_BOSS";
			hit.iPushMs = authored.iPushMs;
			const auto rate = authored.strDamageKind == "PROFILE" ? catalog.Find_DamageRatePercent(authored.strDamageProfileId) : 1u;
			if (!rate || !CServerCombatGeometry::Is_Valid(hit.Shape))
			{ status = "Boss attack damage profile or primitive is unresolved"; return false; }
			const auto raw = authored.strDamageKind == "PROFILE" ? CGameplayCatalog::Resolve_Damage(boss.iAttackPower, rate) : 1u;
			if (!raw) { status = "Boss attack resolves to zero damage"; return false; }
			hit.RepeatRawDamage.assign(authored.iRepeatCount, raw);
			object.Hits.push_back(std::move(hit));
		}
		for (const BOSS_COMBAT_OBJECT_PRESENTATION_PULSE& authored :
			definition.PresentationPulses)
		{
			if (authored.strPresentationEventId.empty() ||
				authored.iAtMs > definition.iLifeMs)
			{
				status = "Boss combat object presentation pulse is invalid";
				return false;
			}
			SERVER_COMBAT_OBJECT_PRESENTATION_PULSE_RUNTIME pulse{};
			pulse.strPresentationEventId = authored.strPresentationEventId;
			pulse.iAtMs = authored.iAtMs;
			object.PresentationPulses.push_back(std::move(pulse));
		}
		object.LiveState.PreviousPose = object.LiveState.CurrentPose;
		try
		{
			if (object.bReplicated)
				transaction.Spawned.push_back(To_SpawnedMessage(object));
			transaction.Objects.push_back(std::move(object));
		}
		catch (const std::bad_alloc&)
		{
			status = "Boss combat object transaction allocation failed";
			return false;
		}
	}
	return true;
}

bool LostArk::Server::CCombatObjectRuntime::Commit(
	SERVER_COMBAT_OBJECT_TRANSACTION&& transaction)
{
	if (transaction.iExpectedRevision != m_iRevision ||
		m_Objects.size() + transaction.Objects.size() > MAX_SERVER_COMBAT_OBJECTS ||
		transaction.Spawned.size() != static_cast<std::size_t>(std::count_if(
			transaction.Objects.begin(), transaction.Objects.end(),
			[](const SERVER_COMBAT_OBJECT& object)
			{ return object.bReplicated; })))
	{
		return false;
	}
	std::vector<SERVER_COMBAT_OBJECT> stagedObjects;
	std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED> stagedSpawned;
	try
	{
		stagedObjects = m_Objects;
		stagedSpawned = m_PendingSpawned;
		stagedObjects.reserve(m_Objects.size() + transaction.Objects.size());
		stagedSpawned.reserve(
			m_PendingSpawned.size() + transaction.Spawned.size());
		stagedObjects.insert(
			stagedObjects.end(),
			std::make_move_iterator(transaction.Objects.begin()),
			std::make_move_iterator(transaction.Objects.end()));
		stagedSpawned.insert(
			stagedSpawned.end(),
			std::make_move_iterator(transaction.Spawned.begin()),
			std::make_move_iterator(transaction.Spawned.end()));
	}
	catch (const std::bad_alloc&)
	{
		return false;
	}
	m_Objects.swap(stagedObjects);
	m_PendingSpawned.swap(stagedSpawned);
	m_iNextCombatObjectId = transaction.iNextCombatObjectId;
	++m_iRevision;
	if (0u == m_iRevision)
		m_iRevision = 1u;
	return true;
}

void LostArk::Server::CCombatObjectRuntime::Update(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog, const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	Update_Objects(players, worldEntities, catalog, fixedDeltaSeconds, serverTick, outDamageEvents, false);
}

void LostArk::Server::CCombatObjectRuntime::Apply_OwnerHits(
	const std::vector<SERVER_BOSS_PATTERN_HIT>& hits,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog, const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	bool armedAny = false;
	for (const auto& hit : hits)
	{
		for (std::size_t seedIndex = 0u; seedIndex < m_Objects.size(); ++seedIndex)
		{
			const auto& seed = m_Objects[seedIndex];
			if (seed.bOwnerHitChainArmed || seed.OwnerHitChain.strTriggerActionId.empty() ||
				seed.eSourceKind != SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY ||
				seed.iSourceNetEntityId != hit.iSourceNetEntityId ||
				seed.LiveState.iOwnerPatternSequence != hit.iPatternSequence ||
				seed.LiveState.strOwnerPatternId != hit.strPatternId ||
				seed.OwnerHitChain.strTriggerActionId != hit.strActionId)
				continue;
			std::vector<std::size_t> indices;
			std::vector<LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ> circles;
			for (std::size_t index = seedIndex; index < m_Objects.size(); ++index)
			{
				const auto& object = m_Objects[index];
				if (!object.bOwnerHitChainArmed && object.iSourceNetEntityId == seed.iSourceNetEntityId &&
					object.LiveState.iOwnerPatternSequence == seed.LiveState.iOwnerPatternSequence &&
					object.strCombatObjectArchetypeId == seed.strCombatObjectArchetypeId &&
					object.iSpawnTick == seed.iSpawnTick)
				{
					indices.push_back(index);
					circles.push_back({object.LiveState.CurrentPose.fPositionX,
						object.LiveState.CurrentPose.fPositionZ, object.fCoverRadiusM});
				}
			}
			std::vector<std::uint32_t> delays;
			if (!LostArk::Shared::CombatObjectHitChain::Resolve_ConeDelays(
				hit.Cone, circles, seed.OwnerHitChain.iDelayMs, delays))
				continue;
			for (std::size_t ordinal = 0u; ordinal < indices.size(); ++ordinal)
			{
				auto& object = m_Objects[indices[ordinal]];
				object.bOwnerHitChainArmed = true;
				object.iOwnerHitChainArmedTick = serverTick;
				object.iOwnerHitChainDelayMs = delays[ordinal];
			}
			armedAny = true;
		}
	}
	if (armedAny)
		Update_Objects(players, worldEntities, catalog, 0.f, serverTick, outDamageEvents, true);
}

void LostArk::Server::CCombatObjectRuntime::Update_Objects(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	const bool onlyNewlyArmed)
{
	const float deltaMilliseconds = fixedDeltaSeconds * SECONDS_TO_MILLISECONDS;
	for (std::size_t objectIndex = 0u; objectIndex < m_Objects.size();)
	{
		SERVER_COMBAT_OBJECT& object = m_Objects[objectIndex];
		if (onlyNewlyArmed && (!object.bOwnerHitChainArmed || object.iOwnerHitChainArmedTick != serverTick))
		{
			++objectIndex;
			continue;
		}
		SERVER_PLAYER* sourcePlayer = nullptr;
		SERVER_WORLD_ENTITY* sourceEntity = nullptr;
		if (SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER == object.eSourceKind)
			sourcePlayer = FindPlayerByEntityId(players, object.iSourceNetEntityId);
		else
			sourceEntity = FindWorldEntityById(worldEntities, object.iSourceNetEntityId);
		const bool sourceInvalid =
			(nullptr != sourcePlayer &&
				(LostArk::Shared::PLAYER_ACTION_STATE::DEAD == sourcePlayer->eAction ||
				 LostArk::Shared::PLAYER_ACTION_STATE::FALLING == sourcePlayer->eAction ||
				 0u == sourcePlayer->iCurrentHp)) ||
			(nullptr != sourceEntity &&
				(SERVER_ENTITY_ACTION::DEAD == sourceEntity->eAction ||
				 0u == sourceEntity->iCurrentHp)) ||
			(nullptr == sourcePlayer && nullptr == sourceEntity);
		if (sourceInvalid)
		{
			Despawn_At(objectIndex);
			continue;
		}
		/* Objects created by a player update or a boss stage edge start moving on
		the following fixed tick, matching the old per-player projectile order. */
		if (object.iSpawnTick == serverTick)
		{
			++objectIndex;
			continue;
		}

		if (!object.bHasExternalPoseStep) object.LiveState.PreviousPose = object.LiveState.CurrentPose;
		object.bHasExternalPoseStep = false;
		if (object.bTrackLockedTargetUntilFirstPulse)
		{
			if (SERVER_PLAYER* target = FindPlayerByEntityId(
				players, object.iLockedTargetNetEntityId))
			{
				if (target->Is_Human() && IsDamageable(*target))
				{
					object.LiveState.CurrentPose.fPositionX =
						target->fPositionX + object.fLockedTargetOffsetX;
					object.LiveState.CurrentPose.fPositionY =
						target->fPositionY;
					object.LiveState.CurrentPose.fPositionZ =
						target->fPositionZ + object.fLockedTargetOffsetZ;
				}
			}
		}
		const float previousElapsedMilliseconds = object.fElapsedMilliseconds;
		object.fPreviousElapsedMilliseconds = previousElapsedMilliseconds;
		object.fElapsedMilliseconds += deltaMilliseconds;
		if (!object.bPersistentLifetime) object.fRemainingMilliseconds -= deltaMilliseconds;
		float homingDistance = -1.f;
		if (object.bHoming)
		{
			auto* target = FindPlayerByEntityId(players, object.iLockedTargetNetEntityId);
			if (!target || target->Is_Guide() || !IsDamageable(*target)) { Despawn_At(objectIndex); continue; }
			const float dx = target->fPositionX - object.LiveState.CurrentPose.fPositionX;
			const float dz = target->fPositionZ - object.LiveState.CurrentPose.fPositionZ;
			homingDistance = std::hypot(dx, dz);
			if (!std::isfinite(homingDistance)) { Despawn_At(objectIndex); continue; }
			if (homingDistance > .000001f)
			{
				object.LiveState.CurrentPose.fDirectionX = dx / homingDistance;
				object.LiveState.CurrentPose.fDirectionZ = dz / homingDistance;
				object.LiveState.CurrentPose.fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
			}
			object.LiveState.CurrentPose.fPositionY = target->fPositionY;
		}
		float movementStep = 0.f;
		if (object.fSpeedMps > 0.f && (object.fRemainingDistanceM > 0.f || object.bPersistentLifetime))
		{
			const float movementStartMilliseconds =
				static_cast<float>(object.iMovementStartDelayMs);
			const float activeMilliseconds =
				(std::max)(0.f, object.fElapsedMilliseconds - movementStartMilliseconds) -
				(std::max)(0.f, previousElapsedMilliseconds - movementStartMilliseconds);
			float step = object.fSpeedMps * activeMilliseconds /
				SECONDS_TO_MILLISECONDS;
			if (homingDistance >= 0.f) step = (std::min)(step, homingDistance);
			if (!object.bPersistentLifetime && object.fRemainingDistanceM >= 0.f)
			{
				step = (std::min)(step, object.fRemainingDistanceM);
				object.fRemainingDistanceM -= step;
			}
			movementStep = step;
			object.LiveState.CurrentPose.fPositionX +=
				object.LiveState.CurrentPose.fDirectionX * step;
			object.LiveState.CurrentPose.fPositionZ +=
				object.LiveState.CurrentPose.fDirectionZ * step;
		}
		if (object.bDetonateOnContact)
		{
			const float progress = object.fProjectileDistanceM > .0001f ?
				std::clamp(1.f - object.fRemainingDistanceM / object.fProjectileDistanceM, 0.f, 1.f) : 1.f;
			object.LiveState.CurrentPose.fPositionY = object.fProjectileStartY +
				(object.fProjectileTargetY - object.fProjectileStartY) * progress +
				4.f * object.fProjectileArcHeightM * progress * (1.f - progress);
		}
		const bool lifetimeExpired = !object.bPersistentLifetime && object.fRemainingMilliseconds <= 0.f;
		const bool expired = lifetimeExpired ||
			(object.bExpireOnDistanceEnd && object.fSpeedMps > 0.f &&
			 object.fRemainingDistanceM <= 0.f);
		const bool contactMotionActive = object.fSpeedMps <= 0.f ||
			(object.fElapsedMilliseconds >=
				static_cast<float>(object.iMovementStartDelayMs) &&
			 (movementStep > 0.f || object.fRemainingDistanceM > 0.f));
		bool firedFirstTimedPulse = false;
		const auto QueuePresentationPulse =
			[this, &object, serverTick](
				const std::string& pulseId, const std::uint32_t repeatIndex)
			{
				if (!object.bReplicated ||
					(SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY != object.eSourceKind &&
					 !object.bDetonateOnContact))
				{
					return;
				}
				LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT event{};
				event.iEventSequence = m_iNextPresentationEventSequence;
				event.iServerTick = serverTick;
				event.iCombatObjectId = object.iCombatObjectId;
				event.iSourceNetEntityId = object.iSourceNetEntityId;
				event.eKind = LostArk::Shared::
					COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE;
				event.strCombatObjectArchetypeId =
					object.strCombatObjectArchetypeId;
				event.strOwnerPatternId = object.LiveState.strOwnerPatternId;
				event.strOwnerStageActionId =
					object.LiveState.strOwnerStageActionId;
				event.strHitId = pulseId;
				event.iRepeatIndex = repeatIndex;
				event.fPositionX = object.LiveState.CurrentPose.fPositionX;
				event.fPositionY = object.LiveState.CurrentPose.fPositionY;
				event.fPositionZ = object.LiveState.CurrentPose.fPositionZ;
				event.fYawDegrees = object.LiveState.CurrentPose.fYawDegrees;
				event.PinnedDefinitionRevision = object.PinnedDefinitionRevision;
				m_PendingPresentationEvents.push_back(std::move(event));
				++m_iNextPresentationEventSequence;
				if (0u == m_iNextPresentationEventSequence)
					m_iNextPresentationEventSequence = 1u;
			};

		if (object.bOwnerHitChainArmed && !object.bOwnerHitChainWarningApplied &&
			static_cast<std::uint64_t>(serverTick - object.iOwnerHitChainArmedTick) * 1000u >=
			static_cast<std::uint64_t>(object.iOwnerHitChainDelayMs) * SERVER_TICK_HZ)
		{
			// The direct/delayed classification schedules the full preparation visual.
			// The hit's own atMs below adds its authored preparation duration.
			object.bOwnerHitChainWarningApplied = true;
			QueuePresentationPulse(object.OwnerHitChain.strArmedPresentationEventId, 0u);
		}

		bool contactTermination = false;
		if (!object.strContactPresentationId.empty() && contactMotionActive)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME contact;
			contact.eContactSampling = SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT;
			contact.Shape.eKind = SERVER_COMBAT_SHAPE_KIND::CIRCLE;
			contact.Shape.fOuterRadius = object.fContactPresentationRadiusM;
			bool contacted = false;
			if (object.bDetonateOnContact)
			{
				SERVER_WORLD_ENTITY* first = nullptr;
				SERVER_PLAYER* firstPlayer = nullptr;
				float nearest = (std::numeric_limits<float>::max)();
				for (auto& target : worldEntities)
				{
					if (!IsDamageable(target) || !ContactOverlaps(object, contact, BodyOf(target))) continue;
					const float distance = std::hypot(target.fPositionX - object.LiveState.PreviousPose.fPositionX,
						target.fPositionZ - object.LiveState.PreviousPose.fPositionZ);
					if (distance < nearest) { nearest = distance; first = &target; }
				}
				if (sourcePlayer) for (auto& [id, target] : players)
				{
					if (!CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, target) || !ContactOverlaps(object, contact, BodyOf(target))) continue;
					const float distance = std::hypot(target.fPositionX - object.LiveState.PreviousPose.fPositionX,
						target.fPositionZ - object.LiveState.PreviousPose.fPositionZ);
					if (distance < nearest) { nearest = distance; firstPlayer = &target; first = nullptr; }
				}
				if (firstPlayer)
				{
					object.LiveState.CurrentPose.fPositionX = firstPlayer->fPositionX;
					object.LiveState.CurrentPose.fPositionY = firstPlayer->fPositionY;
					object.LiveState.CurrentPose.fPositionZ = firstPlayer->fPositionZ;
					QueuePresentationPulse(object.strContactPresentationId, 0u); contacted = true;
				}
				else if (first)
				{
					object.LiveState.CurrentPose.fPositionX = first->fPositionX;
					object.LiveState.CurrentPose.fPositionY = first->fPositionY;
					object.LiveState.CurrentPose.fPositionZ = first->fPositionZ;
					QueuePresentationPulse(object.strContactPresentationId, 0u);
					contacted = true;
				}
			}
			if (!object.bDetonateOnContact) for (auto& [id, player] : players)
			{
				if (sourcePlayer && !CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, player)) continue;
				// Card pursuit selects only its steering target. Every player can intercept
				// the card; its suit-specific damage immunity is evaluated below.
				const bool targetOnly = object.bHoming &&
					object.eDamageImmuneCardSymbol == LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
				if (!IsDamageable(player) || player.Has_TimeStop(serverTick) || (targetOnly && player.iNetEntityId != object.iLockedTargetNetEntityId) ||
					!ContactOverlaps(object, contact, BodyOf(player))) continue;
				// A matching interception rescues this card occurrence's captive. Other
				// owners and later binds must survive an older card reaching the player.
				if (player.bPatternBound &&
					object.eDamageImmuneCardSymbol != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
					player.eMechanicCardSymbol == object.eDamageImmuneCardSymbol &&
					player.iPatternBindOwnerNetEntityId == object.iSourceNetEntityId &&
					player.iPatternBindSequence == object.LiveState.iOwnerPatternSequence)
					player.Clear_PatternBindStatus();
				// A swept hit can cross the whole body in one tick. Anchor its contact
				// burst on the hit player's authoritative pose, never the overshot endpoint.
				object.LiveState.CurrentPose.fPositionX = player.fPositionX;
				object.LiveState.CurrentPose.fPositionY = player.fPositionY;
				object.LiveState.CurrentPose.fPositionZ = player.fPositionZ;
				QueuePresentationPulse(object.strContactPresentationId, 0u);
				contacted = true;
				break;
			}
			contactTermination = contacted;
		}

		if (expired && !contactTermination && !object.strContactPresentationId.empty())
		{
			QueuePresentationPulse(object.strContactPresentationId, 0u);
			contactTermination = true;
		}

		for (SERVER_COMBAT_OBJECT_PRESENTATION_PULSE_RUNTIME& pulse :
			object.PresentationPulses)
		{
			if (pulse.bApplied ||
				(!lifetimeExpired &&
				 object.fElapsedMilliseconds < static_cast<float>(pulse.iAtMs)))
			{
				continue;
			}
			QueuePresentationPulse(pulse.strPresentationEventId, 0u);
			pulse.bApplied = true;
		}

		for (std::size_t hitIndex = 0u; hitIndex < object.Hits.size(); ++hitIndex)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit = object.Hits[hitIndex];
			if (object.bDetonateOnContact && !contactTermination) continue;
			const auto attackPose = AttackPose(object.LiveState.CurrentPose, hit);
			const auto hitPvpTargets = [&](const bool contact, const std::uint32_t rawDamage)
			{
				if (!sourcePlayer || !sourcePlayer->iColosseumMatchId) return;
				std::vector<std::pair<float, SERVER_PLAYER*>> targets;
				for (auto& [id, target] : players)
				{
					if (!CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, target)) continue;
					const bool overlaps = contact ? ContactOverlaps(object, hit, BodyOf(target)) :
						CServerCombatGeometry::Overlaps_Pose(hit.Shape, attackPose.fPositionX, attackPose.fPositionZ,
							attackPose.fDirectionX, attackPose.fDirectionZ, BodyOf(target));
					if (overlaps) targets.emplace_back(std::hypot(target.fPositionX-attackPose.fPositionX,target.fPositionZ-attackPose.fPositionZ), &target);
				}
				std::sort(targets.begin(), targets.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
				std::size_t admitted = 0u;
				for (const auto& [distance, target] : targets)
				{
					if (hit.Shape.iMaximumTargets && admitted >= hit.Shape.iMaximumTargets) break;
                    if (contact && hit.Shape.iMaximumTargets)
                    {
                        const auto existing = std::find_if(object.ContactMarks.begin(), object.ContactMarks.end(),
                            [&](const auto& entry) { return entry.iHitIndex == hitIndex && entry.iTargetNetEntityId == target->iNetEntityId; });
                        const auto count = std::count_if(object.ContactMarks.begin(), object.ContactMarks.end(),
                            [&](const auto& entry) { return entry.iHitIndex == hitIndex; });
                        if (existing == object.ContactMarks.end() && count >= hit.Shape.iMaximumTargets) continue;
                    }
					auto* mark = contact ? FindContactMark(object, target->iNetEntityId, hitIndex) : nullptr;
					if (mark && (mark->iAppliedCount >= hit.RepeatRawDamage.size() || object.fElapsedMilliseconds < mark->fNextMilliseconds)) continue;
					const auto* skill = catalog.Find_Skill(object.iSourceSkillId);
                    if (!skill) continue;
                    const auto subHitIndex = hit.iPlayerSubHitBase +
                        (mark ? mark->iAppliedCount : hit.iAppliedTimedCount);
                    PLAYER_SKILL_HIT shape{};
                    shape.iResultKind = hit.iPlayerResultKind;
                    shape.fPushRange = hit.fPushRangeM; shape.iPushMs = hit.iPushMs;
                    SERVER_COLOSSEUM_COMBAT_CONTEXT context{ LostArk::Shared::WORLD_ID::COLOSSEUM,
                        sourcePlayer->iColosseumMatchId, sourcePlayer->bColosseumCombatActive, {} };
                    CPlayerSkillSystem::Apply_ColosseumObjectHit(*target, *sourcePlayer, *skill, catalog, shape,
                        hit.iPlayerWholeCastDamage,
                        hit.iPlayerSubHitTotal, subHitIndex, object.LiveState.CurrentPose.fPositionX,
                        object.LiveState.CurrentPose.fPositionZ, serverTick, context, outDamageEvents);
					++admitted;
					if (mark) { ++mark->iAppliedCount; mark->fNextMilliseconds = object.fElapsedMilliseconds + hit.iRepeatIntervalMs; }
				}
			};
			if (SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT == hit.eTrigger)
			{
				if (!contactMotionActive)
					continue;
				hitPvpTargets(true, 0u);
				if (nullptr != sourcePlayer)
				{
					for (SERVER_WORLD_ENTITY& target : worldEntities)
					{
						if (!IsDamageable(target) || !ContactOverlaps(
							object, hit, BodyOf(target)))
							continue;
						auto* mark = FindContactMark(
							object, target.iNetEntityId, hitIndex);
						if (mark->iAppliedCount >= hit.RepeatRawDamage.size() ||
							object.fElapsedMilliseconds < mark->fNextMilliseconds)
							continue;
						SERVER_PLAYER_TO_WORLD_HIT incoming{};
						incoming.iSourcePlayerId = object.iSourcePlayerId;
                        incoming.bGuideSource = sourcePlayer && sourcePlayer->Is_Guide();
						incoming.iSkillId = object.iSourceSkillId;
						incoming.iRawDamage =
							hit.RepeatRawDamage[mark->iAppliedCount];
						incoming.iStaggerDamage = hit.iStaggerDamage;
						incoming.iStaggerMaximumDivisor = hit.iStaggerMaximumDivisor;
						incoming.iPartDamage = hit.iPartDamage;
						incoming.bDestructionBombSource =
							object.strCombatObjectArchetypeId == "battle.item.destruction_bomb" &&
							object.LiveState.strOwnerPatternId == "battle.item" &&
							object.LiveState.strOwnerStageActionId == "battle.item.throw" &&
							hit.strHitId == "battle.item.impact";
						incoming.iCounterPower = hit.iCounterPower;
						incoming.bCounterFromPrimarySlot = hit.bCounterFromPrimarySlot;
						incoming.fSourceX =
							object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ =
							object.LiveState.CurrentPose.fPositionZ;
						incoming.fFallbackDirectionX =
							object.LiveState.CurrentPose.fDirectionX;
						incoming.fFallbackDirectionZ =
							object.LiveState.CurrentPose.fDirectionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_PlayerToWorld(
							target, incoming, outDamageEvents);
						++mark->iAppliedCount;
						mark->fNextMilliseconds = object.fElapsedMilliseconds +
							static_cast<float>(hit.iRepeatIntervalMs);
					}
				}
				else
				{
					for (auto& [playerId, target] : players)
					{
						(void)playerId;
						if (!target.iCurrentHp || (!hit.bInstantDeath && (!IsDamageable(target) || target.Has_TimeStop(serverTick))) ||
                            (object.eDamageImmuneCardSymbol != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
                             target.eMechanicCardSymbol == object.eDamageImmuneCardSymbol) || !ContactOverlaps(
							object, hit, BodyOf(target)))
							continue;
						auto* mark = FindContactMark(
							object, target.iNetEntityId, hitIndex);
						if (mark->iAppliedCount >= hit.RepeatRawDamage.size() ||
							object.fElapsedMilliseconds < mark->fNextMilliseconds)
							continue;
						SERVER_WORLD_TO_PLAYER_HIT incoming{};
						incoming.bInstantDeath = hit.bInstantDeath;
						incoming.bIgnoreDefense = hit.bIgnoreDefense;
						incoming.bIgnoreCounter = hit.bIgnoreDefense;
						incoming.iRawDamage =
							AttackDamage(hit, target, hit.RepeatRawDamage[mark->iAppliedCount]);
						incoming.fSourceX =
							object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ =
							object.LiveState.CurrentPose.fPositionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.fPushHeightM = hit.fRiseHeightM;
						incoming.bPushBallistic = hit.fRiseHeightM > 0.f;
						incoming.bForcePush = hit.bForcePush;
						if (hit.bPushFromBoss && sourceEntity)
						{ incoming.fSourceX = sourceEntity->fPositionX; incoming.fSourceZ = sourceEntity->fPositionZ; }
						incoming.bKnockdown = hit.bKnockdown;
						incoming.iDownMs = hit.iDownMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
							target, incoming, catalog, outDamageEvents);
						++mark->iAppliedCount;
						mark->fNextMilliseconds = object.fElapsedMilliseconds +
							static_cast<float>(hit.iRepeatIntervalMs);
					}
				}
				continue;
			}

			while (hit.iAppliedTimedCount < hit.RepeatRawDamage.size())
			{
				const float dueMilliseconds = static_cast<float>(hit.iAtMs) +
					static_cast<float>(hit.iRepeatIntervalMs) *
					static_cast<float>(hit.iAppliedTimedCount);
				if (!object.OwnerHitChain.strTriggerActionId.empty())
				{
					const auto elapsedTicks = serverTick - object.iOwnerHitChainArmedTick;
					if (!object.bOwnerHitChainArmed ||
						static_cast<std::uint64_t>(elapsedTicks) * 1000u <
						(static_cast<std::uint64_t>(object.iOwnerHitChainDelayMs) + hit.iAtMs) * SERVER_TICK_HZ)
						break;
				}
				else if (object.fElapsedMilliseconds < dueMilliseconds)
					break;
				firedFirstTimedPulse = true;
				if (!object.bDetonateOnContact) QueuePresentationPulse(hit.strHitId, hit.iAppliedTimedCount);
				const std::uint32_t rawDamage =
					hit.RepeatRawDamage[hit.iAppliedTimedCount];
				hitPvpTargets(false, rawDamage);
				if (nullptr != sourcePlayer)
				{
					std::vector<std::pair<float, SERVER_WORLD_ENTITY*>> targets;
					for (SERVER_WORLD_ENTITY& target : worldEntities)
					{
						if (!IsDamageable(target) ||
							!CServerCombatGeometry::Overlaps_Pose(
								hit.Shape,
								attackPose.fPositionX,
								attackPose.fPositionZ,
								attackPose.fDirectionX,
								attackPose.fDirectionZ,
								BodyOf(target)))
							continue;
						const float dx = target.fPositionX -
							object.LiveState.CurrentPose.fPositionX;
						const float dz = target.fPositionZ -
							object.LiveState.CurrentPose.fPositionZ;
						targets.emplace_back(dx * dx + dz * dz, &target);
					}
					std::sort(targets.begin(), targets.end(),
						[](const auto& left, const auto& right)
						{
							return left.first < right.first;
						});
					if (0u != hit.Shape.iMaximumTargets &&
						targets.size() > hit.Shape.iMaximumTargets)
					{
						targets.resize(hit.Shape.iMaximumTargets);
					}
					for (auto& [distanceSquared, target] : targets)
					{
						(void)distanceSquared;
						SERVER_PLAYER_TO_WORLD_HIT incoming{};
						incoming.iSourcePlayerId = object.iSourcePlayerId;
                        incoming.bGuideSource = sourcePlayer && sourcePlayer->Is_Guide();
						incoming.iSkillId = object.iSourceSkillId;
						incoming.iRawDamage = rawDamage;
						incoming.iStaggerDamage = hit.iStaggerDamage;
						incoming.iStaggerMaximumDivisor = hit.iStaggerMaximumDivisor;
						incoming.iPartDamage = hit.iPartDamage;
						incoming.bDestructionBombSource =
							object.strCombatObjectArchetypeId == "battle.item.destruction_bomb" &&
							object.LiveState.strOwnerPatternId == "battle.item" &&
							object.LiveState.strOwnerStageActionId == "battle.item.throw" &&
							hit.strHitId == "battle.item.impact";
						incoming.iCounterPower = hit.iCounterPower;
						incoming.bCounterFromPrimarySlot = hit.bCounterFromPrimarySlot;
						incoming.fSourceX =
							object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ =
							object.LiveState.CurrentPose.fPositionZ;
						incoming.fFallbackDirectionX =
							object.LiveState.CurrentPose.fDirectionX;
						incoming.fFallbackDirectionZ =
							object.LiveState.CurrentPose.fDirectionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_PlayerToWorld(
							*target, incoming, outDamageEvents);
					}
				}
				else
				{
					for (auto& [playerId, target] : players)
					{
						(void)playerId;
						if (!target.iCurrentHp || (!hit.bInstantDeath && (!IsDamageable(target) || target.Has_TimeStop(serverTick))) ||
                            (object.eDamageImmuneCardSymbol != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
                             target.eMechanicCardSymbol == object.eDamageImmuneCardSymbol) ||
							!CServerCombatGeometry::Overlaps_Pose(
								hit.Shape,
								attackPose.fPositionX,
								attackPose.fPositionZ,
								attackPose.fDirectionX,
								attackPose.fDirectionZ,
								BodyOf(target)))
							continue;
						SERVER_WORLD_TO_PLAYER_HIT incoming{};
						incoming.bInstantDeath = hit.bInstantDeath;
						incoming.bIgnoreDefense = hit.bIgnoreDefense;
						incoming.bIgnoreCounter = hit.bIgnoreDefense;
						incoming.iRawDamage = AttackDamage(hit, target, rawDamage);
						incoming.fSourceX =
							object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ =
							object.LiveState.CurrentPose.fPositionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.fPushHeightM = hit.fRiseHeightM;
						incoming.bPushBallistic = hit.fRiseHeightM > 0.f;
						incoming.bForcePush = hit.bForcePush;
						if (hit.bPushFromBoss && sourceEntity)
						{ incoming.fSourceX = sourceEntity->fPositionX; incoming.fSourceZ = sourceEntity->fPositionZ; }
						incoming.bKnockdown = hit.bKnockdown;
						incoming.iDownMs = hit.iDownMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
							target, incoming, catalog, outDamageEvents);
					}
				}
				++hit.iAppliedTimedCount;
			}
		}
		if (firedFirstTimedPulse)
			object.bTrackLockedTargetUntilFirstPulse = false;
		// A completed chain has no live spawn to replay to late joiners.
		// The reliable hit pulse owns the independent terminal visual tail.
		if (expired || contactTermination ||
			(firedFirstTimedPulse && !object.OwnerHitChain.strTriggerActionId.empty()))
			Despawn_At(objectIndex);
		else
			++objectIndex;
	}
}

void LostArk::Server::CCombatObjectRuntime::Cancel_Source(
	const LostArk::Shared::NET_ENTITY_ID sourceNetEntityId)
{
	for (std::size_t index = 0u; index < m_Objects.size();)
	{
		if (m_Objects[index].iSourceNetEntityId == sourceNetEntityId)
			Despawn_At(index);
		else
			++index;
	}
}

bool LostArk::Server::CCombatObjectRuntime::Set_OwnedVisualPosition(
	const LostArk::Shared::COMBAT_OBJECT_ID objectId,
	const LostArk::Shared::NET_ENTITY_ID sourceId, const std::uint32_t patternSequence,
	const float x, const float y, const float z)
{
	if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) return false;
	for (auto& object : m_Objects)
	{
		if (object.iCombatObjectId != objectId) continue;
		if (object.iSourceNetEntityId != sourceId || object.LiveState.iOwnerPatternSequence != patternSequence ||
			(!object.Hits.empty() && !object.bRoomOwnedTracking) || object.fSpeedMps != 0.f || object.bTrackLockedTargetUntilFirstPulse) return false;
		if (!object.bHasExternalPoseStep) object.LiveState.PreviousPose = object.LiveState.CurrentPose;
		object.bHasExternalPoseStep = true;
		auto& pose = object.LiveState.CurrentPose;
		pose.fPositionX = x; pose.fPositionY = y; pose.fPositionZ = z;
		++m_iRevision;
		if (m_iRevision == 0u) m_iRevision = 1u;
		return true;
	}
	return false;
}

bool LostArk::Server::CCombatObjectRuntime::Cancel_OwnedVisualObject(
	const LostArk::Shared::COMBAT_OBJECT_ID objectId,
	const LostArk::Shared::NET_ENTITY_ID sourceId, const std::uint32_t patternSequence)
{
	for (std::size_t index = 0u; index < m_Objects.size(); ++index)
	{
		const auto& object = m_Objects[index];
		if (object.iCombatObjectId != objectId) continue;
		if (object.iSourceNetEntityId != sourceId || object.LiveState.iOwnerPatternSequence != patternSequence ||
			(!object.Hits.empty() && !object.bRoomOwnedTracking)) return false;
		Despawn_At(index);
		return true;
	}
	return false;
}

void LostArk::Server::CCombatObjectRuntime::Reset()
{
	while (!m_Objects.empty())
		Despawn_At(m_Objects.size() - 1u);
}

void LostArk::Server::CCombatObjectRuntime::Discard_PendingLifecycle()
{
	m_PendingSpawned.clear();
	m_PendingPresentationEvents.clear();
	m_PendingDespawned.clear();
}

void LostArk::Server::CCombatObjectRuntime::Despawn_At(const std::size_t index)
{
	if (index >= m_Objects.size())
		return;
	if (m_Objects[index].bReplicated)
	{
		LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED message{};
		message.iCombatObjectId = m_Objects[index].iCombatObjectId;
		m_PendingDespawned.push_back(message);
	}
	m_Objects.erase(m_Objects.begin() + index);
	++m_iRevision;
	if (0u == m_iRevision)
		m_iRevision = 1u;
}

LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED
LostArk::Server::CCombatObjectRuntime::To_SpawnedMessage(
	const SERVER_COMBAT_OBJECT& object)
{
	LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED message{};
	message.iCombatObjectId = object.iCombatObjectId;
	message.iSourceNetEntityId = object.iSourceNetEntityId;
	message.iSpawnTick = object.iSpawnTick;
	message.iServerTick = object.iSpawnTick;
	message.strCombatObjectArchetypeId = object.strCombatObjectArchetypeId;
	message.strClientVisualId = object.strClientVisualId;
	message.fPositionX = object.LiveState.CurrentPose.fPositionX;
	message.fPositionY = object.LiveState.CurrentPose.fPositionY;
	message.fPositionZ = object.LiveState.CurrentPose.fPositionZ;
	message.fYawDegrees = object.LiveState.CurrentPose.fYawDegrees;
	message.fUniformScale = object.fUniformScale;
	message.PinnedDefinitionRevision = object.PinnedDefinitionRevision;
	return message;
}

void LostArk::Server::CCombatObjectRuntime::Drain_Lifecycle(
	std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned,
	std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT>&
		outPresentationEvents,
	std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED>& outDespawned)
{
	outSpawned = std::move(m_PendingSpawned);
	outPresentationEvents = std::move(m_PendingPresentationEvents);
	outDespawned = std::move(m_PendingDespawned);
	m_PendingSpawned.clear();
	m_PendingPresentationEvents.clear();
	m_PendingDespawned.clear();
}

void LostArk::Server::CCombatObjectRuntime::Build_LiveSpawnMessages(
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned) const
{
	outSpawned.clear();
	for (const SERVER_COMBAT_OBJECT& object : m_Objects)
	{
		if (object.bReplicated)
		{
			auto spawned = To_SpawnedMessage(object);
			spawned.iServerTick = serverTick;
			outSpawned.push_back(std::move(spawned));
		}
	}
	std::sort(outSpawned.begin(), outSpawned.end(),
		[](const auto& left, const auto& right)
		{
			return left.iCombatObjectId < right.iCombatObjectId;
		});
}

bool LostArk::Server::CCombatObjectRuntime::Build_Snapshots(
	std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& outSnapshots) const
{
	outSnapshots.clear();
	for (const SERVER_COMBAT_OBJECT& object : m_Objects)
	{
		if (!object.bReplicated)
			continue;
		LostArk::Shared::COMBAT_OBJECT_SNAPSHOT snapshot{};
		snapshot.iCombatObjectId = object.iCombatObjectId;
		snapshot.iSourceNetEntityId = object.iSourceNetEntityId;
		snapshot.fPositionX = object.LiveState.CurrentPose.fPositionX;
		snapshot.fPositionY = object.LiveState.CurrentPose.fPositionY;
		snapshot.fPositionZ = object.LiveState.CurrentPose.fPositionZ;
		snapshot.fYawDegrees = object.LiveState.CurrentPose.fYawDegrees;
		snapshot.PinnedDefinitionRevision = object.PinnedDefinitionRevision;
		outSnapshots.push_back(snapshot);
	}
	std::sort(outSnapshots.begin(), outSnapshots.end(),
		[](const auto& left, const auto& right)
		{
			return left.iCombatObjectId < right.iCombatObjectId;
		});
	return outSnapshots.size() <= LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT;
}


void LostArk::Server::CCombatObjectRuntime::Refresh_NumericBalance(
    const CGameplayCatalog& previous, const CGameplayCatalog& updated,
    const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries) noexcept
{
    for (auto& object : m_Objects)
    {
        const auto* oldSkill = previous.Find_Skill(object.iSourceSkillId);
        const auto* newSkill = updated.Find_Skill(object.iSourceSkillId);
        const auto* boss = updated.Find_Boss(object.strSourceArchetypeId);
        std::uint32_t oldPlayerDamage = 0u, newPlayerDamage = 0u;
        if (oldSkill && newSkill)
        {
            const auto* oldPlayer = previous.Find_Player(oldSkill->eCharacterClass);
            const auto* newPlayer = updated.Find_Player(newSkill->eCharacterClass);
            const auto* oldDamage = previous.Find_DamageProfile(oldSkill->strDamageProfileId);
            const auto* newDamage = updated.Find_DamageProfile(newSkill->strDamageProfileId);
            if (oldPlayer && newPlayer && oldDamage && newDamage)
            {
                oldPlayerDamage = CGameplayCatalog::Resolve_Damage(oldPlayer->iAttackPower, *oldDamage);
                newPlayerDamage = CGameplayCatalog::Resolve_Damage(newPlayer->iAttackPower, *newDamage);
            }
        }
        for (auto& hit : object.Hits)
        {
            if (newSkill)
            {
                hit.iStaggerDamage = newSkill->iStaggerDamage;
                hit.iPartDamage = newSkill->iPartDamage;
                if (oldPlayerDamage && newPlayerDamage)
                    for (auto& damage : hit.RepeatRawDamage)
                        damage = static_cast<std::uint32_t>((std::min)(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()),
                            static_cast<std::uint64_t>(damage) * newPlayerDamage / oldPlayerDamage));
            }
            else if (boss && !hit.strDamageProfileId.empty())
            {
                const auto rate = updated.Find_DamageRatePercent(hit.strDamageProfileId);
                if (rate) std::fill(hit.RepeatRawDamage.begin(), hit.RepeatRawDamage.end(), CGameplayCatalog::Resolve_Damage(boss->iAttackPower, rate));
            }
            if (!hit.strNumericBalanceId.empty())
                for (const auto& entry : entries)
                    if (entry.eDomain == LostArk::Shared::BALANCE_DOMAIN::PATTERN_DAMAGE &&
                        entry.strId == hit.strNumericBalanceId && entry.strField == "maxHpDamagePercent")
                    { hit.iDamagePercent = static_cast<std::uint32_t>(entry.fValue); break; }
        }
    }
}
