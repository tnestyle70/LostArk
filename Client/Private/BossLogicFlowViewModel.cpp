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

	bool HasUnambiguousBranchTarget(
		const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
	{
		const bool bHasAction = Branch.strNextActionId.has_value();
		const bool bHasPattern = Branch.strNextPatternId.has_value();
		return !(bHasAction && bHasPattern) &&
			(!bHasAction || !Branch.strNextActionId->empty()) &&
			(!bHasPattern || !Branch.strNextPatternId->empty());
	}

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
			{ return Node.strActionId == Snapshot.strActionId; });
	}

	std::uint32_t NextNonzero(const std::uint32_t Value)
	{
		return (std::numeric_limits<std::uint32_t>::max)() == Value ?
			1u : Value + 1u;
	}

	bool IsConsecutiveTick(
		const std::uint32_t Previous,
		const std::uint32_t Current)
	{
		return 0u != Previous && Current == NextNonzero(Previous);
	}
}

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
	strTargetPatternId.clear();
	bTerminal = false;
	bAuthored = false;
}

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
	if (!IsConsecutiveTick(
			m_PreviousSnapshot.iServerTick, Snapshot.iServerTick) ||
		m_PreviousView.Graph.iSourceGeneration !=
			pCurrentView->Graph.iSourceGeneration)
	{
		StageCurrent();
		return false;
	}

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

void Client::CBossLogicFlowObservedEdgeResolver::Reset()
{
	Clear_Anchor();
	m_History.clear();
	m_iNextObservationSequence = 1u;
}

void Client::CBossLogicFlowObservedEdgeResolver::Clear_Anchor()
{
	m_PreviousSnapshot = {};
	m_PreviousView = {};
	m_bHasAnchor = false;
}
