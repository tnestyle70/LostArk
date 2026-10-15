#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// 반복해서 세우고 부수는 기믹 소품의 수명이다. 일방향 벽 파괴 그룹을 되감는 대신 슬롯별 상태와 소환 회차를 관리한다.
	// Prepare는 변경 후보를 값으로 만들고 Commit은 live version을 확인한다. Client 송신까지 이 클래스가 원자적으로 처리하는 것은 아니다.
	// slot ID가 Server 상태와 Client 표현을 연결하는 stable identity다. 배치 index를 전투 식별자로 쓰지 않는다.
	// INTACT 슬롯은 엄폐 후보가 된다. 게시된 배치 위치를 Server 정의에 넣지만 실제 화면과 일치하는지는 배포/표현 경로에서도 확인해야 한다.
	struct ENCOUNTER_PROP_SLOT_DESCRIPTOR final
	{
		std::string strSlotId;
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
	};

	// 게시된 기믹 소품 집합의 정의다. 슬롯 위치/반경은 월드 XZ의 m이며 정의 vector가 ID와 위치를 소유한다.
	struct ENCOUNTER_PROP_SET_DESCRIPTOR final
	{
		std::string strPropSetId;
		std::string strEncounterId;
		// m 단위 엄폐 원 반경이다. Room은 INTACT 슬롯의 월드 XZ와 함께 발탄 공격의 엄폐 판정에 넘긴다.
		float fCoverRadiusMeters = 0.f;
		std::vector<ENCOUNTER_PROP_SLOT_DESCRIPTOR> Slots;
	};

	// 게시된 소품 정의를 읽는다. 시작 시 outSets를 비우며 파일 열기 실패는 원인을 구분하지 않고 빈 목록 성공으로 처리한다.
	[[nodiscard]] bool Load_EncounterPropSets(LostArk::Shared::WORLD_ID worldId,
	                                          std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR>& outSets,
	                                          std::string& status);

	// 현재 복제할 슬롯 상태다. version은 상태 변경 식별, start tick은 해당 상태의 시작, occurrence는 소환 회차다.
	struct ENCOUNTER_PROP_SLOT_STATE final
	{
		std::string strSlotId;
		LostArk::Shared::ENCOUNTER_PROP_STATE eState = LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN;
		std::uint32_t iStateVersion = 0u;
		std::uint32_t iStateStartTick = 0u;
		std::uint32_t iOccurrenceSequence = 0u;
		// 정의에서 복사한 월드 XZ다. live 슬롯 목록만으로 엄폐 원을 만들 수 있도록 실행 상태에도 보관한다.
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
	};

	enum class ENCOUNTER_PROP_PREPARE_RESULT : std::uint8_t
	{
		READY,
		// 새 후보가 없는 결과다. 동일 상태/회차, 이전 소환 요청, 아직 제거 시각이 아님 등 호출별 사유가 있다.
		NO_CHANGE,
		REJECTED
	};

	// Prepare가 만드는 변경 슬롯의 소유 복사본이다. 현재 runtime은 바꾸지 않지만 실패한 출력에 일부 후보가 남을 수 있다.
	// READY일 때만 Commit하며, 외부에서 임의로 만든 transaction의 모든 필드를 검증하는 API로 사용하지 않는다.
	struct ENCOUNTER_PROP_TRANSACTION final
	{
		std::uint32_t iEncounterEpoch = 0u;
		std::uint32_t iCommitTick = 0u;
		std::uint32_t iOccurrenceSequence = 0u;
		std::vector<ENCOUNTER_PROP_SLOT_STATE> Slots;
	};

	// GameRoom이 발탄의 반복 소품 상태를 소유한다. HIDDEN -> INTACT -> BREAKING -> HIDDEN 수명과 복제 version을 관리한다.
	// 타격/엄폐 계산 및 네트워크 송신은 Room/Brain 소비자가 수행하며 이 객체는 내부 잠금을 제공하지 않는다.
	class CEncounterPropRuntime final
	{
	public:
		// GameRoom 초기화가 정의를 복사하고 슬롯 ID 순서로 HIDDEN 상태를 만든다. 명시적 false는 기존 멤버를 보존한다.
		bool Initialize(const ENCOUNTER_PROP_SET_DESCRIPTOR& descriptor,
		                std::string& status,
		                std::uint32_t initialServerTick = 1u);
		// Room 초기화/퇴장 정리에서 모든 슬롯을 숨기고 epoch를 진행한다. 무효 입력 false는 멤버를 보존한다.
		bool Reset(std::string& status, std::uint32_t resetServerTick = 1u);

		// 새 회차에 모든 HIDDEN 슬롯을 세울 후보를 준비한다. 현재 회차가 0이 아닐 때 그 이하의 소환 요청은 NO_CHANGE로 처리한다.
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Spawn(std::uint32_t occurrenceSequence,
		                                            std::uint32_t serverTick,
		                                            ENCOUNTER_PROP_TRANSACTION& transaction,
		                                            std::string& status) const;
		// INTACT 슬롯을 BREAKING으로 준비한다. 같은 요청 회차에서 이미 BREAKING인 슬롯은 건너뛰며 다른 출발 상태는 거절한다.
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Break(std::uint32_t occurrenceSequence,
		                                            std::uint32_t serverTick,
		                                            ENCOUNTER_PROP_TRANSACTION& transaction,
		                                            std::string& status) const;
		// 호출자가 지정한 slot ID만 파괴 후보에 넣는다. 같은 회차의 기존 BREAKING은 건너뛴다.
		// 벡터 순서가 바뀌어도 ID로 live 슬롯을 찾으며 중복 ID 요청은 거절한다.
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_BreakSlots(const std::vector<std::string>& slotIds,
		                                                 std::uint32_t occurrenceSequence,
		                                                 std::uint32_t serverTick,
		                                                 ENCOUNTER_PROP_TRANSACTION& transaction,
		                                                 std::string& status) const;
		// BREAKING 기간을 서버 tick 단위로 받아 시간이 지난 슬롯을 HIDDEN으로 준비한다.
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_DueRemoval(std::uint32_t serverTick,
		                                                 std::uint32_t breakingTicks,
		                                                 ENCOUNTER_PROP_TRANSACTION& transaction,
		                                                 std::string& status) const;

		// GameRoom이 READY 후보의 epoch와 각 슬롯의 다음 version을 먼저 검사한 뒤 슬롯들을 교체한다.
		// 명시적 false는 live 상태를 보존한다. 상태 전이·중복 ID·좌표 등 임의 transaction 전체 검증이나 할당 예외 rollback은 제공하지 않는다.
		bool Commit(const ENCOUNTER_PROP_TRANSACTION& transaction, std::string& status);

		// 아래 참조는 runtime이 소유하며 재초기화/파괴를 넘겨 보관하지 않는다. 상태 변경과 동시 접근은 호출자가 피한다.
		bool Is_Initialized() const noexcept
		{
			return m_bInitialized;
		}

		std::uint32_t Get_EncounterEpoch() const noexcept
		{
			return m_iEncounterEpoch;
		}

		std::uint32_t Get_OccurrenceSequence() const noexcept
		{
			return m_iOccurrenceSequence;
		}

		float Get_CoverRadiusMeters() const noexcept
		{
			return m_Descriptor.fCoverRadiusMeters;
		}

		const std::string& Get_PropSetId() const noexcept
		{
			return m_Descriptor.strPropSetId;
		}

		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& Get_SlotStates() const
		{
			return m_Slots;
		}

	private:
		// 모든 슬롯에 같은 상태 전이를 준비한다. live는 보존하지만 transaction은 먼저 비우고 검사하며 일부 후보가 남은 채 거절될 수 있다.
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Transition(LostArk::Shared::ENCOUNTER_PROP_STATE fromState,
		                                                 LostArk::Shared::ENCOUNTER_PROP_STATE toState,
		                                                 std::uint32_t occurrenceSequence,
		                                                 std::uint32_t serverTick,
		                                                 ENCOUNTER_PROP_TRANSACTION& transaction,
		                                                 std::string& status) const;

		// 정의와 live 슬롯은 runtime이 소유한다. transaction은 별도 값 복사본이며 Commit 뒤에 Room이 전체 상태를 복제한다.
		ENCOUNTER_PROP_SET_DESCRIPTOR m_Descriptor;
		std::vector<ENCOUNTER_PROP_SLOT_STATE> m_Slots;
		std::uint32_t m_iEncounterEpoch = 0u;
		std::uint32_t m_iOccurrenceSequence = 0u;
		bool m_bInitialized = false;
	};
}
