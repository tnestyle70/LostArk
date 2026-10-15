#pragma once

#include "ServerWorldEntity.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace LostArk::Server
{
	//Brain에 왜 Navigation이랑 CollisionSystem이 존재하는 걸까? 
	// 두 시스템은 Brain의 멤버가 아니다. Apply_StageRootMotion 호출 동안 Room이 소유한 객체를 빌려 이동 가능성과 몸체 충돌을 확인한다.
	class CServerNavigation;
	class CServerCollisionSystem;
	inline constexpr std::string_view KOUKUSAYDON_G1_ENCOUNTER_ID = "ENCOUNTER_KOUKUSAYDON_G1";
	inline constexpr std::string_view KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID = "BOSS_KOUKUSAYDON_G1_KOUKU";
	inline constexpr std::string_view KOUKUSAYDON_G1_BOSS_PLACEMENT_ID = "boss.koukusaydon.g1.kouku";
	inline constexpr std::string_view KOUKUSAYDON_G1_PLAY_ALL_SEQUENCE_ID = "KOUKUSAYDON_G1_PLAY_ALL";
	// 쿠크 아레나의 보스 archetype을 식별하는 공통 접두사다.
	// 관문 활성화는 disabled placement를 찾고, 이 보스들의 profile/pattern 조회에는 G1 encounter ID를 사용한다.
	inline constexpr std::string_view KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX = "BOSS_KOUKUSAYDON_";

	// Update의 stage 진행 결과다. GameRoom은 단계 변경 때 ENTER action을 처리하고, 종료/중단 때 실행 소유 상태를 정리한다.
	enum class KOUKUSAYDON_BRAIN_UPDATE_RESULT : std::uint8_t
	{
		IDLE,
		RUNNING,
		STAGE_CHANGED,
		PATTERN_COMPLETED,
		ABORTED_INVALID_DEFINITION,
		ABORTED_BOSS_DEAD
	};

	// GameRoom이 전달한 보스 상태에 쿠크의 stage 시계와 저작 이동을 적용한다.
	// 패턴 선택 승인, Logic 판정, 플레이어/월드 후처리는 GameRoom과 LogicRuntime이 별도로 담당한다.
	class CKoukuSaydonBrain final
	{
	public:
		//전부 다 static 함수로 정의가 되어있다. why? 왜 전부 다 static fuction으로 정의가 되어있는 거지?
		// 이 클래스는 멤버 상태가 없고, static 함수들은 전달받은 정의나 boss만 사용한다.
		// Begin_Pattern/Update/Abort_Pattern/Complete_Pattern은 static이 아닌 const 함수이며 전달된 boss를 갱신한다.
		// Is_GateOneBoss는 PartyWorld의 G1 주 보스 조회처럼 encounter/archetype/placement를 모두 고정해야 할 때 사용한다.
		[[nodiscard]] static bool Is_GateOneBoss(
			LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept;
		// GameRoom의 쿠크 보스 조회/진행에서 주 보스 여부를 분류한다. G1 전용 placement ID로 제한하지 않는다.
		// world, owner, kind, encounter, archetype 접두사만 확인하며 생존 여부나 실행 승인까지 검사하지 않는다.
		[[nodiscard]] static bool Is_ArenaBoss(
			LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept;
		// GateProgress가 활성화할 disabled BOSS placement인지 검사한다.
		// 이미 enabled인 placement는 false이며, 이 함수가 entity를 생성하거나 Catalog를 검색하지 않는다.
		//아레나 보스가 정말 존재하는지? 서버에 플레이 시킬 수 있는 보스가 존재하는지를 물어보는 API?
		// 여기서는 전달된 placement의 필드를 분류할 뿐이다. 실제 보스 존재, 생성 성공, 패턴 실행 가능 여부는 이후 GameRoom 검증으로 결정한다.
		[[nodiscard]] static bool Is_ArenaBossPlacement(
			LostArk::Shared::WORLD_ID worldId, const WORLD_BOOTSTRAP_PLACEMENT& placement) noexcept;

		// Catalog 로드와 GameRoom 실행 승인에서 쿠크가 지원하는 정의의 조합/범위를 검사한다.
		// 이름과 달리 LogicWindow와 mechanic 공격도 허용한다. stage 자체의 직접 타격 필드는 제한한다.
		// 입력 정의를 바꾸지 않으며 참조 ID 존재/실제 navigation 등 모든 외부 계약을 이 함수 하나로 보장하지 않는다.
		[[nodiscard]] static bool Validate_AnimationOnlyPattern(
			const BOSS_PATTERN_DEFINITION& pattern, std::string& status);
		//무언가 소환을 해야 하는 패턴이 있는지를 물어보는 API?
		// 소환 여부를 검색하는 함수가 아니라, 이미 선택된 owner/child 정의가 함께 실행 가능한 조합인지 검증한다.
		// 기본 child 검증 뒤 몸체/관문/대상 일치와 허용 Logic/trigger를 제한한다. 실제 소환은 하지 않는다.
		[[nodiscard]] static bool Validate_SummonedPattern(
			const BOSS_PATTERN_DEFINITION& owner,
			const BOSS_PATTERN_DEFINITION& child,
			std::string& status,
			bool allowActorLocalAirborne = false);

		// BossSimulation의 방향 분신 준비와 Catalog 검증에서 네 자식을 검사하고 실체 방향을 고른다.
		// 지정한 clone end stage까지 root 변위를 합산한 XZ가 boss spawn XZ와 가장 가까운 후보를 쓴다. 명시 real pattern ID가 있으면 그것이 우선한다.
		// selected/cloneDurationsMs는 모든 검사 뒤 함께 대입하며 false이면 기존 값이 유지된다.
		[[nodiscard]] static bool Select_CrossDirection(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& parent,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			const CGameplayCatalog& catalog,
			std::size_t& selected,
			std::array<std::uint32_t, 4u>& cloneDurationsMs,
			std::string& status);

		// Catalog가 소유한 G1 pattern vector에서 ID를 찾고 현재 지원 계약을 검증한다.
		// 반환 포인터를 소유하거나 삭제하지 않는다. 호출자는 해당 Catalog 세대를 유효하게 유지해야 한다.
		[[nodiscard]] static const BOSS_PATTERN_DEFINITION* Find_AnimationOnlyPattern(
			const CGameplayCatalog& catalog, std::string_view patternId, std::string& status);
		// GameRoom의 Play All 승인에서 몸체/관문/대상 placement에 맞는 패턴을 저작 순서대로 고른다.
		// 남긴 패턴 뒤의 transition은 그 패턴의 원래 값이며, 건너뛴 패턴의 transition을 합산하지 않는다.
		// 명시적 false 반환은 두 출력 vector를 보존하고 status에 원인을 남긴다.
		[[nodiscard]] static bool Select_AnimationOnlySequence(
			const std::vector<BOSS_PATTERN_DEFINITION>& definitions,
			const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
			std::string_view bossArchetypeId,
			std::vector<std::string>& outPatternIds,
			std::vector<std::uint32_t>& outTransitionTicks,
			std::string& status,
			std::string_view gateId = {},
			std::string_view targetBossPlacementId = {});
		// GameRoom/GateProgress가 게시 원본 revision을 비교할 때 사용한다. 현재 Catalog에 저장된 G1 revision을 조회한다.
		[[nodiscard]] static std::uint32_t Resolve_ProductSourceRevision(
			const CGameplayCatalog& catalog) noexcept;
		// GameRoom은 승인/예약 중 boss 복사본에도 이 함수를 호출한다. false이면 boss를 변경하지 않는다.
		// 성공하면 revision/실행 sequence/첫 stage와 Brain 내부 진행 기록을 초기화한다.
		// 몸체별 admission, spawn reset, ENTER action, 플레이어 Logic 기록의 준비와 실제 commit은 GameRoom 책임이다.
		[[nodiscard]] bool Begin_Pattern(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			const LostArk::Shared::GameplayDataRevision& revision,
			std::uint32_t serverTick,
			std::string& status) const;
		// GameRoom이 이동과 Logic 처리 뒤 stage 시계를 진행시킨다. invalid definition/dead는 종료 기록을 남기고 boss 진행 상태를 정리한다.
		// Catalog 세대 선택과 revision 고정은 호출자가 담당한다. 이 함수가 전달된 Catalog와 pinned revision을 다시 비교하지 않는다.
		// 보충: 위 이동/Logic 순서는 주 보스의 일반 tick 기준이다. 방향 child는 별도 복사본에서 이동/접촉 판정 뒤 Update를 호출한다.
		[[nodiscard]] KOUKUSAYDON_BRAIN_UPDATE_RESULT Update(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick,
			std::string& status) const;
		// BossMotion의 절대 월드 위치를 30Hz 경과 tick에서 표본화한다. 같은 입력 시각은 같은 좌표를 낸다.
		static std::array<float, 3u> Sample_BossMotion(
			const BOSS_PATTERN_BOSS_MOTION& motion, std::uint32_t elapsedTicks) noexcept;
		// GameRoom이 Logic 전에 적용하고 Update도 같은 시각을 적용한다. 절대 샘플 대입이므로 이동량을 중복 누적하지 않는다.
		// 이 경로는 navigation/collision을 직접 호출하지 않는다. GameRoom 승인은 key가 있으면 시작 지면만, 없으면 시작/끝과 XZ 경로를 확인한다.
		static void Apply_BossMotion(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			std::uint32_t serverTick) noexcept;
		// 검증된 시간 증가 순서의 root sample을 선형 보간한다. 빈 배열/비정상 시간은 기본 샘플, 범위 밖은 끝점을 반환한다.
		[[nodiscard]] static ROOT_MOTION_SAMPLE Sample_StageRootMotion(
			const std::vector<ROOT_MOTION_SAMPLE>& samples, double timeMs) noexcept;
		// pattern 시작부터 지난 30Hz tick을 ms로 바꾼다. tick wrap에서 예약값 0은 건너뛰며, 유효한 실행 시작 tick은 호출자가 제공한다.
		[[nodiscard]] static double Pattern_ElapsedMs(
			const SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick) noexcept;
		// 고정 시계는 pattern 경과 ms에서 앞 stage 길이 합을 빼고, 일반 시계는 현재 stage의 첫 평가 tick을 포함해 계산한다.
		[[nodiscard]] static double Stage_RootTimeMs(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			std::uint32_t serverTick) noexcept;
		// 현재 공중 phase의 지면 위 높이 m를 계산한다. SLAM에서는 state의 착지 진행도도 증가시킨다.
		// false라도 출력/진행도 보존이 항상 보장되지는 않는다. 이동 적용 경로는 지역 복사본으로 계산한 뒤 반영한다.
		[[nodiscard]] static bool Sample_AlbionAirborneHeight(
			SERVER_ALBION_AIRBORNE_STATE& state,
			double patternMs,
			std::uint32_t stageIndex,
			float sourceUp,
			float& outHeight) noexcept;
		// GameRoom이 Logic 판정 전에 호출하며 Room 소유 navigation/collision을 const 참조로 차용한다.
		// 좌표는 검증 끝에서 함께 반영하지만, false 전에 원점 capture 상태가 바뀔 수 있다.
		// true는 이동 성공뿐 아니라 이미 처리한 tick, 이동 없음, 경계에서 위치 유지도 포함한다.
		// 보충: Logic 전 호출은 주 보스의 일반 tick 기준이다. 방향 child는 접촉 판정 전에, SLAM 전환은 staged 상태 검증에서도 호출한다.
		[[nodiscard]] static bool Apply_StageRootMotion(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			std::uint32_t serverTick,
			const CServerNavigation& navigation,
			const CServerCollisionSystem& collision,
			std::string& status);
		// GameRoom의 취소/정리에서 ABORTED 기록과 Brain 상태 정리를 수행한다. 외부 player/소환/월드 상태를 직접 정리하지 않는다.
		void Abort_Pattern(SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick) const;
		// GameRoom이 Logic 결과나 실행 연쇄 종료를 처리할 때 남은 stage를 건너뛰고 COMPLETED 기록을 만든다.
		// 실행 중인 pattern ID가 없으면 아무 상태도 바꾸지 않는다. 플레이어/월드 정리는 호출자 책임이다.
		void Complete_Pattern(SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick) const;

	private:
		// 현재 stage ID/action/기간과 root sample을 복사하고 이동 원점 capture를 해제한다. 첫 진입과 다음 stage의 평가 시작 tick을 구분한다.
		static void Enter_Stage(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_DEFINITION& stage,
			std::uint32_t stageIndex,
			std::uint32_t serverTick,
			bool evaluatesOnEntryTick);
		// 실행 중이면 terminal receipt를 쓰고 stage/이동/접촉 진행을 비운다. HP가 0이면 DEAD, 아니면 IDLE로 끝낸다.
		// 고정 revision과 실행 sequence 값은 이 정리에서 지우지 않는다.
		static void Finish_Pattern(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT result);
	};
}
