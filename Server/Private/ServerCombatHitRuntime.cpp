#include "ServerCombatHitRuntime.h"

#include "BossCombatRuntime.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;

	bool IsDamageableWorldTarget(
		const LostArk::Server::SERVER_WORLD_ENTITY& target)
	{
		using namespace LostArk::Server;
		return (WORLD_BOOTSTRAP_KIND::BOSS == target.eKind ||
			WORLD_BOOTSTRAP_KIND::MONSTER == target.eKind) &&
			SERVER_ENTITY_ACTION::DEAD != target.eAction &&
			0u != target.iCurrentHp;
	}

	void PushDamageEvent(
		const LostArk::Shared::NET_ENTITY_ID targetId,
		const std::uint32_t amount,
		const float x,
		const float y,
		const float z,
		const bool outgoing,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& events)
	{
		if (0u == amount || events.size() >= LostArk::Shared::MAX_DAMAGE_EVENTS)
			return;
		LostArk::Shared::DAMAGE_EVENT event{};
		event.iTargetNetEntityId = targetId;
		event.iAmount = amount;
		event.fPositionX = x;
		event.fPositionY = y;
		event.fPositionZ = z;
		event.isOutgoing = outgoing;
		events.push_back(event);
	}
}

LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_PlayerToWorld(
	SERVER_WORLD_ENTITY& target,
	const SERVER_PLAYER_TO_WORLD_HIT& hit,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	if (!IsDamageableWorldTarget(target) ||
		LostArk::Shared::INVALID_SKILL_ID == hit.iSkillId)
	{
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	}

	std::uint32_t damage = 0u;
	if (WORLD_BOOTSTRAP_KIND::BOSS == target.eKind)
	{
		BOSS_INCOMING_HIT incoming{};
		incoming.iSourcePlayerId = hit.iSourcePlayerId;
		incoming.iSkillId = hit.iSkillId;
		incoming.iRawDamage = hit.iRawDamage;
		incoming.iStaggerDamage = hit.iStaggerDamage;
		incoming.iPartDamage = hit.iPartDamage;
		incoming.iCounterPower = hit.iCounterPower;
		incoming.iServerTick = hit.iServerTick;
		incoming.fSourceX = hit.fSourceX;
		incoming.fSourceZ = hit.fSourceZ;
		damage = CBossCombatRuntime::Apply_PlayerHit(target, incoming).iHealthDamage;
	}
	else
	{
		damage = CGameplayCatalog::Apply_Defense(hit.iRawDamage, target.iDefense);
		target.iCurrentHp = damage >= target.iCurrentHp ?
			0u : target.iCurrentHp - damage;
	}
	PushDamageEvent(
		target.iNetEntityId, damage,
		target.fPositionX, target.fPositionY, target.fPositionZ,
		true, outDamageEvents);

	const float pushDistance = 0u == hit.iPushMs ?
		0.f : hit.fPushRangeM * target.fHitKnockbackScale;
	if (0u != damage && WORLD_BOOTSTRAP_KIND::MONSTER == target.eKind &&
		0.f != pushDistance && 0u != target.iCurrentHp)
	{
		float directionX = target.fPositionX - hit.fSourceX;
		float directionZ = target.fPositionZ - hit.fSourceZ;
		const float length = std::sqrt(
			directionX * directionX + directionZ * directionZ);
		if (length < 0.0001f)
		{
			directionX = hit.fFallbackDirectionX;
			directionZ = hit.fFallbackDirectionZ;
		}
		else
		{
			directionX /= length;
			directionZ /= length;
		}
		const float durationSeconds =
			static_cast<float>(hit.iPushMs) * MILLISECONDS_TO_SECONDS;
		target.fKnockbackDirectionX = directionX;
		target.fKnockbackDirectionZ = directionZ;
		target.fKnockbackSpeed = pushDistance / durationSeconds;
		target.fKnockbackRemainingSeconds = durationSeconds;
	}
	if (0u != target.iCurrentHp)
		return SERVER_COMBAT_HIT_RESULT::LANDED;
	target.eAction = SERVER_ENTITY_ACTION::DEAD;
	target.iActionStartTick = 0u == hit.iServerTick ? 1u : hit.iServerTick;
	target.MovePath.clear();
	return SERVER_COMBAT_HIT_RESULT::KILLED;
}

LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_WorldToPlayer(
	SERVER_PLAYER& target,
	const SERVER_WORLD_TO_PLAYER_HIT& hit,
	const CGameplayCatalog& catalog,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	using namespace LostArk::Shared;
	if (0u == target.iCurrentHp || !target.isCombatReady ||
		PLAYER_ACTION_STATE::DEAD == target.eAction ||
		PLAYER_ACTION_STATE::FALLING == target.eAction ||
		PLAYER_ACTION_STATE::GRABBED == target.eAction)
	{
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	}
	if (CPlayerSkillSystem::Try_Counter(target, catalog, hit.iServerTick))
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;

	const PLAYER_RUNTIME_PROFILE* playerProfile =
		catalog.Find_Player(target.eCharacterClass);
	const std::uint32_t damage = CGameplayCatalog::Apply_Defense(
		hit.iRawDamage, nullptr == playerProfile ? 0u : playerProfile->iDefense);
	target.iCurrentHp = damage >= target.iCurrentHp ?
		0u : target.iCurrentHp - damage;
	PushDamageEvent(
		target.iNetEntityId, damage,
		target.fPositionX, target.fPositionY, target.fPositionZ,
		false, outDamageEvents);
	if (0u == target.iCurrentHp)
	{
		target.eAction = PLAYER_ACTION_STATE::DEAD;
		target.iCurrentSkillId = INVALID_SKILL_ID;
		target.Clear_SkillTarget();
		target.iActionStartTick = 0u == hit.iServerTick ? 1u : hit.iServerTick;
		target.hasMoveGoal = false;
		target.MovePath.clear();
		return SERVER_COMBAT_HIT_RESULT::KILLED;
	}
	CPlayerSkillSystem::Arm_PlayerHitReaction(
		target,
		hit.fSourceX,
		hit.fSourceZ,
		hit.fPushRangeM,
		hit.iPushMs,
		hit.bKnockdown,
		hit.iDownMs,
		hit.iServerTick);
	return SERVER_COMBAT_HIT_RESULT::LANDED;
}
