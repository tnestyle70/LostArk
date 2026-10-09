#include "BossLogicFlowView.h"
#include "ValtanPatternTree.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace Client;

namespace
{
	void Require(const bool Condition, const char* const pMessage)
	{
		if (!Condition)
			throw std::runtime_error(pMessage);
	}

	VALTAN_STAGE_ACTION_VIEW CounterFlag(
		std::string strTrigger,
		const float fValue)
	{
		VALTAN_STAGE_ACTION_VIEW Action;
		Action.strTrigger = std::move(strTrigger);
		Action.strKind = "SET_BOSS_FLAG";
		Action.strTargetId = "boss.flag.counterable";
		Action.fValue = fValue;
		return Action;
	}

	VALTAN_CLIP_OCCURRENCE_VIEW Clip(std::string strName)
	{
		VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
		Occurrence.strClipOccurrenceId = "occurrence." + strName;
		Occurrence.strClipName = std::move(strName);
		Occurrence.iAuthoringWallMs = 500u;
		return Occurrence;
	}

	VALTAN_PATTERN_VIEW CounterPattern()
	{
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = "VALTAN_LOGIC_FLOW_TEST";
		Pattern.strDisplayName = "Logic Flow Test";
		Pattern.strActionId = "valtan.logic-flow-test";
		Pattern.strEntryActionId = "valtan.logic-flow-test.windup";

		VALTAN_STAGE_VIEW Windup;
		Windup.strStageId = "WINDUP";
		Windup.strSequenceRole = "WINDUP";
		Windup.strStageKind = "WINDUP";
		Windup.strActionId = Pattern.strEntryActionId;
		Windup.iDurationMs = 1000u;
		Windup.ClipOccurrences.push_back(Clip("mesh_att_counter_start"));
		Windup.ClipOccurrences.push_back(Clip("mesh_att_counter_hold"));
		Windup.Actions.push_back(CounterFlag("ENTER", 1.f));
		Windup.Actions.push_back(CounterFlag("EXIT", 0.f));
		Windup.Branches.push_back({
			"COUNTER_HIT", "valtan.logic-flow-test.groggy" });
		Windup.Branches.push_back({
			"TIMEOUT", "valtan.logic-flow-test.recovery" });
		VALTAN_COUNTER_PROXY_VIEW Proxy;
		Proxy.strSpace = "BOSS_LOCAL";
		Proxy.fForwardOffsetM = 1.f;
		Proxy.fRadiusM = 2.25f;
		Windup.CounterProxy = Proxy;

		VALTAN_STAGE_VIEW Groggy;
		Groggy.strStageId = "GROGGY";
		Groggy.strSequenceRole = "GROGGY";
		Groggy.strStageKind = "GROGGY";
		Groggy.strActionId = "valtan.logic-flow-test.groggy";
		Groggy.iDurationMs = 1300u;
		Groggy.ClipOccurrences.push_back(Clip("mesh_abn_groggy_start"));
		Groggy.ClipOccurrences.push_back(Clip("mesh_abn_groggy_loop"));
		Groggy.ClipOccurrences.push_back(Clip("mesh_abn_groggy_end"));

		VALTAN_STAGE_VIEW Recovery;
		Recovery.strStageId = "RECOVERY";
		Recovery.strSequenceRole = "RECOVERY";
		Recovery.strStageKind = "RECOVERY";
		Recovery.strActionId = "valtan.logic-flow-test.recovery";
		Recovery.iDurationMs = 700u;

		Pattern.Stages = {
			std::move(Windup), std::move(Groggy), std::move(Recovery) };
		return Pattern;
	}

	void VerifyClipAndCounterProjection()
	{
		const VALTAN_PATTERN_VIEW Pattern = CounterPattern();
		BOSS_LOGIC_FLOW_VIEW View;
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(CBossLogicFlowViewModel::Project(
			Pattern, 41u, View, Error),
			"valid Counter Pattern should project");
		Require(41u == View.Graph.iSourceGeneration,
			"source generation should remain exact");
		Require(View.strPatternId == Pattern.strPatternId && 3u == View.Nodes.size(),
			"Pattern identity and Stage count should project");
		Require(View.bHasCounterHitBranch,
			"Pattern-level Sequence badge should project from COUNTER_HIT");
		Require(View.Nodes[0].strStageKind == "WINDUP" &&
			View.Nodes[0].ClipNames == std::vector<std::string>{
				"mesh_att_counter_start", "mesh_att_counter_hold" },
			"ordered native clip names should project exactly");
		Require(View.Nodes[0].bCounterWindow && View.Nodes[0].bCounterProxy &&
			!View.Nodes[0].bCounterContractIncomplete,
			"closed WINDUP Counter contract and proxy should be admitted");
		Require(View.Nodes[1].ClipNames == std::vector<std::string>{
				"mesh_abn_groggy_start", "mesh_abn_groggy_loop",
				"mesh_abn_groggy_end" },
			"multi-clip Groggy sequence should remain ordered");
		Require(View.Nodes[2].ClipNames.empty(),
			"Animation NONE Stage should remain explicit");

		std::size_t iCounterHitEdges = 0u;
		std::size_t iCounterTimeoutEdges = 0u;
		for (const BOSS_LOGIC_FLOW_EDGE_VIEW& Edge : View.Edges)
		{
			iCounterHitEdges += Edge.bCounterSuccess ? 1u : 0u;
			iCounterTimeoutEdges += Edge.bCounterTimeout ? 1u : 0u;
		}
		Require(1u == iCounterHitEdges && 1u == iCounterTimeoutEdges,
			"exact authored COUNTER_HIT and paired TIMEOUT should be annotated once");
	}

	void VerifyIncompleteCounterFailsClosed()
	{
		VALTAN_PATTERN_VIEW Pattern = CounterPattern();
		Pattern.Stages.front().Actions.pop_back();
		BOSS_LOGIC_FLOW_VIEW View;
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(CBossLogicFlowViewModel::Project(
			Pattern, 5u, View, Error),
			"read-only graph should still diagnose an incomplete Counter contract");
		Require(!View.Nodes.front().bCounterWindow &&
			View.Nodes.front().bCounterContractIncomplete,
			"incomplete Counter contract must not receive the admitted badge");
		Require(View.bHasCounterHitBranch,
			"Sequence badge should remain a direct COUNTER_HIT branch fact");
		std::size_t iCounterTimeoutEdges = 0u;
		for (const BOSS_LOGIC_FLOW_EDGE_VIEW& Edge : View.Edges)
			iCounterTimeoutEdges += Edge.bCounterTimeout ? 1u : 0u;
		Require(0u == iCounterTimeoutEdges,
			"malformed Counter TIMEOUT must not be styled as an admitted pair");
	}

	void VerifyRejectedProjectionPreservesPreviousView()
	{
		VALTAN_PATTERN_VIEW Pattern = CounterPattern();
		BOSS_LOGIC_FLOW_VIEW View;
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(CBossLogicFlowViewModel::Project(
			Pattern, 7u, View, Error), "baseline graph should project");
		const std::string PreservedPatternId = View.strPatternId;
		const std::vector<std::string> PreservedClips =
			View.Nodes.front().ClipNames;

		Pattern.Stages[1].strStageId = Pattern.Stages[0].strStageId;
		Require(!CBossLogicFlowViewModel::Project(
			Pattern, 8u, View, Error),
			"duplicate Stage identity should reject projection");
		Require(ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_STAGE_ID ==
			Error.eCode,
			"duplicate Stage rejection should preserve the exact graph error");
		Require(View.strPatternId == PreservedPatternId &&
			7u == View.Graph.iSourceGeneration &&
			View.Nodes.front().ClipNames == PreservedClips,
			"rejected projection must preserve the previous Boss Logic Flow view");
	}

	void VerifySelectionIsOptIn()
	{
		const BOSS_LOGIC_FLOW_RENDER_CONTEXT Context;
		Require(!Context.bAllowSelection,
			"Boss Logic Flow must default to read-only presentation");
	}
}

int Run_BossLogicFlowViewModelContractTests()
{
	const std::vector<std::pair<const char*, std::function<void()>>> Tests = {
		{ "clip and Counter projection", VerifyClipAndCounterProjection },
		{ "incomplete Counter fails closed", VerifyIncompleteCounterFailsClosed },
		{ "rejected projection preserves previous view",
			VerifyRejectedProjectionPreservesPreviousView },
		{ "selection is opt-in", VerifySelectionIsOptIn },
	};
	std::size_t Failed = 0u;
	for (const auto& [Name, Test] : Tests)
	{
		try
		{
			Test();
		}
		catch (const std::exception& Error)
		{
			++Failed;
			std::cerr << "FAIL Boss Logic Flow " << Name << ": " <<
				Error.what() << '\n';
		}
	}
	std::cout << "BossLogicFlowViewModelContractTests: " <<
		Tests.size() - Failed << "/" << Tests.size() << " passed\n";
	return static_cast<int>(Failed);
}
