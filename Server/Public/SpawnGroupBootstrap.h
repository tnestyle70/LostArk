#pragma once

// 게시 파일 선택과 헤더 일치 검사에 사용하는 Shared WORLD_ID를 가져온다.
#include "Network/PacketType.h"

// 게시 revision·개수·ms와 정책 enum의 고정 폭 정수를 사용한다.
#include <cstdint>
// archetype/anchor/group/wave ID 및 로드 진단을 값으로 소유한다.
#include <string>
// 생성 시 anchor와 monster profile을 stable ID로 찾는다.
#include <unordered_map>
// 그룹·웨이브·entry의 게시 순서를 보관하며 runtime이 정의 주소를 차용한다.
#include <vector>

namespace LostArk::Server
{
	// Spawn_Monster가 SERVER_WORLD_ENTITY로 복사하는 일반 몬스터의 수치 정의다. 이후 MonsterBrain과 ServerCombatHitRuntime이 소비한다.
	struct MONSTER_RUNTIME_PROFILE final
	{
		std::string strArchetypeId;
		std::uint32_t iMaxHp = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		// 충돌/교전/추적 해제/공격/도착 감속 거리는 m, 이동 속도는 m/s, 가감속은 m/s², 회전 속도는 degree/s다.
		float fCollisionRadius = 0.f;
		float fEngageRange = 0.f;
		float fTargetReleaseRange = 0.f;
		float fMoveSpeed = 0.f;
		float fTurnSpeedDegreesPerSecond = 0.f;
		float fAcceleration = 0.f;
		float fDeceleration = 0.f;
		float fArrivalSlowRadius = 0.f;
		float fAttackRange = 0.f;
		// 공격 예고·판정·회복과 사망 후 제거 지연은 ms다. HP=0이어도 제거 지연 동안 그룹의 잔류 개체 수에는 포함된다.
		std::uint32_t iAttackWindupMs = 0;
		std::uint32_t iAttackActiveMs = 0;
		std::uint32_t iAttackRecoveryMs = 0;
		std::uint32_t iDeadDespawnMs = 0;
		// 플레이어 공격의 저작 push 거리(m)에 곱하는 피격 이동 배율이다. 0이면 이 피격 밀기 경로가 이동시키지 않는다.
		// 몬스터의 일반 이동이나 다른 강제 이동까지 금지하는 값은 아니다.
		float fHitKnockbackScale = 0.f;
		// 몬스터 공격이 플레이어를 iAttackPushMs 동안 이동시킬 거리(m)다. 음수는 공격자 쪽으로 끌어당긴다.
		// 넘어짐 여부와 유지 ms는 아래 별도 필드가 전투 피격 경로로 전달한다.
		float fAttackPushRangeM = 0.f;
		std::uint32_t iAttackPushMs = 0;
		bool bAttackKnockdown = false;
		std::uint32_t iAttackDownMs = 0;
	};

	// Spawn_Monster가 navigation에 투영할 world 위치(m)와 시작 yaw(degree)다. 저장 ID가 모델/prototype 주소를 뜻하지는 않는다.
	struct SPAWN_GROUP_ANCHOR final
	{
		std::string strAnchorId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
	};

	// 같은 archetype을 같은 anchor에서 순차 생성하는 예약이다. 시간은 웨이브 시작 지연 뒤 initialDelay + 성공 ordinal×interval(ms)로 계산한다.
	struct SPAWN_GROUP_ENTRY final
	{
		std::string strArchetypeId;
		std::uint32_t iCount = 0;
		std::string strAnchorId;
		std::uint32_t iInitialDelayMs = 0;
		std::uint32_t iSpawnIntervalMs = 0;
	};

	enum class SPAWN_NEXT_WAVE_POLICY : std::uint8_t
	{
		// 현재 wave의 모든 예약 생성이 성공하고 그룹 잔류 개체 수가 0이면 끝난다. 사망 즉시가 아니라 실제 개체 제거까지 기다린다.
		ALL_DEAD,
		// 웨이브 경과가 startDelay + nextWaveDelay에 도달하면 생성 완료·잔류 개체와 무관하게 끝난다.
		// 미생성 entry가 남을 수 있으며 다음 wave의 생성에도 그룹 iMaxAlive가 적용된다.
		TIMER
	};

	enum class SPAWN_GROUP_REPEAT_POLICY : std::uint8_t
	{
		ONCE,
		// 완료 그룹의 잔류 개체 수가 0인 기간을 iRepeatDelayMs만큼 누적한 뒤 첫 wave를 다시 예약한다. 개체가 남으면 대기 시간을 0으로 돌린다.
		REPEAT
	};

	// 그룹 안에서 게시 순서대로 진행한다. 다음 wave로 넘어간 tick에는 새 생성 루프를 실행하지 않는다.
	struct SPAWN_GROUP_WAVE final
	{
		std::string strWaveId;
		std::uint32_t iStartDelayMs = 0;
		SPAWN_NEXT_WAVE_POLICY eNextWavePolicy = SPAWN_NEXT_WAVE_POLICY::ALL_DEAD;
		// TIMER만 읽는 추가 전환 지연(ms)이다. runtime은 iStartDelayMs+iNextWaveDelayMs를 완료 기준으로 쓴다.
		// ALL_DEAD는 0, TIMER는 양수를 게시해야 하며 bootstrap loader가 정책과 값의 조합을 검사한다.
		std::uint32_t iNextWaveDelayMs = 0;
		std::vector<SPAWN_GROUP_ENTRY> Entries;
	};

	// 그룹 ID와 선행 그룹의 스케줄 완료 조건을 연결한다. 개체 소유와 현재 실행 상태는 bootstrap이 아니라 룸/runtime이 담당한다.
	struct SPAWN_GROUP_DEFINITION final
	{
		std::string strSpawnGroupId;
		std::string strRequiredCompletedGroupId;
		// 여러 wave에 걸친 그룹 전체 잔류 몬스터 한도다. 현재 룸 조회는 HP를 검사하지 않아 제거 대기 중 개체도 포함한다.
		std::uint32_t iMaxAlive = 0;
		SPAWN_GROUP_REPEAT_POLICY eRepeatPolicy = SPAWN_GROUP_REPEAT_POLICY::ONCE;
		// 자동 REPEAT가 개체 수 0을 유지하며 누적하는 지연(ms)이다. REPEAT는 양수, ONCE는 0이어야 한다.
		// trigger의 명시적 Activate_Repeat는 이 자동 지연과 별개다.
		std::uint32_t iRepeatDelayMs = 0;
		std::vector<SPAWN_GROUP_WAVE> Waves;
	};

	// World publisher가 만든 world별 tab 문서의 정의 저장소다. GameRoom이 소유하고 SpawnGroupRuntime이 그룹 주소를 차용한다.
	// 재로드/파괴는 차용 중 runtime과 직렬로 조율해야 한다. 자체 잠금은 없으며 정의와 반환 참조의 수명은 이 저장소에 속한다.
	class CSpawnGroupBootstrap final
	{
	  public:
		// 룸 초기화가 world별 schema4 파일을 읽는다. 파일 열기 실패는 모든 정의·revision을 비운 성공이며, 명시적 형식 실패는 기존 정의를 보존한다.
		// 진단은 실패 때 갱신한다. 할당·경로 등의 예외를 catch하지 않으며 publisher 전체 저작 검증을 대신하지 않는다.
		bool Load(LostArk::Shared::WORLD_ID worldId);

		// runtime 초기화가 vector 원소 주소를 차용한다. Load의 성공한 교체/비우기 이후 기존 주소를 재사용하지 않는다.
		const std::vector<SPAWN_GROUP_DEFINITION>& Get_Groups() const
		{
			return m_Groups;
		}
		// 생성 예약 실행이 anchor를 차용한다. 미등록은 nullptr이며 성공한 재로드/파괴 뒤 포인터를 유지하지 않는다.
		const SPAWN_GROUP_ANCHOR* Find_Anchor(const std::string& anchorId) const;
		// 몬스터 생성 경로가 수치를 복사하기 위한 조회다. 반환 포인터 자체는 profile 저장소의 수명을 늘리지 않는다.
		const MONSTER_RUNTIME_PROFILE* Find_Profile(const std::string& archetypeId) const;
		// 룸 초기화가 실패 이유를 자기 진단에 복사한다. 이 참조는 다음 Load의 진단 갱신에 영향을 받는다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		// 검증을 통과해 교체한 게시 revision 값이다. 선택 문서가 없으면 0이며 이전 revision보다 새로운지 비교하지 않는다.
		std::uint32_t Get_Revision() const
		{
			return m_iRevision;
		}

	  private:
		std::vector<SPAWN_GROUP_DEFINITION> m_Groups;
		std::unordered_map<std::string, SPAWN_GROUP_ANCHOR> m_Anchors;
		std::unordered_map<std::string, MONSTER_RUNTIME_PROFILE> m_Profiles;
		std::string m_strStatus;
		std::uint32_t m_iRevision = 0;
	};
}
