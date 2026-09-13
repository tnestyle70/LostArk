#include "imgui.h"
#include "MapTool_Internal.h"
#include "GameInstance.h"
#include "MapStaticBatchObject.h"
#include "Model.h"
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




bool_t Client::CMapTool::Collect_NavigationBakePlacements(
	std::vector<NAVGRID_BAKE_PLACEMENT>& outPlacements,
	std::string& outStatus) const
{
	outPlacements.clear();
	size_t skippedInstances = 0;
	if (!m_Catalog.Is_Ready())
	{
		outStatus = "Map asset catalog is unavailable";
		return false;
	}

	const f32_t radians =
		XMConvertToRadians(m_NavigationBakeDesc.yawDegrees);
	const f32_t cosine = std::abs(std::cos(radians));
	const f32_t sine = std::abs(std::sin(radians));
	const float3_t halfExtents(
		cosine * m_NavigationBakeDesc.size.x * 0.5f +
			sine * m_NavigationBakeDesc.size.z * 0.5f,
		m_NavigationBakeDesc.size.y * 0.5f,
		sine * m_NavigationBakeDesc.size.x * 0.5f +
			cosine * m_NavigationBakeDesc.size.z * 0.5f);
	const float3_t minimum(
		m_NavigationBakeDesc.position.x - halfExtents.x,
		m_NavigationBakeDesc.position.y - halfExtents.y,
		m_NavigationBakeDesc.position.z - halfExtents.z);
	const float3_t maximum(
		m_NavigationBakeDesc.position.x + halfExtents.x,
		m_NavigationBakeDesc.position.y + halfExtents.y,
		m_NavigationBakeDesc.position.z + halfExtents.z);

	std::unordered_map<std::string, shared_ptr<CModel>> modelCache;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		bool_t visible = entry.record.visible;
		if (entry.record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
		{
			visible =
				m_EnvironmentPhase != ENVIRONMENT_PHASE::BASELINE;
		}
		else if (entry.record.sourceLevel ==
			"VALTAN_PHASE_CHAOSGATE")
		{
			visible =
				m_EnvironmentPhase == ENVIRONMENT_PHASE::CHAOS_GATE;
		}
		if (!visible)
			continue;

		const MAP_ASSET_ENTRY* asset =
			m_Catalog.Find(entry.record.assetId);
		if (nullptr == asset ||
			!IsBatchEligible(*asset) ||
			asset->groupLabel == "LV_NAVIMESH" ||
			std::string::npos != asset->id.find("CUL_BOX"))
		{
			continue;
		}

		auto model = modelCache.find(asset->id);
		if (model == modelCache.end())
		{
			shared_ptr<CModel> cloned =
				dynamic_pointer_cast<CModel>(
					CGameInstance::Get().Clone_Prototype(
						m_iAuthoringLevelIndex,
						asset->prototypeTag));
			if (nullptr == cloned || !cloned->Has_LocalBounds())
			{
				outStatus =
					"Could not inspect model bounds for " +
					asset->id;
				return false;
			}
			model = modelCache.emplace(
				asset->id,
				std::move(cloned)).first;
		}

		FMapStaticInstance instance{};
		if (FAILED(BuildStaticInstance(
			*asset,
			model->second,
			entry.record,
			instance)))
		{
			++skippedInstances;
			continue;
		}

		const f32_t closestX = (std::clamp)(
			instance.WorldBoundsCenter.x,
			minimum.x,
			maximum.x);
		const f32_t closestY = (std::clamp)(
			instance.WorldBoundsCenter.y,
			minimum.y,
			maximum.y);
		const f32_t closestZ = (std::clamp)(
			instance.WorldBoundsCenter.z,
			minimum.z,
			maximum.z);
		const f32_t offsetX =
			instance.WorldBoundsCenter.x - closestX;
		const f32_t offsetY =
			instance.WorldBoundsCenter.y - closestY;
		const f32_t offsetZ =
			instance.WorldBoundsCenter.z - closestZ;
		const f32_t distanceSquared =
			offsetX * offsetX +
			offsetY * offsetY +
			offsetZ * offsetZ;
		if (distanceSquared >
			instance.WorldBoundsRadius *
				instance.WorldBoundsRadius)
		{
			continue;
		}

		NAVGRID_BAKE_PLACEMENT bakePlacement;
		bakePlacement.assetId = asset->id;
		bakePlacement.modelPath = asset->resolvedModelPath;
		bakePlacement.world = instance.World;
		outPlacements.push_back(std::move(bakePlacement));
	}

	if (outPlacements.empty())
	{
		outStatus =
			"No visible deferred map meshes overlap Nav Bounds; skipped " +
			std::to_string(skippedInstances) + " unbuildable instances";
		return false;
	}
	outStatus = std::to_string(outPlacements.size()) +
		" bake meshes; skipped " + std::to_string(skippedInstances) +
		" unbuildable instances";
	return true;
}

void Client::CMapTool::Discard_NavigationBakePreview()
{
	m_bNavigationBakePreviewReady = false;
	m_bNavigationBakePreviewLayoutChanged = false;
	m_iNavigationBakePreviewWalkable = {};
	m_NavigationBakePreview = NAVGRID_BAKE_RESULT{};
}

bool_t Client::CMapTool::Preview_NavigationBake()
{
	/* Everything here happens in memory. The bounds can be wrong, the cell
	   size can be wrong, and nothing on disk changes: the numbers this leaves
	   behind are what the panel shows before Apply is offered at all. */
	Discard_NavigationBakePreview();

	if (!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc))
	{
		m_NavigationBakeStatus =
			"Nav Bounds or bake settings are invalid";
		return false;
	}

	const std::string bakeGridId = m_NavigationRegionId.empty() ?
		m_Catalog.Get_AreaId() :
		m_Catalog.Get_AreaId() + "." + m_NavigationRegionId;

	NAVGRID_AUTHORING_DESC nextDesc;
	std::string status;
	if (!CNavGridBaker::Build_Desc(
		bakeGridId,
		m_NavigationBakeDesc,
		nextDesc,
		status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	const NAVGRID_AUTHORING_DESC* currentDesc =
		m_NavigationDocument.Is_Ready() ?
		&m_NavigationDocument.Get_Desc() :
		nullptr;
	const bool_t layoutChanged =
		nullptr == currentDesc ||
		currentDesc->areaId != nextDesc.areaId ||
		currentDesc->width != nextDesc.width ||
		currentDesc->height != nextDesc.height ||
		std::abs(currentDesc->cellSize - nextDesc.cellSize) > 0.000001f ||
		std::abs(currentDesc->originX - nextDesc.originX) > 0.000001f ||
		std::abs(currentDesc->originZ - nextDesc.originZ) > 0.000001f;

	std::vector<NAVGRID_BAKE_PLACEMENT> placements;
	if (!Collect_NavigationBakePlacements(placements, status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	NAVGRID_BAKE_RESULT result;
	if (!CNavGridBaker::Build(
		bakeGridId,
		m_NavigationBakeDesc,
		placements,
		result,
		status))
	{
		m_NavigationBakeStatus = status;
		return false;
	}

	uint32_t walkable = {};
	for (const NAV_SOURCE_CELL& cell : result.cells)
	{
		if (cell.surfaceResolved && cell.baseWalkable)
			++walkable;
	}

	m_NavigationBakePreview = std::move(result);
	m_bNavigationBakePreviewLayoutChanged = layoutChanged;
	m_iNavigationBakePreviewWalkable = walkable;
	m_bNavigationBakePreviewReady = true;
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_NavigationBakeStatus = status;
	return true;
}
bool_t Client::CMapTool::Bake_Navigation()
{
	/* Only reachable through Preview_NavigationBake, so the grid being written
	   is exactly the one whose numbers the panel showed. This is the single
	   step that touches disk. */
	if (!m_bNavigationBakePreviewReady)
	{
		m_NavigationBakeStatus = "Run Bake Preview before applying";
		return false;
	}

	std::string status;
	const bool_t layoutChanged = m_bNavigationBakePreviewLayoutChanged;
	const NAVGRID_BAKE_RESULT& result = m_NavigationBakePreview;

	const bool_t hasAuthoredCells =
		m_NavigationDocument.Is_Ready() &&
		(0 != m_NavigationDocument.Get_BlockedCount() ||
			0 != m_RuntimeBlockerDocument.Get_RegionCount());
	if (layoutChanged &&
		hasAuthoredCells &&
		!m_bNavigationBakeResetConfirmed)
	{
		m_bNavigationBakeResetPending = true;
		m_NavigationBakeStatus =
			"Grid layout changed; confirm reset of paint and regions";
		return false;
	}

	struct FILE_BACKUP final
	{
		std::filesystem::path path;
		std::filesystem::path backup;
		bool_t existed = false;
	};
	std::array<FILE_BACKUP, 3> backups =
	{{
		{ m_NavigationSourcePath, {}, false },
		{ m_NavigationPaintPath, {}, false },
		{ m_RuntimeBlockerPath, {}, false },
	}};

	auto cleanupBackups = [&backups]()
	{
		for (const FILE_BACKUP& file : backups)
		{
			if (file.backup.empty())
				continue;
			std::error_code error;
			std::filesystem::remove(file.backup, error);
		}
	};
	auto restoreBackups = [&backups]()
	{
		bool_t restored = true;
		for (const FILE_BACKUP& file : backups)
		{
			if (file.path.empty())
				continue;
			std::error_code error;
			if (file.existed)
			{
				std::filesystem::copy_file(
					file.backup,
					file.path,
					std::filesystem::copy_options::overwrite_existing,
					error);
			}
			else
			{
				std::filesystem::remove(file.path, error);
			}
			restored = !error && restored;
		}
		return restored;
	};

	for (FILE_BACKUP& file : backups)
	{
		if (file.path.empty())
			continue;
		file.backup = file.path;
		file.backup += L".bakebak";
		std::error_code error;
		std::filesystem::remove(file.backup, error);
		error.clear();
		file.existed =
			std::filesystem::exists(file.path, error);
		if (error)
		{
			cleanupBackups();
			m_NavigationBakeStatus =
				"Could not inspect existing navigation files";
			return false;
		}
		if (file.existed)
		{
			std::filesystem::copy_file(
				file.path,
				file.backup,
				std::filesystem::copy_options::overwrite_existing,
				error);
			if (error)
			{
				cleanupBackups();
				m_NavigationBakeStatus =
					"Could not create navigation rollback backup";
				return false;
			}
		}
	}

	if (!CNavGridBaker::Save_Source(
		result,
		m_NavigationSourcePath,
		status))
	{
		cleanupBackups();
		m_NavigationBakeStatus = status;
		return false;
	}

	if (layoutChanged)
	{
		std::error_code paintError;
		if (!m_NavigationPaintPath.empty())
		{
			std::filesystem::remove(
				m_NavigationPaintPath,
				paintError);
		}
		std::error_code blockerError;
		if (!m_RuntimeBlockerPath.empty())
		{
			std::filesystem::remove(
				m_RuntimeBlockerPath,
				blockerError);
		}
		if (paintError || blockerError)
		{
			const bool_t restored = restoreBackups();
			cleanupBackups();
			m_NavigationBakeStatus = restored ?
				"Could not reset incompatible navigation authoring files" :
				"Navigation rollback failed after reset error";
			return false;
		}
	}

	if (!Load_NavigationDocument())
	{
		const bool_t restored = restoreBackups();
		cleanupBackups();
		const bool_t reloaded =
			restored && Load_NavigationDocument();
		m_NavigationBakeStatus = reloaded ?
			"Bake validation failed; previous navigation was restored" :
			"Bake validation failed and navigation rollback did not reload";
		return false;
	}

	cleanupBackups();
	/* Registered only after the navsource exists, so the manifest never names
	   a grid the publisher cannot read. */
	if (!m_NavigationRegionId.empty() && !Commit_NavigationRegionManifest())
		return false;
	Discard_NavigationBakePreview();
	m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_NavigationBakeStatus = status;
	m_NavigationStatus =
		"Baked source; Save Navigation to persist authoring paint";
	return true;
}

void Client::CMapTool::Render_NavigationBakeControls()
{
	if (ImGui::Button("Place Nav Bounds"))
	{
		m_eNavigationBoundsState = NAV_BOUNDS_STATE::PLACING;
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		m_NavigationBakeStatus =
			"Click a rendered floor to place Nav Bounds";
	}
	if (NAV_BOUNDS_STATE::PLACING == m_eNavigationBoundsState)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("Picking... Esc cancels");
	}

	ImGui::SeparatorText("Nav Bounds");
	bool_t changed = false;
	f32_t positionXZ[2] =
	{
		m_NavigationBakeDesc.position.x,
		m_NavigationBakeDesc.position.z,
	};
	if (ImGui::DragFloat2("Position XZ", positionXZ, 0.1f))
	{
		m_NavigationBakeDesc.position.x = positionXZ[0];
		m_NavigationBakeDesc.position.z = positionXZ[1];
		changed = true;
	}

	f32_t bottomY =
		m_NavigationBakeDesc.position.y -
		m_NavigationBakeDesc.size.y * 0.5f;
	if (ImGui::DragFloat("Bottom Y", &bottomY, 0.1f))
	{
		m_NavigationBakeDesc.position.y =
			bottomY + m_NavigationBakeDesc.size.y * 0.5f;
		changed = true;
	}

	f32_t sizeXZ[2] =
	{
		m_NavigationBakeDesc.size.x,
		m_NavigationBakeDesc.size.z,
	};
	if (ImGui::DragFloat2(
		"Size XZ",
		sizeXZ,
		0.1f,
		0.1f,
		10000.f))
	{
		m_NavigationBakeDesc.size.x = sizeXZ[0];
		m_NavigationBakeDesc.size.z = sizeXZ[1];
		changed = true;
	}

	/* Position is the box centre, so Size alone pulls both faces inward: a face
	   already lined up on the target drifts off it while you shrink the other
	   side. These fields move one face and leave the opposite one exactly where
	   it is, which is the anchored model Bottom Y and Height from Bottom
	   already use for the vertical axis. The values run along the box's own
	   axes, so at yaw 0 they are literally world X and Z. */
	const f32_t boundsRadians =
		XMConvertToRadians(m_NavigationBakeDesc.yawDegrees);
	const f32_t boundsAxisXx = std::cos(boundsRadians);
	const f32_t boundsAxisXz = -std::sin(boundsRadians);
	const f32_t boundsAxisZx = std::sin(boundsRadians);
	const f32_t boundsAxisZz = std::cos(boundsRadians);
	const f32_t centerAlongX =
		m_NavigationBakeDesc.position.x * boundsAxisXx +
		m_NavigationBakeDesc.position.z * boundsAxisXz;
	const f32_t centerAlongZ =
		m_NavigationBakeDesc.position.x * boundsAxisZx +
		m_NavigationBakeDesc.position.z * boundsAxisZz;

	f32_t spanX[2] =
	{
		centerAlongX - m_NavigationBakeDesc.size.x * 0.5f,
		centerAlongX + m_NavigationBakeDesc.size.x * 0.5f,
	};
	if (ImGui::DragFloat2("X Min / Max", spanX, 0.1f) &&
		spanX[1] - spanX[0] >= 0.1f)
	{
		/* The field the user did not drag keeps its displayed value, so that
		   face stays pinned no matter which side is being moved. */
		const f32_t movedCenter = (spanX[0] + spanX[1]) * 0.5f;
		const f32_t centerDelta = movedCenter - centerAlongX;
		m_NavigationBakeDesc.position.x += boundsAxisXx * centerDelta;
		m_NavigationBakeDesc.position.z += boundsAxisXz * centerDelta;
		m_NavigationBakeDesc.size.x = spanX[1] - spanX[0];
		changed = true;
	}

	f32_t spanZ[2] =
	{
		centerAlongZ - m_NavigationBakeDesc.size.z * 0.5f,
		centerAlongZ + m_NavigationBakeDesc.size.z * 0.5f,
	};
	if (ImGui::DragFloat2("Z Min / Max", spanZ, 0.1f) &&
		spanZ[1] - spanZ[0] >= 0.1f)
	{
		const f32_t movedCenter = (spanZ[0] + spanZ[1]) * 0.5f;
		const f32_t centerDelta = movedCenter - centerAlongZ;
		m_NavigationBakeDesc.position.x += boundsAxisZx * centerDelta;
		m_NavigationBakeDesc.position.z += boundsAxisZz * centerDelta;
		m_NavigationBakeDesc.size.z = spanZ[1] - spanZ[0];
		changed = true;
	}


	f32_t height = m_NavigationBakeDesc.size.y;
	if (ImGui::DragFloat(
		"Height from Bottom",
		&height,
		0.1f,
		0.1f,
		10000.f))
	{
		m_NavigationBakeDesc.size.y = height;
		m_NavigationBakeDesc.position.y = bottomY + height * 0.5f;
		changed = true;
	}
	ImGui::Text(
		"Top Y: %.3f",
		bottomY + m_NavigationBakeDesc.size.y);
	changed =
		ImGui::DragFloat(
			"Yaw",
			&m_NavigationBakeDesc.yawDegrees,
			0.25f,
			-360.f,
			360.f) ||
		changed;

	ImGui::SeparatorText("Bake Settings");
	changed =
		ImGui::DragFloat(
			"Cell Size",
			&m_NavigationBakeDesc.cellSize,
			0.01f,
			0.05f,
			10.f) ||
		changed;
	changed =
		ImGui::DragFloat(
			"Max Slope",
			&m_NavigationBakeDesc.maxSlopeDegrees,
			0.25f,
			0.f,
			89.f) ||
		changed;

	/* Bake refuses anything past the one-million-cell cap, and at a fine cell
	   size a large box crosses it easily. The same arithmetic Build_Desc uses
	   is shown here so the limit is an edit-time number instead of a failed
	   bake after the bounds are already placed. */
	const f32_t previewCos = std::abs(std::cos(boundsRadians));
	const f32_t previewSin = std::abs(std::sin(boundsRadians));
	const f32_t previewHalfX =
		previewCos * m_NavigationBakeDesc.size.x * 0.5f +
		previewSin * m_NavigationBakeDesc.size.z * 0.5f;
	const f32_t previewHalfZ =
		previewSin * m_NavigationBakeDesc.size.x * 0.5f +
		previewCos * m_NavigationBakeDesc.size.z * 0.5f;
	const f64_t previewWidth = m_NavigationBakeDesc.cellSize > 0.f ?
		std::ceil((previewHalfX * 2.f) / m_NavigationBakeDesc.cellSize) : 0.0;
	const f64_t previewHeight = m_NavigationBakeDesc.cellSize > 0.f ?
		std::ceil((previewHalfZ * 2.f) / m_NavigationBakeDesc.cellSize) : 0.0;
	const f64_t previewCells = previewWidth * previewHeight;
	const bool_t previewOverCap = previewCells >
		static_cast<f64_t>(CNavGridPaintDocument::MAX_CELL_COUNT);
	if (previewOverCap)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.4f, 0.4f, 1.f),
			"Grid: %.0f x %.0f = %.0f cells (over the 1,000,000 cap)",
			previewWidth,
			previewHeight,
			previewCells);
	}
	else
	{
		ImGui::Text(
			"Grid: %.0f x %.0f = %.0f cells",
			previewWidth,
			previewHeight,
			previewCells);
	}

	if (changed)
	{
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		/* The staged grid belongs to the bounds it was built from, so an edit
		   to those bounds retires it rather than leaving stale numbers up. */
		Discard_NavigationBakePreview();
		m_NavigationBakeStatus =
			m_NavigationBakeDesc.isReady ?
			"Needs Bake" :
			"Place Nav Bounds first";
	}

	ImGui::BeginDisabled(
		!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc) ||
		NAV_BOUNDS_STATE::PLACING == m_eNavigationBoundsState);
	if (ImGui::Button("Bake Preview##NavigationBakePreview"))
		Preview_NavigationBake();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("(builds in memory; nothing is written yet)");

	if (m_bNavigationBakePreviewReady)
	{
		const NAVGRID_AUTHORING_DESC& previewDesc =
			m_NavigationBakePreview.desc;
		const uint64_t previewCellCount =
			static_cast<uint64_t>(previewDesc.width) * previewDesc.height;
		ImGui::SeparatorText("Bake Preview");
		ImGui::Text(
			"Grid %u x %u @ %.3f m  origin (%.2f, %.2f)",
			previewDesc.width,
			previewDesc.height,
			previewDesc.cellSize,
			previewDesc.originX,
			previewDesc.originZ);
		ImGui::Text(
			"Scanned %u placements, %llu triangles",
			m_NavigationBakePreview.placementCount,
			static_cast<unsigned long long>(
				m_NavigationBakePreview.triangleCount));
		/* A box aimed at nothing still bakes: it just resolves almost no
		   surface. Seeing that here is the difference between noticing now
		   and noticing after the paint is already gone. */
		if (0u == m_NavigationBakePreview.resolvedCellCount)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.4f, 0.4f, 1.f),
				"Found no surface at all - the bounds are probably off target");
		}
		else
		{
			ImGui::Text(
				"Surface on %u of %llu cells, %u walkable",
				m_NavigationBakePreview.resolvedCellCount,
				static_cast<unsigned long long>(previewCellCount),
				m_iNavigationBakePreviewWalkable);
		}
		if (m_bNavigationBakePreviewLayoutChanged &&
			m_NavigationDocument.Is_Ready())
		{
			const uint32_t blocked =
				m_NavigationDocument.Get_BlockedCount();
			const uint32_t forced =
				m_NavigationDocument.Get_ForcedWalkableCount();
			const uint32_t heights =
				m_NavigationDocument.Get_HeightOverrideCount();
			if (0u != blocked || 0u != forced || 0u != heights)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.4f, 0.4f, 1.f),
					"Applying deletes authored paint: %u blocked, "
					"%u forced walkable, %u height overrides",
					blocked,
					forced,
					heights);
			}
		}

		if (ImGui::Button("Apply Bake##NavigationBakeApply"))
			Bake_Navigation();
		ImGui::SameLine();
		if (ImGui::Button("Cancel##NavigationBakeCancel"))
		{
			Discard_NavigationBakePreview();
			m_NavigationBakeStatus = "Bake preview discarded";
		}
		if (m_bNavigationBakeResetPending)
		{
			ImGui::SameLine();
			if (ImGui::Button("Confirm Reset and Rebake"))
			{
				m_bNavigationBakeResetConfirmed = true;
				Bake_Navigation();
			}
		}
	}

	ImGui::TextWrapped(
		"%s",
		m_NavigationBakeStatus.c_str());
	ImGui::TextDisabled(
		"White: Nav Bounds | Bake uses visible static map meshes");
}

bool_t Client::CMapTool::Is_ValidNavigationBakeDesc(
	const NAVGRID_BAKE_DESC& desc)
{
	//Bounds ���� ���� ������ �ϴ� �Լ� - ȣ���� ���� ����?
	//� ���¸� �����ϰ� �����ϴ� ������ �ϴ� ����?
	return
		desc.isReady &&

		std::isfinite(desc.position.x) &&
		std::isfinite(desc.position.y) &&
		std::isfinite(desc.position.z) &&

		std::isfinite(desc.size.x) &&
		std::isfinite(desc.size.y) &&
		std::isfinite(desc.size.z) &&

		desc.size.x >= 0.1f &&
		desc.size.y >= 0.1f &&
		desc.size.z >= 0.1f &&

		std::isfinite(desc.yawDegrees) &&

		std::isfinite(desc.cellSize) &&
		desc.cellSize >= 0.05f &&
		desc.cellSize <= 10.f &&

		std::isfinite(desc.maxSlopeDegrees) &&
		desc.maxSlopeDegrees >= 0.f &&
		desc.maxSlopeDegrees < 90.f;
}
