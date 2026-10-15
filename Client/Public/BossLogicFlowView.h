#pragma once

#include "ActionCompositionGraphModel.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_VIEW;

	// 원본 Pattern 그래프 노드에 clip 이름과 Counter 표시 진단을 덧붙인 읽기 전용 값이다.
	// 실제 gameplay 분기 정의와 실행은 이 표시 모델이 소유하지 않는다.
	struct BOSS_LOGIC_FLOW_NODE_VIEW final
	{
		std::size_t iGraphNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::string strStageId;
		std::string strActionId;
		std::string strSequenceRole;
		std::string strStageKind;
		std::vector<std::string> ClipNames;
		// 저작 조건의 사람이 읽는 요약이다. proxy/response/status/hit 필드를 표시할 뿐 이 문자열로 gameplay를 실행하지 않는다.
		std::vector<std::string> ConditionSummaries;
		bool bCounterWindow = false;
		bool bCounterProxy = false;
		bool bCounterContractIncomplete = false;
	};

	// Graph.Edges와 같은 순서의 표시 속성이다. authored COUNTER_HIT 및 닫힌 Counter의 TIMEOUT 색상 등을 결정한다.
	struct BOSS_LOGIC_FLOW_EDGE_VIEW final
	{
		std::size_t iGraphEdgeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		bool bAuthored = false;
		bool bCounterSuccess = false;
		bool bCounterTimeout = false;
	};

	struct BOSS_LOGIC_FLOW_VIEW final
	{
		std::string strPatternId;
		std::string strPatternDisplayName;
		// 어느 Stage든 정확한 COUNTER_HIT 분기가 있으면 true다. Sequence UI badge용 사실이며 아래의 더 엄격한 노드 Counter 조합 진단과 구분한다.
		bool bHasCounterHitBranch = false;
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Graph;
		std::vector<BOSS_LOGIC_FLOW_NODE_VIEW> Nodes;
		std::vector<BOSS_LOGIC_FLOW_EDGE_VIEW> Edges;
	};

	// 원본을 바꾸지 않는 값 변환이다. 임시 view를 완성해 마지막에 교체하며 명시적 false는 이전 InOutView를 보존한다.
	class CBossLogicFlowViewModel final
	{
	public:
		// 분기 이름 존재만 확인한다. counterable ENTER/EXIT 쌍이나 target의 유효성 검증은 Project와 원본 admission이 담당한다.
		[[nodiscard]] static bool Has_CounterHitBranch(
			const VALTAN_PATTERN_VIEW& Pattern) noexcept;
		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			BOSS_LOGIC_FLOW_VIEW& InOutView,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);
	};

	enum class BOSS_LOGIC_FLOW_SELECTION_KIND : std::uint8_t
	{
		NONE,
		STAGE,
		BRANCH,
	};

	// 캔버스 선택을 소유자에게 stable 문자열로 반환한다. 화면 내부 포인터나 vector index를 선택의 영구 식별자로 넘기지 않는다.
	struct BOSS_LOGIC_FLOW_SELECTION final
	{
		BOSS_LOGIC_FLOW_SELECTION_KIND eKind =
			BOSS_LOGIC_FLOW_SELECTION_KIND::NONE;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bAuthored = false;

		void Clear();
	};

	// Client가 이미 복제받은 보스 cursor에서 뽑은 관찰 입력이다. 네트워크를 별도로 읽거나 두 번째 gameplay 실행 상태를 만들지 않는다.
	struct BOSS_LOGIC_FLOW_LIVE_SNAPSHOT final
	{
		bool bValid = false;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequence = 0u;
		std::string strPatternId;
		std::string strActionId;
	};

	// 연속된 두 authoritative cursor와 일치하는 유일한 그래프 간선으로 추론한 표시 기록이다.
	// Server가 보낸 outcome 영수증이 아니며 gameplay 입력이나 Pattern 선택 명령으로 사용하지 않는다.
	struct BOSS_LOGIC_FLOW_OBSERVED_EDGE final
	{
		std::uint64_t iObservationSequence = 0u;
		std::uint64_t iSourceGeneration = 0u;
		std::uint32_t iSourceServerTick = 0u;
		std::uint32_t iTargetServerTick = 0u;
		std::uint32_t iSourcePatternSequence = 0u;
		std::uint32_t iTargetPatternSequence = 0u;
		std::string strSourcePatternId;
		std::string strSourceStageId;
		std::string strSourceActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bCrossPattern = false;
	};

	// 이미 승인된 Client snapshot을 관찰한다. 입력이 무효하면 anchor를 비우고, tick 공백/모호한 전이는 기록 없이 현재 snapshot을 새 anchor로 삼는다.
	// 과거 tick은 버리고 최대 32개 기록만 소유한다. 관찰 실패가 기존 history까지 지우지는 않는다.
	class CBossLogicFlowObservedEdgeResolver final
	{
	public:
		static constexpr std::size_t MAX_HISTORY_COUNT = 32u;

		// ValtanBossTool이 HUD의 복제 cursor와 같은 source generation의 view를 전달한다. view는 값으로 복사하며 포인터를 보관하지 않는다.
		// 출력 포인터가 있으면 매 호출 처음 비운다. false도 anchor를 변경할 수 있으며 true만 history에 새 기록을 추가한다.
		bool Observe(
			const BOSS_LOGIC_FLOW_LIVE_SNAPSHOT& Snapshot,
			const BOSS_LOGIC_FLOW_VIEW* pCurrentView,
			BOSS_LOGIC_FLOW_OBSERVED_EDGE* pOutObserved = nullptr);
		// anchor, history와 관찰 번호를 함께 초기화한다. 일반 Observe 실패와 달리 이 명시적 호출은 과거 기록도 지운다.
		void Reset();
		// 반환 참조의 소유자는 이 resolver다. Observe/Reset 뒤에는 원소 포인터와 iterator를 계속 사용하지 않는다.
		[[nodiscard]] const std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE>&
			Get_History() const noexcept
		{
			return m_History;
		}

	private:
		void Clear_Anchor();

		// 다음 전이를 대조할 anchor를 독립 값으로 소유한다. 내부 동기화는 없으므로 같은 인스턴스의 갱신/조회는 호출자가 직렬화한다.
		BOSS_LOGIC_FLOW_LIVE_SNAPSHOT m_PreviousSnapshot;
		BOSS_LOGIC_FLOW_VIEW m_PreviousView;
		std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE> m_History;
		std::uint64_t m_iNextObservationSequence = 1u;
		bool m_bHasAnchor = false;
	};

	// UI가 소유하는 pan/zoom과 Fit 요청이다. Pattern/source generation 변경 시 Renderer가 다시 맞춤을 요청한다.
	struct BOSS_LOGIC_FLOW_CANVAS_STATE final
	{
		std::string strPatternId;
		std::uint64_t iSourceGeneration = ~std::uint64_t{ 0u };
		float fPanX = 0.f;
		float fPanY = 0.f;
		float fZoom = 1.f;
		bool bFitRequested = true;
	};

	// string_view와 pObservedEdge는 Render 호출 동안만 차용한다. 데이터 수명은 호출자가 유지한다.
	struct BOSS_LOGIC_FLOW_RENDER_CONTEXT final
	{
		std::string_view strSelectedStageId;
		std::string_view strLiveActionId;
		const BOSS_LOGIC_FLOW_OBSERVED_EDGE* pObservedEdge = nullptr;
		bool bLivePattern = false;
		bool bAllowSelection = false;
		float fMinimumCanvasHeight = 320.f;
	};

	// Valtan Boss Tool이 live identity를 전달하는 ImGui 읽기 전용 canvas다. 클릭 선택은 명시적으로 허용한 경우만 반환한다.
	// 네트워크나 gameplay를 직접 변경하지 않으며 true는 stable 선택 한 건을 출력했다는 뜻이다.
	class CBossLogicFlowRenderer final
	{
	public:
		static bool Render(
			const BOSS_LOGIC_FLOW_VIEW& View,
			const BOSS_LOGIC_FLOW_RENDER_CONTEXT& Context,
			BOSS_LOGIC_FLOW_CANVAS_STATE& InOutState,
			BOSS_LOGIC_FLOW_SELECTION& OutSelection);
	};
}
