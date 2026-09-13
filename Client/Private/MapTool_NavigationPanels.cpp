#include "imgui.h"
#include "MapTool_Internal.h"
#include "GameInstance.h"
#include "MapEditorWorkspaceService.h"
#include "MapNavigationContract.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include "Model.h"

#include "MapTool_NavigationResources.h"



void Client::CMapTool::Render_NavigationRegionControls()
{
	ImGui::SeparatorText("Grid");
	const EDITOR_AREA_DESCRIPTOR* activeArea =
		CMapEditorWorkspaceService::Is_Active() ?
		Get_ActiveEditorArea() : nullptr;
	const std::string& areaId = nullptr != activeArea ?
		activeArea->areaId : m_Catalog.Get_AreaId();
	const std::string baseLabel = areaId + " (base)";
	const std::string preview = m_NavigationRegionId.empty() ?
		baseLabel : m_NavigationRegionId;
	if (ImGui::BeginCombo("Region", preview.c_str()))
	{
		if (ImGui::Selectable(
			baseLabel.c_str(),
			m_NavigationRegionId.empty()))
		{
			Select_NavigationRegion(std::string{});
		}
		/* Selecting reloads the document, which refills m_NavigationRegions.
		   Copy what is needed and stop iterating the vector being replaced. */
		std::string pickedRegionId;
		f32_t pickedStepHeight = m_NewNavigationRegionStepHeight;
		for (const MAP_NAVIGATION_REGION& region : m_NavigationRegions)
		{
			if (ImGui::Selectable(
				region.regionId.c_str(),
				region.regionId == m_NavigationRegionId))
			{
				pickedRegionId = region.regionId;
				pickedStepHeight = region.stepHeight;
				break;
			}
		}
		ImGui::EndCombo();
		if (!pickedRegionId.empty())
		{
			m_NewNavigationRegionStepHeight = pickedStepHeight;
			Select_NavigationRegion(pickedRegionId);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Regions"))
	{
		std::string status;
		(void)CMapNavigationContract::Read_RegionManifest(
			areaId, m_NavigationRegions, status);
		m_NavigationStatus = status;
	}

	ImGui::InputText(
		"New Region ID",
		m_NewNavigationRegionId,
		sizeof(m_NewNavigationRegionId));
	ImGui::DragFloat(
		"Region Step Height",
		&m_NewNavigationRegionStepHeight,
		0.05f,
		0.f,
		10.f);
	const std::string newRegionId = m_NewNavigationRegionId;
	ImGui::BeginDisabled(
		!CMapNavigationContract::Is_ValidRegionId(newRegionId));
	if (ImGui::Button("Create Region"))
	{
		/* Selecting a region with no navsource yet fails on purpose: the panel
		   drops into Bake with empty documents, which is exactly the state
		   needed to place Nav Bounds for it. */
		Select_NavigationRegion(newRegionId);
		m_eNavigationMode = NAVIGATION_MODE::BAKE;
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"A region is a finer grid over part of this Area. Place Nav Bounds "
		"around the whole stage it covers, then Bake.");
}

void Client::CMapTool::Render_NavigationPanel()
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active ||
		EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy)
	{
		ImGui::TextDisabled(
			"Navigation authoring is disabled for this Area.");
		return;
	}

	Render_NavigationRegionControls();
	ImGui::Separator();

	if (ImGui::RadioButton(
		"Bake",
		NAVIGATION_MODE::BAKE == m_eNavigationMode))
	{
		m_eNavigationMode = NAVIGATION_MODE::BAKE;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	const bool_t navigationReady =
		m_NavigationDocument.Is_Ready() &&
		m_RuntimeBlockerDocument.Is_Ready();
	ImGui::BeginDisabled(!navigationReady);
	if (ImGui::RadioButton(
		"Walkability",
		NAVIGATION_MODE::WALKABILITY == m_eNavigationMode))
	{
		m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Destruction Area",
		NAVIGATION_MODE::DESTRUCTION_AREA ==
		m_eNavigationMode))
	{
		m_eNavigationMode =
			NAVIGATION_MODE::DESTRUCTION_AREA;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::EndDisabled();

	if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
	{
		ImGui::Separator();
		Render_NavigationBakeControls();
		return;
	}

	if (!navigationReady)
	{
		ImGui::Separator();
		ImGui::TextUnformatted(
			"Navigation source is unavailable. Bake Nav Bounds first.");
		if (ImGui::Button("Retry Load"))
			Load_NavigationDocument();
		ImGui::TextWrapped("%s", m_NavigationStatus.c_str());
		return;
	}

	ImGui::Separator();

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
		Render_DestructionAreaControls();

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		if (ImGui::RadioButton(
			"Add Cells",
			NAVIGATION_EDIT_ACTION::APPLY ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::APPLY;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Remove Cells",
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::ERASE;
		}
	}
	else
	{
		if (ImGui::RadioButton(
			"Block",
			NAVIGATION_EDIT_ACTION::APPLY ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::APPLY;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Force Walkable",
			NAVIGATION_EDIT_ACTION::FORCE_WALKABLE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::FORCE_WALKABLE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(
			"Reset",
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction))
		{
			m_eNavigationEditAction =
				NAVIGATION_EDIT_ACTION::ERASE;
		}
		ImGui::TextDisabled(
			"Force Walkable overrides baked blocked cells. Reset restores the baked state.");
		if (NAVIGATION_EDIT_ACTION::FORCE_WALKABLE == m_eNavigationEditAction)
		{
			ImGui::Checkbox("Use Picked Height", &m_bNavigationUsePickedHeight);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"\xEC\xBC\x9C\xEB\xA9\xB4\x20\xEB\xB8\x8C\xEB\x9F\xAC\xEC\x8B\x9C\x20\xEB\xB2\x94"
					"\xEC\x9C\x84\xEC\x9D\x98\x20\xEA\xB8\xB0\xEC\xA1\xB4\x20\xEB\x84\xA4\xEB\xB9\x84"
					"\x20\xEB\x86\x92\xEC\x9D\xB4\xEB\x8F\x84\x20\xED\x81\xB4\xEB\xA6\xAD\xED\x95\x9C"
					"\x20\xED\x91\x9C\xEB\xA9\xB4\x20\xEB\x86\x92\xEC\x9D\xB4\xEB\xA1\x9C\x20\xEB\xB0"
					"\x94\xEA\xBF\x89\xEB\x8B\x88\xEB\x8B\xA4\x2E\x0A"
					"\xEC\x9C\x84\xEC\x95\x84\xEB\x9E\x98\x20\xEB\xB0\x9C\xED\x8C\x90\xEC\x9D\xB4\x20"
					"\xEA\xB2\xB9\xEC\xB9\x98\xEB\x8A\x94\x20\xEA\xB3\xB3\xEC\x9D\x80\x20\x42\x72\x75"
					"\x73\x68\x20\x30\xEB\xB6\x80\xED\x84\xB0\x20\xED\x99\x95\xEC\x9D\xB8\xED\x95\x98"
					"\xEC\x84\xB8\xEC\x9A\x94\x2E\x20\x52\x65\x73\x65\x74\xEC\x9D\x80\x20\xEC\x9B\x90"
					"\xEB\x9E\x98\x20\xEB\xB2\xA0\xEC\x9D\xB4\xED\x81\xAC\x20\xEC\x83\x81\xED\x83\x9C"
					"\xEB\xA1\x9C\x20\xEB\x90\x98\xEB\x8F\x8C\xEB\xA6\xBD\xEB\x8B\x88\xEB\x8B\xA4\x2E");
			ImGui::TextWrapped(
				"\xEB\xB9\x88\x20\xEC\x85\x80\xEC\x97\x90\xEB\x8A\x94\x20\xED\x81\xB4\xEB\xA6\xAD"
				"\xED\x95\x9C\x20\xEB\x86\x92\xEC\x9D\xB4\xEB\xA1\x9C\x20\xEB\x84\xA4\xEB\xB9\x84"
				"\xEB\xA5\xBC\x20\xEC\xB6\x94\xEA\xB0\x80\xED\x95\xA9\xEB\x8B\x88\xEB\x8B\xA4\x2E"
				"\x20\xEA\xB8\xB0\xEC\xA1\xB4\x20\xEC\x85\x80\x20\xEB\x86\x92\xEC\x9D\xB4\xEA\xB9"
				"\x8C\xEC\xA7\x80\x20\xEB\xB0\x94\xEA\xBE\xB8\xEB\xA0\xA4\xEB\xA9\xB4\x20\x55\x73"
				"\x65\x20\x50\x69\x63\x6B\x65\x64\x20\x48\x65\x69\x67\x68\x74\xEB\xA5\xBC\x20\xEC"
				"\xBC\x9C\xEC\x84\xB8\xEC\x9A\x94\x2E");
		}
	}

	int32_t brushRadius =
		static_cast<int32_t>(m_iBrushRadius);
	if (ImGui::SliderInt(
		"Brush",
		&brushRadius,
		0,
		static_cast<int32_t>(
			CNavGridPaintDocument::MAX_BRUSH_RADIUS)))
	{
		m_iBrushRadius =
			static_cast<uint32_t>(brushRadius);
	}

	ImGui::Checkbox(
		"Show empty cells",
		&m_bShowUnresolvedCells);

	ImGui::Separator();
	if (ImGui::Button("Save Navigation"))
		Save_Navigation();
	ImGui::SameLine();

	const bool_t dirty =
		m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty();
	ImGui::TextUnformatted(
		dirty ? "Unsaved" : m_NavigationStatus.c_str());
	if (dirty && "Unsaved" != m_NavigationStatus)
		ImGui::TextWrapped("%s", m_NavigationStatus.c_str());

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		ImGui::TextDisabled(
			"Magenta: selected destruction area");
	}
	else
	{
		ImGui::TextDisabled(
			"Cell %.2f x %.2f | Green: walkable | Yellow: blocked",
			m_NavigationDocument.Get_Desc().cellSize,
			m_NavigationDocument.Get_Desc().cellSize);
	}

	Render_NavigationDiagnostics();
}

void Client::CMapTool::Render_NavigationDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	const uint32_t surfaceCells =
		m_NavigationDocument.Get_ResolvedHeightCount();
	ImGui::Text(
		"Surface: %u | Excluded: %u | Blocked overrides: %u | Walkable overrides: %u",
		surfaceCells,
		m_NavigationDocument.Get_CellCount() - surfaceCells,
		m_NavigationDocument.Get_BlockedCount(),
		m_NavigationDocument.Get_ForcedWalkableCount());
	ImGui::TextWrapped(
		"Source: %s",
		m_NavigationSourcePath.string().c_str());
	ImGui::TextWrapped(
		"Paint: %s",
		m_NavigationPaintPath.string().c_str());
	ImGui::TextWrapped(
		"Runtime: %s",
		m_NavigationRuntimePath.string().c_str());
	ImGui::TextWrapped(
		"Blockers: %s",
		m_RuntimeBlockerPath.string().c_str());

	if (ImGui::Button("Reload from Disk"))
	{
		const bool_t dirty =
			m_NavigationDocument.Is_Dirty() ||
			m_RuntimeBlockerDocument.Is_Dirty();
		if (dirty)
			ImGui::OpenPopup("Discard Navigation Changes?");
		else
			Load_NavigationDocument();
	}

	if (ImGui::BeginPopupModal(
		"Discard Navigation Changes?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload and discard unsaved navigation changes?");
		if (ImGui::Button("Discard and Reload"))
		{
			Load_NavigationDocument();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Client::CMapTool::Render_NavigationOverlay()
{
	/* Bake mode shows its staged grid before anything is written. Other modes
	   must show the live paint even when an unapplied preview is kept. A region
	   baked for the first time needs only the preview to draw in Bake mode. */
	const bool_t drawPreview = NAVIGATION_MODE::BAKE == m_eNavigationMode &&
		m_bNavigationBakePreviewReady;
	if ((!m_NavigationDocument.Is_Ready() && !drawPreview) ||
		nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const float4_t green(0.1f, 1.f, 0.2f, 1.f);
	const float4_t yellow(1.f, 0.85f, 0.05f, 1.f);
	const float4_t magenta(1.f, 0.15f, 0.85f, 1.f);
	/* Preview cells read cyan and orange so a staged grid is never mistaken
	   for the one currently saved. */
	const float4_t previewWalkable(0.15f, 0.95f, 0.85f, 1.f);
	const float4_t previewBlocked(1.f, 0.55f, 0.1f, 1.f);
	const NAVGRID_AUTHORING_DESC& desc = drawPreview ?
		m_NavigationBakePreview.desc :
		m_NavigationDocument.Get_Desc();
	const f32_t halfCell = desc.cellSize * 0.5f;
	const uint32_t overlayCellCount = drawPreview ?
		static_cast<uint32_t>(m_NavigationBakePreview.cells.size()) :
		m_NavigationDocument.Get_CellCount();

	resources.pBatch->Begin();
	for (uint32_t index = 0; index < overlayCellCount; ++index)
	{
		const NAVGRID_AUTHORING_CELL_STATE state = drawPreview ?
			(!m_NavigationBakePreview.cells[index].surfaceResolved ?
				NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE :
				m_NavigationBakePreview.cells[index].baseWalkable ?
				NAVGRID_AUTHORING_CELL_STATE::WALKABLE :
				NAVGRID_AUTHORING_CELL_STATE::BLOCKED) :
			m_NavigationDocument.Get_CellState(index);
		if (!m_bShowUnresolvedCells &&
			NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE == state)
		{
			continue;
		}

		const uint32_t cellX = index % desc.width;
		const uint32_t cellZ = index / desc.width;
		const f32_t worldX =
			desc.originX +
			(static_cast<f32_t>(cellX) + 0.5f) * desc.cellSize;
		const f32_t worldZ =
			desc.originZ +
			(static_cast<f32_t>(cellZ) + 0.5f) * desc.cellSize;
		if (drawPreview)
		{
			/* The grid is the axis-aligned box of a possibly rotated bounds,
			   so its corners can fall outside the box the user placed. The
			   staged desc carries those bounds; the saved document does not
			   describe this grid at all yet. */
			const f32_t previewRadians =
				XMConvertToRadians(desc.bake.yawDegrees);
			const f32_t previewCosine = std::cos(previewRadians);
			const f32_t previewSine = std::sin(previewRadians);
			const f32_t offsetX = worldX - desc.bake.position.x;
			const f32_t offsetZ = worldZ - desc.bake.position.z;
			const f32_t localX =
				previewCosine * offsetX - previewSine * offsetZ;
			const f32_t localZ =
				previewSine * offsetX + previewCosine * offsetZ;
			if (std::abs(localX) > desc.bake.size.x * 0.5f + 0.00001f ||
				std::abs(localZ) > desc.bake.size.z * 0.5f + 0.00001f)
			{
				continue;
			}
		}
		else if (!Is_CellInsideNavigationBounds(worldX, worldZ))
		{
			continue;
		}

		const f32_t displayHeight =
			NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE == state ?
			desc.bake.position.y - desc.bake.size.y * 0.5f :
			drawPreview ?
			m_NavigationBakePreview.cells[index].height :
			m_NavigationDocument.Get_CellHeight(index);
		const float3_t center(
			worldX,
			displayHeight + 0.08f,
			worldZ);

		const bool_t isSelectedRuntimeCell =
			!drawPreview &&
			NAVIGATION_MODE::DESTRUCTION_AREA ==
				m_eNavigationMode &&
			m_RuntimeBlockerDocument.Is_CellInRegion(
				m_iSelectedRuntimeRegion,
				index);
		const float4_t& color = isSelectedRuntimeCell ?
			magenta :
			drawPreview ?
			(NAVGRID_AUTHORING_CELL_STATE::WALKABLE != state ?
				previewBlocked : previewWalkable) :
			NAVGRID_AUTHORING_CELL_STATE::WALKABLE != state ?
			yellow :
			green;
		const VertexPositionColor leftTop(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightTop(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightBottom(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z - halfCell),
			color);
		const VertexPositionColor leftBottom(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z - halfCell),
			color);

		resources.pBatch->DrawLine(leftTop, rightTop);
		resources.pBatch->DrawLine(rightTop, rightBottom);
		resources.pBatch->DrawLine(rightBottom, leftBottom);
		resources.pBatch->DrawLine(leftBottom, leftTop);
	}
	resources.pBatch->End();
}

void Client::CMapTool::Render_NavigationBoundsOverlay()
{
	if (!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc) ||
		nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const f32_t radians =
		XMConvertToRadians(m_NavigationBakeDesc.yawDegrees);
	const f32_t cosine = std::cos(radians);
	const f32_t sine = std::sin(radians);
	const float3_t half(
		m_NavigationBakeDesc.size.x * 0.5f,
		m_NavigationBakeDesc.size.y * 0.5f,
		m_NavigationBakeDesc.size.z * 0.5f);
	const std::array<float3_t, 8> local =
	{{
		{ -half.x, -half.y, -half.z },
		{ half.x, -half.y, -half.z },
		{ half.x, -half.y, half.z },
		{ -half.x, -half.y, half.z },
		{ -half.x, half.y, -half.z },
		{ half.x, half.y, -half.z },
		{ half.x, half.y, half.z },
		{ -half.x, half.y, half.z },
	}};
	std::array<VertexPositionColor, 8> vertices;
	const float4_t white(1.f, 1.f, 1.f, 1.f);
	for (size_t index = 0; index < local.size(); ++index)
	{
		const float3_t world(
			m_NavigationBakeDesc.position.x +
				cosine * local[index].x +
				sine * local[index].z,
			m_NavigationBakeDesc.position.y +
				local[index].y,
			m_NavigationBakeDesc.position.z -
				sine * local[index].x +
				cosine * local[index].z);
		vertices[index] =
			VertexPositionColor(world, white);
	}

	constexpr std::array<std::array<uint32_t, 2>, 12> edges =
	{{
		{{ 0, 1 }}, {{ 1, 2 }}, {{ 2, 3 }}, {{ 3, 0 }},
		{{ 4, 5 }}, {{ 5, 6 }}, {{ 6, 7 }}, {{ 7, 4 }},
		{{ 0, 4 }}, {{ 1, 5 }}, {{ 2, 6 }}, {{ 3, 7 }},
	}};
	resources.pBatch->Begin();
	for (const auto& edge : edges)
	{
		resources.pBatch->DrawLine(
			vertices[edge[0]],
			vertices[edge[1]]);
	}
	resources.pBatch->End();
}
