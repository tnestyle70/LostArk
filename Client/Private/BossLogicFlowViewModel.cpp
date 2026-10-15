#include "BossLogicFlowView.h"

#include "ValtanPatternTree.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>

namespace
{
	// 노드/간선이 원본의 stable ID와 맞지 않으면 오류를 기록한다. 임시 view만 폐기하고 기존 표시 view는 유지한다.
	bool FailIdentity(
		Client::ACTION_COMPOSITION_GRAPH_ERROR& OutError,
		const Client::VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strStageId,
		const std::string& strActionId,
		std::string strMessage)
	{
		OutError.eCode = Client::ACTION_COMPOSITION_GRAPH_ERROR_CODE::
			INVALID_ACTION_ID;
		OutError.strMessage = std::move(strMessage);
		OutError.strPatternId = Pattern.strPatternId;
		OutError.strStageId = strStageId;
		OutError.strActionId = strActionId;
		OutError.strOutcome.clear();
		return false;
	}

	// counterable flag가 ENTER 1, EXIT 0의 정확히 두 action으로 닫히는지 진단한다. 존재 여부도 별도 출력해 불완전 표시를 구분한다.
	bool HasClosedCounterableFlag(
		const Client::VALTAN_STAGE_VIEW& Stage,
		bool& bOutHasAnyCounterableFlag)
	{
		std::size_t iTotal = 0u;
		std::size_t iEnter = 0u;
		std::size_t iExit = 0u;
		for (const Client::VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
		{
			if ("SET_BOSS_FLAG" != Action.strKind ||
				"boss.flag.counterable" != Action.strTargetId)
			{
				continue;
			}
			++iTotal;
			if ("ENTER" == Action.strTrigger && 1.f == Action.fValue)
				++iEnter;
			if ("EXIT" == Action.strTrigger && 0.f == Action.fValue)
				++iExit;
		}
		bOutHasAnyCounterableFlag = 0u != iTotal;
		return 2u == iTotal && 1u == iEnter && 1u == iExit;
	}

	// action/pattern 양쪽 동시 지정과 빈 target을 거부한다. 둘 다 없는 terminal은 허용하며 외부 Pattern 존재는 조회하지 않는다.
	bool HasUnambiguousBranchTarget(
		const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
	{
		const bool bHasAction = Branch.strNextActionId.has_value();
		const bool bHasPattern = Branch.strNextPatternId.has_value();
		return !(bHasAction && bHasPattern) &&
			(!bHasAction || !Branch.strNextActionId->empty()) &&
			(!bHasPattern || !Branch.strNextPatternId->empty());
	}

	// 조건 설명 숫자를 소수 두 자리로 표시하고 뒤의 0을 뺀다. 원본 숫자와 저장 데이터를 수정하지 않는다.
	std::string FormatScalar(const float Value)
	{
		std::ostringstream Stream;
		Stream << std::fixed << std::setprecision(2) << Value;
		std::string Result = Stream.str();
		while (!Result.empty() && '0' == Result.back())
			Result.pop_back();
		if (!Result.empty() && '.' == Result.back())
			Result.pop_back();
		return Result.empty() ? "0" : Result;
	}

	// 같은 문구를 중복 표시하지 않도록 이미 만든 요약 목록을 확인한다. 조건 실행이나 정규화된 저장 ID 생성은 아니다.
	void AppendCondition(
		Client::BOSS_LOGIC_FLOW_NODE_VIEW& Node,
		std::string Summary)
	{
		if (!Summary.empty() && Node.ConditionSummaries.end() ==
			std::find(Node.ConditionSummaries.begin(),
				Node.ConditionSummaries.end(), Summary))
		{
			Node.ConditionSummaries.push_back(std::move(Summary));
		}
	}

	// Stage의 proxy/response/bind/silence/파괴/타격 필드를 표시 문구로 만든다.
	// 이 목록은 구현이 선택한 필드의 요약이며 모든 Server 조건을 실행하거나 완전하게 열거하는 validator가 아니다.
	void ProjectConditionSummaries(
		const Client::VALTAN_STAGE_VIEW& Stage,
		Client::BOSS_LOGIC_FLOW_NODE_VIEW& Node)
	{
		if (Stage.CounterProxy.has_value())
		{
			const Client::VALTAN_COUNTER_PROXY_VIEW& Proxy = *Stage.CounterProxy;
			std::string Summary = "Counter proxy: " + Proxy.strKind;
			if ("BOSS_FORWARD_ARC" == Proxy.strKind)
				Summary += " " + FormatScalar(Proxy.fArcDegrees) + " deg";
			else
			{
				Summary += " r=" + FormatScalar(Proxy.fRadiusM) +
					" m offset=(" + FormatScalar(Proxy.fForwardOffsetM) +
					", " + FormatScalar(Proxy.fRightOffsetM) + ") m";
			}
			AppendCondition(Node, std::move(Summary));
		}
		if (Stage.BossResponse.has_value())
		{
			const Client::VALTAN_BOSS_RESPONSE_VIEW& Response =
				*Stage.BossResponse;
			AppendCondition(Node, "Boss response: " + Response.strKind +
				" >= " + std::to_string(Response.iThreshold) + " damage");
		}
		for (const Client::VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
		{
			if ("ENTER" != Action.strTrigger || 0.f == Action.fValue)
				continue;
			if ("SET_PLAYER_BIND" == Action.strKind)
			{
				AppendCondition(Node, "Bind: +" +
					FormatScalar(Action.fValue / 1000.f) + " m / " +
					std::to_string(Action.iDurationMs) + " ms (" +
					Action.strTargetId + ")");
			}
			else if ("SET_PLAYER_SILENCE" == Action.strKind)
			{
				AppendCondition(Node, "Silence: " +
					std::to_string(Action.iDurationMs) + " ms (" +
					Action.strTargetId + ")");
			}
		}
		if ("NORMAL" != Stage.strPartDamagePolicy)
			AppendCondition(Node, "Part damage: " + Stage.strPartDamagePolicy);
		for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if ("PART_DESTROYED" == Branch.strOutcome)
				AppendCondition(Node, "Branch condition: PART_DESTROYED");
		}
		if (0u != Stage.iHitCount ||
			(!Stage.strHitShape.empty() && "NONE" != Stage.strHitShape))
		{
			std::string Summary = "Hit: " + Stage.strHitShape + " x" +
				std::to_string(Stage.iHitCount) + " delay=" +
				std::to_string(Stage.iHitDelayMs) + " ms";
			if (Stage.iHitCount > 1u)
				Summary += " interval=" +
					std::to_string(Stage.iHitIntervalMs) + " ms";
			if (!Stage.strServerDamageProfileId.empty())
				Summary += " | " + Stage.strServerDamageProfileId;
			AppendCondition(Node, std::move(Summary));
		}
	}

	// 유효 tick/occurrence와 Pattern/action이 현재 view에 존재하는지 확인한다. 호출자가 전달한 복제 입력 자체를 재수신하지 않는다.
	bool IsAdmissibleSnapshot(
		const Client::BOSS_LOGIC_FLOW_LIVE_SNAPSHOT& Snapshot,
		const Client::BOSS_LOGIC_FLOW_VIEW* const pView)
	{
		if (!Snapshot.bValid || 0u == Snapshot.iServerTick ||
			0u == Snapshot.iPatternSequence || Snapshot.strPatternId.empty() ||
			Snapshot.strActionId.empty() || nullptr == pView ||
			pView->strPatternId != Snapshot.strPatternId ||
			pView->Graph.strPatternId != Snapshot.strPatternId)
		{
			return false;
		}
		return pView->Nodes.end() != std::find_if(
			pView->Nodes.begin(), pView->Nodes.end(),
			[&Snapshot](const Client::BOSS_LOGIC_FLOW_NODE_VIEW& Node)
			{
				return Node.strActionId == Snapshot.strActionId;
			});
	}

	// tick/sequence의 uint32 wrap에서 예약값 0을 건너뛰어 다음 값을 구한다.
	std::uint32_t NextNonzero(const std::uint32_t Value)
	{
		return (std::numeric_limits<std::uint32_t>::max)() == Value ?
			1u : Value + 1u;
	}

	// 정확히 다음 유효 tick인지 검사한다. 중간 snapshot이 빠졌다면 outcome을 추론하지 않는다.
	bool IsConsecutiveTick(
		const std::uint32_t Previous,
		const std::uint32_t Current)
	{
		return 0u != Previous && Current == NextNonzero(Previous);
	}
}

// Sequence UI badge용으로 COUNTER_HIT 이름이 한 번이라도 있는지만 확인한다. 더 엄격한 닫힌 Counter 진단과 별개다.
bool Client::CBossLogicFlowViewModel::Has_CounterHitBranch(
	const VALTAN_PATTERN_VIEW& Pattern) noexcept
{
	return std::any_of(
		Pattern.Stages.begin(), Pattern.Stages.end(),
		[](const VALTAN_STAGE_VIEW& Stage)
		{
			return std::any_of(
				Stage.Branches.begin(), Stage.Branches.end(),
				[](const VALTAN_STAGE_BRANCH_VIEW& Branch)
				{
					return "COUNTER_HIT" == Branch.strOutcome;
				});
		});
}

// ValtanBossTool이 원본 Pattern을 표시하려고 호출한다. 기본 그래프와 부가 설명을 임시 view에 만든 뒤 한 번 교체한다.
// 명시적 false는 InOutView를 보존하지만 OutError는 실패 위치로 바꾼다.
bool Client::CBossLogicFlowViewModel::Project(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::uint64_t iSourceGeneration,
	BOSS_LOGIC_FLOW_VIEW& InOutView,
	ACTION_COMPOSITION_GRAPH_ERROR& OutError)
{
	BOSS_LOGIC_FLOW_VIEW Staged;
	if (!CActionCompositionGraphModel::Project(
			Pattern, iSourceGeneration, Staged.Graph, OutError))
	{
		return false;
	}

	Staged.strPatternId = Pattern.strPatternId;
	Staged.strPatternDisplayName = Pattern.strDisplayName;
	Staged.bHasCounterHitBranch = Has_CounterHitBranch(Pattern);
	Staged.Nodes.reserve(Staged.Graph.Nodes.size());
	for (std::size_t iNode = 0u; iNode < Staged.Graph.Nodes.size(); ++iNode)
	{
		const ACTION_COMPOSITION_GRAPH_NODE& GraphNode =
			Staged.Graph.Nodes[iNode];
		if (GraphNode.iCanonicalStageIndex >= Pattern.Stages.size())
		{
			return FailIdentity(
				OutError, Pattern, GraphNode.Key.strStageId,
				GraphNode.Key.strActionId,
				"Boss Logic Flow graph node no longer resolves to a canonical Stage.");
		}
		// canonical index와 Pattern/Stage/action ID를 함께 대조해 같은 원본 행인지 확인한다.
		const VALTAN_STAGE_VIEW& Stage =
			Pattern.Stages[GraphNode.iCanonicalStageIndex];
		if (GraphNode.Key.strPatternId != Pattern.strPatternId ||
			GraphNode.Key.strStageId != Stage.strStageId ||
			GraphNode.Key.strActionId != Stage.strActionId)
		{
			return FailIdentity(
				OutError, Pattern, Stage.strStageId, Stage.strActionId,
				"Boss Logic Flow graph node identity does not match its immutable Pattern Stage.");
		}

		BOSS_LOGIC_FLOW_NODE_VIEW Node;
		Node.iGraphNodeIndex = iNode;
		Node.strStageId = Stage.strStageId;
		Node.strActionId = Stage.strActionId;
		Node.strSequenceRole = Stage.strSequenceRole;
		Node.strStageKind = Stage.strStageKind;
		Node.ClipNames.reserve(Stage.ClipOccurrences.size());
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
			Node.ClipNames.push_back(Clip.strClipName);
		ProjectConditionSummaries(Stage, Node);

		// COUNTER_HIT/TIMEOUT 한 개씩, 모호하지 않은 target, 닫힌 flag 쌍을 Counter window 표시 조건으로 사용한다.
		// CounterProxy 존재 여부는 독립 표시이며 window 판정의 필수 조건으로 사용하지 않는다.
		std::size_t iCounterHitCount = 0u;
		std::size_t iTimeoutCount = 0u;
		bool bCounterTargetsResolve = true;
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if ("COUNTER_HIT" == Branch.strOutcome)
			{
				++iCounterHitCount;
				bCounterTargetsResolve = bCounterTargetsResolve &&
					HasUnambiguousBranchTarget(Branch);
			}
			else if ("TIMEOUT" == Branch.strOutcome)
			{
				++iTimeoutCount;
				bCounterTargetsResolve = bCounterTargetsResolve &&
					HasUnambiguousBranchTarget(Branch);
			}
		}
		bool bHasAnyCounterableFlag = false;
		const bool bClosedCounterableFlag =
			HasClosedCounterableFlag(Stage, bHasAnyCounterableFlag);
		Node.bCounterWindow = 1u == iCounterHitCount && 1u == iTimeoutCount &&
			bCounterTargetsResolve && bClosedCounterableFlag;
		Node.bCounterProxy = Stage.CounterProxy.has_value();
		const bool bHasCounterContractSignal =
			0u != iCounterHitCount || bHasAnyCounterableFlag;
		Node.bCounterContractIncomplete =
			bHasCounterContractSignal && !Node.bCounterWindow;
		Staged.Nodes.push_back(std::move(Node));
	}

	// 간선별 authored 여부와 Counter 표시를 계산한다. 유도 TIMEOUT은 authored Counter 실패 표시로 취급하지 않는다.
	Staged.Edges.reserve(Staged.Graph.Edges.size());
	for (std::size_t iEdge = 0u; iEdge < Staged.Graph.Edges.size(); ++iEdge)
	{
		const ACTION_COMPOSITION_GRAPH_EDGE& GraphEdge =
			Staged.Graph.Edges[iEdge];
		if (GraphEdge.iSourceNodeIndex >= Staged.Nodes.size())
		{
			return FailIdentity(
				OutError, Pattern, {}, GraphEdge.strSourceActionId,
				"Boss Logic Flow edge no longer resolves to its source Stage node.");
		}
		BOSS_LOGIC_FLOW_EDGE_VIEW Edge;
		Edge.iGraphEdgeIndex = iEdge;
		Edge.bAuthored = ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::
			AUTHORED_BRANCH == GraphEdge.eOrigin;
		Edge.bCounterSuccess = Edge.bAuthored &&
			"COUNTER_HIT" == GraphEdge.strOutcome;
		Edge.bCounterTimeout = Edge.bAuthored &&
			"TIMEOUT" == GraphEdge.strOutcome &&
			Staged.Nodes[GraphEdge.iSourceNodeIndex].bCounterWindow;
		Staged.Edges.push_back(Edge);
	}

	InOutView = std::move(Staged);
	OutError.Clear();
	return true;
}

// Renderer가 새 선택을 내보내기 전에 종류, stable ID와 분기 속성을 함께 초기화한다.
void Client::BOSS_LOGIC_FLOW_SELECTION::Clear()
{
	eKind = BOSS_LOGIC_FLOW_SELECTION_KIND::NONE;
	strPatternId.clear();
	strStageId.clear();
	strActionId.clear();
	strOutcome.clear();
	strTargetActionId.clear();
	strTargetPatternId.clear();
	bTerminal = false;
	bAuthored = false;
}

// ValtanBossTool의 HUD 관찰 경로가 호출한다. 반환 true는 유일한 전이를 새 history로 기록했다는 뜻이다.
// 출력은 먼저 비우고, 기록 불가능한 경우도 다음 비교를 위한 anchor를 바꿀 수 있다.
bool Client::CBossLogicFlowObservedEdgeResolver::Observe(
	const BOSS_LOGIC_FLOW_LIVE_SNAPSHOT& Snapshot,
	const BOSS_LOGIC_FLOW_VIEW* const pCurrentView,
	BOSS_LOGIC_FLOW_OBSERVED_EDGE* const pOutObserved)
{
	if (nullptr != pOutObserved)
		*pOutObserved = {};
	if (!IsAdmissibleSnapshot(Snapshot, pCurrentView))
	{
		Clear_Anchor();
		return false;
	}

	// 전달 view/snapshot을 값으로 복사해 다음 호출까지 소유한다. 원래 view 포인터는 저장하지 않는다.
	const auto StageCurrent = [this, &Snapshot, pCurrentView]()
	{
		m_PreviousSnapshot = Snapshot;
		m_PreviousView = *pCurrentView;
		m_bHasAnchor = true;
	};
	if (!m_bHasAnchor)
	{
		StageCurrent();
		return false;
	}

	// 과거 tick은 기존 anchor를 그대로 두고 무시한다. 같은 tick에서 identity가 바뀌면 새 anchor만 저장한다.
	const std::int32_t TickDelta = static_cast<std::int32_t>(
		Snapshot.iServerTick - m_PreviousSnapshot.iServerTick);
	if (TickDelta < 0)
		return false;
	const bool bSameIdentity =
		Snapshot.iPatternSequence == m_PreviousSnapshot.iPatternSequence &&
		Snapshot.strPatternId == m_PreviousSnapshot.strPatternId &&
		Snapshot.strActionId == m_PreviousSnapshot.strActionId;
	if (0 == TickDelta)
	{
		if (!bSameIdentity)
			StageCurrent();
		return false;
	}
	if (bSameIdentity)
	{
		StageCurrent();
		return false;
	}
	// identity가 바뀐 두 snapshot은 연속 tick과 같은 source generation이어야 비교한다. 아니면 현재 값으로 anchor만 갱신한다.
	if (!IsConsecutiveTick(
			m_PreviousSnapshot.iServerTick, Snapshot.iServerTick) ||
		m_PreviousView.Graph.iSourceGeneration !=
			pCurrentView->Graph.iSourceGeneration)
	{
		StageCurrent();
		return false;
	}

	// 같은 occurrence 안의 action 변경 또는 다음 sequence의 다른 Pattern 진입만 후보로 삼는다.
	const bool bLocalTransition =
		Snapshot.iPatternSequence == m_PreviousSnapshot.iPatternSequence &&
		Snapshot.strPatternId == m_PreviousSnapshot.strPatternId;
	const bool bCrossPatternTransition =
		Snapshot.iPatternSequence == NextNonzero(
			m_PreviousSnapshot.iPatternSequence) &&
		Snapshot.strPatternId != m_PreviousSnapshot.strPatternId;
	if (!bLocalTransition && !bCrossPatternTransition)
	{
		StageCurrent();
		return false;
	}

	// 이전 view의 source action에서 현재 cursor로 맞는 간선이 정확히 하나여야 한다. 두 개가 맞으면 어느 outcome인지 추정하지 않는다.
	const ACTION_COMPOSITION_GRAPH_EDGE* pMatched = nullptr;
	for (const ACTION_COMPOSITION_GRAPH_EDGE& Edge :
		m_PreviousView.Graph.Edges)
	{
		if (Edge.strSourceActionId != m_PreviousSnapshot.strActionId)
			continue;
		const bool bMatches = bLocalTransition ?
			(!Edge.bTerminal && Edge.strTargetActionId == Snapshot.strActionId) :
			(Edge.bTerminal &&
			 Edge.strTargetPatternId == Snapshot.strPatternId);
		if (!bMatches)
			continue;
		if (nullptr != pMatched)
		{
			StageCurrent();
			return false;
		}
		pMatched = &Edge;
	}
	if (nullptr == pMatched ||
		pMatched->iSourceNodeIndex >= m_PreviousView.Nodes.size())
	{
		StageCurrent();
		return false;
	}

	const BOSS_LOGIC_FLOW_NODE_VIEW& SourceNode =
		m_PreviousView.Nodes[pMatched->iSourceNodeIndex];
	// 이전 source와 현재 target cursor를 값으로 복사한다. 순번이 넘치면 1로 돌아가고 history는 오래된 것부터 제거한다.
	BOSS_LOGIC_FLOW_OBSERVED_EDGE Observed;
	Observed.iObservationSequence = m_iNextObservationSequence;
	Observed.iSourceGeneration = m_PreviousView.Graph.iSourceGeneration;
	Observed.iSourceServerTick = m_PreviousSnapshot.iServerTick;
	Observed.iTargetServerTick = Snapshot.iServerTick;
	Observed.iSourcePatternSequence =
		m_PreviousSnapshot.iPatternSequence;
	Observed.iTargetPatternSequence = Snapshot.iPatternSequence;
	Observed.strSourcePatternId = m_PreviousSnapshot.strPatternId;
	Observed.strSourceStageId = SourceNode.strStageId;
	Observed.strSourceActionId = pMatched->strSourceActionId;
	Observed.strOutcome = pMatched->strOutcome;
	Observed.strTargetActionId = pMatched->strTargetActionId;
	Observed.strTargetPatternId = pMatched->strTargetPatternId;
	Observed.bTerminal = pMatched->bTerminal;
	Observed.bCrossPattern = bCrossPatternTransition;
	if (m_History.size() == MAX_HISTORY_COUNT)
		m_History.erase(m_History.begin());
	m_History.push_back(Observed);
	m_iNextObservationSequence =
		(std::numeric_limits<std::uint64_t>::max)() ==
			m_iNextObservationSequence ?
		1u : m_iNextObservationSequence + 1u;
	if (nullptr != pOutObserved)
		*pOutObserved = Observed;
	StageCurrent();
	return true;
}

// 사용자의 Clear history 등 명시적 초기화다. anchor뿐 아니라 history와 다음 관찰 번호도 초기화한다.
void Client::CBossLogicFlowObservedEdgeResolver::Reset()
{
	Clear_Anchor();
	m_History.clear();
	m_iNextObservationSequence = 1u;
}

// 무효 snapshot이 오면 비교 기준만 비운다. 이미 확정한 표시 history는 유지한다.
void Client::CBossLogicFlowObservedEdgeResolver::Clear_Anchor()
{
	m_PreviousSnapshot = {};
	m_PreviousView = {};
	m_bHasAnchor = false;
}
