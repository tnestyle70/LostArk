#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_VIEW;

	// 현재 snapshot 안에 대응 노드/간선이 없다는 표시다. index는 이 snapshot의 vector에만 유효하다.
	inline constexpr std::size_t ACTION_COMPOSITION_GRAPH_INVALID_INDEX =
		(std::numeric_limits<std::size_t>::max)();

	// pan/zoom 적용 전 2D 그래프 좌표다. 화면 좌표 변환은 Workbench/Renderer가 담당한다.
	struct ACTION_COMPOSITION_GRAPH_POINT final
	{
		float fX = 0.f;
		float fY = 0.f;
	};

	// 노드 선택과 전체 그래프 맞춤에 쓰는 경계다. Contains는 네 변 위의 점도 포함한다.
	struct ACTION_COMPOSITION_GRAPH_RECT final
	{
		float fLeft = 0.f;
		float fTop = 0.f;
		float fRight = 0.f;
		float fBottom = 0.f;

		bool Contains(const ACTION_COMPOSITION_GRAPH_POINT& Point) const noexcept;
	};

	// 패턴, stage, action의 stable 문자열을 함께 보관해 원본의 같은 행인지 대조한다.
	struct ACTION_COMPOSITION_GRAPH_NODE_KEY final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;

		bool operator==(const ACTION_COMPOSITION_GRAPH_NODE_KEY& Right) const noexcept;
	};

	// 원본 Stage의 표시 정보와 개수, 도달 여부, 배치 결과를 값으로 소유한다. 원본 포인터는 보관하지 않는다.
	struct ACTION_COMPOSITION_GRAPH_NODE final
	{
		ACTION_COMPOSITION_GRAPH_NODE_KEY Key;
		std::string strSequenceRole;
		std::string strStageKind;
		std::uint32_t iDurationMs = 0u;
		std::size_t iCanonicalStageIndex = 0u;
		std::size_t iGraphDepth = 0u;
		std::size_t iGameplayActionCount = 0u;
		std::size_t iBranchCount = 0u;
		std::size_t iAnimationOccurrenceCount = 0u;
		std::size_t iEffectReferenceCount = 0u;
		std::size_t iCameraInvocationCount = 0u;
		bool bEventEntered = false;
		bool bReachable = false;
		ACTION_COMPOSITION_GRAPH_RECT Bounds;
	};

	// 저작 분기와 TIMEOUT이 없을 때 계산한 기본 연결을 구분해 UI가 원본 수정 가능 범위를 판단한다.
	enum class ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN : std::uint8_t
	{
		AUTHORED_BRANCH,
		DERIVED_TIMEOUT,
	};

	struct ACTION_COMPOSITION_GRAPH_EDGE final
	{
		std::string strPatternId;
		std::string strSourceActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		// 다른 Pattern으로 나가는 분기는 이 Pattern 그래프의 끝으로 표시한다.
		// 대상 stable ID는 읽기 전용 검사에 남기며 대상 Pattern의 존재는 이 투영에서 조회하지 않는다.
		std::string strTargetPatternId;
		std::size_t iSourceNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::size_t iTargetNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		// 저작 분기의 원래 Stage 내 순번을 보관한다. UI가 클릭을 처리할 때 stable source ID와 함께 다시 대조한다.
		// 이 순번 자체를 영구 저장 ID로 사용하지 않는다.
		std::size_t iSourceBranchIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN eOrigin =
			ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::AUTHORED_BRANCH;
		bool bTerminal = false;
		bool bDefault = false;
		std::vector<ACTION_COMPOSITION_GRAPH_POINT> Polyline;
	};

	// snapshot 내부 index 목록과 stage 기간 합(ms)이다. 조건이 발생한 실제 시각이나 다른 Pattern의 시간은 포함하지 않는다.
	struct ACTION_COMPOSITION_GRAPH_PATH final
	{
		std::vector<std::size_t> NodeIndices;
		std::vector<std::size_t> EdgeIndices;
		std::uint64_t iDurationMs = 0u;
		bool bTerminal = false;
		// 표시 경로는 허용된 재시도 간선에서 한 번 멈추고 반복 표시를 남긴다. Server의 실제 반복 횟수를 계산한 값은 아니다.
		bool bRepeats = false;
	};

	// Stage별 미리보기 분기 선택이다. 입력 Pattern을 바꾸지 않고 SelectedPath를 만드는 데만 사용한다.
	struct ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE final
	{
		std::string strActionId;
		std::string strOutcome;
	};

	// 노드/간선/경로/도형을 독립 값으로 소유한다. source generation은 호출자가 준 식별값이며 모델이 freshness를 확인하는 hash는 아니다.
	struct ACTION_COMPOSITION_GRAPH_SNAPSHOT final
	{
		std::uint64_t iSourceGeneration = 0u;
		std::string strPatternId;
		std::size_t iEntryNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::vector<ACTION_COMPOSITION_GRAPH_NODE> Nodes;
		std::vector<ACTION_COMPOSITION_GRAPH_EDGE> Edges;
		ACTION_COMPOSITION_GRAPH_PATH DefaultPath;
		ACTION_COMPOSITION_GRAPH_PATH SelectedPath;
		ACTION_COMPOSITION_GRAPH_PATH MaximumPath;
		ACTION_COMPOSITION_GRAPH_RECT GraphBounds;
	};

	enum class ACTION_COMPOSITION_GRAPH_ERROR_CODE : std::uint8_t
	{
		NONE,
		INVALID_PATTERN_ID,
		INVALID_PATTERN_ACTION_ID,
		EMPTY_STAGE_SET,
		INVALID_STAGE_ID,
		DUPLICATE_STAGE_ID,
		INVALID_ACTION_ID,
		DUPLICATE_ACTION_ID,
		INVALID_ENTRY_ACTION_ID,
		ENTRY_IS_NOT_FIRST_ACTION,
		INVALID_OUTCOME,
		DUPLICATE_OUTCOME,
		AMBIGUOUS_BRANCH_TARGET,
		INVALID_TARGET_PATTERN_ID,
		DANGLING_TARGET_ACTION,
		DUPLICATE_OUTCOME_OVERRIDE,
		UNKNOWN_OVERRIDE_ACTION,
		UNKNOWN_OVERRIDE_OUTCOME,
		CYCLE,
		DURATION_OVERFLOW,
	};

	// 실패 위치의 stable ID와 사유를 별도 출력한다. snapshot 보존과 달리 오류 출력은 호출마다 갱신한다.
	struct ACTION_COMPOSITION_GRAPH_ERROR final
	{
		ACTION_COMPOSITION_GRAPH_ERROR_CODE eCode =
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::NONE;
		std::string strMessage;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strOutcome;

		void Clear();
	};

	// 한 Pattern의 값을 읽어 검증, 위상 정렬, 표시 경로와 도형을 만든다.
	// 허용된 Trash 재시도 간선은 위상 정렬에서 제외한다. 명시적 false는 이전 출력 snapshot을 보존한다.
	class CActionCompositionGraphModel final
	{
	public:
		// Workbench는 선택 분기를 전달하고 BossLogicFlowViewModel은 기본 경로 overload를 쓴다. 파일 저장이나 Server 명령은 없다.
		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>&
				OutcomeOverrides,
			ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);

		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);

		// 그래프 좌표의 점을 뒤쪽 노드부터 검사한다. 실패하면 INVALID_INDEX를 반환한다.
		static std::size_t Hit_TestNode(
			const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
			const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint) noexcept;

		// 꺾은선과 점 사이 최소 거리를 허용 오차와 비교한다. 오차도 그래프 단위이므로 UI가 zoom을 보정해 전달한다.
		static std::size_t Hit_TestEdge(
			const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
			const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint,
			float fTolerance = 6.f) noexcept;
	};
}
