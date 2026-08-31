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
	constexpr float GRAPH_MARGIN = 32.f;
	constexpr float NODE_WIDTH = 240.f;
	constexpr float NODE_HEIGHT = 104.f;
	constexpr float NODE_COLUMN_GAP = 120.f;
	constexpr float NODE_ROW_GAP = 52.f;
	constexpr float TERMINAL_EDGE_LENGTH = 72.f;

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

	bool Is_EventEnteredStage(const std::string_view StageKind)
	{
		return "GROGGY" == StageKind || "PART_BREAK" == StageKind;
	}

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

	void Expand_Bounds(
		Client::ACTION_COMPOSITION_GRAPH_RECT& Bounds,
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Point)
	{
		Bounds.fLeft = (std::min)(Bounds.fLeft, Point.fX);
		Bounds.fTop = (std::min)(Bounds.fTop, Point.fY);
		Bounds.fRight = (std::max)(Bounds.fRight, Point.fX);
		Bounds.fBottom = (std::max)(Bounds.fBottom, Point.fY);
	}

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
			iNode = Edge.iTargetNodeIndex;
		}
		return true;
	}

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

bool Client::ACTION_COMPOSITION_GRAPH_RECT::Contains(
	const ACTION_COMPOSITION_GRAPH_POINT& Point) const noexcept
{
	return Point.fX >= fLeft && Point.fX <= fRight &&
		Point.fY >= fTop && Point.fY <= fBottom;
}

bool Client::ACTION_COMPOSITION_GRAPH_NODE_KEY::operator==(
	const ACTION_COMPOSITION_GRAPH_NODE_KEY& Right) const noexcept
{
	return strPatternId == Right.strPatternId &&
		strStageId == Right.strStageId && strActionId == Right.strActionId;
}

void Client::ACTION_COMPOSITION_GRAPH_ERROR::Clear()
{
	eCode = ACTION_COMPOSITION_GRAPH_ERROR_CODE::NONE;
	strMessage.clear();
	strPatternId.clear();
	strStageId.clear();
	strActionId.clear();
	strOutcome.clear();
}

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
			if (!Branch.strNextActionId.has_value())
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
			/* A manual audition Pattern is an ordered authoring chain: its
			   implicit default edge is always the immediate vector-next Stage,
			   including WINDUP -> GROGGY -> ACTIVE.  Canonical encounter graphs
			   retain their event-entered fallback rule; an explicit authored
			   TIMEOUT above always owns its exact target in both modes. */
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

	std::vector<std::size_t> Indegrees(Staged.Nodes.size(), 0u);
	for (const ACTION_COMPOSITION_GRAPH_EDGE& Edge : Staged.Edges)
	{
		if (!Edge.bTerminal)
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
			if (Edge.bTerminal)
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
			"Pattern graph contains a cycle; v1 supports bounded DAG paths only.",
			Pattern.strPatternId,
			iCycleNode < Staged.Nodes.size() ?
				Staged.Nodes[iCycleNode].Key.strStageId : std::string{},
			iCycleNode < Staged.Nodes.size() ?
				Staged.Nodes[iCycleNode].Key.strActionId : std::string{});
	}

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
			const std::uint64_t iChildDuration = Edge.bTerminal ? 0u :
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
		iMaximumNode = Staged.Edges[iEdge].iTargetNodeIndex;
	}
	Staged.MaximumPath.iDurationMs =
		MaximumDuration[Staged.iEntryNodeIndex];

	for (const std::size_t iNode : TopologicalOrder)
	{
		for (const std::size_t iEdge : OutgoingEdges[iNode])
		{
			const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Staged.Edges[iEdge];
			if (!Edge.bTerminal)
			{
				Staged.Nodes[Edge.iTargetNodeIndex].iGraphDepth = (std::max)(
					Staged.Nodes[Edge.iTargetNodeIndex].iGraphDepth,
					Staged.Nodes[iNode].iGraphDepth + 1u);
			}
		}
	}
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

	InOutSnapshot = std::move(Staged);
	OutError.Clear();
	return true;
}

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
