#include "ActionCompositionGraphModel.h"

#include "ValtanPatternTree.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string_view>
#include <utility>

namespace
{
	// pan/zoom 전 그래프의 고정 배치 단위다. 실제 화면 확대/이동은 소비자에서 적용한다.
	constexpr float GRAPH_MARGIN = 32.f;
	constexpr float NODE_WIDTH = 240.f;
	constexpr float NODE_HEIGHT = 104.f;
	constexpr float NODE_COLUMN_GAP = 120.f;
	constexpr float NODE_ROW_GAP = 52.f;
	constexpr float TERMINAL_EDGE_LENGTH = 72.f;

	// 입력 stable ID의 길이와 ASCII 문자 집합을 검사한다. 외부 asset이나 다른 Pattern의 존재 확인은 아니다.
	bool Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				'_' != Character && '-' != Character && '.' != Character)
			{
				return false;
			}
		}
		return true;
	}

	// 기본 TIMEOUT 경로가 자동 진입하지 않는 GROGGY/PART_BREAK를 구분한다. 수동 audition은 이 건너뛰기를 쓰지 않는다.
	bool Is_EventEnteredStage(const std::string_view StageKind)
	{
		return "GROGGY" == StageKind || "PART_BREAK" == StageKind;
	}

	// 오류 위치를 값으로 채우고 false를 반환한다. 호출자가 만드는 임시 snapshot 외 기존 화면은 여기서 바꾸지 않는다.
	bool Fail(
		Client::ACTION_COMPOSITION_GRAPH_ERROR& OutError,
		const Client::ACTION_COMPOSITION_GRAPH_ERROR_CODE eCode,
		std::string strMessage,
		const std::string& strPatternId = {},
		const std::string& strStageId = {},
		const std::string& strActionId = {},
		const std::string& strOutcome = {})
	{
		OutError.eCode = eCode;
		OutError.strMessage = std::move(strMessage);
		OutError.strPatternId = strPatternId;
		OutError.strStageId = strStageId;
		OutError.strActionId = strActionId;
		OutError.strOutcome = strOutcome;
		return false;
	}

	// 노드 사각형과 간선 점을 포함하도록 전체 그래프 범위를 넓힌다. Renderer의 Fit 계산에 사용한다.
	void Expand_Bounds(
		Client::ACTION_COMPOSITION_GRAPH_RECT& Bounds,
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Point)
	{
		Bounds.fLeft = (std::min)(Bounds.fLeft, Point.fX);
		Bounds.fTop = (std::min)(Bounds.fTop, Point.fY);
		Bounds.fRight = (std::max)(Bounds.fRight, Point.fX);
		Bounds.fBottom = (std::max)(Bounds.fBottom, Point.fY);
	}

	// 바로 앞 점과 같은 좌표만 생략해 길이 0인 인접 선분을 줄인다. 전체 경로의 중복 점을 제거하는 함수는 아니다.
	void Push_UniquePoint(
		std::vector<Client::ACTION_COMPOSITION_GRAPH_POINT>& Points,
		const Client::ACTION_COMPOSITION_GRAPH_POINT Point)
	{
		if (!Points.empty() && Points.back().fX == Point.fX &&
			Points.back().fY == Point.fY)
		{
			return;
		}
		Points.push_back(Point);
	}

	// stage 기간(ms)을 uint64 합에 더하기 전 overflow를 검사한다. false이면 기존 합은 유지한다.
	bool Add_Duration(
		std::uint64_t& InOutDuration,
		const std::uint32_t iDurationMs)
	{
		const std::uint64_t Duration = iDurationMs;
		if (InOutDuration >
			(std::numeric_limits<std::uint64_t>::max)() - Duration)
		{
			return false;
		}
		InOutDuration += Duration;
		return true;
	}

	// 한 노드의 TIMEOUT 기본 간선을 찾는다. 앞선 구성 단계가 기본 간선을 하나씩 보장한다.
	std::size_t Find_DefaultEdge(
		const Client::ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
		const std::vector<std::vector<std::size_t>>& OutgoingEdges,
		const std::size_t iNodeIndex)
	{
		for (const std::size_t iEdge : OutgoingEdges[iNodeIndex])
		{
			if (Snapshot.Edges[iEdge].bDefault)
				return iEdge;
		}
		return Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
	}

	// 선택된 outcome과 같은 표시 간선을 찾는다. 없는 선택은 Project의 override 검증에서 거부한다.
	std::size_t Find_OutcomeEdge(
		const Client::ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
		const std::vector<std::vector<std::size_t>>& OutgoingEdges,
		const std::size_t iNodeIndex,
		const std::string_view Outcome)
	{
		for (const std::size_t iEdge : OutgoingEdges[iNodeIndex])
		{
			if (Snapshot.Edges[iEdge].strOutcome == Outcome)
				return iEdge;
		}
		return Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
	}

	// 지정한 Trash 패턴/Stage의 TIMEOUT 재진입만 허용한다. 이 간선은 위상 정렬에서 빼고 경로에는 반복 끝점으로 남긴다.
	bool Is_CounterRetryEdge(
		const Client::ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
		const Client::ACTION_COMPOSITION_GRAPH_EDGE& Edge)
	{
		if (Edge.bTerminal || Edge.strOutcome != "TIMEOUT" ||
			(Snapshot.strPatternId != "VALTAN_TRASH" &&
			 Snapshot.strPatternId != "VALTAN_TRASH_CATCH_IF"))
			return false;
		const auto& Source = Snapshot.Nodes[Edge.iSourceNodeIndex];
		return Source.iBranchCount == 1u &&
			(Source.Key.strStageId == "RETRY_EXHAUSTED" ||
			 Source.Key.strStageId == "CATCH_SLAM" || Source.Key.strStageId == "EXECUTE_TAIL") &&
			Snapshot.Nodes[Edge.iTargetNodeIndex].Key.strStageId == "RECHARGE_WAIT_02";
	}

	// entry부터 선택 outcome 또는 기본 TIMEOUT을 따라 한 표시 경로를 만든다. 재시도 간선은 반복 표시 후 멈춘다.
	// OutPath는 처음에 비우며 도중 실패 시 부분 결과가 남지만 Project의 임시 snapshot 안에서만 사용한다.
	bool Build_Path(
		const Client::ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
		const std::vector<std::vector<std::size_t>>& OutgoingEdges,
		const std::map<std::string, std::string, std::less<>>* pOverrides,
		Client::ACTION_COMPOSITION_GRAPH_PATH& OutPath)
	{
		OutPath = {};
		std::size_t iNode = Snapshot.iEntryNodeIndex;
		while (Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iNode)
		{
			OutPath.NodeIndices.push_back(iNode);
			if (!Add_Duration(
					OutPath.iDurationMs, Snapshot.Nodes[iNode].iDurationMs))
			{
				return false;
			}

			std::size_t iEdge = Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
			if (nullptr != pOverrides)
			{
				const auto Override = pOverrides->find(
					Snapshot.Nodes[iNode].Key.strActionId);
				if (Override != pOverrides->end())
				{
					iEdge = Find_OutcomeEdge(
						Snapshot, OutgoingEdges, iNode, Override->second);
				}
			}
			if (Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX == iEdge)
				iEdge = Find_DefaultEdge(Snapshot, OutgoingEdges, iNode);
			if (Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX == iEdge)
				return false;

			OutPath.EdgeIndices.push_back(iEdge);
			const Client::ACTION_COMPOSITION_GRAPH_EDGE& Edge =
				Snapshot.Edges[iEdge];
			if (Edge.bTerminal)
			{
				OutPath.bTerminal = true;
				break;
			}
			if (Is_CounterRetryEdge(Snapshot, Edge))
			{
				OutPath.bRepeats = true;
				break;
			}
			iNode = Edge.iTargetNodeIndex;
		}
		return true;
	}

	// 점의 선분 투영 비율을 0~1로 제한한 뒤 제곱 거리를 구한다. 제곱 오차와 비교하므로 제곱근 계산이 필요 없다.
	float Point_SegmentDistanceSquared(
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Point,
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Start,
		const Client::ACTION_COMPOSITION_GRAPH_POINT& End) noexcept
	{
		const float SegmentX = End.fX - Start.fX;
		const float SegmentY = End.fY - Start.fY;
		const float LengthSquared = SegmentX * SegmentX + SegmentY * SegmentY;
		if (LengthSquared <= 0.f)
		{
			const float DeltaX = Point.fX - Start.fX;
			const float DeltaY = Point.fY - Start.fY;
			return DeltaX * DeltaX + DeltaY * DeltaY;
		}
		const float Projection = (std::max)(0.f, (std::min)(1.f,
			((Point.fX - Start.fX) * SegmentX +
				(Point.fY - Start.fY) * SegmentY) / LengthSquared));
		const float ClosestX = Start.fX + Projection * SegmentX;
		const float ClosestY = Start.fY + Projection * SegmentY;
		const float DeltaX = Point.fX - ClosestX;
		const float DeltaY = Point.fY - ClosestY;
		return DeltaX * DeltaX + DeltaY * DeltaY;
	}
}

// 경계 포함 사각형 검사다. 호출자는 화면 점을 먼저 그래프 좌표로 변환한다.
bool Client::ACTION_COMPOSITION_GRAPH_RECT::Contains(
	const ACTION_COMPOSITION_GRAPH_POINT& Point) const noexcept
{
	return Point.fX >= fLeft && Point.fX <= fRight &&
		Point.fY >= fTop && Point.fY <= fBottom;
}

// vector 위치가 같아도 stable ID가 바뀌면 다른 노드로 취급한다.
bool Client::ACTION_COMPOSITION_GRAPH_NODE_KEY::operator==(
	const ACTION_COMPOSITION_GRAPH_NODE_KEY& Right) const noexcept
{
	return strPatternId == Right.strPatternId &&
		strStageId == Right.strStageId && strActionId == Right.strActionId;
}

// 이전 실패 식별자와 문구를 함께 비운다. 성공한 투영에서도 호출해 오래된 오류가 남지 않게 한다.
void Client::ACTION_COMPOSITION_GRAPH_ERROR::Clear()
{
	eCode = ACTION_COMPOSITION_GRAPH_ERROR_CODE::NONE;
	strMessage.clear();
	strPatternId.clear();
	strStageId.clear();
	strActionId.clear();
	strOutcome.clear();
}

// BossLogicFlowViewModel 등이 분기 override 없이 기본 표시를 요청하는 진입점이다. 빈 목록의 수명은 정적이다.
bool Client::CActionCompositionGraphModel::Project(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::uint64_t iSourceGeneration,
	ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
	ACTION_COMPOSITION_GRAPH_ERROR& OutError)
{
	static const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>
		NoOverrides;
	return Project(Pattern, iSourceGeneration, NoOverrides,
		InOutSnapshot, OutError);
}

// 원본을 변경하지 않고 임시 snapshot을 완성한 뒤 마지막에 교체한다. 명시적 false는 기존 snapshot을 보존하고 OutError만 갱신한다.
bool Client::CActionCompositionGraphModel::Project(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::uint64_t iSourceGeneration,
	const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>& OutcomeOverrides,
	ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
	ACTION_COMPOSITION_GRAPH_ERROR& OutError)
{
	OutError.Clear();
	if (!Is_StableToken(Pattern.strPatternId))
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_PATTERN_ID,
			"Pattern graph requires one stable patternId.",
			Pattern.strPatternId);
	}
	if (!Is_StableToken(Pattern.strActionId))
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_PATTERN_ACTION_ID,
			"Pattern graph requires one stable pattern actionId.",
			Pattern.strPatternId, {}, Pattern.strActionId);
	}
	if (Pattern.Stages.empty())
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::EMPTY_STAGE_SET,
			"Pattern graph requires at least one Stage.",
			Pattern.strPatternId);
	}

	// Stage ID의 중복은 set, action→노드 연결은 map으로 검증한다. 원본 vector 순서는 canonical stage index로 유지한다.
	ACTION_COMPOSITION_GRAPH_SNAPSHOT Staged;
	Staged.iSourceGeneration = iSourceGeneration;
	Staged.strPatternId = Pattern.strPatternId;
	Staged.Nodes.reserve(Pattern.Stages.size());
	std::map<std::string, std::size_t, std::less<>> NodeByActionId;
	std::set<std::string, std::less<>> StageIds;
	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!Is_StableToken(Stage.strStageId))
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_STAGE_ID,
				"Pattern graph Stage has an invalid stable stageId.",
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId);
		}
		if (!StageIds.insert(Stage.strStageId).second)
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_STAGE_ID,
				"Pattern graph contains a duplicate stageId.",
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId);
		}
		if (!Is_StableToken(Stage.strActionId))
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_ACTION_ID,
				"Pattern graph Stage has an invalid stable actionId.",
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId);
		}
		if (!NodeByActionId.emplace(Stage.strActionId, iStage).second)
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_ACTION_ID,
				"Pattern graph contains a duplicate Stage actionId.",
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId);
		}

		ACTION_COMPOSITION_GRAPH_NODE Node;
		Node.Key.strPatternId = Pattern.strPatternId;
		Node.Key.strStageId = Stage.strStageId;
		Node.Key.strActionId = Stage.strActionId;
		Node.strSequenceRole = Stage.strSequenceRole;
		Node.strStageKind = Stage.strStageKind;
		Node.iDurationMs = Stage.iDurationMs;
		Node.iCanonicalStageIndex = iStage;
		Node.iGameplayActionCount = Stage.Actions.size();
		Node.iBranchCount = Stage.Branches.size();
		Node.iAnimationOccurrenceCount = Stage.ClipOccurrences.size();
		Node.iEffectReferenceCount = Stage.ProductCues.size() +
			Stage.Effects.size() + Stage.CombatObjectEffects.size() +
			Stage.IndependentEffectIds.size();
		Node.iCameraInvocationCount = Stage.CameraInvocations.size();
		Node.bEventEntered = Is_EventEnteredStage(Stage.strStageKind);
		Staged.Nodes.push_back(std::move(Node));
	}

	if (!Is_StableToken(Pattern.strEntryActionId) ||
		NodeByActionId.end() == NodeByActionId.find(Pattern.strEntryActionId))
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_ENTRY_ACTION_ID,
			"Pattern graph entryActionId does not resolve to one Stage.",
			Pattern.strPatternId, {}, Pattern.strEntryActionId);
	}
	Staged.iEntryNodeIndex = NodeByActionId.at(Pattern.strEntryActionId);
	if (0u != Staged.iEntryNodeIndex)
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::ENTRY_IS_NOT_FIRST_ACTION,
			"Pattern graph entryActionId is not the first canonical Stage action.",
			Pattern.strPatternId,
			Pattern.Stages[Staged.iEntryNodeIndex].strStageId,
			Pattern.strEntryActionId);
	}

	// 각 노드의 outgoing 간선 index를 따로 모아 경로 탐색과 위상 정렬에서 같은 연결을 사용한다.
	std::vector<std::vector<std::size_t>> OutgoingEdges(Staged.Nodes.size());
	Staged.Edges.reserve(Pattern.Stages.size() * 2u);
	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		std::set<std::string, std::less<>> Outcomes;
		bool bHasTimeout = false;
		for (std::size_t iBranch = 0u;
			iBranch < Stage.Branches.size(); ++iBranch)
		{
			const VALTAN_STAGE_BRANCH_VIEW& Branch = Stage.Branches[iBranch];
			if (!Is_StableToken(Branch.strOutcome))
			{
				return Fail(OutError,
					ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_OUTCOME,
					"Pattern graph branch has an invalid stable outcome.",
					Pattern.strPatternId, Stage.strStageId,
					Stage.strActionId, Branch.strOutcome);
			}
			if (!Outcomes.insert(Branch.strOutcome).second)
			{
				return Fail(OutError,
					ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_OUTCOME,
					"Pattern graph Stage contains a duplicate branch outcome.",
					Pattern.strPatternId, Stage.strStageId,
					Stage.strActionId, Branch.strOutcome);
			}

			ACTION_COMPOSITION_GRAPH_EDGE Edge;
			Edge.strPatternId = Pattern.strPatternId;
			Edge.strSourceActionId = Stage.strActionId;
			Edge.strOutcome = Branch.strOutcome;
			Edge.iSourceNodeIndex = iStage;
			Edge.iSourceBranchIndex = iBranch;
			Edge.eOrigin = ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::AUTHORED_BRANCH;
			Edge.bDefault = "TIMEOUT" == Branch.strOutcome;
			bHasTimeout = bHasTimeout || Edge.bDefault;
			if (Branch.strNextActionId.has_value() &&
				Branch.strNextPatternId.has_value())
			{
				return Fail(OutError,
					ACTION_COMPOSITION_GRAPH_ERROR_CODE::AMBIGUOUS_BRANCH_TARGET,
					"Pattern graph branch cannot target a local action and another Pattern.",
					Pattern.strPatternId, Stage.strStageId,
					Stage.strActionId, Branch.strOutcome);
			}
			if (Branch.strNextPatternId.has_value())
			{
				Edge.strTargetPatternId = *Branch.strNextPatternId;
				if (!Is_StableToken(Edge.strTargetPatternId))
				{
					return Fail(OutError,
						ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_TARGET_PATTERN_ID,
						"Pattern graph branch targets an invalid Pattern identity.",
						Pattern.strPatternId, Stage.strStageId,
						Stage.strActionId, Branch.strOutcome);
				}
				// 한 Pattern만 투영하므로 대상 Pattern ID만 남기고 현재 경로는 끝낸다. 대상의 첫 Stage는 별도 Pattern view가 담당한다.
				Edge.bTerminal = true;
			}
			else if (!Branch.strNextActionId.has_value())
			{
				Edge.bTerminal = true;
			}
			else
			{
				Edge.strTargetActionId = *Branch.strNextActionId;
				const auto Target = NodeByActionId.find(Edge.strTargetActionId);
				if (!Is_StableToken(Edge.strTargetActionId) ||
					Target == NodeByActionId.end())
				{
					return Fail(OutError,
						ACTION_COMPOSITION_GRAPH_ERROR_CODE::DANGLING_TARGET_ACTION,
						"Pattern graph branch targets an unknown Stage actionId.",
						Pattern.strPatternId, Stage.strStageId,
						Edge.strTargetActionId, Branch.strOutcome);
				}
				Edge.iTargetNodeIndex = Target->second;
			}
			OutgoingEdges[iStage].push_back(Staged.Edges.size());
			Staged.Edges.push_back(std::move(Edge));
		}

		if (!bHasTimeout)
		{
			ACTION_COMPOSITION_GRAPH_EDGE Edge;
			Edge.strPatternId = Pattern.strPatternId;
			Edge.strSourceActionId = Stage.strActionId;
			Edge.strOutcome = "TIMEOUT";
			Edge.iSourceNodeIndex = iStage;
			Edge.eOrigin = ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::DERIVED_TIMEOUT;
			Edge.bDefault = true;
			std::size_t iNextStage = iStage + 1u;
			// 수동 audition의 기본 연결은 바로 다음 vector Stage다. 일반 패턴은 이벤트 진입 Stage를 건너뛴다.
			// 명시적으로 저작한 TIMEOUT이 있으면 이 보충 간선을 만들지 않는다.
			while (!Pattern.bManualServerAudition &&
				iNextStage < Pattern.Stages.size() &&
				Is_EventEnteredStage(Pattern.Stages[iNextStage].strStageKind))
			{
				++iNextStage;
			}
			if (iNextStage == Pattern.Stages.size())
			{
				Edge.bTerminal = true;
			}
			else
			{
				Edge.iTargetNodeIndex = iNextStage;
				Edge.strTargetActionId = Pattern.Stages[iNextStage].strActionId;
			}
			OutgoingEdges[iStage].push_back(Staged.Edges.size());
			Staged.Edges.push_back(std::move(Edge));
		}
	}

	// 동일 action의 중복 선택이나 실제 없는 outcome은 허용하지 않는다. 선택 값은 표시 경로에만 반영한다.
	std::map<std::string, std::string, std::less<>> SelectedOutcomes;
	for (const ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE& Override :
		OutcomeOverrides)
	{
		if (!SelectedOutcomes.emplace(
				Override.strActionId, Override.strOutcome).second)
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_OUTCOME_OVERRIDE,
				"Pattern graph received duplicate outcome overrides for one action.",
				Pattern.strPatternId, {}, Override.strActionId,
				Override.strOutcome);
		}
		const auto Node = NodeByActionId.find(Override.strActionId);
		if (!Is_StableToken(Override.strActionId) || Node == NodeByActionId.end())
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::UNKNOWN_OVERRIDE_ACTION,
				"Pattern graph outcome override targets an unknown actionId.",
				Pattern.strPatternId, {}, Override.strActionId,
				Override.strOutcome);
		}
		if (!Is_StableToken(Override.strOutcome) ||
			ACTION_COMPOSITION_GRAPH_INVALID_INDEX == Find_OutcomeEdge(
				Staged, OutgoingEdges, Node->second, Override.strOutcome))
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::UNKNOWN_OVERRIDE_OUTCOME,
				"Pattern graph action has no requested outcome edge.",
				Pattern.strPatternId, Pattern.Stages[Node->second].strStageId,
				Override.strActionId, Override.strOutcome);
		}
	}

	// Kahn 위상 정렬이다. terminal과 허용된 retry는 차수에서 제외하고 남은 모든 노드의 순환 여부를 검사한다.
	// Ready를 정렬 set으로 두어 동시에 가능한 노드는 canonical index가 작은 순서로 처리한다.
	std::vector<std::size_t> Indegrees(Staged.Nodes.size(), 0u);
	for (const ACTION_COMPOSITION_GRAPH_EDGE& Edge : Staged.Edges)
	{
		if (!Edge.bTerminal && !Is_CounterRetryEdge(Staged, Edge))
			++Indegrees[Edge.iTargetNodeIndex];
	}
	std::set<std::size_t> Ready;
	for (std::size_t iNode = 0u; iNode < Indegrees.size(); ++iNode)
	{
		if (0u == Indegrees[iNode])
			Ready.insert(iNode);
	}
	std::vector<std::size_t> TopologicalOrder;
	TopologicalOrder.reserve(Staged.Nodes.size());
	while (!Ready.empty())
	{
		const std::size_t iNode = *Ready.begin();
		Ready.erase(Ready.begin());
		TopologicalOrder.push_back(iNode);
		for (const std::size_t iEdge : OutgoingEdges[iNode])
		{
			const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Staged.Edges[iEdge];
			if (Edge.bTerminal || Is_CounterRetryEdge(Staged, Edge))
				continue;
			if (0u == --Indegrees[Edge.iTargetNodeIndex])
				Ready.insert(Edge.iTargetNodeIndex);
		}
	}
	if (TopologicalOrder.size() != Staged.Nodes.size())
	{
		std::size_t iCycleNode = 0u;
		while (iCycleNode < Indegrees.size() && 0u == Indegrees[iCycleNode])
			++iCycleNode;
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::CYCLE,
			"Pattern graph contains a cycle outside the authored Trash counter retry edges.",
			Pattern.strPatternId,
			iCycleNode < Staged.Nodes.size() ?
				Staged.Nodes[iCycleNode].Key.strStageId : std::string{},
			iCycleNode < Staged.Nodes.size() ?
				Staged.Nodes[iCycleNode].Key.strActionId : std::string{});
	}

	// entry에서 도달 가능한 노드를 stack으로 표시한다. retry도 방문 표시를 이용해 한 번만 확장한다.
	std::vector<std::size_t> ReachableStack{ Staged.iEntryNodeIndex };
	Staged.Nodes[Staged.iEntryNodeIndex].bReachable = true;
	while (!ReachableStack.empty())
	{
		const std::size_t iNode = ReachableStack.back();
		ReachableStack.pop_back();
		for (const std::size_t iEdge : OutgoingEdges[iNode])
		{
			const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Staged.Edges[iEdge];
			if (Edge.bTerminal || Staged.Nodes[Edge.iTargetNodeIndex].bReachable)
				continue;
			Staged.Nodes[Edge.iTargetNodeIndex].bReachable = true;
			ReachableStack.push_back(Edge.iTargetNodeIndex);
		}
	}

	if (!Build_Path(Staged, OutgoingEdges, nullptr, Staged.DefaultPath) ||
		!Build_Path(Staged, OutgoingEdges, &SelectedOutcomes,
			Staged.SelectedPath))
	{
		return Fail(OutError,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::DURATION_OVERFLOW,
			"Pattern graph path duration overflowed uint64 milliseconds.",
			Pattern.strPatternId);
	}

	// 위상 순서의 역순으로 가장 긴 자식 경로 시간을 누적한다. retry는 이후 시간을 0으로 보고 한 표시 회차에서 멈춘다.
	// 기간이 같으면 먼저 만난 outgoing 간선을 유지하므로 저작 순서가 동점 선택을 결정한다.
	std::vector<std::uint64_t> MaximumDuration(Staged.Nodes.size(), 0u);
	std::vector<std::size_t> MaximumEdge(
		Staged.Nodes.size(), ACTION_COMPOSITION_GRAPH_INVALID_INDEX);
	for (auto Order = TopologicalOrder.rbegin();
		Order != TopologicalOrder.rend(); ++Order)
	{
		const std::size_t iNode = *Order;
		std::uint64_t iBestChildDuration = 0u;
		for (const std::size_t iEdge : OutgoingEdges[iNode])
		{
			const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Staged.Edges[iEdge];
			const std::uint64_t iChildDuration =
				(Edge.bTerminal || Is_CounterRetryEdge(Staged, Edge)) ? 0u :
				MaximumDuration[Edge.iTargetNodeIndex];
			if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX == MaximumEdge[iNode] ||
				iChildDuration > iBestChildDuration)
			{
				iBestChildDuration = iChildDuration;
				MaximumEdge[iNode] = iEdge;
			}
		}
		MaximumDuration[iNode] = iBestChildDuration;
		if (!Add_Duration(
				MaximumDuration[iNode], Staged.Nodes[iNode].iDurationMs))
		{
			return Fail(OutError,
				ACTION_COMPOSITION_GRAPH_ERROR_CODE::DURATION_OVERFLOW,
				"Pattern graph maximum path duration overflowed uint64 milliseconds.",
				Pattern.strPatternId, Staged.Nodes[iNode].Key.strStageId,
				Staged.Nodes[iNode].Key.strActionId);
		}
	}

	// 위에서 기록한 최장 간선을 entry부터 따라 실제 표시할 index 목록을 복원한다.
	std::size_t iMaximumNode = Staged.iEntryNodeIndex;
	while (ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iMaximumNode)
	{
		Staged.MaximumPath.NodeIndices.push_back(iMaximumNode);
		const std::size_t iEdge = MaximumEdge[iMaximumNode];
		if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX == iEdge)
			break;
		Staged.MaximumPath.EdgeIndices.push_back(iEdge);
		if (Staged.Edges[iEdge].bTerminal)
		{
			Staged.MaximumPath.bTerminal = true;
			break;
		}
		if (Is_CounterRetryEdge(Staged, Staged.Edges[iEdge]))
		{
			Staged.MaximumPath.bRepeats = true;
			break;
		}
		iMaximumNode = Staged.Edges[iEdge].iTargetNodeIndex;
	}
	Staged.MaximumPath.iDurationMs =
		MaximumDuration[Staged.iEntryNodeIndex];

	// 재시도 간선을 제외한 최장 선행 깊이를 열 번호로 사용한다. 도달 불가 노드도 원본 확인을 위해 배치한다.
	for (const std::size_t iNode : TopologicalOrder)
	{
		for (const std::size_t iEdge : OutgoingEdges[iNode])
		{
			const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Staged.Edges[iEdge];
			if (!Edge.bTerminal && !Is_CounterRetryEdge(Staged, Edge))
			{
				Staged.Nodes[Edge.iTargetNodeIndex].iGraphDepth = (std::max)(
					Staged.Nodes[Edge.iTargetNodeIndex].iGraphDepth,
					Staged.Nodes[iNode].iGraphDepth + 1u);
			}
		}
	}
	// 같은 깊이의 노드는 canonical 순서대로 아래에 쌓는다. 겹침 최적화나 전역 최단 배선을 수행하지는 않는다.
	std::map<std::size_t, std::size_t> RowByDepth;
	for (ACTION_COMPOSITION_GRAPH_NODE& Node : Staged.Nodes)
	{
		const std::size_t iRow = RowByDepth[Node.iGraphDepth]++;
		Node.Bounds.fLeft = GRAPH_MARGIN + static_cast<float>(Node.iGraphDepth) *
			(NODE_WIDTH + NODE_COLUMN_GAP);
		Node.Bounds.fTop = GRAPH_MARGIN + static_cast<float>(iRow) *
			(NODE_HEIGHT + NODE_ROW_GAP);
		Node.Bounds.fRight = Node.Bounds.fLeft + NODE_WIDTH;
		Node.Bounds.fBottom = Node.Bounds.fTop + NODE_HEIGHT;
	}
	Staged.GraphBounds = Staged.Nodes.front().Bounds;
	for (const ACTION_COMPOSITION_GRAPH_NODE& Node : Staged.Nodes)
	{
		Expand_Bounds(Staged.GraphBounds,
			{ Node.Bounds.fLeft, Node.Bounds.fTop });
		Expand_Bounds(Staged.GraphBounds,
			{ Node.Bounds.fRight, Node.Bounds.fBottom });
	}

	// 같은 source의 간선 시작점을 분산하고 직각 꺾은선을 만든다. 뒤로 향하는 연결은 노드 오른쪽 바깥으로 우회한다.
	std::vector<std::size_t> SourceEdgeOrdinal(Staged.Nodes.size(), 0u);
	for (ACTION_COMPOSITION_GRAPH_EDGE& Edge : Staged.Edges)
	{
		const ACTION_COMPOSITION_GRAPH_RECT& Source =
			Staged.Nodes[Edge.iSourceNodeIndex].Bounds;
		const std::size_t iOrdinal = SourceEdgeOrdinal[Edge.iSourceNodeIndex]++;
		const std::size_t iCount = OutgoingEdges[Edge.iSourceNodeIndex].size();
		const float SourceY = Source.fTop + NODE_HEIGHT *
			static_cast<float>(iOrdinal + 1u) / static_cast<float>(iCount + 1u);
		const ACTION_COMPOSITION_GRAPH_POINT Start{ Source.fRight, SourceY };
		Push_UniquePoint(Edge.Polyline, Start);
		if (Edge.bTerminal)
		{
			Push_UniquePoint(Edge.Polyline,
				{ Source.fRight + TERMINAL_EDGE_LENGTH +
					static_cast<float>(iOrdinal) * 12.f, SourceY });
		}
		else
		{
			const ACTION_COMPOSITION_GRAPH_RECT& Target =
				Staged.Nodes[Edge.iTargetNodeIndex].Bounds;
			const ACTION_COMPOSITION_GRAPH_POINT End{
				Target.fLeft, (Target.fTop + Target.fBottom) * 0.5f };
			float RouteX = 0.f;
			if (End.fX > Start.fX + 40.f)
				RouteX = (Start.fX + End.fX) * 0.5f;
			else
			{
				RouteX = (std::max)(Source.fRight, Target.fRight) +
					TERMINAL_EDGE_LENGTH + static_cast<float>(iOrdinal) * 12.f;
			}
			Push_UniquePoint(Edge.Polyline, { RouteX, Start.fY });
			Push_UniquePoint(Edge.Polyline, { RouteX, End.fY });
			Push_UniquePoint(Edge.Polyline, End);
		}
		for (const ACTION_COMPOSITION_GRAPH_POINT& Point : Edge.Polyline)
			Expand_Bounds(Staged.GraphBounds, Point);
	}

	// 검증·경로·배치가 끝난 독립 값을 한 번에 교체한다. 기존 snapshot의 index/참조를 보관한 소비자는 새 generation에 맞춰 다시 조회해야 한다.
	InOutSnapshot = std::move(Staged);
	OutError.Clear();
	return true;
}

// 뒤쪽 노드부터 경계 포함 검사해 하나를 반환한다. 원본 선택의 stable ID 재검증은 Workbench가 담당한다.
std::size_t Client::CActionCompositionGraphModel::Hit_TestNode(
	const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
	const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint) noexcept
{
	for (std::size_t iNode = Snapshot.Nodes.size(); iNode > 0u; --iNode)
	{
		if (Snapshot.Nodes[iNode - 1u].Bounds.Contains(GraphPoint))
			return iNode - 1u;
	}
	return ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
}

// 허용 오차 안의 가장 가까운 선분을 찾는다. 같은 거리면 나중에 검사한 간선을 선택하며 잘못된 오차는 거부한다.
std::size_t Client::CActionCompositionGraphModel::Hit_TestEdge(
	const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
	const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint,
	const float fTolerance) noexcept
{
	if (!std::isfinite(fTolerance) || fTolerance < 0.f)
		return ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
	const float ToleranceSquared = fTolerance * fTolerance;
	float ClosestDistanceSquared = ToleranceSquared;
	std::size_t iClosestEdge = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
	for (std::size_t iEdge = 0u; iEdge < Snapshot.Edges.size(); ++iEdge)
	{
		const std::vector<ACTION_COMPOSITION_GRAPH_POINT>& Points =
			Snapshot.Edges[iEdge].Polyline;
		for (std::size_t iPoint = 1u; iPoint < Points.size(); ++iPoint)
		{
			const float DistanceSquared = Point_SegmentDistanceSquared(
				GraphPoint, Points[iPoint - 1u], Points[iPoint]);
			if (DistanceSquared <= ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				iClosestEdge = iEdge;
			}
		}
	}
	return iClosestEdge;
}
