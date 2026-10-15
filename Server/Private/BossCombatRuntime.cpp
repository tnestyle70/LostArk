#include "BossCombatRuntime.h"

#include "ServerWorldEntity.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace
{
	// 전투 표시 상태가 바뀔 때 사용할 다음 revision이다. 0을 예약하고 최댓값 다음은 1로 순환한다.
	std::uint32_t NextRevision(const std::uint32_t revision) noexcept
	{
		return revision == (std::numeric_limits<std::uint32_t>::max)() ?
			1u : revision + 1u;
	}

	// enum flag를 비트 연산용 정수 mask로 바꾼다.
	std::uint32_t FlagMask(
		const LostArk::Server::SERVER_BOSS_COMBAT_FLAG flag) noexcept
	{
		return static_cast<std::uint32_t>(flag);
	}

	// Apply_PlayerHit이 카운터 입력 원점의 공간 허용 여부를 검사한다. 입력은 월드 XZ m와 degree다.
	// 전방 반원 또는 보스 지역 원만 처리하고 잘못된 수치/정의는 거부한다. boss는 변경하지 않는다.
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
			// 저작 local 원의 offset 거리와 반경을 유지하며 보스 둘레 모든 방향의 같은 거리대를 허용한다.
			// 가장 가깝거나 먼 도달 거리는 바꾸지 않고 방향 조건만 없애는 검사다.
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

// GameRoom의 entity 생성이 호출한다. ID 중복/단일 비트 mask/내구도/감소율을 검증한다.
// 모든 정의가 통과하면 part ID 순으로 정렬한 임시 상태를 이동 대입한다. 검증 실패는 기존 state를 보존한다.
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

// CServerCombatHitRuntime과 Brain의 counter proxy가 호출하는 보스 전투 입력 처리다.
// 입력의 독립 피해 채널을 순서대로 처리하며 결과 신호는 PendingOutcomes에 남겨 Brain이 분기한다.
// HP는 boss에서 바로 줄고 보호막/무력화/부위 및 누적 응답의 표시 변화는 전투 revision에 반영한다.
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
	// 무적은 이 함수의 HP/보호막 피해를 막는다. 아래의 카운터/무력화/부위 채널까지 일괄 return하지는 않는다.
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

	// 명시된 패턴/stage의 주 슬롯 또는 승인된 COUNTER guard는 방향 제한을 완화한다.
	// 다른 입력은 기존 proxy를 사용하며 유효 수치/거리/카운터 창 조건은 계속 적용한다.
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
			// 무력화 게이지와 HP 보호막은 독립 상태다.
			// 무력화를 끝낸 타격은 현재 보호막의 값과 플래그를 함께 지워 서로 어긋난 snapshot을 막는다.
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

	// 부위는 Initialize에서 ID 순으로 정렬했다. 일반 입력은 처음 허용된 부위 하나만 처리한다.
	// 전체 파괴 정책이면 허용된 부위를 모두 순회하고 파괴 표시 event에는 합친 mask를 넣는다.
	std::size_t partBreakEdgeIndex = (std::numeric_limits<std::size_t>::max)();
	if (0u != hit.iPartDamage)
	{
		for (auto& part : state.Parts)
		{
			if (0u == (state.iAlivePartMask & part.iStateMask) ||
				(BOSS_PART_DAMAGE_CONDITION::GROGGY_ONLY == part.eDamageCondition &&
				 !Has_Flag(state, SERVER_BOSS_COMBAT_FLAG::GROGGY)))
				continue;
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
				if (result.strDestroyedPartId.empty())
					result.strDestroyedPartId = part.strPartId;
			}
			if (!hit.bDestroyAllEligibleParts)
				break;
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

// 유효한 현재 창의 결과 게시에 성공하면 COUNTERABLE을 해제한다.
// 현재 entity에서 처리하지 못하면 GameRoom이 유지하는 Kouku 이전 logic 소유자들을 lock하여 조회한다.
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

// 피격이나 외부 충돌이 현재 action의 결과를 게시한다. 같은 실행 identity/outcome은 한 번만 추가한다.
// 이미 대기 중인 중복도 true이며, pattern sequence/ID/action이 없는 경우는 false다.
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

// Brain이 요청한 현재 pattern/sequence의 action 결과 한 개를 꺼낸다.
// 찾지 못하면 대기열을 바꾸지 않고 false다. 찾으면 지워서 같은 신호가 다시 분기하지 않게 한다.
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

// stage 진입이 지난 실행 신호와 종료한 action의 신호를 정리한다.
// 조건은 OR이므로 지정 action만 지우는 함수로 해석하지 않는다.
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

// 패턴 종료와 Kouku logic 정리가 결과 신호만 비운다. PendingPartBreakEdges는 여기서 지우지 않는다.
void LostArk::Server::CBossCombatRuntime::Clear_PatternOutcomes(
	SERVER_WORLD_ENTITY& boss)
{
	boss.BossCombat.PendingOutcomes.clear();
}

// 전투/단계 경로의 공통 비트 조회다. mask에 지정된 비트 중 하나라도 켜져 있으면 true다.
bool LostArk::Server::CBossCombatRuntime::Has_Flag(
	const SERVER_BOSS_COMBAT_STATE& state,
	const SERVER_BOSS_COMBAT_FLAG flag) noexcept
{
	return 0u != (state.iFlags & FlagMask(flag));
}

// stage action 등 호출자가 지정한 비트를 갱신한다. 같은 값이면 false로 끝내고 revision을 유지한다.
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

// stage action이 새 무력화 창을 열거나 닫는다. maximum 변경 여부와 무관하게 누적값을 0으로 맞춘다.
// 이미 같은 최대값/누적 0이면 아무것도 바꾸지 않는다.
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

// Brain/Debug/stage action의 phase 변경이 snapshot revision을 함께 진행하도록 묶는다.
bool LostArk::Server::CBossCombatRuntime::Set_GameplayPhase(
	SERVER_WORLD_ENTITY& boss,
	const std::uint8_t phase) noexcept
{
	if (boss.iPhase == phase)
		return false;
	boss.iPhase = phase;
	// iPhase도 보스 전투 snapshot에 포함되므로 phase 변경과 같은 경계에서 revision을 진행한다.
	// Client가 같은 revision의 다른 내용을 받지 않도록 한다.
	boss.BossCombat.iStateRevision =
		NextRevision(boss.BossCombat.iStateRevision);
	return true;
}

// stage action이 보호막 현재값/최대값과 SHIELDED 비트를 함께 갱신한다.
// 새 maximum으로 가득 채우며 maximum 0은 닫기다. 모두 같은 상태이면 false다.
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
