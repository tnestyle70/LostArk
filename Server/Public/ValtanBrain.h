#pragma once

#include "GameplayCatalog.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/CombatObjectHitChain.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// 패턴 선택의 출처를 기록한다. 선택 결과와 별개로 강제 요청, 순차 선택, 가중치 선택을 구분한다.
	enum class VALTAN_DECISION_SOURCE : std::uint8_t
	{
		NONE,
		INTRO,
		FORCED_HEALTH_BAR,
		FORCED_AUDITION,
		ORDERED,
		WEIGHTED,
		GLOBAL,
		// 기존 Debug 호출부의 소스 호환성을 유지하는 별칭이다. 새 기록은 위의 명시적인 이름을 사용한다.
		FORCED_MECHANIC = FORCED_HEALTH_BAR,
		ROTATION_ORDERED = ORDERED,
		ROTATION_WEIGHTED = WEIGHTED,
		GLOBAL_WEIGHTED = GLOBAL
	};

	// 선택 성공뿐 아니라 대기, 후보 부재, 카탈로그 부재도 남겨 디버그 조회에서 원인을 구분한다.
	enum class VALTAN_DECISION_RESULT : std::uint8_t
	{
		SELECTED,
		WAITING_FOR_INTRO_RANGE,
		NO_ELIGIBLE_PATTERN,
		NO_VALID_TARGET,
		CATALOG_UNAVAILABLE,
		MECHANIC_RESET_REQUIRED
	};

	// 한 후보가 여러 제외 사유를 동시에 가질 수 있으므로 비트 마스크를 사용한다.
	// SOFT_REPEAT_BLOCKED는 영구 제외가 아니다. 다른 조건을 통과한 후보가 모두 반복 제한에 걸릴 수 있다.
	// 이때 선택기가 SOFT_REPEAT_RELAXED를 기록하고 원래 가중치로 추첨한다.
	enum VALTAN_DECISION_EXCLUSION : std::uint32_t
	{
		VALTAN_EXCLUDE_NONE = 0u,
		VALTAN_EXCLUDE_WRONG_SELECTION_KIND = 1u << 0u,
		VALTAN_EXCLUDE_INTRO_ROW = 1u << 1u,
		VALTAN_EXCLUDE_NOT_IN_SELECTION_SET = 1u << 2u,
		VALTAN_EXCLUDE_ARMOR_MISMATCH = 1u << 3u,
		VALTAN_EXCLUDE_PHASE_REQUIREMENT = 1u << 4u,
		VALTAN_EXCLUDE_PHASE_RANGE = 1u << 5u,
		VALTAN_EXCLUDE_HEALTH_BAR_RANGE = 1u << 6u,
		VALTAN_EXCLUDE_NO_TARGET = 1u << 7u,
		VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE = 1u << 8u,
		VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE = 1u << 9u,
		VALTAN_EXCLUDE_COOLDOWN = 1u << 10u,
		VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED = 1u << 11u,
		VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED = 1u << 12u,
		VALTAN_EXCLUDE_DISABLED = 1u << 13u,
		VALTAN_EXCLUDE_UNRESOLVED_DEFINITION = 1u << 14u,
		// 초기 디버그 관측 코드가 사용한 이름과 비트 값을 유지한다.
		VALTAN_EXCLUDE_NOT_NORMAL = VALTAN_EXCLUDE_WRONG_SELECTION_KIND,
		VALTAN_EXCLUDE_INTRO = VALTAN_EXCLUDE_INTRO_ROW,
		VALTAN_EXCLUDE_ROTATION = VALTAN_EXCLUDE_NOT_IN_SELECTION_SET,
		VALTAN_EXCLUDE_ARMOR = VALTAN_EXCLUDE_ARMOR_MISMATCH,
		VALTAN_EXCLUDE_HEALTH_BAR = VALTAN_EXCLUDE_HEALTH_BAR_RANGE,
		VALTAN_EXCLUDE_RANGE = VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE |
			VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE,
		VALTAN_EXCLUDE_MAXIMUM_CONSECUTIVE =
			VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED,
		VALTAN_EXCLUDE_MISSING_DEFINITION =
			VALTAN_EXCLUDE_UNRESOLVED_DEFINITION
	};

	// 한 번의 선택에서 후보 한 개가 통과하거나 탈락한 근거를 값으로 보관한다.
	// 정의의 주소를 저장하지 않으므로 이후 카탈로그 교체와 독립적으로 당시 결과를 조회할 수 있다.
	struct VALTAN_DECISION_CANDIDATE_TRACE final
	{
		std::string strPatternId;
		std::uint32_t iExclusionMask = VALTAN_EXCLUDE_NONE;
		// 저작 가중치와 조건 적용 후 실제 추첨 가중치를 나눠 제한이 미친 영향을 보여 준다.
		std::uint32_t iAuthoredWeight = 0u;
		std::uint32_t iEffectiveWeight = 0u;
		std::uint32_t iCooldownRemainingTicks = 0u;
		std::uint32_t iConsecutiveUses = 0u;
		std::uint32_t iMaximumConsecutiveUses = 0u;
		// 누적 가중치 구간은 [시작, 끝)이다. 추첨 번호가 이 구간에 들어오는 후보가 선택된다.
		std::uint64_t iWeightBeginInclusive = 0u;
		std::uint64_t iWeightEndExclusive = 0u;
		bool bSoftRepeatBlocked = false;
		bool bSoftRepeatRelaxed = false;
		bool bSelected = false;
	};

	// 선택 시점의 입력 상태, 난수와 후보 결과를 묶어 당시 HP, 대상과 선택 경로를 보존한다.
	// CValtanBrain의 제한된 순환 버퍼가 이 값을 소유한다.
	struct VALTAN_DECISION_TRACE final
	{
		// 기록 순번과 서버 틱은 다른 값이다. 패턴 실행 순번도 별도로 남겨 같은 틱의 판단과 실행을 연결한다.
		std::uint64_t iTraceSequence = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequenceBeforeDecision = 0u;
		std::uint32_t iExpectedPatternSequence = 0u;
		std::uint32_t iCurrentHp = 0u;
		std::uint32_t iMaximumHp = 0u;
		std::uint32_t iHealthBar = 0u;
		std::uint8_t iGameplayPhase = 1u;
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fTargetDistance = 0.f;
		bool bIntroPatternConsumed = false;
		std::uint32_t iRotationStepIndex = 0u;
		VALTAN_DECISION_SOURCE eSource = VALTAN_DECISION_SOURCE::NONE;
		VALTAN_DECISION_RESULT eResult =
			VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
		std::string strRotationId;
		// 아직 실행하지 못한 요청과 이번에 선택한 결과를 분리해 강제 패턴의 대기 사유를 추적한다.
		std::string strPendingPatternId;
		VALTAN_DECISION_SOURCE ePendingSource =
			VALTAN_DECISION_SOURCE::NONE;
		std::string strSelectedPatternId;
		// 원래 난수 입력, 혼합 결과, 전체 가중치와 최종 추첨 번호를 함께 보관한다.
		std::uint64_t iRawRandomInput = 0u;
		std::uint64_t iMixedRandomValue = 0u;
		std::uint64_t iTotalWeight = 0u;
		std::uint64_t iRandomTicket = 0u;
		bool bMaximumConsecutiveRelaxed = false;
		bool bCandidatesTruncated = false;
		// 후보 기록은 MAX_DECISION_CANDIDATE_COUNT로 제한한다. 잘린 기록은 위 플래그로 구분한다.
		std::vector<VALTAN_DECISION_CANDIDATE_TRACE> Candidates;
	};

	// Brain은 CAPTURE 공격의 명중과 생존 여부를 판단하고, GameRoom은 행동 중단과 부착 수명을 관리한다.
	// 같은 고정 틱 안에서 두 책임을 잇는 요청이다. 플레이어 포인터 대신 네트워크 ID를 넘긴다.
	// 소비 시 GameRoom이 현재 플레이어를 다시 찾고 부착을 적용한다.
	struct SERVER_PLAYER_CAPTURE_REQUEST final
	{
		LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	};

	enum class SERVER_BOSS_GRAB_CLASSIFICATION : std::uint8_t
	{
		NONE,
		PARTIAL,
		ALL
	};

	// 안내 NPC를 제외한 생존 플레이어와 이 보스의 현재 패턴에 유효하게 붙잡힌 플레이어를 집계한다.
	// 잘못된 부착이 하나라도 있으면 전원 포획으로 확정하지 않도록 별도 플래그를 둔다.
	struct SERVER_BOSS_GRAB_ROSTER final
	{
		std::size_t iAliveCount = 0u;
		std::size_t iGrabbedCount = 0u;
		bool bHasInvalidAttachment = false;
		SERVER_BOSS_GRAB_CLASSIFICATION eClassification =
			SERVER_BOSS_GRAB_CLASSIFICATION::NONE;
	};

	// Brain이 산출한 부채꼴 공격을 GameRoom의 오브젝트 피격 처리로 전달하는 값이다.
	// 공격 출처와 패턴 실행 순번을 함께 넘겨 해당 공격의 오브젝트 반응과 연결한다.
	struct SERVER_BOSS_PATTERN_HIT final
	{
		LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		std::string strPatternId, strActionId;
		LostArk::Shared::CombatObjectHitChain::CONE_HIT Cone;
	};

	// GameRoom의 보스 갱신에서 패턴 선택, 단계 진행과 플레이어 명중을 처리한다.
	// 보스와 플레이어의 실제 상태는 호출자가 소유하고, Brain은 선택 판단의 최근 기록만 보관한다.
	// 이동 후보의 충돌 검증, 단계 액션의 적용과 포획 부착은 GameRoom의 후속 처리로 연결된다.
	class CValtanBrain final
	{
	public:
		// GameRoom의 안내 NPC 위협 판단이 호출하는 읽기 전용 조회다. horizonMs는 앞으로 살펴볼 밀리초다.
		// 현재 보스 위치와 공격 도형, 단계별 명중 시점을 사용해 probe가 닿을 예정인 공격 수를 누적한다.
		// 미래 이동 전체를 시뮬레이션한 피해량이나 확률이 아니며 실제 피해는 Update의 명중 처리에서 정한다.
		[[nodiscard]] static float Predict_ContactRisk(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const SERVER_PLAYER& probe,
			std::uint32_t serverTick, std::uint32_t horizonMs);

		// 기록과 기믹 발생 내역의 보관량을 제한한다. 오래된 선택 기록은 새 기록이 들어오면 덮어쓴다.
		static constexpr std::size_t MAX_DECISION_TRACE_COUNT = 32u;
		static constexpr std::size_t MAX_DECISION_CANDIDATE_COUNT = 64u;
		static constexpr std::size_t MAX_MECHANIC_OCCURRENCE_COUNT = 64u;

		// Brain의 분기 선택과 GameRoom의 단계 액션 검증이 같은 포획 기준을 사용한다.
		// 생존, 보스 ID, 현재 패턴 순번, 손 부착 슬롯과 유한한 부착 좌표를 확인하며 입력은 변경하지 않는다.
		[[nodiscard]] static SERVER_BOSS_GRAB_ROSTER Classify_GrabbedPlayers(
			const SERVER_WORLD_ENTITY& boss,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);

		// 현재 HP를 전체 체력줄 수에 비례시켜 올림한다. HP가 남아 있으면 마지막 줄도 1로 표시한다.
		// 패턴의 체력줄 임계값 판단과 GameRoom의 상태 조회가 같은 환산을 사용한다. 유효한 HP 범위가 없으면 0이다.
		[[nodiscard]] static std::uint32_t Calculate_HealthBar(
			const SERVER_WORLD_ENTITY& boss);
		// GameRoom의 Debug 체력줄 지정에서 최대 HP * healthBar / 전체 체력줄 수를 내림해 HP를 구한다.
		// 유효한 요청의 계산값은 최소 1이며, 0 또는 저작 범위를 벗어난 체력줄과 유효하지 않은 최대값은 0이다.
		// HP가 정수이므로 모든 가능한 설정에서 Calculate_HealthBar의 정확한 역함수인 것은 아니다.
		[[nodiscard]] static std::uint32_t Resolve_HealthBarHp(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t healthBar);

		// 패턴 후보 검사가 현재 갑옷 상태와 저작 조건을 대조한다. 내구도가 남은 갑옷판이 있으면 ARMORED다.
		// 저작된 갑옷판이 없으면 갑옷이 벗겨진 상태로 취급하며, ANY 조건은 항상 통과한다.
		[[nodiscard]] static bool Is_ArmorRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_ARMOR_REQUIREMENT requirement);

		// 패턴 후보 검사가 현재 게임플레이 페이즈와 저작 조건을 대조한다.
		// 이 함수는 상태를 읽기만 한다. 체력줄 임계값에 따른 페이즈 갱신은 Update 쪽의 책임이다.
		[[nodiscard]] static bool Is_PhaseRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_PHASE_REQUIREMENT requirement);

		// GameRoom의 단계 액션 검증이나 적용이 실패하면 현재 패턴 ID를 Fail_Mechanic에 전달한다.
		// 대상 부재로 재시도할 경우와 초기화가 필요한 실패는 전달된 failure에 따라 구분한다.
		static void Fail_ActiveMechanic(
			SERVER_WORLD_ENTITY& boss,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);
		// GameRoom은 특정 패턴 ID의 기믹 실패도 전달할 수 있다.
		// 현재 실행 중인 패턴과 일치하면 임시 높이 보정을 복원하고 중단 영수증을 남긴다.
		// NO_VALID_TARGET은 진행 또는 대기 중인 기믹을 재예약한다. 다른 실패는 초기화 필요 상태로 바꾼다.
		// 해당 기믹 발생 기록이 없으면 기믹 원장은 변경하지 않는다.
		static void Fail_Mechanic(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);

		// 외부 디버그 조회가 순환 버퍼에서 현재 보관 중인 기록 수를 확인한다.
		[[nodiscard]] std::size_t Get_DecisionTraceCount() const noexcept
		{
			return m_iDecisionTraceCount;
		}
		// age 0은 보관된 기록 중 가장 오래된 항목이며 범위를 벗어나면 nullptr다.
		// 반환 포인터는 Brain 소유 기록을 차용한다. 새 기록이 쌓이면 내용이 덮어써질 수 있다.
		// 내부 동기화가 없으므로 기록 중인 Update와 동시에 조회하지 않도록 호출자가 접근을 조율한다.
		[[nodiscard]] const VALTAN_DECISION_TRACE* Get_DecisionTrace(
			std::size_t age) const noexcept;
		// 가장 최근 기록을 차용하며 아직 기록이 없으면 nullptr다. 수명과 접근 조건은 위 조회와 같다.
		[[nodiscard]] const VALTAN_DECISION_TRACE*
			Get_LatestDecisionTrace() const noexcept;

		// GameRoom_ValtanSimulation.cpp의 Update_ValtanWorldEntity가 고정 틱마다 호출한다. fixedDeltaSeconds는 초, serverTick은 서버 틱이다.
		// 호출자가 소유한 boss와 players에 패턴 진행, 명중과 행동 상태를 반영하며 피해 이벤트를 뒤에 추가한다.
		// coverCircles는 이번 틱의 엄폐물 XZ 원이다. 적용 대상 공격은 계산된 공격 원점과 플레이어 사이의 가림도 확인한다.
		// catalog는 실행 정의를 제공한다. activeThresholdCatalog는 교체된 체력줄 선택 정의를 제공할 수 있다.
		// 제품 시퀀스가 정의를 고정한 동안에는 그 실행 catalog를 계속 사용한다.
		// 선택적 출력 벡터는 GameRoom이 소유하고 같은 틱의 포획 부착과 오브젝트 피격 후속 처리에서 소비한다.
		// automaticSequenceOverride는 외부에서 지정한 시퀀스다. 포인터 입력과 출력은 이 호출 동안만 차용한다.
		// 보스가 아니면 즉시 반환한다. 사망 시에는 진행 중 패턴과 이동을 정리하므로 실패 시 항상 무변경인 API는 아니다.
		void Update(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
				coverCircles,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const CGameplayCatalog* activeThresholdCatalog = nullptr,
			std::uint16_t activeThresholdGenerationEpoch = 1u,
			std::vector<SERVER_PLAYER_CAPTURE_REQUEST>* outCaptureRequests =
				nullptr,
			const BOSS_PATTERN_SEQUENCE_DEFINITION*
				automaticSequenceOverride = nullptr,
			std::vector<SERVER_BOSS_PATTERN_HIT>* outOwnerHits = nullptr) const;
		// GameRoom이 내비게이션 검증 전에 다음 월드 XZ 위치를 요청한다. 좌표는 m, 시간은 초다.
		// 명시적인 단계 이동 또는 루트 모션 차분으로 후보만 계산하며 boss 자체는 움직이지 않는다.
		// 점프와 아레나 횡단처럼 별도 경로가 위치를 소유하는 단계는 여기서 중복 이동시키지 않는다.
		// false일 때 출력은 사용할 수 없다. 마지막 유한성 검사 전 출력에 값을 쓴 경로도 있다.
		bool Try_BuildStageMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		// 현재 계약 테스트가 돌진의 다음 월드 XZ 후보를 확인할 때 호출한다. 속도와 고정 틱 시간으로 계산한다.
		// boss는 변경하지 않으며 false인 경우 출력은 사용하지 않는다. 이 함수 자체는 충돌과 단계 종료를 판단하지 않는다.
		bool Try_BuildImpactMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		// GameRoom이 실제 벽 접촉을 확인한 뒤 WALL_CONTACT 분기를 적용한다.
		// 대응 단계와 분기가 없으면 false다. 분기를 처리하면 돌진 속도를 0으로 만들고 true를 반환한다.
		// 잘못된 다음 정의를 중단 처리한 경우도 true이므로 패턴 성공 여부는 보스 상태와 종료 영수증으로 확인한다.
		bool Complete_ImpactStage(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;

		// GameRoom이 이동 불가를 확인한 뒤 호출한다. 포획 인원 조건에 맞는 분기를 우선 처리한다.
		// 포획 분기가 있으면 그 처리 결과를 바로 반환하고, 없으면 NAVIGATION_BLOCKED 분기를 처리한다.
		// true는 분기가 처리됐다는 뜻이다. 잘못된 다음 정의에 따른 중단도 포함하므로 패턴 성공 여부와 구분한다.
		bool Complete_NavigationBlockedStage(
			SERVER_WORLD_ENTITY& boss,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;
		// GameRoom의 유령 포털 반복 진행에서 기존 고정 정의 revision으로 패턴을 다시 시작한다.
		// 해당 기믹인지, 단계와 생존 HP가 있는지, 틱과 패턴 순번이 유효한지 확인한 후 상태를 갱신한다.
		bool Restart_FinaleCycle(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const BOSS_PATTERN_DEFINITION& pattern,
			std::uint32_t serverTick) const;
		// Brain의 단계 진입과 GameRoom의 단계 액션이 포털 이동 상태를 준비할 때 호출한다.
		// 이전 포털 상태를 먼저 초기화한다. 포털 단계에서는 이동 경로도 비운다.
		// 아레나 횡단은 시작 위치와 방향까지 즉시 설정하므로 단순한 읽기 전용 설정 조회가 아니다.
		static void Configure_PortalMotion(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_DEFINITION& stage);
		// GameRoom의 단계 시작에서 마지막 유효 대상을 향한 돌진 경로와 방향을 한 번 고정한다.
		// 이미 고정됐으면 true다. 아직 고정되지 않았고 입력이 유효하지 않으면 상태 변경 전 false를 반환한다.
		static bool Lock_PortalTargetRushAtStageStart(
			SERVER_WORLD_ENTITY& boss);

	private:
		// Update의 판단 결과를 다음 슬롯으로 이동하고 기록 순번을 부여한다. 포화 시 가장 오래된 기록을 교체한다.
		void Record_DecisionTrace(VALTAN_DECISION_TRACE&& trace) const;

		// const Update에서도 관측 기록은 쌓는다. 보스의 게임플레이 상태는 외부에 있고 이 버퍼만 Brain이 소유한다.
		// 쓰기 위치는 다음 교체 슬롯, 개수는 현재 유효 기록 수이며 다음 순번은 넘칠 때 1부터 다시 시작한다.
		mutable std::array<VALTAN_DECISION_TRACE, MAX_DECISION_TRACE_COUNT>
			m_DecisionTraces{};
		mutable std::size_t m_iDecisionTraceWriteIndex = 0u;
		mutable std::size_t m_iDecisionTraceCount = 0u;
		mutable std::uint64_t m_iNextDecisionTraceSequence = 1u;
	};
}
