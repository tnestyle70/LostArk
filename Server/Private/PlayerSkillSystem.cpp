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

	bool IsInsideComboWindow(
		const LostArk::Server::PLAYER_SKILL_DEFINITION& skill,
		const LostArk::Server::SERVER_PLAYER& player)
	{
		if (0u == player.iComboStage ||
			player.iComboStage > skill.ComboStages.size())
		{
			return false;
		}
		const auto& stage = skill.ComboStages[player.iComboStage - 1u];
		if (0u == stage.iInputCloseMs)
			return false;
		const float elapsedMs = player.fActionElapsedSeconds * 1000.f;
		return elapsedMs >= static_cast<float>(stage.iInputOpenMs) &&
			elapsedMs <= static_cast<float>(stage.iInputCloseMs);
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
		0u == player.iCurrentHp ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ) ||
		(PLAYER_STANCE_ID::NONE != skill->eRequiredStance &&
			skill->eRequiredStance != player.eStance))
	{
		return false;
	}

	/* A combo is the only reason to accept input while an action runs, and only
	for the same skill inside the stage's own window. Everything else keeps the
	original guard, so no skill can be interrupted by another. */
	if (PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		const bool isComboContinuation =
			PLAYER_SKILL_KIND::COMBO == skill->eSkillKind &&
			PLAYER_ACTION_STATE::SKILL == player.eAction &&
			player.iCurrentSkillId == command.iSkillId &&
			IsInsideComboWindow(*skill, player);
		/* Buffering reports false: GameRoom discards the result, and the client
		only advances its sequence on true, so a buffered press must not look
		like a fresh approval. A second press in the same window is dropped so
		one window can never advance two stages. */
		if (isComboContinuation && !player.hasBufferedComboInput)
		{
			player.iLastSkillSequence = command.iClientSequence;
			player.hasBufferedComboInput = true;
		}
		return false;
	}
	const auto cooldown = player.CooldownEndTickBySkillId.find(command.iSkillId);
	/* Signed difference so a tick counter that wrapped keeps ordering; same
	idiom as IsNewerSequence above. */
	if ((cooldown != player.CooldownEndTickBySkillId.end() &&
		static_cast<std::int32_t>(cooldown->second - actionStartTick) > 0) ||
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
	player.iComboStage =
		PLAYER_SKILL_KIND::COMBO == skill->eSkillKind ? 1u : 0u;
	player.hasBufferedComboInput = false;
	return true;
}

void LostArk::Server::CPlayerSkillSystem::Update(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const CServerNavigation* navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::DEAD == player.eAction || 0u == player.iCurrentHp)
	{
		player.eAction = PLAYER_ACTION_STATE::DEAD;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.hasMoveGoal = false;
		player.iComboStage = 0;
		player.hasBufferedComboInput = false;
		return;
	}
	if (PLAYER_ACTION_STATE::SKILL != player.eAction)
	{
		(void)serverTick;
		/* Regen is data now: the pool is sized to official CostMp, so the refill
		rate has to come from the same profile document. The accumulator pays out
		exactly resourceRegenPerSecond per second without float drift. */
		if (player.iCurrentResource >= player.iMaximumResource)
		{
			player.iResourceAccumulator = 0;
			return;
		}
		const PLAYER_RUNTIME_PROFILE* profile =
			catalog.Find_Player(player.eCharacterClass);
		if (nullptr == profile)
			return;
		player.iResourceAccumulator += profile->iResourceRegenPerSecond;
		while (player.iResourceAccumulator >= SERVER_TICK_HZ &&
			player.iCurrentResource < player.iMaximumResource)
		{
			player.iResourceAccumulator -= SERVER_TICK_HZ;
			++player.iCurrentResource;
		}
		return;
	}

	const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill)
	{
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0;
		player.iComboStage = 0;
		player.hasBufferedComboInput = false;
		return;
	}
	player.fActionElapsedSeconds += fixedDeltaSeconds;
	/* A combo's timing lives on the running stage, not on the skill row; the row
	carries stage one so a non-combo path reads the same as before. */
	const std::size_t stageIndex =
		0u == player.iComboStage ? 0u : player.iComboStage - 1u;
	const bool hasStage =
		PLAYER_SKILL_KIND::COMBO == skill->eSkillKind &&
		stageIndex < skill->ComboStages.size();
	const std::uint32_t durationMs = hasStage ?
		skill->ComboStages[stageIndex].iActionDurationMs :
		skill->iActionDurationMs;
	const std::uint32_t hitMs = hasStage ?
		skill->ComboStages[stageIndex].iHitTimeMs :
		skill->iHitTimeMs;
	const float durationSeconds =
		static_cast<float>(durationMs) * MILLISECONDS_TO_SECONDS;
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
		static_cast<float>(hitMs) * MILLISECONDS_TO_SECONDS;
	if (!skill->strDamageProfileId.empty() &&
		!player.hasAppliedSkillDamage && player.fActionElapsedSeconds >= hitSeconds)
	{
		SERVER_WORLD_ENTITY* closestBoss = nullptr;
		float closestDistanceSquared = 0.f;
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
			/* Official ranges are measured to the target's surface while this test
			is centre to centre, so the boss's collision radius extends the reach.
			Without it every official melee range would whiff on its own boss. */
			const BOSS_RUNTIME_PROFILE* bossProfile =
				catalog.Find_Boss(entity.strArchetypeId);
			const float reach = skill->fMaximumRange +
				(nullptr == bossProfile ? 0.f : bossProfile->fCollisionRadius);
			if (distanceSquared <= reach * reach &&
				(nullptr == closestBoss || distanceSquared < closestDistanceSquared))
			{
				closestDistanceSquared = distanceSquared;
				closestBoss = &entity;
			}
		}
		if (nullptr != closestBoss)
		{
			const PLAYER_RUNTIME_PROFILE* playerProfile =
				catalog.Find_Player(player.eCharacterClass);
			const std::uint32_t damage = CGameplayCatalog::Resolve_Damage(
				nullptr == playerProfile ? 0u : playerProfile->iAttackPower,
				catalog.Find_DamageRatePercent(skill->strDamageProfileId));
			closestBoss->iCurrentHp =
				damage >= closestBoss->iCurrentHp ? 0u : closestBoss->iCurrentHp - damage;
			/* A zero amount is not a hit and the snapshot writer rejects it; the
			cap keeps one overfull tick from suppressing the whole snapshot. */
			if (0u != damage &&
				outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS)
			{
				LostArk::Shared::DAMAGE_EVENT damageEvent{};
				damageEvent.iTargetNetEntityId = closestBoss->iNetEntityId;
				damageEvent.iAmount = damage;
				damageEvent.fPositionX = closestBoss->fPositionX;
				damageEvent.fPositionY = closestBoss->fPositionY;
				damageEvent.fPositionZ = closestBoss->fPositionZ;
				damageEvent.isOutgoing = true;
				outDamageEvents.push_back(damageEvent);
			}
			if (0u == closestBoss->iCurrentHp)
			{
				closestBoss->eAction = SERVER_ENTITY_ACTION::DEAD;
				closestBoss->iActionStartTick = 0u == serverTick ? 1u : serverTick;
				closestBoss->MovePath.clear();
			}
		}
		player.hasAppliedSkillDamage = true;
	}

	const bool hasNextStage = hasStage &&
		player.hasBufferedComboInput &&
		static_cast<std::size_t>(player.iComboStage) <
			skill->ComboStages.size();
	/* A buffered press cancels the rest of the clip once the hit has landed,
	which is what makes a combo read as fast. Every stage's hit time is inside
	its own input window, so cutting here never drops damage. */
	const bool cancelsIntoNextStage =
		hasNextStage && player.hasAppliedSkillDamage;

	if (cancelsIntoNextStage || player.fActionElapsedSeconds >= durationSeconds)
	{
		if (hasNextStage)
		{
			++player.iComboStage;
			player.hasBufferedComboInput = false;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			// The client treats a changed start tick as a new action edge, which
			// is how it learns to play the next stage's clip.
			player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		}
		else
		{
			if (PLAYER_STANCE_ID::NONE != skill->eSetsStance)
				player.eStance = skill->eSetsStance;
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.iActionStartTick = 0;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			player.iComboStage = 0;
			player.hasBufferedComboInput = false;
		}
	}
}
