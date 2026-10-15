#include "ValtanBrain.h"

#include "BossCombatRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/EstherStrikeContract.h"
#include "Gameplay/KoukuTargetTracking.h"
#include "Gameplay/WorldCollisionContract.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"
#include "ServerCombatGeometry.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
	using namespace LostArk::Server;

	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;
	constexpr std::uint8_t MAX_PATTERN_FOLLOWUP_DEPTH = 32u;
	constexpr float PATH_POINT_STOP_DISTANCE = 0.1f;
	// 도약은 Server가 위치 곡선으로 진행하며 도약 중에는 클립 루트모션을 중복 적용하지 않는다.
	// 높이와 착지점은 게시된 pattern.Motion에서 받아 사용한다.

	// 패턴 이름 대신 게시된 이동 종류로 도약 여부를 판정한다.
	// 첫 stage에서 상승하고, 지정된 이동 stage에서 하강하며 그 사이에는 정점을 유지한다.
	bool Is_LeapPattern(const BOSS_PATTERN_DEFINITION& pattern)
	{
		return BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind ||
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == pattern.Motion.eKind;
	}

	// Configure_PortalMotion이 현재 stage에 포탈 전용 이동 상태를 열지 결정하는 분류다.
	bool Is_PortalMotion(const BOSS_PATTERN_STAGE_MOTION_KIND kind)
	{
		return BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH == kind ||
			BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA == kind;
	}

	constexpr std::uint32_t LEAP_TAKEOFF_STAGE_INDEX = 0u;

	// EnterPatternStage가 다음 평가 tick을 예약한다. 미설정을 뜻하는 0은 반환하지 않는다.
	std::uint32_t NextServerTickSkippingReservedZero(
		const std::uint32_t serverTick)
	{
		return (std::numeric_limits<std::uint32_t>::max)() == serverTick ?
			1u : serverTick + 1u;
	}

	// 표시되는 누적 피해 진행도가 바뀔 때 전투 snapshot revision을 진행시킨다. 0은 예약한다.
	std::uint32_t NextBossCombatRevision(
		const std::uint32_t revision) noexcept
	{
		return (std::numeric_limits<std::uint32_t>::max)() == revision ?
			1u : revision + 1u;
	}

	// stage 진입/패턴 시작/종료가 호출하는 누적 피해 응답 상태의 공통 갱신 경계다.
	// 임계값까지 잘라 표시한 진행도나 임계값 자체가 달라질 때만 BossCombat revision을 바꾼다.
	void SetPatternBossResponseState(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_BOSS_RESPONSE_KIND kind,
		const std::uint32_t threshold,
		const std::uint32_t accumulatedHealthDamage,
		const bool published)
	{
		const bool previousActive =
			BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
				boss.ePatternBossResponseKind;
		const std::uint32_t previousThreshold = previousActive ?
			boss.iPatternBossResponseThreshold : 0u;
		const std::uint32_t previousProgress = previousActive ?
			(std::min)(boss.iPatternBossResponseAccumulatedHealthDamage,
				previousThreshold) : 0u;

		boss.ePatternBossResponseKind = kind;
		boss.iPatternBossResponseThreshold = threshold;
		boss.iPatternBossResponseAccumulatedHealthDamage =
			accumulatedHealthDamage;
		boss.bPatternBossResponsePublished = published;

		const bool currentActive =
			BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE == kind;
		const std::uint32_t currentThreshold = currentActive ? threshold : 0u;
		const std::uint32_t currentProgress = currentActive ?
			(std::min)(accumulatedHealthDamage, currentThreshold) : 0u;
		if (previousProgress != currentProgress ||
			previousThreshold != currentThreshold)
		{
			boss.BossCombat.iStateRevision = NextBossCombatRevision(
				boss.BossCombat.iStateRevision);
		}
	}

	// Update와 조준 갱신이 공유하는 stage 평가 횟수다.
	// 첫 평가 tick부터 현재 tick까지를 세며, 시작 직후 평가 포함 여부는 EnterPatternStage가 정한다.
	std::uint64_t StageElapsedTicks(
		const SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick)
	{
		const std::uint32_t firstTick =
			boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		// 0은 예약값이므로 UINT32_MAX 다음은 1이다.
		// 이 순환 규칙으로 지난 tick 수를 세고 첫 평가 tick도 포함한다.
		const std::uint64_t ageTicks = serverTick >= firstTick ?
			static_cast<std::uint64_t>(serverTick - firstTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - firstTick) +
				static_cast<std::uint64_t>(serverTick);
		return ageTicks + 1u;
	}

	// 30Hz tick과 저작 ms를 각각 1000 및 30배 해 같은 단위의 정수로 비교한다.
	// 예약 판정/마감의 경계에서 소수점 누적 오차를 피하고 곱셈은 64bit로 수행한다.
	bool HasElapsedMilliseconds(
		const std::uint64_t elapsedTicks,
		const std::uint32_t milliseconds)
	{
		return static_cast<std::uint64_t>(elapsedTicks) *
			MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(milliseconds) * SERVER_TICK_HZ;
	}

	// Advance_ArenaBreakLeap이 직접 위치를 갱신할 이륙/이동 stage인지 검사한다.
	bool Is_LeapArcStage(const SERVER_WORLD_ENTITY& boss)
	{
		return boss.fPatternLeapApexHeight > 0.f &&
			(LEAP_TAKEOFF_STAGE_INDEX == boss.iPatternStageIndex ||
				boss.iPatternLeapTravelStageIndex == boss.iPatternStageIndex);
	}

	// 저작한 부분 시간 구간을 0~1 진행률로 바꾼다.
	// 잘못된 구간이나 비유한 경과 시간은 끝값으로 처리하여 나눗셈을 막는다.
	float StageWindowRatio(
		const SERVER_WORLD_ENTITY& boss,
		const std::uint32_t startMs,
		const std::uint32_t endMs)
	{
		if (startMs >= endMs)
			return 1.f;
		if (!std::isfinite(boss.fActionElapsedSeconds))
			return 1.f;
		const float elapsedMs = (std::max)(0.f,
			boss.fActionElapsedSeconds * MILLISECONDS_PER_SECOND);
		return (std::min)(1.f, (std::max)(0.f,
			(elapsedMs - static_cast<float>(startMs)) /
			static_cast<float>(endMs - startMs)));
	}

	// 이륙 stage에서는 제자리에서 상승하고, 이동 stage에서는 착지점으로 가며 내려온다.
	// X/Z는 선형 보간하고 Y는 정점에서 착지점까지 제곱 진행률로 보간한다.
	void Advance_ArenaBreakLeap(SERVER_WORLD_ENTITY& boss)
	{
		if (!Is_LeapArcStage(boss))
			return;
		const float apex = boss.fLeapApexHeight;
		if (LEAP_TAKEOFF_STAGE_INDEX == boss.iPatternStageIndex)
		{
			if (boss.bPatternMoveToAnchorBeforeTakeoff)
			{
				const float elapsedMs = boss.fActionElapsedSeconds * 1000.f;
				const float moveRatio = std::clamp(elapsedMs /
					static_cast<float>(boss.iPatternLeapTakeoffStartMs), 0.f, 1.f);
				boss.fPositionX = boss.fLeapOriginX +
					(boss.fLeapLandingX - boss.fLeapOriginX) * moveRatio;
				boss.fPositionY = boss.fLeapOriginY +
					(boss.fLeapLandingY - boss.fLeapOriginY) * moveRatio;
				boss.fPositionZ = boss.fLeapOriginZ +
					(boss.fLeapLandingZ - boss.fLeapOriginZ) * moveRatio;
				if (moveRatio < 1.f)
					return;
				boss.fLeapOriginX = boss.fLeapLandingX;
				boss.fLeapOriginY = boss.fLeapLandingY;
				boss.fLeapOriginZ = boss.fLeapLandingZ;
			}
			const float ratio = StageWindowRatio(
				boss, boss.iPatternLeapTakeoffStartMs,
				boss.iPatternLeapTakeoffEndMs);
			boss.fPositionX = boss.fLeapOriginX;
			boss.fPositionZ = boss.fLeapOriginZ;
			boss.fPositionY = boss.fLeapOriginY + apex * ratio;
			return;
		}
		const float ratio = StageWindowRatio(
			boss, boss.iPatternLeapTravelStartMs,
			boss.iPatternLeapTravelEndMs);
		boss.fPositionX = boss.fLeapOriginX +
			(boss.fLeapLandingX - boss.fLeapOriginX) * ratio;
		boss.fPositionZ = boss.fLeapOriginZ +
			(boss.fLeapLandingZ - boss.fLeapOriginZ) * ratio;
		const float apexY = boss.fLeapOriginY + apex;
		boss.fPositionY = apexY + (boss.fLeapLandingY - apexY) * ratio * ratio;
	}

	// 대기/추적/사망 action이 달라질 때만 경과 시간을 초기화한다.
	// 같은 action으로의 반복 요청은 시작 tick을 다시 쓰지 않는다.
	void Transition(
		SERVER_WORLD_ENTITY& boss,
		const SERVER_ENTITY_ACTION action,
		const std::uint32_t serverTick)
	{
		if (boss.eAction == action)
			return;
		boss.eAction = action;
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	}

	// FailMechanic이 재예약 전에 대기열 안의 동일 stable pattern ID를 검사한다.
	bool ContainsPatternId(
		const std::vector<std::string>& ids,
		const std::string& patternId)
	{
		return ids.end() != std::find(ids.begin(), ids.end(), patternId);
	}

	// 기믹 상태 전이와 선택 기록이 boss 소유의 실행 기록을 pattern ID로 찾는다.
	// 반환 포인터는 해당 vector 요소를 차용하며 vector 재배치 이후까지 보관하면 안 된다.
	SERVER_BOSS_MECHANIC_OCCURRENCE* FindMechanicOccurrence(
		SERVER_WORLD_ENTITY& boss,
		const std::string& patternId)
	{
		const auto found = std::find_if(
			boss.MechanicOccurrences.begin(), boss.MechanicOccurrences.end(),
			[&patternId](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return occurrence.strPatternId == patternId;
			});
		return boss.MechanicOccurrences.end() == found ? nullptr : &*found;
	}

	// 기믹 상태 전이와 선택 기록이 boss 소유의 실행 기록을 pattern ID로 찾는다.
	// 반환 포인터는 해당 vector 요소를 차용하며 vector 재배치 이후까지 보관하면 안 된다.
	const SERVER_BOSS_MECHANIC_OCCURRENCE* FindMechanicOccurrence(
		const SERVER_WORLD_ENTITY& boss,
		const std::string& patternId)
	{
		const auto found = std::find_if(
			boss.MechanicOccurrences.begin(), boss.MechanicOccurrences.end(),
			[&patternId](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return occurrence.strPatternId == patternId;
			});
		return boss.MechanicOccurrences.end() == found ? nullptr : &*found;
	}

	// BeginPattern이 pattern sequence를 발급한 뒤 QUEUED 기록을 ACTIVE로 전환한다.
	// 해당 기록이 없거나 이미 다른 상태이면 변경하지 않는다.
	void MarkMechanicActive(
		SERVER_WORLD_ENTITY& boss,
		const std::string& patternId,
		const std::uint32_t serverTick)
	{
		SERVER_BOSS_MECHANIC_OCCURRENCE* occurrence =
			FindMechanicOccurrence(boss, patternId);
		if (nullptr == occurrence ||
			SERVER_BOSS_MECHANIC_STATE::QUEUED != occurrence->eState)
		{
			return;
		}
		occurrence->eState = SERVER_BOSS_MECHANIC_STATE::ACTIVE;
		occurrence->iStartedTick = serverTick;
		occurrence->iPatternSequence = boss.iPatternSequence;
	}

	// FinishPattern이 정상 종료한 현재 sequence와 일치하는 ACTIVE 기록만 완료한다.
	// 이전 실행이나 다른 sequence의 기록을 완료 처리하지 않는다.
	void CompleteActiveMechanic(
		SERVER_WORLD_ENTITY& boss,
		const std::string& patternId,
		const std::uint32_t serverTick)
	{
		SERVER_BOSS_MECHANIC_OCCURRENCE* occurrence =
			FindMechanicOccurrence(boss, patternId);
		if (nullptr == occurrence ||
			SERVER_BOSS_MECHANIC_STATE::ACTIVE != occurrence->eState ||
			occurrence->iPatternSequence != boss.iPatternSequence)
		{
			return;
		}
		occurrence->eState = SERVER_BOSS_MECHANIC_STATE::COMPLETED;
		occurrence->eFailure = SERVER_BOSS_MECHANIC_FAILURE::NONE;
		occurrence->iFinishedTick = serverTick;
	}

	// 패턴 취소와 외부 실패 통지가 공유하는 기믹 기록 갱신이다.
	// 대상 부재는 재예약하고, 그 밖의 기록된 실패는 reset이 필요한 상태로 남긴다.
	void FailMechanic(
		SERVER_WORLD_ENTITY& boss,
		const std::string& patternId,
		const SERVER_BOSS_MECHANIC_FAILURE failure,
		const std::uint32_t serverTick)
	{
		SERVER_BOSS_MECHANIC_OCCURRENCE* occurrence =
			FindMechanicOccurrence(boss, patternId);
		if (SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET == failure)
		{
			// 대상이 일시적으로 없어진 경우에는 기믹 정의 손상으로 기록하지 않는다.
			// 소유 중인 체력 기믹은 대기열 맨 앞에 다시 넣고, 순서형 커서는 FinishPattern이 보존한다.
			// 이후 대상이 다시 허용되면 같은 실행 항목을 재시도한다.
			if (nullptr != occurrence &&
				(SERVER_BOSS_MECHANIC_STATE::ACTIVE == occurrence->eState ||
				 SERVER_BOSS_MECHANIC_STATE::QUEUED == occurrence->eState))
			{
				occurrence->eState = SERVER_BOSS_MECHANIC_STATE::QUEUED;
				occurrence->eFailure = SERVER_BOSS_MECHANIC_FAILURE::NONE;
				occurrence->iStartedTick = 0u;
				occurrence->iFinishedTick = 0u;
				occurrence->iPatternSequence = 0u;
				if (!ContainsPatternId(boss.PendingPatternIds, patternId))
					boss.PendingPatternIds.insert(
						boss.PendingPatternIds.begin(), patternId);
			}
			return;
		}
		if (nullptr == occurrence ||
			SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET == occurrence->eState)
		{
			return;
		}
		occurrence->eState =
			SERVER_BOSS_MECHANIC_STATE::FAILED_REQUIRES_RESET;
		occurrence->eFailure = failure;
		occurrence->iFinishedTick = serverTick;
		boss.bMechanicLedgerRequiresReset = true;
	}

	// 패턴 선택과 대상 부재 판단이 같은 참가 조건을 사용한다.
	// 일시적으로 전투 불가능한 플레이어 때문에 인접 tick마다 대기와 공격이 엇갈리는 것을 막는다.
	bool IsEngageablePlayer(const SERVER_PLAYER& player)
	{
		return player.Is_Human() && 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction;
	}

	// Update와 조준 갱신이 사용할 가장 가까운 참가 가능 플레이어를 XZ 거리 제곱으로 찾는다.
	// players의 요소를 차용하며 대상이 없으면 nullptr, 요청된 거리 출력에는 최대 float를 남긴다.
	SERVER_PLAYER* SelectNearestEngageableTarget(
		const SERVER_WORLD_ENTITY& boss,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		float* outDistanceSquared = nullptr)
	{
		float nearestDistanceSquared = (std::numeric_limits<float>::max)();
		SERVER_PLAYER* nearest = nullptr;
		if (std::isfinite(boss.fPositionX) && std::isfinite(boss.fPositionZ))
		{
			for (auto& [playerId, player] : players)
			{
				(void)playerId;
				if (!IsEngageablePlayer(player) ||
					!std::isfinite(player.fPositionX) ||
					!std::isfinite(player.fPositionZ))
				{
					continue;
				}
				const float deltaX = player.fPositionX - boss.fPositionX;
				const float deltaZ = player.fPositionZ - boss.fPositionZ;
				const float distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
				if (distanceSquared < nearestDistanceSquared)
				{
					nearest = &player;
					nearestDistanceSquared = distanceSquared;
				}
			}
		}
		if (nullptr != outDistanceSquared)
			*outDistanceSquared = nearestDistanceSquared;
		return nearest;
	}

	// 같은 원본 주 action ID를 공유하면 재사용 대기를 함께 적용한다.
	// 원본 ID가 없는 정의끼리는 stable pattern ID로 구분한다.
	bool IsSamePatternCooldownFamily(
		const SERVER_BOSS_PATTERN_COOLDOWN& cooldown,
		const BOSS_PATTERN_DEFINITION& pattern)
	{
		if (0u != pattern.iSourcePrimaryActionId)
		{
			return cooldown.iSourcePrimaryActionId ==
				pattern.iSourcePrimaryActionId;
		}
		return 0u == cooldown.iSourcePrimaryActionId &&
			cooldown.strPatternId == pattern.strPatternId;
	}

	// 일반 선택의 후보 검사에서 남은 재사용 대기 tick을 읽는다. 없거나 준비 시각을 지났으면 0이다.
	std::uint32_t PatternCooldownRemainingTicks(
		const SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t serverTick)
	{
		const auto found = std::find_if(
			boss.PatternCooldowns.begin(), boss.PatternCooldowns.end(),
			[&pattern](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{
				return IsSamePatternCooldownFamily(cooldown, pattern);
			});
		if (boss.PatternCooldowns.end() == found ||
			static_cast<std::int32_t>(serverTick - found->iReadyTick) >= 0)
		{
			return 0u;
		}
		return found->iReadyTick - serverTick;
	}

	// BeginPattern이 실제 실행을 시작할 때 해당 family의 준비 tick을 추가하거나 갱신한다.
	void StartPatternCooldown(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t serverTick)
	{
		if (0u == pattern.iSourceCooldownTicks)
			return;
		const std::uint32_t readyTick =
			serverTick + pattern.iSourceCooldownTicks;
		auto found = std::find_if(
			boss.PatternCooldowns.begin(), boss.PatternCooldowns.end(),
			[&pattern](const SERVER_BOSS_PATTERN_COOLDOWN& cooldown)
			{
				return IsSamePatternCooldownFamily(cooldown, pattern);
			});
		if (boss.PatternCooldowns.end() == found)
		{
			SERVER_BOSS_PATTERN_COOLDOWN cooldown{};
			cooldown.iSourcePrimaryActionId = pattern.iSourcePrimaryActionId;
			if (0u == pattern.iSourcePrimaryActionId)
				cooldown.strPatternId = pattern.strPatternId;
			cooldown.iReadyTick = readyTick;
			boss.PatternCooldowns.push_back(std::move(cooldown));
			return;
		}
		found->iReadyTick = readyTick;
	}

	// 선택/분기/현재 실행 검사가 Catalog에서 받은 정의 목록을 stable pattern ID로 조회한다.
	// 반환 포인터는 전달받은 정의 vector의 수명 안에서만 유효하다.
	const BOSS_PATTERN_DEFINITION* FindPattern(
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::string& patternId)
	{
		const auto found = std::find_if(patterns.begin(), patterns.end(),
			[&patternId](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == patternId;
			});
		return patterns.end() == found ? nullptr : &*found;
	}

	// Update에서 대상이 없어도 이미 시작한 아레나 무작위 연속 생성을 이어갈지 검사한다.
	// 현재 action과 stage가 일치하고 ENTER의 다중 volley가 예약된 경우에만 허용한다.
	bool CanContinueTargetlessScheduledArenaStage(
		const SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns)
	{
		if (SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
			SERVER_ENTITY_ACTION::CHASE == boss.eAction ||
			0u == boss.iActionStartTick ||
			0u == boss.iAppliedPatternStageSpawnWaveCount)
		{
			return false;
		}
		const BOSS_PATTERN_DEFINITION* pattern =
			FindPattern(patterns, boss.strPatternId);
		if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size())
			return false;
		const BOSS_PATTERN_STAGE_DEFINITION& stage =
			pattern->Stages[boss.iPatternStageIndex];
		if (stage.strActionId != boss.strActionId)
			return false;
		return stage.Actions.end() != std::find_if(
			stage.Actions.begin(), stage.Actions.end(),
			[](const BOSS_PATTERN_STAGE_ACTION& action)
			{
				return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == action.eTrigger &&
					BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
						action.eKind &&
					action.Volley.iSpawnCount > 1u &&
					action.Volley.iArenaRandomCount > 0u;
			});
	}

	// 잡기와 해제 action을 함께 가진 패턴인지 검사한다. 잡은 대상을 풀기 전 중단하지 않기 위한 분류다.
	bool PatternOwnsGrabLifecycle(
		const BOSS_PATTERN_DEFINITION& pattern)
	{
		bool hasCapture = false;
		bool hasRelease = false;
		for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
		{
			hasCapture = hasCapture ||
				BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
					stage.ePlayerResponse;
			hasRelease = hasRelease ||
				stage.Actions.end() != std::find_if(
					stage.Actions.begin(), stage.Actions.end(),
					[](const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_KIND::
							RELEASE_GRABBED_PLAYERS == action.eKind;
					});
		}
		return hasCapture && hasRelease;
	}

	// Update가 잡기 패턴을 대상 부재로 취소하기 전에 호출한다.
	// 해제 stage, 이미 확정된 잡기 실행 또는 현재 sequence가 소유한 잡힌 플레이어가 있으면 계속한다.
	bool CanContinueTargetlessOwnedGrabPattern(
		const SERVER_WORLD_ENTITY& boss,
		const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns)
	{
		using namespace LostArk::Shared;
		if (players.empty() || boss.strPatternId.empty() ||
			SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
			SERVER_ENTITY_ACTION::CHASE == boss.eAction)
		{
			return false;
		}
		const BOSS_PATTERN_DEFINITION* pattern =
			FindPattern(patterns, boss.strPatternId);
		if (nullptr == pattern || !PatternOwnsGrabLifecycle(*pattern))
			return false;
		if (boss.iPatternStageIndex < pattern->Stages.size())
		{
			const BOSS_PATTERN_STAGE_DEFINITION& stage =
				pattern->Stages[boss.iPatternStageIndex];
			const bool ownsReleaseStage = stage.strActionId == boss.strActionId &&
				stage.Actions.end() != std::find_if(
					stage.Actions.begin(), stage.Actions.end(),
					[](const BOSS_PATTERN_STAGE_ACTION& action)
					{
						return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
							action.eTrigger &&
							BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS ==
							action.eKind;
					});
			if (ownsReleaseStage)
				return true;
		}
		if (boss.iGrabExecutionCommittedPatternSequence == boss.iPatternSequence &&
			0u != boss.iPatternSequence &&
			boss.iGrabExecutionCommittedStageIndex == boss.iPatternStageIndex)
		{
			return true;
		}
		return 0u != CValtanBrain::Classify_GrabbedPlayers(
			boss, players).iGrabbedCount;
	}

	// SET_PLAYER_BIND의 ENTER와 EXIT를 모두 가진 정의만 묶기 생명주기를 소유한 것으로 본다.
	bool PatternOwnsBindLifecycle(
		const BOSS_PATTERN_DEFINITION& pattern)
	{
		bool hasEnter = false;
		bool hasExit = false;
		for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
		{
			for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
			{
				if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND !=
					action.eKind)
				{
					continue;
				}
				hasEnter = hasEnter ||
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == action.eTrigger;
				hasExit = hasExit ||
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT == action.eTrigger;
			}
		}
		return hasEnter && hasExit;
	}

	// 살아 있는 플레이어의 묶기 owner와 sequence가 이 보스의 현재 실행과 일치하는지 검사한다.
	// 묶어서 일반 대상이 없어졌다는 이유로 해제까지의 패턴을 취소하지 않기 위한 조건이다.
	bool CanContinueTargetlessOwnedBindPattern(
		const SERVER_WORLD_ENTITY& boss,
		const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns)
	{
		using namespace LostArk::Shared;
		if (players.empty() || boss.strPatternId.empty() ||
			SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
			SERVER_ENTITY_ACTION::CHASE == boss.eAction)
		{
			return false;
		}
		const BOSS_PATTERN_DEFINITION* pattern =
			FindPattern(patterns, boss.strPatternId);
		if (nullptr == pattern || !PatternOwnsBindLifecycle(*pattern))
			return false;
		return players.end() != std::find_if(
			players.begin(), players.end(),
			[&boss](const auto& entry)
			{
				const SERVER_PLAYER& player = entry.second;
				return player.bPatternBound && 0u != player.iCurrentHp &&
					PLAYER_ACTION_STATE::DEAD != player.eAction &&
					PLAYER_ACTION_STATE::FALLING != player.eAction &&
					player.iPatternBindOwnerNetEntityId == boss.iNetEntityId &&
					player.iPatternBindSequence == boss.iPatternSequence;
			});
	}

	// ApplyStageBranch가 같은 패턴 안의 다음 action을 stage와 index로 해석한다.
	// 못 찾으면 nullptr을 반환하며 outStageIndex는 갱신하지 않는다.
	const BOSS_PATTERN_STAGE_DEFINITION* FindStageByActionId(
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::string& actionId,
		std::uint32_t& outStageIndex)
	{
		for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
		{
			if (pattern.Stages[index].strActionId != actionId)
				continue;
			outStageIndex = static_cast<std::uint32_t>(index);
			return &pattern.Stages[index];
		}
		return nullptr;
	}

	// 고정/무작위 대상 재조회에는 참가 조건 외에 GRABBED도 제외한다. IsEngageablePlayer와 적용 위치가 다르다.
	bool IsTargetable(const SERVER_PLAYER& player)
	{
		return player.Is_Human() && 0u != player.iCurrentHp && player.isCombatReady &&
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction &&
			LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != player.eAction;
	}

	// 저장된 NetEntityId를 이번 tick의 players에서 다시 찾아 유효성을 검사한다. 포인터를 장기 저장하지 않는다.
	SERVER_PLAYER* FindPatternTarget(
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const LostArk::Shared::NET_ENTITY_ID netEntityId)
	{
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.iNetEntityId == netEntityId && IsTargetable(player))
				return &player;
		}
		return nullptr;
	}

	// 선택/조준 갱신 시 대상을 마지막으로 확인한 위치를 boss에 복사한다. 도약 착지와 포탈 경로가 소비한다.
	void RememberPatternTargetPosition(
		SERVER_WORLD_ENTITY& boss,
		const SERVER_PLAYER& target)
	{
		boss.bHasPatternTargetLastPosition = true;
		boss.fPatternTargetLastPositionX = target.fPositionX;
		boss.fPatternTargetLastPositionY = target.fPositionY;
		boss.fPatternTargetLastPositionZ = target.fPositionZ;
	}

	// 대상 선택 실패나 패턴 종료 시 마지막 위치의 유효 플래그와 좌표를 함께 비운다.
	void ClearPatternTargetPosition(SERVER_WORLD_ENTITY& boss)
	{
		boss.bHasPatternTargetLastPosition = false;
		boss.fPatternTargetLastPositionX = 0.f;
		boss.fPatternTargetLastPositionY = 0.f;
		boss.fPatternTargetLastPositionZ = 0.f;
	}

	// stage 전환 또는 패턴 정리 때 적용했던 stage 높이 보정의 기준 Y로 되돌린다.
	void RestorePatternStageVerticalOffset(SERVER_WORLD_ENTITY& boss)
	{
		if (boss.bPatternStageVerticalOffsetApplied &&
			std::isfinite(boss.fPatternStageVerticalBaseY))
		{
			boss.fPositionY = boss.fPatternStageVerticalBaseY;
		}
		boss.bPatternStageVerticalOffsetApplied = false;
		boss.fPatternStageVerticalBaseY = 0.f;
	}

	// stage 보정을 먼저 제거한 뒤 pattern 보정을 제거한다. 진입 때 쌓인 두 보정을 역순으로 해제한다.
	void RestorePatternVerticalOffset(SERVER_WORLD_ENTITY& boss)
	{
		RestorePatternStageVerticalOffset(boss);
		if (boss.bPatternVerticalOffsetApplied &&
			std::isfinite(boss.fPatternVerticalBaseY))
		{
			boss.fPositionY = boss.fPatternVerticalBaseY;
		}
		boss.bPatternVerticalOffsetApplied = false;
		boss.fPatternVerticalBaseY = 0.f;
	}

	// 입력한 월드 XZ 지점을 향한 yaw(degree)를 갱신한다. 같은 위치에 가까우면 기존 방향을 유지한다.
	void FacePoint(SERVER_WORLD_ENTITY& boss, const float x, const float z)
	{
		const float deltaX = x - boss.fPositionX;
		const float deltaZ = z - boss.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= 0.000001f)
			return;
		//boss의 yawdegrees를 atan2로 입력 매개 변수로 받은 x, z를 사용해서 
		//boss의 yaw degrees를 갱신한다.
		boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	// 패턴 시작/추적이 사용할 조준 원점을 결정하고 대상 방향으로 boss yaw를 갱신한다.
	void FacePatternTarget(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const SERVER_PLAYER& target)
	{
		float originX = boss.fPositionX;
		float originZ = boss.fPositionZ;
		if (BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind &&
			pattern.Motion.bMoveToAnchorBeforeTakeoff)
		{
			// 이륙 전에 중앙 anchor로 이동하는 패턴은 그 anchor를 조준 기준으로 삼는다.
			// 몸체가 이동하는 동안에도 중앙을 기준으로 하는 판정 방향의 원점이 흔들리지 않게 한다.
			originX = pattern.Motion.fLandingX;
			originZ = pattern.Motion.fLandingZ;
		}
		if (!std::isfinite(originX) || !std::isfinite(originZ) ||
			!std::isfinite(target.fPositionX) ||
			!std::isfinite(target.fPositionZ))
		{
			return;
		}
		const float deltaX = target.fPositionX - originX;
		const float deltaZ = target.fPositionZ - originZ;
		if (deltaX * deltaX + deltaZ * deltaZ <= 0.000001f)
			return;
		boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	}

	// 차용한 후보 포인터 목록을 PlayerId로 정렬한 뒤 patternSequence로 하나를 선택한다.
	// 같은 후보 집합과 sequence는 같은 결과를 만들며 암호학적 난수를 제공하는 함수는 아니다.
	SERVER_PLAYER* SelectDeterministicRandomTarget(
		std::vector<SERVER_PLAYER*>& candidates,
		const std::uint32_t patternSequence)
	{
		if (candidates.empty())
			return nullptr;
		std::sort(candidates.begin(), candidates.end(),
			[](const SERVER_PLAYER* left, const SERVER_PLAYER* right)
			{
				return left->iPlayerId < right->iPlayerId;
			});
		std::uint64_t random =
			static_cast<std::uint64_t>(patternSequence) + 0x9e3779b97f4a7c15ull;
		random ^= random >> 30u;
		random *= 0xbf58476d1ce4e5b9ull;
		random ^= random >> 27u;
		random *= 0x94d049bb133111ebull;
		random ^= random >> 31u;
		return candidates[static_cast<std::size_t>(random % candidates.size())];
	}

	// 패턴 시작 시 현재 대상 가능 플레이어만 모아 sequence 기반 선택 함수에 전달한다.
	SERVER_PLAYER* SelectRandomTarget(
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::uint32_t patternSequence)
	{
		std::vector<SERVER_PLAYER*> candidates;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (IsTargetable(player))
				candidates.push_back(&player);
		}
		return SelectDeterministicRandomTarget(candidates, patternSequence);
	}

	// 시작 방향과 대상 방향의 내적이 음수인 뒤쪽 후보를 먼저 고른다.
	// 뒤쪽 후보가 없으면 전체 대상 가능 플레이어의 무작위 선택으로 이어진다.
	SERVER_PLAYER* SelectRandomTargetBehind(
		const SERVER_WORLD_ENTITY& boss,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::uint32_t patternSequence)
	{
		std::vector<SERVER_PLAYER*> candidates;
		if (std::isfinite(boss.fYawDegrees) &&
			std::isfinite(boss.fPositionX) && std::isfinite(boss.fPositionZ))
		{
			const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
			const float forwardX = std::sin(yawRadians);
			const float forwardZ = std::cos(yawRadians);
			for (auto& [playerId, player] : players)
			{
				(void)playerId;
				if (!IsTargetable(player) ||
					!std::isfinite(player.fPositionX) ||
					!std::isfinite(player.fPositionZ))
				{
					continue;
				}
				const float deltaX = player.fPositionX - boss.fPositionX;
				const float deltaZ = player.fPositionZ - boss.fPositionZ;
				if (deltaX * forwardX + deltaZ * forwardZ < 0.f)
					candidates.push_back(&player);
			}
		}
		SERVER_PLAYER* selected =
			SelectDeterministicRandomTarget(candidates, patternSequence);
		return nullptr == selected ?
			SelectRandomTarget(players, patternSequence) : selected;
	}

	// BeginPattern이 한 실행의 대상 ID/마지막 위치/시작 방향을 확정한다.
	// 대상 정책과 조준 정책은 별개이므로 대상 선택 후 anchor 조준 또는 대상 조준을 적용한다.
	void BeginPatternTargetAndAim(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget)
	{
		SERVER_PLAYER* selected = nullptr;
		switch (pattern.eTargetPolicy)
		{
		case BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK:
		case BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START:
			selected = nullptr == nearestTarget ?
				SelectNearestEngageableTarget(boss, players) : nearestTarget;
			break;
		case BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START:
			selected = SelectRandomTarget(players, boss.iPatternSequence);
			break;
		case BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START:
			selected = SelectRandomTargetBehind(
				boss, players, boss.iPatternSequence);
			break;
		default:
			break;
		}
		boss.iPatternTargetEntityId = nullptr == selected ?
			LostArk::Shared::INVALID_NET_ENTITY_ID : selected->iNetEntityId;
		if (nullptr != selected)
			RememberPatternTargetPosition(boss, *selected);
		else
			ClearPatternTargetPosition(boss);

		if (BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR == pattern.eAimPolicy &&
			BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR == pattern.Motion.eKind)
		{
			FacePoint(boss, pattern.Motion.fLandingX, pattern.Motion.fLandingZ);
		}
		else if (nullptr != selected &&
			(BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK == pattern.eAimPolicy ||
			 BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START == pattern.eAimPolicy))
		{
			FacePatternTarget(boss, pattern, *selected);
		}
	}

	// Update와 stage 진입 직후 갱신이 현재 정의의 대상/조준 정책을 적용한다.
	// stage의 최근접 추적이 우선하며, 조준 종료 tick을 지났으면 저장된 방향을 유지한다.
	void UpdatePatternTargetAndAim(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget, const std::uint32_t serverTick)
	{
		const auto* stage = boss.iPatternStageIndex < pattern.Stages.size() ?
			&pattern.Stages[boss.iPatternStageIndex] : nullptr;
		std::uint64_t previousTicks = boss.iPatternAimLastElapsedTicks;
		std::uint64_t elapsedTicks = previousTicks;
		std::uint64_t endTicks = 0u;
		if (stage && (stage->bHasAimEnd || stage->bHasAimResponseScale))
		{
			// stage 진입 직후의 조준 갱신은 경과 tick을 0으로 본다.
			// 첫 시뮬레이션 평가는 별도로 세며, 정수 tick의 순환도 StageElapsedTicks가 처리한다.
			elapsedTicks = serverTick == boss.iActionStartTick &&
				boss.iPatternStageFirstEvaluationTick != serverTick ? 0u :
				StageElapsedTicks(boss, serverTick);
			const auto endMs = stage->bHasAimEnd ? stage->iAimEndMs : stage->iDurationMs;
			endTicks = (static_cast<std::uint64_t>(endMs) * SERVER_TICK_HZ + 999u) / 1000u;
			if (elapsedTicks >= endTicks)
				return;
			previousTicks = (std::min)(previousTicks, elapsedTicks);
			boss.iPatternAimLastElapsedTicks = elapsedTicks;
		}
		const auto applyResponse = [&](const float previousYaw)
		{
			if (stage && stage->bHasAimResponseScale && endTicks > previousTicks)
				boss.fYawDegrees = static_cast<float>(LostArk::Shared::KoukuTargetTracking::RotateOnlyYaw(
					previousYaw, boss.fYawDegrees, elapsedTicks - previousTicks,
					endTicks - previousTicks, stage->fAimResponseScale));
		};
		if (boss.iPatternStageIndex < pattern.Stages.size() &&
			pattern.Stages[boss.iPatternStageIndex].bTrackNearestTarget)
		{
			// stage의 최근접 조준은 중앙 이동이 끝난 위치를 포함한 현재 몸체 원점을 사용한다.
			SERVER_PLAYER* selected = SelectNearestEngageableTarget(boss, players);
			boss.iPatternTargetEntityId = nullptr == selected ?
				LostArk::Shared::INVALID_NET_ENTITY_ID : selected->iNetEntityId;
			if (nullptr != selected)
			{
				RememberPatternTargetPosition(boss, *selected);
				const float previousYaw = boss.fYawDegrees;
				FacePoint(boss, selected->fPositionX, selected->fPositionZ);
				applyResponse(previousYaw);
			}
			else
				ClearPatternTargetPosition(boss);
			return;
		}
		if (BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK ==
			pattern.eTargetPolicy)
		{
			boss.iPatternTargetEntityId = nullptr == nearestTarget ?
				LostArk::Shared::INVALID_NET_ENTITY_ID :
				nearestTarget->iNetEntityId;
		}
		SERVER_PLAYER* patternTarget =
			FindPatternTarget(players, boss.iPatternTargetEntityId);
		if (nullptr != patternTarget)
			RememberPatternTargetPosition(boss, *patternTarget);
		if (((stage && stage->bTrackPatternTarget) ||
			BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK == pattern.eAimPolicy) &&
			nullptr != patternTarget)
		{
			const float previousYaw = boss.fYawDegrees;
			FacePatternTarget(boss, pattern, *patternTarget);
			applyResponse(previousYaw);
		}
	}

	// Update가 넘긴 현재 체력 줄과 이전 평가 기록으로 새 체력 기믹을 예약한다.
	// 체력 경계 내림차순, trigger 순서, pattern ID 순으로 정렬하여 대기열 순서를 안정화한다.
	// 정의 revision/세대가 유효하지 않거나 기록 상한을 넘으면 reset 필요 상태를 남긴다.
	void QueueCrossedHealthBarPatterns(
		SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::uint32_t currentHealthBar,
		const LostArk::Shared::GameplayDataRevision& definitionRevision,
		const std::uint16_t definitionGenerationEpoch,
		const std::uint32_t serverTick)
	{
		if (!definitionRevision.Is_Valid() || 0u == definitionGenerationEpoch)
		{
			boss.bMechanicLedgerRequiresReset = true;
			return;
		}
		const bool definitionGenerationChanged =
			0u != boss.iLastHealthMechanicGenerationEpoch &&
			boss.iLastHealthMechanicGenerationEpoch !=
				definitionGenerationEpoch;
		std::vector<const BOSS_PATTERN_DEFINITION*> crossed;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection ||
				boss.iPhase < pattern.iMinimumPhase ||
				boss.iPhase > pattern.iMaximumPhase ||
				nullptr != FindMechanicOccurrence(boss, pattern.strPatternId) ||
				(!definitionGenerationChanged &&
				 boss.iLastEvaluatedHealthBar <= pattern.iTriggerHealthBar) ||
				currentHealthBar > pattern.iTriggerHealthBar)
			{
				continue;
			}
			crossed.push_back(&pattern);
		}
		std::sort(crossed.begin(), crossed.end(),
			[](const BOSS_PATTERN_DEFINITION* left,
				const BOSS_PATTERN_DEFINITION* right)
			{
				if (left->iTriggerHealthBar != right->iTriggerHealthBar)
					return left->iTriggerHealthBar > right->iTriggerHealthBar;
				if (left->iTriggerOrder != right->iTriggerOrder)
					return left->iTriggerOrder < right->iTriggerOrder;
				return left->strPatternId < right->strPatternId;
			});
		for (const BOSS_PATTERN_DEFINITION* pattern : crossed)
		{
			if (boss.MechanicOccurrences.size() >=
				CValtanBrain::MAX_MECHANIC_OCCURRENCE_COUNT)
			{
				boss.bMechanicLedgerRequiresReset = true;
				continue;
			}
			SERVER_BOSS_MECHANIC_OCCURRENCE occurrence{};
			occurrence.strPatternId = pattern->strPatternId;
			// 이번 체력 경계 평가에 사용한 Catalog revision을 실행 기록에 고정한다.
			// 현재 실행 중인 패턴의 revision과 새로 활성화된 정의 세대가 다를 수 있기 때문이다.
			occurrence.PinnedDefinitionRevision = definitionRevision;
			occurrence.iTriggerHealthBar = pattern->iTriggerHealthBar;
			occurrence.iQueuedTick = serverTick;
			boss.MechanicOccurrences.push_back(std::move(occurrence));
			boss.PendingPatternIds.push_back(pattern->strPatternId);
			boss.TriggeredPatternIds.push_back(pattern->strPatternId);
		}
		boss.iLastEvaluatedHealthBar = currentHealthBar;
		boss.iLastHealthMechanicGenerationEpoch =
			definitionGenerationEpoch;
	}

	struct VALTAN_PATTERN_SELECTION_RESULT final
	{
		const BOSS_PATTERN_DEFINITION* pPattern = nullptr;
		VALTAN_DECISION_TRACE Trace;
	};

	// SelectNormalPattern의 tick/sequence 입력을 섞는다. 선택 과정을 trace 값으로 재현할 수 있다.
	std::uint64_t MixDecisionRandom(std::uint64_t value)
	{
		value ^= value >> 30u;
		value *= 0xbf58476d1ce4e5b9ull;
		value ^= value >> 27u;
		value *= 0x94d049bb133111ebull;
		value ^= value >> 31u;
		return value;
	}

	// 결정 직전 boss 상태와 가장 앞선 대기 항목을 복사한다. 선택 자체나 boss 상태는 변경하지 않는다.
	VALTAN_DECISION_TRACE MakeDecisionTraceHeader(
		const SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick,
		const std::uint32_t currentHealthBar)
	{
		VALTAN_DECISION_TRACE trace{};
		trace.iServerTick = serverTick;
		trace.iPatternSequenceBeforeDecision = boss.iPatternSequence;
		trace.iExpectedPatternSequence =
			(std::numeric_limits<std::uint32_t>::max)() ==
				boss.iPatternSequence ? 1u : boss.iPatternSequence + 1u;
		trace.iCurrentHp = boss.iCurrentHp;
		trace.iMaximumHp = boss.iMaximumHp;
		trace.iHealthBar = currentHealthBar;
		trace.iGameplayPhase = boss.iPhase;
		trace.iTargetNetEntityId = boss.iTargetEntityId;
		trace.bIntroPatternConsumed = boss.bIntroPatternConsumed;
		trace.strRotationId = boss.strRotationId;
		trace.iRotationStepIndex = boss.iRotationStepIndex;
		if (boss.PendingPatternFollowup.Is_Pending())
		{
			trace.strPendingPatternId =
				boss.PendingPatternFollowup.strPatternId;
			trace.ePendingSource =
				VALTAN_DECISION_SOURCE::FORCED_AUDITION;
		}
		else if (!boss.PendingPatternIds.empty())
		{
			trace.strPendingPatternId = boss.PendingPatternIds.front();
			const SERVER_BOSS_MECHANIC_OCCURRENCE* occurrence =
				FindMechanicOccurrence(boss, trace.strPendingPatternId);
			trace.ePendingSource = nullptr != occurrence &&
				SERVER_BOSS_MECHANIC_STATE::QUEUED == occurrence->eState ?
					VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR :
					VALTAN_DECISION_SOURCE::FORCED_AUDITION;
		}
		return trace;
	}

	// 일반 가중치 선택과 대상 부재의 제외 이유 기록에 사용한다. 반환 정의는 patterns에서 차용한다.
	// 강제 제외 조건과 연속 사용 제한을 나눠, 다른 후보가 없을 때만 연속 사용 제한을 완화한다.
	// 후보 기록 상한은 trace만 제한하며 실제 선택 후보 목록을 자르지는 않는다.
	VALTAN_PATTERN_SELECTION_RESULT SelectNormalPattern(
		const SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const std::string& introPatternId,
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick,
		VALTAN_DECISION_TRACE trace,
		const VALTAN_DECISION_SOURCE source,
		const BOSS_PATTERN_ROTATION_DEFINITION* managedRotation = nullptr,
		const bool hasValidTarget = true)
	{
		struct WEIGHTED_PATTERN_VIEW final
		{
			const BOSS_PATTERN_DEFINITION* pPattern = nullptr;
			std::uint32_t iSelectionWeight = 0u;
		};
		const bool hasManagedSelectionSet = nullptr != managedRotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				managedRotation->eSelectionMode;
		std::vector<WEIGHTED_PATTERN_VIEW> eligible;
		std::vector<WEIGHTED_PATTERN_VIEW> repeatAllowed;
		std::vector<const BOSS_PATTERN_DEFINITION*> evaluationOrder;
		evaluationOrder.reserve(patterns.size());
		if (hasManagedSelectionSet)
		{
			// 가중치 구간의 순서는 저작된 candidate 순서를 따른다.
			// 선택 집합 밖의 정의는 제외 이유를 기록하기 위해 뒤에 붙이며 추첨에는 넣지 않는다.
			for (const BOSS_PATTERN_ROTATION_CANDIDATE& candidate :
				managedRotation->Candidates)
			{
				const auto definition = std::find_if(
					patterns.begin(), patterns.end(),
					[&candidate](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == candidate.strPatternId;
					});
				if (patterns.end() != definition)
					evaluationOrder.push_back(&*definition);
			}
			for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
			{
				const bool belongsToManagedSet =
					managedRotation->Candidates.end() != std::find_if(
						managedRotation->Candidates.begin(),
						managedRotation->Candidates.end(),
						[&pattern](
							const BOSS_PATTERN_ROTATION_CANDIDATE& candidate)
						{
							return candidate.strPatternId == pattern.strPatternId;
						});
				if (!belongsToManagedSet)
					evaluationOrder.push_back(&pattern);
			}
		}
		else
		{
			for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
				evaluationOrder.push_back(&pattern);
		}
		for (const BOSS_PATTERN_DEFINITION* patternDefinition : evaluationOrder)
		{
			const BOSS_PATTERN_DEFINITION& pattern = *patternDefinition;
			const BOSS_PATTERN_ROTATION_CANDIDATE* managedCandidate = nullptr;
			if (hasManagedSelectionSet)
			{
				const auto candidate = std::find_if(
					managedRotation->Candidates.begin(),
					managedRotation->Candidates.end(),
					[&pattern](
						const BOSS_PATTERN_ROTATION_CANDIDATE& row)
					{
						return row.strPatternId == pattern.strPatternId;
					});
				if (managedRotation->Candidates.end() != candidate)
					managedCandidate = &*candidate;
			}
			const std::uint32_t authoredSelectionWeight =
				nullptr == managedCandidate ? pattern.iSelectionWeight :
					managedCandidate->iSelectionWeight;
			std::uint32_t exclusionMask = VALTAN_EXCLUDE_NONE;
			if (BOSS_PATTERN_SELECTION::NORMAL != pattern.eSelection)
				exclusionMask |= VALTAN_EXCLUDE_WRONG_SELECTION_KIND;
			if (pattern.strPatternId == introPatternId)
				exclusionMask |= VALTAN_EXCLUDE_INTRO_ROW;
			if (hasManagedSelectionSet && nullptr == managedCandidate)
			{
				exclusionMask |= VALTAN_EXCLUDE_NOT_IN_SELECTION_SET;
			}
			if (!CValtanBrain::Is_ArmorRequirementMet(
				boss, pattern.eArmorRequirement))
				exclusionMask |= VALTAN_EXCLUDE_ARMOR_MISMATCH;
			if (!CValtanBrain::Is_PhaseRequirementMet(
				boss, pattern.ePhaseRequirement))
				exclusionMask |= VALTAN_EXCLUDE_PHASE_REQUIREMENT;
			if (boss.iPhase < pattern.iMinimumPhase ||
				boss.iPhase > pattern.iMaximumPhase)
				exclusionMask |= VALTAN_EXCLUDE_PHASE_RANGE;
			if (currentHealthBar < pattern.iMinimumHealthBar ||
				currentHealthBar > pattern.iMaximumHealthBar)
				exclusionMask |= VALTAN_EXCLUDE_HEALTH_BAR_RANGE;
			if (!hasValidTarget)
				exclusionMask |= VALTAN_EXCLUDE_NO_TARGET;
			else
			{
				if (targetDistance < pattern.fMinimumRange)
					exclusionMask |= VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE;
				if (targetDistance > pattern.fMaximumRange)
					exclusionMask |= VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE;
			}
			const std::uint32_t cooldownRemaining =
				PatternCooldownRemainingTicks(boss, pattern, serverTick);
			if (0u != cooldownRemaining)
				exclusionMask |= VALTAN_EXCLUDE_COOLDOWN;
			const bool maximumConsecutiveRejected =
				BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection &&
				pattern.iMaximumConsecutiveUses > 0u &&
				pattern.strPatternId == boss.strLastPatternId &&
				boss.iConsecutivePatternUses >= pattern.iMaximumConsecutiveUses;
			if (maximumConsecutiveRejected)
				exclusionMask |= VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED;
			if (BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection &&
				((nullptr != managedCandidate && !managedCandidate->bEnabled) ||
				 (nullptr == managedCandidate && 0u == authoredSelectionWeight)))
				exclusionMask |= VALTAN_EXCLUDE_DISABLED;

			if (trace.Candidates.size() <
				CValtanBrain::MAX_DECISION_CANDIDATE_COUNT)
			{
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = pattern.strPatternId;
				candidate.iExclusionMask = exclusionMask;
				candidate.iAuthoredWeight = authoredSelectionWeight;
				candidate.iCooldownRemainingTicks = cooldownRemaining;
				candidate.iConsecutiveUses =
					pattern.strPatternId == boss.strLastPatternId ?
						boss.iConsecutivePatternUses : 0u;
				candidate.iMaximumConsecutiveUses =
					pattern.iMaximumConsecutiveUses;
				candidate.bSoftRepeatBlocked = maximumConsecutiveRejected;
				trace.Candidates.push_back(std::move(candidate));
			}
			else
			{
				trace.bCandidatesTruncated = true;
			}

			const std::uint32_t hardMask = exclusionMask &
				~VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED;
			if (VALTAN_EXCLUDE_NONE != hardMask)
				continue;
			eligible.push_back({ &pattern, authoredSelectionWeight });
			if (!maximumConsecutiveRejected)
				repeatAllowed.push_back({ &pattern, authoredSelectionWeight });
		}
		// 거리/phase/갑옷/재사용 대기 등 강제 조건은 풀지 않는다.
		// 연속 사용 제한을 지키는 후보만 비었을 때 그 제한 하나를 완화한다.
		const bool relaxMaximumConsecutive =
			repeatAllowed.empty() && !eligible.empty();
		trace.bMaximumConsecutiveRelaxed = relaxMaximumConsecutive;
		const auto& candidates = relaxMaximumConsecutive ? eligible : repeatAllowed;
		if (relaxMaximumConsecutive)
		{
			for (VALTAN_DECISION_CANDIDATE_TRACE& candidate : trace.Candidates)
			{
				if (0u == (candidate.iExclusionMask &
					VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED))
				{
					continue;
				}
				const std::uint32_t hardMask = candidate.iExclusionMask &
					~VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED;
				if (VALTAN_EXCLUDE_NONE != hardMask)
					continue;
				candidate.iExclusionMask |=
					VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED;
				candidate.bSoftRepeatRelaxed = true;
			}
		}
		// 누적 가중치의 반열린 구간 [begin, end)에 ticket이 들어가는 후보를 고른다.
		// 목록 순서가 구간 경계를 정하므로 추첨 입력과 구간을 함께 trace에 남긴다.
		std::uint64_t totalWeight = 0u;
		for (const WEIGHTED_PATTERN_VIEW& pattern : candidates)
			totalWeight += pattern.iSelectionWeight;
		trace.eSource = source;
		trace.iTotalWeight = totalWeight;
		if (0u == totalWeight)
		{
			trace.eResult = VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
			return { nullptr, std::move(trace) };
		}

		const std::uint64_t rawRandomInput =
			(static_cast<std::uint64_t>(serverTick) << 32u) |
			static_cast<std::uint64_t>(boss.iPatternSequence + 1u);
		const std::uint64_t random = MixDecisionRandom(rawRandomInput);
		trace.iRawRandomInput = rawRandomInput;
		trace.iMixedRandomValue = random;
		const std::uint64_t selectedTicket = random % totalWeight;
		trace.iRandomTicket = selectedTicket;
		std::uint64_t weightBegin = 0u;
		const BOSS_PATTERN_DEFINITION* selectedPattern = nullptr;
		for (const WEIGHTED_PATTERN_VIEW& weighted : candidates)
		{
			const BOSS_PATTERN_DEFINITION* pattern = weighted.pPattern;
			const std::uint64_t weightEnd =
				weightBegin + weighted.iSelectionWeight;
			auto traced = std::find_if(
				trace.Candidates.begin(), trace.Candidates.end(),
				[pattern](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
				{
					return candidate.strPatternId == pattern->strPatternId;
				});
			if (trace.Candidates.end() != traced)
			{
				traced->iEffectiveWeight = weighted.iSelectionWeight;
				traced->iWeightBeginInclusive = weightBegin;
				traced->iWeightEndExclusive = weightEnd;
			}
			if (nullptr == selectedPattern &&
				selectedTicket >= weightBegin && selectedTicket < weightEnd)
			{
				selectedPattern = pattern;
				if (trace.Candidates.end() != traced)
					traced->bSelected = true;
			}
			weightBegin = weightEnd;
		}
		if (nullptr != selectedPattern)
		{
			trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
			trace.strSelectedPatternId = selectedPattern->strPatternId;
			return { selectedPattern, std::move(trace) };
		}
		// 선택 ticket은 totalWeight 미만이며 모든 후보의 가중치 구간을 기록한다.
		// 끝까지 선택되지 않았다면 마지막 후보를 반환하는 기존 방어 경로를 유지한다.
		trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
		trace.strSelectedPatternId = candidates.back().pPattern->strPatternId;
		for (VALTAN_DECISION_CANDIDATE_TRACE& candidate : trace.Candidates)
		{
			if (candidate.strPatternId ==
				candidates.back().pPattern->strPatternId)
			{
				candidate.bSelected = true;
				break;
			}
		}
		return { candidates.back().pPattern, std::move(trace) };
	}

	// 순서형 sequence의 다음 항목 전 추적 대기 tick을 선택한다. 마지막 항목 뒤에는 대기를 만들지 않는다.
	std::uint32_t ResolveSequencePursuitTicksAfter(
		const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
		const std::uint32_t stepIndex)
	{
		if (stepIndex + 1u >= sequence.PatternIds.size())
			return 0u;
		if (sequence.TransitionPursuitTicks.size() + 1u ==
			sequence.PatternIds.size())
		{
			return sequence.TransitionPursuitTicks[stepIndex];
		}
		return sequence.iInterStepPursuitTicks;
	}

	// Update의 IDLE/CHASE 경로에서 다음 정의를 선택하며 큐와 순서 커서를 필요에 따라 변경한다.
	// 입장 연출, 예약 후속 패턴, 명시 sequence, 입장 공격, 강제 큐를 앞에서부터 검사한다.
	// 이후 rotation 모드에 맞춰 순환/가중치/일회 소개를 처리하고 조건부 전체 가중치 선택으로 이어진다.
	// nullptr은 대기일 수도 정의 오류일 수도 있으므로 Trace 결과와 reset 상태를 함께 소비해야 한다.
	const BOSS_PATTERN_DEFINITION* SelectPattern(
		SERVER_WORLD_ENTITY& boss,
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns,
		const LostArk::Shared::GameplayDataRevision& definitionRevision,
		const std::string& introPatternId,
		const BOSS_PATTERN_SEQUENCE_DEFINITION* scriptedSequence,
		const BOSS_PATTERN_ROTATION_DEFINITION* rotation,
		const std::uint32_t currentHealthBar,
		const float targetDistance,
		const std::uint32_t serverTick,
		VALTAN_DECISION_TRACE& trace)
	{
		const bool healthRotations = nullptr != scriptedSequence &&
			BOSS_PATTERN_SEQUENCE_MODE::HEALTH_BAR_ROTATIONS == scriptedSequence->eMode;
		if (healthRotations && !boss.bIntroPatternConsumed &&
			!boss.bEntranceCinematicConsumed && !boss.bScriptedPatternPlayback &&
			!boss.bAutomaticPatternSequenceAuditionOverride &&
			!scriptedSequence->strEntranceCinematicPatternId.empty())
		{
			const auto* cinematic = FindPattern(patterns, scriptedSequence->strEntranceCinematicPatternId);
			if (nullptr == cinematic)
			{
				boss.bMechanicLedgerRequiresReset = true;
				trace.eResult = VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				return nullptr;
			}
			// 자동 체력별 순환은 입장 연출 뒤에 저작된 첫 실행 항목부터 시작한다.
			// 이 경로에서는 기존 입장 공격을 별도로 넣지 않도록 소비 상태를 함께 기록한다.
			// 이 소비 처리는 아래의 입장 연출 선택 경로에만 적용한다.
			boss.bEntranceCinematicConsumed = true;
			boss.bIntroPatternConsumed = true;
			trace.eSource = VALTAN_DECISION_SOURCE::INTRO;
			trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
			trace.strSelectedPatternId = cinematic->strPatternId;
			VALTAN_DECISION_CANDIDATE_TRACE candidate{};
			candidate.strPatternId = cinematic->strPatternId;
			candidate.iAuthoredWeight = candidate.iEffectiveWeight = candidate.iWeightEndExclusive = 1u;
			candidate.bSelected = true;
			trace.Candidates.push_back(std::move(candidate));
			return cinematic;
		}
		if (healthRotations)
			scriptedSequence = nullptr;
		if (boss.PendingPatternFollowup.Is_Pending())
		{
			const SERVER_BOSS_PATTERN_FOLLOWUP pending =
				boss.PendingPatternFollowup;
			trace.strPendingPatternId = pending.strPatternId;
			trace.ePendingSource = VALTAN_DECISION_SOURCE::FORCED_AUDITION;
			const bool ownsExactCompletedSource =
				pending.PinnedDefinitionRevision.Is_Valid() &&
				pending.PinnedDefinitionRevision == definitionRevision &&
				0u != pending.iSourcePatternSequence &&
				0u != pending.iRootPatternSequence &&
				pending.iSourcePatternSequence == boss.iPatternSequence &&
				pending.iSourcePatternSequence ==
					boss.PatternTerminalReceipt.iPatternSequence &&
				pending.iRootPatternSequence ==
					boss.PatternTerminalReceipt.iRootPatternSequence &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					boss.PatternTerminalReceipt.eResult &&
				pending.iDepth > 0u &&
				pending.iDepth <= MAX_PATTERN_FOLLOWUP_DEPTH;
			const BOSS_PATTERN_DEFINITION* followup =
				ownsExactCompletedSource ?
					FindPattern(patterns, pending.strPatternId) : nullptr;
			const bool validFollowup = nullptr != followup &&
				BOSS_PATTERN_SELECTION::AUDITION_ONLY == followup->eSelection &&
				BOSS_PATTERN_TARGET_POLICY::NONE == followup->eTargetPolicy &&
				BOSS_PATTERN_AIM_POLICY::NONE == followup->eAimPolicy;
			if (!validFollowup)
			{
				boss.PendingPatternFollowup = {};
				boss.iPatternFollowupDepth = 0u;
				boss.iPatternFollowupRootSequence = 0u;
				boss.bMechanicLedgerRequiresReset = true;
				trace.eSource = VALTAN_DECISION_SOURCE::FORCED_AUDITION;
				trace.eResult =
					VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = pending.strPatternId;
				candidate.iExclusionMask =
					VALTAN_EXCLUDE_UNRESOLVED_DEFINITION;
				trace.Candidates.push_back(std::move(candidate));
				return nullptr;
			}
			boss.PendingPatternFollowup = {};
			boss.iPatternFollowupDepth = pending.iDepth;
			boss.iPatternFollowupRootSequence =
				pending.iRootPatternSequence;
			trace.eSource = VALTAN_DECISION_SOURCE::FORCED_AUDITION;
			trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
			trace.strSelectedPatternId = followup->strPatternId;
			VALTAN_DECISION_CANDIDATE_TRACE candidate{};
			candidate.strPatternId = followup->strPatternId;
			candidate.iAuthoredWeight = 1u;
			candidate.iEffectiveWeight = 1u;
			candidate.iWeightEndExclusive = 1u;
			candidate.bSelected = true;
			trace.Candidates.push_back(std::move(candidate));
			return followup;
		}
		// 명시된 자동 sequence는 입장부터 마지막 항목까지의 실행 순서를 소유한다.
		// 마지막 항목 뒤에는 가중치 선택으로 넘어가지 않는다.
		// Debug 강제 대기열과 입장/체력 기믹 시험 재생은 별도 조건으로 이 경로를 우회한다.
		if (!boss.bScriptedPatternPlayback &&
			!boss.bAutomaticPatternSequenceAuditionOverride &&
			boss.PendingPatternIds.empty() && nullptr != scriptedSequence)
		{
			boss.bIntroPatternConsumed = true;
			trace.bIntroPatternConsumed = true;
			trace.strRotationId = scriptedSequence->strSequenceId;
			if (boss.strRotationId != scriptedSequence->strSequenceId)
			{
				boss.strRotationId = scriptedSequence->strSequenceId;
				boss.iRotationStepIndex = 0u;
				boss.iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
				boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			}
			if (boss.iRotationStepIndex >=
				static_cast<std::uint32_t>(scriptedSequence->PatternIds.size()))
			{
				boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
				trace.iRotationStepIndex = boss.iRotationStepIndex;
				trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
				trace.eResult = VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
				return nullptr;
			}
			if (boss.bAutomaticPatternSequenceStepRunning)
			{
				boss.bMechanicLedgerRequiresReset = true;
				trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
				trace.eResult =
					VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				return nullptr;
			}
			if (boss.iAutomaticPatternSequencePursuitTicksRemaining > 0u)
			{
				--boss.iAutomaticPatternSequencePursuitTicksRemaining;
				trace.iRotationStepIndex = boss.iRotationStepIndex;
				trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
				trace.eResult = VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
				return nullptr;
			}
			const std::string& patternId =
				scriptedSequence->PatternIds[boss.iRotationStepIndex];
			const BOSS_PATTERN_DEFINITION* step = FindPattern(patterns, patternId);
			if (nullptr == step)
			{
				boss.bMechanicLedgerRequiresReset = true;
				trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
				trace.eResult =
					VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = patternId;
				candidate.iExclusionMask =
					VALTAN_EXCLUDE_UNRESOLVED_DEFINITION;
				trace.Candidates.push_back(std::move(candidate));
				return nullptr;
			}
			boss.iAutomaticPatternSequenceInterStepPursuitTicks =
				ResolveSequencePursuitTicksAfter(
					*scriptedSequence, boss.iRotationStepIndex);
			boss.bAutomaticPatternSequenceStepRunning = true;
			trace.iRotationStepIndex = boss.iRotationStepIndex;
			trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
			trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
			trace.strSelectedPatternId = step->strPatternId;
			VALTAN_DECISION_CANDIDATE_TRACE candidate{};
			candidate.strPatternId = step->strPatternId;
			candidate.iAuthoredWeight = 1u;
			candidate.iEffectiveWeight = 1u;
			candidate.iWeightEndExclusive = 1u;
			candidate.bSelected = true;
			trace.Candidates.push_back(std::move(candidate));
			return step;
		}
		// 기존 입장 및 명시적인 시험 재생은 체력 기믹 대기열보다 입장 공격을 먼저 검사한다.
		// 위에서 입장 연출을 선택한 자동 경로는 이미 입장 공격을 소비했다.
		// 입장 정의가 없어도 소비 상태를 남겨 같은 누락을 매 tick 기다리지 않는다.
		if (!boss.bIntroPatternConsumed &&
			(nullptr == scriptedSequence ||
			 boss.bAutomaticPatternSequenceAuditionOverride))
		{
			const BOSS_PATTERN_DEFINITION* intro = introPatternId.empty() ?
				nullptr : FindPattern(patterns, introPatternId);
			if (nullptr == intro)
			{
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = introPatternId;
				candidate.iExclusionMask =
					VALTAN_EXCLUDE_MISSING_DEFINITION;
				trace.Candidates.push_back(std::move(candidate));
				boss.bIntroPatternConsumed = true;
			}
			else if (BOSS_PATTERN_SELECTION::NORMAL != intro->eSelection)
			{
				// 검증되지 않은 정의가 전달되어도 NORMAL이 아닌 입장 공격은 실행하지 않는다.
				// 제외 이유를 기록하고 입장 항목을 소비한다.
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = intro->strPatternId;
				candidate.iExclusionMask =
					VALTAN_EXCLUDE_WRONG_SELECTION_KIND;
				trace.Candidates.push_back(std::move(candidate));
				boss.bIntroPatternConsumed = true;
			}
			else if (targetDistance >= intro->fMinimumRange &&
				targetDistance <= intro->fMaximumRange)
			{
				boss.bIntroPatternConsumed = true;
				boss.bAutomaticPatternSequenceAuditionOverride = false;
				boss.bAutomaticPatternSequenceAuditionHold = false;
				trace.eSource = VALTAN_DECISION_SOURCE::INTRO;
				trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
				trace.strSelectedPatternId = intro->strPatternId;
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = intro->strPatternId;
				candidate.iAuthoredWeight = 1u;
				candidate.iEffectiveWeight = 1u;
				candidate.iWeightEndExclusive = 1u;
				candidate.bSelected = true;
				trace.Candidates.push_back(std::move(candidate));
				return intro;
			}
			else if (boss.iCurrentHp < boss.iMaximumHp)
			{
				// 입장 공격의 사거리에 들어오기 전에 이미 피해를 받았다면 입장 공격을 생략한다.
				// 진행 중인 전투를 입장 조건으로 계속 막지 않기 위한 경로다.
				boss.bIntroPatternConsumed = true;
			}
			else
			{
				// 아직 입장 사거리를 기다리는 동안에는 뒤의 대기열이나 일반 선택을 실행하지 않는다.
				// 이번 tick의 선택 결과만 대기 상태로 기록한다.
				trace.eSource = VALTAN_DECISION_SOURCE::INTRO;
				trace.eResult =
					VALTAN_DECISION_RESULT::WAITING_FOR_INTRO_RANGE;
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = intro->strPatternId;
				candidate.iExclusionMask = VALTAN_EXCLUDE_RANGE;
				trace.Candidates.push_back(std::move(candidate));
				return nullptr;
			}
		}
		// 큐 맨 앞의 stable ID부터 소비한다. 정의를 찾지 못하면 실패 기록을 남기고 reset 여부를 검사한다.
		while (!boss.PendingPatternIds.empty())
		{
			const std::string patternId = boss.PendingPatternIds.front();
			boss.PendingPatternIds.erase(boss.PendingPatternIds.begin());
			if (boss.PendingPatternIds.empty())
			{
				boss.bAutomaticPatternSequenceAuditionOverride = false;
				boss.bAutomaticPatternSequenceAuditionHold = false;
			}
			const SERVER_BOSS_MECHANIC_OCCURRENCE* occurrence =
				FindMechanicOccurrence(boss, patternId);
			const VALTAN_DECISION_SOURCE pendingSource =
				nullptr != occurrence &&
				SERVER_BOSS_MECHANIC_STATE::QUEUED == occurrence->eState ?
					VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR :
					VALTAN_DECISION_SOURCE::FORCED_AUDITION;
			trace.strPendingPatternId = patternId;
			trace.ePendingSource = pendingSource;
			if (const BOSS_PATTERN_DEFINITION* pattern =
				FindPattern(patterns, patternId))
			{
				trace.eSource = pendingSource;
				trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
				trace.strSelectedPatternId = pattern->strPatternId;
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = pattern->strPatternId;
				candidate.iAuthoredWeight = 1u;
				candidate.iEffectiveWeight = 1u;
				candidate.iWeightEndExclusive = 1u;
				candidate.bSelected = true;
				trace.Candidates.push_back(std::move(candidate));
				return pattern;
			}
			FailMechanic(
				boss, patternId,
				SERVER_BOSS_MECHANIC_FAILURE::MISSING_PATTERN_DEFINITION,
				serverTick);
			if (trace.Candidates.size() <
				CValtanBrain::MAX_DECISION_CANDIDATE_COUNT)
			{
				VALTAN_DECISION_CANDIDATE_TRACE candidate{};
				candidate.strPatternId = patternId;
				candidate.iExclusionMask =
					VALTAN_EXCLUDE_UNRESOLVED_DEFINITION;
				trace.Candidates.push_back(std::move(candidate));
			}
			if (boss.bMechanicLedgerRequiresReset)
			{
				trace.eSource = pendingSource;
				trace.eResult =
					VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				return nullptr;
			}
		}
		if (boss.bAutomaticPatternSequenceAuditionHold &&
			nullptr != scriptedSequence)
		{
			// ARM_HEALTH_BAR 시험에서는 Debug 명령으로 체력 경계를 넘긴다.
			// 강제 기믹이 대기열에 들어올 때까지 일반 자동 선택을 보류한다.
			trace.eSource = VALTAN_DECISION_SOURCE::NONE;
			trace.eResult = VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
			return nullptr;
		}
		// 순서형 일반 패턴은 정상 완료 때 FinishPattern이 다음 순번으로 진행시킨다.
		// 위의 체력 기믹 대기열이 우선이며, 순환 ID가 바뀌면 새 목록의 첫 항목부터 시작한다.
		if (!boss.bScriptedPatternPlayback && nullptr != rotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP == rotation->eSelectionMode &&
			!rotation->PatternIds.empty())
		{
			if (boss.strRotationId != rotation->strRotationId)
			{
				boss.strRotationId = rotation->strRotationId;
				boss.iRotationStepIndex = 0u;
			}
			boss.iRotationStepIndex %= static_cast<std::uint32_t>(rotation->PatternIds.size());
			const auto* step = FindPattern(patterns, rotation->PatternIds[boss.iRotationStepIndex]);
			trace.strRotationId = rotation->strRotationId;
			trace.iRotationStepIndex = boss.iRotationStepIndex;
			trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
			if (nullptr == step || BOSS_PATTERN_SELECTION::NORMAL != step->eSelection)
			{
				boss.bMechanicLedgerRequiresReset = true;
				trace.eResult = VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
				return nullptr;
			}
			boss.bAutomaticPatternSequenceStepRunning = true;
			boss.iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
			trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
			trace.strSelectedPatternId = step->strPatternId;
			VALTAN_DECISION_CANDIDATE_TRACE candidate{};
			candidate.strPatternId = step->strPatternId;
			candidate.iAuthoredWeight = candidate.iEffectiveWeight = candidate.iWeightEndExclusive = 1u;
			candidate.bSelected = true;
			trace.Candidates.push_back(std::move(candidate));
			return step;
		}
		if (healthRotations)
		{
			trace.eResult = VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
			return nullptr;
		}
		// WEIGHTED_POOL은 첫 일반 선택부터 가중치를 사용한다.
		// candidate는 활성 여부와 가중치를, 패턴은 거리/phase/갑옷/재사용 대기/연속 사용 조건을 소유한다.
		// 앞의 강제 체력 기믹은 이미 대기열 경로에서 처리했다.
		if (!boss.bScriptedPatternPlayback && nullptr != rotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				rotation->eSelectionMode)
		{
			boss.strRotationId = rotation->strRotationId;
			boss.iRotationStepIndex = 0u;
			trace.strRotationId = rotation->strRotationId;
			VALTAN_PATTERN_SELECTION_RESULT result = SelectNormalPattern(
				boss, patterns, introPatternId, currentHealthBar,
				targetDistance, serverTick, std::move(trace),
				VALTAN_DECISION_SOURCE::WEIGHTED,
				rotation);
			trace = std::move(result.Trace);
			return result.pPattern;
		}
		// ORDERED_INTRO_THEN_WEIGHTED는 목록을 한 번 순서대로 소개한 뒤 가중치 선택으로 넘어간다.
		// 소개 커서는 boss에 남으므로 체력 기믹이 끼어들어도 이어서 진행하고, 순환 ID 변경 시 초기화한다.
		// 소개 뒤의 선택은 목록에 한정되지 않고 조건을 만족하는 전체 일반 패턴을 대상으로 한다.
		if (!boss.bScriptedPatternPlayback && nullptr != rotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == rotation->eSelectionMode &&
			!rotation->PatternIds.empty())
		{
			trace.strRotationId = rotation->strRotationId;
			if (boss.strRotationId != rotation->strRotationId)
			{
				boss.strRotationId = rotation->strRotationId;
				boss.iRotationStepIndex = 0u;
			}
			if (boss.iRotationStepIndex <
				static_cast<std::uint32_t>(rotation->PatternIds.size()))
			{
				const std::size_t stepIndex = boss.iRotationStepIndex;
				++boss.iRotationStepIndex;
				const BOSS_PATTERN_DEFINITION* step =
					FindPattern(patterns, rotation->PatternIds[stepIndex]);
				if (nullptr != step &&
					BOSS_PATTERN_SELECTION::NORMAL == step->eSelection)
				{
					trace.eSource = VALTAN_DECISION_SOURCE::ORDERED;
					trace.eResult = VALTAN_DECISION_RESULT::SELECTED;
					trace.strSelectedPatternId = step->strPatternId;
					VALTAN_DECISION_CANDIDATE_TRACE candidate{};
					candidate.strPatternId = step->strPatternId;
					candidate.iAuthoredWeight = 1u;
					candidate.iEffectiveWeight = 1u;
					candidate.iWeightEndExclusive = 1u;
					candidate.bSelected = true;
					trace.Candidates.push_back(std::move(candidate));
					return step;
				}
				// 정의를 찾지 못하거나 NORMAL이 아닌 소개 항목은 커서를 되돌리지 않는다.
				// 이번 선택은 아래의 가중치 경로로 이어진다.
				if (trace.Candidates.size() <
					CValtanBrain::MAX_DECISION_CANDIDATE_COUNT)
				{
					VALTAN_DECISION_CANDIDATE_TRACE candidate{};
					candidate.strPatternId = rotation->PatternIds[stepIndex];
					candidate.iExclusionMask = nullptr == step ?
						VALTAN_EXCLUDE_UNRESOLVED_DEFINITION :
						VALTAN_EXCLUDE_WRONG_SELECTION_KIND;
					trace.Candidates.push_back(std::move(candidate));
				}
			}
		}
		VALTAN_PATTERN_SELECTION_RESULT result = SelectNormalPattern(
			boss, patterns, introPatternId,
			currentHealthBar, targetDistance, serverTick, std::move(trace),
			VALTAN_DECISION_SOURCE::GLOBAL);
		trace = std::move(result.Trace);
		return result.pPattern;
	}

	// 시간 만료만으로는 이벤트 진입 전용 stage에 들어가지 않는다.
	// 다음 순번부터 Is_EventEnteredStage인 항목을 건너뛰고 일반 시간 진행 stage를 찾는다.
	std::uint32_t NextClockStageIndex(
		const BOSS_PATTERN_DEFINITION& pattern,
		const std::uint32_t currentStageIndex)
	{
		std::uint32_t next = currentStageIndex + 1u;
		while (next < pattern.Stages.size() &&
			Is_EventEnteredStage(pattern.Stages[next].eStageKind))
		{
			++next;
		}
		return next;
	}

	// EnterPatternStage가 내부 stage 종류를 기존 복제 action으로 변환한다.
	SERVER_ENTITY_ACTION ToServerAction(const BOSS_PATTERN_STAGE_KIND kind)
	{
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_KIND::WINDUP:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		case BOSS_PATTERN_STAGE_KIND::ACTIVE:
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		case BOSS_PATTERN_STAGE_KIND::RECOVERY:
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		case BOSS_PATTERN_STAGE_KIND::PART_BREAK:
			// PART_BREAK는 공격이 아니라 반응이므로 복제 action은 기존 회복 상태를 사용한다.
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		case BOSS_PATTERN_STAGE_KIND::GROGGY:
			// GROGGY의 저작 action은 계속 재생하므로 복제 action은 활성 상태를 사용한다.
			// 갑옷 파괴 창 여부는 Server의 별도 stage 상태로 관리한다.
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		default:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		}
	}

	// EnterPatternStage가 이전 돌진 stage의 끝점을 먼저 확정한다. 잠기지 않은 경로나 잘못된 수치는 건너뛴다.
	void CompletePortalTargetRushAtDeadline(SERVER_WORLD_ENTITY& boss)
	{
		if (!boss.bPortalMotionActive ||
			!boss.bPortalRushTargetLocked ||
			BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
				boss.ePatternStageMotionKind ||
			0u == boss.iPatternStageDurationMs ||
			!std::isfinite(boss.fActionElapsedSeconds) ||
			!std::isfinite(boss.fPortalStartX) ||
			!std::isfinite(boss.fPortalStartZ) ||
			!std::isfinite(boss.fPortalRushDistanceM) ||
			boss.fPortalRushDistanceM <= 0.f ||
			!std::isfinite(boss.fYawDegrees))
		{
			return;
		}
		const float durationSeconds = static_cast<float>(
			boss.iPatternStageDurationMs) * MILLISECONDS_TO_SECONDS;
		if (boss.fActionElapsedSeconds + 0.0001f < durationSeconds)
			return;
		// Brain의 stage 전환은 GameRoom의 일반 이동 처리보다 먼저 일어난다.
		// 마감에 남은 마지막 이동을 여기서 끝점으로 확정하여 다음 stage가 완료 위치에서 시작하게 한다.
		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		boss.fPositionX = boss.fPortalStartX +
			std::sin(yawRadians) * boss.fPortalRushDistanceM;
		boss.fPositionZ = boss.fPortalStartZ +
			std::cos(yawRadians) * boss.fPortalRushDistanceM;
	}

	// 공개 wrapper를 거쳐 GameRoom의 stage action commit에서 호출한다.
	// 마지막 대상 위치를 기준으로 방향과 포탈 끝점을 한 번 잠그며 경로가 성립하지 않으면 false다.
	//포탈의 시작 위치를 stage가 시작할 때 target의 위치로 고정한다. 
	bool LockPortalTargetRushAtStageStart(SERVER_WORLD_ENTITY& boss)
	{
		//이미 boss의 portal rush target이 locked되어있어서, 다시 target을 설정할 필요가 없는 경우
		if (boss.bPortalRushTargetLocked)
			return true;
		//portalmotion이 active되어있지 않거나, pattern stage motion kind가 
		//portal target rush가 아니거나, pattern target의 last position을 가지지 못했거나
		//boss의 x z, yaw degrees, pattern target의 last position x z, 
		//portal rush speed meter per second portal rush distance가 isfinite가 아닐 경우 
		//그리고 boss의 portalrushspeed meter per second가 0.f와 작거나 같을 경우에 return false
		//굳이 이렇게까지? 조건을 걸었으면, 그 근거가 될만한 명분이 반드시 존재해야 하는데, 
		//어떤 명분이 존재하는 거지?
		if (!boss.bPortalMotionActive ||
			BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
				boss.ePatternStageMotionKind ||
			!boss.bHasPatternTargetLastPosition ||
			!std::isfinite(boss.fPositionX) ||
			!std::isfinite(boss.fPositionZ) ||
			!std::isfinite(boss.fYawDegrees) ||
			!std::isfinite(boss.fPatternTargetLastPositionX) ||
			!std::isfinite(boss.fPatternTargetLastPositionZ) ||
			!std::isfinite(boss.fPortalRushSpeedMps) ||
			boss.fPortalRushSpeedMps <= 0.f ||
			!std::isfinite(boss.fPortalRushDistanceM) ||
			boss.fPortalRushDistanceM <= 0.f)
		{
			return false;
		}

		// 포탈 쌍이 stage 전환에 맞춰 생성되므로 그 경계를 복제하기 전에 방향과 끝점을 고정한다.
		// retargetDelayMs는 이동과 판정의 시작을 늦추며, 경로 선택 자체를 늦추지는 않는다.
		if (boss.bHasPatternTargetLastPosition)
		{
			//facepoint를 target의 last position x, z로 고정한다. 
			FacePoint(boss, boss.fPatternTargetLastPositionX,
				boss.fPatternTargetLastPositionZ);
		}
		boss.fPortalStartX = boss.fPositionX;
		boss.fPortalStartZ = boss.fPositionZ;
		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		const float portalEndX = boss.fPortalStartX +
			std::sin(yawRadians) * boss.fPortalRushDistanceM;
		const float portalEndZ = boss.fPortalStartZ +
			std::cos(yawRadians) * boss.fPortalRushDistanceM;

		if (!std::isfinite(portalEndX) || !std::isfinite(portalEndZ))
			return false;

		boss.fPortalEndX = portalEndX;
		boss.fPortalEndZ = portalEndZ;
		//이 lasthitsample이 의미하는 바는 뭘까? 
		boss.fPortalLastHitSampleX = boss.fPortalStartX;
		boss.fPortalLastHitSampleZ = boss.fPortalStartZ;
		boss.bPortalRushTargetLocked = true;
		return true;
	}

	// 패턴 시작/시간 만료/이벤트 분기가 공통으로 호출하는 stage 진입 경계다.
	// 이전 stage 결과와 높이를 정리하고 정의를 boss 실행 상태에 복사한 뒤 판정 소비 횟수와 대상 이력을 초기화한다.
	// stage ID는 저장 식별자이고 stageIndex는 현재 정의 vector를 읽는 실행 위치다.
	void EnterPatternStage(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t stageIndex,
		const std::uint32_t serverTick,
		const bool evaluatesOnEntryTick = false)
	{
		CompletePortalTargetRushAtDeadline(boss);
		RestorePatternStageVerticalOffset(boss);
		const std::string previousActionId = boss.strActionId;
		if (!previousActionId.empty())
			CBossCombatRuntime::Discard_PatternOutcomes(
				boss, previousActionId);
		boss.iPatternStageIndex = stageIndex;
		boss.strPatternStageId = stage.strStageId;
		boss.iPatternStageDurationMs = stage.iDurationMs;
		boss.iPatternAimLastElapsedTicks = 0u;
		boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ?
			serverTick : NextServerTickSkippingReservedZero(serverTick);
		boss.fPatternStageOriginX = boss.fPositionX;
		boss.fPatternStageOriginZ = boss.fPositionZ;
		boss.fPatternStageOriginYawDegrees = boss.fYawDegrees;
		boss.iAppliedPatternStageSpawnWaveCount = 0u;
		boss.ePatternStageMotionKind = stage.Motion.eKind;
		boss.PortalStageHitTargets.clear();
		boss.PatternActiveWindowHitTargets.clear();
		CValtanBrain::Configure_PortalMotion(boss, stage);
		boss.strActionId = stage.strActionId;
		boss.strDamageProfileId = stage.strDamageProfileId;
		boss.ePatternHitShape = stage.eHitShape;
		boss.ePatternPlayerResponse = stage.ePlayerResponse;
		boss.ePatternAttachmentSlot = stage.eAttachmentSlot;
		boss.ePatternPartDamagePolicy = stage.ePartDamagePolicy;
		boss.bPatternHasCounterProxy = stage.bHasCounterProxy;
		boss.ePatternCounterProxyKind = stage.eCounterProxyKind;
		boss.fPatternCounterProxyForwardOffsetM =
			stage.fCounterProxyForwardOffsetM;
		boss.fPatternCounterProxyRightOffsetM =
			stage.fCounterProxyRightOffsetM;
		boss.fPatternCounterProxyRadiusM = stage.fCounterProxyRadiusM;
		boss.fPatternCounterProxyArcDegrees = stage.fCounterProxyArcDegrees;
		SetPatternBossResponseState(
			boss, stage.eBossResponseKind, stage.iBossResponseThreshold,
			boss.iPatternBossResponseAccumulatedHealthDamage,
			boss.bPatternBossResponsePublished);
		boss.fPatternHitOuterRadius = stage.fHitOuterRadius;
		boss.fPatternHitInnerRadius = stage.fHitInnerRadius;
		boss.fPatternHitAngleDegrees = stage.fHitAngleDegrees;
		boss.fPatternHitLength = stage.fHitLength;
		boss.fPatternHitHalfWidth = stage.fHitHalfWidth;
		boss.iPatternHitCount = stage.iHitCount;
		boss.iPatternHitIntervalMs = stage.iHitIntervalMs;
		boss.iPatternHitDelayMs = stage.iHitDelayMs;
		boss.iAppliedPatternHitCount = 0u;
		boss.ePatternHitAnchorKind = stage.eHitAnchorKind;
		boss.fPatternHitAnchorForwardOffsetM =
			stage.fHitAnchorForwardOffsetM;
		boss.fPatternHitAnchorRightOffsetM = stage.fHitAnchorRightOffsetM;
		boss.fPatternHitAnchorYawOffsetDegrees =
			stage.fHitAnchorYawOffsetDegrees;
		boss.ePatternHitActivationKind = stage.eHitActivationKind;
		boss.iPatternHitActivationStartMs = stage.iHitActivationStartMs;
		boss.iPatternHitActivationLifetimeMs = stage.iHitActivationLifetimeMs;
		boss.bPatternWallContact = stage.bWallContact;
		boss.bPatternPiercesCover = stage.bPiercesCover;
		boss.fPatternPushRangeM = stage.fPushRangeM;
		boss.iPatternPushMs = stage.iPushMs;
		boss.bPatternKnockdown = stage.bKnockdown;
		boss.iPatternDownMs = stage.iDownMs;
		boss.bPatternGroggy =
			BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind;
		boss.bPatternChargeImpact = stage.bChargeImpact;
		const float durationSeconds =
			static_cast<float>(stage.iDurationMs) * MILLISECONDS_TO_SECONDS;
		boss.fPatternForcedMotionSpeed =
			BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == stage.Motion.eKind &&
			durationSeconds > 0.f ?
			stage.Motion.fDistance / durationSeconds : 0.f;
		boss.PatternStageRootMotion =
			BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
				stage.Motion.eKind ?
				std::vector<ROOT_MOTION_SAMPLE>{} : stage.Motion.RootMotion;
		boss.eAction = ToServerAction(stage.eStageKind);
		boss.fActionElapsedSeconds = 0.f;
		boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		if (boss.fPatternLeapApexHeight > 0.f &&
			LEAP_TAKEOFF_STAGE_INDEX == stageIndex)
		{
			boss.fLeapOriginX = boss.fPositionX;
			boss.fLeapOriginY = boss.fPositionY;
			boss.fLeapOriginZ = boss.fPositionZ;
			boss.fLeapApexHeight = boss.fPatternLeapApexHeight;
			boss.MovePath.clear();
		}
		else if (boss.fPatternLeapApexHeight > 0.f &&
			stageIndex < boss.iPatternLeapTravelStageIndex)
		{
			// 하강 stage 전까지 공중 대기 stage는 같은 정점 높이와 수평 원점을 유지한다.
			boss.fPositionX = boss.fLeapOriginX;
			boss.fPositionY = boss.fLeapOriginY + boss.fLeapApexHeight;
			boss.fPositionZ = boss.fLeapOriginZ;
		}
		else if (boss.fLeapApexHeight > 0.f &&
			stageIndex > boss.iPatternLeapTravelStageIndex)
		{
			// 하강 stage를 떠날 때 착지점을 한 번 확정한다.
			// 이후 stage마다 이 위치를 다시 적용하면 후속 이동을 되돌리므로 정점 상태도 해제한다.
			boss.fPositionX = boss.fLeapLandingX;
			boss.fPositionY = boss.fLeapLandingY;
			boss.fPositionZ = boss.fLeapLandingZ;
			boss.fLeapApexHeight = 0.f;
		}
		Advance_ArenaBreakLeap(boss);
		if (BOSS_PATTERN_STAGE_MOTION_KIND::TO_ARENA_CENTER == stage.Motion.eKind)
		{
			// 명시적인 조준 시간 구간은 중앙으로 이동하는 동안에도 방향 갱신을 제어한다.
			// 특히 endMs=0인 후속 구간은 stage 진입만으로 다시 목표를 바라보면 안 된다.
			if (!stage.bHasAimEnd && !stage.bHasAimResponseScale)
				FacePoint(boss, boss.fSpawnPositionX, boss.fSpawnPositionZ);
			boss.fPatternStageOriginX = boss.fPositionX;
			boss.fPatternStageOriginZ = boss.fPositionZ;
			boss.fPatternStageOriginYawDegrees = boss.fYawDegrees;
			boss.MovePath.clear();
			boss.PatternStageRootMotion.clear();
		}
		if (0.f != stage.fVerticalOffsetM)
		{
			boss.fPatternStageVerticalBaseY = boss.fPositionY;
			boss.fPositionY = boss.fPatternStageVerticalBaseY +
				stage.fVerticalOffsetM;
			boss.bPatternStageVerticalOffsetApplied = true;
		}
	}

	// 선택 성공 또는 피날레 재시작이 실행 revision과 새 sequence를 확정하고 첫 stage에 진입한다.
	// Catalog에서 검증된 비어 있지 않은 Stages가 입력 전제이며, 재사용 대기는 선택 시도가 아닌 이 지점에서 시작한다.
	void BeginPattern(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		SERVER_PLAYER* nearestTarget,
		const LostArk::Shared::GameplayDataRevision& definitionRevision,
		const std::uint32_t serverTick)
	{
		// 이전 실행이 취소되었더라도 높이 보정과 반응 누적 상태를 새 패턴에 넘기지 않는다.
		// 일반 종료는 FinishPattern에서 정리하며, 여기서는 직접 재진입 경로도 같은 초기 상태로 맞춘다.
		RestorePatternVerticalOffset(boss);
		SetPatternBossResponseState(
			boss, BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE, 0u, 0u, false);
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		boss.PinnedDefinitionRevision = definitionRevision;
		boss.PatternTerminalReceipt = {};
		boss.iGrabExecutionCommittedPatternSequence = 0u;
		boss.iGrabExecutionCommittedStageIndex = 0u;
		boss.strPatternId = pattern.strPatternId;
		boss.iPatternStartTick = 0u == serverTick ? 1u : serverTick;
		boss.bPatternInvulnerable = pattern.bInvulnerableWhileRunning;
		boss.bPatternMoveToAnchorBeforeTakeoff =
			pattern.Motion.bMoveToAnchorBeforeTakeoff;
		boss.fPatternMinimumRange = pattern.fMinimumRange;
		boss.fPatternMaximumRange = pattern.fMaximumRange;
		if (0.f != pattern.fVerticalOffsetM)
		{
			boss.fPatternVerticalBaseY = boss.fPositionY;
			boss.fPositionY = boss.fPatternVerticalBaseY +
				pattern.fVerticalOffsetM;
			boss.bPatternVerticalOffsetApplied = true;
		}
		StartPatternCooldown(boss, pattern, serverTick);
		boss.iPatternSequence = boss.iPatternSequence ==
			(std::numeric_limits<std::uint32_t>::max)() ?
			1u : boss.iPatternSequence + 1u;
		MarkMechanicActive(boss, pattern.strPatternId, serverTick);
		if (boss.strLastPatternId == pattern.strPatternId)
			++boss.iConsecutivePatternUses;
		else
		{
			boss.strLastPatternId = pattern.strPatternId;
			boss.iConsecutivePatternUses = 1u;
		}
		// 대상 위치로 도약하는 패턴은 먼저 대상을 선택해야 그 위치를 착지점으로 저장할 수 있다.
		BeginPatternTargetAndAim(boss, pattern, players, nearestTarget);
		// LEAP_TO_TARGET은 저장한 대상 위치를, 나머지 도약은 저작된 착지 anchor를 사용한다.
		// 대상 위치가 없는 도약도 저작 anchor로 진행한다. 비도약 패턴은 도약 상태를 초기화한다.
		if (BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET == pattern.Motion.eKind &&
			boss.bHasPatternTargetLastPosition)
		{
			boss.fLeapLandingX = boss.fPatternTargetLastPositionX;
			boss.fLeapLandingY = boss.fPatternTargetLastPositionY;
			boss.fLeapLandingZ = boss.fPatternTargetLastPositionZ;
			boss.fPatternLeapApexHeight = pattern.Motion.fApexHeight;
			boss.iPatternLeapTravelStageIndex =
				pattern.Motion.iTravelStageIndex;
			boss.iPatternLeapTakeoffStartMs =
				pattern.Motion.iTakeoffStartMs;
			boss.iPatternLeapTakeoffEndMs =
				pattern.Motion.iTakeoffEndMs;
			boss.iPatternLeapTravelStartMs =
				pattern.Motion.iTravelStartMs;
			boss.iPatternLeapTravelEndMs =
				pattern.Motion.iTravelEndMs;
		}
		else if (Is_LeapPattern(pattern))
		{
			boss.fLeapLandingX = pattern.Motion.fLandingX;
			boss.fLeapLandingY = pattern.Motion.fLandingY;
			boss.fLeapLandingZ = pattern.Motion.fLandingZ;
			boss.fPatternLeapApexHeight = pattern.Motion.fApexHeight;
			boss.iPatternLeapTravelStageIndex =
				pattern.Motion.iTravelStageIndex;
			boss.iPatternLeapTakeoffStartMs =
				pattern.Motion.iTakeoffStartMs;
			boss.iPatternLeapTakeoffEndMs =
				pattern.Motion.iTakeoffEndMs;
			boss.iPatternLeapTravelStartMs =
				pattern.Motion.iTravelStartMs;
			boss.iPatternLeapTravelEndMs =
				pattern.Motion.iTravelEndMs;
		}
		else
		{
			boss.fLeapLandingX = boss.fSpawnPositionX;
			boss.fLeapLandingY = boss.fSpawnPositionY;
			boss.fLeapLandingZ = boss.fSpawnPositionZ;
			boss.fPatternLeapApexHeight = 0.f;
			boss.iPatternLeapTravelStageIndex = 1u;
			boss.iPatternLeapTakeoffStartMs = 0u;
			boss.iPatternLeapTakeoffEndMs = 0u;
			boss.iPatternLeapTravelStartMs = 0u;
			boss.iPatternLeapTravelEndMs = 0u;
		}
		// BeginPattern 뒤에도 같은 Update가 계속되므로 첫 stage는 진입 tick에 평가한다.
		// 그 밖의 stage 전환은 현재 Update를 끝내거나 마지막에 발생하므로 다음 tick부터 평가한다.
		EnterPatternStage(
			boss, pattern.Stages.front(), 0u, serverTick, true);
	}

	// stage 시작부터 누적된 전방 이동 거리(m)를 양쪽 시간 샘플 사이에서 선형 보간한다.
	// 첫/마지막 시간 밖에서는 끝 샘플 값을 유지한다.
	void Sample_StageRootMotionForward(
		const std::vector<ROOT_MOTION_SAMPLE>& samples,
		const float elapsedSeconds,
		float& outForward)
	{
		outForward = 0.f;
		if (samples.empty() || !std::isfinite(elapsedSeconds))
			return;
		const float elapsedMs = (std::max)(0.f, elapsedSeconds) * 1000.f;
		if (elapsedMs <= static_cast<float>(samples.front().iTimeMs))
		{
			outForward = samples.front().fForward;
			return;
		}
		for (std::size_t index = 1u; index < samples.size(); ++index)
		{
			const ROOT_MOTION_SAMPLE& previous = samples[index - 1u];
			const ROOT_MOTION_SAMPLE& current = samples[index];
			if (elapsedMs > static_cast<float>(current.iTimeMs))
				continue;
			const float span = static_cast<float>(
				current.iTimeMs - previous.iTimeMs);
			if (span <= 0.f)
			{
				outForward = current.fForward;
				return;
			}
			const float ratio =
				(elapsedMs - static_cast<float>(previous.iTimeMs)) / span;
			outForward = previous.fForward +
				(current.fForward - previous.fForward) * ratio;
			return;
		}
		outForward = samples.back().fForward;
	}

	// 정상 완료/중도 취소가 사용하는 공통 정리다. 완료 기록, 기믹 기록, 순서 커서를 갱신한 뒤 실행 상태를 비운다.
	// 대상 부재는 같은 항목 재시도를 보존하지만 그 밖의 순서형 실패는 reset을 요구한다.
	// 플레이어 잡기 부착의 실제 해제나 stage action commit은 이 함수를 호출한 GameRoom의 후속 책임이다.
	void FinishPattern(
		SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick,
		const bool completed = false,
		const SERVER_BOSS_MECHANIC_FAILURE failure =
			SERVER_BOSS_MECHANIC_FAILURE::NONE)
	{
		RestorePatternVerticalOffset(boss);
		if (!boss.strPatternId.empty())
		{
			boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
			boss.PatternTerminalReceipt.iRootPatternSequence =
				0u == boss.iPatternFollowupDepth ? boss.iPatternSequence :
				boss.iPatternFollowupRootSequence;
			boss.PatternTerminalReceipt.eResult = completed ?
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED :
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED;
		}
		boss.iGrabExecutionCommittedPatternSequence = 0u;
		boss.iGrabExecutionCommittedStageIndex = 0u;
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		const bool waitForTarget = !completed &&
			SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET == failure;
		if (completed)
			CompleteActiveMechanic(boss, boss.strPatternId, serverTick);
		else
			FailMechanic(boss, boss.strPatternId, failure, serverTick);
		if (boss.bAutomaticPatternSequenceStepRunning)
		{
			if (completed && !boss.strPatternId.empty() &&
				boss.iRotationStepIndex <
					(std::numeric_limits<std::uint32_t>::max)())
			{
				++boss.iRotationStepIndex;
				boss.iAutomaticPatternSequencePursuitTicksRemaining =
					boss.iAutomaticPatternSequenceInterStepPursuitTicks;
			}
			else if (waitForTarget)
			{
				// 순서형 커서를 현재 실행 항목에 둔다.
				// 새로 허용된 대상이 생기면 같은 항목을 다시 시작하며 일시적인 대상 부재를 정의 오류로 보지 않는다.
				boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			}
			else
			{
				// 그 밖의 순서형 실행 실패는 다음 항목으로 건너뛰지 않고 reset 필요 상태로 남긴다.
				boss.bMechanicLedgerRequiresReset = true;
				boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			}
			boss.bAutomaticPatternSequenceStepRunning = false;
		}
		if (boss.fLeapApexHeight > 0.f)
		{
			// 도약이 취소되어도 몸체가 공중에 남지 않도록 저장된 착지점에 놓는다.
			boss.fPositionX = boss.fLeapLandingX;
			boss.fPositionY = boss.fLeapLandingY;
			boss.fPositionZ = boss.fLeapLandingZ;
			boss.fLeapApexHeight = 0.f;
		}
		boss.strPatternId.clear();
		boss.iPatternStartTick = 0u;
		boss.strPatternStageId.clear();
		boss.strActionId.clear();
		boss.strDamageProfileId.clear();
		boss.iPatternStageIndex = 0u;
		boss.iPatternStageDurationMs = 0u;
		boss.iPatternStageFirstEvaluationTick = 0u;
		boss.fPatternForcedMotionSpeed = 0.f;
		boss.iPatternLeapTravelStageIndex = 1u;
		boss.ePatternStageMotionKind = BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		boss.PatternStageRootMotion.clear();
		boss.bPortalMotionActive = false;
		boss.bPortalRushTargetLocked = false;
		boss.PortalStageHitTargets.clear();
		boss.PatternActiveWindowHitTargets.clear();
		boss.fPortalLastHitSampleX = 0.f;
		boss.fPortalLastHitSampleZ = 0.f;
		boss.bPatternMoveToAnchorBeforeTakeoff = false;
		boss.ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		boss.ePatternPlayerResponse = BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		boss.ePatternAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		boss.ePatternPartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		boss.bPatternHasCounterProxy = false;
		boss.ePatternCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
		boss.fPatternCounterProxyForwardOffsetM = 0.f;
		boss.fPatternCounterProxyRightOffsetM = 0.f;
		boss.fPatternCounterProxyRadiusM = 0.f;
		boss.fPatternCounterProxyArcDegrees = 0.f;
		SetPatternBossResponseState(
			boss, BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE, 0u, 0u, false);
		boss.iPatternHitCount = 0u;
		boss.iPatternHitDelayMs = 0u;
		boss.iAppliedPatternHitCount = 0u;
		boss.ePatternHitAnchorKind =
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		boss.fPatternHitAnchorForwardOffsetM = 0.f;
		boss.fPatternHitAnchorRightOffsetM = 0.f;
		boss.fPatternHitAnchorYawOffsetDegrees = 0.f;
		boss.ePatternHitActivationKind =
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		boss.iPatternHitActivationStartMs = 0u;
		boss.iPatternHitActivationLifetimeMs = 0u;
		boss.fPatternStageOriginX = 0.f;
		boss.fPatternStageOriginZ = 0.f;
		boss.fPatternStageOriginYawDegrees = 0.f;
		boss.bPatternWallContact = false;
		boss.bPatternPiercesCover = false;
		boss.fPatternPushRangeM = 0.f;
		boss.iPatternPushMs = 0u;
		boss.bPatternKnockdown = false;
		boss.iPatternDownMs = 0u;
		boss.bPatternGroggy = false;
		boss.bPatternChargeImpact = false;
		boss.bPatternInvulnerable = false;
		boss.bPendingArmorBreakReaction = false;
		// 종료는 대상과 이동 경로의 소유 경계이기도 하다.
		// 잡기 실패 뒤에 잠근 대상이나 이전 추적 경로가 남지 않도록 모두 해제한다.
		// 다음 Update가 현재 Server 위치를 기준으로 대상을 다시 찾고 경로를 계산한다.
		boss.iTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.iPatternTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		ClearPatternTargetPosition(boss);
		boss.MovePath.clear();
		boss.iMovePathIndex = 0u;
		CBossCombatRuntime::Clear_PatternOutcomes(boss);
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
	}

	// 게시된 결과/시간 만료/외부 충돌이 선택한 분기를 실행한다.
	// 다음 action, 후속 pattern 또는 종료 중 하나로 연결하며, 잘못된 목적지는 공통 취소 경로로 정리한다.
	// true는 분기를 처리했다는 뜻으로 정상 완료만을 뜻하지 않는다.
	bool ApplyStageBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_BRANCH& branch,
		const std::uint32_t serverTick)
	{
		if (!branch.strNextActionId.empty() &&
			!branch.strNextPatternId.empty())
		{
			boss.bMechanicLedgerRequiresReset = true;
			FinishPattern(
				boss, serverTick, false,
				SERVER_BOSS_MECHANIC_FAILURE::INVALID_RUNNING_DEFINITION);
			return true;
		}
		if (!branch.strNextPatternId.empty())
		{
			const std::uint32_t nextDepth =
				static_cast<std::uint32_t>(boss.iPatternFollowupDepth) + 1u;
			if (boss.PendingPatternFollowup.Is_Pending() ||
				!boss.PinnedDefinitionRevision.Is_Valid() ||
				nextDepth > MAX_PATTERN_FOLLOWUP_DEPTH ||
				0u == boss.iPatternSequence)
			{
				boss.bMechanicLedgerRequiresReset = true;
				FinishPattern(
					boss, serverTick, false,
					SERVER_BOSS_MECHANIC_FAILURE::INVALID_RUNNING_DEFINITION);
				return true;
			}
			boss.PendingPatternFollowup.strPatternId =
				branch.strNextPatternId;
			boss.PendingPatternFollowup.PinnedDefinitionRevision =
				boss.PinnedDefinitionRevision;
			boss.PendingPatternFollowup.iSourcePatternSequence =
				boss.iPatternSequence;
			boss.PendingPatternFollowup.iRootPatternSequence =
				0u == boss.iPatternFollowupDepth ? boss.iPatternSequence :
				boss.iPatternFollowupRootSequence;
			boss.PendingPatternFollowup.iDepth =
				static_cast<std::uint8_t>(nextDepth);
			// 순서형 커서는 FinishPattern이 한 번 진행한다.
			// 후속 패턴은 다음 tick에 정확한 완료 기록을 확인해 선택하므로 같은 커서를 다시 진행하지 않는다.
			FinishPattern(boss, serverTick, true);
			return true;
		}
		if (branch.strNextActionId.empty())
		{
			FinishPattern(boss, serverTick, true);
			return true;
		}
		std::uint32_t nextStageIndex = 0u;
		const BOSS_PATTERN_STAGE_DEFINITION* next =
			FindStageByActionId(pattern, branch.strNextActionId, nextStageIndex);
		if (nullptr == next)
		{
			// 분기 목적 stage를 찾지 못하면 현재 패턴을 취소한다.
			// 끝나지 않는 stage에 머무르지 않도록 공통 종료 경로로 상태를 정리한다.
			FinishPattern(
				boss, serverTick, false,
				SERVER_BOSS_MECHANIC_FAILURE::INVALID_RUNNING_DEFINITION);
			return true;
		}
		EnterPatternStage(boss, *next, nextStageIndex, serverTick);
		return true;
	}

	// Update가 BossCombatRuntime에 게시된 현재 action의 결과를 정의 순서로 소비한다.
	// 잡힌 인원과 TIMEOUT 분기는 여기서 제외하고 stage 마감 경로에서 처리한다.
	bool ApplyPublishedOutcomeBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t serverTick)
	{
		// 게시된 판정 결과는 같은 tick의 시간 만료보다 먼저 처리한다.
		// TIMEOUT은 이 Update에서 시간과 공격 판정을 처리한 뒤 검사한다.
		for (const BOSS_PATTERN_STAGE_BRANCH& branch : stage.Branches)
		{
			if (BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == branch.eOutcome ||
				BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED == branch.eOutcome ||
				BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED == branch.eOutcome ||
				!CBossCombatRuntime::Consume_PatternOutcome(
					boss, stage.strActionId, branch.eOutcome))
			{
				continue;
			}
			if (BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED ==
				branch.eOutcome)
			{
				// PART_DESTROYED 분기가 같은 파괴 결과를 소비했다.
				// PlayerSkillSystem이 남긴 기존 갑옷 파괴 반응 플래그도 함께 해제한다.
				boss.bPendingArmorBreakReaction = false;
			}
			return ApplyStageBranch(boss, pattern, branch, serverTick);
		}
		return false;
	}

	// Update가 피해 도형 없는 카운터 창을 일반 플레이어/보스 카운터 경로에 연결한다.
	// 플레이어 복사본에서 스킬 응답을 검사하고 보스 승인 후에만 원본 플레이어를 교체한다.
	bool TryConsumeDamageLessCounterProxy(
		SERVER_WORLD_ENTITY& boss,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const CGameplayCatalog& catalog,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::uint32_t serverTick,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
	{
		const bool ownsCounterBranch = std::any_of(
			stage.Branches.begin(), stage.Branches.end(),
			[](const BOSS_PATTERN_STAGE_BRANCH& branch)
			{
				return BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT ==
					branch.eOutcome;
			});
		// counter proxy는 일반 피해 도형이 없는 stage에서 방어형 COUNTER 상호작용을 연결한다.
		// 카운터 허용 여부와 도형 판단은 기존 Apply_PlayerHit 경로가 소유한다.
		// stage 정의와 현재 실행 상태가 일치할 때만 이 무피해 입력을 전달한다.
		if (!ownsCounterBranch ||
			BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
			!stage.bHasCounterProxy || !boss.bPatternHasCounterProxy ||
			boss.strActionId != stage.strActionId ||
			boss.ePatternCounterProxyKind != stage.eCounterProxyKind ||
			boss.fPatternCounterProxyForwardOffsetM !=
				stage.fCounterProxyForwardOffsetM ||
			boss.fPatternCounterProxyRightOffsetM !=
				stage.fCounterProxyRightOffsetM ||
			boss.fPatternCounterProxyRadiusM != stage.fCounterProxyRadiusM ||
			boss.fPatternCounterProxyArcDegrees != stage.fCounterProxyArcDegrees ||
			!CBossCombatRuntime::Has_Flag(
				boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE))
		{
			return false;
		}

		const BOSS_RUNTIME_PROFILE* bossProfile =
			catalog.Find_Boss(boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ?
                "BOSS_VALTAN_GHOST" : boss.strArchetypeId);
		if (nullptr == bossProfile ||
			!std::isfinite(bossProfile->fCollisionRadius) ||
			bossProfile->fCollisionRadius <= 0.f)
		{
			return false;
		}

		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (player.Is_Guide() || 0u == player.iCurrentHp || !player.isCombatReady ||
				!std::isfinite(player.fPositionX) ||
				!std::isfinite(player.fPositionZ))
			{
				continue;
			}
			const PLAYER_SKILL_DEFINITION* skill =
				catalog.Find_Skill(player.iCurrentSkillId);
			if (nullptr == skill || 0u == skill->iCounterPower ||
				LostArk::Shared::PLAYER_SKILL_KIND::COUNTER !=
					skill->eSkillKind ||
				!std::isfinite(skill->fMaximumRange) ||
				skill->fMaximumRange <= 0.f ||
				!LostArk::Shared::CombatCollision::Circles_Overlap(
					LostArk::Shared::CombatCollision::CIRCLE_XZ{
						player.fPositionX, player.fPositionZ,
						skill->fMaximumRange },
					LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ{
						boss.fPositionX, boss.fPositionZ,
						bossProfile->fCollisionRadius }))
			{
				continue;
			}

			// Try_Counter는 플레이어 action을 바꾸므로 우선 복사본에서 수행한다.
			// 보스의 기존 카운터 처리도 성공한 뒤에만 실제 플레이어에 반영한다.
			SERVER_PLAYER stagedPlayer = player;
			if (!CPlayerSkillSystem::Try_Counter(
				stagedPlayer, catalog, serverTick))
			{
				continue;
			}
			BOSS_INCOMING_HIT counterInteraction{};
			counterInteraction.iSourcePlayerId = player.iPlayerId;
			counterInteraction.iSkillId = skill->iSkillId;
			counterInteraction.iCounterPower = skill->iCounterPower;
			counterInteraction.bCounterFromPrimarySlot = Is_PrimaryCounterSkill(*skill);
			counterInteraction.bCounterFromActiveGuard = true;
			counterInteraction.iServerTick = serverTick;
			counterInteraction.fSourceX = player.fPositionX;
			counterInteraction.fSourceZ = player.fPositionZ;
			if (!CBossCombatRuntime::Apply_PlayerHit(
				boss, counterInteraction).bCounterTriggered)
			{
				continue;
			}
			if (outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS &&
				(boss.strPatternId == "VALTAN_TRASH" || boss.strPatternId == "VALTAN_TRIPLE_COUNTER"))
			{
				LostArk::Shared::DAMAGE_EVENT success{};
				success.iTargetNetEntityId = boss.iNetEntityId;
				success.iSourcePlayerId = player.iPlayerId;
				success.fPositionX = boss.fPositionX;
				success.fPositionY = boss.fPositionY;
				success.fPositionZ = boss.fPositionZ;
				success.isOutgoing = true;
				success.isCounterSuccess = true;
				outDamageEvents.push_back(success);
			}
			player = std::move(stagedPlayer);
			return true;
		}
		return false;
	}

	// stage 마감에서 확정된 잡기 명단을 먼저 정의 순서로 검사하고, 해당 분기가 없으면 TIMEOUT을 찾는다.
	bool ApplyTimeoutBranch(
		SERVER_WORLD_ENTITY& boss,
		const BOSS_PATTERN_DEFINITION& pattern,
		const BOSS_PATTERN_STAGE_DEFINITION& stage,
		const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const std::uint32_t serverTick)
	{
		// 잡힌 인원에 따른 분기는 stage의 마감에서 평가한다.
		// 이전 tick의 잡기 확정 결과를 사용하며 잡기 성공만으로 저작된 돌진 시간을 생략하지 않는다.
		const SERVER_BOSS_GRAB_ROSTER roster =
			CValtanBrain::Classify_GrabbedPlayers(boss, players);
		for (const BOSS_PATTERN_STAGE_BRANCH& candidate : stage.Branches)
		{
			const bool all = BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED ==
				candidate.eOutcome && SERVER_BOSS_GRAB_CLASSIFICATION::ALL ==
				roster.eClassification;
			const bool any = BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED ==
				candidate.eOutcome && 0u != roster.iGrabbedCount;
			if (all || any)
				return ApplyStageBranch(boss, pattern, candidate, serverTick);
		}
		const auto branch = std::find_if(
			stage.Branches.begin(), stage.Branches.end(),
			[](const BOSS_PATTERN_STAGE_BRANCH& candidate)
			{
				return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
					candidate.eOutcome;
			});
		return stage.Branches.end() != branch &&
			ApplyStageBranch(boss, pattern, *branch, serverTick);
	}

	// Update의 전투 이탈 거리 상한을 위해 정의 목록의 최대 사거리를 구한다.
	float MaximumPatternRange(
		const std::vector<BOSS_PATTERN_DEFINITION>& patterns)
	{
		float maximum = 0.f;
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
			maximum = (std::max)(maximum, pattern.fMaximumRange);
		return maximum;
	}

	// 패턴을 고르지 못한 CHASE 상태가 이번 고정 시간만큼 navigation 경로를 진행한다.
	// 허용된 이동을 한 번 수행하면 true, 경로 완료/구간 실패 때는 경로를 비우고 false다.
	bool MoveAlongPath(
		SERVER_WORLD_ENTITY& boss,
		const CServerNavigation& navigation,
		const float fixedDeltaSeconds)
	{
		// 경로를 만든 뒤 바닥이 붕괴할 수 있으므로 이번 이동 구간을 현재 navigation으로 다시 검사한다.
		// 붕괴 구역 위의 보스는 이전 높이를 유지하며 같은 높이의 복구 지점으로 이동한다.
		// 모든 중간 샘플이 통과한 뒤에만 몸체 위치를 확정한다.
		const auto commitStep = [&boss, &navigation](
			const float x, const float z, const float detachedY)
			{
				if (!navigation.Is_Loaded())
				{
					boss.fPositionX = x;
					boss.fPositionY = detachedY;
					boss.fPositionZ = z;
					return true;
				}
				SERVER_NAV_POINT previous{
					boss.fPositionX, boss.fPositionY, boss.fPositionZ };
				bool recovering = navigation.Is_PointInVoidRegion(
					previous.x, previous.z);
				SERVER_NAV_POINT source{};
				if (!(recovering ? navigation.Sample_Position(previous.x, previous.z, source, previous.y) :
					navigation.Sample_SurfacePosition(previous.x, previous.z, source)) ||
					!navigation.Is_HeightTransitionAllowed(previous.y, source.y))
					return false;
				const float dx = x - previous.x, dz = z - previous.z;
				const float distance = std::hypot(dx, dz);
				const float sampleStep = (std::max)(0.01f, navigation.Get_CellSize() * 0.5f);
				if (!std::isfinite(distance) || !std::isfinite(sampleStep))
					return false;
				const auto count = static_cast<std::uint32_t>((std::max)(1.f, std::ceil(distance / sampleStep)));
				for (std::uint32_t sample = 1u; sample <= count; ++sample)
				{
					const float fraction = static_cast<float>(sample) / count;
					const float nextX = boss.fPositionX + dx * fraction;
					const float nextZ = boss.fPositionZ + dz * fraction;
					const bool overVoid = navigation.Is_PointInVoidRegion(nextX, nextZ);
					const bool walkable = navigation.Is_PointWalkableExact(nextX, nextZ, previous.y);
					SERVER_NAV_POINT next{};
					if (!walkable && !(recovering && overVoid))
						return false;
					if (recovering)
					{
						if (!navigation.Sample_Position(nextX, nextZ, next, previous.y) ||
							std::abs(next.y - previous.y) > 1.f)
							return false;
						if (overVoid)
							next.y = previous.y;
						else
							recovering = false;
					}
					else if (!navigation.Sample_SurfacePosition(nextX, nextZ, next) ||
						!navigation.Is_HeightTransitionAllowed(previous.y, next.y) ||
						!navigation.Resolve_TraversalStep(previous.x, previous.z, nextX, nextZ, next, previous.y))
						return false;
					previous = next;
				}
				boss.fPositionX = previous.x;
				boss.fPositionY = previous.y;
				boss.fPositionZ = previous.z;
				return true;
			};
		while (boss.iMovePathIndex < boss.MovePath.size())
		{
			const SERVER_NAV_POINT& point = boss.MovePath[boss.iMovePathIndex];
			const float deltaX = point.x - boss.fPositionX;
			const float deltaZ = point.z - boss.fPositionZ;
			const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
			if (distance <= PATH_POINT_STOP_DISTANCE)
			{
				if (!commitStep(point.x, point.z, point.y))
					break;
				++boss.iMovePathIndex;
				continue;
			}
			const float step =
				(std::min)(boss.fMoveSpeed * fixedDeltaSeconds, distance);
			if (!commitStep(boss.fPositionX + deltaX / distance * step,
				boss.fPositionZ + deltaZ / distance * step, boss.fPositionY))
				break;
			boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			return true;
		}
		boss.MovePath.clear();
		boss.iMovePathIndex = 0;
		return false;
	}

	// 아래 판정 원점/방향은 피해 도형과 엄폐 검사가 함께 사용한다.
	// 엄폐는 판정 원점에서 플레이어까지의 선분과 살아 있는 엄폐 원의 교차로 검사한다.
	struct PATTERN_HIT_TRANSFORM final
	{
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
	};

	// 현재 boss 위치 또는 stage 시작 위치 중 선언된 기준을 골라 도형 원점을 만든다.
	// 전방/오른쪽 offset은 m, yaw offset은 degree이며 결과는 명중과 엄폐 검사에서 공유한다.
	PATTERN_HIT_TRANSFORM ResolvePatternHitTransform(
		const SERVER_WORLD_ENTITY& boss)
	{
		const bool stageOrigin =
			BOSS_PATTERN_HIT_ANCHOR_KIND::STAGE_ORIGIN ==
				boss.ePatternHitAnchorKind;
		const float basisX = stageOrigin ?
			boss.fPatternStageOriginX : boss.fPositionX;
		const float basisZ = stageOrigin ?
			boss.fPatternStageOriginZ : boss.fPositionZ;
		const float basisYaw = stageOrigin ?
			boss.fPatternStageOriginYawDegrees : boss.fYawDegrees;
		const float yawRadians = basisYaw * DEGREES_TO_RADIANS;
		const float forwardX = std::sin(yawRadians);
		const float forwardZ = std::cos(yawRadians);
		const float rightX = std::cos(yawRadians);
		const float rightZ = -std::sin(yawRadians);
		return {
			basisX + forwardX * boss.fPatternHitAnchorForwardOffsetM +
				rightX * boss.fPatternHitAnchorRightOffsetM,
			basisZ + forwardZ * boss.fPatternHitAnchorForwardOffsetM +
				rightZ * boss.fPatternHitAnchorRightOffsetM,
			basisYaw + boss.fPatternHitAnchorYawOffsetDegrees
		};
	}

	// ApplyPatternHit이 엄폐 관통이 아닌 공격에만 호출한다. 판정 원점과 플레이어 사이를 막는 원이 하나라도 있는지 읽는다.
	bool IsShieldedByCover(
		const PATTERN_HIT_TRANSFORM& hitTransform,
		const SERVER_PLAYER& player,
		const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
			coverCircles)
	{
		return std::any_of(
			coverCircles.begin(), coverCircles.end(),
			[&hitTransform, &player](
				const LostArk::Shared::CombatCollision::CIRCLE_XZ& circle)
			{
				return LostArk::Shared::CombatCollision::Segment_IntersectsCircle(
					hitTransform.fPositionX, hitTransform.fPositionZ,
					player.fPositionX, player.fPositionZ, circle);
			});
	}

	// 기존 stage 공통 도형과 플레이어 몸체 원의 XZ 교차를 검사한다. HP나 판정 횟수는 변경하지 않는다.
	// 포탈 BOX는 마지막 판정 위치부터 현재 위치까지의 전진 구간을 길이에 포함한다.
	bool ContainsPatternHit(
		const SERVER_WORLD_ENTITY& boss,
		const SERVER_PLAYER& player,
		const PATTERN_HIT_TRANSFORM& hitTransform)
	{
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ playerBody{
			player.fPositionX,
			player.fPositionZ,
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X
		};
		const float yawRadians =
			hitTransform.fYawDegrees * DEGREES_TO_RADIANS;
		const float forwardX = std::sin(yawRadians);
		const float forwardZ = std::cos(yawRadians);
		switch (boss.ePatternHitShape)
		{
		case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			return LostArk::Shared::CombatCollision::Circles_Overlap(
				LostArk::Shared::CombatCollision::CIRCLE_XZ{
					hitTransform.fPositionX,
					hitTransform.fPositionZ,
					boss.fPatternHitOuterRadius
				},
				playerBody);
		case BOSS_PATTERN_HIT_SHAPE::RING:
			return LostArk::Shared::CombatCollision::Circle_IntersectsRing(
				playerBody,
				hitTransform.fPositionX,
				hitTransform.fPositionZ,
				boss.fPatternHitInnerRadius,
				boss.fPatternHitOuterRadius);
		case BOSS_PATTERN_HIT_SHAPE::CONE:
			return LostArk::Shared::CombatCollision::Circle_IntersectsCone(
				playerBody,
				hitTransform.fPositionX,
				hitTransform.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitAngleDegrees);
		case BOSS_PATTERN_HIT_SHAPE::BOX:
		{
			const float portalTravel = boss.bPortalMotionActive ?
				(std::max)(0.f,
					(boss.fPositionX - boss.fPortalLastHitSampleX) * forwardX +
					(boss.fPositionZ - boss.fPortalLastHitSampleZ) * forwardZ) : 0.f;
			return LostArk::Shared::CombatCollision::Circle_IntersectsForwardBox(
				playerBody,
				boss.bPortalMotionActive ? boss.fPortalLastHitSampleX : hitTransform.fPositionX,
				boss.bPortalMotionActive ? boss.fPortalLastHitSampleZ : hitTransform.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength + portalTravel,
				boss.fPatternHitHalfWidth);
		}
		case BOSS_PATTERN_HIT_SHAPE::CROSS:
			return LostArk::Shared::CombatCollision::Circle_IntersectsCross(
				playerBody,
				hitTransform.fPositionX,
				hitTransform.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitHalfWidth);
		case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
			return LostArk::Shared::CombatCollision::Circle_IntersectsSixDirections(
				playerBody,
				hitTransform.fPositionX,
				hitTransform.fPositionZ,
				forwardX,
				forwardZ,
				boss.fPatternHitLength,
				boss.fPatternHitHalfWidth);
		default:
			return false;
		}
	}

	// stage 판정 기준 위에 개별 contact의 지역 offset과 yaw를 더한다. 입력 base는 값 복사라 원본 boss는 바뀌지 않는다.
	PATTERN_HIT_TRANSFORM ResolveContactTransform(PATTERN_HIT_TRANSFORM base,
		const LostArk::Shared::ATTACK_HIT_TEMPLATE& contact)
	{
		const float yaw = base.fYawDegrees * DEGREES_TO_RADIANS;
		base.fPositionX += std::sin(yaw) * float(contact.fOffsetForwardM) + std::cos(yaw) * float(contact.fOffsetRightM);
		base.fPositionZ += std::cos(yaw) * float(contact.fOffsetForwardM) - std::sin(yaw) * float(contact.fOffsetRightM);
		base.fYawDegrees += float(contact.fYawOffsetDegrees);
		return base;
	}

	// 개별 AttackContacts 도형을 공용 ServerCombatGeometry 형식으로 옮겨 몸체와 교차 검사한다.
	// contact의 저장 시간/범위 검증은 Catalog 진입에서 끝났다는 전제로 도형을 선택한다.
	bool ContainsStageContact(const LostArk::Shared::ATTACK_HIT_TEMPLATE& contact,
		const SERVER_PLAYER& player, const PATTERN_HIT_TRANSFORM& pose)
	{
		SERVER_COMBAT_SHAPE_XZ shape{};
		shape.eKind = contact.strShape == "CIRCLE" ? SERVER_COMBAT_SHAPE_KIND::CIRCLE :
			contact.strShape == "RING" ? SERVER_COMBAT_SHAPE_KIND::RING :
			contact.strShape == "BOX" ? SERVER_COMBAT_SHAPE_KIND::FORWARD_BOX : SERVER_COMBAT_SHAPE_KIND::CONE;
		if (contact.strShape == "CIRCLE" || contact.strShape == "RING")
			shape.fOuterRadius = float(contact.fRadiusM);
		if (contact.strShape == "RING" || contact.strShape == "CONE")
			shape.fInnerRadius = float(contact.fInnerRadiusM);
		if (contact.strShape == "BOX" || contact.strShape == "CONE")
			shape.fLength = float(contact.fLengthM);
		if (contact.strShape == "BOX")
			shape.fHalfWidth = float(contact.fHalfWidthM);
		if (contact.strShape == "CONE")
			shape.fAngleDegrees = float(contact.fAngleDegrees);
		const float yaw = pose.fYawDegrees * DEGREES_TO_RADIANS;
		return CServerCombatGeometry::Overlaps_Pose(shape, pose.fPositionX, pose.fPositionZ,
			std::sin(yaw), std::cos(yaw), {player.fPositionX, player.fPositionZ,
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X});
	}

	// Update가 예약 기회 또는 활성 구간마다 호출하는 실제 타격 경로다.
	// 개별 contact가 있으면 그 도형/피해/밀기 정의를 사용하고 없으면 boss의 stage 공통 정의를 사용한다.
	// 대상별 중복/도형/엄폐/카운터 검사 후 ServerCombatHitRuntime이 HP와 피해 event를 처리한다.
	// 잡기 요청은 출력 vector에 모으고 GameRoom이 확정하며, 이 함수는 예약 기회 카운터를 증가시키지 않는다.
	void ApplyPatternHit(
		SERVER_WORLD_ENTITY& boss,
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		const CGameplayCatalog& catalog,
		const std::uint32_t serverTick,
		const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
			coverCircles,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
		std::vector<SERVER_PLAYER_CAPTURE_REQUEST>* outCaptureRequests,
		const LostArk::Shared::ATTACK_HIT_TEMPLATE* contact = nullptr)
	{
		if (nullptr == contact && (BOSS_PATTERN_HIT_SHAPE::NONE == boss.ePatternHitShape ||
			boss.strDamageProfileId.empty()))
		{
			return;
		}
		const BOSS_RUNTIME_PROFILE* bossProfile =
			catalog.Find_Boss(boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ?
                "BOSS_VALTAN_GHOST" : boss.strArchetypeId);
		const bool encounterWipe =
			("VALTAN_TRIPLE_COUNTER" == boss.strPatternId && "FAIL_3" == boss.strPatternStageId) ||
			("VALTAN_STAGGER_SLOT" == boss.strPatternId && "FINAL_ATTACK" == boss.strPatternStageId);
		const auto& damageProfile = contact ? contact->strDamageProfileId : boss.strDamageProfileId;
		const bool ignoreDefense = contact && contact->strDamageKind != "PROFILE";
		const bool instantDeath = contact && (contact->strDamageKind == "INSTANT_DEATH" ||
			(contact->strDamageKind == "MAX_HP_PERCENT" && contact->iDamagePercent >= 100u));
		const std::uint32_t rawDamage = CGameplayCatalog::Resolve_Damage(
			nullptr == bossProfile ? 0u : bossProfile->iAttackPower,
			catalog.Find_DamageRatePercent(damageProfile));
		const PATTERN_HIT_TRANSFORM hitTransform = contact ?
			ResolveContactTransform(ResolvePatternHitTransform(boss), *contact) : ResolvePatternHitTransform(boss);
		const bool activeWindow =
			BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ==
				boss.ePatternHitActivationKind;
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			const bool alreadyHit = activeWindow ?
				boss.PatternActiveWindowHitTargets.end() != std::find(
					boss.PatternActiveWindowHitTargets.begin(),
					boss.PatternActiveWindowHitTargets.end(), player.iNetEntityId) :
				(boss.bPortalMotionActive &&
				 boss.PortalStageHitTargets.end() != std::find(
					boss.PortalStageHitTargets.begin(),
					boss.PortalStageHitTargets.end(), player.iNetEntityId));
			// 일반 카운터가 성공하면 피해 대신 응답한 것이므로 피해 적용보다 먼저 검사한다.
			if (0u == player.iCurrentHp || (encounterWipe && player.Is_Guide()) ||
				(!encounterWipe && ((!instantDeath && !player.isCombatReady) || alreadyHit ||
				 !(contact ? ContainsStageContact(*contact, player, hitTransform) : ContainsPatternHit(boss, player, hitTransform)) ||
				 (!boss.bPatternPiercesCover &&
					IsShieldedByCover(hitTransform, player, coverCircles)))))
			{
				continue;
			}
			if (!encounterWipe && !ignoreDefense && player.Is_Human() && CPlayerSkillSystem::Try_Counter(player, catalog, serverTick))
			{
				(void)CBossCombatRuntime::Try_TriggerCounter(boss, serverTick);
				continue;
			}
			SERVER_WORLD_TO_PLAYER_HIT incoming{};
			incoming.iRawDamage = !ignoreDefense ? rawDamage :
				contact->strDamageKind == "INSTANT_DEATH" ? player.iMaximumHp :
				(std::max)(1u, static_cast<std::uint32_t>(std::uint64_t(player.iMaximumHp) * contact->iDamagePercent / 100u));
			incoming.bEncounterWipe = encounterWipe;
			incoming.bInstantDeath = instantDeath;
			incoming.bIgnoreDefense = ignoreDefense;
			incoming.bIgnoreCounter = ignoreDefense;
			incoming.fSourceX = hitTransform.fPositionX;
			incoming.fSourceZ = hitTransform.fPositionZ;
			incoming.fPushRangeM = boss.fPatternPushRangeM;
			incoming.iPushMs = boss.iPatternPushMs;
			incoming.bKnockdown = boss.bPatternKnockdown;
			incoming.iDownMs = boss.iPatternDownMs;
			if (contact)
			{
				incoming.fPushRangeM = float(contact->fPushRangeM);
				incoming.iPushMs = contact->iPushMs;
				incoming.fPushHeightM = float(contact->fRiseHeightM);
				incoming.bPushBallistic = contact->fRiseHeightM > 0.0;
				incoming.bForcePush = contact->ForcePush.value_or(contact->fRiseHeightM > 0.0);
				incoming.bKnockdown = false;
				incoming.iDownMs = 0u;
				if (contact->strPushDirection == "AWAY_FROM_BOSS")
				{
					incoming.fSourceX = boss.fPositionX;
					incoming.fSourceZ = boss.fPositionZ;
				}
			}
			incoming.iServerTick = serverTick;
			incoming.bEstherGuardBlockable =
				LostArk::Shared::EstherStrike::Is_GuardBlockedDamageProfile(damageProfile.c_str());
			const SERVER_COMBAT_HIT_RESULT hitResult =
				CServerCombatHitRuntime::Apply_WorldToPlayer(
					player, incoming, catalog, outDamageEvents);
			if (boss.bPortalMotionActive &&
				SERVER_COMBAT_HIT_RESULT::LANDED == hitResult)
				boss.PortalStageHitTargets.push_back(player.iNetEntityId);
			if (activeWindow && SERVER_COMBAT_HIT_RESULT::LANDED == hitResult)
				boss.PatternActiveWindowHitTargets.push_back(player.iNetEntityId);
			if (SERVER_COMBAT_HIT_RESULT::LANDED == hitResult &&
				BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
					boss.ePatternPlayerResponse &&
				LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
					boss.ePatternAttachmentSlot &&
				nullptr != outCaptureRequests)
			{
				outCaptureRequests->push_back({
					player.iNetEntityId, boss.ePatternAttachmentSlot });
			}
		}
	}
}

// CGameRoom::Predict_GuideContactRisk가 가이드의 후보 위치에서 향후 horizonMs 동안의 위험도를 묻는다.
// 미래 이동을 시뮬레이션하지 않고 현재 확정 위치로 도형을 검사하므로 실제 명중 예측 보장은 아니다.
// boss/players/HP는 바꾸지 않고 임시 sampled와 위험도 합계만 계산한다.
float LostArk::Server::CValtanBrain::Predict_ContactRisk(
    const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
    const SERVER_PLAYER& probe, const std::uint32_t serverTick, const std::uint32_t horizonMs)
{
    if (!boss.iCurrentHp || boss.strPatternId != pattern.strPatternId ||
        boss.iPatternStageIndex >= pattern.Stages.size() || !boss.iActionStartTick)
        return 0.f;
    // 미래 stage는 현재 확정된 보스 위치를 기준으로 위험도를 추정한다.
    // 이 값은 이동 후보 평가용이며 실제 명중과 피해는 ApplyPatternHit 경로에서 결정한다.
    SERVER_WORLD_ENTITY sampled;
    sampled.fPositionX = boss.fPositionX;
    sampled.fPositionZ = boss.fPositionZ;
    sampled.fYawDegrees = boss.fYawDegrees;
    const float ageMs = static_cast<float>(serverTick - boss.iActionStartTick) * 1000.f / SERVER_TICK_HZ;
    float startMs = -ageMs, risk = 0.f;
    for (std::size_t index = boss.iPatternStageIndex; index < pattern.Stages.size() && startMs <= horizonMs; ++index)
    {
        const auto& stage = pattern.Stages[index];
        if (!stage.AttackContacts.empty())
        {
            sampled.ePatternHitAnchorKind = stage.eHitAnchorKind;
            sampled.fPatternHitAnchorForwardOffsetM = stage.fHitAnchorForwardOffsetM;
            sampled.fPatternHitAnchorRightOffsetM = stage.fHitAnchorRightOffsetM;
            sampled.fPatternHitAnchorYawOffsetDegrees = stage.fHitAnchorYawOffsetDegrees;
            sampled.fPatternStageOriginX = boss.fPositionX;
            sampled.fPatternStageOriginZ = boss.fPositionZ;
            sampled.fPatternStageOriginYawDegrees = boss.fYawDegrees;
            const auto base = ResolvePatternHitTransform(index == boss.iPatternStageIndex ? boss : sampled);
            const auto first = index == boss.iPatternStageIndex ? boss.iAppliedPatternHitCount : 0u;
            for (std::size_t hit = first; hit < stage.AttackContacts.size(); ++hit)
            {
                const auto& contact = stage.AttackContacts[hit];
                const float at = startMs + contact.iAtMs;
                if (at >= 0.f && at <= horizonMs &&
                    ContainsStageContact(contact, probe, ResolveContactTransform(base, contact)))
                    risk += 1.f;
            }
            startMs += stage.iDurationMs;
            continue;
        }
        bool impending = false;
        if (stage.eHitActivationKind == BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW)
        {
            const float begin = startMs + stage.iHitActivationStartMs;
            impending = begin <= horizonMs && begin + stage.iHitActivationLifetimeMs >= 0.f;
        }
        else
        {
            const auto first = index == boss.iPatternStageIndex ? boss.iAppliedPatternHitCount : 0u;
            for (std::uint32_t hit = first; hit < stage.iHitCount; ++hit)
            {
                const float at = startMs + (hit < stage.HitOffsetsMs.size() ? stage.HitOffsetsMs[hit] :
                    stage.iHitDelayMs + hit * stage.iHitIntervalMs);
                if (at >= 0.f && at <= horizonMs)
                {
                    impending = true;
                    break;
                }
            }
        }
        if (impending && stage.eHitShape != BOSS_PATTERN_HIT_SHAPE::NONE && !stage.strDamageProfileId.empty())
        {
            if (index == boss.iPatternStageIndex)
            {
                const auto& hitTargets = stage.eHitActivationKind == BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ?
                    boss.PatternActiveWindowHitTargets : boss.PortalStageHitTargets;
                const bool consumed = (stage.eHitActivationKind == BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW || boss.bPortalMotionActive) &&
                    std::find(hitTargets.begin(), hitTargets.end(), probe.iNetEntityId) != hitTargets.end();
                if (!consumed &&
                    ContainsPatternHit(boss, probe, ResolvePatternHitTransform(boss)))
                    risk += 1.f;
            }
            else
            {
                sampled.ePatternHitShape = stage.eHitShape;
                sampled.fPatternHitOuterRadius = stage.fHitOuterRadius;
                sampled.fPatternHitInnerRadius = stage.fHitInnerRadius;
                sampled.fPatternHitAngleDegrees = stage.fHitAngleDegrees;
                sampled.fPatternHitLength = stage.fHitLength;
                sampled.fPatternHitHalfWidth = stage.fHitHalfWidth;
                sampled.ePatternHitAnchorKind = stage.eHitAnchorKind;
                sampled.fPatternHitAnchorForwardOffsetM = stage.fHitAnchorForwardOffsetM;
                sampled.fPatternHitAnchorRightOffsetM = stage.fHitAnchorRightOffsetM;
                sampled.fPatternHitAnchorYawOffsetDegrees = stage.fHitAnchorYawOffsetDegrees;
                sampled.fPatternStageOriginX = boss.fPositionX;
                sampled.fPatternStageOriginZ = boss.fPositionZ;
                sampled.fPatternStageOriginYawDegrees = boss.fYawDegrees;
                if (ContainsPatternHit(sampled, probe, ResolvePatternHitTransform(sampled)))
                    risk += 1.f;
            }
        }
        startMs += stage.iDurationMs;
    }
    return risk;
}

// 시간 만료/이동 막힘 분기와 GameRoom 잡기 stage action이 현재 잡힌 명단을 분류한다.
// 살아 있는 대상 수와 현재 보스/sequence/손 부착이 일치하는 수를 따로 센다. 잘못된 부착은 ALL 판정을 막는다.
LostArk::Server::SERVER_BOSS_GRAB_ROSTER
LostArk::Server::CValtanBrain::Classify_GrabbedPlayers(
	const SERVER_WORLD_ENTITY& boss,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	SERVER_BOSS_GRAB_ROSTER result{};
	for (const auto& [playerId, player] : players)
	{
		if (player.Is_Guide())
			continue;
		(void)playerId;
		const bool alive = 0u != player.iCurrentHp &&
			PLAYER_ACTION_STATE::DEAD != player.eAction &&
			PLAYER_ACTION_STATE::FALLING != player.eAction;
		const bool hasAttachment = PLAYER_ACTION_STATE::GRABBED == player.eAction ||
			INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::NONE != player.eAttachmentSlot ||
			0u != player.iAttachmentPatternSequence;
		const bool owned = alive && PLAYER_ACTION_STATE::GRABBED == player.eAction &&
			INVALID_NET_ENTITY_ID != boss.iNetEntityId &&
			player.iAttachmentOwnerNetEntityId == boss.iNetEntityId &&
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND == player.eAttachmentSlot &&
			0u != boss.iPatternSequence &&
			player.iAttachmentPatternSequence == boss.iPatternSequence &&
			INVALID_NET_ENTITY_ID != player.iNetEntityId && player.iPlayerId == playerId &&
			std::isfinite(player.fAttachmentLocalOffsetX) &&
			std::isfinite(player.fAttachmentLocalOffsetY) &&
			std::isfinite(player.fAttachmentLocalOffsetZ) &&
			std::isfinite(player.fAttachmentYawOffsetDegrees);
		result.bHasInvalidAttachment = result.bHasInvalidAttachment ||
			(hasAttachment && !owned);
		if (alive)
			++result.iAliveCount;
		if (owned)
			++result.iGrabbedCount;
	}
	if (0u != result.iGrabbedCount)
	{
		result.eClassification = !result.bHasInvalidAttachment &&
			result.iAliveCount == result.iGrabbedCount ?
			SERVER_BOSS_GRAB_CLASSIFICATION::ALL :
			SERVER_BOSS_GRAB_CLASSIFICATION::PARTIAL;
	}
	return result;
}

// 선택과 표시가 사용하는 현재 체력 줄을 올림 나눗셈으로 구한다. HP/최대 HP/줄 수가 0이면 0이다.
std::uint32_t LostArk::Server::CValtanBrain::Calculate_HealthBar(
	const SERVER_WORLD_ENTITY& boss)
{
	if (0u == boss.iCurrentHp || 0u == boss.iMaximumHp ||
		0u == boss.iMaximumHealthBars)
	{
		return 0u;
	}
	const std::uint64_t scaled =
		static_cast<std::uint64_t>(boss.iCurrentHp) * boss.iMaximumHealthBars;
	return static_cast<std::uint32_t>((scaled + boss.iMaximumHp - 1u) /
		boss.iMaximumHp);
}

// GameRoom의 체력 줄 지정 경로가 사용할 HP 경계를 구한다. 범위 밖의 줄 입력은 0을 반환한다.
std::uint32_t LostArk::Server::CValtanBrain::Resolve_HealthBarHp(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t healthBar)
{
	if (0u == healthBar || 0u == boss.iMaximumHp ||
		0u == boss.iMaximumHealthBars ||
		healthBar > boss.iMaximumHealthBars)
	{
		return 0u;
	}
	// 최대 HP와 체력 줄 비율의 곱을 64bit로 계산하고 경계 HP는 내림으로 구한다.
	// Calculate_HealthBar는 반대로 올림하며, 유효한 입력의 결과가 0이면 1HP를 반환한다.
	const std::uint64_t scaled =
		static_cast<std::uint64_t>(boss.iMaximumHp) * healthBar;
	const std::uint32_t resolved =
		static_cast<std::uint32_t>(scaled / boss.iMaximumHealthBars);
	return 0u == resolved ? 1u : resolved;
}

// CGameRoom::Update_WorldEntities가 고정 tick마다 보스와 방의 플레이어 상태를 전달한다.
// catalog은 현재 실행 정의, activeThresholdCatalog는 새 체력 기믹 평가에 사용할 수 있는 정의다.
// 사망/대기/정의/대상 검사를 먼저 하고, IDLE/CHASE에서 선택한 뒤 실행 stage의 조준/판정/분기를 진행한다.
// 피해는 공용 hit runtime으로 적용하고 잡기/owner hit 요청은 출력에 모아 호출자가 후속 처리한다.
void LostArk::Server::CValtanBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const CServerNavigation& navigation,
	const float fixedDeltaSeconds,
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
		coverCircles,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	const CGameplayCatalog* activeThresholdCatalog,
	const std::uint16_t activeThresholdGenerationEpoch,
	std::vector<SERVER_PLAYER_CAPTURE_REQUEST>* outCaptureRequests,
	const BOSS_PATTERN_SEQUENCE_DEFINITION* automaticSequenceOverride,
	std::vector<SERVER_BOSS_PATTERN_HIT>* outOwnerHits) const
{
	if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind)
		return;
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		RestorePatternVerticalOffset(boss);
		if (!boss.strPatternId.empty())
		{
			boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
			boss.PatternTerminalReceipt.iRootPatternSequence =
				0u == boss.iPatternFollowupDepth ? boss.iPatternSequence :
				boss.iPatternFollowupRootSequence;
			boss.PatternTerminalReceipt.eResult =
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED;
		}
		boss.iGrabExecutionCommittedPatternSequence = 0u;
		boss.iGrabExecutionCommittedStageIndex = 0u;
		boss.PendingPatternFollowup = {};
		boss.iPatternFollowupDepth = 0u;
		boss.iPatternFollowupRootSequence = 0u;
		FailMechanic(
			boss, boss.strPatternId,
			SERVER_BOSS_MECHANIC_FAILURE::BOSS_DIED, serverTick);
		boss.iCurrentHp = 0u;
		boss.strPatternId.clear();
		boss.iPatternStartTick = 0u;
		if (boss.bAutomaticPatternSequenceStepRunning)
		{
			boss.bAutomaticPatternSequenceStepRunning = false;
			boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			boss.bMechanicLedgerRequiresReset = true;
		}
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		boss.strPatternStageId.clear();
		boss.strActionId.clear();
		boss.bPatternHasCounterProxy = false;
		boss.ePatternCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
		boss.fPatternCounterProxyForwardOffsetM = 0.f;
		boss.fPatternCounterProxyRightOffsetM = 0.f;
		boss.fPatternCounterProxyRadiusM = 0.f;
		boss.fPatternCounterProxyArcDegrees = 0.f;
		SetPatternBossResponseState(
			boss, BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE, 0u, 0u, false);
		boss.ePatternStageMotionKind = BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		boss.PatternStageRootMotion.clear();
		boss.bPortalMotionActive = false;
		boss.bPortalRushTargetLocked = false;
		boss.PortalStageHitTargets.clear();
		boss.fPortalLastHitSampleX = 0.f;
		boss.fPortalLastHitSampleZ = 0.f;
		Transition(boss, SERVER_ENTITY_ACTION::DEAD, serverTick);
		boss.MovePath.clear();
		return;
	}
	const BOSS_RUNTIME_PROFILE* bossProfile =
		catalog.Find_Boss(boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ?
                "BOSS_VALTAN_GHOST" : boss.strArchetypeId);
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == bossProfile || nullptr == patterns || patterns->empty())
	{
		VALTAN_DECISION_TRACE trace = MakeDecisionTraceHeader(
			boss, serverTick, Calculate_HealthBar(boss));
		trace.eResult = VALTAN_DECISION_RESULT::CATALOG_UNAVAILABLE;
		Record_DecisionTrace(std::move(trace));
		return;
	}
	// 완료된 Debug 시험 재생의 대기는 Server 상태다.
	// 다음 tick의 체력 경계 평가나 자동 선택이 이 상태를 덮지 않도록 먼저 반환한다.
	if (boss.bAutomaticPatternSequenceAuditionHold)
	{
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
		return;
	}
	if (BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD ==
		bossProfile->PhasePolicy.eKind && boss.iMaximumHp > 0u &&
		bossProfile->PhasePolicy.iThresholdPercent > 0u &&
		static_cast<std::uint64_t>(boss.iCurrentHp) * 100u <=
		static_cast<std::uint64_t>(boss.iMaximumHp) *
			bossProfile->PhasePolicy.iThresholdPercent)
	{
		// phase는 보스 전투 snapshot의 일부이며 별도 revision 관리가 필요하다.
		// 그 revision을 관리하는 CBossCombatRuntime을 통해 갱신한다.
		(void)CBossCombatRuntime::Set_GameplayPhase(boss, 2u);
	}
	const std::uint32_t currentHealthBar = Calculate_HealthBar(boss);
	const bool ownsPinnedProductSequence = nullptr == automaticSequenceOverride &&
		boss.ProductSequencePinnedDefinitionRevision.Is_Valid() &&
		!boss.bScriptedPatternPlayback && !boss.bAutomaticPatternSequenceAuditionOverride;
	const CGameplayCatalog& thresholdCatalog =
		ownsPinnedProductSequence || nullptr == activeThresholdCatalog ?
			catalog : *activeThresholdCatalog;
	const auto* thresholdPatterns =
		thresholdCatalog.Find_BossPatterns(boss.strEncounterId);
	const BOSS_PATTERN_SEQUENCE_DEFINITION* thresholdSequence =
		nullptr != automaticSequenceOverride ? automaticSequenceOverride :
		thresholdCatalog.Find_BossPatternSequence(boss.strEncounterId);
	if (nullptr == thresholdPatterns)
	{
		boss.bMechanicLedgerRequiresReset = true;
	}
	else if (nullptr != thresholdSequence &&
		BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE == thresholdSequence->eMode &&
		!boss.bAutomaticPatternSequenceAuditionOverride)
	{
		// ORDERED_ONCE_THEN_IDLE은 HP로 자동 항목을 추가하거나 순서를 바꾸지 않는다.
		// 정의 갱신 뒤 과거 체력 경계를 다시 예약하지 않도록 마지막 평가 기록만 갱신한다.
		boss.iLastEvaluatedHealthBar = currentHealthBar;
		boss.iLastHealthMechanicGenerationEpoch =
			activeThresholdGenerationEpoch;
	}
	else
	{
		QueueCrossedHealthBarPatterns(
			boss, *thresholdPatterns, currentHealthBar,
			thresholdCatalog.Get_ActiveRevision(),
			activeThresholdGenerationEpoch, serverTick);
	}
	if (boss.bMechanicLedgerRequiresReset)
	{
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		VALTAN_DECISION_TRACE trace = MakeDecisionTraceHeader(
			boss, serverTick, currentHealthBar);
		trace.eResult = VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED;
		trace.eSource = trace.ePendingSource;
		Record_DecisionTrace(std::move(trace));
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
		Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
		return;
	}

	float targetDistanceSquared = (std::numeric_limits<float>::max)();
	SERVER_PLAYER* target = SelectNearestEngageableTarget(
		boss, players, &targetDistanceSquared);
	const float engageDistance = (std::max)(
		boss.fEngageDistance, MaximumPatternRange(*patterns));
	const BOSS_PATTERN_SEQUENCE_DEFINITION* automaticSequence =
		nullptr != automaticSequenceOverride ? automaticSequenceOverride :
		catalog.Find_BossPatternSequence(boss.strEncounterId);
	const bool sequenceIgnoresEngageDistance =
		nullptr != automaticSequence &&
		BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE == automaticSequence->eMode &&
		!boss.bScriptedPatternPlayback &&
		!boss.bAutomaticPatternSequenceAuditionOverride;
	const bool continueTargetlessScheduledArenaStage =
		nullptr == target &&
		CanContinueTargetlessScheduledArenaStage(boss, *patterns);
	const bool continueTargetlessOwnedGrabPattern =
		nullptr == target &&
		CanContinueTargetlessOwnedGrabPattern(boss, players, *patterns);
	const bool continueTargetlessOwnedBindPattern =
		nullptr == target &&
		CanContinueTargetlessOwnedBindPattern(boss, players, *patterns);
	const bool continueTargetlessPatternFollowup = nullptr == target &&
		(boss.PendingPatternFollowup.Is_Pending() ||
		 (boss.iPatternFollowupDepth > 0u && !boss.strPatternId.empty()));
	const auto* runningDefinition = FindPattern(*patterns, boss.strPatternId);
	// 마지막 stage의 예약 판정 기회를 모두 처리했다면 남은 종료 시간을 진행할 수 있다.
	// 마지막 대상을 처치한 뒤에도 NO_VALID_TARGET으로 취소하지 않기 위한 조건이다.
	const bool terminalStageDamageCommitted =
		nullptr == target && !players.empty() &&
		nullptr != runningDefinition &&
		boss.iPatternStageIndex + 1u == runningDefinition->Stages.size() &&
		0u != boss.iPatternHitCount &&
		boss.iAppliedPatternHitCount >= boss.iPatternHitCount;
	const bool floorWipe = "VALTAN_FLOOR_WIPE_130" == boss.strPatternId;
	const bool highJump = "VALTAN_HIGH_JUMP" == boss.strPatternId;
	const bool finalHitCommitted = 0u != boss.iPatternHitCount &&
		boss.iAppliedPatternHitCount >= boss.iPatternHitCount;
	// 이 마지막 타격들은 순서형 실행과 ID 지정 시험 재생 모두에서 남은 이동/회복을 끝낸다.
	// 마지막 대상을 처치한 경우도 포함한다.
	const bool continueTargetlessCommittedFinalStrike =
		nullptr == target && !players.empty() && (floorWipe || highJump) &&
		("RECOVERY" == boss.strPatternStageId ||
			(finalHitCommitted &&
				((floorWipe && "SECOND_SMASH" == boss.strPatternStageId) ||
				 (highJump && "LAND" == boss.strPatternStageId))));
	const bool independentFinaleOrChild = !players.empty() &&
		(LostArk::Shared::INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId ||
			(nullptr != runningDefinition &&
				BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP ==
					runningDefinition->Finale.eKind));
	const bool continueTargetlessRunningStage =
		continueTargetlessScheduledArenaStage ||
		continueTargetlessOwnedGrabPattern ||
		continueTargetlessOwnedBindPattern ||
		continueTargetlessCommittedFinalStrike ||
		terminalStageDamageCommitted ||
		continueTargetlessPatternFollowup ||
		independentFinaleOrChild;
	// 위에서 허용한 독립 진행 조건 외에 대상이 없거나 사거리를 벗어나면 현재 패턴을 끝낸다.
	// NO_VALID_TARGET은 재시도 가능한 대기로 처리하며, FinishPattern이 순서형 커서나 체력 기믹 대기열을 보존한다.
	if ((!continueTargetlessRunningStage && nullptr == target) ||
		(!sequenceIgnoresEngageDistance && nullptr != target &&
		 targetDistanceSquared > engageDistance * engageDistance))
	{
		VALTAN_DECISION_TRACE trace = MakeDecisionTraceHeader(
			boss, serverTick, currentHealthBar);
		trace.Candidates.reserve((std::min)(
			patterns->size(), MAX_DECISION_CANDIDATE_COUNT));
		const BOSS_PATTERN_ROTATION_DEFINITION* noTargetRotation =
			catalog.Find_BossPatternRotation(
				boss.strEncounterId, boss.iPhase, currentHealthBar);
		const BOSS_PATTERN_ROTATION_DEFINITION* managedNoTargetRotation =
			nullptr != noTargetRotation &&
			BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				noTargetRotation->eSelectionMode ? noTargetRotation : nullptr;
		VALTAN_PATTERN_SELECTION_RESULT evaluated = SelectNormalPattern(
			boss, *patterns, catalog.Find_IntroPatternId(boss.strEncounterId),
			currentHealthBar, 0.f, serverTick, std::move(trace),
			VALTAN_DECISION_SOURCE::NONE, managedNoTargetRotation, false);
		trace = std::move(evaluated.Trace);
		trace.eSource = VALTAN_DECISION_SOURCE::NONE;
		trace.eResult = VALTAN_DECISION_RESULT::NO_VALID_TARGET;
		trace.strSelectedPatternId.clear();
		trace.iTotalWeight = 0u;
		trace.iRawRandomInput = 0u;
		trace.iMixedRandomValue = 0u;
		trace.iRandomTicket = 0u;
		Record_DecisionTrace(std::move(trace));
		FinishPattern(
			boss, serverTick, false,
			SERVER_BOSS_MECHANIC_FAILURE::NO_VALID_TARGET);
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
		return;
	}
	if (nullptr == target)
	{
		boss.iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		boss.MovePath.clear();
	}
	else
	{
		boss.iTargetEntityId = target->iNetEntityId;
	}
	const float distance = nullptr == target ?
		0.f : std::sqrt(targetDistanceSquared);

	if (SERVER_ENTITY_ACTION::IDLE == boss.eAction ||
		SERVER_ENTITY_ACTION::CHASE == boss.eAction)
	{
		VALTAN_DECISION_TRACE trace = MakeDecisionTraceHeader(
			boss, serverTick, currentHealthBar);
		trace.iTargetNetEntityId = nullptr == target ?
			LostArk::Shared::INVALID_NET_ENTITY_ID : target->iNetEntityId;
		trace.fTargetDistance = distance;
		trace.Candidates.reserve((std::min)(
			patterns->size(), MAX_DECISION_CANDIDATE_COUNT));
		const bool selectingPatternFollowup =
			boss.PendingPatternFollowup.Is_Pending();
		const BOSS_PATTERN_DEFINITION* selected = SelectPattern(
			boss, *patterns, catalog.Get_ActiveRevision(),
			catalog.Find_IntroPatternId(boss.strEncounterId),
			automaticSequence,
			catalog.Find_BossPatternRotation(
				boss.strEncounterId, boss.iPhase, currentHealthBar),
			currentHealthBar, distance, serverTick, trace);
		Record_DecisionTrace(std::move(trace));
		if (nullptr == selected)
		{
			if (boss.bMechanicLedgerRequiresReset)
			{
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			if (!boss.bIntroPatternConsumed)
			{
				// 입장 공격을 아직 소비하지 않았다면 현재 위치에서 사거리를 기다린다.
				// 입장 조건을 맞추려고 일반 추적 이동을 시작하지 않는다.
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			if (boss.bAutomaticPatternSequenceAuditionHold)
			{
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			const BOSS_PATTERN_SEQUENCE_DEFINITION* scriptedSequence =
				automaticSequence;
			if (!boss.bScriptedPatternPlayback &&
				nullptr != scriptedSequence &&
				boss.strRotationId == scriptedSequence->strSequenceId &&
				boss.iRotationStepIndex >= static_cast<std::uint32_t>(
					scriptedSequence->PatternIds.size()))
			{
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			if (nullptr == target)
			{
				Transition(boss, SERVER_ENTITY_ACTION::IDLE, serverTick);
				boss.MovePath.clear();
				return;
			}
			Transition(boss, SERVER_ENTITY_ACTION::CHASE, serverTick);
			const float pathDeltaX = target->fPositionX - boss.fLastPathGoalX;
			const float pathDeltaZ = target->fPositionZ - boss.fLastPathGoalZ;
			if (boss.MovePath.empty() ||
				pathDeltaX * pathDeltaX + pathDeltaZ * pathDeltaZ > 1.f)
			{
				std::vector<SERVER_NAV_POINT> path;
				SERVER_NAV_POINT start{ boss.fPositionX, boss.fPositionY, boss.fPositionZ };
				const bool recovering = navigation.Is_Loaded() &&
					navigation.Is_PointInVoidRegion(start.x, start.z);
				const bool admittedStart = !recovering ||
					(navigation.Project_PointOnSameLevel(start.x, start.z, start, boss.fPositionY) &&
					 std::abs(start.y - boss.fPositionY) <= 1.f);
				const bool found = admittedStart && navigation.Find_Path(
					start.x, start.z, target->fPositionX, target->fPositionZ, path, start.y);
				// Find_Path는 시작 위치를 투영하고 첫 셀을 결과에서 생략한다.
				// 붕괴 지형의 보스는 그 복구 시작점까지 먼저 걸어가도록 경로 맨 앞에 넣는다.
				// 목표까지의 전체 경로를 찾지 못해도 허용된 복구 구간은 사용할 수 있다.
				if (admittedStart && recovering)
					path.insert(path.begin(), start);
				if (found || (admittedStart && recovering))
				{
					boss.MovePath = std::move(path);
					boss.iMovePathIndex = 0;
					boss.fLastPathGoalX = target->fPositionX;
					boss.fLastPathGoalZ = target->fPositionZ;
				}
			}
			MoveAlongPath(boss, navigation, fixedDeltaSeconds);
			return;
		}
		boss.MovePath.clear();
		// 사방 공격은 저작된 시작 방향을 유지한다.
		// BeginPattern의 조준 정책 적용 전에도 최근접 대상 방향으로 미리 회전시키지 않는다.
		if (nullptr != target && selected->strPatternId != "VALTAN_SEQUENCE_FOUR" &&
			BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START !=
			selected->eTargetPolicy)
		{
			const float deltaX = target->fPositionX - boss.fPositionX;
			const float deltaZ = target->fPositionZ - boss.fPositionZ;
			boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
		}
		if (nullptr == automaticSequenceOverride &&
			boss.bAutomaticPatternSequenceStepRunning &&
			!boss.ProductSequencePinnedDefinitionRevision.Is_Valid())
		{
			boss.ProductSequencePinnedDefinitionRevision = catalog.Get_ActiveRevision();
		}
		if (!selectingPatternFollowup)
		{
			boss.iPatternFollowupDepth = 0u;
			boss.iPatternFollowupRootSequence = 0u;
		}
		BeginPattern(
			boss, *selected, players, target,
			catalog.Get_ActiveRevision(), serverTick);
	}

	const BOSS_PATTERN_DEFINITION* currentPattern =
		FindPattern(*patterns, boss.strPatternId);
	if (nullptr == currentPattern ||
		boss.iPatternStageIndex >= currentPattern->Stages.size())
	{
		FinishPattern(
			boss, serverTick, false,
			SERVER_BOSS_MECHANIC_FAILURE::INVALID_RUNNING_DEFINITION);
		return;
	}
	const auto refreshEnteredStageAim = [&]()
	{
		if (boss.strPatternId == currentPattern->strPatternId &&
			boss.iPatternStageIndex < currentPattern->Stages.size() &&
			(currentPattern->Stages[boss.iPatternStageIndex].bTrackNearestTarget ||
			 currentPattern->Stages[boss.iPatternStageIndex].bTrackPatternTarget))
			UpdatePatternTargetAndAim(boss, *currentPattern, players, nullptr, serverTick);
	};
	UpdatePatternTargetAndAim(boss, *currentPattern, players, target, serverTick);
	const BOSS_PATTERN_STAGE_DEFINITION& currentStage =
		currentPattern->Stages[boss.iPatternStageIndex];
	(void)TryConsumeDamageLessCounterProxy(
		boss, players, catalog, currentStage, serverTick, outDamageEvents);
	if (ApplyPublishedOutcomeBranch(
		boss, *currentPattern, currentStage, serverTick))
	{
		refreshEnteredStageAim();
		return;
	}

	// 갑옷 파괴 반응이 대기 중이면 현재 패턴에 선언된 PART_BREAK stage를 찾는다.
	// 일반 시간 진행으로 우연히 다음에 놓인 stage를 반응으로 사용하지 않는다.
	if (boss.bPendingArmorBreakReaction)
	{
		boss.bPendingArmorBreakReaction = false;
		for (std::uint32_t reactionIndex = 0u;
			reactionIndex < currentPattern->Stages.size();
			++reactionIndex)
		{
			if (BOSS_PATTERN_STAGE_KIND::PART_BREAK !=
				currentPattern->Stages[reactionIndex].eStageKind)
			{
				continue;
			}
			EnterPatternStage(
				boss, currentPattern->Stages[reactionIndex],
				reactionIndex, serverTick);
			refreshEnteredStageAim();
			return;
		}
	}
	const std::uint64_t elapsedStageTicks =
		StageElapsedTicks(boss, serverTick);
	// 표시/이동용 초 단위 시간도 같은 정수 stage tick에서 계산한다.
	// 예약 판정과 stage 마감은 누적 float delta 대신 정수 시간 비교를 사용한다.
	boss.fActionElapsedSeconds =
		static_cast<float>(elapsedStageTicks) /
		static_cast<float>(SERVER_TICK_HZ);
	Advance_ArenaBreakLeap(boss);
	if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA ==
		boss.ePatternStageMotionKind && boss.iPatternStageDurationMs > 0u)
	{
		const float ratio = std::clamp(boss.fActionElapsedSeconds * 1000.f /
			static_cast<float>(boss.iPatternStageDurationMs), 0.f, 1.f);
		boss.fPositionX = boss.fPortalStartX +
			(boss.fPortalEndX - boss.fPortalStartX) * ratio;
		boss.fPositionZ = boss.fPortalStartZ +
			(boss.fPortalEndZ - boss.fPortalStartZ) * ratio;
		boss.fPositionY = boss.fSpawnPositionY;
	}
	if (BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ==
		boss.ePatternHitActivationKind &&
		HasElapsedMilliseconds(
			elapsedStageTicks, boss.iPatternHitActivationStartMs) &&
		!HasElapsedMilliseconds(
			elapsedStageTicks,
			boss.iPatternHitActivationStartMs +
				boss.iPatternHitActivationLifetimeMs))
	{
		ApplyPatternHit(
			boss, players, catalog, serverTick, coverCircles,
			outDamageEvents, outCaptureRequests);
		if (nullptr != outOwnerHits && BOSS_PATTERN_HIT_SHAPE::CONE == boss.ePatternHitShape)
		{
			const auto pose = ResolvePatternHitTransform(boss);
			SERVER_BOSS_PATTERN_HIT hit;
			hit.iSourceNetEntityId = boss.iNetEntityId;
			hit.iPatternSequence = boss.iPatternSequence;
			hit.strPatternId = boss.strPatternId;
			hit.strActionId = currentStage.strActionId;
			hit.Cone = {pose.fPositionX, pose.fPositionZ,
				std::sin(pose.fYawDegrees * DEGREES_TO_RADIANS),
				std::cos(pose.fYawDegrees * DEGREES_TO_RADIANS),
				boss.fPatternHitLength, boss.fPatternHitAngleDegrees};
			outOwnerHits->push_back(std::move(hit));
		}
	}
	// 이 카운터는 명중한 대상 수가 아니라 처리한 예약 판정 기회 수다.
	// 한 tick에 여러 시각이 지났으면 while로 모두 소비하지만 과거 위치를 재현하지는 않는다.
	// 각 ApplyPatternHit은 지금의 boss/players 위치로 검사하며 miss여도 아래 카운터가 증가한다.
	while (boss.iAppliedPatternHitCount < boss.iPatternHitCount)
	{
		const std::uint32_t hitOffsetMs =
			currentStage.HitOffsetsMs.empty() ?
				boss.iPatternHitDelayMs + boss.iAppliedPatternHitCount *
					boss.iPatternHitIntervalMs :
				currentStage.HitOffsetsMs[boss.iAppliedPatternHitCount];
		if (!HasElapsedMilliseconds(elapsedStageTicks, hitOffsetMs))
		{
			break;
		}
		const auto* contact = boss.iAppliedPatternHitCount < currentStage.AttackContacts.size() ?
			&currentStage.AttackContacts[boss.iAppliedPatternHitCount] : nullptr;
		ApplyPatternHit(
			boss, players, catalog, serverTick, coverCircles,
			outDamageEvents, outCaptureRequests, contact);
		if (nullptr != outOwnerHits &&
			(contact ? contact->strShape == "CONE" :
				BOSS_PATTERN_HIT_SHAPE::CONE == boss.ePatternHitShape))
		{
			const auto pose = contact ? ResolveContactTransform(ResolvePatternHitTransform(boss), *contact) : ResolvePatternHitTransform(boss);
			SERVER_BOSS_PATTERN_HIT hit;
			hit.iSourceNetEntityId = boss.iNetEntityId;
			hit.iPatternSequence = boss.iPatternSequence;
			hit.strPatternId = boss.strPatternId;
			hit.strActionId = currentStage.strActionId;
			hit.Cone = {pose.fPositionX, pose.fPositionZ,
				std::sin(pose.fYawDegrees * DEGREES_TO_RADIANS),
				std::cos(pose.fYawDegrees * DEGREES_TO_RADIANS),
				contact ? float(contact->fLengthM) : boss.fPatternHitLength,
				contact ? float(contact->fAngleDegrees) : boss.fPatternHitAngleDegrees};
			outOwnerHits->push_back(std::move(hit));
		}
		if (boss.bPortalMotionActive)
		{
			boss.fPortalLastHitSampleX = boss.fPositionX;
			boss.fPortalLastHitSampleZ = boss.fPositionZ;
		}
		// 이번 기회가 피해를 주지 못했어도 다시 시도하지 않는다. 다음 stage 진입 시 0으로 초기화한다.
		++boss.iAppliedPatternHitCount;
	}
	if (!HasElapsedMilliseconds(
		elapsedStageTicks, boss.iPatternStageDurationMs))
	{
		return;
	}
	// 이번 tick에 만든 잡기 요청은 Brain 반환 뒤 GameRoom이 확정한다.
	// 일반 마감에서는 미확정 요청을 잡기 실패로 분류하지 않도록 다음 tick까지 기다린다.
	// 이미 소비한 예약 판정 기회는 그 대기 tick에 다시 적용하지 않는다.
	if (BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == currentStage.ePlayerResponse &&
		nullptr != outCaptureRequests && !outCaptureRequests->empty())
		return;
	if (ApplyTimeoutBranch(
		boss, *currentPattern, currentStage, players, serverTick))
	{
		refreshEnteredStageAim();
		return;
	}

	const std::uint32_t nextStageIndex =
		NextClockStageIndex(*currentPattern, boss.iPatternStageIndex);
	if (nextStageIndex >= currentPattern->Stages.size())
	{
		FinishPattern(boss, serverTick, true);
		return;
	}
	EnterPatternStage(
		boss, currentPattern->Stages[nextStageIndex], nextStageIndex, serverTick);
	// 새 조준 stage의 대상도 진입 snapshot에 맞춰 갱신한다.
	// 시간 0부터 대상을 따르는 cue가 첫 snapshot에서 대상을 놓치지 않도록 한다.
	refreshEnteredStageAim();
}

// 일반 패턴 후보 필터가 갑옷 plate의 남은 내구도로 착용 여부를 검사한다. 상태는 변경하지 않는다.
bool LostArk::Server::CValtanBrain::Is_ArmorRequirementMet(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_ARMOR_REQUIREMENT requirement)
{
	if (BOSS_PATTERN_ARMOR_REQUIREMENT::ANY == requirement)
		return true;
	bool wearsAnyPlate = false;
	for (const SERVER_BOSS_ARMOR_PLATE_STATE& plate : boss.ArmorPlates)
	{
		if (0u != plate.iRemainingDurability)
		{
			wearsAnyPlate = true;
			break;
		}
	}
	return BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED == requirement ?
		wearsAnyPlate : !wearsAnyPlate;
}

// 일반 패턴 후보 필터가 요구 phase를 검사한다. PHASE_TWO는 2 이상을 허용한다.
bool LostArk::Server::CValtanBrain::Is_PhaseRequirementMet(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_PHASE_REQUIREMENT requirement)
{
	switch (requirement)
	{
	case BOSS_PATTERN_PHASE_REQUIREMENT::ANY:
		return true;
	case BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_ONE:
		return 1u == boss.iPhase;
	case BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO:
		return boss.iPhase >= 2u;
	}
	return false;
}

// GameRoom의 실행 commit 실패가 현재 pattern ID를 지정하여 공통 공개 실패 처리로 전달한다.
void LostArk::Server::CValtanBrain::Fail_ActiveMechanic(
	SERVER_WORLD_ENTITY& boss,
	const SERVER_BOSS_MECHANIC_FAILURE failure,
	const std::uint32_t serverTick)
{
	Fail_Mechanic(boss, boss.strPatternId, failure, serverTick);
}

// GameRoom의 정의/전환 실패가 지정한 기믹을 실패 처리한다.
// 현재 실행 ID와 일치하면 높이 보정 복구와 취소 기록을 추가하지만 FinishPattern의 전체 초기화를 대신하지는 않는다.
void LostArk::Server::CValtanBrain::Fail_Mechanic(
	SERVER_WORLD_ENTITY& boss,
	const std::string& patternId,
	const SERVER_BOSS_MECHANIC_FAILURE failure,
	const std::uint32_t serverTick)
{
	if (!patternId.empty() && patternId == boss.strPatternId)
	{
		// GameRoom의 사전 검사/commit 실패는 FinishPattern을 거치지 않고 이 공개 실패 경로로 들어올 수 있다.
		// 현재 실행에 속한 높이 보정을 되돌리고 취소 기록을 남긴다.
		RestorePatternVerticalOffset(boss);
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence =
			0u == boss.iPatternFollowupDepth ? boss.iPatternSequence :
			boss.iPatternFollowupRootSequence;
		boss.PatternTerminalReceipt.eResult =
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED;
	}
	FailMechanic(boss, patternId, failure, serverTick);
}

// 링 버퍼에서 보존 중인 가장 오래된 기록을 0으로 하는 index를 조회한다.
// 반환 포인터는 Brain 내부 저장소를 차용하므로 이후 기록에 의해 내용이 덮일 수 있다.
const LostArk::Server::VALTAN_DECISION_TRACE*
LostArk::Server::CValtanBrain::Get_DecisionTrace(
	const std::size_t age) const noexcept
{
	if (age >= m_iDecisionTraceCount)
		return nullptr;
	const std::size_t oldest =
		(m_iDecisionTraceWriteIndex + MAX_DECISION_TRACE_COUNT -
			m_iDecisionTraceCount) % MAX_DECISION_TRACE_COUNT;
	return &m_DecisionTraces[(oldest + age) % MAX_DECISION_TRACE_COUNT];
}

// GameRoom의 결정 기록 복제/조회가 최근 항목을 읽는다. 기록이 없으면 nullptr이다.
const LostArk::Server::VALTAN_DECISION_TRACE*
LostArk::Server::CValtanBrain::Get_LatestDecisionTrace() const noexcept
{
	return 0u == m_iDecisionTraceCount ? nullptr :
		Get_DecisionTrace(m_iDecisionTraceCount - 1u);
}

// Update가 만든 선택 결과를 고정 크기 링 버퍼에 이동 저장한다.
// 기록 상한을 넘으면 오래된 항목을 덮으며, 게임플레이 상태와 별도로 진단 sequence를 진행한다.
void LostArk::Server::CValtanBrain::Record_DecisionTrace(
	VALTAN_DECISION_TRACE&& trace) const
{
	trace.iTraceSequence = m_iNextDecisionTraceSequence;
	m_iNextDecisionTraceSequence =
		(std::numeric_limits<std::uint64_t>::max)() ==
			m_iNextDecisionTraceSequence ? 1u :
			m_iNextDecisionTraceSequence + 1u;
	m_DecisionTraces[m_iDecisionTraceWriteIndex] = std::move(trace);
	m_iDecisionTraceWriteIndex =
		(m_iDecisionTraceWriteIndex + 1u) % MAX_DECISION_TRACE_COUNT;
	m_iDecisionTraceCount = (std::min)(
		m_iDecisionTraceCount + 1u, MAX_DECISION_TRACE_COUNT);
}

// 돌진 충돌 contract test에서 강제 속도로 다음 XZ 위치를 계산하는 공개 함수다.
// boss를 변경하지 않으며 false인 경우 출력 좌표를 사용하면 안 된다.
bool LostArk::Server::CValtanBrain::Try_BuildImpactMotion(
	const SERVER_WORLD_ENTITY& boss,
	const float fixedDeltaSeconds,
	float& outProposedX,
	float& outProposedZ) const
{
	if (!boss.bPatternChargeImpact ||
		!std::isfinite(boss.fPatternForcedMotionSpeed) ||
		boss.fPatternForcedMotionSpeed <= 0.f ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		return false;
	}
	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	const float distance = boss.fPatternForcedMotionSpeed * fixedDeltaSeconds;
	outProposedX = boss.fPositionX + std::sin(yawRadians) * distance;
	outProposedZ = boss.fPositionZ + std::cos(yawRadians) * distance;
	return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
}

// GameRoom이 Brain Update 뒤에 요청하는 XZ 이동 제안이다. 실제 위치 commit과 충돌 처리는 호출자가 맡는다.
// 직접 위치 갱신 중인 도약/횡단을 제외하고 중앙 이동, 포탈 돌진, 루트모션, 고정 전진을 순서대로 검사한다.
// 실패 시 출력이 일부 계산되었을 수 있으므로 true일 때만 제안값을 사용한다.
bool LostArk::Server::CValtanBrain::Try_BuildStageMotion(
	const SERVER_WORLD_ENTITY& boss,
	const float fixedDeltaSeconds,
	float& outProposedX,
	float& outProposedZ) const
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
		return false;
	// 도약 곡선이나 아레나 횡단이 직접 X/Z를 갱신하는 동안에는 별도 이동을 제안하지 않는다.
	// 여기에 클립 루트모션까지 더하면 시작 위치와 착지점이 달라지기 때문이다.
	// 도약 정점 상태가 해제된 후속 stage는 자기 루트모션을 사용할 수 있다.
	if (boss.fLeapApexHeight > 0.f ||
		BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA ==
			boss.ePatternStageMotionKind)
		return false;

	// 중앙 이동의 제안값은 GameRoom이 실제 이동 처리에 사용한다.
	// 다음 고정 간격까지 포함한 진행률로 끝점을 계산하며, 이 경로에는 클립 루트모션을 더하지 않는다.
	if (BOSS_PATTERN_STAGE_MOTION_KIND::TO_ARENA_CENTER ==
		boss.ePatternStageMotionKind)
	{
		if (0u == boss.iPatternStageDurationMs ||
			!std::isfinite(boss.fActionElapsedSeconds) ||
			!std::isfinite(boss.fPatternStageOriginX) ||
			!std::isfinite(boss.fPatternStageOriginZ) ||
			!std::isfinite(boss.fSpawnPositionX) ||
			!std::isfinite(boss.fSpawnPositionZ))
			return false;
		const float ratio = std::clamp(
			(boss.fActionElapsedSeconds + fixedDeltaSeconds) * 1000.f /
				static_cast<float>(boss.iPatternStageDurationMs), 0.f, 1.f);
		outProposedX = boss.fPatternStageOriginX +
			(boss.fSpawnPositionX - boss.fPatternStageOriginX) * ratio;
		outProposedZ = boss.fPatternStageOriginZ +
			(boss.fSpawnPositionZ - boss.fPatternStageOriginZ) * ratio;
		return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
	}

	// 포탈 대상 돌진은 저작된 지연 뒤에 정해진 속도와 거리로 진행한다.
	// 이 분기가 남은 클립 루트모션보다 우선하므로 두 이동을 중복 적용하지 않는다.
	if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
		boss.ePatternStageMotionKind)
	{
		if (!boss.bPortalRushTargetLocked ||
			!std::isfinite(boss.fPortalRushSpeedMps) ||
			boss.fPortalRushSpeedMps <= 0.f ||
			!std::isfinite(boss.fPortalRushDistanceM) ||
			boss.fPortalRushDistanceM <= 0.f)
		{
			return false;
		}
		const float delaySeconds = static_cast<float>(
			boss.iPortalRushRetargetDelayMs) * MILLISECONDS_TO_SECONDS;
		const float travelSeconds =
			boss.fPortalRushDistanceM / boss.fPortalRushSpeedMps;
		const float previousStageSeconds = (std::max)(
			0.f, boss.fActionElapsedSeconds - fixedDeltaSeconds);
		const float previousTravelSeconds = std::clamp(
			previousStageSeconds - delaySeconds, 0.f, travelSeconds);
		const float currentTravelSeconds = std::clamp(
			boss.fActionElapsedSeconds - delaySeconds, 0.f, travelSeconds);
		const float distance = (currentTravelSeconds - previousTravelSeconds) *
			boss.fPortalRushSpeedMps;
		if (!std::isfinite(distance) || distance <= 0.f)
			return false;
		const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
		outProposedX = boss.fPositionX + std::sin(yawRadians) * distance;
		outProposedZ = boss.fPositionZ + std::cos(yawRadians) * distance;
		return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
	}

	// 루트모션 샘플이 있으면 이전 tick과 현재 tick의 누적 거리 차를 이번 이동량으로 사용한다.
	// 샘플이 없고 FORWARD 이동이 선언된 경우에는 stage 거리/시간으로 구한 속도를 사용한다.
	float distance = 0.f;
	if (!boss.PatternStageRootMotion.empty())
	{
		const float previousSeconds = (std::max)(
			0.f, boss.fActionElapsedSeconds - fixedDeltaSeconds);
		float previousForward = 0.f;
		float currentForward = 0.f;
		Sample_StageRootMotionForward(
			boss.PatternStageRootMotion, previousSeconds, previousForward);
		Sample_StageRootMotionForward(
			boss.PatternStageRootMotion, boss.fActionElapsedSeconds,
			currentForward);
		distance = currentForward - previousForward;
		if (0.f == distance)
			return false;
	}
	else
	{
		if (BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD !=
			boss.ePatternStageMotionKind ||
			!std::isfinite(boss.fPatternForcedMotionSpeed) ||
			boss.fPatternForcedMotionSpeed <= 0.f)
		{
			return false;
		}
		distance = boss.fPatternForcedMotionSpeed * fixedDeltaSeconds;
	}
	if (!std::isfinite(distance))
		return false;
	const float yawRadians = boss.fYawDegrees * DEGREES_TO_RADIANS;
	outProposedX = boss.fPositionX + std::sin(yawRadians) * distance;
	outProposedZ = boss.fPositionZ + std::cos(yawRadians) * distance;
	return std::isfinite(outProposedX) && std::isfinite(outProposedZ);
}

// GameRoom의 피날레 반복 처리에서 같은 고정 revision의 패턴을 새 sequence로 다시 시작한다.
// 피날레 종류/생존/시간/sequence 조건이 맞지 않으면 상태를 바꾸지 않고 false다.
bool LostArk::Server::CValtanBrain::Restart_FinaleCycle(
	SERVER_WORLD_ENTITY& boss,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick) const
{
	if (BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != pattern.Finale.eKind ||
		pattern.Stages.empty() || 0u == boss.iCurrentHp ||
		0u == serverTick ||
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence)
		return false;
	const auto revision = boss.PinnedDefinitionRevision;
	// 재시작 호출에는 미리 계산한 최근접 대상이 없다.
	// BeginPattern의 같은 대상 정책을 다시 거쳐 현재 보스 위치를 기준으로 선택한다.
	BeginPattern(boss, pattern, players, nullptr, revision, serverTick);
	return true;
}

// EnterPatternStage가 새 이동 정의를 boss의 포탈 실행 상태로 복사한다.
// 대상 돌진은 시작점만 준비하고 별도 잠금 호출이 경로를 확정한다. 횡단은 모서리와 반대 경계로 위치를 설정한다.
void LostArk::Server::CValtanBrain::Configure_PortalMotion(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage)
{
	boss.bPortalMotionActive = Is_PortalMotion(stage.Motion.eKind);
	boss.bPortalRushTargetLocked = false;
	boss.iPortalRushRetargetDelayMs = 0u;
	boss.fPortalRushSpeedMps = 0.f;
	boss.fPortalRushDistanceM = 0.f;
	if (!boss.bPortalMotionActive)
		return;
	boss.MovePath.clear();
	if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH == stage.Motion.eKind)
	{
		boss.iPortalRushRetargetDelayMs = stage.Motion.iRetargetDelayMs;
		boss.fPortalRushSpeedMps = stage.Motion.fSpeedMps;
		boss.fPortalRushDistanceM = stage.Motion.fDistance;
		boss.fPortalStartX = boss.fPositionX;
		boss.fPortalStartZ = boss.fPositionZ;
		boss.fPortalEndX = boss.fPortalStartX;
		boss.fPortalEndZ = boss.fPortalStartZ;
		boss.fPortalLastHitSampleX = boss.fPositionX;
		boss.fPortalLastHitSampleZ = boss.fPositionZ;
		return;
	}
	const float signX = 0u == stage.Motion.iCornerIndex ||
		3u == stage.Motion.iCornerIndex ? -1.f : 1.f;
	const float signZ = stage.Motion.iCornerIndex < 2u ? -1.f : 1.f;
	boss.fPortalStartX = boss.fSpawnPositionX + signX * stage.Motion.fHalfExtentsX;
	boss.fPortalStartZ = boss.fSpawnPositionZ + signZ * stage.Motion.fHalfExtentsZ;
	float deltaX = boss.bHasPatternTargetLastPosition ?
		boss.fPatternTargetLastPositionX - boss.fPortalStartX :
		-signX * stage.Motion.fHalfExtentsX;
	float deltaZ = boss.bHasPatternTargetLastPosition ?
		boss.fPatternTargetLastPositionZ - boss.fPortalStartZ :
		-signZ * stage.Motion.fHalfExtentsZ;
	if (deltaX * signX >= -0.001f || deltaZ * signZ >= -0.001f)
	{
		deltaX = -signX * stage.Motion.fHalfExtentsX;
		deltaZ = -signZ * stage.Motion.fHalfExtentsZ;
	}
	const float scale = (std::min)(
		2.f * stage.Motion.fHalfExtentsX / std::fabs(deltaX),
		2.f * stage.Motion.fHalfExtentsZ / std::fabs(deltaZ));
	boss.fPortalEndX = boss.fPortalStartX + deltaX * scale;
	boss.fPortalEndZ = boss.fPortalStartZ + deltaZ * scale;
	boss.fPositionX = boss.fPortalStartX;
	boss.fPositionY = boss.fSpawnPositionY;
	boss.fPositionZ = boss.fPortalStartZ;
	boss.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	boss.fPortalLastHitSampleX = boss.fPositionX;
	boss.fPortalLastHitSampleZ = boss.fPositionZ;
}

// GameRoom의 stage action commit이 대상 재선택 이후 호출하는 공개 진입점이다. 내부 경로 잠금 결과를 전달한다.
bool LostArk::Server::CValtanBrain::Lock_PortalTargetRushAtStageStart(
	SERVER_WORLD_ENTITY& boss)
{
	return LockPortalTargetRushAtStageStart(boss);
}

// GameRoom이 벽 충돌을 확정한 뒤 현재 stage의 WALL_CONTACT 분기를 적용한다.
// 분기를 처리하면 강제 이동 속도를 0으로 두며, 벽 파괴/후속 stage action의 transaction은 GameRoom이 소유한다.
bool LostArk::Server::CValtanBrain::Complete_ImpactStage(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick) const
{
	if (!boss.bPatternChargeImpact)
		return false;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	const BOSS_PATTERN_DEFINITION* pattern = nullptr;
	if (nullptr != patterns)
		pattern = FindPattern(*patterns, boss.strPatternId);
	if (nullptr == pattern ||
		boss.iPatternStageIndex >= pattern->Stages.size())
		return false;
	const BOSS_PATTERN_STAGE_DEFINITION& stage =
		pattern->Stages[boss.iPatternStageIndex];
	const auto branch = std::find_if(
		stage.Branches.begin(), stage.Branches.end(),
		[](const BOSS_PATTERN_STAGE_BRANCH& candidate)
		{
			return BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT ==
				candidate.eOutcome;
		});
	if (stage.Branches.end() == branch ||
		!ApplyStageBranch(boss, *pattern, *branch, serverTick))
	{
		return false;
	}
	boss.fPatternForcedMotionSpeed = 0.f;
	return true;
}

// GameRoom이 잡기 돌진의 navigation 막힘을 확인한 뒤 호출한다.
// NAVIGATION_BLOCKED 분기가 있어야 진입하며, 확정 명단에 맞는 ALL/ANY 잡기 분기를 우선 적용한다.
bool LostArk::Server::CValtanBrain::Complete_NavigationBlockedStage(
	SERVER_WORLD_ENTITY& boss,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick) const
{
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	const auto* pattern = nullptr == patterns ? nullptr :
		FindPattern(*patterns, boss.strPatternId);
	if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size())
		return false;
	const auto& stage = pattern->Stages[boss.iPatternStageIndex];
	const auto blocked = std::find_if(stage.Branches.begin(), stage.Branches.end(),
		[](const BOSS_PATTERN_STAGE_BRANCH& branch)
		{
			return BOSS_PATTERN_STAGE_OUTCOME::NAVIGATION_BLOCKED == branch.eOutcome;
		});
	if (blocked == stage.Branches.end())
		return false;
	const SERVER_BOSS_GRAB_ROSTER roster = Classify_GrabbedPlayers(boss, players);
	for (const auto outcome : { BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED,
		BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED })
	{
		const bool eligible = BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED ==
			outcome ? SERVER_BOSS_GRAB_CLASSIFICATION::ALL == roster.eClassification :
			roster.iGrabbedCount > 0u;
		if (!eligible)
			continue;
		const auto branch = std::find_if(stage.Branches.begin(), stage.Branches.end(),
			[outcome](const BOSS_PATTERN_STAGE_BRANCH& candidate)
			{
				return outcome == candidate.eOutcome;
			});
		if (branch != stage.Branches.end())
			return ApplyStageBranch(boss, *pattern, *branch, serverTick);
	}
	return ApplyStageBranch(boss, *pattern, *blocked, serverTick);
}
