#ifdef _DEBUG
#include "imgui.h"
#endif
#include "LevelNavigationDebug.h"

#ifdef _DEBUG
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "MapNavigationContract.h"
#include "NavGridPaintDocument.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <iomanip>
#include <set>

namespace
{
	using namespace Client;
	using namespace Engine;
	constexpr uint32_t MAX_DRAW_CELLS = 4000;
	constexpr float MAX_CELL_RADIUS = 70.f; // At most 143 x 143 candidates per frame.

	bool Contains(const CNavGrid::NAVGRID_DESC& desc, float x, float z)
	{
		return x >= desc.fOriginX && z >= desc.fOriginZ &&
			x < desc.fOriginX + desc.iWidth * desc.fCellSize &&
			z < desc.fOriginZ + desc.iHeight * desc.fCellSize;
	}

	// CMapNavigationContract::Read_RegionManifest reads authoring. The overlay
	// instead inspects the published manifest beside the actual .navgrid files.
	bool ReadPublishedRegions(const MAP_NAVIGATION_CONTRACT& base,
		std::vector<MAP_NAVIGATION_REGION>& regions, std::string& status)
	{
		auto path = base.runtimePath;
		path.replace_extension(L".navregions");
		std::error_code error;
		if (!std::filesystem::exists(path, error) && !error)
			return true;
		std::ifstream input(path);
		std::string magic, area;
		uint32_t version = 0, count = 0;
		if (error || !(input >> magic >> version >> std::quoted(area) >> count) ||
			magic != "LOSTARK_NAVGRID_REGIONS" || version != 1 || area != base.areaId || count > 64)
		{ status = "Published navigation region manifest is invalid: " + path.string(); return false; }
		std::set<std::string> names;
		for (uint32_t index = 0; index < count; ++index)
		{
			MAP_NAVIGATION_REGION row;
			if (!(input >> magic >> std::quoted(row.regionId) >> row.stepHeight) ||
				magic != "REGION" || !CMapNavigationContract::Is_ValidRegionId(row.regionId) ||
				!std::isfinite(row.stepHeight) || row.stepHeight < 0.f ||
				!names.insert(row.regionId).second)
			{ status = "Published navigation region row is invalid: " + path.string(); return false; }
			regions.push_back(std::move(row));
		}
		input >> std::ws;
		if (!input.eof())
		{ status = "Published navigation region manifest has trailing data: " + path.string(); return false; }
		return true;
	}

	float ClipDistance(const float4_t& point, int plane)
	{
		switch (plane)
		{
		case 0: return point.w + point.x;
		case 1: return point.w - point.x;
		case 2: return point.w + point.y;
		case 3: return point.w - point.y;
		case 4: return point.z;
		default: return point.w - point.z;
		}
	}

	bool ClipLine(float4_t& first, float4_t& second)
	{
		for (int plane = 0; plane < 6; ++plane)
		{
			const float a = ClipDistance(first, plane), b = ClipDistance(second, plane);
			if (a < 0.f && b < 0.f) return false;
			if ((a < 0.f) != (b < 0.f))
			{
				float4_t cut;
				XMStoreFloat4(&cut, XMVectorLerp(XMLoadFloat4(&first), XMLoadFloat4(&second), a / (a - b)));
				if (a < 0.f) first = cut; else second = cut;
			}
		}
		return first.w > 0.00001f && second.w > 0.00001f;
	}

	bool Finite(const float4_t& point)
	{
		return std::isfinite(point.x) && std::isfinite(point.y) &&
			std::isfinite(point.z) && std::isfinite(point.w);
	}

	using RECT_XZ = std::array<float, 4>;
	void SubtractRectangle(std::vector<RECT_XZ>& pieces, const CNavGrid::NAVGRID_DESC& desc)
	{
		const RECT_XZ cut{ desc.fOriginX, desc.fOriginZ,
			desc.fOriginX + desc.iWidth * desc.fCellSize, desc.fOriginZ + desc.iHeight * desc.fCellSize };
		if (std::none_of(pieces.begin(), pieces.end(), [&cut](const RECT_XZ& piece)
			{ return piece[0] < cut[2] && cut[0] < piece[2] && piece[1] < cut[3] && cut[1] < piece[3]; }))
			return;
		std::vector<RECT_XZ> remaining;
		for (const auto& piece : pieces)
		{
			const float x0 = (std::max)(piece[0], cut[0]), z0 = (std::max)(piece[1], cut[1]);
			const float x1 = (std::min)(piece[2], cut[2]), z1 = (std::min)(piece[3], cut[3]);
			if (x0 >= x1 || z0 >= z1) { remaining.push_back(piece); continue; }
			if (piece[0] < x0) remaining.push_back({ piece[0], piece[1], x0, piece[3] });
			if (x1 < piece[2]) remaining.push_back({ x1, piece[1], piece[2], piece[3] });
			if (piece[1] < z0) remaining.push_back({ x0, piece[1], x1, z0 });
			if (z1 < piece[3]) remaining.push_back({ x0, z1, x1, piece[3] });
		}
		pieces.swap(remaining);
	}
}

void Client::CLevelNavigationDebug::Sync_Level(const LEVEL level)
{
	if (m_Level == level) return;
	m_Level = level;
	m_Show = false;
	m_SelectedGrid = -1;
	m_Grids.clear();
	m_AreaId.clear();
	m_Status.clear();
	m_DrawnCells = m_VisibleOmittedCells = m_SupersededCells = m_FrustumRejectedCells = 0;
	m_FirstDrawnCell = {};
	const auto* descriptor = CLevelRegistry::Find(level);
	if (descriptor && descriptor->pMapAreaId) m_AreaId = descriptor->pMapAreaId;
}

bool Client::CLevelNavigationDebug::Reload()
{
	try
	{
		MAP_NAVIGATION_CONTRACT base;
		std::string status;
		std::vector<MAP_NAVIGATION_REGION> regions;
		if (!CMapNavigationContract::Resolve_Area(m_AreaId, base, status) ||
			!ReadPublishedRegions(base, regions, status))
		{ m_Status = "Reload failed; previous same-area snapshot retained. " + status; return false; }
		std::vector<GRID> staged;
		for (size_t gridIndex = 0; gridIndex <= regions.size(); ++gridIndex)
		{
			MAP_NAVIGATION_CONTRACT contract = base;
			if (gridIndex > 0 && !CMapNavigationContract::Resolve_Region(
				m_AreaId, regions[gridIndex - 1].regionId, contract, status))
			{ m_Status = "Reload failed; previous same-area snapshot retained. " + status; return false; }
			GRID grid;
			grid.name = gridIndex == 0 ? "Base" : regions[gridIndex - 1].regionId;
			if (FAILED(grid.runtime.Load(contract.runtimePath.c_str())) ||
				!CMapNavigationContract::Read_RuntimeStepHeight(contract, grid.maximumStepHeight, status))
			{ m_Status = "Reload failed; previous same-area snapshot retained. " + contract.runtimePath.string() + " " + status; return false; }
			if (gridIndex > 0 && std::fabs(grid.maximumStepHeight - regions[gridIndex - 1].stepHeight) > .000001f)
			{ m_Status = "Reload failed: published detail policy differs from manifest. Previous snapshot retained."; return false; }
			// Detail grids may share an XZ footprint as stacked height layers; each is drawn at its own
			// heights. The publisher and the Server load already reject layers too close to tell apart.
			const auto& desc = grid.runtime.Get_Desc();
			CNavGridPaintDocument source;
			bool matches = source.Load(contract.sourcePath, contract.paintPath, grid.sourceStatus);
			if (matches)
			{
				const auto& authored = source.Get_Desc();
				matches = authored.areaId == contract.areaId && authored.width == desc.iWidth && authored.height == desc.iHeight &&
					std::fabs(authored.cellSize - desc.fCellSize) <= .000001f &&
					std::fabs(authored.originX - desc.fOriginX) <= .000001f &&
					std::fabs(authored.originZ - desc.fOriginZ) <= .000001f;
				for (uint32_t cell = 0; matches && cell < grid.runtime.Get_NumCells(); ++cell)
				{
					matches = (source.Get_CellState(cell) == NAVGRID_AUTHORING_CELL_STATE::WALKABLE) == grid.runtime.Is_Walkable(cell) &&
						std::fabs(source.Get_RuntimeCellHeight(cell) - grid.runtime.Get_Height(cell)) <= .0001f;
				}
				grid.sourceStatus = matches ? "Source/paint walkability and heights match every published cell." :
					"Source/paint differs from published grid; blocked cause is unknown.";
			}
			grid.sourceMatched = matches;
			grid.kinds.reserve(grid.runtime.Get_NumCells());
			grid.missingSurfaceDisplayHeights.assign(grid.runtime.Get_NumCells(), 0.f);
			for (uint32_t cell = 0; cell < grid.runtime.Get_NumCells(); ++cell)
			{
				CELL_KIND kind = CELL_KIND::UNKNOWN_BLOCKED;
				if (grid.runtime.Is_Walkable(cell)) { kind = CELL_KIND::WALKABLE; ++grid.walkableCount; }
				else if (matches && source.Get_CellState(cell) == NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE)
				{
					kind = CELL_KIND::NO_SURFACE;
					grid.missingSurfaceDisplayHeights[cell] = source.Get_CellHeight(cell);
					++grid.noSurfaceCount;
				}
				else { if (matches) kind = CELL_KIND::BLOCKED; ++grid.blockedCount; }
				grid.kinds.push_back(kind);
			}
			staged.push_back(std::move(grid));
		}
		status = "Published navigation files loaded. Live Server blockers/support changes are not included.";
		m_Grids.swap(staged);
		m_SelectedGrid = -1; // Region order may have changed in the new snapshot.
		m_Status.swap(status);
		return true;
	}
	catch (const std::exception& error)
	{
		m_Status = std::string("Reload failed; previous same-area snapshot retained. ") + error.what();
		return false;
	}
	catch (...)
	{
		m_Status = "Reload failed; previous same-area snapshot retained (unexpected read error).";
		return false;
	}
}

size_t Client::CLevelNavigationDebug::Selected_Grid(const float cameraX, const float cameraZ) const
{
	if (m_SelectedGrid >= 0 && m_SelectedGrid < static_cast<int>(m_Grids.size()))
		return static_cast<size_t>(m_SelectedGrid);
	for (size_t index = 1; index < m_Grids.size(); ++index)
		if (Contains(m_Grids[index].runtime.Get_Desc(), cameraX, cameraZ)) return index;
	return 0;
}

void Client::CLevelNavigationDebug::Render_Controls()
{
	ImGui::PushID("PublishedLevelNavigation");
	ImGui::BeginDisabled(m_AreaId.empty());
	if (ImGui::Checkbox("Show Navigation", &m_Show) && m_Show && m_Grids.empty())
		if (!Reload()) m_Show = false;
	ImGui::SameLine();
	if (ImGui::SmallButton("Reload Navigation")) Reload();
	ImGui::EndDisabled();
	if (m_AreaId.empty()) ImGui::TextDisabled("This level has no navigation Area in LevelRegistry.");
	else if (m_Show)
	{
		ImGui::Text("Area: %s", m_AreaId.c_str());
		const char* selected = m_SelectedGrid < 0 ? "Auto (base + details)" : m_Grids[static_cast<size_t>(m_SelectedGrid)].name.c_str();
		if (ImGui::BeginCombo("Inspect grid", selected))
		{
			if (ImGui::Selectable("Auto (base + details)", m_SelectedGrid < 0)) m_SelectedGrid = -1;
			for (size_t index = 0; index < m_Grids.size(); ++index)
				if (ImGui::Selectable(m_Grids[index].name.c_str(), m_SelectedGrid == static_cast<int>(index))) m_SelectedGrid = static_cast<int>(index);
			ImGui::EndCombo();
		}
		ImGui::SliderFloat("Camera range (m)", &m_Radius, 10.f, 200.f, "%.0f");
		ImGui::Checkbox("Show missing surfaces", &m_ShowMissing);
		if (!m_Grids.empty())
		{
			const auto* camera = Engine::CGameInstance::Get().Get_CamPosition();
			const GRID& grid = m_Grids[Selected_Grid(camera ? camera->x : 0.f, camera ? camera->z : 0.f)];
			const auto& desc = grid.runtime.Get_Desc();
			ImGui::Text("%s: %u x %u, %.2f m cells, step %.2f m", grid.name.c_str(), desc.iWidth, desc.iHeight, desc.fCellSize, grid.maximumStepHeight);
			ImGui::Text("X [%.2f, %.2f), Z [%.2f, %.2f)", desc.fOriginX, desc.fOriginX + desc.iWidth * desc.fCellSize, desc.fOriginZ, desc.fOriginZ + desc.iHeight * desc.fCellSize);
			ImGui::Text("Walkable %u | blocked %u | no surface %u", grid.walkableCount, grid.blockedCount, grid.noSurfaceCount);
			ImGui::TextWrapped("%s", grid.sourceStatus.c_str());
		}
		ImGui::TextColored(ImVec4(.25f, 1.f, .35f, 1.f), "Green: published walkable");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f, .6f, .15f, 1.f), "Orange: resolved blocked");
		ImGui::TextColored(ImVec4(1.f, .25f, 1.f, 1.f), "Magenta: NO_SURFACE (display height inferred)");
		ImGui::TextColored(ImVec4(1.f, .2f, .2f, 1.f), "Red: published blocked, source cause unavailable");
		ImGui::TextDisabled("X-ray file inspection. Auto draws nearby base and details. Manual selection draws one grid.");
		ImGui::TextDisabled("Auto clips base outlines at detail bounds. Manual Base shows the complete base grid.");
		ImGui::Text("Drawn %u | visible omitted %u | superseded %u | range %.1f m", m_DrawnCells, m_VisibleOmittedCells, m_SupersededCells, m_EffectiveRadius);
		ImGui::Text("Frustum rejected %u | camera (%.1f, %.1f, %.1f)", m_FrustumRejectedCells, m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
		if (m_FirstDrawnCell.valid)
			ImGui::Text("First drawn cell (%d, %d) world (%.1f, %.1f) h %.2f -> screen (%.0f, %.0f)", m_FirstDrawnCell.cellX, m_FirstDrawnCell.cellZ,
				m_FirstDrawnCell.worldX, m_FirstDrawnCell.worldZ, m_FirstDrawnCell.height, m_FirstDrawnCell.screenX, m_FirstDrawnCell.screenY);
		else ImGui::TextDisabled("No cell passed the frustum test in the last frame.");
		if (m_EffectiveRadius < m_Radius || m_VisibleOmittedCells > 0)
			ImGui::TextColored(ImVec4(1.f, .75f, .2f, 1.f), "Display budget omits cells. Reduce range or move camera; grid data is unchanged.");
	}
	if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::PopID();
}

void Client::CLevelNavigationDebug::Render_Overlay()
{
	m_DrawnCells = m_VisibleOmittedCells = m_SupersededCells = m_FrustumRejectedCells = 0;
	m_FirstDrawnCell = {};
	if (!m_Show || m_Grids.empty() || !ImGui::GetCurrentContext()) return;
	auto& game = Engine::CGameInstance::Get();
	const auto* camera = game.Get_CamPosition();
	if (!camera || !std::isfinite(camera->x) || !std::isfinite(camera->z)) return;
	m_CameraPosition = float3_t(camera->x, camera->y, camera->z);
	m_EffectiveRadius = m_Radius;
	const matrix_t viewProjection = XMLoadFloat4x4(game.Get_Transform(D3DTS::VIEW)) * XMLoadFloat4x4(game.Get_Transform(D3DTS::PROJ));
	// Draw into the main viewport explicitly. With ViewportsEnable the no-argument
	// overload resolves to GImGui->CurrentWindow->Viewport, and at this call site
	// the current window is the implicit "Debug##Default" window. imgui.ini can pin
	// that window to its own viewport, which never gets a platform window and is
	// never rendered, so everything added to its background list is dropped.
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImDrawList* draw = ImGui::GetBackgroundDrawList(viewport);
	const std::array<ImU32, 4> lineColors = { IM_COL32(60, 255, 90, 230), IM_COL32(255, 155, 40, 235), IM_COL32(255, 60, 255, 220), IM_COL32(255, 50, 50, 235) };
	const std::array<ImU32, 4> fillColors = { IM_COL32(60, 255, 90, 60), IM_COL32(255, 155, 40, 70), IM_COL32(255, 60, 255, 50), IM_COL32(255, 50, 50, 70) };
	const auto screen = [viewport](const float4_t& point)
	{
		return ImVec2(viewport->Pos.x + (.5f + .5f * point.x / point.w) * viewport->Size.x,
			viewport->Pos.y + (.5f - .5f * point.y / point.w) * viewport->Size.y);
	};
	const size_t firstGrid = m_SelectedGrid < 0 ? 0 : static_cast<size_t>(m_SelectedGrid);
	const size_t endGrid = m_SelectedGrid < 0 ? m_Grids.size() : firstGrid + 1;
	for (size_t selected = firstGrid; selected < endGrid; ++selected)
	{
		const GRID& grid = m_Grids[selected];
		const auto& desc = grid.runtime.Get_Desc();
		if (camera->x + m_Radius < desc.fOriginX || camera->z + m_Radius < desc.fOriginZ ||
			camera->x - m_Radius >= desc.fOriginX + desc.iWidth * desc.fCellSize ||
			camera->z - m_Radius >= desc.fOriginZ + desc.iHeight * desc.fCellSize)
			continue;
		const float range = (std::min)(m_Radius, desc.fCellSize * MAX_CELL_RADIUS);
		m_EffectiveRadius = (std::min)(m_EffectiveRadius, range);
		// Intersect and clamp in floating point before converting to cell indices.
		// This also handles a camera far outside a manually inspected grid.
		const double minX = (static_cast<double>(camera->x) - range - desc.fOriginX) / desc.fCellSize;
		const double maxX = (static_cast<double>(camera->x) + range - desc.fOriginX) / desc.fCellSize;
		const double minZ = (static_cast<double>(camera->z) - range - desc.fOriginZ) / desc.fCellSize;
		const double maxZ = (static_cast<double>(camera->z) + range - desc.fOriginZ) / desc.fCellSize;
		if (maxX < 0 || maxZ < 0 || minX >= desc.iWidth || minZ >= desc.iHeight) continue;
		const int minimumX = static_cast<int>(std::floor((std::max)(0.0, minX)));
		const int maximumX = static_cast<int>(std::floor((std::min)(static_cast<double>(desc.iWidth - 1), maxX)));
		const int minimumZ = static_cast<int>(std::floor((std::max)(0.0, minZ)));
		const int maximumZ = static_cast<int>(std::floor((std::min)(static_cast<double>(desc.iHeight - 1), maxZ)));
		for (int z = minimumZ; z <= maximumZ; ++z)
		{
			for (int x = minimumX; x <= maximumX; ++x)
			{
				const uint32_t index = grid.runtime.To_Index(x, z);
				const CELL_KIND kind = grid.kinds[index];
				if (!m_ShowMissing && kind == CELL_KIND::NO_SURFACE) continue;
				const float worldX = desc.fOriginX + x * desc.fCellSize, worldZ = desc.fOriginZ + z * desc.fCellSize;
				const float dx = worldX + desc.fCellSize * .5f - camera->x, dz = worldZ + desc.fCellSize * .5f - camera->z;
				if (dx * dx + dz * dz > range * range) continue;
				std::vector<RECT_XZ> pieces{ { worldX, worldZ, worldX + desc.fCellSize, worldZ + desc.fCellSize } };
				if (selected == 0 && m_SelectedGrid < 0)
					for (size_t detail = 1; detail < m_Grids.size() && !pieces.empty(); ++detail)
						SubtractRectangle(pieces, m_Grids[detail].runtime.Get_Desc());
				if (pieces.empty()) { ++m_SupersededCells; continue; }
				const float height = (kind == CELL_KIND::NO_SURFACE ? grid.missingSurfaceDisplayHeights[index] : grid.runtime.Get_Height(index)) + .035f;
				bool visible = false;
				for (const auto& piece : pieces)
				{
					std::array<float4_t, 4> points;
					const std::array<float2_t, 4> offsets = { float2_t(0, 0), float2_t(1, 0), float2_t(1, 1), float2_t(0, 1) };
					bool finite = true;
					for (size_t corner = 0; corner < points.size(); ++corner)
					{
						XMStoreFloat4(&points[corner], XMVector4Transform(XMVectorSet(piece[0] + offsets[corner].x * (piece[2] - piece[0]), height, piece[1] + offsets[corner].y * (piece[3] - piece[1]), 1.f), viewProjection));
						finite = finite && Finite(points[corner]);
					}
					if (!finite) continue;
					const bool budget = m_DrawnCells < MAX_DRAW_CELLS;
					// Translucent fill only when every corner lies between the near and far
					// planes and the piece is not wholly beyond one side plane. Such a piece
					// fails every edge below and is counted as frustum-rejected, so it must
					// add no draw work; partial side overlap is left to the viewport clip rect.
					const auto beyondSidePlane = [&points](const int plane)
						{ return std::all_of(points.begin(), points.end(), [plane](const float4_t& point) { return ClipDistance(point, plane) < 0.f; }); };
					if (budget && std::all_of(points.begin(), points.end(), [](const float4_t& point)
						{ return point.w > 0.00001f && point.z >= 0.f && point.z <= point.w; })
						&& !beyondSidePlane(0) && !beyondSidePlane(1) && !beyondSidePlane(2) && !beyondSidePlane(3))
						draw->AddQuadFilled(screen(points[0]), screen(points[1]), screen(points[2]), screen(points[3]), fillColors[static_cast<size_t>(kind)]);
					for (size_t edge = 0; edge < points.size(); ++edge)
					{
						auto first = points[edge], second = points[(edge + 1) % points.size()];
						if (!ClipLine(first, second)) continue;
						visible = true;
						if (!budget) continue;
						const ImVec2 a = screen(first), b = screen(second);
						draw->AddLine(a, b, lineColors[static_cast<size_t>(kind)], 2.f);
						if (!m_FirstDrawnCell.valid)
							m_FirstDrawnCell = { true, x, z, worldX, worldZ, height, a.x, a.y };
					}
				}
				if (visible) { if (m_DrawnCells < MAX_DRAW_CELLS) ++m_DrawnCells; else ++m_VisibleOmittedCells; }
				else ++m_FrustumRejectedCells;
			}
		}
	}
}
#endif
