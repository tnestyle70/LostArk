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
	// 월드 거리/속도는 m와 m/s, 저작 타이밍은 ms, 소유 연쇄 예약은 30Hz tick으로 비교한다.
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float SECONDS_TO_MILLISECONDS = 1000.f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;
	constexpr std::size_t MAX_SERVER_COMBAT_OBJECTS = 1024u;

	// 전체 피해를 누적 정수 비율의 차이로 분할한다. 각 결과를 최소 1로 올리므로 작은 전체 피해의 분할 합이 원래 값과 같다고 보장하지 않는다.
	std::uint32_t DamageOfSubHit(const std::uint64_t totalDamage,
	                             const std::uint32_t subHitTotal,
	                             const std::uint32_t index)
	{
		const std::uint64_t total = (std::max)(1u, subHitTotal);
		const std::uint64_t share = totalDamage * (index + 1u) / total - totalDamage * index / total;
		return static_cast<std::uint32_t>(share < 1u ? 1u : share);
	}

	// player hit의 도형 종류/치수를 공용 XZ 값으로 옮긴다. 박스의 전체 폭은 반폭으로 바꾸고 지원하지 않는 종류는 END로 남긴다.
	LostArk::Server::SERVER_COMBAT_SHAPE_XZ ToShape(const LostArk::Server::PLAYER_SKILL_HIT& hit)
	{
		using namespace LostArk::Server;
		SERVER_COMBAT_SHAPE_XZ shape{};
		shape.fOffset = hit.fOffset;
		shape.iMaximumTargets = hit.iMaxTargets;
		switch (hit.iAreaType)
		{
		case 1u:
			shape.eKind =
			    hit.fInner > 0.f ? SERVER_COMBAT_SHAPE_KIND::RING : SERVER_COMBAT_SHAPE_KIND::CIRCLE;
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

	// 보스 hit의 이미 m 단위인 도형 값을 복사한다. 유효성 검사는 Stage_BossCombatObject가 다음 단계에서 수행한다.
	LostArk::Server::SERVER_COMBAT_SHAPE_XZ ToShape(const LostArk::Server::BOSS_COMBAT_OBJECT_HIT& hit)
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

	// map의 key는 PlayerId이므로 NetEntityId는 값들을 순회해 찾는다. 반환 포인터는 players가 소유하며 이 호출의 처리 동안만 차용한다.
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

	// world entity vector의 값을 차용한다. 호출 중 vector 재할당/제거가 일어나면 포인터를 계속 사용할 수 없다.
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

	// 플레이어 공격이 때릴 수 있는 월드 대상 자격은 공용 피해 runtime의 규칙을 사용한다.
	bool IsDamageable(const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		return LostArk::Server::CServerCombatHitRuntime::Is_PlayerDamageableWorldTarget(entity);
	}

	// 추적/접촉 후보의 기본 생존·준비·행동 상태를 확인한다. 시간 정지와 최종 무적/저항 검사는 호출 경로에서 별도로 처리한다.
	bool IsDamageable(const LostArk::Server::SERVER_PLAYER& player)
	{
		return 0u != player.iCurrentHp && player.isCombatReady &&
		       LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
		       LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction &&
		       LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != player.eAction;
	}

	// 월드 대상은 현재 XZ와 개체별 충돌 반경으로 판정한다. Y는 이 원에 포함되지 않는다.
	LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ BodyOf(
	    const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		return {entity.fPositionX, entity.fPositionZ, entity.fCollisionRadius};
	}

	// 플레이어도 XZ 원으로 투영하며 반경은 공용 몸체의 X 반크기를 사용한다.
	LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ BodyOf(const LostArk::Server::SERVER_PLAYER& player)
	{
		return {player.fPositionX,
		        player.fPositionZ,
		        LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X};
	}

	// 오브젝트 자세의 복사본에 원래 전방/오른쪽 기준 m offset을 더한 뒤 yaw offset을 적용한다. 저장된 원본 pose는 바꾸지 않는다.
	LostArk::Server::SERVER_COMBAT_OBJECT_POSE AttackPose(
	    LostArk::Server::SERVER_COMBAT_OBJECT_POSE pose,
	    const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit)
	{
		pose.fPositionX += pose.fDirectionX * hit.fOffsetForwardM + pose.fDirectionZ * hit.fOffsetRightM;
		pose.fPositionZ += pose.fDirectionZ * hit.fOffsetForwardM - pose.fDirectionX * hit.fOffsetRightM;
		pose.fYawDegrees += hit.fYawOffsetDegrees;
		const float angle = pose.fYawDegrees * DEGREES_TO_RADIANS;
		pose.fDirectionX = std::sin(angle);
		pose.fDirectionZ = std::cos(angle);
		return pose;
	}

	// 보스 hit의 즉사/최대 HP 비율/고정 raw 우선순위를 결정한다. 실제 방어·시간 정지·밀기 적용은 다음 피해 runtime에서 처리한다.
	std::uint32_t AttackDamage(const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit,
	                           const LostArk::Server::SERVER_PLAYER& target,
	                           const std::uint32_t raw)
	{
		if (hit.bInstantDeath)
			return (std::max)(1u, target.iCurrentHp);
		if (hit.iDamagePercent)
			return (std::max)(1u,
			                  static_cast<std::uint32_t>(std::uint64_t(target.iMaximumHp) *
			                                             hit.iDamagePercent / 100u));
		return raw;
	}

	// 오브젝트가 이번 tick에 지나온 시간 구간을 CONTACT 활성 구간으로 자른다. 대상은 현재 XZ 원이며 과거 대상 이동을 복원하지 않는다.
	// 잘린 pose의 XZ만 선형 보간하고 방향/Y는 현재값을 사용한다. 끝 도형이 겹치지 않으면 원형 SWEPT만 추가 검사한다.
	bool ContactOverlaps(const LostArk::Server::SERVER_COMBAT_OBJECT& object,
	                     const LostArk::Server::SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit,
	                     const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target)
	{
		using namespace LostArk::Server;
		if (object.fElapsedMilliseconds < float(hit.iAtMs) ||
		    (hit.iEndMs && object.fPreviousElapsedMilliseconds >= float(hit.iEndMs)))
			return false;
		auto start = object.LiveState.PreviousPose, end = object.LiveState.CurrentPose;
		const float delta = object.fElapsedMilliseconds - object.fPreviousElapsedMilliseconds;
		if (delta > 0.f)
		{
			const auto at = [&](float time)
			{
				const float alpha =
				    (std::clamp)((time - object.fPreviousElapsedMilliseconds) / delta, 0.f, 1.f);
				auto pose = end;
				pose.fPositionX = start.fPositionX + (end.fPositionX - start.fPositionX) * alpha;
				pose.fPositionZ = start.fPositionZ + (end.fPositionZ - start.fPositionZ) * alpha;
				return pose;
			};
			const auto clippedStart = at(float(hit.iAtMs));
			if (hit.iEndMs)
				end = at(float(hit.iEndMs));
			start = clippedStart;
		}
		start = AttackPose(start, hit);
		end = AttackPose(end, hit);
		if (CServerCombatGeometry::Overlaps_Pose(hit.Shape,
		                                         end.fPositionX,
		                                         end.fPositionZ,
		                                         end.fDirectionX,
		                                         end.fDirectionZ,
		                                         target))
			return true;
		return SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT == hit.eContactSampling &&
		       SERVER_COMBAT_SHAPE_KIND::CIRCLE == hit.Shape.eKind &&
		       CServerCombatGeometry::SweptCircle_Overlaps(start.fPositionX,
		                                                   start.fPositionZ,
		                                                   end.fPositionX,
		                                                   end.fPositionZ,
		                                                   hit.Shape.fOuterRadius,
		                                                   target);
	}

	// 접촉한 대상과 hit index의 소비 기록을 찾거나 0회 기록을 추가한다. 반환 포인터는 이후 ContactMarks 재할당 전까지만 유효하다.
	LostArk::Server::SERVER_COMBAT_OBJECT_CONTACT_MARK* FindContactMark(
	    LostArk::Server::SERVER_COMBAT_OBJECT& object,
	    const LostArk::Shared::NET_ENTITY_ID targetId,
	    const std::size_t hitIndex)
	{
		for (auto& mark : object.ContactMarks)
		{
			if (mark.iTargetNetEntityId == targetId && mark.iHitIndex == hitIndex)
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

// 각 Stage 호출이 공유할 빈 후보를 만든다. Commit은 이 expected revision을 현재 runtime과 대조한다.
LostArk::Server::SERVER_COMBAT_OBJECT_TRANSACTION LostArk::Server::CCombatObjectRuntime::Begin_Transaction()
    const
{
	SERVER_COMBAT_OBJECT_TRANSACTION transaction{};
	transaction.iExpectedRevision = m_iRevision;
	transaction.iNextCombatObjectId = m_iNextCombatObjectId;
	return transaction;
}

// 동시에 유지할 전체 개수 상한을 먼저 확인하고 live/staged 양쪽의 ID 충돌을 피한다. 실패는 outId와 다음 ID를 바꾸지 않는다.
bool LostArk::Server::CCombatObjectRuntime::Allocate_Id(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
                                                        LostArk::Shared::COMBAT_OBJECT_ID& outId) const
{
	using namespace LostArk::Shared;
	if (m_Objects.size() + transaction.Objects.size() >= MAX_SERVER_COMBAT_OBJECTS)
		return false;
	COMBAT_OBJECT_ID candidate = transaction.iNextCombatObjectId;
	if (INVALID_COMBAT_OBJECT_ID == candidate)
		candidate = 1u;
	const std::size_t maximumAttempts = m_Objects.size() + transaction.Objects.size() + 1u;
	for (std::size_t attempt = 0u; attempt < maximumAttempts; ++attempt)
	{
		const bool usedLive = std::any_of(m_Objects.begin(),
		                                  m_Objects.end(),
		                                  [candidate](const SERVER_COMBAT_OBJECT& object)
		                                  {
			                                  return object.iCombatObjectId == candidate;
		                                  });
		const bool usedStaged = std::any_of(transaction.Objects.begin(),
		                                    transaction.Objects.end(),
		                                    [candidate](const SERVER_COMBAT_OBJECT& object)
		                                    {
			                                    return object.iCombatObjectId == candidate;
		                                    });
		if (!usedLive && !usedStaged)
		{
			outId = candidate;
			transaction.iNextCombatObjectId =
			    (std::numeric_limits<COMBAT_OBJECT_ID>::max)() == candidate ? 1u : candidate + 1u;
			return true;
		}
		candidate = (std::numeric_limits<COMBAT_OBJECT_ID>::max)() == candidate ? 1u : candidate + 1u;
	}
	return false;
}

// GameRoom의 물총 시뮬레이션이 표현만 생성한다. Hits는 비워 두고 Room이 위치와 종료를 공급하므로 이 객체가 물총 피해를 판정하지 않는다.
bool LostArk::Server::CCombatObjectRuntime::Stage_WaterGunPresentation(
    SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
    const SERVER_PLAYER& source,
    const std::uint32_t skillId,
    const CGameplayCatalog& catalog,
    const std::uint32_t spawnTick,
    std::string& status,
    const std::uint32_t projectileIndex) const
{
	const char* archetype = LostArk::Shared::MaharakaWaterGunProjectileArchetype(skillId);
	const auto* skill = LostArk::Shared::Find_MaharakaWaterGunSkill(skillId);
	if (!archetype || !skill || projectileIndex >= skill->iProjectileCount || !source.iNetEntityId ||
	    !spawnTick || !catalog.Get_ActiveRevision().Is_Valid() || !std::isfinite(source.fPositionX) ||
	    !std::isfinite(source.fPositionY) || !std::isfinite(source.fPositionZ) ||
	    !std::isfinite(source.fYawDegrees))
	{
		status = "Waterpang projectile presentation source is invalid";
		return false;
	}
	SERVER_COMBAT_OBJECT object;
	if (!Allocate_Id(transaction, object.iCombatObjectId))
	{
		status = "Waterpang projectile presentation capacity is exhausted";
		return false;
	}
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
	const auto launch = LostArk::Shared::Sample_MaharakaWaterGunLaunch(*skill,
	                                                                   projectileIndex,
	                                                                   source.fPositionX,
	                                                                   source.fPositionY,
	                                                                   source.fPositionZ,
	                                                                   source.fYawDegrees);
	pose.fPositionX = launch.fX;
	pose.fPositionY = launch.fY;
	pose.fPositionZ = launch.fZ;
	pose.fYawDegrees = launch.fYawDegrees;
	pose.fDirectionX = launch.fDirX;
	pose.fDirectionZ = launch.fDirZ;
	object.LiveState.PreviousPose = pose;
	transaction.Spawned.push_back(To_SpawnedMessage(object));
	transaction.Objects.push_back(std::move(object));
	return true;
}

// Room이 확정한 물총 접촉/만료를 소비한다. source와 spawn 회차를 재확인해 오래된 종료 요청이 다른 발사체를 지우지 않게 한다.
bool LostArk::Server::CCombatObjectRuntime::Finish_WaterGunPresentation(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId,
    const LostArk::Shared::NET_ENTITY_ID sourceId,
    const std::uint32_t spawnTick,
    const std::uint32_t serverTick,
    const bool impact)
{
	using namespace LostArk::Shared;
	for (std::size_t index = 0u; index < m_Objects.size(); ++index)
	{
		const auto& object = m_Objects[index];
		if (object.iCombatObjectId != objectId)
			continue;
		if (!serverTick || object.iSourceNetEntityId != sourceId || object.iSpawnTick != spawnTick ||
		    !object.bRoomOwnedTracking || !object.Hits.empty() ||
		    !Is_MaharakaWaterGunProjectile(object.strCombatObjectArchetypeId))
			return false;
		if (impact)
		{
			S2C_COMBAT_OBJECT_PRESENTATION_EVENT event;
			event.iEventSequence = m_iNextPresentationEventSequence++;
			if (!m_iNextPresentationEventSequence)
				m_iNextPresentationEventSequence = 1u;
			event.iServerTick = serverTick;
			event.iCombatObjectId = objectId;
			event.iSourceNetEntityId = sourceId;
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

// 아이템 사용 경로에서 지면 투영을 마친 목표를 받는다. 파괴 폭탄/회오리 수류탄의 이동 값과 한 번의 원형 폭발을 후보에 묶는다.
bool LostArk::Server::CCombatObjectRuntime::Stage_BattleItemProjectile(
    SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
    const SERVER_PLAYER& source,
    const SERVER_BATTLE_ITEM_USE& use,
    const float targetX,
    const float targetY,
    const float targetZ,
    const CGameplayCatalog& catalog,
    const std::uint32_t serverTick,
    std::string& status) const
{
	const auto* profile = catalog.Find_Player(source.eCharacterClass);
	const float dx = targetX - source.fPositionX, dz = targetZ - source.fPositionZ;
	const float distance = std::hypot(dx, dz);
	if (!profile || !serverTick || !std::isfinite(distance) || !std::isfinite(targetY) ||
	    !source.iNetEntityId || !use.iProjectileSpeedCmPerSecond || !use.iRadiusCm ||
	    (use.eKind != BATTLE_ITEM_KIND::DESTRUCTION && use.eKind != BATTLE_ITEM_KIND::WHIRLWIND))
	{
		status = "Battle projectile definition is invalid";
		return false;
	}
	SERVER_COMBAT_OBJECT object;
	if (!Allocate_Id(transaction, object.iCombatObjectId))
	{
		status = "Battle projectile capacity is exhausted";
		return false;
	}
	object.eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
	object.iSourcePlayerId = source.iPlayerId;
	object.iSourceNetEntityId = source.iNetEntityId;
	object.iSourceSkillId = use.iSkillId;
	object.iSpawnTick = serverTick;
	object.strCombatObjectArchetypeId = use.eKind == BATTLE_ITEM_KIND::DESTRUCTION
	                                        ? "battle.item.destruction_bomb"
	                                        : "battle.item.whirlwind_grenade";
	object.strClientVisualId = object.strCombatObjectArchetypeId;
	object.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
	object.bReplicated = object.bDetonateOnContact = true;
	object.LiveState.strOwnerPatternId = "battle.item";
	object.LiveState.strOwnerStageActionId = "battle.item.throw";
	// 아이템 catalog의 cm 계열 수치를 m로 변환한다. 수명은 수평 비행 시간에 100ms 여유를 더하며 목표 지면 높이는 별도 입력이다.
	object.fSpeedMps = static_cast<float>(use.iProjectileSpeedCmPerSecond) * .01f;
	object.fRemainingDistanceM = object.fProjectileDistanceM = distance;
	object.fProjectileStartY = source.fPositionY + static_cast<float>(use.iProjectileLaunchHeightCm) * .01f;
	object.fProjectileTargetY = targetY;
	object.fProjectileArcHeightM = static_cast<float>(use.iProjectileArcHeightCm) * .01f;
	object.fRemainingMilliseconds = (distance / object.fSpeedMps) * 1000.f + 100.f;
	auto& pose = object.LiveState.CurrentPose;
	pose.fPositionX = source.fPositionX;
	pose.fPositionY = object.fProjectileStartY;
	pose.fPositionZ = source.fPositionZ;
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
	hit.iPartDamage = use.iPartDamage;
	hit.iStaggerDamage = use.iStaggerDamage;
	hit.iStaggerMaximumDivisor = use.iStaggerMaximumDivisor;
	hit.RepeatRawDamage.push_back(
	    CGameplayCatalog::Resolve_Damage(profile->iAttackPower, use.iDamageRatePercent));
	object.Hits.push_back(std::move(hit));
	transaction.Spawned.push_back(To_SpawnedMessage(object));
	transaction.Objects.push_back(std::move(object));
	return true;
}

// 공용 projectile 어댑터의 현재 직접 호출은 Server 계약 테스트다. 실제 일반 스킬 갱신은 별도 player.Projectiles도 사용한다.
// 여기서는 live가 아니라 transaction을 변경한다. 타격 검증 실패 전 할당한 다음 ID는 되돌리지 않는다.
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
	    LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId || definition.Hits.empty() ||
	    0u == definition.iLifeMs || projectileIndex > (std::numeric_limits<std::uint8_t>::max)())
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
	// 플레이어 객체의 발생 identity에 보스 회차 정보를 상속하지 않는다.
	object.LiveState.iOwnerPatternSequence = 0u;
	object.LiveState.strOwnerPatternId.clear();
	object.LiveState.strOwnerStageActionId.clear();
	object.LiveState.CurrentPose.fDirectionX = source.fSkillAimDirectionX;
	object.LiveState.CurrentPose.fDirectionZ = source.fSkillAimDirectionZ;
	object.LiveState.CurrentPose.fYawDegrees =
	    std::atan2(object.LiveState.CurrentPose.fDirectionX, object.LiveState.CurrentPose.fDirectionZ) *
	    RADIANS_TO_DEGREES;
	object.fSpeedMps = definition.fSpeed;
	object.fRemainingMilliseconds = static_cast<float>(definition.iLifeMs);
	object.LiveState.CurrentPose.fPositionX = source.fPositionX;
	object.LiveState.CurrentPose.fPositionY = source.fPositionY;
	object.LiveState.CurrentPose.fPositionZ = source.fPositionZ;
	// 지정 지점 고정 영역은 조준 거리로 위치를 옮기고 나머지는 저작 로컬 offset을 사용한다.
	if (PLAYER_PROJECTILE_ORIGIN::AIM == definition.eOrigin &&
	    PLAYER_PROJECTILE_KIND::FIXAREA == definition.eKind)
	{
		const float distance = definition.fMaxDistance > 0.f
		                           ? (std::min)(source.fSkillAimDistance, definition.fMaxDistance)
		                           : source.fSkillAimDistance;
		object.LiveState.CurrentPose.fPositionX += object.LiveState.CurrentPose.fDirectionX * distance;
		object.LiveState.CurrentPose.fPositionZ += object.LiveState.CurrentPose.fDirectionZ * distance;
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
		object.fRemainingDistanceM =
		    definition.fMaxDistance > 0.f
		        ? (std::min)((std::max)(source.fSkillAimDistance, definition.fMinDistance),
		                     definition.fMaxDistance)
		        : -1.f;
	}

	// 시전 전체 피해의 sub-hit 구간을 각 hit 반복 raw 목록으로 복사한다. PvP용 전체 시전 값도 별도로 보관한다.
	std::uint32_t resolvedSubHit = subHitBase;
	for (const PLAYER_PROJECTILE_HIT& authored : definition.Hits)
	{
		SERVER_COMBAT_OBJECT_HIT_RUNTIME hit{};
		hit.eTrigger = authored.isContact ? SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT
		                                  : SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
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
		for (std::uint32_t repeat = 0u; repeat < authored.Hit.iRepeatCount; ++repeat, ++resolvedSubHit)
		{
			hit.RepeatRawDamage.push_back(DamageOfSubHit(totalDamage, subHitTotal, resolvedSubHit));
		}
		if (!CServerCombatGeometry::Is_Valid(hit.Shape) || hit.RepeatRawDamage.empty())
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

// GameRoom이 고른 정의/원점/volley를 실행 객체들로 복사한다. 정의는 차용하고 완료한 각 객체와 spawn 메시지는 transaction이 소유한다.
// 실패한 후보 전체를 호출자가 폐기해야 하며, 앞선 ordinal이나 ID 진행을 이 함수가 rollback하지 않는다.
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
	    (definition.Hits.empty() && definition.AttackTemplates.empty() &&
	     definition.PresentationPulses.empty()) ||
	    0u == definition.iLifeMs || !std::isfinite(definition.fCoverRadiusM) ||
	    definition.fCoverRadiusM < 0.f || definition.fCoverRadiusM > 100.f ||
	    (definition.fCoverRadiusM > 0.f &&
	     (BOSS_COMBAT_OBJECT_KIND::FIXED_AREA != definition.eKind || definition.Hits.empty() ||
	      std::any_of(definition.Hits.begin(),
	                  definition.Hits.end(),
	                  [](const BOSS_COMBAT_OBJECT_HIT& hit)
	                  {
		                  return BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED != hit.eTrigger;
	                  }))) ||
	    definition.strCombatObjectArchetypeId.empty() || definition.strClientVisualId.empty() ||
	    definition.strOwnerPatternId.empty() || definition.strOwnerStageActionId.empty() ||
	    0u == boss.iPatternSequence || definition.strEncounterId != boss.strEncounterId)
	{
		status = "Boss combat object spawn is invalid";
		return false;
	}
	if (!LostArk::Shared::Validate_AttackHitTemplates(definition.AttackTemplates, definition.iLifeMs))
	{
		status = "Boss attack templates exceed their bounded shape, damage or lifetime contract";
		return false;
	}
	// volley의 개수·배치·시간·원점 정책 조합을 검사한다. 저작 catalog의 모든 참조/수치 검증을 이 경계 하나가 대신하지 않는다.
	if (nullptr != volley)
	{
		const bool radial = BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL == volley->eLayout;
		const bool single = BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE == volley->eLayout;
		const bool perAlivePlayer = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER == volley->ePolicy;
		const bool bossRelative = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE == volley->ePolicy;
		const bool arenaCenter = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER == volley->ePolicy;
		if ((!perAlivePlayer && !bossRelative && !arenaCenter) ||
		    count != volley->iCountPerResolvedTarget || count < 1u || count > 8u ||
		    static_cast<std::uint64_t>(volley->iMaximumTotalObjects) <
		        static_cast<std::uint64_t>(count) + static_cast<std::uint64_t>(volley->iArenaRandomCount) ||
		    volley->iMaximumTotalObjects > 64u || volley->iSpawnCount < 1u || volley->iSpawnCount > 8u ||
		    (1u == volley->iSpawnCount ? 0u != volley->iSpawnIntervalMs : 0u == volley->iSpawnIntervalMs) ||
		    volley->iArenaRandomCount > 32u || !std::isfinite(volley->fRadiusM) || volley->fRadiusM < 0.f ||
		    !std::isfinite(volley->fStartAngleDegrees) || !std::isfinite(volley->fAngleStepDegrees) ||
		    !std::isfinite(volley->fArenaRandomRadiusM) || !std::isfinite(volley->fArenaHeightToleranceM) ||
		    (0u == volley->iArenaRandomCount
		         ? (BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE != volley->eArenaAnchorPolicy ||
		            volley->fArenaRandomRadiusM != 0.f || volley->fArenaHeightToleranceM != 0.f)
		         : (BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION !=
		                volley->eArenaAnchorPolicy ||
		            volley->fArenaRandomRadiusM <= 0.f || volley->fArenaHeightToleranceM <= 0.f)) ||
		    (count > 1u && (!radial || volley->fRadiusM <= 0.f || 0.f == volley->fAngleStepDegrees ||
		                    volley->bAllowOverlap)) ||
		    (1u == count && (!single || 0.f != volley->fRadiusM || 0.f != volley->fStartAngleDegrees ||
		                     0.f != volley->fAngleStepDegrees || volley->bAllowOverlap)) ||
		    (perAlivePlayer && BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER !=
		                           definition.eOriginPolicy) ||
		    (bossRelative &&
		     (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION != definition.eOriginPolicy ||
		      nullptr != lockedTarget || 1u != volley->iSpawnCount || 0u != volley->iArenaRandomCount)))
		{
			status = "Boss combat object volley layout is invalid";
			return false;
		}
	}
	// 보스 생성 전에는 복제될 live/staged 개수도 protocol snapshot 상한에 맞는지 검사한다.
	std::size_t replicatedCount =
	    static_cast<std::size_t>(std::count_if(transaction.Objects.begin(),
	                                           transaction.Objects.end(),
	                                           [](const SERVER_COMBAT_OBJECT& object)
	                                           {
		                                           return object.bReplicated;
	                                           }));
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
		object.strSourceArchetypeId =
		    boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ? "BOSS_VALTAN_GHOST"
		                                                                              : boss.strArchetypeId;
		object.iSpawnTick = serverTick;
		object.strCombatObjectArchetypeId = definition.strCombatObjectArchetypeId;
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
		object.LiveState.strOwnerStageActionId = definition.strOwnerStageActionId;
		object.LiveState.CurrentPose.fDirectionX = directionX;
		object.LiveState.CurrentPose.fDirectionZ = directionZ;
		object.LiveState.CurrentPose.fYawDegrees = boss.fYawDegrees;
		object.fSpeedMps = definition.fSpeedMps;
		object.iMovementStartDelayMs = definition.iMovementStartDelayMs;
		object.bExpireOnDistanceEnd = definition.bExpireOnDistanceEnd;
		object.fCoverRadiusM = definition.fCoverRadiusM;
		object.OwnerHitChain = definition.OwnerHitChain;
		object.fRemainingDistanceM =
		    BOSS_COMBAT_OBJECT_KIND::MISSILE == definition.eKind ? definition.fMaximumDistanceM : 0.f;
		object.fRemainingMilliseconds = static_cast<float>(definition.iLifeMs);
		// 두 locked-origin 정책은 Room이 넘긴 위치에서 시작한다. UNTIL_FIRST_PULSE 또는 명시적인 target flag만 위치 추적을 켠다.
		if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE == definition.eOriginPolicy ||
		    BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER == definition.eOriginPolicy)
		{
			const bool shouldTrackUntilFirstPulse =
			    BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE ==
			        definition.eOriginPolicy ||
			    (nullptr != lockedTarget && lockedTarget->bTrackUntilFirstPulse);
			if (nullptr == lockedTarget || !std::isfinite(lockedTarget->fPositionX) ||
			    !std::isfinite(lockedTarget->fPositionY) || !std::isfinite(lockedTarget->fPositionZ) ||
			    (shouldTrackUntilFirstPulse &&
			     LostArk::Shared::INVALID_NET_ENTITY_ID == lockedTarget->iNetEntityId))
			{
				status = "Boss combat object resolved origin is unavailable";
				return false;
			}
			object.iLockedTargetNetEntityId = lockedTarget->iNetEntityId;
			object.bTrackLockedTargetUntilFirstPulse = shouldTrackUntilFirstPulse;
			// 대상 중심의 radial offset은 월드 절대 각도로 계산한다. 이후 추적 때 같은 offset을 더해 여러 발사 위치가 중심에 겹치지 않게 한다.
			if (nullptr != volley && BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL == volley->eLayout)
			{
				const float radialDegrees =
				    volley->fStartAngleDegrees + volley->fAngleStepDegrees * static_cast<float>(ordinal);
				const float radialRadians = radialDegrees * DEGREES_TO_RADIANS;
				object.fLockedTargetOffsetX = std::sin(radialRadians) * volley->fRadiusM;
				object.fLockedTargetOffsetZ = std::cos(radialRadians) * volley->fRadiusM;
			}
			object.LiveState.CurrentPose.fPositionX =
			    lockedTarget->fPositionX + object.fLockedTargetOffsetX;
			object.LiveState.CurrentPose.fPositionY = lockedTarget->fPositionY;
			object.LiveState.CurrentPose.fPositionZ =
			    lockedTarget->fPositionZ + object.fLockedTargetOffsetZ;
		}
		else
		{
			// ARENA_CENTER volley는 boss spawn 위치와 월드 절대 radial 각도를 사용한다. 다른 경우는 현재 boss 위치/yaw를 사용한다.
			// 아래 definition의 forward/right offset은 어느 경우에도 현재 boss 방향으로 변환한다.
			const bool arenaCenterVolley =
			    nullptr != volley && BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER == volley->ePolicy;
			const float originX = arenaCenterVolley ? boss.fSpawnPositionX : boss.fPositionX;
			const float originY = arenaCenterVolley ? boss.fSpawnPositionY : boss.fPositionY;
			const float originZ = arenaCenterVolley ? boss.fSpawnPositionZ : boss.fPositionZ;
			const float yawBasisDegrees = arenaCenterVolley ? 0.f : boss.fYawDegrees;
			const float rightX = directionZ;
			const float rightZ = -directionX;
			object.LiveState.CurrentPose.fPositionX =
			    originX + directionX * definition.fOffsetForwardM + rightX * definition.fOffsetRightM;
			object.LiveState.CurrentPose.fPositionY = originY;
			object.LiveState.CurrentPose.fPositionZ =
			    originZ + directionZ * definition.fOffsetForwardM + rightZ * definition.fOffsetRightM;
			if (nullptr != volley &&
			    (BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE == volley->ePolicy || arenaCenterVolley))
			{
				const float relativeDegrees =
				    volley->fStartAngleDegrees + volley->fAngleStepDegrees * static_cast<float>(ordinal);
				const float worldDegrees = yawBasisDegrees + relativeDegrees;
				const float worldRadians = worldDegrees * DEGREES_TO_RADIANS;
				const float radialDirectionX = std::sin(worldRadians);
				const float radialDirectionZ = std::cos(worldRadians);
				object.LiveState.CurrentPose.fPositionX += radialDirectionX * volley->fRadiusM;
				object.LiveState.CurrentPose.fPositionZ += radialDirectionZ * volley->fRadiusM;
				const bool nextRadialSlot =
				    BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT == definition.eDirectionPolicy;
				const bool radialInward =
				    BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD == definition.eDirectionPolicy;
				if (nextRadialSlot)
				{
					// 현재 radial slot에서 다음 slot까지의 직선 chord로 이동 방향과 남은 거리를 만든다. 마지막 ordinal은 slot 0으로 순환한다.
					const std::uint32_t nextOrdinal = (ordinal + 1u) % count;
					const float nextWorldDegrees =
					    yawBasisDegrees + volley->fStartAngleDegrees +
					    volley->fAngleStepDegrees * static_cast<float>(nextOrdinal);
					const float nextWorldRadians = nextWorldDegrees * DEGREES_TO_RADIANS;
					const float routeX = (std::sin(nextWorldRadians) - radialDirectionX) * volley->fRadiusM;
					const float routeZ = (std::cos(nextWorldRadians) - radialDirectionZ) * volley->fRadiusM;
					const float routeLengthSquared = routeX * routeX + routeZ * routeZ;
					if (!std::isfinite(routeLengthSquared) || routeLengthSquared <= 0.000001f)
					{
						status = "Boss combat object next radial slot is invalid";
						return false;
					}
					const float inverseRouteLength = 1.f / std::sqrt(routeLengthSquared);
					object.LiveState.CurrentPose.fDirectionX = routeX * inverseRouteLength;
					object.LiveState.CurrentPose.fDirectionZ = routeZ * inverseRouteLength;
					object.LiveState.CurrentPose.fYawDegrees =
					    std::atan2(object.LiveState.CurrentPose.fDirectionX,
					               object.LiveState.CurrentPose.fDirectionZ) *
					    RADIANS_TO_DEGREES;
				}
				else
				{
					// NEXT_RADIAL_SLOT 외 정책은 기존 반경 방향을 사용한다. INWARD만 방향을 반대로 뒤집는다.
					object.LiveState.CurrentPose.fDirectionX =
					    radialInward ? -radialDirectionX : radialDirectionX;
					object.LiveState.CurrentPose.fDirectionZ =
					    radialInward ? -radialDirectionZ : radialDirectionZ;
					object.LiveState.CurrentPose.fYawDegrees =
					    radialInward ? worldDegrees + 180.f : worldDegrees;
				}
			}
		}
		// 기존 보스 hit를 도형/반복/피해 값으로 복사한다. MISSILE만 기본 CONTACT 샘플링을 SWEPT로 설정한다.
		for (const BOSS_COMBAT_OBJECT_HIT& authored : definition.Hits)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME hit{};
			hit.strHitId = authored.strHitId;
			hit.strDamageProfileId = authored.strDamageProfileId;
			hit.eTrigger = BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT == authored.eTrigger
			                   ? SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT
			                   : SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
			hit.eContactSampling = BOSS_COMBAT_OBJECT_KIND::MISSILE == definition.eKind
			                           ? SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT
			                           : SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::POSE;
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
			for (std::uint32_t repeat = 0u; repeat < authored.iRepeatCount; ++repeat)
			{
				hit.RepeatRawDamage.push_back(rawDamage);
			}
			if (!CServerCombatGeometry::Is_Valid(hit.Shape) || hit.RepeatRawDamage.empty() ||
			    0u == rawDamage)
			{
				status = "Boss combat object hit is invalid";
				return false;
			}
			object.Hits.push_back(std::move(hit));
		}
		// 공용 attack template을 같은 hit 실행 값으로 옮긴다. template의 sampling은 SWEPT지만 실제 연속 검사는 원형에만 적용된다.
		for (const auto& authored : definition.AttackTemplates)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME hit;
			hit.strHitId = authored.strHitId;
			hit.strDamageProfileId = authored.strDamageProfileId;
			hit.eTrigger = authored.strTrigger == "CONTACT" ? SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT
			                                                : SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
			hit.eContactSampling = SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::SWEPT;
			hit.iAtMs = authored.iAtMs;
			hit.iEndMs = authored.iEndMs;
			hit.iRepeatIntervalMs = authored.iRepeatIntervalMs;
			hit.fOffsetForwardM = float(authored.fOffsetForwardM);
			hit.fOffsetRightM = float(authored.fOffsetRightM);
			hit.fYawOffsetDegrees = float(authored.fYawOffsetDegrees);
			hit.Shape.eKind = authored.strShape == "CIRCLE" ? SERVER_COMBAT_SHAPE_KIND::CIRCLE
			                  : authored.strShape == "RING" ? SERVER_COMBAT_SHAPE_KIND::RING
			                  : authored.strShape == "BOX"  ? SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX
			                                                : SERVER_COMBAT_SHAPE_KIND::CONE;
			hit.Shape.fOuterRadius = float(authored.fRadiusM);
			hit.Shape.fInnerRadius = float(authored.fInnerRadiusM);
			hit.Shape.fLength = float(authored.fLengthM);
			hit.Shape.fHalfWidth = float(authored.fHalfWidthM);
			hit.Shape.fAngleDegrees = float(authored.fAngleDegrees);
			hit.bInstantDeath =
			    authored.strDamageKind == "INSTANT_DEATH" ||
			    (authored.strDamageKind == "MAX_HP_PERCENT" && authored.iDamagePercent >= 100u);
			hit.bIgnoreDefense = authored.strDamageKind != "PROFILE";
			hit.iDamagePercent = authored.iDamagePercent;
			hit.strNumericBalanceId = authored.strNumericBalanceId;
			hit.fRiseHeightM = static_cast<float>(authored.fRiseHeightM);
			hit.fPushRangeM = static_cast<float>(authored.fPushRangeM);
			hit.bForcePush = authored.ForcePush.value_or(authored.fRiseHeightM > 0.0);
			hit.bPushFromBoss = authored.strPushDirection == "AWAY_FROM_BOSS";
			hit.iPushMs = authored.iPushMs;
			const auto rate = authored.strDamageKind == "PROFILE"
			                      ? catalog.Find_DamageRatePercent(authored.strDamageProfileId)
			                      : 1u;
			if (!rate || !CServerCombatGeometry::Is_Valid(hit.Shape))
			{
				status = "Boss attack damage profile or primitive is unresolved";
				return false;
			}
			const auto raw = authored.strDamageKind == "PROFILE"
			                     ? CGameplayCatalog::Resolve_Damage(boss.iAttackPower, rate)
			                     : 1u;
			if (!raw)
			{
				status = "Boss attack resolves to zero damage";
				return false;
			}
			hit.RepeatRawDamage.assign(authored.iRepeatCount, raw);
			object.Hits.push_back(std::move(hit));
		}
		// 타격 없는 표현 예약도 같은 오브젝트 수명 안에 저장한다. Update의 lifetime 만료 tick은 아직 남은 표현 예약까지 처리한다.
		for (const BOSS_COMBAT_OBJECT_PRESENTATION_PULSE& authored : definition.PresentationPulses)
		{
			if (authored.strPresentationEventId.empty() || authored.iAtMs > definition.iLifeMs)
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
		// 마지막 후보 삽입의 bad_alloc만 false로 바꾼다. Spawned 삽입 뒤 Objects 삽입이 실패하면 이 transaction에 부분 결과가 남을 수 있다.
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

// revision/전체 개수/복제 후보 수를 확인한 뒤 기존 live와 spawn 큐의 지역 복사본에 후보를 합친다.
// 명시적 false는 live/큐를 보존한다. 임의 transaction의 메시지 내용/ID를 모두 재검증하거나 입력 후보의 이동 상태를 복원하지는 않는다.
bool LostArk::Server::CCombatObjectRuntime::Commit(SERVER_COMBAT_OBJECT_TRANSACTION&& transaction)
{
	if (transaction.iExpectedRevision != m_iRevision ||
	    m_Objects.size() + transaction.Objects.size() > MAX_SERVER_COMBAT_OBJECTS ||
	    transaction.Spawned.size() !=
	        static_cast<std::size_t>(std::count_if(transaction.Objects.begin(),
	                                               transaction.Objects.end(),
	                                               [](const SERVER_COMBAT_OBJECT& object)
	                                               {
		                                               return object.bReplicated;
	                                               })))
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
		stagedSpawned.reserve(m_PendingSpawned.size() + transaction.Spawned.size());
		stagedObjects.insert(stagedObjects.end(),
		                     std::make_move_iterator(transaction.Objects.begin()),
		                     std::make_move_iterator(transaction.Objects.end()));
		stagedSpawned.insert(stagedSpawned.end(),
		                     std::make_move_iterator(transaction.Spawned.begin()),
		                     std::make_move_iterator(transaction.Spawned.end()));
	}
	catch (const std::bad_alloc&)
	{
		return false;
	}
	// 두 지역 vector 준비가 끝난 뒤 swap으로 채택한다. 다음 ID와 revision도 이 시점에만 live에 반영한다.
	m_Objects.swap(stagedObjects);
	m_PendingSpawned.swap(stagedSpawned);
	m_iNextCombatObjectId = transaction.iNextCombatObjectId;
	++m_iRevision;
	if (0u == m_iRevision)
		m_iRevision = 1u;
	return true;
}

// GameRoom 고정 tick의 정상 진입점이다. damage 출력은 비우지 않고 이번 처리 결과를 뒤에 추가한다.
void LostArk::Server::CCombatObjectRuntime::Update(
    std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
    std::vector<SERVER_WORLD_ENTITY>& worldEntities,
    const CGameplayCatalog& catalog,
    const float fixedDeltaSeconds,
    const std::uint32_t serverTick,
    std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	Update_Objects(players, worldEntities, catalog, fixedDeltaSeconds, serverTick, outDamageEvents, false);
}

// ValtanBrain의 실제 owner hit를 받아 같은 source/발생 회차/action을 기다리는 연쇄를 찾는다.
// 도형별 지연 계산 성공 뒤에만 무장하며 이번 tick에 무장한 객체를 delta 0으로 즉시 판정한다.
void LostArk::Server::CCombatObjectRuntime::Apply_OwnerHits(
    const std::vector<SERVER_BOSS_PATTERN_HIT>& hits,
    std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
    std::vector<SERVER_WORLD_ENTITY>& worldEntities,
    const CGameplayCatalog& catalog,
    const std::uint32_t serverTick,
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
			// seed 이후에서 같은 source/회차/archetype/spawnTick의 미무장 항목을 묶는다. 계산한 index는 이번 vector가 유지되는 동안만 사용한다.
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
					                   object.LiveState.CurrentPose.fPositionZ,
					                   object.fCoverRadiusM});
				}
			}
			std::vector<std::uint32_t> delays;
			if (!LostArk::Shared::CombatObjectHitChain::Resolve_ConeDelays(hit.Cone,
			                                                               circles,
			                                                               seed.OwnerHitChain.iDelayMs,
			                                                               delays))
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

// Room의 live 목록을 제자리 갱신한다. 삭제하면 같은 index로 다음 원소를 검사하고 남기면 index를 한 칸 진행한다.
// players/worldEntities는 이 루프 동안 차용하며 피해 적용 결과는 직접 남긴다. 예외/실패에 대한 tick 전체 rollback은 없다.
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
		// source가 사라졌거나 죽었으면 즉시 제거한다. player의 FALLING도 포함하지만 source 시간 정지 자체로 이 오브젝트 시계를 멈추지는 않는다.
		const bool sourceInvalid =
		    (nullptr != sourcePlayer &&
		     (LostArk::Shared::PLAYER_ACTION_STATE::DEAD == sourcePlayer->eAction ||
		      LostArk::Shared::PLAYER_ACTION_STATE::FALLING == sourcePlayer->eAction ||
		      0u == sourcePlayer->iCurrentHp)) ||
		    (nullptr != sourceEntity &&
		     (SERVER_ENTITY_ACTION::DEAD == sourceEntity->eAction || 0u == sourceEntity->iCurrentHp)) ||
		    (nullptr == sourcePlayer && nullptr == sourceEntity);
		if (sourceInvalid)
		{
			Despawn_At(objectIndex);
			continue;
		}
		// 생성 tick과 현재 tick이 같으면 이동/타격/수명 진행을 생략한다. 다음 고정 tick부터 일반 갱신을 수행한다.
		if (object.iSpawnTick == serverTick)
		{
			++objectIndex;
			continue;
		}

		// Room이 이번 tick에 외부 위치를 공급했다면 첫 이전 pose를 유지한다. 그렇지 않으면 현재 pose를 이번 이동의 시작점으로 복사한다.
		if (!object.bHasExternalPoseStep)
			object.LiveState.PreviousPose = object.LiveState.CurrentPose;
		object.bHasExternalPoseStep = false;
		// 첫 TIMED 판정을 소비할 때까지 살아 있는 인간 대상 위치를 갱신한다. 대상이 없거나 추적 자격이 없으면 마지막 위치를 유지한다.
		if (object.bTrackLockedTargetUntilFirstPulse)
		{
			if (SERVER_PLAYER* target = FindPlayerByEntityId(players, object.iLockedTargetNetEntityId))
			{
				if (target->Is_Human() && IsDamageable(*target))
				{
					object.LiveState.CurrentPose.fPositionX =
					    target->fPositionX + object.fLockedTargetOffsetX;
					object.LiveState.CurrentPose.fPositionY = target->fPositionY;
					object.LiveState.CurrentPose.fPositionZ =
					    target->fPositionZ + object.fLockedTargetOffsetZ;
				}
			}
		}
		const float previousElapsedMilliseconds = object.fElapsedMilliseconds;
		object.fPreviousElapsedMilliseconds = previousElapsedMilliseconds;
		object.fElapsedMilliseconds += deltaMilliseconds;
		if (!object.bPersistentLifetime)
			object.fRemainingMilliseconds -= deltaMilliseconds;
		float homingDistance = -1.f;
		// homing은 매 tick 목표 방향/Y를 갱신한다. 목표 부재/가이드/피격 자격 상실 또는 비정상 거리는 오브젝트 제거로 끝낸다.
		if (object.bHoming)
		{
			auto* target = FindPlayerByEntityId(players, object.iLockedTargetNetEntityId);
			if (!target || target->Is_Guide() || !IsDamageable(*target))
			{
				Despawn_At(objectIndex);
				continue;
			}
			const float dx = target->fPositionX - object.LiveState.CurrentPose.fPositionX;
			const float dz = target->fPositionZ - object.LiveState.CurrentPose.fPositionZ;
			homingDistance = std::hypot(dx, dz);
			if (!std::isfinite(homingDistance))
			{
				Despawn_At(objectIndex);
				continue;
			}
			if (homingDistance > .000001f)
			{
				object.LiveState.CurrentPose.fDirectionX = dx / homingDistance;
				object.LiveState.CurrentPose.fDirectionZ = dz / homingDistance;
				object.LiveState.CurrentPose.fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
			}
			object.LiveState.CurrentPose.fPositionY = target->fPositionY;
		}
		float movementStep = 0.f;
		// 이번 delta 중 이동 지연이 지난 부분만 m/s와 곱한다. 유한 거리와 homing 목표까지 남은 거리가 step을 제한한다.
		if (object.fSpeedMps > 0.f && (object.fRemainingDistanceM > 0.f || object.bPersistentLifetime))
		{
			const float movementStartMilliseconds = static_cast<float>(object.iMovementStartDelayMs);
			const float activeMilliseconds =
			    (std::max)(0.f, object.fElapsedMilliseconds - movementStartMilliseconds) -
			    (std::max)(0.f, previousElapsedMilliseconds - movementStartMilliseconds);
			float step = object.fSpeedMps * activeMilliseconds / SECONDS_TO_MILLISECONDS;
			if (homingDistance >= 0.f)
				step = (std::min)(step, homingDistance);
			if (!object.bPersistentLifetime && object.fRemainingDistanceM >= 0.f)
			{
				step = (std::min)(step, object.fRemainingDistanceM);
				object.fRemainingDistanceM -= step;
			}
			movementStep = step;
			object.LiveState.CurrentPose.fPositionX += object.LiveState.CurrentPose.fDirectionX * step;
			object.LiveState.CurrentPose.fPositionZ += object.LiveState.CurrentPose.fDirectionZ * step;
		}
		// 수평 진행률로 시작/목표 높이를 보간하고 4*h*t*(1-t) 포물선 높이를 더한다. 타격 도형의 XZ 판정과 분리된 표현 Y다.
		if (object.bDetonateOnContact)
		{
			const float progress =
			    object.fProjectileDistanceM > .0001f
			        ? std::clamp(1.f - object.fRemainingDistanceM / object.fProjectileDistanceM, 0.f, 1.f)
			        : 1.f;
			object.LiveState.CurrentPose.fPositionY =
			    object.fProjectileStartY +
			    (object.fProjectileTargetY - object.fProjectileStartY) * progress +
			    4.f * object.fProjectileArcHeightM * progress * (1.f - progress);
		}
		// 수명 또는 거리 만료를 먼저 기록하지만 현재 tick의 표현/타격 처리를 마친 뒤 제거한다.
		const bool lifetimeExpired = !object.bPersistentLifetime && object.fRemainingMilliseconds <= 0.f;
		const bool expired = lifetimeExpired || (object.bExpireOnDistanceEnd && object.fSpeedMps > 0.f &&
		                                         object.fRemainingDistanceM <= 0.f);
		const bool contactMotionActive =
		    object.fSpeedMps <= 0.f ||
		    (object.fElapsedMilliseconds >= static_cast<float>(object.iMovementStartDelayMs) &&
		     (movementStep > 0.f || object.fRemainingDistanceM > 0.f));
		bool firedFirstTimedPulse = false;
		// 현재 pose와 생성 때 저장한 owner identity로 표현 event를 큐에 쌓는다. 피해 성공 여부와 별개이며 여기서 네트워크 송신하지 않는다.
		const auto QueuePresentationPulse =
		    [this, &object, serverTick](const std::string& pulseId, const std::uint32_t repeatIndex)
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
			event.eKind = LostArk::Shared::COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE;
			event.strCombatObjectArchetypeId = object.strCombatObjectArchetypeId;
			event.strOwnerPatternId = object.LiveState.strOwnerPatternId;
			event.strOwnerStageActionId = object.LiveState.strOwnerStageActionId;
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
			// 연쇄의 직접/지연 분류는 준비 표현의 시작을 예약한다.
			// 실제 hit는 아래에서 자체 atMs만큼의 준비 시간을 추가로 기다린다.
			object.bOwnerHitChainWarningApplied = true;
			QueuePresentationPulse(object.OwnerHitChain.strArmedPresentationEventId, 0u);
		}

		// 접촉 종료용 원은 이동 전후를 sweep한다. 투척물은 겹친 대상 중 이전 위치에서 중심까지 가장 가까운 대상을 고른다.
		// 이 선택은 정확한 연속 충돌 시각 순서가 아니며 최종 폭발 위치는 선택한 대상의 현재 XYZ로 옮긴다.
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
					if (!IsDamageable(target) || !ContactOverlaps(object, contact, BodyOf(target)))
						continue;
					const float distance =
					    std::hypot(target.fPositionX - object.LiveState.PreviousPose.fPositionX,
					               target.fPositionZ - object.LiveState.PreviousPose.fPositionZ);
					if (distance < nearest)
					{
						nearest = distance;
						first = &target;
					}
				}
				if (sourcePlayer)
					for (auto& [id, target] : players)
					{
						if (!CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, target) ||
						    !ContactOverlaps(object, contact, BodyOf(target)))
							continue;
						const float distance =
						    std::hypot(target.fPositionX - object.LiveState.PreviousPose.fPositionX,
						               target.fPositionZ - object.LiveState.PreviousPose.fPositionZ);
						if (distance < nearest)
						{
							nearest = distance;
							firstPlayer = &target;
							first = nullptr;
						}
					}
				if (firstPlayer)
				{
					object.LiveState.CurrentPose.fPositionX = firstPlayer->fPositionX;
					object.LiveState.CurrentPose.fPositionY = firstPlayer->fPositionY;
					object.LiveState.CurrentPose.fPositionZ = firstPlayer->fPositionZ;
					QueuePresentationPulse(object.strContactPresentationId, 0u);
					contacted = true;
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
			if (!object.bDetonateOnContact)
				for (auto& [id, player] : players)
				{
					if (sourcePlayer && !CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, player))
						continue;
					// 일반 homing은 지정 목표만 접촉하고, 문양 면역이 있는 카드 추적은 다른 플레이어도 가로막을 수 있다.
					// 문양별 피해 면역은 아래 타격 경로에서 따로 평가한다.
					const bool targetOnly =
					    object.bHoming &&
					    object.eDamageImmuneCardSymbol == LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
					if (!IsDamageable(player) || player.Has_TimeStop(serverTick) ||
					    (targetOnly && player.iNetEntityId != object.iLockedTargetNetEntityId) ||
					    !ContactOverlaps(object, contact, BodyOf(player)))
						continue;
					// 같은 문양으로 가로막았을 때 이 source/발생 회차가 잡은 플레이어만 해제한다.
					// 다른 소유자나 이후 회차의 포획은 이전 카드가 닿아도 유지한다.
					if (player.bPatternBound &&
					    object.eDamageImmuneCardSymbol != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
					    player.eMechanicCardSymbol == object.eDamageImmuneCardSymbol &&
					    player.iPatternBindOwnerNetEntityId == object.iSourceNetEntityId &&
					    player.iPatternBindSequence == object.LiveState.iOwnerPatternSequence)
						player.Clear_PatternBindStatus();
					// 한 tick의 sweep이 몸체를 지나칠 수 있으므로 접촉 표현은 대상의 현재 서버 XYZ에 둔다.
					// 오브젝트가 이미 지나간 이동 끝점에서 폭발 표현을 만들지 않는다.
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

		// 일반 tick은 atMs에 도달한 표현만 내보내고 lifetime 만료 tick은 남은 예약도 내보낸다. 거리 만료만으로 미래 예약을 앞당기지는 않는다.
		for (SERVER_COMBAT_OBJECT_PRESENTATION_PULSE_RUNTIME& pulse : object.PresentationPulses)
		{
			if (pulse.bApplied ||
			    (!lifetimeExpired && object.fElapsedMilliseconds < static_cast<float>(pulse.iAtMs)))
			{
				continue;
			}
			QueuePresentationPulse(pulse.strPresentationEventId, 0u);
			pulse.bApplied = true;
		}

		for (std::size_t hitIndex = 0u; hitIndex < object.Hits.size(); ++hitIndex)
		{
			SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit = object.Hits[hitIndex];
			if (object.bDetonateOnContact && !contactTermination)
				continue;
			const auto attackPose = AttackPose(object.LiveState.CurrentPose, hit);
			// 플레이어 대 플레이어 경로는 현재 catalog에서 skill을 다시 조회한다. 같은 경기 상대를 거리순으로 제한하고 공용 PvP 피해 함수에 넘긴다.
			// CONTACT는 대상별 mark, TIMED는 현재 공통 소비 index를 사용한다. 적용 결과를 검사하지 않고 admitted/소비 횟수를 진행한다.
			const auto hitPvpTargets = [&](const bool contact, const std::uint32_t rawDamage)
			{
				if (!sourcePlayer || !sourcePlayer->iColosseumMatchId)
					return;
				std::vector<std::pair<float, SERVER_PLAYER*>> targets;
				for (auto& [id, target] : players)
				{
					if (!CServerCombatHitRuntime::Is_EnemyPlayer(*sourcePlayer, target))
						continue;
					const bool overlaps = contact
					                          ? ContactOverlaps(object, hit, BodyOf(target))
					                          : CServerCombatGeometry::Overlaps_Pose(hit.Shape,
					                                                                 attackPose.fPositionX,
					                                                                 attackPose.fPositionZ,
					                                                                 attackPose.fDirectionX,
					                                                                 attackPose.fDirectionZ,
					                                                                 BodyOf(target));
					if (overlaps)
						targets.emplace_back(std::hypot(target.fPositionX - attackPose.fPositionX,
						                                target.fPositionZ - attackPose.fPositionZ),
						                     &target);
				}
				std::sort(targets.begin(),
				          targets.end(),
				          [](const auto& a, const auto& b)
				          {
					          return a.first < b.first;
				          });
				std::size_t admitted = 0u;
				for (const auto& [distance, target] : targets)
				{
					if (hit.Shape.iMaximumTargets && admitted >= hit.Shape.iMaximumTargets)
						break;
					if (contact && hit.Shape.iMaximumTargets)
					{
						const auto existing =
						    std::find_if(object.ContactMarks.begin(),
						                 object.ContactMarks.end(),
						                 [&](const auto& entry)
						                 {
							                 return entry.iHitIndex == hitIndex &&
							                        entry.iTargetNetEntityId == target->iNetEntityId;
						                 });
						const auto count = std::count_if(object.ContactMarks.begin(),
						                                 object.ContactMarks.end(),
						                                 [&](const auto& entry)
						                                 {
							                                 return entry.iHitIndex == hitIndex;
						                                 });
						if (existing == object.ContactMarks.end() && count >= hit.Shape.iMaximumTargets)
							continue;
					}
					auto* mark =
					    contact ? FindContactMark(object, target->iNetEntityId, hitIndex) : nullptr;
					if (mark && (mark->iAppliedCount >= hit.RepeatRawDamage.size() ||
					             object.fElapsedMilliseconds < mark->fNextMilliseconds))
						continue;
					const auto* skill = catalog.Find_Skill(object.iSourceSkillId);
					if (!skill)
						continue;
					const auto subHitIndex =
					    hit.iPlayerSubHitBase + (mark ? mark->iAppliedCount : hit.iAppliedTimedCount);
					PLAYER_SKILL_HIT shape{};
					shape.iResultKind = hit.iPlayerResultKind;
					shape.fPushRange = hit.fPushRangeM;
					shape.iPushMs = hit.iPushMs;
					SERVER_COLOSSEUM_COMBAT_CONTEXT context{LostArk::Shared::WORLD_ID::COLOSSEUM,
					                                        sourcePlayer->iColosseumMatchId,
					                                        sourcePlayer->bColosseumCombatActive,
					                                        {}};
					CPlayerSkillSystem::Apply_ColosseumObjectHit(*target,
					                                             *sourcePlayer,
					                                             *skill,
					                                             catalog,
					                                             shape,
					                                             hit.iPlayerWholeCastDamage,
					                                             hit.iPlayerSubHitTotal,
					                                             subHitIndex,
					                                             object.LiveState.CurrentPose.fPositionX,
					                                             object.LiveState.CurrentPose.fPositionZ,
					                                             serverTick,
					                                             context,
					                                             outDamageEvents);
					++admitted;
					if (mark)
					{
						++mark->iAppliedCount;
						mark->fNextMilliseconds = object.fElapsedMilliseconds + hit.iRepeatIntervalMs;
					}
				}
			};
			// CONTACT는 이번 이동 활성 구간의 접촉만 평가한다. 각 (hit, target)은 한 Update 호출에서 최대 한 반복을 소비하고 다음 시각을 현재 elapsed+interval로 잡는다.
			if (SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT == hit.eTrigger)
			{
				if (!contactMotionActive)
					continue;
				hitPvpTargets(true, 0u);
				if (nullptr != sourcePlayer)
				{
					for (SERVER_WORLD_ENTITY& target : worldEntities)
					{
						if (!IsDamageable(target) || !ContactOverlaps(object, hit, BodyOf(target)))
							continue;
						auto* mark = FindContactMark(object, target.iNetEntityId, hitIndex);
						if (mark->iAppliedCount >= hit.RepeatRawDamage.size() ||
						    object.fElapsedMilliseconds < mark->fNextMilliseconds)
							continue;
						SERVER_PLAYER_TO_WORLD_HIT incoming{};
						incoming.iSourcePlayerId = object.iSourcePlayerId;
						incoming.bGuideSource = sourcePlayer && sourcePlayer->Is_Guide();
						incoming.iSkillId = object.iSourceSkillId;
						incoming.iRawDamage = hit.RepeatRawDamage[mark->iAppliedCount];
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
						incoming.fSourceX = object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ = object.LiveState.CurrentPose.fPositionZ;
						incoming.fFallbackDirectionX = object.LiveState.CurrentPose.fDirectionX;
						incoming.fFallbackDirectionZ = object.LiveState.CurrentPose.fDirectionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_PlayerToWorld(target,
						                                                   incoming,
						                                                   outDamageEvents);
						// 피해 함수의 결과와 무관하게 이번 접촉 반복을 소비한다. 저항/무적 등으로 피해가 없었다고 같은 반복을 재시도하지 않는다.
						++mark->iAppliedCount;
						mark->fNextMilliseconds =
						    object.fElapsedMilliseconds + static_cast<float>(hit.iRepeatIntervalMs);
					}
				}
				else
				{
					for (auto& [playerId, target] : players)
					{
						(void)playerId;
						if (!target.iCurrentHp ||
						    (!hit.bInstantDeath &&
						     (!IsDamageable(target) || target.Has_TimeStop(serverTick))) ||
						    (object.eDamageImmuneCardSymbol !=
						         LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
						     target.eMechanicCardSymbol == object.eDamageImmuneCardSymbol) ||
						    !ContactOverlaps(object, hit, BodyOf(target)))
							continue;
						auto* mark = FindContactMark(object, target.iNetEntityId, hitIndex);
						if (mark->iAppliedCount >= hit.RepeatRawDamage.size() ||
						    object.fElapsedMilliseconds < mark->fNextMilliseconds)
							continue;
						SERVER_WORLD_TO_PLAYER_HIT incoming{};
						incoming.bInstantDeath = hit.bInstantDeath;
						incoming.bIgnoreDefense = hit.bIgnoreDefense;
						incoming.bIgnoreCounter = hit.bIgnoreDefense;
						incoming.iRawDamage =
						    AttackDamage(hit, target, hit.RepeatRawDamage[mark->iAppliedCount]);
						incoming.fSourceX = object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ = object.LiveState.CurrentPose.fPositionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.fPushHeightM = hit.fRiseHeightM;
						incoming.bPushBallistic = hit.fRiseHeightM > 0.f;
						incoming.bForcePush = hit.bForcePush;
						if (hit.bPushFromBoss && sourceEntity)
						{
							incoming.fSourceX = sourceEntity->fPositionX;
							incoming.fSourceZ = sourceEntity->fPositionZ;
						}
						incoming.bKnockdown = hit.bKnockdown;
						incoming.iDownMs = hit.iDownMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_WorldToPlayer(target,
						                                                   incoming,
						                                                   catalog,
						                                                   outDamageEvents);
						++mark->iAppliedCount;
						mark->fNextMilliseconds =
						    object.fElapsedMilliseconds + static_cast<float>(hit.iRepeatIntervalMs);
					}
				}
				continue;
			}

			// TIMED는 이번 호출에서 이미 지난 예약을 모두 소비한다. 반복마다 현재 attackPose와 현재 대상 위치를 사용하므로 과거 tick의 충돌 재현은 아니다.
			while (hit.iAppliedTimedCount < hit.RepeatRawDamage.size())
			{
				const float dueMilliseconds =
				    static_cast<float>(hit.iAtMs) +
				    static_cast<float>(hit.iRepeatIntervalMs) * static_cast<float>(hit.iAppliedTimedCount);
				// owner 연쇄는 무장 tick+연쇄 지연+hit atMs가 문턱이다. 이 분기는 반복 간격을 문턱에 더하지 않아 도달하면 남은 반복을 같은 호출에서 소비한다.
				if (!object.OwnerHitChain.strTriggerActionId.empty())
				{
					const auto elapsedTicks = serverTick - object.iOwnerHitChainArmedTick;
					if (!object.bOwnerHitChainArmed ||
					    static_cast<std::uint64_t>(elapsedTicks) * 1000u <
					        (static_cast<std::uint64_t>(object.iOwnerHitChainDelayMs) + hit.iAtMs) *
					            SERVER_TICK_HZ)
						break;
				}
				else if (object.fElapsedMilliseconds < dueMilliseconds)
					break;
				firedFirstTimedPulse = true;
				if (!object.bDetonateOnContact)
					QueuePresentationPulse(hit.strHitId, hit.iAppliedTimedCount);
				const std::uint32_t rawDamage = hit.RepeatRawDamage[hit.iAppliedTimedCount];
				hitPvpTargets(false, rawDamage);
				if (nullptr != sourcePlayer)
				{
					std::vector<std::pair<float, SERVER_WORLD_ENTITY*>> targets;
					for (SERVER_WORLD_ENTITY& target : worldEntities)
					{
						if (!IsDamageable(target) ||
						    !CServerCombatGeometry::Overlaps_Pose(hit.Shape,
						                                          attackPose.fPositionX,
						                                          attackPose.fPositionZ,
						                                          attackPose.fDirectionX,
						                                          attackPose.fDirectionZ,
						                                          BodyOf(target)))
							continue;
						const float dx = target.fPositionX - object.LiveState.CurrentPose.fPositionX;
						const float dz = target.fPositionZ - object.LiveState.CurrentPose.fPositionZ;
						targets.emplace_back(dx * dx + dz * dz, &target);
					}
					std::sort(targets.begin(),
					          targets.end(),
					          [](const auto& left, const auto& right)
					          {
						          return left.first < right.first;
					          });
					if (0u != hit.Shape.iMaximumTargets && targets.size() > hit.Shape.iMaximumTargets)
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
						incoming.fSourceX = object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ = object.LiveState.CurrentPose.fPositionZ;
						incoming.fFallbackDirectionX = object.LiveState.CurrentPose.fDirectionX;
						incoming.fFallbackDirectionZ = object.LiveState.CurrentPose.fDirectionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_PlayerToWorld(*target,
						                                                   incoming,
						                                                   outDamageEvents);
					}
				}
				else
				{
					for (auto& [playerId, target] : players)
					{
						(void)playerId;
						if (!target.iCurrentHp ||
						    (!hit.bInstantDeath &&
						     (!IsDamageable(target) || target.Has_TimeStop(serverTick))) ||
						    (object.eDamageImmuneCardSymbol !=
						         LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE &&
						     target.eMechanicCardSymbol == object.eDamageImmuneCardSymbol) ||
						    !CServerCombatGeometry::Overlaps_Pose(hit.Shape,
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
						incoming.fSourceX = object.LiveState.CurrentPose.fPositionX;
						incoming.fSourceZ = object.LiveState.CurrentPose.fPositionZ;
						incoming.fPushRangeM = hit.fPushRangeM;
						incoming.iPushMs = hit.iPushMs;
						incoming.fPushHeightM = hit.fRiseHeightM;
						incoming.bPushBallistic = hit.fRiseHeightM > 0.f;
						incoming.bForcePush = hit.bForcePush;
						if (hit.bPushFromBoss && sourceEntity)
						{
							incoming.fSourceX = sourceEntity->fPositionX;
							incoming.fSourceZ = sourceEntity->fPositionZ;
						}
						incoming.bKnockdown = hit.bKnockdown;
						incoming.iDownMs = hit.iDownMs;
						incoming.iServerTick = serverTick;
						(void)CServerCombatHitRuntime::Apply_WorldToPlayer(target,
						                                                   incoming,
						                                                   catalog,
						                                                   outDamageEvents);
					}
				}
				// 겹친 대상이 없거나 피해 적용이 거절돼도 예약 하나를 소비한다. 이 카운터는 명중 횟수가 아니다.
				++hit.iAppliedTimedCount;
			}
		}
		if (firedFirstTimedPulse)
			object.bTrackLockedTargetUntilFirstPulse = false;
		// 이번 호출에서 TIMED 예약을 하나라도 소비한 연쇄 객체는 live에서 제거한다. 다른 미래 hit까지 모두 소비했는지 확인하는 조건은 아니다.
		// 마지막 표현은 별도 hit pulse 메시지로 남겨 Client가 독립적인 종료 표현을 처리하게 한다.
		if (expired || contactTermination ||
		    (firedFirstTimedPulse && !object.OwnerHitChain.strTriggerActionId.empty()))
			Despawn_At(objectIndex);
		else
			++objectIndex;
	}
}

// 퇴장/기믹 취소 등이 지정한 source의 오브젝트를 모두 지운다. 복제 제거는 대기열에 남고 실제 송신은 Room이 처리한다.
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

// 물총/기믹 추적의 외부 월드 위치(m)를 현재 객체에 반영한다. identity/속도/추적 제약을 모두 통과한 뒤에만 바꾼다.
// 같은 tick에 여러 번 공급해도 첫 PreviousPose는 유지해 다음 Update의 sweep 구간을 보존한다.
bool LostArk::Server::CCombatObjectRuntime::Set_OwnedVisualPosition(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId,
    const LostArk::Shared::NET_ENTITY_ID sourceId,
    const std::uint32_t patternSequence,
    const float x,
    const float y,
    const float z)
{
	if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
		return false;
	for (auto& object : m_Objects)
	{
		if (object.iCombatObjectId != objectId)
			continue;
		if (object.iSourceNetEntityId != sourceId ||
		    object.LiveState.iOwnerPatternSequence != patternSequence ||
		    (!object.Hits.empty() && !object.bRoomOwnedTracking) || object.fSpeedMps != 0.f ||
		    object.bTrackLockedTargetUntilFirstPulse)
			return false;
		if (!object.bHasExternalPoseStep)
			object.LiveState.PreviousPose = object.LiveState.CurrentPose;
		object.bHasExternalPoseStep = true;
		auto& pose = object.LiveState.CurrentPose;
		pose.fPositionX = x;
		pose.fPositionY = y;
		pose.fPositionZ = z;
		++m_iRevision;
		if (m_iRevision == 0u)
			m_iRevision = 1u;
		return true;
	}
	return false;
}

// ID·source·pattern 회차가 맞고 hit가 없거나 Room 추적인 항목만 취소한다. 오래된 요청의 명시적 false는 live를 바꾸지 않는다.
bool LostArk::Server::CCombatObjectRuntime::Cancel_OwnedVisualObject(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId,
    const LostArk::Shared::NET_ENTITY_ID sourceId,
    const std::uint32_t patternSequence)
{
	for (std::size_t index = 0u; index < m_Objects.size(); ++index)
	{
		const auto& object = m_Objects[index];
		if (object.iCombatObjectId != objectId)
			continue;
		if (object.iSourceNetEntityId != sourceId ||
		    object.LiveState.iOwnerPatternSequence != patternSequence ||
		    (!object.Hits.empty() && !object.bRoomOwnedTracking))
			return false;
		Despawn_At(index);
		return true;
	}
	return false;
}

// live를 끝에서부터 비우고 필요한 despawn을 남긴다. 다음 ID/event sequence와 기존 lifecycle 대기열을 초기화하지 않는다.
void LostArk::Server::CCombatObjectRuntime::Reset()
{
	while (!m_Objects.empty())
		Despawn_At(m_Objects.size() - 1u);
}

// 관찰자가 없는 빈 방 reset이 이전 세대의 spawn/표현/despawn을 버리는 별도 단계다. live 자체는 바꾸지 않는다.
void LostArk::Server::CCombatObjectRuntime::Discard_PendingLifecycle()
{
	m_PendingSpawned.clear();
	m_PendingPresentationEvents.clear();
	m_PendingDespawned.clear();
}

// 복제 대상이면 제거 메시지를 먼저 추가한 뒤 vector 원소를 지운다. 할당 예외까지 삼키는 함수는 아니며 뒤쪽 원소 참조는 erase로 무효화된다.
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

// 생성 또는 늦은 입장용 메시지의 공통 필드를 현재 객체에서 복사한다. 늦은 입장의 serverTick 교체는 호출자가 따로 수행한다.
LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED LostArk::Server::CCombatObjectRuntime::To_SpawnedMessage(
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

// GameRoom::Broadcast_CombatObjectLifecycle이 출력 vector로 세 큐를 이동한다. 이후 송신 실패가 발생해도 이 함수가 큐를 되돌리지는 않는다.
void LostArk::Server::CCombatObjectRuntime::Drain_Lifecycle(
    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned,
    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT>& outPresentationEvents,
    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED>& outDespawned)
{
	outSpawned = std::move(m_PendingSpawned);
	outPresentationEvents = std::move(m_PendingPresentationEvents);
	outDespawned = std::move(m_PendingDespawned);
	m_PendingSpawned.clear();
	m_PendingPresentationEvents.clear();
	m_PendingDespawned.clear();
}

// 입장 준비에서 현재 살아 있는 복제 객체만 새 출력에 담는다. 원래 spawnTick은 유지하며 관측 시각과 현재 pose로 늦은 입장을 구성한다.
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
	std::sort(outSpawned.begin(),
	          outSpawned.end(),
	          [](const auto& left, const auto& right)
	          {
		          return left.iCombatObjectId < right.iCombatObjectId;
	          });
}

// 복제 pose 목록을 새로 채워 ID순으로 정렬한다. 반환 false는 protocol 상한 초과이며 출력 vector를 이전 값으로 복원하지 않는다.
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
	std::sort(outSnapshots.begin(),
	          outSnapshots.end(),
	          [](const auto& left, const auto& right)
	          {
		          return left.iCombatObjectId < right.iCombatObjectId;
	          });
	return outSnapshots.size() <= LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT;
}

// 숫자 밸런스 반영 때 live hit를 갱신한다. player는 이전/새 raw 비율, boss는 새 attack/rate, 패턴 비율은 matching entry를 사용한다.
// 존재하지 않는 정의는 해당 갱신을 건너뛴다. PvP whole-cast 저장값과 counter 자격, pinned revision은 이 함수가 바꾸지 않는다.
void LostArk::Server::CCombatObjectRuntime::Refresh_NumericBalance(
    const CGameplayCatalog& previous,
    const CGameplayCatalog& updated,
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
						damage = static_cast<std::uint32_t>(
						    (std::min)(static_cast<std::uint64_t>(
						                   (std::numeric_limits<std::uint32_t>::max)()),
						               static_cast<std::uint64_t>(damage) * newPlayerDamage /
						                   oldPlayerDamage));
			}
			else if (boss && !hit.strDamageProfileId.empty())
			{
				const auto rate = updated.Find_DamageRatePercent(hit.strDamageProfileId);
				if (rate)
					std::fill(hit.RepeatRawDamage.begin(),
					          hit.RepeatRawDamage.end(),
					          CGameplayCatalog::Resolve_Damage(boss->iAttackPower, rate));
			}
			if (!hit.strNumericBalanceId.empty())
				for (const auto& entry : entries)
					if (entry.eDomain == LostArk::Shared::BALANCE_DOMAIN::PATTERN_DAMAGE &&
					    entry.strId == hit.strNumericBalanceId && entry.strField == "maxHpDamagePercent")
					{
						hit.iDamagePercent = static_cast<std::uint32_t>(entry.fValue);
						break;
					}
		}
	}
}
