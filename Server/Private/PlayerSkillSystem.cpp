#include "PlayerSkillSystem.h"

#include "Gameplay/CombatCollisionContract.h"
#include "ServerCombatHitRuntime.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30;

	void Sample_RootMotion(
		const std::vector<LostArk::Server::ROOT_MOTION_SAMPLE>& samples,
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

	/* Lands every caster/projectile hit through the same typed adapter used by
	combat objects.  HP, damage events, counter/stagger/part power, legacy armour
	compatibility, death, and knockback are therefore committed once. */
	void ApplyPlayerHitDamage(
		LostArk::Server::SERVER_WORLD_ENTITY& target,
		const LostArk::Shared::PLAYER_ID sourcePlayerId,
		const LostArk::Shared::SKILL_ID skillId,
		const std::uint32_t staggerDamage,
		const std::uint32_t partDamage,
		const std::uint32_t counterPower,
		const std::uint32_t rawDamage,
		const LostArk::Server::PLAYER_SKILL_HIT* pHit,
		const float sourceX,
		const float sourceZ,
		const float fallbackDirectionX,
		const float fallbackDirectionZ,
		const std::uint32_t serverTick,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
	{
		using namespace LostArk::Server;
		SERVER_PLAYER_TO_WORLD_HIT incoming{};
		incoming.iSourcePlayerId = sourcePlayerId;
		incoming.iSkillId = skillId;
		incoming.iRawDamage = rawDamage;
		incoming.iStaggerDamage = staggerDamage;
		incoming.iPartDamage = partDamage;
		incoming.iCounterPower = counterPower;
		incoming.fSourceX = sourceX;
		incoming.fSourceZ = sourceZ;
		incoming.fFallbackDirectionX = fallbackDirectionX;
		incoming.fFallbackDirectionZ = fallbackDirectionZ;
		incoming.fPushRangeM = nullptr == pHit ? 0.f : pHit->fPushRange;
		incoming.iPushMs = nullptr == pHit ? 0u : pHit->iPushMs;
		incoming.iServerTick = serverTick;
		(void)CServerCombatHitRuntime::Apply_PlayerToWorld(
			target, incoming, outDamageEvents);
	}

	LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ TargetBodyOf(
		const LostArk::Server::CGameplayCatalog& catalog,
		const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		using namespace LostArk::Server;
		const BOSS_RUNTIME_PROFILE* bossProfile =
			catalog.Find_Boss(entity.strArchetypeId);
		const float targetRadius =
			(WORLD_BOOTSTRAP_KIND::MONSTER == entity.eKind ?
				entity.fCollisionRadius :
				(nullptr == bossProfile ? 0.f : bossProfile->fCollisionRadius));
		return LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ{
			entity.fPositionX, entity.fPositionZ, targetRadius };
	}

	bool IsDamageable(const LostArk::Server::SERVER_WORLD_ENTITY& entity)
	{
		using namespace LostArk::Server;
		return (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind ||
			WORLD_BOOTSTRAP_KIND::MONSTER == entity.eKind) &&
			SERVER_ENTITY_ACTION::DEAD != entity.eAction && 0u != entity.iCurrentHp;
	}

	/* The share of a skill's total damage one sub-hit carries: cumulative
	splits so the shares always sum to the whole, never below 1. */
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

	std::uint32_t ProjectileSubHitCount(
		const LostArk::Server::PLAYER_SKILL_PROJECTILE& projectile)
	{
		std::uint32_t count = 0;
		for (const LostArk::Server::PLAYER_PROJECTILE_HIT& hit : projectile.Hits)
			count += hit.Hit.iRepeatCount;
		return count;
	}

	bool Hit_ShapeOverlaps(
		const LostArk::Server::PLAYER_SKILL_HIT& hit,
		const float casterX,
		const float casterZ,
		const float forwardX,
		const float forwardZ,
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target)
	{
		using namespace LostArk::Shared::CombatCollision;
		const float originX = casterX + forwardX * hit.fOffset;
		const float originZ = casterZ + forwardZ * hit.fOffset;
		const bool fullSweep =
			hit.fAngleDegrees <= 0.f || hit.fAngleDegrees >= 360.f;
		switch (hit.iAreaType)
		{
		case 1u:
			return hit.fInner > 0.f ?
				Circle_IntersectsRing(
					target, originX, originZ, hit.fInner, hit.fRange) :
				Circles_Overlap(CIRCLE_XZ{ originX, originZ, hit.fRange }, target);
		case 2u:
			return Circle_IntersectsForwardBox(
				target, originX, originZ, forwardX, forwardZ,
				hit.fRange, hit.fWidth * 0.5f);
		case 3u:
		{
			const bool inRing = hit.fInner > 0.f ?
				Circle_IntersectsRing(
					target, originX, originZ, hit.fInner, hit.fRange) :
				Circles_Overlap(CIRCLE_XZ{ originX, originZ, hit.fRange }, target);
			return inRing && (fullSweep || Circle_IntersectsCone(
				target, originX, originZ, forwardX, forwardZ,
				hit.fRange, hit.fAngleDegrees));
		}
		default:
			return false;
		}
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

	bool IsAutomaticComboStage(
		const LostArk::Server::PLAYER_COMBO_STAGE& stage)
	{
		/* A non-final COMBO stage with no input window is authored to advance on
		its own full-motion boundary. The catalog validators require advance ==
		duration for this shape, so automatic presentation is never truncated. */
		return 0u == stage.iInputOpenMs && 0u == stage.iInputCloseMs &&
			stage.iComboAdvanceMs == stage.iActionDurationMs;
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

	float AimDistance(
		const LostArk::Server::SERVER_PLAYER& player,
		const float aimX,
		const float aimZ)
	{
		const float deltaX = aimX - player.fPositionX;
		const float deltaZ = aimZ - player.fPositionZ;
		return std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
	}

	/* The aim arrives as a world point, so it is only a direction once the
	player's own position is taken out of it. A press on top of the player
	carries no direction and keeps the facing it already had. */
	void ResolveAimDirection(
		const LostArk::Server::SERVER_PLAYER& player,
		const float aimX,
		const float aimZ,
		float& outDirectionX,
		float& outDirectionZ)
	{
		outDirectionX = aimX - player.fPositionX;
		outDirectionZ = aimZ - player.fPositionZ;
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

	bool TryResolveSkillTarget(
		const LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Server::PLAYER_SKILL_DEFINITION& skill,
		const LostArk::Shared::C2S_USE_SKILL& command,
		const LostArk::Server::CServerNavigation* navigation,
		LostArk::Server::SERVER_NAV_POINT& outTarget)
	{
		using namespace LostArk::Shared;
		if (command.eTargetIntent != skill.eTargetIntent)
			return false;
		if (SKILL_TARGET_INTENT_KIND::AIM_POINT == skill.eTargetIntent)
		{
			outTarget = {};
			return true;
		}
		if (SKILL_TARGET_INTENT_KIND::GROUND_POINT != skill.eTargetIntent ||
			nullptr == navigation || !navigation->Is_Loaded() ||
			!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ) ||
			!std::isfinite(skill.fTargetMaximumRange) ||
			skill.fTargetMaximumRange <= 0.f)
		{
			return false;
		}
		const float deltaX = command.fAimX - player.fPositionX;
		const float deltaZ = command.fAimZ - player.fPositionZ;
		const float rangeSquared =
			skill.fTargetMaximumRange * skill.fTargetMaximumRange;
		/* Small float tolerance accepts a Client point clamped to exactly 11 m
		 without widening the authored gameplay range by a visible amount. */
		if (deltaX * deltaX + deltaZ * deltaZ > rangeSquared + 0.001f)
			return false;
		if (skill.requiresWalkableTarget)
			return navigation->Sample_Position(
				command.fAimX, command.fAimZ, outTarget);
		outTarget = { command.fAimX, player.fPositionY, command.fAimZ };
		return true;
	}
}

bool LostArk::Server::CPlayerSkillSystem::Try_Start(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog,
	const std::uint32_t actionStartTick,
	const CServerNavigation* navigation) const
{
	return Try_StartInternal(
		player, command, catalog, actionStartTick, false, navigation);
}

bool LostArk::Server::CPlayerSkillSystem::Try_StartPending(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog,
	const std::uint32_t actionStartTick,
	const CServerNavigation* navigation) const
{
	return Try_StartInternal(
		player, command, catalog, actionStartTick, true, navigation);
}

bool LostArk::Server::CPlayerSkillSystem::Try_StagePendingSkill(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog,
	const CServerNavigation* navigation) const
{
	using namespace LostArk::Shared;
	const PLAYER_SKILL_DEFINITION* running =
		catalog.Find_Skill(player.iCurrentSkillId);
	const PLAYER_SKILL_DEFINITION* requested =
		catalog.Find_Skill(command.iSkillId);
	SERVER_NAV_POINT stagedTarget{};
	if (PLAYER_ACTION_STATE::SKILL != player.eAction ||
		nullptr == running || PLAYER_SKILL_KIND::COMBO != running->eSkillKind ||
		command.iSkillId == player.iCurrentSkillId ||
		!IsNewerSequence(command.iClientSequence, player.iLastSkillSequence) ||
		nullptr == requested || requested->eCharacterClass != player.eCharacterClass ||
		0u == player.iCurrentHp ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ) ||
		!TryResolveSkillTarget(
			player, *requested, command, navigation, stagedTarget))
	{
		return false;
	}

	player.iLastSkillSequence = command.iClientSequence;
	player.PendingCommand.Set_Skill(command);
	return true;
}

bool LostArk::Server::CPlayerSkillSystem::Try_StartInternal(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_USE_SKILL& command,
	const CGameplayCatalog& catalog,
	const std::uint32_t actionStartTick,
	const bool sequenceAlreadyConsumed,
	const CServerNavigation* navigation) const
{
	using namespace LostArk::Shared;
	const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(command.iSkillId);
	SERVER_NAV_POINT stagedTarget{};
	if ((!sequenceAlreadyConsumed &&
			!IsNewerSequence(command.iClientSequence, player.iLastSkillSequence)) ||
		nullptr == skill || skill->eCharacterClass != player.eCharacterClass ||
		0u == player.iCurrentHp ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ) ||
		(PLAYER_STANCE_ID::NONE != skill->eRequiredStance &&
			skill->eRequiredStance != player.eStance))
	{
		return false;
	}
	if (!TryResolveSkillTarget(
		player, *skill, command, navigation, stagedTarget))
	{
		return false;
	}

	/* A STANDUP skill exists only to leave KNOCKDOWN: knocked down is the one
	state it starts from and the one state it cannot be pressed outside of. */
	const bool isStandup = PLAYER_SKILL_KIND::STANDUP == skill->eSkillKind;
	if (isStandup != (PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction))
		return false;

	/* A combo is the only reason to accept input while an action runs, and only
	for the same skill inside the stage's own window. Everything else keeps the
	original guard, so no skill can be interrupted by another. */
	if (!isStandup && PLAYER_ACTION_STATE::NONE != player.eAction)
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
				command.fAimX,
				command.fAimZ,
				player.fBufferedComboAimX,
				player.fBufferedComboAimZ);
			player.fBufferedComboAimDistance =
				AimDistance(player, command.fAimX, command.fAimZ);
		}
		return false;
	}
	const auto cooldown = player.CooldownEndTickBySkillId.find(command.iSkillId);
	/* Signed difference so a tick counter that wrapped keeps ordering; same
	idiom as IsNewerSequence above. */
	if ((cooldown != player.CooldownEndTickBySkillId.end() &&
		static_cast<std::int32_t>(cooldown->second - actionStartTick) > 0) ||
		player.iCurrentResource < skill->iResourceCost ||
			player.iCurrentIdentity < skill->iIdentityCost)
	{
		return false;
	}
	/* A pair of opposite-direction stance-swap skills (LanceMaster's 34000/34500)
	sit on their own independent CooldownEndTickBySkillId entries, so the reverse
	skill is otherwise free to fire the instant the first one's action completes
	and flips eStance -- same key, opposite skillId, no cooldown in its way. Gate
	any stance-setting skill on the shared timer below so a swap can't be undone
	within the same window it just opened. */
	if (PLAYER_STANCE_ID::NONE != skill->eSetsStance &&
		static_cast<std::int32_t>(player.iStanceSwitchCooldownEndTick - actionStartTick) > 0)
	{
		return false;
	}

	float directionX = 0.f;
	float directionZ = 0.f;
	ResolveAimDirection(
		player, command.fAimX, command.fAimZ, directionX, directionZ);

	if (isStandup)
	{
		player.iKnockdownEndTick = 0u;
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.iHitReactionGraceEndTick =
			actionStartTick + PLAYER_HIT_REACTION_GRACE_TICKS;
	}
	if (!sequenceAlreadyConsumed)
		player.iLastSkillSequence = command.iClientSequence;
	player.eAction = PLAYER_ACTION_STATE::SKILL;
	player.iCurrentSkillId = command.iSkillId;
	player.iActionStartTick = 0u == actionStartTick ? 1u : actionStartTick;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = directionX;
	player.fSkillAimDirectionZ = directionZ;
	player.fSkillAimDistance = AimDistance(player, command.fAimX, command.fAimZ);
	player.Clear_SkillTarget();
	if (SKILL_TARGET_INTENT_KIND::GROUND_POINT == skill->eTargetIntent)
	{
		player.hasSkillTarget = true;
		player.fSkillTargetX = stagedTarget.x;
		player.fSkillTargetY = stagedTarget.y;
		player.fSkillTargetZ = stagedTarget.z;
	}
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.iCurrentResource -= skill->iResourceCost;
	player.iCurrentIdentity -= skill->iIdentityCost;
	player.CooldownEndTickBySkillId.insert_or_assign(
		command.iSkillId,
		player.iActionStartTick + MillisecondsToTicks(skill->iCooldownMs));
	if (PLAYER_STANCE_ID::NONE != skill->eSetsStance)
	{
		player.iStanceSwitchCooldownEndTick =
			player.iActionStartTick + MillisecondsToTicks(skill->iCooldownMs);
	}
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.fYawDegrees = std::atan2(directionX, directionZ) * RADIANS_TO_DEGREES;
	player.iComboStage =
		PLAYER_SKILL_KIND::ACTIVE == skill->eSkillKind || isStandup ?
		0u : 1u;
	player.hasBufferedComboInput = false;
	player.hasReleasedHold = false;
	player.PendingCommand.Clear();
	return true;
}

bool LostArk::Server::CPlayerSkillSystem::Is_HoldingGaugedStance(
	const SERVER_PLAYER& player,
	const PLAYER_RUNTIME_PROFILE& profile)
{
	/* A class with iIdentityStanceSwitchCost instead of a drain rate pays once
	at the moment it switches (see Update, the eSetsStance branch), not for
	however long it stays switched, so it never holds a gauged stance here. */
	return 0u != profile.iIdentityDrainPerSecond &&
		player.eStance != profile.eDefaultStance;
}

/* The gauge fills whenever the stance it pays for is not held, and empties while
it is. Reaching empty is what drops the stance, so the player never has to press
the toggle to be let out. A class that spends by iIdentityStanceSwitchCost
instead never holds here, so this only ever regenerates it. A cyclic class
(DimensionMaster) never holds either, and never caps at full: reaching the
maximum wraps it back to 0 and the fill keeps going, like a clock hand. */
void LostArk::Server::CPlayerSkillSystem::Update_Identity(
	SERVER_PLAYER& player,
	const PLAYER_RUNTIME_PROFILE& profile)
{
	if (0u == profile.iMaximumIdentity)
		return;
	const bool isHolding = Is_HoldingGaugedStance(player, profile);
	const bool isCyclic = 0u != profile.iIdentityCyclic;
	const std::uint32_t rate = isHolding ?
		profile.iIdentityDrainPerSecond : profile.iIdentityRegenPerSecond;
	if (0u == rate ||
		(!isHolding && !isCyclic && player.iCurrentIdentity >= player.iMaximumIdentity))
	{
		player.iIdentityAccumulator = 0u;
	}
	else
	{
		player.iIdentityAccumulator += rate;
		while (player.iIdentityAccumulator >= SERVER_TICK_HZ)
		{
			player.iIdentityAccumulator -= SERVER_TICK_HZ;
			if (isHolding)
			{
				if (0u == player.iCurrentIdentity)
					break;
				--player.iCurrentIdentity;
			}
			else if (isCyclic)
			{
				++player.iCurrentIdentity;
				if (player.iCurrentIdentity >= player.iMaximumIdentity)
					player.iCurrentIdentity = 0u;
			}
			else
			{
				if (player.iCurrentIdentity >= player.iMaximumIdentity)
					break;
				++player.iCurrentIdentity;
			}
		}
	}
	if (isHolding && 0u == player.iCurrentIdentity)
	{
		player.eStance = profile.eDefaultStance;
		player.iIdentityAccumulator = 0u;
	}
}

void LostArk::Server::CPlayerSkillSystem::Update_Projectiles(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	using namespace LostArk::Shared;
	for (std::size_t index = 0; index < player.Projectiles.size();)
	{
		SERVER_SKILL_PROJECTILE& projectile = player.Projectiles[index];
		const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(projectile.iSkillId);
		const std::vector<PLAYER_SKILL_PROJECTILE>* definitions = nullptr;
		if (nullptr != skill)
		{
			definitions = projectile.iStageIndex < skill->ComboStages.size() &&
				!skill->ComboStages[projectile.iStageIndex].Projectiles.empty() ?
				&skill->ComboStages[projectile.iStageIndex].Projectiles :
				&skill->Projectiles;
		}
		if (nullptr == definitions ||
			projectile.iProjectileIndex >= definitions->size())
		{
			player.Projectiles.erase(player.Projectiles.begin() + index);
			continue;
		}
		const PLAYER_SKILL_PROJECTILE& definition =
			(*definitions)[projectile.iProjectileIndex];

		/* Move first, then judge at the new pose: a missile that reaches its
		distance this tick still lands the hits at the point it stopped. */
		projectile.fElapsedSeconds += fixedDeltaSeconds;
		projectile.fRemainingSeconds -= fixedDeltaSeconds;
		if (projectile.fSpeed > 0.f)
		{
			float step = projectile.fSpeed * fixedDeltaSeconds;
			if (projectile.fRemainingDistance >= 0.f)
			{
				step = (std::min)(step, projectile.fRemainingDistance);
				projectile.fRemainingDistance -= step;
			}
			projectile.fPositionX += projectile.fDirectionX * step;
			projectile.fPositionZ += projectile.fDirectionZ * step;
		}
		const bool expired = projectile.fRemainingSeconds <= 0.f ||
			(projectile.fSpeed > 0.f && 0.f == projectile.fRemainingDistance);

		std::uint32_t subHitIndex = projectile.iSubHitBase;
		std::uint64_t timedBit = 1ull;
		for (std::size_t hitIndex = 0; hitIndex < definition.Hits.size(); ++hitIndex)
		{
			const PLAYER_PROJECTILE_HIT& hit = definition.Hits[hitIndex];
			if (hit.isContact)
			{
				/* Every damageable body inside the shape right now takes the
				next repeat it is owed; a target already at the count is done. */
				for (SERVER_WORLD_ENTITY& entity : worldEntities)
				{
					if (!IsDamageable(entity) ||
						!Hit_ShapeOverlaps(hit.Hit,
							projectile.fPositionX, projectile.fPositionZ,
							projectile.fDirectionX, projectile.fDirectionZ,
							TargetBodyOf(catalog, entity)))
					{
						continue;
					}
					SERVER_PROJECTILE_CONTACT_MARK* mark = nullptr;
					for (SERVER_PROJECTILE_CONTACT_MARK& existing : projectile.ContactMarks)
					{
						if (existing.iNetEntityId == entity.iNetEntityId &&
							existing.iHitIndex == hitIndex)
						{
							mark = &existing;
							break;
						}
					}
					if (nullptr == mark)
					{
						SERVER_PROJECTILE_CONTACT_MARK fresh{};
						fresh.iNetEntityId = entity.iNetEntityId;
						fresh.iHitIndex = static_cast<std::uint8_t>(hitIndex);
						projectile.ContactMarks.push_back(fresh);
						mark = &projectile.ContactMarks.back();
					}
					if (mark->iAppliedCount >= hit.Hit.iRepeatCount ||
						projectile.fElapsedSeconds < mark->fNextSeconds)
					{
						continue;
					}
					ApplyPlayerHitDamage(entity,
						player.iPlayerId, projectile.iSkillId,
						skill->iStaggerDamage, skill->iPartDamage,
						skill->iCounterPower,
						DamageOfSubHit(projectile.iTotalDamage, projectile.iSubHitTotal,
							subHitIndex + mark->iAppliedCount),
						&hit.Hit, projectile.fPositionX, projectile.fPositionZ,
						projectile.fDirectionX, projectile.fDirectionZ,
						serverTick, outDamageEvents);
					++mark->iAppliedCount;
					mark->fNextSeconds = projectile.fElapsedSeconds +
						static_cast<float>(hit.Hit.iRepeatMs) * MILLISECONDS_TO_SECONDS;
				}
				subHitIndex += hit.Hit.iRepeatCount;
				continue;
			}
			/* A timed hit fires once per repeat on its own schedule from spawn,
			on everything the shape covers at that moment; a missile that has
			already stopped keeps firing where it stands until its life ends. */
			for (std::uint32_t repeat = 0; repeat < hit.Hit.iRepeatCount;
				++repeat, ++subHitIndex, timedBit <<= 1)
			{
				if (0u != (projectile.iAppliedTimedMask & timedBit))
					continue;
				const float fireSeconds = static_cast<float>(
					hit.Hit.iTimeMs + hit.Hit.iRepeatMs * repeat) * MILLISECONDS_TO_SECONDS;
				if (projectile.fElapsedSeconds < fireSeconds)
					continue;
				projectile.iAppliedTimedMask |= timedBit;
				std::vector<std::pair<float, SERVER_WORLD_ENTITY*>> targets;
				for (SERVER_WORLD_ENTITY& entity : worldEntities)
				{
					if (!IsDamageable(entity) ||
						!Hit_ShapeOverlaps(hit.Hit,
							projectile.fPositionX, projectile.fPositionZ,
							projectile.fDirectionX, projectile.fDirectionZ,
							TargetBodyOf(catalog, entity)))
					{
						continue;
					}
					const float deltaX = entity.fPositionX - projectile.fPositionX;
					const float deltaZ = entity.fPositionZ - projectile.fPositionZ;
					targets.emplace_back(deltaX * deltaX + deltaZ * deltaZ, &entity);
				}
				std::sort(targets.begin(), targets.end(),
					[](const auto& left, const auto& right)
					{
						return left.first < right.first;
					});
				if (0u != hit.Hit.iMaxTargets && targets.size() > hit.Hit.iMaxTargets)
					targets.resize(hit.Hit.iMaxTargets);
				for (auto& [distanceSquared, target] : targets)
				{
					ApplyPlayerHitDamage(*target,
						player.iPlayerId, projectile.iSkillId,
						skill->iStaggerDamage, skill->iPartDamage,
						skill->iCounterPower,
						DamageOfSubHit(projectile.iTotalDamage, projectile.iSubHitTotal,
							subHitIndex),
						&hit.Hit, projectile.fPositionX, projectile.fPositionZ,
						projectile.fDirectionX, projectile.fDirectionZ,
						serverTick, outDamageEvents);
				}
			}
		}
		if (expired)
			player.Projectiles.erase(player.Projectiles.begin() + index);
		else
			++index;
	}
}

void LostArk::Server::CPlayerSkillSystem::Clamp_StepToWalkable(
	const CServerNavigation& navigation,
	const float startX,
	const float startZ,
	const float desiredX,
	const float desiredZ,
	SERVER_NAV_POINT& outPoint,
	bool& outWasClamped)
{
	outWasClamped = false;
	SERVER_NAV_POINT start{};
	if (!navigation.Sample_Position(startX, startZ, start))
	{
		/* Standing off the grid already; refuse the step rather than snap to a
		cell the player never walked to. */
		outWasClamped = true;
		outPoint = { startX, outPoint.y, startZ };
		return;
	}

	const float deltaX = desiredX - startX;
	const float deltaZ = desiredZ - startZ;
	const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);

	/* Walk the whole segment in sub-cell increments. Testing the destination
	alone is what lets a long dash cross a blocked band and call the open floor
	behind it reachable, so every sample along the way has to answer, and the
	first refusal is where the player stops. */
	float reachableRatio = 0.f;
	float blockedRatio = 1.f;
	bool wasBlocked = false;
	SERVER_NAV_POINT reachable = start;
	const float cellSize = navigation.Get_CellSize();
	constexpr int MAX_MARCH_SAMPLES = 256;
	const int marchSamples = (cellSize > 0.f && distance > 0.f) ?
		(std::min)(MAX_MARCH_SAMPLES,
			static_cast<int>(distance / (cellSize * 0.5f)) + 1) : 1;
	for (int sample = 1; sample <= marchSamples; ++sample)
	{
		const float ratio =
			static_cast<float>(sample) / static_cast<float>(marchSamples);
		const bool isLastSample = sample == marchSamples;
		const float sampleX = isLastSample ? desiredX : startX + deltaX * ratio;
		const float sampleZ = isLastSample ? desiredZ : startZ + deltaZ * ratio;
		SERVER_NAV_POINT sampled{};
		if (!navigation.Resolve_TraversalStep(
			reachable.x,
			reachable.z,
			sampleX,
			sampleZ,
			sampled))
		{
			blockedRatio = ratio;
			wasBlocked = true;
			break;
		}
		reachableRatio = ratio;
		reachable = sampled;
	}

	if (!wasBlocked)
	{
		outPoint = reachable;
		return;
	}

	outWasClamped = true;
	/* Bisect the one bracket that straddles the boundary so the stop lands
	against the wall instead of up to half a cell short of it. */
	constexpr int CLAMP_BISECTION_STEPS = 12;
	for (int step = 0; step < CLAMP_BISECTION_STEPS; ++step)
	{
		const float midRatio = (reachableRatio + blockedRatio) * 0.5f;
		SERVER_NAV_POINT sampled{};
		if (navigation.Resolve_TraversalStep(
			reachable.x,
			reachable.z,
			startX + deltaX * midRatio,
			startZ + deltaZ * midRatio,
			sampled))
		{
			reachableRatio = midRatio;
			reachable = sampled;
		}
		else
		{
			blockedRatio = midRatio;
		}
	}
	outPoint = reachable;
}

void LostArk::Server::CPlayerSkillSystem::Update(
	SERVER_PLAYER& player,
	std::vector<SERVER_WORLD_ENTITY>& worldEntities,
	const CGameplayCatalog& catalog,
	const CServerNavigation* navigation,
	const CServerCollisionSystem* collision,
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
		player.PendingCommand.Clear();
		player.Clear_SkillTarget();
		player.Projectiles.clear();
		return;
	}
	/* A falling player is still alive, so the guard above does not catch them.
	The room owns the descent this tick; nothing here may advance. */
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
		return;
	if (const PLAYER_RUNTIME_PROFILE* identityProfile =
		catalog.Find_Player(player.eCharacterClass))
	{
		Update_Identity(player, *identityProfile);
	}
	Update_Projectiles(player, worldEntities, catalog, fixedDeltaSeconds,
		serverTick, outDamageEvents);
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
		player.PendingCommand.Clear();
		player.Clear_SkillTarget();
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
	const std::uint32_t comboAdvanceMs = hasStage ?
		skill->ComboStages[stageIndex].iComboAdvanceMs : durationMs;
	const float durationSeconds =
		static_cast<float>(durationMs) * MILLISECONDS_TO_SECONDS;
	float stepForward = 0.f;
	float stepLateral = 0.f;
	/* A stage advance resets the action clock, so a staged skill reads the
	running stage's own curve on that same clock. */
	const std::vector<ROOT_MOTION_SAMPLE>& rootMotion = hasStage ?
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
		/* The clip keeps producing displacement after the player runs out of
		floor, so each tick is clamped on its own instead of cancelling the
		action: a later tick whose curve bends back inside moves again. */
		SERVER_NAV_POINT reachable{ nextX, player.fPositionY, nextZ };
		if (nullptr != navigation && navigation->Is_Loaded())
		{
			bool wasClamped = false;
			Clamp_StepToWalkable(
				*navigation,
				player.fPositionX,
				player.fPositionZ,
				nextX,
				nextZ,
				reachable,
				wasClamped);
		}
		float resolvedX = reachable.x;
		float resolvedY = reachable.y;
		float resolvedZ = reachable.z;
		bool wasBlocked = false;
		if (nullptr == collision ||
			collision->Resolve_PlayerMove(
				player,
				reachable.x,
				reachable.y,
				reachable.z,
				resolvedX,
				resolvedY,
				resolvedZ,
				wasBlocked))
		{
			player.fPositionX = resolvedX;
			player.fPositionY = resolvedY;
			player.fPositionZ = resolvedZ;
		}
	}

	const float hitSeconds =
		static_cast<float>(hitMs) * MILLISECONDS_TO_SECONDS;
	const bool holdWithoutDamage = isHold && 3u != player.iComboStage;
	/* The guard stage lands nothing: the damage belongs to the counter it buys. */
	const bool counterWithoutDamage = isCounter && 2u != player.iComboStage;
	const bool dealsDamage = !skill->strDamageProfileId.empty() &&
		!holdWithoutDamage && !counterWithoutDamage;
	const std::vector<PLAYER_SKILL_HIT>& shapeHits = hasStage ?
		skill->ComboStages[stageIndex].Hits : skill->Hits;
	const std::vector<PLAYER_SKILL_PROJECTILE>& projectiles = hasStage ?
		skill->ComboStages[stageIndex].Projectiles : skill->Projectiles;

	const auto targetBodyOf = [&catalog](const SERVER_WORLD_ENTITY& entity)
	{
		return TargetBodyOf(catalog, entity);
	};
	const auto isDamageable = [](const SERVER_WORLD_ENTITY& entity)
	{
		return IsDamageable(entity);
	};
	const auto applyDamage = [&](SERVER_WORLD_ENTITY& target,
		const std::uint32_t rawDamage, const PLAYER_SKILL_HIT* pHit)
	{
		ApplyPlayerHitDamage(target,
			player.iPlayerId, skill->iSkillId,
			skill->iStaggerDamage, skill->iPartDamage, skill->iCounterPower,
			rawDamage, pHit,
			player.fPositionX, player.fPositionZ,
			player.fSkillAimDirectionX, player.fSkillAimDirectionZ,
			serverTick, outDamageEvents);
	};
	const auto resolveRawDamage = [&]()
	{
		const PLAYER_RUNTIME_PROFILE* playerProfile =
			catalog.Find_Player(player.eCharacterClass);
		return CGameplayCatalog::Resolve_Damage(
			nullptr == playerProfile ? 0u : playerProfile->iAttackPower,
			catalog.Find_DamageRatePercent(skill->strDamageProfileId));
	};

	if (dealsDamage && (!shapeHits.empty() || !projectiles.empty()))
	{
		/* The caster's hits and every object's hits split one damage rate:
		caster sub-hits are numbered first, then each projectile's in order. */
		std::uint32_t subHitCount = 0;
		for (const PLAYER_SKILL_HIT& hit : shapeHits)
			subHitCount += hit.iRepeatCount;
		const std::uint32_t casterSubHits = subHitCount;
		for (const PLAYER_SKILL_PROJECTILE& projectile : projectiles)
			subHitCount += ProjectileSubHitCount(projectile);
		const std::uint64_t totalDamage = resolveRawDamage();
		const std::uint32_t subHitTotal = (std::max)(1u, subHitCount);
		const auto damageOfSubHit = [&](const std::uint32_t index)
		{
			return DamageOfSubHit(totalDamage, subHitTotal, index);
		};
		const float elapsedMs = player.fActionElapsedSeconds * 1000.f;
		std::uint32_t projectileSubHitBase = casterSubHits;
		for (std::size_t index = 0; index < projectiles.size(); ++index)
		{
			const PLAYER_SKILL_PROJECTILE& definition = projectiles[index];
			const std::uint16_t bit = static_cast<std::uint16_t>(1u << index);
			const std::uint32_t subHitBase = projectileSubHitBase;
			projectileSubHitBase += ProjectileSubHitCount(definition);
			if (0u != (player.iSpawnedProjectileMask & bit) ||
				elapsedMs < static_cast<float>(definition.iTimeMs))
			{
				continue;
			}
			player.iSpawnedProjectileMask |= bit;
			SERVER_SKILL_PROJECTILE spawned{};
			spawned.iSkillId = skill->iSkillId;
			spawned.iStageIndex = static_cast<std::uint8_t>(hasStage ? stageIndex : 0u);
			spawned.iProjectileIndex = static_cast<std::uint8_t>(index);
			spawned.fDirectionX = player.fSkillAimDirectionX;
			spawned.fDirectionZ = player.fSkillAimDirectionZ;
			spawned.fSpeed = definition.fSpeed;
			spawned.fRemainingSeconds =
				static_cast<float>(definition.iLifeMs) * MILLISECONDS_TO_SECONDS;
			spawned.iTotalDamage = totalDamage;
			spawned.iSubHitTotal = subHitTotal;
			spawned.iSubHitBase = subHitBase;
			spawned.fPositionX = player.fPositionX;
			spawned.fPositionY = player.fPositionY;
			spawned.fPositionZ = player.fPositionZ;
			/* An AIM origin lands where the aim points, no farther than the
			definition allows; a CASTER origin sits on the caster pushed by the
			authored forward/right offsets. A moving object still starts on the
			caster: its aim only sets the direction. */
			if (PLAYER_PROJECTILE_ORIGIN::AIM == definition.eOrigin &&
				PLAYER_PROJECTILE_KIND::FIXAREA == definition.eKind)
			{
				const float distance = definition.fMaxDistance > 0.f ?
					(std::min)(player.fSkillAimDistance, definition.fMaxDistance) :
					player.fSkillAimDistance;
				spawned.fPositionX += spawned.fDirectionX * distance;
				spawned.fPositionZ += spawned.fDirectionZ * distance;
			}
			else
			{
				const float rightX = spawned.fDirectionZ;
				const float rightZ = -spawned.fDirectionX;
				spawned.fPositionX += spawned.fDirectionX * definition.fOffsetForward +
					rightX * definition.fOffsetRight;
				spawned.fPositionZ += spawned.fDirectionZ * definition.fOffsetForward +
					rightZ * definition.fOffsetRight;
			}
			if (PLAYER_PROJECTILE_KIND::FIXAREA == definition.eKind)
			{
				spawned.fRemainingDistance = 0.f;
			}
			else
			{
				/* Flies the aim distance clamped to the definition's band; no
				cap means it only stops when its life runs out. */
				spawned.fRemainingDistance = definition.fMaxDistance > 0.f ?
					(std::min)((std::max)(player.fSkillAimDistance,
						definition.fMinDistance), definition.fMaxDistance) : -1.f;
			}
			player.Projectiles.push_back(std::move(spawned));
		}
		std::uint32_t subHitIndex = 0;
		bool allFired = true;
		for (const PLAYER_SKILL_HIT& hit : shapeHits)
		{
			for (std::uint32_t repeat = 0; repeat < hit.iRepeatCount;
				++repeat, ++subHitIndex)
			{
				const std::uint64_t bit = 1ull << subHitIndex;
				if (0u != (player.iAppliedHitMask & bit))
					continue;
				const float fireMs =
					static_cast<float>(hit.iTimeMs + hit.iRepeatMs * repeat);
				if (elapsedMs < fireMs)
				{
					allFired = false;
					continue;
				}
				player.iAppliedHitMask |= bit;
				const float hitOriginX = player.hasSkillTarget ?
					player.fSkillTargetX : player.fPositionX;
				const float hitOriginZ = player.hasSkillTarget ?
					player.fSkillTargetZ : player.fPositionZ;
				std::vector<std::pair<float, SERVER_WORLD_ENTITY*>> targets;
				for (SERVER_WORLD_ENTITY& entity : worldEntities)
				{
					if (!isDamageable(entity) ||
						!Hit_ShapeOverlaps(hit, hitOriginX, hitOriginZ,
							player.fSkillAimDirectionX, player.fSkillAimDirectionZ,
							targetBodyOf(entity)))
					{
						continue;
					}
					const float deltaX = entity.fPositionX - hitOriginX;
					const float deltaZ = entity.fPositionZ - hitOriginZ;
					targets.emplace_back(deltaX * deltaX + deltaZ * deltaZ, &entity);
				}
				std::sort(targets.begin(), targets.end(),
					[](const auto& left, const auto& right)
					{
						return left.first < right.first;
					});
				if (0u != hit.iMaxTargets && targets.size() > hit.iMaxTargets)
					targets.resize(hit.iMaxTargets);
				for (auto& [distanceSquared, target] : targets)
					applyDamage(*target, damageOfSubHit(subHitIndex), &hit);
			}
		}
		const std::uint16_t expectedProjectileMask = projectiles.empty() ? 0u :
			static_cast<std::uint16_t>((1u << projectiles.size()) - 1u);
		if (allFired &&
			(player.iSpawnedProjectileMask & expectedProjectileMask) ==
				expectedProjectileMask)
			player.hasAppliedSkillDamage = true;
	}
	else if (dealsDamage &&
		!player.hasAppliedSkillDamage && player.fActionElapsedSeconds >= hitSeconds)
	{
		SERVER_WORLD_ENTITY* closestBoss = nullptr;
		float closestDistanceSquared = 0.f;
		const float hitOriginX = player.hasSkillTarget ?
			player.fSkillTargetX : player.fPositionX;
		const float hitOriginZ = player.hasSkillTarget ?
			player.fSkillTargetZ : player.fPositionZ;
		const LostArk::Shared::CombatCollision::CIRCLE_XZ skillCircle{
			hitOriginX,
			hitOriginZ,
			skill->fMaximumRange
		};
		for (SERVER_WORLD_ENTITY& entity : worldEntities)
		{
			if (!isDamageable(entity))
				continue;
			const float deltaX = entity.fPositionX - hitOriginX;
			const float deltaZ = entity.fPositionZ - hitOriginZ;
			const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
			if (LostArk::Shared::CombatCollision::Circles_Overlap(
				skillCircle, targetBodyOf(entity)) &&
				(nullptr == closestBoss || distanceSquared < closestDistanceSquared))
			{
				closestDistanceSquared = distanceSquared;
				closestBoss = &entity;
			}
		}
		if (nullptr != closestBoss)
			applyDamage(*closestBoss, resolveRawDamage(), nullptr);
		player.hasAppliedSkillDamage = true;
	}

	const bool holdLeavesLoop = isHold && 2u == player.iComboStage &&
		player.hasReleasedHold;
	const bool holdSkipsLoop = isHold && 1u == player.iComboStage &&
		player.hasReleasedHold &&
		player.fActionElapsedSeconds >= durationSeconds;

	/* A counter never advances on its own clock: only a hit taken inside the
	guard window promotes it, which Try_Counter does. */
	const bool isCombo = PLAYER_SKILL_KIND::COMBO == skill->eSkillKind;
	const bool hasFollowingStage = hasStage &&
		static_cast<std::size_t>(player.iComboStage) <
			skill->ComboStages.size();
	const bool automaticComboStage = isCombo && hasFollowingStage &&
		IsAutomaticComboStage(skill->ComboStages[stageIndex]);
	const bool hasNextStage = hasStage && !isCounter &&
		(isHold ? static_cast<std::size_t>(player.iComboStage) <
				skill->ComboStages.size()
			: hasFollowingStage &&
				(automaticComboStage || player.hasBufferedComboInput));
	const bool reachedComboBoundary = isCombo &&
		player.fActionElapsedSeconds >=
			static_cast<float>(comboAdvanceMs) * MILLISECONDS_TO_SECONDS;
	const bool stageDamageComplete = !dealsDamage || player.hasAppliedSkillDamage;
	const bool hasPendingExplicit =
		PLAYER_PENDING_COMMAND_KIND::NONE != player.PendingCommand.eKind;
	/* comboAdvanceMs is the COMBO continuation boundary. A manual stage needs a
	buffered press; an automatic stage owns a full-duration boundary. MOVE/SKILL
	still keeps the current animation locked through its authored duration. */
	/* A pending explicit command may replace a manual COMBO at its current full
	stage boundary.  An automatic COMBO is one authored action, however, so keep
	the pending intent across every automatic stage and commit it only after the
	last motion completes. */
	const bool defersPendingThroughAutomaticChain = hasPendingExplicit &&
		automaticComboStage && hasNextStage;
	const bool commitsPendingExplicit = hasPendingExplicit &&
		!defersPendingThroughAutomaticChain && stageDamageComplete &&
		player.fActionElapsedSeconds >= durationSeconds;
	/* hitTimeMs only owns damage. Continuation waits for the authored combo
	boundary, keeping the current stage's presentation/root motion intact. */
	const bool advancesComboStage =
		reachedComboBoundary && stageDamageComplete &&
		hasNextStage &&
		(!hasPendingExplicit || defersPendingThroughAutomaticChain);

	if (advancesComboStage || commitsPendingExplicit || holdLeavesLoop ||
		player.fActionElapsedSeconds >= durationSeconds)
	{
		if (!commitsPendingExplicit && hasNextStage)
		{
			/* The press that bought this stage aimed somewhere, and that is where
			the stage plays: facing and root motion both turn to it. A hold
			advances on its own clock, so it keeps the facing it started with. */
			if (player.hasBufferedComboInput)
			{
				player.fSkillAimDirectionX = player.fBufferedComboAimX;
				player.fSkillAimDirectionZ = player.fBufferedComboAimZ;
				player.fSkillAimDistance = player.fBufferedComboAimDistance;
				player.fYawDegrees = std::atan2(
					player.fBufferedComboAimX,
					player.fBufferedComboAimZ) * RADIANS_TO_DEGREES;
			}
			player.iComboStage = holdSkipsLoop || holdLeavesLoop ?
				3u : player.iComboStage + 1u;
			player.hasBufferedComboInput = false;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			player.iAppliedHitMask = 0;
			player.iSpawnedProjectileMask = 0;
			// The client treats a changed start tick as a new action edge, which
			// is how it learns to play the next stage's clip.
			player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		}
		else
		{
			if (PLAYER_STANCE_ID::NONE != skill->eSetsStance)
			{
				/* A class that spends identity per switch (LanceMaster's spear
				swap) pays here, once, only if enough is already banked. A short
				fall still lets the swap through for free -- see
				Is_HoldingGaugedStance for why this never also drains. */
				if (skill->eSetsStance != player.eStance)
				{
					if (const PLAYER_RUNTIME_PROFILE* stanceProfile =
						catalog.Find_Player(player.eCharacterClass))
					{
						if (0u != stanceProfile->iIdentityStanceSwitchCost &&
							player.iCurrentIdentity >=
								stanceProfile->iIdentityStanceSwitchCost)
						{
							player.iCurrentIdentity -=
								stanceProfile->iIdentityStanceSwitchCost;
						}
					}
				}
				player.eStance = skill->eSetsStance;
			}
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.iActionStartTick = 0;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			player.iAppliedHitMask = 0;
			player.iSpawnedProjectileMask = 0;
			player.iComboStage = 0;
			player.hasBufferedComboInput = false;
			player.hasReleasedHold = false;
			player.Clear_SkillTarget();
			/* A pending explicit command deliberately survives this action reset;
			GameRoom consumes it immediately after Update from the final position. */
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
	player.PendingCommand.Clear();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	// A changed start tick is how the client learns to play the counter clip.
	player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	return true;
}

void LostArk::Server::CPlayerSkillSystem::Arm_PlayerHitReaction(
	SERVER_PLAYER& player,
	const float sourceX,
	const float sourceZ,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction ||
		PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction ||
		player.fKnockbackRemainingSeconds > 0.f ||
		static_cast<std::int32_t>(
			player.iHitReactionGraceEndTick - serverTick) > 0)
	{
		return;
	}
	const bool hasPush =
		0.f != pushRangeM && 0u != pushMs && std::isfinite(pushRangeM);
	if (!hasPush && !knockdown)
		return;
	player.PendingCommand.Clear();
	if (hasPush)
	{
		float directionX = player.fPositionX - sourceX;
		float directionZ = player.fPositionZ - sourceZ;
		const float length = std::sqrt(
			directionX * directionX + directionZ * directionZ);
		if (length > 0.0001f)
		{
			directionX /= length;
			directionZ /= length;
		}
		else
		{
			/* The hit source stands exactly on the player, so away-from-source
			has no direction; falling backwards along the facing reads right. */
			const float yawRadians = player.fYawDegrees / RADIANS_TO_DEGREES;
			directionX = -std::sin(yawRadians);
			directionZ = -std::cos(yawRadians);
		}
		if (pushRangeM < 0.f)
		{
			directionX = -directionX;
			directionZ = -directionZ;
		}
		const float windowSeconds =
			static_cast<float>(pushMs) * MILLISECONDS_TO_SECONDS;
		player.fKnockbackDirectionX = directionX;
		player.fKnockbackDirectionZ = directionZ;
		player.fKnockbackSpeed = std::fabs(pushRangeM) / windowSeconds;
		player.fKnockbackRemainingSeconds = windowSeconds;
	}
	if (knockdown && 0u != downMs)
	{
		player.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.hasReleasedHold = false;
		player.fActionElapsedSeconds = 0.f;
		player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		player.iKnockdownEndTick =
			player.iActionStartTick + MillisecondsToTicks(downMs);
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0;
	}
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
	if (nullptr == skill || PLAYER_SKILL_KIND::HOLD != skill->eSkillKind ||
		!IsNewerSequence(command.iClientSequence, player.iLastSkillSequence))
		return;
	player.iLastSkillSequence = command.iClientSequence;
	player.hasReleasedHold = true;
}

void LostArk::Server::CPlayerSkillSystem::Update_Aim(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_UPDATE_SKILL_AIM& command,
	const CGameplayCatalog& catalog) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::SKILL != player.eAction ||
		player.iCurrentSkillId != command.iSkillId ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ))
	{
		return;
	}
	const PLAYER_SKILL_DEFINITION* skill =
		catalog.Find_Skill(player.iCurrentSkillId);
	if (nullptr == skill || PLAYER_SKILL_KIND::HOLD != skill->eSkillKind)
		return;
	/* Stage 3 is the shot itself, and a released key means the charge is over
	even if the stage jump lands next tick -- both keep the last aim. */
	if (player.iComboStage >= 3u || player.hasReleasedHold)
		return;

	float directionX = 0.f;
	float directionZ = 0.f;
	ResolveAimDirection(
		player, command.fAimX, command.fAimZ, directionX, directionZ);
	player.fSkillAimDirectionX = directionX;
	player.fSkillAimDirectionZ = directionZ;
	player.fSkillAimDistance = AimDistance(player, command.fAimX, command.fAimZ);
	player.fYawDegrees =
		std::atan2(directionX, directionZ) * RADIANS_TO_DEGREES;
}
