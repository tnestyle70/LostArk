// 룸 도우미가 사용하는 player·boss·bootstrap·transaction 형식을 제공한다.
#include "GameRoom.h"

// ClientSession·ServerCombatHitRuntime·PacketWriter·WorldCollision API는 아래 helper 본문이 직접 호출하지 않는 기존 include다.
#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

// 입장·pattern-flow 요청 검증과 world/파괴 wire enum 변환을 사용한다.
#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

// ID 문자 검사·정적 목록 검색·hit 최대 범위를 계산한다.
#include <algorithm>
// 공통 내부 헤더가 제공하는 정적 Mario/발탄 정책 배열을 조회한다.
#include <array>
// 실제 Unix ms와 성능 계측의 steady_clock 경과 μs를 구분한다.
#include <chrono>
// stable flow ID의 허용 문자 분류에 unsigned byte를 전달한다.
#include <cctype>
// hit 외곽 길이·착지 높이 차이의 수치 계산을 사용한다.
#include <cmath>
// tick·sequence·seed의 고정 폭 정수와 wrap 계산을 사용한다.
#include <cstdint>
// 테스트 모드 환경 문자열을 _dupenv_s로 받고 free로 해제한다.
#include <cstdlib>
// iostream은 이 본문에서 직접 사용하지 않는 기존 include다.
#include <iostream>
// 예약 0을 건너뛴 tick wrap에서 uint32 최댓값을 사용한다.
#include <limits>
// new·set은 이 본문에서 직접 사용하지 않는 기존 include다.
#include <new>
#include <set>
// ID 검사 입력과 정적 enqueue 진단명을 비소유 view로 전달한다.
#include <string_view>
// 지역 collision/navigation 후보를 출력 vector에 이동한다.
#include <utility>

// 아래 함수의 선언과 룸 공통 tick·Mario rail·발탄 정책 상수를 공유한다.
#include "GameRoom_Internal.h"

namespace GameRoomDetail
{

	// 쿠크 패턴 준비/시작이 룸 navigation을 차용해 boss motion의 진입 조건을 읽는다. motion이 없으면 true이며 검사 중 상태를 변경하지 않는다.
	bool Is_KoukuBossMotionNavigable(const BOSS_PATTERN_DEFINITION& pattern, const CServerNavigation& navigation)
	{
		if (!pattern.BossMotion)
			return true;
		const auto& motion = *pattern.BossMotion;
		if (!motion.Keys.empty())
		{
			SERVER_NAV_POINT ground{};
			// 명시적 키가 있는 연출은 경로 전체를 navigation에 제한하지 않는다. 시작 XZ의 보행 가능 여부와 샘플 높이가 저작 Y에서 1m 이내인지 확인한다.
			return navigation.Is_PointWalkableExact(motion.StartPosition[0], motion.StartPosition[2]) &&
				navigation.Sample_Position(motion.StartPosition[0], motion.StartPosition[2], ground) &&
				std::abs(ground.y - motion.StartPosition[1]) <= 1.f;
		}
		return navigation.Is_PointWalkableExact(motion.StartPosition[0], motion.StartPosition[2]) &&
			navigation.Is_PointWalkableExact(motion.EndPosition[0], motion.EndPosition[2]) &&
			navigation.Has_LineOfSight(
				motion.StartPosition[0], motion.StartPosition[2], motion.EndPosition[0], motion.EndPosition[2]);
	}

	// 이동/스킬 명령이 실제 망치 타격 tick 이후의 recovery만 취소한다.
	// 미로 INTERACTION·유효 시작 tick·전환 미진행을 확인한 뒤 action/skill/timer/pending command를 지운다. 타격 전이나 탈출 이동 중이면 false로 상태를 유지한다.
	bool Cancel_MazeHammerRecovery(LostArk::Server::SERVER_PLAYER& player, const std::uint32_t tick)
	{
		using namespace LostArk::Shared;
		if (PLAYER_ACTION_STATE::INTERACTION != player.eAction || KOUKU_HUD_MODE::MAZE != player.eKoukuHudMode ||
			0u != player.CardMaze.transferStartTick || 0u == player.iActionStartTick ||
			static_cast<std::int32_t>(tick -
				(player.iActionStartTick +
					LostArk::Server::CKoukuCardMazeRuntime::Hammer_HitTickOffset(player.iCurrentSkillId))) < 0)
		{
			return false;
		}
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u;
		player.fActionElapsedSeconds = 0.f;
		player.PendingCommand.Clear();
		return true;
	}

	// Mario 이동/충돌 보정이 world XZ를 player의 rail 직선에 투영한다. rail이 없거나 축 길이 제곱이 0.5 미만이면 두 출력은 유지된다.
	void Project_MarioRailPoint(const SERVER_PLAYER& player, float& x, float& z)
	{
		if (0u == player.iMarioStage || !player.bMarioRailReady)
			return;
		const double axisX = player.fMarioRailRightX;
		const double axisZ = player.fMarioRailRightZ;
		const double lengthSquared = axisX * axisX + axisZ * axisZ;
		if (lengthSquared < 0.5)
			return;
		// 저장된 right 축이 정확히 단위 길이라는 가정 없이 길이 제곱으로 나누어 world XZ 직선에 정사영한다. 구간 양 끝이나 높이·충돌은 여기서 제한하지 않는다.
		const double along = ((static_cast<double>(x) - player.fMarioRailOriginX) * axisX +
			(static_cast<double>(z) - player.fMarioRailOriginZ) * axisZ) /
			lengthSquared;
		x = static_cast<float>(player.fMarioRailOriginX + axisX * along);
		z = static_cast<float>(player.fMarioRailOriginZ + axisZ * along);
	}

	// 레이드 출구 검사가 process 환경 변수의 현재 값을 읽는다. 동적 할당한 환경 문자열은 함수 안에서 해제한다.
	bool Is_RaidClearTestModeEnabled()
	{
		char* value = nullptr;
		size_t valueLength = 0u;
		if (0 != _dupenv_s(&value, &valueLength, RAID_CLEAR_TEST_MODE_ENV))
			return false;
		// _dupenv_s의 길이에는 끝 NUL이 포함되므로 길이 2와 첫 byte 1을 함께 요구한다. 성공/비활성 모두 이 함수가 받은 buffer를 해제한다.
		const bool enabled = nullptr != value && 2u == valueLength && '1' == value[0];
		std::free(value);
		return enabled;
	}

	// 입장 진단의 실제 시각을 Unix ms로 기록한다. simulation deadline용 단조 clock은 아니다.
	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
				.count());
	}

	// pattern flow 요청의 ID 길이와 허용 byte를 검사한다. isalnum은 현재 C locale의 분류를 따른다.
	bool Is_StablePatternFlowId(const std::string_view value)
	{
		return !value.empty() && value.size() <= MAX_STABLE_NETWORK_ID_BYTES &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' || character == '-' || character == '.';
				});
	}

	// pattern flow 저작 revision이 고정 길이 소문자 16진 문자열인지 검사한다.
	bool Is_PatternFlowRevision(const std::string_view value)
	{
		return VALTAN_PATTERN_FLOW_REVISION_HEX_BYTES == value.size() &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f');
				});
	}

	// 중복 시작 요청을 대조할 문자열을 만든다. 요청 sequence를 포함하지 않으며 ID 검증·구분자 제외는 호출자에서 수행한다.
	std::string Build_PatternFlowStartRequestIdentity(const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request)
	{
		// revision·boss·flow·시작 slot·추적 대기시간·순서 있는 slot 목록을 포함한다. 같은 명령 sequence의 중복 요청이 payload도 같은지 호출자가 이 문자열로 비교한다.
		std::string identity = Format_GameplayDataRevision(request.ExpectedDefinitionRevision) + "|" +
			request.strBossPlacementId + "|" + request.strFlowId + "|" + request.strFlowRevision + "|" +
			request.strStartSlotId + "|" + std::to_string(request.iInterStepPursuitMs);
		for (const VALTAN_PATTERN_FLOW_SLOT_WIRE& slot : request.Slots)
		{
			identity += "|" + slot.strSlotId + "=" + slot.strPatternId;
		}
		return identity;
	}

	// 각 hit shape의 원점 기준 최대 외곽 범위 두 배(m)를 volley 원점의 기본 최소 간격으로 반환한다.
	float Resolve_VolleyMinimumSpacing(const BOSS_COMBAT_OBJECT_DEFINITION& definition)
	{
		// 원/고리는 바깥 반경, cone은 길이, 상자·cross·six 방향은 길이/반폭의 대각 범위를 취한다. hit가 없거나 알려진 shape가 없으면 결과 0이다.
		float maximumExtent = 0.f;
		for (const BOSS_COMBAT_OBJECT_HIT& hit : definition.Hits)
		{
			float extent = 0.f;
			switch (hit.eHitShape)
			{
			case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
			case BOSS_PATTERN_HIT_SHAPE::RING:
				extent = hit.fHitOuterRadius;
				break;
			case BOSS_PATTERN_HIT_SHAPE::CONE:
				extent = hit.fHitLength;
				break;
			case BOSS_PATTERN_HIT_SHAPE::BOX:
			case BOSS_PATTERN_HIT_SHAPE::CROSS:
			case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
				extent = std::sqrt(hit.fHitLength * hit.fHitLength + hit.fHitHalfWidth * hit.fHitHalfWidth);
				break;
			default:
				break;
			}
			maximumExtent = (std::max)(maximumExtent, extent);
		}
		return maximumExtent * 2.f;
	}

	// 룸 난수 선택용 64비트 입력을 순수 정수 연산으로 섞는다. 전역 난수 상태를 소비하지 않는다.
	std::uint64_t Mix_DeterministicRandom(std::uint64_t value)
	{
		value += 0x9e3779b97f4a7c15ull;
		value = (value ^ (value >> 30u)) * 0xbf58476d1ce4e5b9ull;
		value = (value ^ (value >> 27u)) * 0x94d049bb133111ebull;
		return value ^ (value >> 31u);
	}

	// stable ID의 원문 byte를 고정 순서로 누적해 결정적 seed를 만든다. 문자열 정규화나 고유성 보장은 없다.
	std::uint64_t Hash_StableId(const std::string& value)
	{
		std::uint64_t hash = 1469598103934665603ull;
		for (const unsigned char character : value)
		{
			hash ^= character;
			hash *= 1099511628211ull;
		}
		return hash;
	}

	// 섞은 값의 상위 24bit를 2²⁴로 나누어 0 이상 1 미만의 float를 반환한다.
	float DeterministicUnitFloat(const std::uint64_t value)
	{
		return static_cast<float>((Mix_DeterministicRandom(value) >> 40u) & 0xffffffull) / 16777216.f;
	}

	// 파괴 transaction을 collision/navigation 후보 배열로 변환한다. 두 출력을 먼저 비우며 실제 시스템 commit은 호출자가 맡는다.
	void Build_WorldDestructionStateChanges(const WORLD_DESTRUCTION_TRANSACTION& transaction,
		std::vector<SERVER_COLLISION_STATE_CHANGE>& collisionChanges,
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& navigationChanges)
	{
		// 출력을 먼저 비우고 각 transition을 순서대로 반영한다. INTACT/BREAKING은 player 차단, INTACT만 impact receiver 활성으로 변환한다.
		collisionChanges.clear();
		navigationChanges.clear();
		for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition : transaction.Transitions)
		{
			if (!transition.strCollisionStateId.empty())
			{
				SERVER_COLLISION_STATE_CHANGE change{};
				change.strPlacementId = transition.strCollisionStateId;
				change.bPlayerBlocking = WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState ||
					WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState;
				change.bImpactReceiverEnabled = WORLD_DESTRUCTION_STATE::INTACT == transition.eNextState;
				collisionChanges.push_back(std::move(change));
			}
			// navigation은 영속 변경이면서 condition ID가 있는 경우만 추가한다. FRACTURED/DESPAWNED에서 condition을 켜며 실제 nav 변경은 수행하지 않는다.
			if (transition.bApplyPersistentMutation && !transition.strNavigationStateId.empty())
			{
				SERVER_NAVIGATION_CONDITION_CHANGE change{};
				change.strConditionId = transition.strNavigationStateId;
				change.bValue = WORLD_DESTRUCTION_STATE::FRACTURED == transition.eNextState ||
					WORLD_DESTRUCTION_STATE::DESPAWNED == transition.eNextState;
				navigationChanges.push_back(std::move(change));
			}
		}
	}

	// 같은 group에 모이는 파괴 변경의 상태·version·member·충돌/nav 목표를 비교한다. binding application의 출처는 비교하지 않는다.
	bool Is_SameDestructionTransitionTarget(
		const WORLD_DESTRUCTION_STATE_TRANSITION& left, const WORLD_DESTRUCTION_STATE_TRANSITION& right)
	{
		return left.strGroupId == right.strGroupId && left.strMutationId == right.strMutationId &&
			left.ePreviousState == right.ePreviousState && left.eNextState == right.eNextState &&
			left.eFinalState == right.eFinalState && left.iPreviousStateVersion == right.iPreviousStateVersion &&
			left.iNextStateVersion == right.iNextStateVersion && left.iCommitTick == right.iCommitTick &&
			left.bApplyPersistentMutation == right.bApplyPersistentMutation &&
			left.MemberPlacementIds == right.MemberPlacementIds &&
			left.strCollisionStateId == right.strCollisionStateId &&
			left.strNavigationStateId == right.strNavigationStateId;
	}

	// 룸 성능 계측이 steady_clock의 비음수 경과를 정수 μs로 내림한다. 음수 입력을 별도로 거절하지는 않는다.
	std::uint64_t To_Microseconds(const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
	}

	// 룸 ingress에서 MOVE와 UPDATE_SKILL_AIM만 합치기/용량 제한 대상인 best-effort로 분류한다.
	bool Is_BestEffortCommand(const ROOM_COMMAND_TYPE type)
	{
		return ROOM_COMMAND_TYPE::MOVE == type || ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == type;
	}

	// enqueue 진단이 정적 결과명 문자열을 차용한다. 알 수 없는 enum은 UNKNOWN이다.
	std::string_view To_RoomCommandEnqueueResultName(const ROOM_COMMAND_ENQUEUE_RESULT result)
	{
		switch (result)
		{
		case ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED:
			return "ACCEPTED";
		case ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT:
			return "DROPPED_BEST_EFFORT";
		case ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP:
			return "DEDUPLICATED_CLEANUP";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND:
			return "REJECTED_INVALID_COMMAND";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY:
			return "REJECTED_ROOM_NOT_READY";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_SEALED:
			return "REJECTED_ROOM_SEALED";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP:
			return "REJECTED_PENDING_CLEANUP";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY:
			return "REJECTED_RELIABLE_CAPACITY";
		case ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING:
			return "REJECTED_BINDING_MISSING";
		default:
			return "UNKNOWN";
		}
	}

	// 같은 session·명령 종류를 확인하고 aim이면 skill ID까지 같아야 같은 합치기 stream으로 본다.
	bool Is_SameBestEffortStream(const ROOM_COMMAND& queued, const ROOM_COMMAND& incoming)
	{
		if (queued.iSessionId != incoming.iSessionId || queued.eType != incoming.eType ||
			!Is_BestEffortCommand(incoming.eType))
		{
			return false;
		}
		if (ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM == incoming.eType)
		{
			return queued.UpdateSkillAim.iSkillId == incoming.UpdateSkillAim.iSkillId;
		}
		return true;
	}

	// CGameRoom::Enqueue_Detailed가 command mutex를 잡은 채 호출한다. 같은 session의 가장 뒤 명령이 같은 stream일 때 그 한 개만 지우며 새 명령 append는 호출자가 한다.
	bool Try_RemoveCoalescedCommand(std::deque<ROOM_COMMAND>& commands, const ROOM_COMMAND& incoming)
	{
		// 다른 session 명령은 건너뛰되 같은 session에서 만난 첫 명령이 다른 stream이면 중단한다. 새 move/aim을 skill·release·다른 lifecycle 명령 앞까지 합치지 않는다.
		for (std::size_t index = commands.size(); index > 0u; --index)
		{
			const ROOM_COMMAND& queued = commands[index - 1u];
			if (queued.iSessionId != incoming.iSessionId)
				continue;
			if (!Is_SameBestEffortStream(queued, incoming))
				return false;
			commands.erase(commands.begin() + static_cast<std::ptrdiff_t>(index - 1u));
			return true;
		}
		return false;
	}

	// 이동 yaw 차이의 유한한 도 값을 -180~180 범위로 접는다. 경계 두 값은 그대로 두며 비유한 입력 검사는 호출자 책임이다.
	float Wrap_Degrees(float degrees)
	{
		while (degrees > 180.f)
			degrees -= 360.f;
		while (degrees < -180.f)
			degrees += 360.f;
		return degrees;
	}

	// 0이면 경과 0, 그 밖에는 예약 0을 건너뛰는 한 번의 wrap 차이를 반환한다. 역방향 여부나 여러 회전 경과는 구분하지 않는다.
	std::uint64_t Elapsed_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick, const std::uint32_t currentTick)
	{
		if (0u == startTick || 0u == currentTick)
			return 0u;
		return currentTick >= startTick
			? static_cast<std::uint64_t>(currentTick - startTick)
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - startTick) + currentTick;
	}

	// 기한과 같거나 signed32 차이가 양수인지 판정한다. 기한 간격이 반주기 미만이라는 룸 예약 계약을 따르며 0을 특별히 거절하지 않는다.
	bool Has_ReachedServerTick(const std::uint32_t currentTick, const std::uint32_t targetTick)
	{
		return currentTick == targetTick || static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	// 유령 재배치의 소유 표시가 켜진 hidden/invulnerable flag만 내리고 예약 tick·소유 표시를 비운다. 소유 표시가 없는 flag는 변경하지 않는다.
	void Clear_ValtanGhostRelocationState(SERVER_WORLD_ENTITY& boss)
	{
		// 이번 재배치가 직접 도입했다고 기록한 flag만 해제한다. 다른 경로가 소유한 무적/숨김까지 함께 지우지 않도록 소유 표시를 검사한다.
		if (boss.bGhostRelocationOwnsHiddenFlag)
		{
			(void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, false);
		}
		if (boss.bGhostRelocationOwnsInvulnerableFlag)
		{
			(void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, false);
		}
		boss.bGhostRepositionPending = false;
		boss.iGhostReappearTick = 0u;
		boss.bGhostRelocationRetryPending = false;
		boss.iGhostRelocationRetryTick = 0u;
		boss.bGhostRelocationOwnsHiddenFlag = false;
		boss.bGhostRelocationOwnsInvulnerableFlag = false;
	}

	// ms를 64비트 중간 곱으로 30Hz tick에 올림한다. 0ms는 0tick이며 단위 변환만 한다.
	std::uint32_t DurationMillisecondsToServerTicks(const std::uint32_t durationMs)
	{
		return static_cast<std::uint32_t>((static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ + 999u) / 1000u);
	}

	// 패턴 구속 적용이 진행 스킬·투사체·이동/낙하/피격 예약을 지우고 전투 준비를 끈다. HP·인벤토리나 룸 combat-object 저장소는 이 함수가 수정하지 않는다.
	void Cancel_PlayerActionForPatternStatus(SERVER_PLAYER& player)
	{
		// 현재 행동과 연결된 타격·투사체·이동 예약을 정리한다. pattern bind/silence 자체를 해제하는 호출은 포함하지 않는다.
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iActionStartTick = 0u;

		// 부착 상태와 낙하 속도·사망 예약 tick을 지운다.
		player.Clear_Attachment();
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;

		// 행동 경과 시간과 스킬 조준 값을 초기 상태로 돌린다.
		player.fActionElapsedSeconds = 0.f;
		player.fSkillAimDirectionX = 0.f;
		player.fSkillAimDirectionZ = 1.f;
		player.fSkillAimDistance = 0.f;

		// 이번 행동의 타격 적용 기록과 player 소유 투사체를 비운다.
		player.hasAppliedSkillDamage = false;
		player.iAppliedHitMask = 0u;
		player.HitWindowTargets.clear();
		player.iSpawnedProjectileMask = 0u;
		player.Projectiles.clear();

		// 콤보 단계와 보류 입력·조준·홀드 해제 상태를 정리한다.
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.fBufferedComboAimX = 0.f;
		player.fBufferedComboAimZ = 1.f;
		player.fBufferedComboAimDistance = 0.f;
		player.hasReleasedHold = false;

		// 강제 이동과 일반 목적지·경로 진행 상태를 비운다.
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;

		// 넉백 속도·잔여 시간과 넘어짐·피격 유예의 종료 tick을 지운다.
		player.fKnockbackDirectionX = 0.f;
		player.fKnockbackDirectionZ = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.fKnockbackRemainingSeconds = 0.f;
		player.iKnockdownEndTick = 0u;
		player.iHitReactionGraceEndTick = 0u;

		// 다음 준비 경로가 명시적으로 다시 켤 때까지 전투 준비를 해제한다.
		player.isCombatReady = false;
	}

	// Character Select audition이 필요한 지형/기믹이 없는 패턴을 정적 stable ID 목록으로 거른다.
	bool Is_CharacterSelectEnvironmentDependentPattern(const std::string& patternId)
	{
		return std::any_of(CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS.begin(),
			CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS.end(),
			[&patternId](const char* blockedPatternId)
			{
				return patternId == blockedPatternId;
			});
	}

	// root sequence와 pinned revision의 후속 패턴이 예약/실행 중인지 읽는다. pending이 존재하면 그 identity 결과를 바로 반환한다.
	bool Is_ValtanOutcomeFollowupInFlight(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence, const LostArk::Shared::GameplayDataRevision& definitionRevision)
	{
		if (0u == rootPatternSequence || !definitionRevision.Is_Valid())
			return false;
		if (boss.PendingPatternFollowup.Is_Pending())
		{
			return boss.PendingPatternFollowup.iRootPatternSequence == rootPatternSequence &&
				boss.PendingPatternFollowup.PinnedDefinitionRevision == definitionRevision;
		}
		return boss.iPatternFollowupDepth > 0u && !boss.strPatternId.empty() &&
			boss.iPatternFollowupRootSequence == rootPatternSequence &&
			boss.PinnedDefinitionRevision == definitionRevision;
	}

	// 후속 예약과 실행 패턴이 없고 같은 root의 COMPLETED receipt·revision이 남았는지 확인한다. IDLE만으로 성공 완료를 추정하지 않는다.
	bool Has_ValtanOutcomeGroupCompleted(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence, const LostArk::Shared::GameplayDataRevision& definitionRevision)
	{
		return 0u != rootPatternSequence && definitionRevision.Is_Valid() && boss.strPatternId.empty() &&
			!boss.PendingPatternFollowup.Is_Pending() &&
			boss.PatternTerminalReceipt.iRootPatternSequence == rootPatternSequence &&
			LostArk::Server::SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == boss.PatternTerminalReceipt.eResult &&
			boss.PinnedDefinitionRevision == definitionRevision;
	}

	// Debug 지형 선택 wire 값을 runtime 상태와 정적 label로 바꾼다. 모르는 값은 false이며 두 출력은 보존한다.
	bool Resolve_ValtanArenaPreset(
		const std::uint32_t rawPreset, LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& outState, const char*& outLabel)
	{
		using LostArk::Shared::VALTAN_ARENA_PRESET;
		using LostArk::Server::VALTAN_TIMELINE_ARENA_STATE;
		switch (static_cast<VALTAN_ARENA_PRESET>(rawPreset))
		{
		case VALTAN_ARENA_PRESET::FRESH:
			outState = VALTAN_TIMELINE_ARENA_STATE::FRESH;
			outLabel = "Fresh / all walls intact";
			return true;
		case VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE:
			outState = VALTAN_TIMELINE_ARENA_STATE::ALL_WALLS_GONE;
			outLabel = "Circle / all walls gone";
			return true;
		case VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_GONE;
			outLabel = "3 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR30_GONE;
			outLabel = "9 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN:
			outState = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE;
			outLabel = "3 and 9 o'clock broken";
			return true;
		case VALTAN_ARENA_PRESET::END:
		default:
			return false;
		}
	}

	// Debug 시작 페이지의 stable timeline row ID를 정적 정책 배열에서 찾는다. 포인터 수명은 프로그램 수명이며 누락은 nullptr다.
	const VALTAN_FIGHT_PAGE_POLICY* Find_ValtanFightPagePolicy(const std::string& rowId)
	{
		const auto found = std::find_if(VALTAN_FIGHT_PAGE_POLICIES.begin(), VALTAN_FIGHT_PAGE_POLICIES.end(),
			[&rowId](const VALTAN_FIGHT_PAGE_POLICY& policy)
			{
				return rowId == policy.pTimelineRowId;
			});
		return VALTAN_FIGHT_PAGE_POLICIES.end() == found ? nullptr : &*found;
	}

	// timeline audition이 현재 HP에 따라 DEAD/NONE을 정하고 skill·이동·부착·구속·침묵을 정리해 전투 준비를 끈다. 회복은 수행하지 않는다.
	void Freeze_TimelineAuditionPlayer(SERVER_PLAYER& player)
	{
		// 현재 HP에 맞는 행동 상태를 남기고 진행 스킬과 타깃·시작 tick을 해제한다.
		player.eAction = 0u == player.iCurrentHp ? PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iActionStartTick = 0u;

		// 부착·패턴 구속·침묵과 낙하 상태를 함께 해제한다.
		player.Clear_Attachment();
		player.Clear_PatternBindStatus();
		player.Clear_SilenceStatus();
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;

		// 행동 경과 시간과 스킬 조준 값을 초기 상태로 돌린다.
		player.fActionElapsedSeconds = 0.f;
		player.fSkillAimDirectionX = 0.f;
		player.fSkillAimDirectionZ = 1.f;
		player.fSkillAimDistance = 0.f;

		// 이번 행동의 타격 적용 기록과 player 소유 투사체를 비운다.
		player.hasAppliedSkillDamage = false;
		player.iAppliedHitMask = 0u;
		player.HitWindowTargets.clear();
		player.iSpawnedProjectileMask = 0u;
		player.Projectiles.clear();

		// 콤보 단계와 보류 입력·조준·홀드 해제 상태를 정리한다.
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.PendingCommand.Clear();
		player.fBufferedComboAimX = 0.f;
		player.fBufferedComboAimZ = 1.f;
		player.fBufferedComboAimDistance = 0.f;
		player.hasReleasedHold = false;

		// 강제 이동과 일반 목적지·경로 진행 상태를 비운다.
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;

		// 다음 준비 경로가 명시적으로 다시 켤 때까지 전투 준비를 해제한다.
		player.isCombatReady = false;
	}

	// audition 시작/다음 단계가 Freeze 뒤 HP·자원·identity를 채우고 광기/상호작용을 초기화해 지정 tick으로 준비한다. 입력 tick의 유효성은 호출자 책임이다.
	void Prepare_TimelineAuditionPlayer(SERVER_PLAYER& player, const std::uint32_t actionTick)
	{
		// 기존 행동 정리를 먼저 수행한 뒤 시작용 자원을 채운다. 전투 준비가 켜질 때에는 지정 actionTick과 초기 광기 상태도 함께 반영된다.
		Freeze_TimelineAuditionPlayer(player);
		player.bRonaunGuard = false;
		player.iRonaunGrantTick = 0u;
		player.iCurrentHp = player.iMaximumHp;
		player.iCurrentResource = player.iMaximumResource;
		player.iResourceAccumulator = 0u;
		player.iCurrentIdentity = player.iMaximumIdentity;
		player.iIdentityAccumulator = 0u;
		player.iCurrentMadness = 0u;
		player.dMadnessRemainder = 0.;
		player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
		player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		player.Clear_KoukuInteractionState();
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = actionTick;
		player.isCombatReady = true;
	}

	// 룸 입장이 protocol·world·playable class·닉네임·음성·외형 형식만 검사한다. 정원·session binding·실제 spawn 승인은 별도 입장 처리다.
	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message)
	{
		return message.iProtocolVersion == NETWORK_PROTOCOL_VERSION && Is_Known_World_Id(message.eWorldId) &&
			Is_Supported_Playable_Character_Class(message.eCharacterClass) &&
			Is_Valid_PlayerNickname(message.strNickName) && Is_Valid_VoiceType(message.iVoiceType) &&
			Is_Valid_PlayerAppearance(message.strAppearanceJson, message.eCharacterClass);
	}

	// 명령 sequence 0을 거부하고 signed32 차이로 최신 여부를 비교한다. 반주기 이상 떨어진 요청의 순서를 확정하는 함수는 아니다.
	bool Is_NewerSequence(const std::uint32_t candidate, const std::uint32_t previous)
	{
		return 0u != candidate && static_cast<std::int32_t>(candidate - previous) > 0;
	}

	// snapshot이 world bootstrap 종류를 wire enum으로 바꾼다. 지원하지 않는 값은 END로 표시한다.
	WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind)
	{
		switch (kind)
		{
		case WORLD_BOOTSTRAP_KIND::NPC:
			return WORLD_ENTITY_KIND::NPC;
		case WORLD_BOOTSTRAP_KIND::BOSS:
			return WORLD_ENTITY_KIND::BOSS;
		case WORLD_BOOTSTRAP_KIND::MONSTER:
			return WORLD_ENTITY_KIND::MONSTER;
		default:
			return WORLD_ENTITY_KIND::END;
		}
	}

	// snapshot이 Server 행동을 wire enum으로 바꾼다. 모르는 행동을 정상 IDLE로 대체하지 않고 END를 반환한다.
	WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action)
	{
		switch (action)
		{
		case SERVER_ENTITY_ACTION::IDLE:
			return WORLD_ENTITY_ACTION::IDLE;
		case SERVER_ENTITY_ACTION::CHASE:
			return WORLD_ENTITY_ACTION::CHASE;
		case SERVER_ENTITY_ACTION::PATTERN_WINDUP:
			return WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		case SERVER_ENTITY_ACTION::PATTERN_ACTIVE:
			return WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		case SERVER_ENTITY_ACTION::PATTERN_RECOVERY:
			return WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		case SERVER_ENTITY_ACTION::DEAD:
			return WORLD_ENTITY_ACTION::DEAD;
		default:
			return WORLD_ENTITY_ACTION::END;
		}
	}

	// 파괴 runtime의 네 상태를 wire 상태로 바꾸며 모르는 값은 END다.
	WORLD_DESTRUCTION_RUNTIME_STATE To_NetworkDestructionState(const WORLD_DESTRUCTION_STATE state)
	{
		switch (state)
		{
		case WORLD_DESTRUCTION_STATE::INTACT:
			return WORLD_DESTRUCTION_RUNTIME_STATE::INTACT;
		case WORLD_DESTRUCTION_STATE::BREAKING:
			return WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING;
		case WORLD_DESTRUCTION_STATE::FRACTURED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED;
		case WORLD_DESTRUCTION_STATE::DESPAWNED:
			return WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED;
		default:
			return WORLD_DESTRUCTION_RUNTIME_STATE::END;
		}
	}

	// 파괴 동기화가 group ID·상태·version·시작/commit tick을 wire 값으로 복사한다. 직렬화 검증이나 전송은 호출자 책임이다.
	WORLD_DESTRUCTION_STATE_WIRE To_NetworkDestructionState(const WORLD_DESTRUCTION_GROUP_STATE& state)
	{
		WORLD_DESTRUCTION_STATE_WIRE wire{};
		wire.strGroupId = state.strGroupId;
		wire.eState = To_NetworkDestructionState(state.eState);
		wire.iStateVersion = state.iStateVersion;
		wire.iStateStartTick = state.iStateStartTick;
		wire.iCommitTick = state.iCommitTick;
		return wire;
	}

	// 파괴 live-event가 epoch·event sequence·group/mutation/binding·source·tick에서 재현 가능한 32비트 seed를 만든다. 충돌 없는 event ID나 인증 값은 아니다.
	std::uint32_t Hash_DestructionEventIdentity(const std::uint32_t encounterEpoch, const std::uint64_t eventSequence,
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition, const WORLD_DESTRUCTION_BINDING_APPLICATION& application,
		const std::uint32_t serverTick)
	{
		std::uint32_t hash = 2166136261u;
		const auto appendByte = [&hash](const std::uint8_t value)
		{
			hash ^= value;
			hash *= 16777619u;
		};
		// 각 정수는 낮은 byte부터 8개를 섞고 문자열 뒤에는 0xff 구분 byte를 붙인다. 엔디안 메모리 표현을 직접 읽지 않는다.
		const auto appendU64 = [&appendByte](const std::uint64_t value)
		{
			for (std::uint32_t shift = 0u; shift < 64u; shift += 8u)
				appendByte(static_cast<std::uint8_t>(value >> shift));
		};
		const auto appendString = [&appendByte](const std::string& value)
		{
			for (const unsigned char character : value)
				appendByte(character);
			appendByte(0xffu);
		};

		appendU64(encounterEpoch);
		appendU64(eventSequence);
		appendString(transition.strGroupId);
		appendString(transition.strMutationId);
		appendString(application.strBindingId);
		appendU64(application.iPatternSequence);
		appendU64(application.iSourceNetEntityId);
		appendU64(serverTick);
		// wire에서 예약된 seed 0을 피한다. 서로 다른 입력의 hash 충돌을 방지하는 검사는 아니다.
		return 0u == hash ? 1u : hash;
	}

}
