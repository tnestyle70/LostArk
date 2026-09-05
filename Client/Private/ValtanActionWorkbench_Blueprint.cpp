#include "imgui.h"

#include "ValtanActionWorkbench.h"

#include "BalanceTool.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <utility>

namespace
{
	constexpr float BOSS_PATTERN_MIN_ZOOM = 0.35f;
	constexpr float BOSS_PATTERN_MAX_ZOOM = 2.50f;
	constexpr float BOSS_PATTERN_CANVAS_MIN_HEIGHT = 220.f;

	bool_t BuildNextBossPatternStageIdentity(
		const Client::VALTAN_PATTERN_VIEW& Pattern,
		std::string& strOutStageId,
		std::string& strOutActionId)
	{
		for (std::size_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
		{
			std::ostringstream Suffix;
			Suffix << std::setw(2) << std::setfill('0') << ordinal;
			const std::string StageId = "COMPOSITION_" + Suffix.str();
			std::string ActionId = Pattern.strActionId +
				".composition-stage." + Suffix.str();
			if (ActionId.size() > 160u)
			{
				std::uint64_t Hash = 1469598103934665603ull;
				for (const unsigned char Character : Pattern.strPatternId)
				{
					Hash ^= Character;
					Hash *= 1099511628211ull;
				}
				std::ostringstream Compact;
				Compact << "valtan.composition." << std::hex <<
					std::setw(16) << std::setfill('0') << Hash << std::dec <<
					".stage." << Suffix.str();
				ActionId = Compact.str();
			}
			const bool_t bStageExists = std::any_of(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[&StageId](const Client::VALTAN_STAGE_VIEW& Stage)
				{ return Stage.strStageId == StageId; });
			const bool_t bActionExists = std::any_of(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[&ActionId](const Client::VALTAN_STAGE_VIEW& Stage)
				{ return Stage.strActionId == ActionId; });
			if (!bStageExists && !bActionExists)
			{
				strOutStageId = StageId;
				strOutActionId = ActionId;
				return true;
			}
		}
		strOutStageId.clear();
		strOutActionId.clear();
		return false;
	}

	bool PathContainsNode(
		const Client::ACTION_COMPOSITION_GRAPH_PATH& Path,
		const std::size_t iNode)
	{
		return Path.NodeIndices.end() != std::find(
			Path.NodeIndices.begin(), Path.NodeIndices.end(), iNode);
	}

	bool PathContainsEdge(
		const Client::ACTION_COMPOSITION_GRAPH_PATH& Path,
		const std::size_t iEdge)
	{
		return Path.EdgeIndices.end() != std::find(
			Path.EdgeIndices.begin(), Path.EdgeIndices.end(), iEdge);
	}

	ImVec2 GraphToScreen(
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Point,
		const ImVec2& Origin,
		const float fPanX,
		const float fPanY,
		const float fZoom)
	{
		return ImVec2(
			Origin.x + fPanX + Point.fX * fZoom,
			Origin.y + fPanY + Point.fY * fZoom);
	}

	Client::ACTION_COMPOSITION_GRAPH_POINT ScreenToGraph(
		const ImVec2& Point,
		const ImVec2& Origin,
		const float fPanX,
		const float fPanY,
		const float fZoom)
	{
		return {
			(Point.x - Origin.x - fPanX) / fZoom,
			(Point.y - Origin.y - fPanY) / fZoom };
	}

	void DrawEdgeArrow(
		ImDrawList* const pDrawList,
		const std::vector<Client::ACTION_COMPOSITION_GRAPH_POINT>& Polyline,
		const ImVec2& Origin,
		const float fPanX,
		const float fPanY,
		const float fZoom,
		const ImU32 Color)
	{
		if (nullptr == pDrawList || Polyline.size() < 2u)
			return;
		const ImVec2 Tip = GraphToScreen(
			Polyline.back(), Origin, fPanX, fPanY, fZoom);
		const ImVec2 Previous = GraphToScreen(
			Polyline[Polyline.size() - 2u], Origin, fPanX, fPanY, fZoom);
		const float DeltaX = Tip.x - Previous.x;
		const float DeltaY = Tip.y - Previous.y;
		const float Length = std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
		if (Length <= 0.001f)
			return;
		const float DirectionX = DeltaX / Length;
		const float DirectionY = DeltaY / Length;
		const float ArrowLength = 9.f;
		const float ArrowWidth = 4.5f;
		const ImVec2 Base(
			Tip.x - DirectionX * ArrowLength,
			Tip.y - DirectionY * ArrowLength);
		const ImVec2 Perpendicular(-DirectionY, DirectionX);
		pDrawList->AddTriangleFilled(
			Tip,
			ImVec2(Base.x + Perpendicular.x * ArrowWidth,
				Base.y + Perpendicular.y * ArrowWidth),
			ImVec2(Base.x - Perpendicular.x * ArrowWidth,
				Base.y - Perpendicular.y * ArrowWidth),
			Color);
	}

	bool_t MatchesBossPatternEdgeSource(
		const Client::ACTION_COMPOSITION_GRAPH_EDGE& Edge,
		const Client::ACTION_COMPOSITION_GRAPH_NODE& Source,
		const Client::VALTAN_PATTERN_VIEW& Pattern,
		const Client::VALTAN_STAGE_VIEW& Stage)
	{
		if (Edge.strPatternId != Pattern.strPatternId ||
			Source.Key.strPatternId != Pattern.strPatternId ||
			Source.Key.strStageId != Stage.strStageId ||
			Source.Key.strActionId != Stage.strActionId ||
			Edge.strSourceActionId != Stage.strActionId ||
			Edge.iSourceNodeIndex != Source.iCanonicalStageIndex)
		{
			return false;
		}

		if (Client::ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::DERIVED_TIMEOUT ==
			Edge.eOrigin)
		{
			return Client::ACTION_COMPOSITION_GRAPH_INVALID_INDEX ==
					Edge.iSourceBranchIndex &&
				"TIMEOUT" == Edge.strOutcome;
		}
		if (Edge.iSourceBranchIndex >= Stage.Branches.size())
			return false;

		const Client::VALTAN_STAGE_BRANCH_VIEW& Branch =
			Stage.Branches[Edge.iSourceBranchIndex];
		if (Branch.strOutcome != Edge.strOutcome ||
			Branch.strNextPatternId.value_or(std::string{}) !=
				Edge.strTargetPatternId ||
			Branch.strNextActionId.has_value() == Edge.bTerminal)
		{
			return false;
		}
		return Edge.bTerminal ||
			*Branch.strNextActionId == Edge.strTargetActionId;
	}
}

bool_t Client::CValtanActionWorkbench::
Render_BossPatternStageTopologyControls(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bPatternMutationAdmitted)
{
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled("Gameplay owner is unavailable.");
		return false;
	}
	if (!Pattern.bManualServerAudition)
	{
		ImGui::TextDisabled(
			"Canonical gameplay topology is read-only. Select nodes/edges here; tune Stage clocks, motion, collider and typed Logic in Details.");
		return false;
	}

	std::string TopologyStatus;
	const bool_t bTopologyEditable =
		m_pBalanceTool->Can_Edit_ValtanManualStageTopology(
			Pattern.strPatternId, TopologyStatus);
	if (!bTopologyEditable)
		ImGui::TextWrapped("Topology is read-only: %s", TopologyStatus.c_str());

	int32_t iDurationMs = static_cast<int32_t>((std::min)(
		m_iManualStageInsertDurationMs, 600000u));
	bool_t bChanged = false;
	ImGui::BeginDisabled(!bPatternMutationAdmitted || !bTopologyEditable);
	ImGui::SetNextItemWidth(180.f);
	if (ImGui::DragInt(
			"New Stage (ms)##BossPattern", &iDurationMs,
			10.f, 1, 600000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
	{
		m_iManualStageInsertDurationMs = static_cast<uint32_t>(
			(std::max)(iDurationMs, 1));
	}

	const auto InsertAfter = [this, &Pattern, &Stage](
		const char_t* const pRole)
	{
		std::string NewStageId;
		std::string NewActionId;
		std::string Status;
		if (!BuildNextBossPatternStageIdentity(
				Pattern, NewStageId, NewActionId))
		{
			m_strStatus =
				"Manual Stage insertion rejected: no free stable Stage/Action suffix remains.";
			return false;
		}
		if (!m_pBalanceTool->Insert_ValtanManualStageAfter(
				Pattern.strPatternId, Stage.strStageId,
				NewStageId, NewActionId, pRole,
				m_iManualStageInsertDurationMs, Status))
		{
			m_strStatus = std::move(Status);
			return false;
		}
		m_strSelectedStageId = NewStageId;
		m_strSelectedStableId = NewStageId;
		m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		m_bDetailsWindowVisible = true;
		m_strStatus = std::move(Status);
		return true;
	};

	ImGui::SameLine();
	ImGui::TextDisabled("Add after %s:", Stage.strStageId.c_str());
	ImGui::SameLine();
	if (ImGui::SmallButton("ACTIVE##BossPatternAdd"))
		bChanged = InsertAfter("ACTIVE");
	ImGui::SameLine();
	if (ImGui::SmallButton("WINDUP##BossPatternAdd"))
		bChanged = InsertAfter("WINDUP");
	ImGui::SameLine();
	if (ImGui::SmallButton("GROGGY##BossPatternAdd"))
		bChanged = InsertAfter("GROGGY");
	ImGui::SameLine();
	if (ImGui::SmallButton("WAIT / GAP##BossPatternAdd"))
		bChanged = InsertAfter("WAIT");

	const auto Selected = std::find_if(
		Pattern.Stages.begin(), Pattern.Stages.end(),
		[&Stage](const VALTAN_STAGE_VIEW& Candidate)
		{ return Candidate.strStageId == Stage.strStageId; });
	const std::size_t iStageIndex = Selected == Pattern.Stages.end() ?
		Pattern.Stages.size() : static_cast<std::size_t>(
			Selected - Pattern.Stages.begin());
	ImGui::BeginDisabled(0u == iStageIndex || iStageIndex >= Pattern.Stages.size());
	if (ImGui::SmallButton("Move Earlier##BossPattern") && iStageIndex > 0u)
	{
		std::string Status;
		bChanged = m_pBalanceTool->Move_ValtanManualStage(
			Pattern.strPatternId, Stage.strStageId,
			Pattern.Stages[iStageIndex - 1u].strStageId, true, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(iStageIndex + 1u >= Pattern.Stages.size());
	if (ImGui::SmallButton("Move Later##BossPattern") &&
		iStageIndex + 1u < Pattern.Stages.size())
	{
		std::string Status;
		bChanged = m_pBalanceTool->Move_ValtanManualStage(
			Pattern.strPatternId, Stage.strStageId,
			Pattern.Stages[iStageIndex + 1u].strStageId, false, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		Pattern.Stages.size() <= 1u || iStageIndex >= Pattern.Stages.size());
	if (ImGui::SmallButton("Delete Stage##BossPattern") &&
		Pattern.Stages.size() > 1u && iStageIndex < Pattern.Stages.size())
	{
		VALTAN_PATTERN_VIEW Candidate = Pattern;
		Candidate.Stages.erase(Candidate.Stages.begin() +
			static_cast<std::ptrdiff_t>(iStageIndex));
		Candidate.strEntryActionId = Candidate.Stages.front().strActionId;
		std::string Status;
		if (Validate_ManualStageTopologySoundDependencies(Candidate, Status) &&
			m_pBalanceTool->Remove_ValtanManualStage(
				Pattern.strPatternId, Stage.strStageId, Status))
		{
			const std::size_t iReplacement = (std::min)(
				iStageIndex, Candidate.Stages.size() - 1u);
			m_strSelectedStageId = Candidate.Stages[iReplacement].strStageId;
			m_strSelectedStableId = m_strSelectedStageId;
			m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
			m_bDetailsWindowVisible = true;
			bChanged = true;
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	ImGui::TextDisabled(
		"WAIT/GAP is a real Server Stage clock with animation NONE. Dragging graph wires is read-only until that branch type has a typed writer.");
	if (bChanged)
	{
		/* Stage insertion/removal/reorder can invalidate either the source action
		   or the outcome selected by a preview-only route.  Reset the whole route
		   transactionally after the typed topology writer succeeds; the next
		   immutable Pattern view will rebuild its default path. */
		m_BossPatternOutcomeOverrides.clear();
		++m_iBossPatternRouteGeneration;
		Invalidate_TimelineCache();
		m_bBossPatternFitRequested = true;
	}
	return bChanged;
}

void Client::CValtanActionWorkbench::Render_BossPatternWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const std::uint64_t iPatternViewDraftGeneration,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bBossPatternWindowVisible)
		return;

	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const ImVec2 WorkPos = nullptr == pViewport ?
		ImVec2(120.f, 80.f) : pViewport->WorkPos;
	const ImVec2 WorkSize = nullptr == pViewport ?
		ImVec2(1600.f, 900.f) : pViewport->WorkSize;
	ImGui::SetNextWindowPos(
		ImVec2(WorkPos.x + WorkSize.x * 0.18f,
			WorkPos.y + WorkSize.y * 0.14f),
		ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(
		ImVec2((std::max)(720.f, WorkSize.x * 0.64f),
			(std::max)(520.f, WorkSize.y * 0.68f)),
		ImGuiCond_FirstUseEver);
	if (m_bBossPatternFocusRequested)
	{
		ImGui::SetNextWindowFocus();
		m_bBossPatternFocusRequested = false;
	}
	if (!ImGui::Begin(
			"Composition Boss Pattern###CompositionBossPatternWindow",
			&m_bBossPatternWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return;
	}
	Render_WindowMenu();

	Render_BossPatternPane(
		pPattern, iPatternViewDraftGeneration, bMutationAdmitted, bPatternMutationAdmitted);
	ImGui::End();
}

void Client::CValtanActionWorkbench::Render_BossPatternPane(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const std::uint64_t iPatternViewDraftGeneration,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	ImGui::TextUnformatted("Pattern branches = where / Sequencer lanes = when");
	ImGui::SameLine();
	if (ImGui::SmallButton("Open Sequencer"))
		m_bSequencerWindowVisible = true;
	ImGui::SameLine();
	if (ImGui::SmallButton("Animation Resources"))
	{
		m_bResourcesWindowVisible = true;
		m_eRequestedResourceDomain = RESOURCE_DOMAIN::ANIMATION;
		m_bResourceDomainSelectionRequested = true;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Effect Resources"))
	{
		m_bResourcesWindowVisible = true;
		m_eRequestedResourceDomain = RESOURCE_DOMAIN::EFFECT;
		m_bResourceDomainSelectionRequested = true;
	}

	if (nullptr == pPattern)
	{
		ImGui::SeparatorText("Boss Pattern Graph");
		ImGui::TextDisabled("Select one admitted Pattern in Composition Patterns.");
		return;
	}

	if (m_strBossPatternRoutePatternId != pPattern->strPatternId)
	{
		m_strBossPatternRoutePatternId = pPattern->strPatternId;
		m_BossPatternOutcomeOverrides.clear();
		++m_iBossPatternRouteGeneration;
		m_bBossPatternFitRequested = true;
	}
	const bool_t bAttemptKeyMatches = m_bBossPatternGraphAttempted &&
		m_strBossPatternGraphAttemptPatternId == pPattern->strPatternId &&
		m_strBossPatternGraphAttemptCanonicalRevision ==
			m_strPinnedCanonicalSourceRevision &&
		m_iBossPatternGraphAttemptDraftGeneration ==
			iPatternViewDraftGeneration &&
		m_iBossPatternGraphAttemptRouteGeneration ==
			m_iBossPatternRouteGeneration;
	if (!bAttemptKeyMatches)
	{
		/* Commit the attempted key before projection. A malformed graph is one
		   diagnostic event, not a per-frame projection loop. */
		m_bBossPatternGraphAttempted = true;
		m_strBossPatternGraphAttemptPatternId = pPattern->strPatternId;
		m_strBossPatternGraphAttemptCanonicalRevision =
			m_strPinnedCanonicalSourceRevision;
		m_iBossPatternGraphAttemptDraftGeneration =
			iPatternViewDraftGeneration;
		m_iBossPatternGraphAttemptRouteGeneration =
			m_iBossPatternRouteGeneration;
		const std::size_t iPreviousNodeCount =
			m_BossPatternGraphSnapshot.Nodes.size();
		const std::size_t iPreviousEdgeCount =
			m_BossPatternGraphSnapshot.Edges.size();
		const std::string PreviousPatternId =
			m_BossPatternGraphSnapshot.strPatternId;
		m_bBossPatternGraphReady = CActionCompositionGraphModel::Project(
			*pPattern, iPatternViewDraftGeneration,
			m_BossPatternOutcomeOverrides,
			m_BossPatternGraphSnapshot, m_BossPatternGraphError);
		if (m_bBossPatternGraphReady &&
			(PreviousPatternId != m_BossPatternGraphSnapshot.strPatternId ||
			 iPreviousNodeCount != m_BossPatternGraphSnapshot.Nodes.size() ||
			 iPreviousEdgeCount != m_BossPatternGraphSnapshot.Edges.size()))
		{
			m_bBossPatternFitRequested = true;
		}
	}

	ImGui::SeparatorText("Boss Pattern Graph");
	ImGui::Text("%s | %s", pPattern->strDisplayName.c_str(),
		pPattern->strPatternId.c_str());
	if (m_bAuthoringDraftDirty)
	{
		ImGui::SameLine();
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.25f, 1.f), "| UNSAVED PATTERN DRAFT");
	}
	ImGui::SameLine();
	const bool_t bCanSave = bPatternMutationAdmitted &&
		m_bAuthoringDraftDirty && nullptr != m_pBalanceTool;
	ImGui::BeginDisabled(!bCanSave);
	if (ImGui::SmallButton("Save Pattern##BossPattern"))
		m_bSavePatternRequested = true;
	ImGui::EndDisabled();

	const bool_t bCanDrawSnapshot =
		m_BossPatternGraphSnapshot.strPatternId == pPattern->strPatternId &&
		!m_BossPatternGraphSnapshot.Nodes.empty();
	if (!m_bBossPatternGraphReady)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.38f, 0.30f, 1.f),
			"GRAPH REJECTED: %s", m_BossPatternGraphError.strMessage.c_str());
		if (bCanDrawSnapshot)
			ImGui::TextDisabled("Showing the previous same-Pattern snapshot read-only.");
	}
	if (!bCanDrawSnapshot)
	{
		return;
	}

	const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Graph =
		m_BossPatternGraphSnapshot;
	bool_t bCanInteractSnapshot = m_bBossPatternGraphReady &&
		Graph.strPatternId == pPattern->strPatternId &&
		Graph.iSourceGeneration == iPatternViewDraftGeneration;
	ImGui::Text(
		"Default %llu ms | Selected %llu ms | Maximum %llu ms",
		static_cast<unsigned long long>(Graph.DefaultPath.iDurationMs),
		static_cast<unsigned long long>(Graph.SelectedPath.iDurationMs),
		static_cast<unsigned long long>(Graph.MaximumPath.iDurationMs));
	ImGui::SameLine();
	if (ImGui::SmallButton("Fit"))
		m_bBossPatternFitRequested = true;
	ImGui::SameLine();
	ImGui::BeginDisabled(m_BossPatternOutcomeOverrides.empty());
	if (ImGui::SmallButton("Reset Route"))
	{
		m_BossPatternOutcomeOverrides.clear();
		++m_iBossPatternRouteGeneration;
		bCanInteractSnapshot = false;
		m_strStatus =
			"Boss Pattern preview route reset to the authored default path.";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Zoom %.0f%%", m_fBossPatternZoom * 100.f);
	const auto SelectedStage = std::find_if(
		pPattern->Stages.begin(), pPattern->Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Stage)
		{ return Stage.strStageId == m_strSelectedStageId; });
	if (SelectedStage != pPattern->Stages.end())
	{
		ImGui::SeparatorText("Selected Stage Structure");
		if (Render_BossPatternStageTopologyControls(
				*pPattern, *SelectedStage, bPatternMutationAdmitted))
		{
			/* The writer advanced the live draft after this frame's immutable
			   Pattern view was chosen.  Do not hit-test the old projection again
			   before next-frame rematerialization. */
			bCanInteractSnapshot = false;
		}
	}
	else
	{
		ImGui::TextDisabled("Click a Stage node to open its typed structure controls.");
	}
	if (!bCanInteractSnapshot)
		ImGui::TextDisabled(
			"Canvas selection is disabled until this exact Pattern view projects successfully.");

	ImGui::SeparatorText("Canvas");
	ImVec2 CanvasSize = ImGui::GetContentRegionAvail();
	CanvasSize.x = (std::max)(CanvasSize.x, 240.f);
	CanvasSize.y = (std::max)(CanvasSize.y, BOSS_PATTERN_CANVAS_MIN_HEIGHT);
	const ImVec2 CanvasOrigin = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##BossPatternCanvas", CanvasSize);
	const bool_t bCanvasHovered = ImGui::IsItemHovered();
	ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
	pDrawList->AddRectFilled(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		IM_COL32(18, 22, 29, 255));
	pDrawList->AddRect(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		IM_COL32(70, 80, 96, 255));

	if (m_bBossPatternFitRequested)
	{
		const float GraphWidth = (std::max)(1.f,
			Graph.GraphBounds.fRight - Graph.GraphBounds.fLeft);
		const float GraphHeight = (std::max)(1.f,
			Graph.GraphBounds.fBottom - Graph.GraphBounds.fTop);
		m_fBossPatternZoom = (std::clamp)((std::min)(
			(CanvasSize.x - 80.f) / GraphWidth,
			(CanvasSize.y - 80.f) / GraphHeight),
			BOSS_PATTERN_MIN_ZOOM, BOSS_PATTERN_MAX_ZOOM);
		m_fBossPatternPanX = (CanvasSize.x - GraphWidth * m_fBossPatternZoom) *
			0.5f - Graph.GraphBounds.fLeft * m_fBossPatternZoom;
		m_fBossPatternPanY = (CanvasSize.y - GraphHeight * m_fBossPatternZoom) *
			0.5f - Graph.GraphBounds.fTop * m_fBossPatternZoom;
		m_bBossPatternFitRequested = false;
	}

	if (bCanvasHovered &&
		(ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f) ||
		 ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f)))
	{
		const ImVec2 Delta = ImGui::GetIO().MouseDelta;
		m_fBossPatternPanX += Delta.x;
		m_fBossPatternPanY += Delta.y;
	}
	if (bCanvasHovered && 0.f != ImGui::GetIO().MouseWheel)
	{
		const ImVec2 Mouse = ImGui::GetMousePos();
		const ACTION_COMPOSITION_GRAPH_POINT Before = ScreenToGraph(
			Mouse, CanvasOrigin, m_fBossPatternPanX,
			m_fBossPatternPanY, m_fBossPatternZoom);
		m_fBossPatternZoom = (std::clamp)(
			m_fBossPatternZoom *
				(1.f + ImGui::GetIO().MouseWheel * 0.12f),
			BOSS_PATTERN_MIN_ZOOM, BOSS_PATTERN_MAX_ZOOM);
		m_fBossPatternPanX = Mouse.x - CanvasOrigin.x -
			Before.fX * m_fBossPatternZoom;
		m_fBossPatternPanY = Mouse.y - CanvasOrigin.y -
			Before.fY * m_fBossPatternZoom;
	}

	pDrawList->PushClipRect(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		true);
	for (std::size_t iEdge = 0u; iEdge < Graph.Edges.size(); ++iEdge)
	{
		const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Graph.Edges[iEdge];
		const bool_t bSelectedPath = PathContainsEdge(Graph.SelectedPath, iEdge);
		const bool_t bAuthored = ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::
			AUTHORED_BRANCH == Edge.eOrigin;
		const ImU32 Color = bSelectedPath ? IM_COL32(92, 224, 142, 255) :
			("COUNTER_HIT" == Edge.strOutcome ? IM_COL32(70, 190, 232, 255) :
				(bAuthored ? IM_COL32(204, 126, 86, 255) :
					IM_COL32(108, 120, 138, 210)));
		std::vector<ImVec2> Points;
		Points.reserve(Edge.Polyline.size());
		for (const ACTION_COMPOSITION_GRAPH_POINT& Point : Edge.Polyline)
			Points.push_back(GraphToScreen(
				Point, CanvasOrigin, m_fBossPatternPanX,
				m_fBossPatternPanY, m_fBossPatternZoom));
		if (Points.size() >= 2u)
			pDrawList->AddPolyline(
				Points.data(), static_cast<int32_t>(Points.size()), Color,
				0, bSelectedPath ? 3.f : 2.f);
		DrawEdgeArrow(
			pDrawList, Edge.Polyline, CanvasOrigin,
			m_fBossPatternPanX, m_fBossPatternPanY,
			m_fBossPatternZoom, Color);
		if (!Points.empty())
		{
			const std::string Label = Edge.strOutcome +
				(Edge.strTargetPatternId.empty() ? std::string{} :
					" -> PATTERN " + Edge.strTargetPatternId) +
				(bAuthored ? std::string{} : " (derived)");
			pDrawList->AddText(
				ImVec2(Points.front().x + 6.f, Points.front().y - 17.f),
				Color, Label.c_str());
		}
	}

	for (std::size_t iNode = 0u; iNode < Graph.Nodes.size(); ++iNode)
	{
		const ACTION_COMPOSITION_GRAPH_NODE& Node = Graph.Nodes[iNode];
		const ImVec2 Minimum = GraphToScreen(
			{ Node.Bounds.fLeft, Node.Bounds.fTop }, CanvasOrigin,
			m_fBossPatternPanX, m_fBossPatternPanY, m_fBossPatternZoom);
		const ImVec2 Maximum = GraphToScreen(
			{ Node.Bounds.fRight, Node.Bounds.fBottom }, CanvasOrigin,
			m_fBossPatternPanX, m_fBossPatternPanY, m_fBossPatternZoom);
		const bool_t bSelected = Node.Key.strStageId == m_strSelectedStageId;
		const bool_t bSelectedPath = PathContainsNode(Graph.SelectedPath, iNode);
		const ImU32 Fill = !Node.bReachable ? IM_COL32(45, 46, 52, 220) :
			("WAIT" == Node.strSequenceRole ? IM_COL32(62, 66, 75, 255) :
				(Node.bEventEntered ? IM_COL32(92, 65, 124, 255) :
					IM_COL32(48, 74, 112, 255)));
		const ImU32 Border = bSelected ? IM_COL32(255, 205, 84, 255) :
			(bSelectedPath ? IM_COL32(92, 224, 142, 255) :
				IM_COL32(125, 143, 168, 255));
		pDrawList->AddRectFilled(Minimum, Maximum, Fill, 6.f);
		pDrawList->AddRect(
			Minimum, Maximum, Border, 6.f, 0, bSelected ? 3.f : 2.f);
		const ImVec2 TextOrigin(Minimum.x + 9.f, Minimum.y + 7.f);
		pDrawList->AddText(
			TextOrigin, IM_COL32(245, 248, 252, 255),
			Node.Key.strStageId.c_str());
		const std::string Role = Node.strSequenceRole + " / " +
			Node.strStageKind + " | " + std::to_string(Node.iDurationMs) + " ms";
		pDrawList->AddText(
			ImVec2(TextOrigin.x, TextOrigin.y + 20.f),
			IM_COL32(190, 204, 224, 255), Role.c_str());
		if (m_fBossPatternZoom >= 0.58f)
		{
			const std::string Counts =
				"Anim " + std::to_string(Node.iAnimationOccurrenceCount) +
				" | FX " + std::to_string(Node.iEffectReferenceCount) +
				" | Logic " + std::to_string(
					Node.iGameplayActionCount + Node.iBranchCount) +
				" | Cam " + std::to_string(Node.iCameraInvocationCount);
			pDrawList->AddText(
				ImVec2(TextOrigin.x, TextOrigin.y + 40.f),
				IM_COL32(155, 172, 194, 255), Counts.c_str());
		}
	}
	pDrawList->PopClipRect();

	if (bCanInteractSnapshot && bCanvasHovered &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		const ACTION_COMPOSITION_GRAPH_POINT GraphPoint = ScreenToGraph(
			ImGui::GetMousePos(), CanvasOrigin, m_fBossPatternPanX,
			m_fBossPatternPanY, m_fBossPatternZoom);
		const std::size_t iNode = CActionCompositionGraphModel::Hit_TestNode(
			Graph, GraphPoint);
		if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iNode &&
			iNode < Graph.Nodes.size())
		{
			const ACTION_COMPOSITION_GRAPH_NODE& Node = Graph.Nodes[iNode];
			if (Node.iCanonicalStageIndex < pPattern->Stages.size())
			{
				const VALTAN_STAGE_VIEW& Stage =
					pPattern->Stages[Node.iCanonicalStageIndex];
				if (pPattern->strPatternId == Node.Key.strPatternId &&
					Stage.strStageId == Node.Key.strStageId &&
					Stage.strActionId == Node.Key.strActionId)
				{
					m_bDetailsWindowVisible = true;
					Select_Stage(*pPattern, Stage,
						DETAIL_OWNER::GAMEPLAY_STAGE, Stage.strStageId);
					m_strStatus =
						"Boss Pattern node selected. Details owns its numeric tuning; Sequencer owns its timed resources.";
				}
			}
		}
		else
		{
			const std::size_t iEdge = CActionCompositionGraphModel::Hit_TestEdge(
				Graph, GraphPoint, 6.f / m_fBossPatternZoom);
			if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iEdge &&
				iEdge < Graph.Edges.size())
			{
				const ACTION_COMPOSITION_GRAPH_EDGE& Edge = Graph.Edges[iEdge];
				if (Edge.iSourceNodeIndex < Graph.Nodes.size())
				{
					const ACTION_COMPOSITION_GRAPH_NODE& Source =
						Graph.Nodes[Edge.iSourceNodeIndex];
					if (Source.iCanonicalStageIndex < pPattern->Stages.size())
					{
						const VALTAN_STAGE_VIEW& Stage =
							pPattern->Stages[Source.iCanonicalStageIndex];
						if (!MatchesBossPatternEdgeSource(
								Edge, Source, *pPattern, Stage))
						{
							m_strStatus =
								"Boss Pattern edge selection rejected: projected source identity no longer matches this Pattern view.";
						}
						else
						{
							const bool_t bAuthored =
								ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::
									AUTHORED_BRANCH == Edge.eOrigin;
							const std::string TargetLabel =
								!Edge.strTargetPatternId.empty() ?
									"PATTERN " + Edge.strTargetPatternId :
									(Edge.bTerminal ? "PATTERN END" :
										Edge.strTargetActionId);
							const std::string StableId = bAuthored ?
								Stage.strStageId + "/branch/" + Edge.strOutcome + "/" +
									TargetLabel :
								Stage.strStageId;
							m_bDetailsWindowVisible = true;
							Select_Stage(*pPattern, Stage,
								DETAIL_OWNER::GAMEPLAY_STAGE, StableId);

							const auto Override = std::find_if(
								m_BossPatternOutcomeOverrides.begin(),
								m_BossPatternOutcomeOverrides.end(),
								[&Edge](
									const ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE& Row)
								{ return Row.strActionId == Edge.strSourceActionId; });
							if (Override == m_BossPatternOutcomeOverrides.end())
							{
								m_BossPatternOutcomeOverrides.push_back({
									Edge.strSourceActionId, Edge.strOutcome });
							}
							else
							{
								Override->strOutcome = Edge.strOutcome;
							}
							++m_iBossPatternRouteGeneration;
							m_strStatus = bAuthored ?
								"Boss Pattern branch selected for preview. Details shows the typed edge; no branch bytes changed." :
								"Derived TIMEOUT/default route selected for preview. It is read-only and owns no independent JSON edge.";
						}
					}
				}
			}
		}
	}

	ImGui::TextDisabled(
		"Left click node/edge: select + preview route | wheel: zoom | right/middle drag: pan. Effect/Animation/Sound/Camera stay on Sequencer lanes instead of becoming graph wires.");
	if (!bMutationAdmitted)
		ImGui::TextDisabled("Graph is inspection-only until the canonical join is admitted.");
}
