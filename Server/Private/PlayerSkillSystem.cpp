#include "PlayerSkillSystem.h"

#include "Gameplay/CombatCollisionContract.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30;

	void Sample_RootMotion(
		const std::vector<LostArk::Server::PLAYER_ROOT_MOTION_SAMPLE>& samples,
		const float elapsedSeconds,
		float& outForward,
		float& outLateral)
	{
		outForward = 0.f;
		outLateral = 0.f;
		if (samples.empty())
			return;
		const float elapsedMs = elapsedSeconds * 1000.f;
		if (elapsedMs <= static_cast<float>(samples.front().iTimeMs))
		{
			outForward = samples.front().fForward;
			outLateral = samples.front().fLateral;
			return;
		}
		for (std::size_t index = 1; index < samples.size(); ++index)
		{
			const auto& previous = samples[index - 1];
			const auto& current = samples[index];
			if (elapsedMs > static_cast<float>(current.iTimeMs))
				continue;
			const float span = static_cast<float>(current.iTimeMs) -
				static_cast<float>(previous.iTimeMs);
			const float alpha = span <= 0.f ? 0.f :
				(elapsedMs - static_cast<float>(previous.iTimeMs)) / span;
			outForward = previous.fForward +
				(current.fForward - previous.fForward) * alpha;
			outLateral = previous.fLateral +
				(current.fLateral - previous.fLateral) * alpha;
			return;
		}
		outForward = samples.back().fForward;
		outLateral = samples.back().fLateral;
	}

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

	/* The aim arrives as a world point, so it is only a direction once the
	player's own position is taken out of it. A press on top of the player
	carries no direction and keeps the facing it already had. */
	void ResolveAimDirection(
		const LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Shared::C2S_USE_SKILL& command,
		float& outDirectionX,
		float& outDirectionZ)
	{
		outDirectionX = command.fAimX - player.fPositionX;
		outDirectionZ = command.fAimZ - player.fPositionZ;
		const float length = std::sqrt(
			outDirectionX * outDirectionX + outDirectionZ * outDirectionZ);
		if (length > 0.0001f)
		{
			outDirectionX /= length;
			outDirectionZ /= length;
			return;
		}
		const float yawRadians = player.fYawDegrees / RADIANS_TO_DEGREES;
		outDirectionX = std::sin(yawRadians);
		outDirectionZ = std::cos(yawRadians);
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
			ResolveAimDirection(
				player,
				command,
				player.fBufferedComboAimX,
				player.fBufferedComboAimZ);
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

	float directionX = 0.f;
	float directionZ = 0.f;
	ResolveAimDirection(player, command, directionX, directionZ);

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
		PLAYER_SKILL_KIND::ACTIVE == skill->eSkillKind ? 0u : 1u;
	player.hasBufferedComboInput = false;
	player.hasReleasedHold = false;
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
	const bool isHold = PLAYER_SKILL_KIND::HOLD == skill->eSkillKind;
	const bool isCounter = PLAYER_SKILL_KIND::COUNTER == skill->eSkillKind;
	const bool hasStage =
		(PLAYER_SKILL_KIND::COMBO == skill->eSkillKind || isHold ||
			isCounter) &&
		stageIndex < skill->ComboStages.size();
	const std::uint32_t durationMs = hasStage ?
		skill->ComboStages[stageIndex].iActionDurationMs :
		skill->iActionDurationMs;
	const std::uint32_t hitMs = hasStage ?
		skill->ComboStages[stageIndex].iHitTimeMs :
		skill->iHitTimeMs;
	const float durationSeconds =
		static_cast<float>(durationMs) * MILLISECONDS_TO_SECONDS;
	float stepForward = 0.f;
	float stepLateral = 0.f;
	/* A stage advance resets the action clock, so a staged skill reads the
	running stage's own curve on that same clock. */
	const std::vector<PLAYER_ROOT_MOTION_SAMPLE>& rootMotion = hasStage ?
		skill->ComboStages[stageIndex].RootMotion : skill->RootMotion;
	if (!rootMotion.empty())
	{
		const float previousSeconds =
			(std::max)(0.f, player.fActionElapsedSeconds - fixedDeltaSeconds);
		float previousForward = 0.f;
		float previousLateral = 0.f;
		float currentForward = 0.f;
		float currentLateral = 0.f;
		Sample_RootMotion(rootMotion, previousSeconds,
			previousForward, previousLateral);
		Sample_RootMotion(rootMotion, player.fActionElapsedSeconds,
			currentForward, currentLateral);
		stepForward = currentForward - previousForward;
		stepLateral = currentLateral - previousLateral;
	}
	else if (skill->fMovementDistance > 0.f && durationSeconds > 0.f)
	{
		stepForward = skill->fMovementDistance /
			durationSeconds * fixedDeltaSeconds;
	}

	if (0.f != stepForward || 0.f != stepLateral)
	{
		const float rightX = player.fSkillAimDirectionZ;
		const float rightZ = -player.fSkillAimDirectionX;
		const float nextX = player.fPositionX +
			player.fSkillAimDirectionX * stepForward + rightX * stepLateral;
		const float nextZ = player.fPositionZ +
			player.fSkillAimDirectionZ * stepForward + rightZ * stepLateral;
		SERVER_NAV_POINT projected{};
		if (nullptr != navigation && navigation->Is_Loaded())
		{
			if (navigation->Project_Point(nextX, nextZ, projected))
			{
				player.fPositionX = nextX;
				player.fPositionY = projected.y;
				player.fPositionZ = nextZ;
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
	const bool holdWithoutDamage = isHold && 3u != player.iComboStage;
	/* The guard stage lands nothing: the damage belongs to the counter it buys. */
	const bool counterWithoutDamage = isCounter && 2u != player.iComboStage;
	if (!skill->strDamageProfileId.empty() && !holdWithoutDamage &&
		!counterWithoutDamage &&
		!player.hasAppliedSkillDamage && player.fActionElapsedSeconds >= hitSeconds)
	{
		SERVER_WORLD_ENTITY* closestBoss = nullptr;
		float closestDistanceSquared = 0.f;
		const LostArk::Shared::CombatCollision::CIRCLE_XZ skillCircle{
			player.fPositionX,
			player.fPositionZ,
			skill->fMaximumRange
		};
		for (SERVER_WORLD_ENTITY& entity : worldEntities)
		{
			if ((WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind &&
				WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind) ||
				SERVER_ENTITY_ACTION::DEAD == entity.eAction || 0u == entity.iCurrentHp)
			{
				continue;
			}
			const float deltaX = entity.fPositionX - player.fPositionX;
			const float deltaZ = entity.fPositionZ - player.fPositionZ;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			const BOSS_RUNTIME_PROFILE* bossProfile =
				catalog.Find_Boss(entity.strArchetypeId);
			const float targetRadius =
				(WORLD_BOOTSTRAP_KIND::MONSTER == entity.eKind ?
					entity.fCollisionRadius :
					(nullptr == bossProfile ? 0.f : bossProfile->fCollisionRadius));
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ targetBody{
				entity.fPositionX,
				entity.fPositionZ,
				targetRadius
			};
			if (LostArk::Shared::CombatCollision::Circles_Overlap(
				skillCircle, targetBody) &&
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
			const std::uint32_t rawDamage = CGameplayCatalog::Resolve_Damage(
				nullptr == playerProfile ? 0u : playerProfile->iAttackPower,
				catalog.Find_DamageRatePercent(skill->strDamageProfileId));
			const std::uint32_t damage = WORLD_BOOTSTRAP_KIND::MONSTER == closestBoss->eKind ?
				CGameplayCatalog::Apply_Defense(rawDamage, closestBoss->iDefense) : rawDamage;
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

	const bool holdLeavesLoop = isHold && 2u == player.iComboStage &&
		player.hasReleasedHold;
	const bool holdSkipsLoop = isHold && 1u == player.iComboStage &&
		player.hasReleasedHold &&
		player.fActionElapsedSeconds >= durationSeconds;

	/* A counter never advances on its own clock: only a hit taken inside the
	guard window promotes it, which Try_Counter does. */
	const bool hasNextStage = hasStage && !isCounter &&
		(isHold ? static_cast<std::size_t>(player.iComboStage) <
				skill->ComboStages.size()
			: player.hasBufferedComboInput &&
				static_cast<std::size_t>(player.iComboStage) <
					skill->ComboStages.size());
	/* A buffered press cancels the rest of the clip once the hit has landed,
	which is what makes a combo read as fast. Every stage's hit time is inside
	its own input window, so cutting here never drops damage. */
	const bool cancelsIntoNextStage =
		!isHold && hasNextStage && player.hasAppliedSkillDamage;

	if (cancelsIntoNextStage || holdLeavesLoop ||
		player.fActionElapsedSeconds >= durationSeconds)
	{
		if (hasNextStage)
		{
			/* The press that bought this stage aimed somewhere, and that is where
			the stage plays: facing and root motion both turn to it. A hold
			advances on its own clock, so it keeps the facing it started with. */
			if (player.hasBufferedComboInput)
			{
				player.fSkillAimDirectionX = player.fBufferedComboAimX;
				player.fSkillAimDirectionZ = player.fBufferedComboAimZ;
				player.fYawDegrees = std::atan2(
					player.fBufferedComboAimX,
					player.fBufferedComboAimZ) * RADIANS_TO_DEGREES;
			}
			player.iComboStage = holdSkipsLoop || holdLeavesLoop ?
				3u : player.iComboStage + 1u;
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
			player.hasReleasedHold = false;
		}
	}
}

bool LostArk::Server::CPlayerSkillSystem::Try_Counter(
	SERVER_PLAYER& player,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::SKILL != player.eAction || 1u != player.iComboStage)
		return false;
	const PLAYER_SKILL_DEFINITION* skill =
		catalog.Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill ||
		PLAYER_SKILL_KIND::COUNTER != skill->eSkillKind ||
		2u != skill->ComboStages.size())
	{
		return false;
	}
	/* The guard window reuses the stage's input window: for a counter the thing
	that advances the stage is the hit, so the same two numbers gate it. */
	const PLAYER_COMBO_STAGE& guard = skill->ComboStages[0];
	const float elapsedMs = player.fActionElapsedSeconds * 1000.f;
	if (0u == guard.iInputCloseMs ||
		elapsedMs < static_cast<float>(guard.iInputOpenMs) ||
		elapsedMs > static_cast<float>(guard.iInputCloseMs))
	{
		return false;
	}
	player.iComboStage = 2u;
	player.hasBufferedComboInput = false;
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	// A changed start tick is how the client learns to play the counter clip.
	player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	return true;
}

void LostArk::Server::CPlayerSkillSystem::Release(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_RELEASE_SKILL& command,
	const CGameplayCatalog& catalog)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::SKILL != player.eAction ||
		player.iCurrentSkillId != command.iSkillId)
	{
		return;
	}
	const PLAYER_SKILL_DEFINITION* skill =
		catalog.Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill || PLAYER_SKILL_KIND::HOLD != skill->eSkillKind)
		return;
	player.hasReleasedHold = true;
}
