#include "ActionCompositionGraphModel.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
	using namespace Client;

	constexpr std::uint64_t SOURCE_GENERATION = 0x47524150484D4F44ull;
	constexpr std::uint64_t DASH_DEFAULT_DURATION_MS = 5150u;
	constexpr std::uint64_t DASH_WALL_DURATION_MS = 5150u;
	constexpr std::uint64_t DASH_MAXIMUM_DURATION_MS = 5150u;

	void Require(const bool bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	const VALTAN_PATTERN_VIEW& FindPattern(
		const VALTAN_PATTERN_TREE_VIEW& View,
		const std::string& PatternId)
	{
		const VALTAN_PATTERN_VIEW* pResult = nullptr;
		const auto Inspect = [&](const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
		{
			for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
			{
				if (Pattern.strPatternId != PatternId)
					continue;
				Require(nullptr == pResult,
					"production graph contains a duplicate Pattern identity");
				pResult = &Pattern;
			}
		};
		Inspect(View.Gimmicks);
		Inspect(View.Rotation);
		Require(nullptr != pResult,
			"production graph does not contain VALTAN_DASH_CHARGE");
		return *pResult;
	}

	const VALTAN_STAGE_VIEW& FindStage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& StageId)
	{
		const auto Found = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&](const VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == StageId;
			});
		Require(Found != Pattern.Stages.end(),
			"production graph is missing an expected Stage identity");
		return *Found;
	}

	VALTAN_STAGE_VIEW& FindStage(
		VALTAN_PATTERN_VIEW& Pattern,
		const std::string& StageId)
	{
		const auto Found = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&](const VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == StageId;
			});
		Require(Found != Pattern.Stages.end(),
			"production graph is missing an expected Stage identity");
		return *Found;
	}

	VALTAN_STAGE_BRANCH_VIEW& FindBranch(
		VALTAN_STAGE_VIEW& Stage,
		const std::string& Outcome)
	{
		const auto Found = std::find_if(
			Stage.Branches.begin(), Stage.Branches.end(),
			[&](const VALTAN_STAGE_BRANCH_VIEW& Branch)
			{
				return Branch.strOutcome == Outcome;
			});
		Require(Found != Stage.Branches.end(),
			"production graph is missing an expected branch outcome");
		return *Found;
	}

	ACTION_COMPOSITION_GRAPH_SNAPSHOT ProjectOrThrow(
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>& Overrides = {})
	{
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot;
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		if (!CActionCompositionGraphModel::Project(
			Pattern, SOURCE_GENERATION, Overrides, Snapshot, Error))
		{
			throw std::runtime_error(Error.strMessage);
		}
		return Snapshot;
	}

	void RequireRejectedWithoutSnapshotMutation(
		const VALTAN_PATTERN_VIEW& InvalidPattern,
		ACTION_COMPOSITION_GRAPH_ERROR_CODE ExpectedCode,
		ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT& ExpectedSnapshot);

	bool SamePoint(
		const ACTION_COMPOSITION_GRAPH_POINT& Left,
		const ACTION_COMPOSITION_GRAPH_POINT& Right)
	{
		return Left.fX == Right.fX && Left.fY == Right.fY;
	}

	bool SameRect(
		const ACTION_COMPOSITION_GRAPH_RECT& Left,
		const ACTION_COMPOSITION_GRAPH_RECT& Right)
	{
		return Left.fLeft == Right.fLeft && Left.fTop == Right.fTop &&
			Left.fRight == Right.fRight && Left.fBottom == Right.fBottom;
	}

	bool SamePath(
		const ACTION_COMPOSITION_GRAPH_PATH& Left,
		const ACTION_COMPOSITION_GRAPH_PATH& Right)
	{
		return Left.NodeIndices == Right.NodeIndices &&
			Left.EdgeIndices == Right.EdgeIndices &&
			Left.iDurationMs == Right.iDurationMs &&
			Left.bTerminal == Right.bTerminal;
	}

	bool SameNode(
		const ACTION_COMPOSITION_GRAPH_NODE& Left,
		const ACTION_COMPOSITION_GRAPH_NODE& Right)
	{
		return Left.Key == Right.Key &&
			Left.strSequenceRole == Right.strSequenceRole &&
			Left.strStageKind == Right.strStageKind &&
			Left.iDurationMs == Right.iDurationMs &&
			Left.iCanonicalStageIndex == Right.iCanonicalStageIndex &&
			Left.iGraphDepth == Right.iGraphDepth &&
			Left.iGameplayActionCount == Right.iGameplayActionCount &&
			Left.iBranchCount == Right.iBranchCount &&
			Left.iAnimationOccurrenceCount == Right.iAnimationOccurrenceCount &&
			Left.iEffectReferenceCount == Right.iEffectReferenceCount &&
			Left.iCameraInvocationCount == Right.iCameraInvocationCount &&
			Left.bEventEntered == Right.bEventEntered &&
			Left.bReachable == Right.bReachable &&
			SameRect(Left.Bounds, Right.Bounds);
	}

	bool SameEdge(
		const ACTION_COMPOSITION_GRAPH_EDGE& Left,
		const ACTION_COMPOSITION_GRAPH_EDGE& Right)
	{
		if (Left.strPatternId != Right.strPatternId ||
			Left.strSourceActionId != Right.strSourceActionId ||
			Left.strOutcome != Right.strOutcome ||
			Left.strTargetActionId != Right.strTargetActionId ||
			Left.strTargetPatternId != Right.strTargetPatternId ||
			Left.iSourceNodeIndex != Right.iSourceNodeIndex ||
			Left.iTargetNodeIndex != Right.iTargetNodeIndex ||
			Left.iSourceBranchIndex != Right.iSourceBranchIndex ||
			Left.eOrigin != Right.eOrigin ||
			Left.bTerminal != Right.bTerminal ||
			Left.bDefault != Right.bDefault ||
			Left.Polyline.size() != Right.Polyline.size())
		{
			return false;
		}
		for (std::size_t iPoint = 0u; iPoint < Left.Polyline.size(); ++iPoint)
		{
			if (!SamePoint(Left.Polyline[iPoint], Right.Polyline[iPoint]))
				return false;
		}
		return true;
	}

	bool SameSnapshot(
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Left,
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Right)
	{
		if (Left.iSourceGeneration != Right.iSourceGeneration ||
			Left.strPatternId != Right.strPatternId ||
			Left.iEntryNodeIndex != Right.iEntryNodeIndex ||
			Left.Nodes.size() != Right.Nodes.size() ||
			Left.Edges.size() != Right.Edges.size() ||
			!SamePath(Left.DefaultPath, Right.DefaultPath) ||
			!SamePath(Left.SelectedPath, Right.SelectedPath) ||
			!SamePath(Left.MaximumPath, Right.MaximumPath) ||
			!SameRect(Left.GraphBounds, Right.GraphBounds))
		{
			return false;
		}
		for (std::size_t iNode = 0u; iNode < Left.Nodes.size(); ++iNode)
		{
			if (!SameNode(Left.Nodes[iNode], Right.Nodes[iNode]))
				return false;
		}
		for (std::size_t iEdge = 0u; iEdge < Left.Edges.size(); ++iEdge)
		{
			if (!SameEdge(Left.Edges[iEdge], Right.Edges[iEdge]))
				return false;
		}
		return true;
	}

	std::vector<std::string> StageIds(
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
		const ACTION_COMPOSITION_GRAPH_PATH& Path)
	{
		std::vector<std::string> Result;
		Result.reserve(Path.NodeIndices.size());
		for (const std::size_t iNode : Path.NodeIndices)
		{
			Require(iNode < Snapshot.Nodes.size(),
				"projected path references an invalid node index");
			Result.push_back(Snapshot.Nodes[iNode].Key.strStageId);
		}
		return Result;
	}

	VALTAN_PATTERN_VIEW BuildManualLinearPattern()
	{
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = "VALTAN_GRAPH_MANUAL_TEST";
		Pattern.strActionId = "valtan.graph.manual-test";
		Pattern.strEntryActionId = "valtan.graph.manual-test.windup";
		Pattern.bManualServerAudition = true;

		const auto AddStage = [&Pattern](
			const char* const pStageId,
			const char* const pRole,
			const char* const pActionId,
			const char* const pStageKind,
			const std::uint32_t iDurationMs)
		{
			VALTAN_STAGE_VIEW Stage;
			Stage.strStageId = pStageId;
			Stage.strSequenceRole = pRole;
			Stage.strActionId = pActionId;
			Stage.strStageKind = pStageKind;
			Stage.iDurationMs = iDurationMs;
			Pattern.Stages.push_back(std::move(Stage));
		};
		AddStage("WINDUP", "WINDUP", "valtan.graph.manual-test.windup",
			"WINDUP", 1000u);
		AddStage("GROGGY", "GROGGY", "valtan.graph.manual-test.groggy",
			"GROGGY", 2000u);
		AddStage("ACTIVE", "ACTIVE", "valtan.graph.manual-test.active",
			"ACTIVE", 3000u);
		return Pattern;
	}

	void VerifyManualImmediateTimeoutPath()
	{
		const VALTAN_PATTERN_VIEW Manual = BuildManualLinearPattern();
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot =
			ProjectOrThrow(Manual);
		Require(StageIds(Snapshot, Snapshot.DefaultPath) ==
			std::vector<std::string>{ "WINDUP", "GROGGY", "ACTIVE" },
			"manual derived TIMEOUT skipped its immediate GROGGY vector-next Stage");
		Require(6000u == Snapshot.DefaultPath.iDurationMs &&
			Snapshot.DefaultPath.bTerminal,
			"manual WINDUP -> GROGGY -> ACTIVE default clock is not 6000 ms terminal");

		const auto FirstTimeout = std::find_if(
			Snapshot.Edges.begin(), Snapshot.Edges.end(),
			[](const ACTION_COMPOSITION_GRAPH_EDGE& Edge)
			{
				return "valtan.graph.manual-test.windup" ==
					Edge.strSourceActionId && "TIMEOUT" == Edge.strOutcome;
			});
		Require(FirstTimeout != Snapshot.Edges.end() &&
			ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::DERIVED_TIMEOUT ==
				FirstTimeout->eOrigin &&
			"valtan.graph.manual-test.groggy" ==
				FirstTimeout->strTargetActionId,
			"manual derived TIMEOUT did not target the immediate GROGGY action");

		VALTAN_PATTERN_VIEW Explicit = Manual;
		VALTAN_STAGE_BRANCH_VIEW Timeout;
		Timeout.strOutcome = "TIMEOUT";
		Timeout.strNextActionId = Explicit.Stages[2].strActionId;
		Explicit.Stages[0].Branches.push_back(std::move(Timeout));
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT ExplicitSnapshot =
			ProjectOrThrow(Explicit);
		Require(StageIds(ExplicitSnapshot, ExplicitSnapshot.DefaultPath) ==
			std::vector<std::string>{ "WINDUP", "ACTIVE" },
			"authored TIMEOUT did not retain its explicit manual target");
		Require(4000u == ExplicitSnapshot.DefaultPath.iDurationMs,
			"authored manual TIMEOUT path has the wrong duration");
	}

	void VerifyDeletedRouteRecoversAfterOverrideReset()
	{
		VALTAN_PATTERN_VIEW Manual = BuildManualLinearPattern();
		const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE> SelectedRoute{
			{ Manual.Stages[1].strActionId, "TIMEOUT" },
		};
		(void)ProjectOrThrow(Manual, SelectedRoute);

		Manual.Stages.erase(Manual.Stages.begin() + 1);
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Preserved = ProjectOrThrow(
			BuildManualLinearPattern());
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT BeforeFailure = Preserved;
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(!CActionCompositionGraphModel::Project(
				Manual, SOURCE_GENERATION + 1u, SelectedRoute, Preserved, Error) &&
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::UNKNOWN_OVERRIDE_ACTION ==
				Error.eCode,
			"deleted Stage left a stale route override that was not rejected");
		Require(SameSnapshot(BeforeFailure, Preserved),
			"stale deleted-route rejection replaced the admitted graph snapshot");

		ACTION_COMPOSITION_GRAPH_ERROR ResetError;
		const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE> ResetRoute;
		Require(CActionCompositionGraphModel::Project(
				Manual, SOURCE_GENERATION + 2u, ResetRoute, Preserved, ResetError),
			"clearing the route did not recover projection after Stage deletion");
		Require(StageIds(Preserved, Preserved.DefaultPath) ==
			std::vector<std::string>{ "WINDUP", "ACTIVE" },
			"recovered deleted-route default path is not the new Stage order");
	}

	void VerifyCrossPatternBoundary()
	{
		VALTAN_PATTERN_VIEW CrossPattern = BuildManualLinearPattern();
		VALTAN_STAGE_BRANCH_VIEW Cross;
		Cross.strOutcome = "TIMEOUT";
		Cross.strNextPatternId = "VALTAN_GRAPH_FOLLOW_UP";
		CrossPattern.Stages.front().Branches.push_back(Cross);

		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot =
			ProjectOrThrow(CrossPattern);
		const auto Boundary = std::find_if(
			Snapshot.Edges.begin(), Snapshot.Edges.end(),
			[](const ACTION_COMPOSITION_GRAPH_EDGE& Edge)
			{
				return "valtan.graph.manual-test.windup" ==
					Edge.strSourceActionId && "TIMEOUT" == Edge.strOutcome;
			});
		Require(Boundary != Snapshot.Edges.end() && Boundary->bTerminal &&
			Boundary->strTargetActionId.empty() &&
			"VALTAN_GRAPH_FOLLOW_UP" == Boundary->strTargetPatternId,
			"cross-Pattern edge was not preserved as a local graph boundary");
		Require(StageIds(Snapshot, Snapshot.DefaultPath) ==
			std::vector<std::string>{ "WINDUP" } &&
			1000u == Snapshot.DefaultPath.iDurationMs &&
			Snapshot.DefaultPath.bTerminal,
			"cross-Pattern edge leaked into the local action path");

		ACTION_COMPOSITION_GRAPH_SNAPSHOT Preserved = Snapshot;
		VALTAN_PATTERN_VIEW Ambiguous = CrossPattern;
		Ambiguous.Stages.front().Branches.front().strNextActionId =
			Ambiguous.Stages[1].strActionId;
		RequireRejectedWithoutSnapshotMutation(
			Ambiguous,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::AMBIGUOUS_BRANCH_TARGET,
			Preserved, Snapshot);
		VALTAN_PATTERN_VIEW InvalidTarget = CrossPattern;
		InvalidTarget.Stages.front().Branches.front().strNextPatternId =
			"not/a/stable/pattern";
		RequireRejectedWithoutSnapshotMutation(
			InvalidTarget,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::INVALID_TARGET_PATTERN_ID,
			Preserved, Snapshot);
	}

	void VerifyAuthoredEdgeSourceIdentity(const VALTAN_PATTERN_VIEW& Dash)
	{
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot = ProjectOrThrow(Dash);
		for (const ACTION_COMPOSITION_GRAPH_EDGE& Edge : Snapshot.Edges)
		{
			Require(Edge.iSourceNodeIndex < Snapshot.Nodes.size(),
				"projected edge has no source node");
			const ACTION_COMPOSITION_GRAPH_NODE& Source =
				Snapshot.Nodes[Edge.iSourceNodeIndex];
			Require(Source.iCanonicalStageIndex < Dash.Stages.size(),
				"projected edge source has no canonical Stage");
			const VALTAN_STAGE_VIEW& Stage =
				Dash.Stages[Source.iCanonicalStageIndex];
			Require(Edge.strPatternId == Dash.strPatternId &&
				Edge.strSourceActionId == Stage.strActionId &&
				Source.Key.strStageId == Stage.strStageId &&
				Source.Key.strActionId == Stage.strActionId,
				"projected edge lost its stable Pattern/Stage/action source identity");
			if (ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::DERIVED_TIMEOUT ==
				Edge.eOrigin)
			{
				Require(ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
					Edge.iSourceBranchIndex,
					"derived TIMEOUT falsely owns an authored branch ordinal");
				continue;
			}
			Require(Edge.iSourceBranchIndex < Stage.Branches.size(),
				"authored edge source branch ordinal is out of range");
			const VALTAN_STAGE_BRANCH_VIEW& Branch =
				Stage.Branches[Edge.iSourceBranchIndex];
			Require(Branch.strOutcome == Edge.strOutcome &&
				Branch.strNextPatternId.value_or(std::string{}) ==
					Edge.strTargetPatternId &&
				Branch.strNextActionId.has_value() != Edge.bTerminal &&
				(Edge.bTerminal ||
				 *Branch.strNextActionId == Edge.strTargetActionId),
				"authored edge does not resolve to its exact source branch");
		}
	}

	void VerifyProductionDurations(const VALTAN_PATTERN_VIEW& Dash)
	{
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot = ProjectOrThrow(Dash);
		Require(SOURCE_GENERATION == Snapshot.iSourceGeneration &&
			"VALTAN_DASH_CHARGE" == Snapshot.strPatternId,
			"projection lost its source generation or stable Pattern identity");
		Require(DASH_DEFAULT_DURATION_MS == Snapshot.DefaultPath.iDurationMs &&
			Snapshot.DefaultPath.bTerminal,
			"VALTAN_DASH_CHARGE default path is no longer a 5150 ms local boundary");
		Require(DASH_MAXIMUM_DURATION_MS == Snapshot.MaximumPath.iDurationMs &&
			Snapshot.MaximumPath.bTerminal,
			"VALTAN_DASH_CHARGE maximum path is no longer a 5150 ms local boundary");
		Require(StageIds(Snapshot, Snapshot.DefaultPath) ==
			std::vector<std::string>{ "WINDUP", "CHARGE" },
			"default path did not stop at the saved groggy follow-up boundary");
		Require(StageIds(Snapshot, Snapshot.MaximumPath) ==
			std::vector<std::string>{ "WINDUP", "CHARGE" },
			"maximum path did not stop at the saved groggy follow-up boundary");
	}

	void VerifyPreviewOutcomeOverride(const VALTAN_PATTERN_VIEW& Dash)
	{
		const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE> Overrides{
			{ "valtan.attack.dash-charge.active", "WALL_CONTACT" },
		};
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot =
			ProjectOrThrow(Dash, Overrides);
		Require(DASH_DEFAULT_DURATION_MS == Snapshot.DefaultPath.iDurationMs,
			"preview outcome override mutated the default path");
		Require(DASH_WALL_DURATION_MS == Snapshot.SelectedPath.iDurationMs &&
			Snapshot.SelectedPath.bTerminal,
			"WALL_CONTACT preview path is no longer a 5150 ms local boundary");
		Require(StageIds(Snapshot, Snapshot.SelectedPath) ==
			std::vector<std::string>{ "WINDUP", "CHARGE" },
			"selected WALL_CONTACT outcome crossed into the follow-up Pattern");
		Require(!Snapshot.SelectedPath.EdgeIndices.empty() &&
			Snapshot.SelectedPath.EdgeIndices.back() < Snapshot.Edges.size(),
			"selected WALL_CONTACT path lost its terminal boundary edge");
		const ACTION_COMPOSITION_GRAPH_EDGE& Boundary = Snapshot.Edges[
			Snapshot.SelectedPath.EdgeIndices.back()];
		Require("WALL_CONTACT" == Boundary.strOutcome && Boundary.bTerminal &&
			"VALTAN_DASH_CHARGE_GROGGY" == Boundary.strTargetPatternId,
			"selected WALL_CONTACT path lost its groggy follow-up identity");
		Require(DASH_MAXIMUM_DURATION_MS == Snapshot.MaximumPath.iDurationMs,
			"preview outcome override mutated the maximum path");
	}

	void VerifyDeterministicProjection(const VALTAN_PATTERN_VIEW& Dash)
	{
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT First = ProjectOrThrow(Dash);
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Second;
		Second.iSourceGeneration = 999u;
		Second.strPatternId = "PRESERVED_SENTINEL";
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(CActionCompositionGraphModel::Project(
			Dash, SOURCE_GENERATION, Second, Error),
			"second projection of the same admitted Pattern failed");
		Require(SameSnapshot(First, Second),
			"same Pattern and generation produced a non-deterministic graph snapshot");
	}

	void RequireRejectedWithoutSnapshotMutation(
		const VALTAN_PATTERN_VIEW& InvalidPattern,
		const ACTION_COMPOSITION_GRAPH_ERROR_CODE ExpectedCode,
		ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT& ExpectedSnapshot)
	{
		ACTION_COMPOSITION_GRAPH_ERROR Error;
		Require(!CActionCompositionGraphModel::Project(
			InvalidPattern, SOURCE_GENERATION + 1u, InOutSnapshot, Error),
			"malformed graph projection unexpectedly succeeded");
		Require(ExpectedCode == Error.eCode,
			"malformed graph projection returned the wrong typed error");
		Require(SameSnapshot(ExpectedSnapshot, InOutSnapshot),
			"failed graph projection replaced the previously admitted snapshot");
	}

	void VerifyMalformedInputsPreserveSnapshot(const VALTAN_PATTERN_VIEW& Dash)
	{
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot = ProjectOrThrow(Dash);
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Admitted = Snapshot;

		VALTAN_PATTERN_VIEW DuplicateStage = Dash;
		Require(DuplicateStage.Stages.size() >= 2u,
			"production dash Pattern has fewer than two Stages");
		DuplicateStage.Stages[1].strStageId =
			DuplicateStage.Stages[0].strStageId;
		RequireRejectedWithoutSnapshotMutation(
			DuplicateStage,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_STAGE_ID,
			Snapshot, Admitted);

		VALTAN_PATTERN_VIEW DuplicateAction = Dash;
		DuplicateAction.Stages[1].strActionId =
			DuplicateAction.Stages[0].strActionId;
		RequireRejectedWithoutSnapshotMutation(
			DuplicateAction,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::DUPLICATE_ACTION_ID,
			Snapshot, Admitted);

		VALTAN_PATTERN_VIEW Dangling = Dash;
		VALTAN_STAGE_BRANCH_VIEW& DanglingBranch =
			FindBranch(FindStage(Dangling, "CHARGE"), "WALL_CONTACT");
		DanglingBranch.strNextPatternId.reset();
		DanglingBranch.strNextActionId =
			"valtan.attack.dash-charge.missing";
		RequireRejectedWithoutSnapshotMutation(
			Dangling,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::DANGLING_TARGET_ACTION,
			Snapshot, Admitted);

		VALTAN_PATTERN_VIEW Cyclic = Dash;
		VALTAN_STAGE_BRANCH_VIEW& CyclicBranch =
			FindBranch(FindStage(Cyclic, "CHARGE"), "TIMEOUT");
		CyclicBranch.strNextPatternId.reset();
		CyclicBranch.strNextActionId = Cyclic.strEntryActionId;
		RequireRejectedWithoutSnapshotMutation(
			Cyclic,
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::CYCLE,
			Snapshot, Admitted);
	}

	void VerifyNodeHitTesting(const VALTAN_PATTERN_VIEW& Dash)
	{
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot = ProjectOrThrow(Dash);
		for (std::size_t iNode = 0u; iNode < Snapshot.Nodes.size(); ++iNode)
		{
			const ACTION_COMPOSITION_GRAPH_RECT& Bounds =
				Snapshot.Nodes[iNode].Bounds;
			const ACTION_COMPOSITION_GRAPH_POINT Center{
				(Bounds.fLeft + Bounds.fRight) * 0.5f,
				(Bounds.fTop + Bounds.fBottom) * 0.5f,
			};
			Require(iNode == CActionCompositionGraphModel::Hit_TestNode(
				Snapshot, Center),
				"node hit-test did not return the stable projected node");
		}
		const ACTION_COMPOSITION_GRAPH_POINT Outside{
			Snapshot.GraphBounds.fRight + 512.f,
			Snapshot.GraphBounds.fBottom + 512.f,
		};
		Require(ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
			CActionCompositionGraphModel::Hit_TestNode(Snapshot, Outside),
			"node hit-test selected a node outside graph bounds");
	}

	void VerifyEdgeHitTesting(const VALTAN_PATTERN_VIEW& Dash)
	{
		const ACTION_COMPOSITION_GRAPH_SNAPSHOT Snapshot = ProjectOrThrow(Dash);
		const auto Edge = std::find_if(
			Snapshot.Edges.begin(), Snapshot.Edges.end(),
			[](const ACTION_COMPOSITION_GRAPH_EDGE& Candidate)
			{
				return "valtan.attack.dash-charge.active" ==
					Candidate.strSourceActionId &&
					"TIMEOUT" == Candidate.strOutcome &&
					Candidate.bTerminal &&
					"VALTAN_DASH_CHARGE_GROGGY" ==
						Candidate.strTargetPatternId;
			});
		Require(Edge != Snapshot.Edges.end() && Edge->Polyline.size() >= 2u,
			"projection is missing the CHARGE follow-up boundary geometry");
		const std::size_t iEdge = static_cast<std::size_t>(
			std::distance(Snapshot.Edges.begin(), Edge));
		const ACTION_COMPOSITION_GRAPH_POINT OnEdge{
			(Edge->Polyline[0].fX + Edge->Polyline[1].fX) * 0.5f,
			(Edge->Polyline[0].fY + Edge->Polyline[1].fY) * 0.5f,
		};
		Require(iEdge == CActionCompositionGraphModel::Hit_TestEdge(
			Snapshot, OnEdge, 0.25f),
			"edge hit-test did not return the projected terminal edge");

		const ACTION_COMPOSITION_GRAPH_POINT Outside{
			Snapshot.GraphBounds.fRight + 512.f,
			Snapshot.GraphBounds.fBottom + 512.f,
		};
		Require(ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
			CActionCompositionGraphModel::Hit_TestEdge(Snapshot, Outside),
			"edge hit-test selected an edge outside graph bounds");
		Require(ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
			CActionCompositionGraphModel::Hit_TestEdge(
				Snapshot, OnEdge, -1.f),
			"edge hit-test accepted a negative tolerance");
		Require(ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
			CActionCompositionGraphModel::Hit_TestEdge(
				Snapshot, OnEdge,
				(std::numeric_limits<float>::quiet_NaN)()),
			"edge hit-test accepted a non-finite tolerance");
	}
}

int Run_ActionCompositionGraphModelContractTests()
{
	using namespace Client;

	VALTAN_PATTERN_TREE_VIEW View;
	std::string Status;
	if (!CValtanPatternTree::Load(View, Status))
	{
		std::cerr << "FAIL Action Composition graph load: " << Status << '\n';
		return 1;
	}
	const VALTAN_PATTERN_VIEW* pDash = nullptr;
	try
	{
		pDash = &FindPattern(View, "VALTAN_DASH_CHARGE");
	}
	catch (const std::exception& Error)
	{
		std::cerr << "FAIL Action Composition graph inventory: " <<
			Error.what() << '\n';
		return 1;
	}
	const VALTAN_PATTERN_VIEW& Dash = *pDash;
	const std::vector<std::pair<const char*, std::function<void()>>> Tests{
		{ "production default/maximum path durations",
			[&] { VerifyProductionDurations(Dash); } },
		{ "preview outcome selected path",
			[&] { VerifyPreviewOutcomeOverride(Dash); } },
		{ "manual immediate TIMEOUT path",
			[&] { VerifyManualImmediateTimeoutPath(); } },
		{ "cross-Pattern boundary",
			[&] { VerifyCrossPatternBoundary(); } },
		{ "deleted route override reset",
			[&] { VerifyDeletedRouteRecoversAfterOverrideReset(); } },
		{ "authored edge source identity",
			[&] { VerifyAuthoredEdgeSourceIdentity(Dash); } },
		{ "deterministic projection",
			[&] { VerifyDeterministicProjection(Dash); } },
		{ "malformed graph rollback",
			[&] { VerifyMalformedInputsPreserveSnapshot(Dash); } },
		{ "node hit testing",
			[&] { VerifyNodeHitTesting(Dash); } },
		{ "edge hit testing",
			[&] { VerifyEdgeHitTesting(Dash); } },
	};

	std::size_t iFailures = 0u;
	for (const auto& [Name, Test] : Tests)
	{
		try
		{
			Test();
		}
		catch (const std::exception& Error)
		{
			++iFailures;
			std::cerr << "FAIL Action Composition graph " << Name << ": " <<
				Error.what() << '\n';
		}
	}
	std::cout << "ActionCompositionGraphModelContractTests: " <<
		Tests.size() - iFailures << "/" << Tests.size() << " passed\n";
	return 0u == iFailures ? 0 : 1;
}
