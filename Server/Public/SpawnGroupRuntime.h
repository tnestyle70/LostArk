#pragma once

// 스케줄이 차용할 그룹/웨이브 정의와 spawn callback에 넘길 anchor·monster profile을 사용한다.
#include "SpawnGroupBootstrap.h"

// 성공한 생성 수와 경과 ms를 고정 폭 정수로 보관한다.
#include <cstdint>
// 룸의 현재 개체 수 조회와 실제 몬스터 생성 함수를 callback으로 받는다.
#include <functional>
// stable spawnGroup ID 조회와 초기화 진단을 전달한다.
#include <string>
// 그룹별 실행 상태와 현재 웨이브 entry별 생성 완료 횟수를 소유한다.
#include <vector>

namespace LostArk::Server
{
	// GameRoom이 소유하는 웨이브 스케줄러다. 몬스터 객체를 소유하지 않고 룸이 제공한 개체 수/생성 callback으로 진행한다.
	// Initialize가 받은 bootstrap의 정의 주소를 차용하므로 bootstrap의 그룹 저장소가 더 오래 살아야 하며 임의 재로드로 주소를 바꾸면 안 된다.
	// 룸 worker가 직렬 호출하며 자체 잠금은 없다. callback 안에서 이 runtime/정의 목록을 다시 초기화하지 않는다.
	class CSpawnGroupRuntime final
	{
	  public:
		// group ID의 잔류 개체 수다. 실제 Count_SpawnGroupEntities는 HP를 검사하지 않아 사망 후 제거 대기 중인 MONSTER도 센다.
		using ACTIVE_COUNT_QUERY = std::function<std::uint32_t(const std::string&)>;
		// group/entry/anchor/profile과 entry 안의 0부터 시작하는 생성 ordinal을 받는다. 참조는 호출 중 차용하며 callback을 저장하지 않는다.
		// true일 때만 스케줄의 생성 수를 올린다. 실제 룸은 Spawn_Monster의 개체 삽입 후 true를 반환하며 여기서 송신 성공을 보증하지 않는다.
		using SPAWN_CALLBACK =
		    std::function<bool(const std::string&, const SPAWN_GROUP_ENTRY&, const SPAWN_GROUP_ANCHOR&,
		                       const MONSTER_RUNTIME_PROFILE&, std::uint32_t)>;

		// 룸 초기화/리셋이 그룹과 참조를 지역 목록으로 검사한다. false는 기존 스케줄을 유지하고 outStatus를 남긴다.
		// 성공하면 모든 그룹을 DORMANT로 다시 만들며 기존 몬스터 제거는 호출자가 담당한다.
		bool Initialize(const CSpawnGroupBootstrap& bootstrap, std::string& outStatus);
		// 휴면 그룹의 선행 그룹 완료를 검사하고 첫 웨이브를 예약한다. 이 호출 자체는 몬스터를 생성하지 않으며 거절은 상태를 유지한다.
		bool Activate(const std::string& spawnGroupId);
		// trigger 재진입용 활성화다. DORMANT는 Activate와 같고 COMPLETED는 잔류 개체 수 0과 선행 그룹 완료를 확인한 뒤 다시 시작한다.
		// RUNNING은 바꾸지 않는다. 이 명시적 재시작은 자동 REPEAT policy/지연과 별개이며 false는 스케줄 변경 없음이다.
		bool Activate_Repeat(const std::string& spawnGroupId, const ACTIVE_COUNT_QUERY& activeCount);
		// 호출자가 해당 그룹 개체를 먼저 제거한 뒤 스케줄만 초기화한다. 정의 포인터는 유지하고 DORMANT·첫 wave·경과/생성 수를 되돌린다.
		bool Reset_Group(const std::string& spawnGroupId);
		// Character Select 등의 단일 몬스터 저작 요청이 즉시 생성 여부를 확인한다. 선행 조건·지연 없는 1wave/1entry/1개체만 허용한다.
		// spawn 성공 뒤 스케줄을 RUNNING으로 바꾸며 false는 스케줄을 유지한다. callback의 외부 변경을 되돌리거나 예외를 catch하지는 않는다.
		bool Activate_Immediate(const std::string& spawnGroupId, const CSpawnGroupBootstrap& bootstrap,
		                        const SPAWN_CALLBACK& spawn);
		// 룸 고정 tick이 초 단위 dt를 넘긴다. 유효한 양수 dt를 반올림한 최소 1ms로 누적해 생성/웨이브/자동 반복을 진행한다.
		// 정의와 조회/생성 callback은 호출 동안 차용하며 실제 개체 수는 룸이 유지한다. 생성 실패는 해당 entry의 시도를 중단한다.
		void Update(float fixedDeltaSeconds, const CSpawnGroupBootstrap& bootstrap,
		            const ACTIVE_COUNT_QUERY& activeCount, const SPAWN_CALLBACK& spawn);
		// 스케줄 완료 상태만 조회한다. TIMER로 끝난 그룹에는 아직 개체가 남아 있을 수 있다.
		bool Is_Completed(const std::string& spawnGroupId) const;
		// 중복 활성화 요청과 빈 Mario stage 정리가 휴면 여부를 읽는다. 미등록은 false다.
		bool Is_ActiveOrCompleted(const std::string& spawnGroupId) const;

	  private:
		// 스케줄의 휴면/진행/완료다. 몬스터 생존 판정은 이 enum이 아니라 callback의 개체 수로 따로 확인한다.
		enum class GROUP_STATE
		{
			DORMANT,
			RUNNING,
			COMPLETED
		};

		// 정의는 bootstrap을 차용하고 실행 커서/시간/생성 횟수만 소유한다. 웨이브 전환 때 Begin_Wave가 시간과 entry 횟수를 초기화한다.
		struct RUNTIME_GROUP final
		{
			const SPAWN_GROUP_DEFINITION* pDefinition = nullptr;
			GROUP_STATE eState = GROUP_STATE::DORMANT;
			std::size_t iWaveIndex = 0;
			std::uint64_t iElapsedMs = 0;
			std::vector<std::uint32_t> SpawnedByEntry;
		};

		RUNTIME_GROUP* Find(const std::string& spawnGroupId);
		const RUNTIME_GROUP* Find(const std::string& spawnGroupId) const;
		// 현재 wave index는 호출자가 결정한다. 이 helper는 경과 ms와 entry별 생성 수만 초기화하며 개체/상태 enum을 변경하지 않는다.
		static void Begin_Wave(RUNTIME_GROUP& group);

	  private:
		std::vector<RUNTIME_GROUP> m_Groups;
	};
}
