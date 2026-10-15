#pragma once

#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CServerNavigation;
	class CServerCollisionSystem;
	struct SERVER_WORLD_ENTITY;

	// 행동 정의를 실행하는 현재 단계다. 서버 tick 대기, 경로 이동, 논리 action, 일회 순찰 완료를 구분한다.
	enum class SERVER_NPC_BEHAVIOR_PHASE : std::uint8_t
	{
		START_DELAY,
		IDLE_WAIT,
		MOVING,
		AMBIENT_ACTION,
		COMPLETE
	};

	// 개체마다 변하는 실행 상태만 보관한다. 저작 경로/action 정의는 world bootstrap에 남고 Room이 stable placement ID로 조회한다.
	struct SERVER_NPC_BEHAVIOR_STATE final
	{
		NPC_BEHAVIOR_MODE eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		SERVER_NPC_BEHAVIOR_PHASE ePhase = SERVER_NPC_BEHAVIOR_PHASE::START_DELAY;
		std::size_t iWaypointIndex = 0u;
		std::size_t iActionIndex = 0u;
		// 대기 종료는 30Hz 절대 tick, pending 대기는 ms다. iRandomState는 개체마다 진행하므로 정의의 seed와 분리한다.
		std::uint32_t iWaitUntilTick = 0u;
		std::uint32_t iRandomState = 1u;
		std::uint32_t iPendingWaitMs = 0u;
		std::uint64_t iNavigationRevision = 0u;
		bool bRouteForward = true;
		bool bRouteComplete = false;
		bool bInitialized = false;
	};

	// GameRoom이 소유한 NPC entity의 행동 진행을 담당한다. 이 클래스 자체에는 실행 상태를 보관하지 않는다.
	// placement/navigation/collision과 lookTarget은 호출 동안 차용하고, 경로·진행 상태는 entity가 소유한다.
	class CNpcBehaviorRuntime final
	{
	public:
		static constexpr const char* IDLE_ACTION_ID = "npc.idle";
		static constexpr const char* WALK_ACTION_ID = "npc.move.walk";

		// GameRoom 초기화가 bootstrap 전체의 시선 참조와 활성 NPC의 이동 경로를 확인한다.
		// 정의·entity를 변경하지 않으며 결과 이유를 outStatus에 쓴다. JSON/행 필드의 모든 검증을 대신하지 않는다.
		bool Validate_Admission(const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
		                        const CServerNavigation& navigation,
		                        std::string& outStatus) const;

		// Build_WorldEntity가 준비 중인 NPC에 호출한다. 이동 NPC는 navigation에 투영한 위치를 spawn 기준으로 삼는다.
		// 명시적 false는 entity를 보존한다. 성공하면 기존 경로/행동 상태를 초기화하며 문자열 할당 예외까지 rollback하지는 않는다.
		bool Initialize(const WORLD_BOOTSTRAP_PLACEMENT& placement,
		                const CServerNavigation& navigation,
		                std::uint32_t startTick,
		                SERVER_WORLD_ENTITY& entity,
		                std::string& outStatus) const;

		// GameRoom의 월드 갱신이 호출한다. 위치는 m, fixedDeltaSeconds는 초, updateTick은 30Hz 서버 tick이다.
		// true에는 이동·대기·완료와 충돌 후 재시도 예약이 포함된다. false 전에도 방향·경로 진행·난수 상태가 바뀔 수 있다.
		// Room은 false를 runtime 실패로 처리하고 성공 후 blocking body 위치를 동기화한다.
		bool Update(const WORLD_BOOTSTRAP_PLACEMENT& placement,
		            const SERVER_WORLD_ENTITY* lookTarget,
		            const CServerNavigation& navigation,
		            const CServerCollisionSystem& collision,
		            float fixedDeltaSeconds,
		            std::uint32_t updateTick,
		            SERVER_WORLD_ENTITY& entity,
		            std::string& outStatus) const;

	private:
		// 행동 소유자/예약 action ID와 이동 가능성을 검사한다. STATIONARY는 이동 검사를 건너뛰므로 bootstrap 검증을 전제로 사용한다.
		static bool Validate_Descriptor(const WORLD_BOOTSTRAP_PLACEMENT& placement,
		                                const CServerNavigation& navigation,
		                                std::string& outStatus);
	};
}
