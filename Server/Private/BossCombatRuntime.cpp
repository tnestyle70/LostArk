#include "BossCombatRuntime.h"

#include "ServerWorldEntity.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace
{
	std::uint32_t NextRevision(const std::uint32_t revision) noexcept
	{
		return revision == (std::numeric_limits<std::uint32_t>::max)() ?
			1u : revision + 1u;
	}

	std::uint32_t FlagMask(
		const LostArk::Server::SERVER_BOSS_COMBAT_FLAG flag) noexcept
	{
		return static_cast<std::uint32_t>(flag);
	}

	bool ContainsCounterProxySource(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const float sourceX,
		const float sourceZ,
		const bool ignoreForwardDirection) noexcept
	{
		using LostArk::Server::BOSS_PATTERN_COUNTER_PROXY_KIND;
		if (!boss.bPatternHasCounterProxy ||
			!std::isfinite(sourceX) || !std::isfinite(sourceZ) ||
			!std::isfinite(boss.fPositionX) ||
			!std::isfinite(boss.fPositionZ) ||
			!std::isfinite(boss.fYawDegrees))
		{
			return false;
		}
		constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		const float forwardX = std::sin(yawRadians);
		const float forwardZ = std::cos(yawRadians);
		if (BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC ==
			boss.ePatternCounterProxyKind)
		{
			if (!std::isfinite(boss.fPatternCounterProxyArcDegrees) ||
				180.f != boss.fPatternCounterProxyArcDegrees ||
				0.f != boss.fPatternCounterProxyForwardOffsetM ||
				0.f != boss.fPatternCounterProxyRightOffsetM ||
				0.f != boss.fPatternCounterProxyRadiusM)
			{
				return false;
			}
			const float deltaX = sourceX - boss.fPositionX;
			const float deltaZ = sourceZ - boss.fPositionZ;
			return ignoreForwardDirection || deltaX * forwardX + deltaZ * forwardZ >= 0.f;
		}
		if (BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE !=
				boss.ePatternCounterProxyKind ||
			!std::isfinite(boss.fPatternCounterProxyForwardOffsetM) ||
			!std::isfinite(boss.fPatternCounterProxyRightOffsetM) ||
			!std::isfinite(boss.fPatternCounterProxyRadiusM) ||
			0.f != boss.fPatternCounterProxyArcDegrees ||
			boss.fPatternCounterProxyRadiusM <= 0.f)
		{
			return false;
		}
		if (ignoreForwardDirection)
		{
			// Rotate the authored local circle around the boss: preserve its closest
			// and furthest reach while admitting that same reach from every direction.
			const float sourceDistance = std::hypot(sourceX - boss.fPositionX, sourceZ - boss.fPositionZ);
			const float offsetDistance = std::hypot(boss.fPatternCounterProxyForwardOffsetM,
				boss.fPatternCounterProxyRightOffsetM);
			return std::fabs(sourceDistance - offsetDistance) <= boss.fPatternCounterProxyRadiusM;
		}
		const float rightX = forwardZ;
		const float rightZ = -forwardX;
		const float proxyX = boss.fPositionX +
			forwardX * boss.fPatternCounterProxyForwardOffsetM +
			rightX * boss.fPatternCounterProxyRightOffsetM;
		const float proxyZ = boss.fPositionZ +
			forwardZ * boss.fPatternCounterProxyForwardOffsetM +
			rightZ * boss.fPatternCounterProxyRightOffsetM;
		const float deltaX = sourceX - proxyX;
		const float deltaZ = sourceZ - proxyZ;
		const float radiusSquared = boss.fPatternCounterProxyRadiusM *
			boss.fPatternCounterProxyRadiusM;
		return deltaX * deltaX + deltaZ * deltaZ <= radiusSquared;
	}
}

bool LostArk::Server::CBossCombatRuntime::Initialize(
	SERVER_BOSS_COMBAT_STATE& state,
	const std::vector<BOSS_PART_DEFINITION>& definitions,
	std::string& status)
{
	SERVER_BOSS_COMBAT_STATE staged{};
	std::set<std::string> partIds;
	std::uint32_t usedMasks = 0u;
	staged.Parts.reserve(definitions.size());
	for (const BOSS_PART_DEFINITION& definition : definitions)
	{
		if (definition.strPartId.empty() ||
			!partIds.insert(definition.strPartId).second ||
			0u == definition.iStateMask ||
			0u != (definition.iStateMask & (definition.iStateMask - 1u)) ||
			0u != (usedMasks & definition.iStateMask) ||
			0u == definition.iMaximumDurability ||
			definition.iDamageReductionPercent > 100u)
		{
			status = "Boss part runtime definition is invalid: " +
				definition.strPartId;
			return false;
		}
		usedMasks |= definition.iStateMask;
		SERVER_BOSS_PART_STATE part{};
		part.strPartId = definition.strPartId;
		part.iStateMask = definition.iStateMask;
		part.iCurrentDurability = definition.iMaximumDurability;
		part.iMaximumDurability = definition.iMaximumDurability;
		part.iDamageReductionPercent =
			definition.iDamageReductionPercent;
		part.eDamageCondition = definition.eDamageCondition;
		staged.Parts.push_back(std::move(part));
	}
	std::sort(staged.Parts.begin(), staged.Parts.end(),
		[](const SERVER_BOSS_PART_STATE& left,
			const SERVER_BOSS_PART_STATE& right)
		{
			return left.strPartId < right.strPartId;
		});
	staged.iAlivePartMask = usedMasks;
	staged.iStateRevision = 1u;
	state = std::move(staged);
	status = "Boss combat runtime initialized";
	return true;
}

LostArk::Server::BOSS_HIT_RESULT
LostArk::Server::CBossCombatRuntime::Apply_PlayerHit(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_INCOMING_HIT& hit)
{
	BOSS_HIT_RESULT result{};
	result.iAlivePartMask = boss.BossCombat.iAlivePartMask;
	if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind || 0u == boss.iCurrentHp)
		return result;

	SERVER_BOSS_COMBAT_STATE& state = boss.BossCombat;
	bool combatStateChanged = false;
	std::uint32_t reductionPercent = 0u;
	for (const SERVER_BOSS_PART_STATE& part : state.Parts)
	{
		if (0u != (state.iAlivePartMask & part.iStateMask))
		{
			reductionPercent = (std::min)(100u,
				reductionPercent + part.iDamageReductionPercent);
		}
	}
	if (Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE))
	{
		result.bBlockedByInvulnerability = 0u != hit.iRawDamage;
	}
	else if (0u != hit.iRawDamage)
	{
		std::uint32_t resolved = hit.iRawDamage;
		if (!hit.bHealthDamagePreResolved)
		{
			const std::uint64_t retained =
				static_cast<std::uint64_t>(hit.iRawDamage) *
				(100u - reductionPercent) / 100u;
			resolved = static_cast<std::uint32_t>(retained);
			if (0u == resolved && reductionPercent < 100u)
				resolved = 1u;
		}
		if (0u != state.iShieldCurrent)
		{
			result.iShieldDamage =
				(std::min)(resolved, state.iShieldCurrent);
			state.iShieldCurrent -= result.iShieldDamage;
			resolved -= result.iShieldDamage;
			combatStateChanged = 0u != result.iShieldDamage;
			if (0u == state.iShieldCurrent)
			{
				state.iShieldMaximum = 0u;
				state.iFlags &= ~FlagMask(
					SERVER_BOSS_COMBAT_FLAG::SHIELDED);
			}
		}
		result.iHealthDamage = (std::min)(resolved, boss.iCurrentHp);
		boss.iCurrentHp -= result.iHealthDamage;
	}
	if (BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
			boss.ePatternBossResponseKind &&
		!boss.bPatternBossResponsePublished &&
		0u != boss.iPatternBossResponseThreshold &&
		0u != result.iHealthDamage)
	{
		const std::uint32_t previousResponseProgress =
			boss.iPatternBossResponseAccumulatedHealthDamage;
		const std::uint64_t accumulated =
			static_cast<std::uint64_t>(
				boss.iPatternBossResponseAccumulatedHealthDamage) +
			result.iHealthDamage;
		boss.iPatternBossResponseAccumulatedHealthDamage =
			static_cast<std::uint32_t>((std::min)(
				accumulated,
				static_cast<std::uint64_t>(boss.iPatternBossResponseThreshold)));
		combatStateChanged = combatStateChanged ||
			previousResponseProgress !=
				boss.iPatternBossResponseAccumulatedHealthDamage;
		if (boss.iPatternBossResponseAccumulatedHealthDamage >=
			boss.iPatternBossResponseThreshold)
		{
			result.bHealthDamageThresholdReached = Publish_PatternOutcome(
				boss,
				BOSS_PATTERN_STAGE_OUTCOME::HEALTH_DAMAGE_THRESHOLD_REACHED,
				hit.iServerTick);
			boss.bPatternBossResponsePublished =
				result.bHealthDamageThresholdReached;
		}
	}

	/* These authored counter windows accept Q/W/E/R hits and an admitted
	   active COUNTER guard from every side. Other keys and patterns retain
	   their proxy, while range, finite geometry and the counter clock still apply. */
	const bool primarySlotCounter = (hit.bCounterFromPrimarySlot || hit.bCounterFromActiveGuard) &&
		(("VALTAN_TRIPLE_COUNTER" == boss.strPatternId &&
		  ("COUNTER_1" == boss.strPatternStageId || "COUNTER_2" == boss.strPatternStageId ||
		   "COUNTER_3" == boss.strPatternStageId)) ||
		 ("VALTAN_TRASH" == boss.strPatternId &&
		  ("STEP_07" == boss.strPatternStageId || "RETRY_WINDUP_02" == boss.strPatternStageId ||
		   "RETRY_WINDUP_03" == boss.strPatternStageId)));
	if (0u != hit.iCounterPower &&
		(!boss.bPatternHasCounterProxy ||
		 ContainsCounterProxySource(boss, hit.fSourceX, hit.fSourceZ, primarySlotCounter)))
	{
		result.bCounterTriggered = Try_TriggerCounter(boss, hit.iServerTick);
	}
	if (0u != hit.iStaggerDamage && 0u != state.iStaggerMaximum &&
		state.iStaggerCurrent < state.iStaggerMaximum)
	{
		result.iStaggerDamage = (std::min)(hit.iStaggerDamage,
			state.iStaggerMaximum - state.iStaggerCurrent);
		state.iStaggerCurrent += result.iStaggerDamage;
		combatStateChanged = combatStateChanged ||
			0u != result.iStaggerDamage;
		if (state.iStaggerCurrent >= state.iStaggerMaximum)
		{
			result.bStaggerBroken = Publish_PatternOutcome(
				boss, BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN,
				hit.iServerTick);
			/* The stagger gauge is independent from an HP shield. If this
			window also raised one, success removes every shield field in the
			same hit so the persistent snapshot cannot say "flag off, value on". */
			if (0u != state.iShieldCurrent || 0u != state.iShieldMaximum ||
				Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::SHIELDED))
			{
				state.iShieldCurrent = 0u;
				state.iShieldMaximum = 0u;
				state.iFlags &= ~FlagMask(
					SERVER_BOSS_COMBAT_FLAG::SHIELDED);
				combatStateChanged = true;
			}
		}
	}

	std::size_t partBreakEdgeIndex = (std::numeric_limits<std::size_t>::max)();
	if (0u != hit.iPartDamage)
	{
		for (auto& part : state.Parts)
		{
			if (0u == (state.iAlivePartMask & part.iStateMask) ||
				(BOSS_PART_DAMAGE_CONDITION::GROGGY_ONLY == part.eDamageCondition &&
				 !Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::GROGGY))) continue;
			const auto dealt = hit.bDestroyAllEligibleParts ||
				BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE == boss.ePatternPartDamagePolicy ?
				part.iCurrentDurability : (std::min)(hit.iPartDamage, part.iCurrentDurability);
			result.iPartDamage += dealt;
			part.iCurrentDurability -= dealt;
			combatStateChanged = combatStateChanged || dealt != 0u;
			if (0u == part.iCurrentDurability)
			{
				state.iAlivePartMask &= ~part.iStateMask;
				result.iDestroyedPartMask |= part.iStateMask;
				if (result.strDestroyedPartId.empty()) result.strDestroyedPartId = part.strPartId;
			}
			if (!hit.bDestroyAllEligibleParts) break;
		}
		if (result.iDestroyedPartMask)
		{
			result.bPartDestroyed = Publish_PatternOutcome(
				boss, BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED, hit.iServerTick);
			partBreakEdgeIndex = state.PendingPartBreakEdges.size();
			state.PendingPartBreakEdges.push_back({result.iDestroyedPartMask, hit.iServerTick, 0u});
		}
	}
	result.iAlivePartMask = state.iAlivePartMask;
	if (combatStateChanged)
		state.iStateRevision = NextRevision(state.iStateRevision);
	if (partBreakEdgeIndex < state.PendingPartBreakEdges.size())
	{
		state.PendingPartBreakEdges[partBreakEdgeIndex].iStateRevision =
			state.iStateRevision;
	}
	return result;
}

bool LostArk::Server::CBossCombatRuntime::Try_TriggerCounter(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	SERVER_BOSS_COMBAT_STATE& state = boss.BossCombat;
	if (Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) &&
		Publish_PatternOutcome(boss, BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT, serverTick))
	{
		(void)Set_Flag(state, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
		return true;
	}
	for (const auto& retained : boss.KoukuRetainedLogicOwners)
		if (const auto owner = retained.lock(); owner && Try_TriggerCounter(*owner, serverTick))
			return true;
	return false;
}

bool LostArk::Server::CBossCombatRuntime::Publish_PatternOutcome(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_OUTCOME outcome,
	const std::uint32_t serverTick)
{
	if (0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strActionId.empty())
	{
		return false;
	}
	const auto duplicate = std::find_if(
		boss.BossCombat.PendingOutcomes.begin(),
		boss.BossCombat.PendingOutcomes.end(),
		[&boss, outcome](const SERVER_BOSS_PATTERN_OUTCOME_SIGNAL& signal)
		{
			return signal.iPatternSequence == boss.iPatternSequence &&
				signal.strPatternId == boss.strPatternId &&
				signal.strActionId == boss.strActionId &&
				signal.eOutcome == outcome;
		});
	if (boss.BossCombat.PendingOutcomes.end() != duplicate)
		return true;
	SERVER_BOSS_PATTERN_OUTCOME_SIGNAL signal{};
	signal.strPatternId = boss.strPatternId;
	signal.strActionId = boss.strActionId;
	signal.iPatternSequence = boss.iPatternSequence;
	signal.iServerTick = serverTick;
	signal.eOutcome = outcome;
	boss.BossCombat.PendingOutcomes.push_back(std::move(signal));
	return true;
}

bool LostArk::Server::CBossCombatRuntime::Consume_PatternOutcome(
	SERVER_WORLD_ENTITY& boss,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_OUTCOME outcome)
{
	const auto found = std::find_if(
		boss.BossCombat.PendingOutcomes.begin(),
		boss.BossCombat.PendingOutcomes.end(),
		[&boss, &actionId, outcome](
			const SERVER_BOSS_PATTERN_OUTCOME_SIGNAL& signal)
		{
			return signal.iPatternSequence == boss.iPatternSequence &&
				signal.strPatternId == boss.strPatternId &&
				signal.strActionId == actionId &&
				signal.eOutcome == outcome;
		});
	if (boss.BossCombat.PendingOutcomes.end() == found)
		return false;
	boss.BossCombat.PendingOutcomes.erase(found);
	return true;
}

void LostArk::Server::CBossCombatRuntime::Discard_PatternOutcomes(
	SERVER_WORLD_ENTITY& boss,
	const std::string& actionId)
{
	std::erase_if(boss.BossCombat.PendingOutcomes,
		[&boss, &actionId](const SERVER_BOSS_PATTERN_OUTCOME_SIGNAL& signal)
		{
			return signal.iPatternSequence != boss.iPatternSequence ||
				signal.strPatternId != boss.strPatternId ||
				signal.strActionId == actionId;
		});
}

void LostArk::Server::CBossCombatRuntime::Clear_PatternOutcomes(
	SERVER_WORLD_ENTITY& boss)
{
	boss.BossCombat.PendingOutcomes.clear();
}

bool LostArk::Server::CBossCombatRuntime::Has_Flag(
	const SERVER_BOSS_COMBAT_STATE& state,
	const SERVER_BOSS_COMBAT_FLAG flag) noexcept
{
	return 0u != (state.iFlags & FlagMask(flag));
}

bool LostArk::Server::CBossCombatRuntime::Set_Flag(
	SERVER_BOSS_COMBAT_STATE& state,
	const SERVER_BOSS_COMBAT_FLAG flag,
	const bool enabled) noexcept
{
	const std::uint32_t mask = FlagMask(flag);
	const std::uint32_t next = enabled ?
		state.iFlags | mask : state.iFlags & ~mask;
	if (next == state.iFlags)
		return false;
	state.iFlags = next;
	state.iStateRevision = NextRevision(state.iStateRevision);
	return true;
}

bool LostArk::Server::CBossCombatRuntime::Set_StaggerGauge(
	SERVER_BOSS_COMBAT_STATE& state,
	const std::uint32_t maximum) noexcept
{
	if (state.iStaggerCurrent == 0u &&
		state.iStaggerMaximum == maximum)
	{
		return false;
	}
	state.iStaggerCurrent = 0u;
	state.iStaggerMaximum = maximum;
	state.iStateRevision = NextRevision(state.iStateRevision);
	return true;
}

bool LostArk::Server::CBossCombatRuntime::Set_GameplayPhase(
	SERVER_WORLD_ENTITY& boss,
	const std::uint8_t phase) noexcept
{
	if (boss.iPhase == phase)
		return false;
	boss.iPhase = phase;
	/* iPhase is replicated as BOSS_COMBAT_SNAPSHOT::iGameplayPhase, so the
	snapshot the client compares against its own copy has changed. Leaving the
	revision behind makes an honest frame look corrupt and the client drops
	every later frame that carries the same revision. */
	boss.BossCombat.iStateRevision =
		NextRevision(boss.BossCombat.iStateRevision);
	return true;
}

bool LostArk::Server::CBossCombatRuntime::Set_Shield(
	SERVER_BOSS_COMBAT_STATE& state,
	const std::uint32_t maximum) noexcept
{
	const std::uint32_t shieldMask =
		FlagMask(SERVER_BOSS_COMBAT_FLAG::SHIELDED);
	const std::uint32_t nextFlags = 0u == maximum ?
		state.iFlags & ~shieldMask : state.iFlags | shieldMask;
	if (state.iShieldCurrent == maximum &&
		state.iShieldMaximum == maximum && state.iFlags == nextFlags)
	{
		return false;
	}
	state.iShieldCurrent = maximum;
	state.iShieldMaximum = maximum;
	state.iFlags = nextFlags;
	state.iStateRevision = NextRevision(state.iStateRevision);
	return true;
}
