#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace LostArk::Server
{
	// 그룹의 현재 파괴 상태다. BREAKING은 예약된 최종 전이 전의 임시 상태이며 END는 유효 상태가 아닌 표식이다.
	enum class WORLD_DESTRUCTION_STATE : std::uint8_t
	{
		INTACT,
		BREAKING,
		FRACTURED,
		DESPAWNED,
		END
	};

	// stage 진입, 보스 돌진 receiver, 일반 collider 접촉을 별도 연결 조건으로 구분한다.
	enum class WORLD_DESTRUCTION_TRIGGER_KIND : std::uint8_t
	{
		STAGE,
		BOSS_IMPACT,
		// 실제 도형 접촉으로 요청한다. pattern/stage/action 시각표를 요구하지 않고 receiver ID로 연결한다.
		// 이 enum 자체가 충돌을 계산하지는 않으며 GameRoom이 확인한 접촉을 전달한다.
		COLLIDER_CONTACT,
		END
	};

	// READY만 commit 가능한 후보가 있음을 뜻한다. NO_MATCH/중복/무변경과 요청 거부를 구분한다.
	enum class WORLD_DESTRUCTION_PREPARE_RESULT : std::uint8_t
	{
		READY,
		NO_MATCH,
		DUPLICATE_REQUEST,
		NO_CHANGE,
		REJECTED
	};

	// 함께 파괴할 placement ID 집합과 초기 상태다. 전체 graph에서 placement 하나는 한 그룹만 소유한다.
	struct WORLD_DESTRUCTION_GROUP_DESCRIPTOR final
	{
		std::string strGroupId;
		std::vector<std::string> MemberPlacementIds;
		WORLD_DESTRUCTION_STATE eInitialState =
			WORLD_DESTRUCTION_STATE::INTACT;
	};

	// 그룹을 FRACTURED 또는 DESPAWNED로 바꿀 규칙이다. 기간은 초나 ms가 아닌 Server tick 수다.
	// 0이면 즉시 최종 상태로, 양수이면 BREAKING을 거쳐 예약 tick에 최종 상태로 전이한다.
	struct WORLD_DESTRUCTION_MUTATION_DESCRIPTOR final
	{
		std::string strMutationId;
		std::string strGroupId;
		WORLD_DESTRUCTION_STATE eFinalState =
			WORLD_DESTRUCTION_STATE::FRACTURED;
		std::uint32_t iBreakingDurationTicks = 0u;
		std::string strCollisionStateId;
		std::string strNavigationStateId;
		// 옆의 장애물이 아니라 서 있는 바닥을 제거하는 정책이다. GameRoom이 navigation의 void 조건에 등록한다.
		// 게시 bootstrap 파서는 이 값이 true이면 navigation condition ID도 요구한다. Runtime은 그 외부 참조를 직접 검증하지 않는다.
		bool bRemovesGround = false;
	};

	// 어떤 trigger가 어떤 mutation을 요청하는지 연결한다. contact는 action tuple을 비우고 receiver만 사용한다.
	struct WORLD_DESTRUCTION_BINDING_DESCRIPTOR final
	{
		std::string strBindingId;
		std::string strMutationId;
		WORLD_DESTRUCTION_TRIGGER_KIND eTriggerKind =
			WORLD_DESTRUCTION_TRIGGER_KIND::STAGE;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::uint32_t iStageIndex = 0u;
		std::string strImpactReceiverId;
	};

	// bootstrap이 읽은 값 그래프다. Runtime은 검증한 정의를 자체 컨테이너로 복사하여 입력 수명에 의존하지 않는다.
	struct WORLD_DESTRUCTION_DESCRIPTOR_GRAPH final
	{
		std::vector<WORLD_DESTRUCTION_GROUP_DESCRIPTOR> Groups;
		std::vector<WORLD_DESTRUCTION_MUTATION_DESCRIPTOR> Mutations;
		std::vector<WORLD_DESTRUCTION_BINDING_DESCRIPTOR> Bindings;
	};

	// 보스의 현재 pattern/stage/action ID와 stage index다. stage와 impact 연결은 네 값이 모두 같아야 한다.
	struct WORLD_DESTRUCTION_ACTION_TUPLE final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::uint32_t iStageIndex = 0u;
	};

	// 이번 transaction의 trigger 적용 기록이다. binding ID/sequence/source ID가 commit 중복 방지 키다.
	struct WORLD_DESTRUCTION_BINDING_APPLICATION final
	{
		std::string strBindingId;
		std::string strMutationId;
		std::uint64_t iSourceNetEntityId = 0u;
		std::uint32_t iPatternSequence = 0u;
		WORLD_DESTRUCTION_TRIGGER_KIND eTriggerKind =
			WORLD_DESTRUCTION_TRIGGER_KIND::END;
	};

	// 준비 당시 상태/version과 다음 상태/version을 함께 실어 Commit에서 오래된 후보를 거부한다.
	// 멤버/충돌/navigation ID도 정의와 다시 대조하며 실제 외부 시스템 변경은 GameRoom이 조율한다.
	struct WORLD_DESTRUCTION_STATE_TRANSITION final
	{
		std::string strGroupId;
		std::string strMutationId;
		WORLD_DESTRUCTION_STATE ePreviousState =
			WORLD_DESTRUCTION_STATE::END;
		WORLD_DESTRUCTION_STATE eNextState =
			WORLD_DESTRUCTION_STATE::END;
		WORLD_DESTRUCTION_STATE eFinalState =
			WORLD_DESTRUCTION_STATE::END;
		std::uint32_t iPreviousStateVersion = 0u;
		std::uint32_t iNextStateVersion = 0u;
		std::uint32_t iCommitTick = 0u;
		bool bApplyPersistentMutation = false;
		std::vector<std::string> MemberPlacementIds;
		std::string strCollisionStateId;
		std::string strNavigationStateId;
	};

	// 한 epoch 안의 여러 그룹 변경 후보다. trigger 요청이면 application과 transition의 같은 index가 대응한다.
	// 예약 완료 요청이면 BindingApplications가 비어 있으며 BREAKING 완료 전이만 허용한다.
	struct WORLD_DESTRUCTION_TRANSACTION final
	{
		std::uint32_t iEncounterEpoch = 0u;
		std::uint32_t iRequestTick = 0u;
		std::vector<WORLD_DESTRUCTION_BINDING_APPLICATION>
			BindingApplications;
		std::vector<WORLD_DESTRUCTION_STATE_TRANSITION> Transitions;
	};

	// 조회/복제를 위한 그룹 상태 값 복사본이다. 내부 map 요소나 문자열 참조를 노출하지 않는다.
	struct WORLD_DESTRUCTION_GROUP_STATE final
	{
		std::string strGroupId;
		WORLD_DESTRUCTION_STATE eState =
			WORLD_DESTRUCTION_STATE::END;
		std::uint32_t iStateVersion = 0u;
		std::uint32_t iStateStartTick = 0u;
		std::uint32_t iCommitTick = 0u;
		std::string strPendingMutationId;
	};

	// GameRoom이 소유하는 파괴 상태 머신이다. 충돌/nav/패킷은 직접 변경하지 않고 transaction으로 경계를 넘긴다.
	// 내부 동기화는 없으므로 방의 호출 경로가 Prepare와 Commit 및 Reset의 순서를 보장해야 한다.
	class CWorldDestructionRuntime final
	{
	public:
		// 처음 한 번 graph를 검증해 소유한다. false이면 멤버를 교체하지 않으며 이미 초기화된 객체의 재초기화는 거부한다.
		bool Initialize(
			const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& descriptorGraph,
			std::string& status,
			std::uint32_t initialServerTick = 1u);
		// 정의는 유지하고 초기 상태/version 1로 되돌리며 epoch를 증가시킨다. 기존 중복 기록과 예약 mutation은 비운다.
		bool Reset(
			std::string& status,
			std::uint32_t resetServerTick = 1u);

		// GameRoom의 stage 진입 요청이다. 정확한 action tuple에 연결된 변경 후보만 만들며 실제 상태는 유지한다.
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_StageTrigger(
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		// 돌진이 만난 receiver와 현재 action tuple을 함께 검사한다. 도형 검사는 호출자가 이미 수행한다.
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_ImpactTrigger(
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			const std::string& impactReceiverId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		// GameRoom이 실제 접촉한 벽 ID를 전달한다. 현재 제품 호출은 server tick을 contactSequence로 사용한다.
		// 이 runtime은 sequence의 증가 순서를 검사하지 않고 binding/sequence/source의 같은 조합을 중복으로 거른다.
		// 이미 파괴 중이거나 파괴된 그룹은 새 상태 전이가 없으며 reset 전에는 INTACT로 되돌리지 않는다.
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_ContactTrigger(
			const std::string& contactCollisionId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t contactSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		// GameRoom의 tick 처리가 기한에 도달한 BREAKING 그룹을 모은다. 출력 transaction은 매 호출 먼저 비운다.
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_DueStateCommits(
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		// epoch/상태/version/정의/중복 키를 복사본에서 검사하고 성공할 때 그룹과 적용 기록을 함께 교체한다.
		// false 반환이면 두 실제 컨테이너는 보존한다. 방의 충돌/navigation/송신 성공까지 뜻하는 반환값은 아니다.
		bool Commit(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status);

		// 복제와 pickup/벽 조회가 한 그룹의 값 복사본을 받는다. ID가 없으면 false이고 출력은 유지한다.
		bool Find_GroupState(
			const std::string& groupId,
			WORLD_DESTRUCTION_GROUP_STATE& state) const;
		// 전체 동기화용 상태를 group ID의 map 순서로 복사해 반환한다.
		std::vector<WORLD_DESTRUCTION_GROUP_STATE> Get_GroupStates() const;

		// 정의 graph가 한 번 승인되었는지 조회한다. 개별 벽의 현재 상태와는 별도다.
		bool Is_Initialized() const noexcept
		{
			return m_bInitialized;
		}
		// Reset 전 transaction을 거부하고 전체 동기화 세대를 식별하는 번호다.
		std::uint32_t Get_EncounterEpoch() const noexcept
		{
			return m_iEncounterEpoch;
		}

	private:
		// 그룹 정의와 변경 가능한 진행 상태를 map 요소 하나가 소유한다. 예약 ID/기한은 BREAKING 동안 유지한다.
		struct GROUP_RUNTIME final
		{
			WORLD_DESTRUCTION_GROUP_DESCRIPTOR Descriptor;
			WORLD_DESTRUCTION_STATE eState =
				WORLD_DESTRUCTION_STATE::END;
			std::uint32_t iStateVersion = 0u;
			std::uint32_t iStateStartTick = 0u;
			std::uint32_t iCommitTick = 0u;
			std::string strPendingMutationId;
		};

		// 세 종류 요청이 공유하는 후보 생성 경로다. 결과가 READY가 아니면 transaction을 빈 값으로 돌려준다.
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_Trigger(
			WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			const std::string& impactReceiverId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		bool m_bInitialized = false;
		std::uint32_t m_iEncounterEpoch = 0u;
		// ID 조회와 안정된 전체 조회 순서를 위한 map이다. 값이 정의/상태를 소유하며 입력 graph를 차용하지 않는다.
		std::map<std::string, GROUP_RUNTIME> m_Groups;
		std::map<std::string, WORLD_DESTRUCTION_MUTATION_DESCRIPTOR>
			m_Mutations;
		// 요청 평가에서는 원래 binding 순서를 유지하고, Commit은 아래 ID map으로 정확한 정의를 재조회한다.
		std::vector<WORLD_DESTRUCTION_BINDING_DESCRIPTOR> m_Bindings;
		std::map<std::string, WORLD_DESTRUCTION_BINDING_DESCRIPTOR>
			m_BindingById;
		// 성공한 binding/sequence/source 조합만 보관한다. epoch는 키에 넣지 않고 Reset 때 전체 기록을 지운다.
		std::set<std::tuple<std::string, std::uint32_t, std::uint64_t>>
			m_AppliedBindingSequences;
	};
}
