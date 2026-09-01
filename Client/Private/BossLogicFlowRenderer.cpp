#include "imgui.h"

#include "BossLogicFlowView.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
	constexpr float MIN_ZOOM = 0.35f;
	constexpr float MAX_ZOOM = 2.50f;

	ImVec2 GraphToScreen(
		const Client::ACTION_COMPOSITION_GRAPH_POINT& Point,
		const ImVec2& Origin,
		const Client::BOSS_LOGIC_FLOW_CANVAS_STATE& State)
	{
		return ImVec2(
			Origin.x + State.fPanX + Point.fX * State.fZoom,
			Origin.y + State.fPanY + Point.fY * State.fZoom);
	}

	Client::ACTION_COMPOSITION_GRAPH_POINT ScreenToGraph(
		const ImVec2& Point,
		const ImVec2& Origin,
		const Client::BOSS_LOGIC_FLOW_CANVAS_STATE& State)
	{
		return {
			(Point.x - Origin.x - State.fPanX) / State.fZoom,
			(Point.y - Origin.y - State.fPanY) / State.fZoom };
	}

	void DrawArrow(
		ImDrawList* const pDrawList,
		const std::vector<Client::ACTION_COMPOSITION_GRAPH_POINT>& Polyline,
		const ImVec2& Origin,
		const Client::BOSS_LOGIC_FLOW_CANVAS_STATE& State,
		const ImU32 Color)
	{
		if (nullptr == pDrawList || Polyline.size() < 2u)
			return;
		const ImVec2 Tip = GraphToScreen(Polyline.back(), Origin, State);
		const ImVec2 Previous = GraphToScreen(
			Polyline[Polyline.size() - 2u], Origin, State);
		const float DeltaX = Tip.x - Previous.x;
		const float DeltaY = Tip.y - Previous.y;
		const float Length = std::sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
		if (Length <= 0.001f)
			return;
		const float DirectionX = DeltaX / Length;
		const float DirectionY = DeltaY / Length;
		const ImVec2 Base(
			Tip.x - DirectionX * 9.f,
			Tip.y - DirectionY * 9.f);
		const ImVec2 Perpendicular(-DirectionY, DirectionX);
		pDrawList->AddTriangleFilled(
			Tip,
			ImVec2(Base.x + Perpendicular.x * 4.5f,
				Base.y + Perpendicular.y * 4.5f),
			ImVec2(Base.x - Perpendicular.x * 4.5f,
				Base.y - Perpendicular.y * 4.5f),
			Color);
	}

	std::string ShortLabel(const std::string& Value)
	{
		constexpr std::size_t MAX_LABEL_BYTES = 38u;
		if (Value.size() <= MAX_LABEL_BYTES)
			return Value;
		return Value.substr(0u, MAX_LABEL_BYTES - 3u) + "...";
	}
}

bool Client::CBossLogicFlowRenderer::Render(
	const BOSS_LOGIC_FLOW_VIEW& View,
	const BOSS_LOGIC_FLOW_RENDER_CONTEXT& Context,
	BOSS_LOGIC_FLOW_CANVAS_STATE& InOutState,
	BOSS_LOGIC_FLOW_SELECTION& OutSelection)
{
	OutSelection.Clear();
	if (View.strPatternId.empty() || View.Graph.Nodes.empty() ||
		View.Nodes.size() != View.Graph.Nodes.size() ||
		View.Edges.size() != View.Graph.Edges.size())
	{
		ImGui::TextDisabled("No admitted Boss Logic Flow Pattern is selected.");
		return false;
	}

	ImGui::PushID(View.strPatternId.c_str());
	if (InOutState.strPatternId != View.strPatternId ||
		InOutState.iSourceGeneration != View.Graph.iSourceGeneration)
	{
		InOutState.strPatternId = View.strPatternId;
		InOutState.iSourceGeneration = View.Graph.iSourceGeneration;
		InOutState.bFitRequested = true;
	}

	ImGui::Text("%s | %s",
		View.strPatternDisplayName.empty() ? View.strPatternId.c_str() :
			View.strPatternDisplayName.c_str(),
		View.strPatternId.c_str());
	ImGui::SameLine();
	if (ImGui::SmallButton("Fit##BossLogicFlow"))
		InOutState.bFitRequested = true;
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Default %llu ms | Maximum %llu ms | Zoom %.0f%%",
		static_cast<unsigned long long>(View.Graph.DefaultPath.iDurationMs),
		static_cast<unsigned long long>(View.Graph.MaximumPath.iDurationMs),
		InOutState.fZoom * 100.f);

	ImVec2 CanvasSize = ImGui::GetContentRegionAvail();
	CanvasSize.x = (std::max)(CanvasSize.x, 320.f);
	CanvasSize.y = (std::max)(CanvasSize.y, Context.fMinimumCanvasHeight);
	const ImVec2 CanvasOrigin = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##BossLogicFlowCanvas", CanvasSize);
	const bool bCanvasHovered = ImGui::IsItemHovered();
	ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
	pDrawList->AddRectFilled(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		IM_COL32(18, 22, 29, 255));
	pDrawList->AddRect(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		IM_COL32(70, 80, 96, 255));

	if (InOutState.bFitRequested)
	{
		const float GraphWidth = (std::max)(1.f,
			View.Graph.GraphBounds.fRight - View.Graph.GraphBounds.fLeft);
		const float GraphHeight = (std::max)(1.f,
			View.Graph.GraphBounds.fBottom - View.Graph.GraphBounds.fTop);
		InOutState.fZoom = (std::clamp)((std::min)(
			(CanvasSize.x - 80.f) / GraphWidth,
			(CanvasSize.y - 80.f) / GraphHeight), MIN_ZOOM, MAX_ZOOM);
		InOutState.fPanX = (CanvasSize.x - GraphWidth * InOutState.fZoom) *
			0.5f - View.Graph.GraphBounds.fLeft * InOutState.fZoom;
		InOutState.fPanY = (CanvasSize.y - GraphHeight * InOutState.fZoom) *
			0.5f - View.Graph.GraphBounds.fTop * InOutState.fZoom;
		InOutState.bFitRequested = false;
	}

	if (bCanvasHovered &&
		(ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f) ||
		 ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f)))
	{
		const ImVec2 Delta = ImGui::GetIO().MouseDelta;
		InOutState.fPanX += Delta.x;
		InOutState.fPanY += Delta.y;
	}
	if (bCanvasHovered && 0.f != ImGui::GetIO().MouseWheel)
	{
		const ImVec2 Mouse = ImGui::GetMousePos();
		const ACTION_COMPOSITION_GRAPH_POINT Before = ScreenToGraph(
			Mouse, CanvasOrigin, InOutState);
		InOutState.fZoom = (std::clamp)(
			InOutState.fZoom * (1.f + ImGui::GetIO().MouseWheel * 0.12f),
			MIN_ZOOM, MAX_ZOOM);
		InOutState.fPanX = Mouse.x - CanvasOrigin.x -
			Before.fX * InOutState.fZoom;
		InOutState.fPanY = Mouse.y - CanvasOrigin.y -
			Before.fY * InOutState.fZoom;
	}

	pDrawList->PushClipRect(
		CanvasOrigin,
		ImVec2(CanvasOrigin.x + CanvasSize.x, CanvasOrigin.y + CanvasSize.y),
		true);
	for (std::size_t iEdge = 0u; iEdge < View.Graph.Edges.size(); ++iEdge)
	{
		const ACTION_COMPOSITION_GRAPH_EDGE& GraphEdge = View.Graph.Edges[iEdge];
		const BOSS_LOGIC_FLOW_EDGE_VIEW& Edge = View.Edges[iEdge];
		const ImU32 Color = Edge.bCounterSuccess ?
			IM_COL32(70, 190, 232, 255) :
			(Edge.bCounterTimeout ? IM_COL32(236, 142, 76, 255) :
				(Edge.bAuthored ? IM_COL32(188, 132, 92, 255) :
					IM_COL32(108, 120, 138, 210)));
		std::vector<ImVec2> Points;
		Points.reserve(GraphEdge.Polyline.size());
		for (const ACTION_COMPOSITION_GRAPH_POINT& Point : GraphEdge.Polyline)
			Points.push_back(GraphToScreen(Point, CanvasOrigin, InOutState));
		if (Points.size() >= 2u)
		{
			pDrawList->AddPolyline(
				Points.data(), static_cast<int>(Points.size()), Color, 0,
				Edge.bCounterSuccess || Edge.bCounterTimeout ? 3.f : 2.f);
		}
		DrawArrow(
			pDrawList, GraphEdge.Polyline, CanvasOrigin, InOutState, Color);
		if (!Points.empty())
		{
			const std::string Label = GraphEdge.strOutcome +
				(Edge.bAuthored ? std::string{} : " (derived)");
			pDrawList->AddText(
				ImVec2(Points.front().x + 6.f, Points.front().y - 17.f),
				Color, Label.c_str());
		}
	}

	for (std::size_t iNode = 0u; iNode < View.Graph.Nodes.size(); ++iNode)
	{
		const ACTION_COMPOSITION_GRAPH_NODE& GraphNode = View.Graph.Nodes[iNode];
		const BOSS_LOGIC_FLOW_NODE_VIEW& Node = View.Nodes[iNode];
		const ImVec2 Minimum = GraphToScreen(
			{ GraphNode.Bounds.fLeft, GraphNode.Bounds.fTop },
			CanvasOrigin, InOutState);
		const ImVec2 Maximum = GraphToScreen(
			{ GraphNode.Bounds.fRight, GraphNode.Bounds.fBottom },
			CanvasOrigin, InOutState);
		const bool bSelected = Node.strStageId == Context.strSelectedStageId;
		const bool bLive = Context.bLivePattern &&
			Node.strActionId == Context.strLiveActionId;
		const ImU32 Fill = Node.bCounterWindow ? IM_COL32(38, 78, 92, 255) :
			("WAIT" == Node.strSequenceRole ? IM_COL32(62, 66, 75, 255) :
				(GraphNode.bEventEntered ? IM_COL32(92, 65, 124, 255) :
					IM_COL32(48, 74, 112, 255)));
		const ImU32 Border = bLive ? IM_COL32(92, 224, 142, 255) :
			(bSelected ? IM_COL32(255, 205, 84, 255) :
				(Node.bCounterContractIncomplete ? IM_COL32(242, 88, 75, 255) :
					IM_COL32(125, 143, 168, 255)));
		pDrawList->AddRectFilled(Minimum, Maximum, Fill, 6.f);
		pDrawList->AddRect(
			Minimum, Maximum, Border, 6.f, 0,
			bLive || bSelected ? 3.f : 2.f);

		const ImVec2 TextOrigin(Minimum.x + 9.f, Minimum.y + 7.f);
		const std::string StageTitle =
			(Node.strStageKind.empty() ? Node.strSequenceRole : Node.strStageKind) +
			" | " + Node.strStageId;
		pDrawList->AddText(
			TextOrigin, IM_COL32(245, 248, 252, 255), StageTitle.c_str());
		if (Node.bCounterWindow)
		{
			pDrawList->AddText(
				ImVec2(Maximum.x - 68.f, Minimum.y + 7.f),
				IM_COL32(105, 224, 246, 255), "COUNTER");
		}
		else if (Node.bCounterContractIncomplete)
		{
			pDrawList->AddText(
				ImVec2(Maximum.x - 88.f, Minimum.y + 7.f),
				IM_COL32(255, 120, 105, 255), "COUNTER ?");
		}
		if (InOutState.fZoom >= 0.55f)
		{
			const std::string Role = Node.strSequenceRole + " | " +
				std::to_string(GraphNode.iDurationMs) + " ms";
			pDrawList->AddText(
				ImVec2(TextOrigin.x, TextOrigin.y + 18.f),
				IM_COL32(190, 204, 224, 255), Role.c_str());
			if (Node.ClipNames.empty())
			{
				pDrawList->AddText(
					ImVec2(TextOrigin.x, TextOrigin.y + 38.f),
					IM_COL32(145, 158, 176, 255), "Animation NONE");
			}
			else
			{
				const std::size_t iVisibleClips = (std::min)(
					Node.ClipNames.size(), std::size_t{ 3u });
				for (std::size_t iClip = 0u; iClip < iVisibleClips; ++iClip)
				{
					std::string Label = ShortLabel(Node.ClipNames[iClip]);
					if (2u == iClip && Node.ClipNames.size() > iVisibleClips)
					{
						Label += " +" + std::to_string(
							Node.ClipNames.size() - iVisibleClips);
					}
					pDrawList->AddText(
						ImVec2(TextOrigin.x, TextOrigin.y + 38.f +
							static_cast<float>(iClip) * 16.f),
						IM_COL32(168, 214, 244, 255), Label.c_str());
				}
			}
			if (Node.bCounterProxy)
			{
				pDrawList->AddText(
					ImVec2(Maximum.x - 54.f, Maximum.y - 18.f),
					IM_COL32(132, 208, 220, 255), "PROXY");
			}
		}
	}
	pDrawList->PopClipRect();

	if (Context.bAllowSelection && bCanvasHovered &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		const ACTION_COMPOSITION_GRAPH_POINT GraphPoint = ScreenToGraph(
			ImGui::GetMousePos(), CanvasOrigin, InOutState);
		const std::size_t iNode = CActionCompositionGraphModel::Hit_TestNode(
			View.Graph, GraphPoint);
		if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iNode &&
			iNode < View.Nodes.size())
		{
			const BOSS_LOGIC_FLOW_NODE_VIEW& Node = View.Nodes[iNode];
			OutSelection.eKind = BOSS_LOGIC_FLOW_SELECTION_KIND::STAGE;
			OutSelection.strPatternId = View.strPatternId;
			OutSelection.strStageId = Node.strStageId;
			OutSelection.strActionId = Node.strActionId;
		}
		else
		{
			const std::size_t iEdge = CActionCompositionGraphModel::Hit_TestEdge(
				View.Graph, GraphPoint, 6.f / InOutState.fZoom);
			if (ACTION_COMPOSITION_GRAPH_INVALID_INDEX != iEdge &&
				iEdge < View.Graph.Edges.size())
			{
				const ACTION_COMPOSITION_GRAPH_EDGE& Edge =
					View.Graph.Edges[iEdge];
				if (Edge.iSourceNodeIndex < View.Nodes.size())
				{
					const BOSS_LOGIC_FLOW_NODE_VIEW& Source =
						View.Nodes[Edge.iSourceNodeIndex];
					OutSelection.eKind =
						BOSS_LOGIC_FLOW_SELECTION_KIND::BRANCH;
					OutSelection.strPatternId = View.strPatternId;
					OutSelection.strStageId = Source.strStageId;
					OutSelection.strActionId = Source.strActionId;
					OutSelection.strOutcome = Edge.strOutcome;
					OutSelection.strTargetActionId = Edge.strTargetActionId;
					OutSelection.bTerminal = Edge.bTerminal;
					OutSelection.bAuthored = View.Edges[iEdge].bAuthored;
				}
			}
		}
	}

	if (Context.bAllowSelection)
	{
		ImGui::TextDisabled(
			"Inspect: left click Stage/branch | wheel zoom | right/middle drag pan. Blue = COUNTER_HIT, orange = authored Counter TIMEOUT.");
	}
	else
	{
		ImGui::TextDisabled(
			"Read only: wheel zoom | right/middle drag pan. Blue = COUNTER_HIT, orange = authored Counter TIMEOUT.");
	}
	ImGui::PopID();
	return BOSS_LOGIC_FLOW_SELECTION_KIND::NONE != OutSelection.eKind;
}
