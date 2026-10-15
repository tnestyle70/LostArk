#pragma once

// 룸·player·boss·저작 정의와 내부 helper 인자 형식을 공유한다. simulation 상태 변경 helper는 룸 실행 스레드의 직렬 호출을 전제로 한다. 큐 helper는 호출자가 command mutex를 잡아 ingress 접근을 직렬화한다.
#include "GameRoom.h"
// 이 헤더의 inline 본문은 session API를 직접 호출하지 않는다. 기존 include 구성을 보존한다.
#include "ClientSession.h"
// set·cmath·cstdlib는 이 헤더의 inline 본문에서 직접 사용하지 않는 기존 include다.
#include <set>
#include <cmath>
#include <cstdlib>
// 호출 중만 차용하는 placement ID와 pattern-flow 입력을 복사 없이 검사한다.
#include <string_view>

namespace GameRoomDetail
{

	using namespace LostArk::Shared;

	using namespace LostArk::Server;

	// 콜로세움 복귀/입장 요청이 허용하는 Bern SquareHole 세 stable ID만 숫자로 바꾼다. 다른 문자열은 0이며 출발지나 이동 권한 검증은 호출자 책임이다.
	inline std::uint16_t Resolve_BernSquareHoleId(const std::string_view placementId)
	{
		if (placementId == "squarehole.1")
			return 1u;
		if (placementId == "squarehole.2")
			return 2u;
		if (placementId == "squarehole.3")
			return 3u;
		return 0u;
	}

	// 제품 경로는 VALTAN_FOUR_PILLARS_105의 RECOVERY에 들어갈 때 기둥을 생성한다.
	// 추가 붕괴 cycle은 현재 Debug audition이 별도로 예약하며, 아래 dwell을 제품 패턴 시간표로 사용하지 않는다.
	constexpr const char* PILLAR_PATTERN_ID = "VALTAN_FOUR_PILLARS_105";

	constexpr const char* PILLAR_SPAWN_STAGE_ID = "RECOVERY";

	// 기둥 BREAKING 뒤 제거 대기다. 30Hz에서 250ms를 올림한 8tick이며 실제 tick 간격은 약 266.7ms다.
	constexpr std::uint32_t PILLAR_BREAKING_TICKS = 8u;

	// Debug 기둥 cycle이 생성 성공 뒤 붕괴를 예약할 때 더하는 475tick(30Hz에서 약 15.83초)이다. 제품 생성 경로는 이 상수로 자동 붕괴를 예약하지 않는다.
	constexpr std::uint32_t PILLAR_AUDITION_DWELL_TICKS = 475u;

	// 쿠크 묶음 준비/시작이 boss motion의 navigation 진입 조건을 읽는다. 키가 있으면 시작점·높이만, 없으면 양 끝점과 직선 연결을 검사한다.
	bool Is_KoukuBossMotionNavigable(const BOSS_PATTERN_DEFINITION& pattern, const CServerNavigation& navigation);

	// 이동 요청 world X/Z의 절댓값 상한(m)이다. 실제 finite·navigation·충돌 검증은 이동 명령 소비자가 수행한다.
	constexpr float MAX_ABS_MOVE_GOAL = 10000.f;

	// 일반 이동이 경로점 도착으로 보는 수평 거리(m)다.
	constexpr float MOVE_STOP_DISTANCE = 0.05f;

	// atan2 결과와 player yaw 사이의 라디안↔도 변환 상수다.
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;

	constexpr float DEGREES_TO_RADIANS = 0.01745329251994329577f;

	// 일반 보행 yaw가 한 초 동안 회전할 수 있는 최대 각도다. 이동 simulation이 dt와 곱해 회전량을 제한한다.
	constexpr float PLAYER_TURN_DEGREES_PER_SECOND = 540.f;

	// 탑승하면 player와 탈것 몸체를 함께 돌리므로 보행과 다른 초당 yaw 회전 한도를 사용한다.
	constexpr float VEHICLE_TURN_DEGREES_PER_SECOND = 300.f;

	// 최종 목적지가 이 거리(m) 안이면 이동 방향 판단에서 직접 목표 방위를 사용하는 기준이다.
	constexpr float DIRECT_BEARING_DISTANCE = 1.5f;

	// 이미 이동 경로를 가진 player의 새 요청 좌표가 직전 요청에서 1m 이내면 경로를 유지한다.
	// 투영된 목적지가 아니라 원래 요청을 비교해 같은 장애물 주변의 반복 입력으로 경로가 다시 시작되는 일을 줄인다.
	constexpr float MOVE_ROUTE_KEEP_DISTANCE = 1.0f;

	// 몸체 sweep이 막혔을 때 경로를 다시 찾는 최소 간격이다. 30Hz의 6tick=200ms 사이에는 기존 이동을 유지한다.
	constexpr std::uint32_t MOVE_REROUTE_MIN_TICKS = 6u;

	// volley 원점 사이의 거리 제곱 비교에 더하는 허용값(m²)이다. 실제 거리 epsilon으로 다시 제곱하지 않는다.
	constexpr float VOLLEY_SPACING_EPSILON = 0.0001f;

	// 룸 고정 simulation은 초당 30tick을 사용한다. 아래 ms/tick 변환과 낙하·상호작용 시간의 공통 단위다.
	constexpr std::uint32_t SERVER_TICK_HZ = 30u;

	// arena-random 원점 생성이 navigation·최소 간격을 만족하는 후보를 찾는 총 시도 상한이다.
	constexpr std::uint32_t ARENA_RANDOM_MAXIMUM_ATTEMPTS = 512u;

	// 결정적 단위 난수를 world XZ 각도로 바꿀 때 쓰는 한 회전의 라디안 값이다.
	constexpr float TWO_PI = 6.28318530717958647692f;

	// 공유 Esther 시전 ms를 30Hz tick으로 올림해 player 행동 잠금에 사용한다.
	constexpr std::uint32_t ESTHER_CAST_TICKS = (ESTHER_CAST_DURATION_MS * 30u + 999u) / 1000u;

	// SquareHole 노래 ms를 30Hz tick으로 올림한다. 노래가 끝난 뒤 위치 이동을 수행하는 시점을 정한다.
	constexpr std::uint32_t SQUAREHOLE_SONG_TICKS = (LostArk::Shared::SQUAREHOLE_SONG_DURATION_MS * 30u + 999u) / 1000u;
	// SquareHole 전체 행동 잠금은 노래 tick과 blackout 유지 tick의 합이다. 화면 연출 자체는 Client가 같은 공유 시간 계약을 소비한다.
	constexpr std::uint32_t SQUAREHOLE_LOCK_TICKS =
		SQUAREHOLE_SONG_TICKS + (LostArk::Shared::SQUAREHOLE_BLACKOUT_HOLD_MS * 30u + 999u) / 1000u;

	// 쿠크 상호작용 한 번의 행동 ms를 30Hz tick으로 올림한다.
	constexpr std::uint32_t KOUKU_INTERACTION_TICKS =
		(LostArk::Shared::KOUKU_INTERACTION_ACTION_MS * 30u + 999u) / 1000u;

	// 이동/스킬 입력이 미로 망치 타격 시각 이후의 recovery를 취소한다. 실제로 action을 NONE으로 바꿨을 때만 true다.
	// 같은 INTERACTION을 쓰는 탈출 이동이 진행 중이면 취소하지 않으며, 실패는 player 상태를 유지한다.
	bool Cancel_MazeHammerRecovery(LostArk::Server::SERVER_PLAYER& player, const std::uint32_t tick);

	// 레이드 완료 조건을 검사하는 테스트 모드 환경 변수명이다. helper는 문자열이 정확히 1일 때만 활성으로 판단한다.
	constexpr const char* RAID_CLEAR_TEST_MODE_ENV = "LOSTARK_RAID_CLEAR_TEST_MODE";

	// Mario 구간은 게시 placement ID 두 개와 화면 오른쪽 부호로 연결한다. 실제 world 위치는 룸 bootstrap에서 찾으며 Client 카메라 값을 Server 입력으로 사용하지 않는다.
	struct MARIO_LANE_BINDING
	{
		// 해당 구간을 쓰는 Mario 단계 번호다.
		std::uint8_t stage;
		// 도착/출구 stable placement ID는 아래 정적 문자열을 차용한다.
		const char* arrival;
		const char* exit;
		// rail 설정 당시 player 위치→출구 축에 곱해 화면 오른쪽을 만드는 +1/-1 부호다.
		float rightSign;
	};

	// Mario trigger 입장·rail 초기화·최종 출구 판정이 공유한다. ID 철자(Tigger 포함)는 게시 저장 계약이므로 그대로 사용한다.
	constexpr std::array<MARIO_LANE_BINDING, 18u> MARIO_LANES{{
		{1u, "Mario1_go", "Mario1_Trigger_1", 1.f},
		{1u, "Mario1_Trigger_1", "Mario1_Trigger_3", 1.f},
		{1u, "Mario1_Trigger_3", "Mario1_Trigger_5", 1.f},
		{2u, "Mario2_go", "Mario2_Trigger_2", 1.f},
		{2u, "Mario2_Trigger_2", "Mario2_Trigger_4", -1.f},
		{2u, "Mario2_Trigger_4", "Mario2_Trigger_7", 1.f},
		{3u, "Mario3_go", "Mario3_Trigger_4", 1.f},
		{3u, "Mario3_Trigger_4", "Mario3_Trigger_5", -1.f},
		{3u, "Mario3_Trigger_5", "Mario3_Trigger_6", 1.f},
		{3u, "Mario3_Trigger_6", "Mario3_Trigger_8", 1.f},
		{3u, "Mario3_Trigger_8", "Mario3_Trigger_10", -1.f},
		{3u, "Mario3_Trigger_10", "Mario3_Trigger_12", 1.f},
		{4u, "Mario4_go", "Mario4_Tigger_2", 1.f},
		// 이 구간은 rail 설정 당시 player 위치→출구 방향을 뒤집어 화면 오른쪽 축으로 사용한다. runtime은 카메라를 읽지 않고 이 -1 부호를 rail 계산에 적용한다.
		{4u, "Mario4_Tigger_2", "Mario4_Tigger_5", -1.f},
		{4u, "Mario4_Tigger_3", "Mario4_Tigger_6", 1.f},
		{4u, "Mario4_Tigger_6", "Mario4_Tigger_7", -1.f},
		{4u, "Mario4_Tigger_7", "Mario4_Tigger_13", 1.f},
		{4u, "Mario4_Tigger_5", "Mario4_Tigger_7", 1.f}
	}};

	// Mario 이동/충돌 보정이 world XZ를 player의 rail 직선에 투영한다. rail이 없거나 축 길이 제곱이 0.5 미만이면 두 출력은 유지된다.
	void Project_MarioRailPoint(const SERVER_PLAYER& player, float& x, float& z);

	// 레이드 출구 검사가 process 환경 변수의 현재 값을 읽는다. 동적 할당한 환경 문자열은 함수 안에서 해제한다.
	bool Is_RaidClearTestModeEnabled();

	// 입장 진단의 실제 시각을 Unix ms로 기록한다. simulation deadline용 단조 clock은 아니다.
	std::uint64_t Current_UnixMilliseconds();

	// pattern flow 요청의 ID 길이와 허용 byte를 검사한다. isalnum은 현재 C locale의 분류를 따른다.
	bool Is_StablePatternFlowId(const std::string_view value);

	// pattern flow 저작 revision이 고정 길이 소문자 16진 문자열인지 검사한다.
	bool Is_PatternFlowRevision(const std::string_view value);

	// 중복 시작 요청을 대조할 문자열을 만든다. 요청 sequence를 포함하지 않으며 ID 검증·구분자 제외는 호출자에서 수행한다.
	std::string Build_PatternFlowStartRequestIdentity(const C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);

	// 각 hit shape의 원점 기준 최대 외곽 범위 두 배(m)를 volley 원점의 기본 최소 간격으로 반환한다.
	float Resolve_VolleyMinimumSpacing(const BOSS_COMBAT_OBJECT_DEFINITION& definition);

	// 룸 난수 선택용 64비트 입력을 순수 정수 연산으로 섞는다. 전역 난수 상태를 소비하지 않는다.
	std::uint64_t Mix_DeterministicRandom(std::uint64_t value);

	// stable ID의 원문 byte를 고정 순서로 누적해 결정적 seed를 만든다. 문자열 정규화나 고유성 보장은 없다.
	std::uint64_t Hash_StableId(const std::string& value);

	// 섞은 값의 상위 24bit를 2²⁴로 나누어 0 이상 1 미만의 float를 반환한다.
	float DeterministicUnitFloat(const std::uint64_t value);

	// 파괴 transaction을 collision/navigation 후보 배열로 변환한다. 두 출력을 먼저 비우며 실제 시스템 commit은 호출자가 맡는다.
	void Build_WorldDestructionStateChanges(const WORLD_DESTRUCTION_TRANSACTION& transaction,
		std::vector<SERVER_COLLISION_STATE_CHANGE>& collisionChanges,
		std::vector<SERVER_NAVIGATION_CONDITION_CHANGE>& navigationChanges);

	// 같은 group에 모이는 파괴 변경의 상태·version·member·충돌/nav 목표를 비교한다. binding application의 출처는 비교하지 않는다.
	bool Is_SameDestructionTransitionTarget(
		const WORLD_DESTRUCTION_STATE_TRANSITION& left, const WORLD_DESTRUCTION_STATE_TRANSITION& right);

	// Debug 지형 일괄 변경이 transaction의 application/transition 쌍을 group별로 합친다.
	// 출력 vector/map은 호출자가 지역 후보로 소유하며 중간 false에서도 앞서 추가한 항목은 남으므로 호출자가 후보 전체를 버린다.
	// 기존 map index가 vector의 같은 group을 가리킨다는 불변식과 PAIR의 Application/Transition 필드를 전제로 한다.
	template <typename PAIR>
	bool Append_UniqueDestructionTransitions(const WORLD_DESTRUCTION_TRANSACTION& transaction,
		const std::uint32_t expectedEncounterEpoch, const std::uint32_t expectedRequestTick,
		std::vector<PAIR>& stagedPairs, std::map<std::string, std::size_t>& stagedIndexByGroupId)
	{
		if (transaction.iEncounterEpoch != expectedEncounterEpoch || transaction.iRequestTick != expectedRequestTick ||
			transaction.BindingApplications.size() != transaction.Transitions.size() || transaction.Transitions.empty())
		{
			return false;
		}
		for (std::size_t index = 0u; index < transaction.Transitions.size(); ++index)
		{
			const WORLD_DESTRUCTION_STATE_TRANSITION& transition = transaction.Transitions[index];
			const auto existing = stagedIndexByGroupId.find(transition.strGroupId);
			if (stagedIndexByGroupId.end() != existing)
			{
				if (!Is_SameDestructionTransitionTarget(stagedPairs[existing->second].Transition, transition))
				{
					return false;
				}
				// 호출자는 접촉 binding을 먼저 모은다. 뒤의 stage binding이 같은 변경을 요청해도 첫 application/transition 한 쌍만 유지한다.
				continue;
			}
			stagedIndexByGroupId.emplace(transition.strGroupId, stagedPairs.size());
			stagedPairs.push_back({transaction.BindingApplications[index], transition});
		}
		return true;
	}

	// 룸 성능 계측이 steady_clock의 비음수 경과를 정수 μs로 내림한다. 음수 입력을 별도로 거절하지는 않는다.
	std::uint64_t To_Microseconds(const std::chrono::steady_clock::duration duration);

	// 룸 ingress에서 MOVE와 UPDATE_SKILL_AIM만 합치기/용량 제한 대상인 best-effort로 분류한다.
	bool Is_BestEffortCommand(const ROOM_COMMAND_TYPE type);

	// enqueue 진단이 정적 결과명 문자열을 차용한다. 알 수 없는 enum은 UNKNOWN이다.
	std::string_view To_RoomCommandEnqueueResultName(const ROOM_COMMAND_ENQUEUE_RESULT result);

	// 같은 session·명령 종류를 확인하고 aim이면 skill ID까지 같아야 같은 합치기 stream으로 본다.
	bool Is_SameBestEffortStream(const ROOM_COMMAND& queued, const ROOM_COMMAND& incoming);

	// CGameRoom::Enqueue_Detailed가 command mutex를 잡은 채 호출한다. 같은 session의 가장 뒤 명령이 같은 stream일 때 그 한 개만 지우며 새 명령 append는 호출자가 한다.
	bool Try_RemoveCoalescedCommand(std::deque<ROOM_COMMAND>& commands, const ROOM_COMMAND& incoming);

	// 이동 yaw 차이의 유한한 도 값을 -180~180 범위로 접는다. 경계 두 값은 그대로 두며 비유한 입력 검사는 호출자 책임이다.
	float Wrap_Degrees(float degrees);

	// 지형 붕괴로 시작한 낙하는 45tick(30Hz에서 1.5초) 뒤 사망 기한에 도달한다.
	// 9.8m/s² 중력은 그 사이의 위치 표시를 내려가게 하며 낙하 승인 자체를 다시 판정하지 않는다.
	constexpr float FALL_GRAVITY_METERS_PER_SECOND_SQUARED = 9.8f;

	constexpr std::uint32_t FALL_DEATH_TICKS = 45u;

	// 시작 tick이 1 이상인 순환 시계에 경과 tick을 더한다. 예약값 0을 건너뛰는 1~UINT32_MAX 모듈러 연산이며 startTick 0을 거절하는 검사는 없다.
	constexpr std::uint32_t Add_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick, const std::uint32_t elapsedTicks)
	{
		constexpr std::uint64_t SERVER_TICK_CARDINALITY =
			(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(startTick - 1u) + elapsedTicks) % SERVER_TICK_CARDINALITY) + 1u);
	}

	// 0이면 경과 0, 그 밖에는 예약 0을 건너뛰는 한 번의 wrap 차이를 반환한다. 역방향 여부나 여러 회전 경과는 구분하지 않는다.
	std::uint64_t Elapsed_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick, const std::uint32_t currentTick);

	// 예약 0을 건너뛰는 tick 덧셈의 일반/경계 사례를 컴파일 시 확인한다.
	static_assert(91u == Add_ServerTicksSkippingReservedZero(1u, 90u));

	static_assert(90u == Add_ServerTicksSkippingReservedZero((std::numeric_limits<std::uint32_t>::max)(), 90u));

	static_assert(1u == Add_ServerTicksSkippingReservedZero((std::numeric_limits<std::uint32_t>::max)() - 89u, 90u));

	// Server combat flag의 각 bit와 전체 알려진 mask가 Shared wire 계약과 같은지 아래 static_assert들로 확인한다.
	static_assert(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::INVULNERABLE));

	static_assert(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::SHIELDED) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::SHIELDED));

	static_assert(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::COUNTERABLE));

	static_assert(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GROGGY) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GROGGY));

	static_assert(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) ==
		static_cast<std::uint16_t>(BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN));

	static_assert(
		(static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) |
			static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::SHIELDED) |
			static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::COUNTERABLE) |
			static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GROGGY) |
			static_cast<std::uint32_t>(SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN)) == BOSS_COMBAT_STATE_KNOWN_FLAG_MASK);

	// 기한과 같거나 signed32 차이가 양수인지 판정한다. 기한 간격이 반주기 미만이라는 룸 예약 계약을 따르며 0을 특별히 거절하지 않는다.
	bool Has_ReachedServerTick(const std::uint32_t currentTick, const std::uint32_t targetTick);

	// 유령 재배치의 소유 표시가 켜진 hidden/invulnerable flag만 내리고 예약 tick·소유 표시를 비운다. 소유 표시가 없는 flag는 변경하지 않는다.
	void Clear_ValtanGhostRelocationState(SERVER_WORLD_ENTITY& boss);

	// ms를 64비트 중간 곱으로 30Hz tick에 올림한다. 0ms는 0tick이며 단위 변환만 한다.
	std::uint32_t DurationMillisecondsToServerTicks(const std::uint32_t durationMs);

	// 패턴 구속 적용이 진행 스킬·투사체·이동/낙하/피격 예약을 지우고 전투 준비를 끈다. HP·인벤토리나 룸 combat-object 저장소는 이 함수가 수정하지 않는다.
	void Cancel_PlayerActionForPatternStatus(SERVER_PLAYER& player);

	// Debug 발탄 audition이 world별로 찾는 stable placement ID다. 포인터/배열 index를 저장 identity로 사용하지 않는다.
	constexpr const char* VALTAN_ARENA_AUDITION_PLACEMENT_ID = "boss.valtan.center";

	constexpr const char* CHARACTER_SELECT_AUDITION_PLACEMENT_ID = "boss.valtan.character-select.lazy";

	// Character Select의 지형/전용 기믹 부재로 직접 재생을 제한하는 정확한 pattern ID 집합이다.
	constexpr std::array<const char*, 8u> CHARACTER_SELECT_ENVIRONMENT_DEPENDENT_PATTERNS{"VALTAN_ARMOR_BREAK_OPENING",
		"VALTAN_ENTRANCE_WHIRLWIND", "VALTAN_ARENA_BREAK_109", "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
		"VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK", "VALTAN_FOUR_PILLARS_105", "VALTAN_SIX_PIZZA_106",
		"VALTAN_GHOST_PORTAL_ONCE"};

	// Character Select audition이 필요한 지형/기믹이 없는 패턴을 정적 stable ID 목록으로 거른다.
	bool Is_CharacterSelectEnvironmentDependentPattern(const std::string& patternId);

	// root sequence와 pinned revision의 후속 패턴이 예약/실행 중인지 읽는다. pending이 존재하면 그 identity 결과를 바로 반환한다.
	bool Is_ValtanOutcomeFollowupInFlight(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence, const LostArk::Shared::GameplayDataRevision& definitionRevision);

	// 후속 예약과 실행 패턴이 없고 같은 root의 COMPLETED receipt·revision이 남았는지 확인한다. IDLE만으로 성공 완료를 추정하지 않는다.
	bool Has_ValtanOutcomeGroupCompleted(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t rootPatternSequence, const LostArk::Shared::GameplayDataRevision& definitionRevision);

	// 벽 파괴 audition과 최종 지형 재현이 게시 pattern/stage/action의 stable ID를 조합한다.
	constexpr const char* WALL_ATTACK_PATTERN_ID = "VALTAN_DOWN_SMASH";

	constexpr const char* FINAL_ARENA_PATTERN_ID = "VALTAN_ARENA_BREAK_109";

	constexpr const char* FINAL_ARENA_STAGE_ID = "IMPACT";

	constexpr const char* FINAL_ARENA_ACTION_ID = "valtan.mechanic.arena-break-109.impact";

	// Debug 최종 지형 재현은 외곽 벽 파괴와 함께 두 바닥 붕괴도 stage한다.
	// 3시·9시 파괴의 게시 pattern/stage/action ID와 stage index를 아래처럼 고정해 transaction 준비에 사용한다.
	constexpr const char* FINAL_ARENA_FLOOR_A_PATTERN_ID = "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK";

	constexpr const char* FINAL_ARENA_FLOOR_A_STAGE_ID = "IMPACT";

	constexpr const char* FINAL_ARENA_FLOOR_A_ACTION_ID = "valtan.mechanic.terrain-destruction-3.impact";

	constexpr std::uint32_t FINAL_ARENA_FLOOR_A_STAGE_INDEX = 3u;

	constexpr const char* FINAL_ARENA_FLOOR_B_PATTERN_ID = "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK";

	constexpr const char* FINAL_ARENA_FLOOR_B_STAGE_ID = "IMPACT";

	constexpr const char* FINAL_ARENA_FLOOR_B_ACTION_ID = "valtan.mechanic.terrain-destruction-9.impact";

	constexpr std::uint32_t FINAL_ARENA_FLOOR_B_STAGE_INDEX = 3u;

	// Debug 지형 선택 wire 값을 runtime 상태와 정적 label로 바꾼다. 모르는 값은 false이며 두 출력은 보존한다.
	bool Resolve_ValtanArenaPreset(
		const std::uint32_t rawPreset, LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& outState, const char*& outLabel);

	// Debug 전투 페이지가 stable timeline row에서 시작할 때 초기 phase와 입장 연출 여부를 고른다. 문자열은 아래 정적 배열이 소유한다.
	struct VALTAN_FIGHT_PAGE_POLICY final
	{
		const char* pTimelineRowId;
		std::uint8_t iInitialGameplayPhase;
		bool bPlayEntrance;
	};

	// 각 key는 저작 timeline row ID다. Start_ValtanFightPage는 선택 경계보다 앞선 기믹을 완료 ledger로 넣고 경계 기믹부터 정상 진행하게 한다.
	constexpr std::array<VALTAN_FIGHT_PAGE_POLICY, 4u> VALTAN_FIGHT_PAGE_POLICIES{
		VALTAN_FIGHT_PAGE_POLICY{"valtan.timeline.160-entrance-whirlwind", 1u, true},
		VALTAN_FIGHT_PAGE_POLICY{"valtan.timeline.109-arena-break", 1u, false},
		VALTAN_FIGHT_PAGE_POLICY{"valtan.timeline.62-center-grab-counter", 2u, false},
		VALTAN_FIGHT_PAGE_POLICY{"valtan.timeline.14-ghost-transition", 2u, false}};

	// Debug 시작 페이지의 stable timeline row ID를 정적 정책 배열에서 찾는다. 포인터 수명은 프로그램 수명이며 누락은 nullptr다.
	const VALTAN_FIGHT_PAGE_POLICY* Find_ValtanFightPagePolicy(const std::string& rowId);

	// Debug 벽 공격 audition이 사용하는 world 중심(m)과 보스/플레이어 Z offset이다.
	// 보스는 중심에서 -6m, player 후보는 +8m에 두어 DOWN_SMASH 타격을 준비하고 실제 착지는 호출자가 navigation으로 확인한다.
	constexpr float WALL_ATTACK_CENTER_X = 161.402061f;

	constexpr float WALL_ATTACK_CENTER_Y = 23.04f;

	constexpr float WALL_ATTACK_CENTER_Z = -133.312236f;

	constexpr float WALL_ATTACK_BOSS_OFFSET_Z = -6.f;

	constexpr float WALL_ATTACK_PLAYER_OFFSET_Z = 8.f;

	// timeline audition이 현재 HP에 따라 DEAD/NONE을 정하고 skill·이동·부착·구속·침묵을 정리해 전투 준비를 끈다. 회복은 수행하지 않는다.
	void Freeze_TimelineAuditionPlayer(SERVER_PLAYER& player);

	// audition 시작/다음 단계가 Freeze 뒤 HP·자원·identity를 채우고 광기/상호작용을 초기화해 지정 tick으로 준비한다. 입력 tick의 유효성은 호출자 책임이다.
	void Prepare_TimelineAuditionPlayer(SERVER_PLAYER& player, const std::uint32_t actionTick);

	// 룸 입장이 protocol·world·playable class·닉네임·음성·외형 형식만 검사한다. 정원·session binding·실제 spawn 승인은 별도 입장 처리다.
	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message);

	// 명령 sequence 0을 거부하고 signed32 차이로 최신 여부를 비교한다. 반주기 이상 떨어진 요청의 순서를 확정하는 함수는 아니다.
	bool Is_NewerSequence(const std::uint32_t candidate, const std::uint32_t previous);

	// snapshot이 world bootstrap 종류를 wire enum으로 바꾼다. 지원하지 않는 값은 END로 표시한다.
	WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind);

	// snapshot이 Server 행동을 wire enum으로 바꾼다. 모르는 행동을 정상 IDLE로 대체하지 않고 END를 반환한다.
	WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action);

	// 파괴 runtime의 네 상태를 wire 상태로 바꾸며 모르는 값은 END다.
	WORLD_DESTRUCTION_RUNTIME_STATE To_NetworkDestructionState(const WORLD_DESTRUCTION_STATE state);

	// 파괴 동기화가 group ID·상태·version·시작/commit tick을 wire 값으로 복사한다. 직렬화 검증이나 전송은 호출자 책임이다.
	WORLD_DESTRUCTION_STATE_WIRE To_NetworkDestructionState(const WORLD_DESTRUCTION_GROUP_STATE& state);

	// 파괴 live-event가 epoch·event sequence·group/mutation/binding·source·tick에서 재현 가능한 32비트 seed를 만든다. 충돌 없는 event ID나 인증 값은 아니다.
	std::uint32_t Hash_DestructionEventIdentity(const std::uint32_t encounterEpoch, const std::uint64_t eventSequence,
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition, const WORLD_DESTRUCTION_BINDING_APPLICATION& application,
		const std::uint32_t serverTick);

}
