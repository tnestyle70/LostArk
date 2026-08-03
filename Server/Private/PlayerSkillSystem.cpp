#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30;

	bool IsNewerSequence(
		const std::uint32_t candidate,
		const std::uint32_t previous)
	{
		return 0u != candidate &&
			static_cast<std::int32_t>(candidate - previous) > 0;
	}

	std::uint32_t MillisecondsToTicks(const std::uint32_t milliseconds)
	{
		return (milliseconds * SERVER_TICK_HZ + 999u) / 1000u;
	}
}

bool LostArk::Server::CPlayerSkillSystem::Try_Start(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog,
	const std::uint32_t actionStartTick) const
{
	using namespace LostArk::Shared;
	const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(command.iSkillId);
	if (!IsNewerSequence(command.iClientSequence, player.iLastSkillSequence) ||
		nullptr == skill || skill->eCharacterClass != player.eCharacterClass ||
		PLAYER_ACTION_STATE::NONE != player.eAction || 0u == player.iCurrentHp ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ))
	{
		return false;
	}
	const auto cooldown = player.CooldownEndTickBySkillId.find(command.iSkillId);
	if ((cooldown != player.CooldownEndTickBySkillId.end() &&
		cooldown->second > actionStartTick) ||
		player.iCurrentResource < skill->iResourceCost)
	{
		return false;
	}

	float directionX = command.fAimX - player.fPositionX;
	float directionZ = command.fAimZ - player.fPositionZ;
	const float length = std::sqrt(directionX * directionX + directionZ * directionZ);
	if (length > 0.0001f)
	{
		directionX /= length;
		directionZ /= length;
	}
	else
	{
		const float yawRadians = player.fYawDegrees / RADIANS_TO_DEGREES;
		directionX = std::sin(yawRadians);
		directionZ = std::cos(yawRadians);
	}

	player.iLastSkillSequence = command.iClientSequence;
	player.eAction = PLAYER_ACTION_STATE::SKILL;
	player.iCurrentSkillId = command.iSkillId;
	player.iActionStartTick = 0u == actionStartTick ? 1u : actionStartTick;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = directionX;
	player.fSkillAimDirectionZ = directionZ;
	player.hasAppliedSkillDamage = false;
	player.iCurrentResource -= skill->iResourceCost;
	player.CooldownEndTickBySkillId.insert_or_assign(
		command.iSkillId,
		player.iActionStartTick + MillisecondsToTicks(skill->iCooldownMs));
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.fYawDegrees = std::atan2(directionX, directionZ) * RADIANS_TO_DEGREES;
	return true;
}

void LostArk::Server::CPlayerSkillSystem::Update(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const CServerNavigation* navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::DEAD == player.eAction || 0u == player.iCurrentHp)
	{
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.hasMoveGoal = false;
		return;
	}
	if (PLAYER_ACTION_STATE::SKILL != player.eAction)
	{
		if (0u == serverTick % 6u && player.iCurrentResource < player.iMaximumResource)
			++player.iCurrentResource;
		return;
	}

	const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill)
	{
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0;
		return;
	}
	player.fActionElapsedSeconds += fixedDeltaSeconds;
	const float durationSeconds =
		static_cast<float>(skill->iActionDurationMs) * MILLISECONDS_TO_SECONDS;
	if (skill->fMovementDistance > 0.f && durationSeconds > 0.f)
	{
		const float step = skill->fMovementDistance /
			durationSeconds * fixedDeltaSeconds;
		const float nextX = player.fPositionX + player.fSkillAimDirectionX * step;
		const float nextZ = player.fPositionZ + player.fSkillAimDirectionZ * step;
		SERVER_NAV_POINT projected{};
		if (nullptr != navigation && navigation->Is_Loaded())
		{
			if (navigation->Project_Point(nextX, nextZ, projected))
			{
				player.fPositionX = projected.x;
				player.fPositionY = projected.y;
				player.fPositionZ = projected.z;
			}
		}
		else
		{
			player.fPositionX = nextX;
			player.fPositionZ = nextZ;
		}
	}

	const float hitSeconds =
		static_cast<float>(skill->iHitTimeMs) * MILLISECONDS_TO_SECONDS;
	if (!player.hasAppliedSkillDamage && player.fActionElapsedSeconds >= hitSeconds)
	{
		SERVER_WORLD_ENTITY* closestBoss = nullptr;
		float closestDistanceSquared = skill->fMaximumRange * skill->fMaximumRange;
		for (SERVER_WORLD_ENTITY& entity : worldEntities)
		{
			if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind ||
				SERVER_ENTITY_ACTION::DEAD == entity.eAction || 0u == entity.iCurrentHp)
			{
				continue;
			}
			const float deltaX = entity.fPositionX - player.fPositionX;
			const float deltaZ = entity.fPositionZ - player.fPositionZ;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			if (distanceSquared <= closestDistanceSquared)
			{
				closestDistanceSquared = distanceSquared;
				closestBoss = &entity;
			}
		}
		if (nullptr != closestBoss)
		{
			const std::uint32_t damage = catalog.Find_Damage(skill->strDamageProfileId);
			closestBoss->iCurrentHp =
				damage >= closestBoss->iCurrentHp ? 0u : closestBoss->iCurrentHp - damage;
			if (0u == closestBoss->iCurrentHp)
			{
				closestBoss->eAction = SERVER_ENTITY_ACTION::DEAD;
				closestBoss->iActionStartTick = 0u == serverTick ? 1u : serverTick;
				closestBoss->MovePath.clear();
			}
		}
		player.hasAppliedSkillDamage = true;
	}

	if (player.fActionElapsedSeconds >= durationSeconds)
	{
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0;
		player.fActionElapsedSeconds = 0.f;
		player.hasAppliedSkillDamage = false;
	}
}
