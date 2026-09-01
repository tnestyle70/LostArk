#include "BossLogicFlowView.h"

#include "ValtanPatternTree.h"

#include <algorithm>
#include <string_view>
#include <utility>

namespace
{
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
}

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

		std::size_t iCounterHitCount = 0u;
		std::size_t iTimeoutCount = 0u;
		bool bCounterTargetsResolve = true;
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if ("COUNTER_HIT" == Branch.strOutcome)
			{
				++iCounterHitCount;
				bCounterTargetsResolve = bCounterTargetsResolve &&
					Branch.strNextActionId.has_value();
			}
			else if ("TIMEOUT" == Branch.strOutcome)
			{
				++iTimeoutCount;
				bCounterTargetsResolve = bCounterTargetsResolve &&
					Branch.strNextActionId.has_value();
			}
		}
		bool bHasAnyCounterableFlag = false;
		const bool bClosedCounterableFlag =
			HasClosedCounterableFlag(Stage, bHasAnyCounterableFlag);
		Node.bCounterWindow = "WINDUP" == Stage.strStageKind &&
			1u == iCounterHitCount && 1u == iTimeoutCount &&
			bCounterTargetsResolve && bClosedCounterableFlag;
		Node.bCounterProxy = Stage.CounterProxy.has_value();
		const bool bHasCounterContractSignal =
			0u != iCounterHitCount || bHasAnyCounterableFlag;
		Node.bCounterContractIncomplete =
			bHasCounterContractSignal && !Node.bCounterWindow;
		Staged.Nodes.push_back(std::move(Node));
	}

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

void Client::BOSS_LOGIC_FLOW_SELECTION::Clear()
{
	eKind = BOSS_LOGIC_FLOW_SELECTION_KIND::NONE;
	strPatternId.clear();
	strStageId.clear();
	strActionId.clear();
	strOutcome.clear();
	strTargetActionId.clear();
	bTerminal = false;
	bAuthored = false;
}
