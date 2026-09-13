#include "imgui.h"
#include "MapTool_Internal.h"
#include "GameInstance.h"
#include "MapEditorWorkspaceService.h"
#include "MapNavigationContract.h"
#include "Navigation.h"
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




bool_t Client::CMapTool::Validate_WorldNpcBehaviorNavigation(
	const WORLD_GAMEPLAY_PLACEMENT& placement,
	const CNavGridPaintDocument& navigation,
	const CNavRuntimeBlockerDocument& blockers,
	std::string& outStatus) const
{
	if (!placement.npcBehavior.has_value() ||
		WORLD_NPC_BEHAVIOR_MODE::STATIONARY ==
			placement.npcBehavior->eMode)
	{
		return true;
	}
	if (!navigation.Is_Ready())
	{
		outStatus = "Moving NPC behavior requires loaded navigation: " +
			placement.placementId;
		return false;
	}
	if (!blockers.Is_Ready() ||
		blockers.Get_Desc().areaId != navigation.Get_Desc().areaId ||
		blockers.Get_Desc().width != navigation.Get_Desc().width ||
		blockers.Get_Desc().height != navigation.Get_Desc().height)
	{
		outStatus = "Moving NPC behavior requires matching runtime blockers: " +
			placement.placementId;
		return false;
	}

	std::vector<uint8_t> initiallyBlocked(navigation.Get_CellCount(), 0u);
	for (size_t regionIndex = 0u;
		regionIndex < blockers.Get_RegionCount(); ++regionIndex)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			blockers.Get_Region(regionIndex);
		if (nullptr == region || region->activateWhenConditionTrue)
			continue;
		for (const uint32_t cellIndex : blockers.Get_CellIndices(regionIndex))
		{
			if (cellIndex < initiallyBlocked.size())
				initiallyBlocked[cellIndex] = 1u;
		}
	}
	const auto isInitiallyWalkable =
		[&navigation, &initiallyBlocked](const uint32_t index)
		{
			return index < initiallyBlocked.size() &&
				0u == initiallyBlocked[index] &&
				NAVGRID_AUTHORING_CELL_STATE::WALKABLE ==
					navigation.Get_CellState(index);
		};

	const auto resolveWalkableCell = [&navigation, &isInitiallyWalkable](
		const float3_t& position, int32_t& outX, int32_t& outZ)
	{
		if (!navigation.World_ToCell(
			XMLoadFloat3(&position), outX, outZ))
		{
			return false;
		}
		const uint32_t index = navigation.To_Index(outX, outZ);
		return navigation.Has_ResolvedHeight(index) &&
			isInitiallyWalkable(index);
	};
	int32_t startX = 0;
	int32_t startZ = 0;
	if (!resolveWalkableCell(placement.position, startX, startZ))
	{
		outStatus = "Moving NPC spawn is outside walkable navigation: " +
			placement.placementId;
		return false;
	}

	const NAVGRID_AUTHORING_DESC& navDesc = navigation.Get_Desc();
	const uint32_t cellCount = navigation.Get_CellCount();
	std::vector<uint8_t> reachable(cellCount, 0u);
	std::vector<uint32_t> frontier;
	frontier.reserve(cellCount);
	const uint32_t startIndex = navigation.To_Index(startX, startZ);
	reachable[startIndex] = 1u;
	frontier.push_back(startIndex);
	constexpr int32_t directions[8][2] = {
		{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}
	};
	for (size_t cursor = 0u; cursor < frontier.size(); ++cursor)
	{
		const uint32_t current = frontier[cursor];
		const int32_t currentX = static_cast<int32_t>(
			current % navDesc.width);
		const int32_t currentZ = static_cast<int32_t>(
			current / navDesc.width);
		for (const auto& direction : directions)
		{
			const int32_t nextX = currentX + direction[0];
			const int32_t nextZ = currentZ + direction[1];
			if (!navigation.Is_ValidCell(nextX, nextZ))
				continue;
			const uint32_t next = navigation.To_Index(nextX, nextZ);
			if (0u != reachable[next] || !isInitiallyWalkable(next))
			{
				continue;
			}
			const bool_t diagonal =
				0 != direction[0] && 0 != direction[1];
			if (diagonal)
			{
				const uint32_t sideA =
					navigation.To_Index(nextX, currentZ);
				const uint32_t sideB =
					navigation.To_Index(currentX, nextZ);
				if (!isInitiallyWalkable(sideA) ||
					!isInitiallyWalkable(sideB))
				{
					continue;
				}
			}
			reachable[next] = 1u;
			frontier.push_back(next);
		}
	}

	const WORLD_NPC_BEHAVIOR& behavior = *placement.npcBehavior;
	if (WORLD_NPC_BEHAVIOR_MODE::PATROL == behavior.eMode)
	{
		for (const WORLD_NPC_WAYPOINT& waypoint : behavior.waypoints)
		{
			int32_t waypointX = 0;
			int32_t waypointZ = 0;
			if (!resolveWalkableCell(
					waypoint.position, waypointX, waypointZ) ||
				0u == reachable[navigation.To_Index(waypointX, waypointZ)])
			{
				outStatus = "NPC patrol waypoint is unreachable: " +
					placement.placementId + "/" + waypoint.waypointId;
				return false;
			}
		}
		return true;
	}

	const f32_t spawnX = navDesc.originX +
		(static_cast<f32_t>(startX) + 0.5f) * navDesc.cellSize;
	const f32_t spawnZ = navDesc.originZ +
		(static_cast<f32_t>(startZ) + 0.5f) * navDesc.cellSize;
	const f32_t radiusSquared =
		behavior.wanderRadius * behavior.wanderRadius;
	constexpr f32_t nontrivialDistanceSquared = 0.05f * 0.05f;
	std::vector<uint8_t> radialReachable(cellCount, 0u);
	std::vector<uint32_t> radialFrontier;
	radialFrontier.reserve(cellCount);
	radialReachable[startIndex] = 1u;
	radialFrontier.push_back(startIndex);
	for (size_t cursor = 0u; cursor < radialFrontier.size(); ++cursor)
	{
		const uint32_t index = radialFrontier[cursor];
		const int32_t cellX = static_cast<int32_t>(index % navDesc.width);
		const int32_t cellZ = static_cast<int32_t>(index / navDesc.width);
		const f32_t worldX = navDesc.originX +
			(static_cast<f32_t>(cellX) + 0.5f) * navDesc.cellSize;
		const f32_t worldZ = navDesc.originZ +
			(static_cast<f32_t>(cellZ) + 0.5f) * navDesc.cellSize;
		const f32_t deltaX = worldX - spawnX;
		const f32_t deltaZ = worldZ - spawnZ;
		const f32_t distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
		if (index != startIndex &&
			distanceSquared >= nontrivialDistanceSquared &&
			distanceSquared <= radiusSquared + 0.001f)
		{
			return true;
		}
		for (const auto& direction : directions)
		{
			const int32_t nextX = cellX + direction[0];
			const int32_t nextZ = cellZ + direction[1];
			if (!navigation.Is_ValidCell(nextX, nextZ))
				continue;
			const uint32_t next = navigation.To_Index(nextX, nextZ);
			if (0u != radialReachable[next] || !isInitiallyWalkable(next))
				continue;
			const f32_t nextWorldX = navDesc.originX +
				(static_cast<f32_t>(nextX) + 0.5f) * navDesc.cellSize;
			const f32_t nextWorldZ = navDesc.originZ +
				(static_cast<f32_t>(nextZ) + 0.5f) * navDesc.cellSize;
			const f32_t nextDeltaX = nextWorldX - spawnX;
			const f32_t nextDeltaZ = nextWorldZ - spawnZ;
			if (nextDeltaX * nextDeltaX + nextDeltaZ * nextDeltaZ >
				radiusSquared + 0.001f)
			{
				continue;
			}
			if (0 != direction[0] && 0 != direction[1])
			{
				const uint32_t sideA = navigation.To_Index(nextX, cellZ);
				const uint32_t sideB = navigation.To_Index(cellX, nextZ);
				if (!isInitiallyWalkable(sideA) ||
					!isInitiallyWalkable(sideB))
				{
					continue;
				}
			}
			radialReachable[next] = 1u;
			radialFrontier.push_back(next);
		}
	}
	outStatus = "NPC wander owns no reachable destination inside its radius: " +
		placement.placementId;
	return false;
}

bool_t Client::CMapTool::Load_NavigationDocument()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
		if (nullptr == active ||
			EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy)
		{
			m_NavigationDocument = CNavGridPaintDocument{};
			m_RuntimeBlockerDocument = CNavRuntimeBlockerDocument{};
			m_NavigationStatus =
				"Navigation authoring disabled for this Area";
			return true;
		}

		std::error_code sourceError;
		bool_t hasSource = std::filesystem::is_regular_file(
			active->navigationSource, sourceError);
		if (IsFileInspectionFailure(sourceError) ||
			(!hasSource && !active->allowNavigationBootstrap))
		{
			m_NavigationStatus =
				"Required navigation source is missing: " + active->areaId;
			return false;
		}

		std::string manifestStatus;
		(void)CMapNavigationContract::Read_RegionManifest(
			active->areaId, m_NavigationRegions, manifestStatus);
		/* The descriptor owns the Area's base grid. A selected detail region is
		   a different grid beside it, so its paths come from the contract. */
		if (!m_NavigationRegionId.empty())
		{
			MAP_NAVIGATION_CONTRACT selectedContract;
			std::string selectedStatus;
			if (!Resolve_SelectedNavigationContract(
				selectedContract, selectedStatus))
			{
				m_NavigationStatus = selectedStatus;
				return false;
			}
			m_NavigationSourcePath = selectedContract.sourcePath;
			m_NavigationPaintPath = selectedContract.paintPath;
			m_RuntimeBlockerPath = selectedContract.blockerPath;
			m_NavigationRuntimePath = selectedContract.runtimePath;
			/* The gate above asked whether the Area's base grid exists. A
			   region is judged on its own file: a missing one is the normal
			   state of a region that has not been baked yet, and the bake is
			   what creates it, so bootstrap is always allowed here. */
			std::error_code regionSourceError;
			hasSource = std::filesystem::is_regular_file(
				m_NavigationSourcePath, regionSourceError);
			if (IsFileInspectionFailure(regionSourceError))
			{
				m_NavigationStatus =
					"Could not inspect navigation region source: " +
					m_NavigationSourcePath.string();
				return false;
			}
		}
		else
		{
			m_NavigationSourcePath = active->navigationSource;
			m_NavigationPaintPath = active->navigationPaint;
			m_RuntimeBlockerPath = active->navigationBlockers;
			m_NavigationRuntimePath.clear();
		}
		if (!hasSource)
		{
			m_NavigationDocument = CNavGridPaintDocument{};
			m_RuntimeBlockerDocument = CNavRuntimeBlockerDocument{};
			/* Bounds placed before creating the region are the bounds meant
			   for it. Only a base bootstrap starts from nothing. */
			if (m_NavigationRegionId.empty())
				m_NavigationBakeDesc = NAVGRID_BAKE_DESC{};
			m_NavigationStatus = m_NavigationRegionId.empty() ?
				"Navigation bootstrap: place Nav Bounds and Bake" :
				"Region " + m_NavigationRegionId +
				" has no bake yet: place Nav Bounds and Bake";
			return false;
		}

		std::string status;
		CNavGridPaintDocument stagedNavigation;
		CNavRuntimeBlockerDocument stagedBlockers;
		if (!stagedNavigation.Load(
			m_NavigationSourcePath,
			m_NavigationPaintPath,
			status) ||
			stagedNavigation.Get_Desc().areaId !=
				(m_NavigationRegionId.empty() ?
					active->areaId :
					active->areaId + "." + m_NavigationRegionId) ||
			!stagedBlockers.Load(
				m_RuntimeBlockerPath,
				stagedNavigation.Get_Desc(),
				status))
		{
			m_NavigationStatus = status;
			return false;
		}
		if (m_DestructionDocument.Is_Ready())
		{
			if (!Validate_DestructionExternalReferences(
				m_DestructionDocument,
				m_DeployRuntime,
				stagedBlockers,
				m_WorldGameplayDocument,
				m_EncounterReference,
				status))
			{
				m_NavigationStatus = status;
				return false;
			}
		}

		m_NavigationDocument = std::move(stagedNavigation);
		m_RuntimeBlockerDocument = std::move(stagedBlockers);
		m_NavigationBakeDesc = m_NavigationDocument.Get_BakeDesc();
		m_iSelectedRuntimeRegion = 0;
		m_NavigationStatus = "Navigation authoring ready";
		return true;
	}

	MAP_NAVIGATION_CONTRACT stagedContract;
	std::string stagedStatus;
	std::string manifestStatus;
	(void)CMapNavigationContract::Read_RegionManifest(
		m_Catalog.Get_AreaId(), m_NavigationRegions, manifestStatus);
	if (!Resolve_SelectedNavigationContract(stagedContract, stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	std::error_code sourceError;
	const bool_t hasSource = std::filesystem::is_regular_file(
		stagedContract.sourcePath, sourceError);
	if (IsFileInspectionFailure(sourceError))
	{
		m_NavigationStatus =
			"Could not inspect navigation source for " +
			stagedContract.areaId;
		return false;
	}
	if (!hasSource)
	{
		CNavGridPaintDocument stagedNavigationDocument;
		CNavRuntimeBlockerDocument stagedBlockerDocument;
		NAVGRID_BAKE_DESC stagedBakeDesc;

		m_NavigationSourcePath = stagedContract.sourcePath;
		m_NavigationPaintPath = stagedContract.paintPath;
		m_NavigationRuntimePath = stagedContract.runtimePath;
		m_RuntimeBlockerPath = stagedContract.blockerPath;
		m_NavigationDocument = std::move(stagedNavigationDocument);
		m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
		m_NavigationBakeDesc = stagedBakeDesc;
		m_iSelectedRuntimeRegion = 0;
		m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
		m_bNavigationStrokeActive = false;
		m_NavigationStatus =
			"Navigation bootstrap: place Nav Bounds and Bake for " +
			stagedContract.areaId;
		m_NavigationBakeStatus = "Create Nav Bounds";
		m_bNavigationBakeResetConfirmed = false;
		m_bNavigationBakeResetPending = false;
		return false;
	}

	CNavGridPaintDocument stagedNavigationDocument;
	if (!stagedNavigationDocument.Load(
		stagedContract.sourcePath,
		stagedContract.paintPath,
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}
	if (stagedNavigationDocument.Get_Desc().areaId !=
		stagedContract.areaId)
	{
		m_NavigationStatus =
			"NavGrid source area does not match the selected grid: " +
			stagedContract.areaId;
		return false;
	}

	CNavRuntimeBlockerDocument stagedBlockerDocument;
	if (!stagedBlockerDocument.Load(
		stagedContract.blockerPath,
		stagedNavigationDocument.Get_Desc(),
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			stagedBlockerDocument,
			m_WorldGameplayDocument,
			m_EncounterReference,
			stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	size_t stagedSelectedRuntimeRegion = m_iSelectedRuntimeRegion;
	if (stagedSelectedRuntimeRegion >=
		stagedBlockerDocument.Get_RegionCount())
	{
		stagedSelectedRuntimeRegion = 0;
	}
	const NAVGRID_BAKE_DESC stagedBakeDesc =
		stagedNavigationDocument.Get_BakeDesc();
	stagedStatus = stagedContract.runtimeGridAvailable ?
		"Saved" :
		"Authoring ready; save and re-enter ASSET_TEST for runtime navigation";

	m_NavigationSourcePath = stagedContract.sourcePath;
	m_NavigationPaintPath = stagedContract.paintPath;
	m_NavigationRuntimePath = stagedContract.runtimePath;
	m_RuntimeBlockerPath = stagedContract.blockerPath;
	m_NavigationDocument = std::move(stagedNavigationDocument);
	m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
	m_NavigationBakeDesc = stagedBakeDesc;
	m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
	m_NavigationStatus = stagedStatus;
	m_NavigationBakeStatus = "Baked source loaded";
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationStrokeActive = false;

	return !stagedContract.runtimeGridAvailable ||
		Register_RuntimeBlockers();
}

bool_t Client::CMapTool::Load_RuntimeBlockers()
{
	if (!m_NavigationDocument.Is_Ready())
	{
		m_NavigationStatus = "Load NavGrid source before runtime blockers";
		return false;
	}

	CNavRuntimeBlockerDocument stagedBlockerDocument;
	std::string stagedStatus;
	if (!stagedBlockerDocument.Load(
		m_RuntimeBlockerPath,
		m_NavigationDocument.Get_Desc(),
		stagedStatus))
	{
		m_NavigationStatus = stagedStatus;
		return false;
	}

	std::error_code runtimeError;
	const bool_t hasRuntime = std::filesystem::is_regular_file(
		m_NavigationRuntimePath, runtimeError);
	if (runtimeError)
	{
		m_NavigationStatus = "Could not inspect navigation runtime";
		return false;
	}
	size_t stagedSelectedRuntimeRegion = m_iSelectedRuntimeRegion;
	if (stagedSelectedRuntimeRegion >=
		stagedBlockerDocument.Get_RegionCount())
	{
		stagedSelectedRuntimeRegion = 0;
	}
	if (!hasRuntime)
	{
		m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
		m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
		m_NavigationStatus =
			"Authoring ready; save and re-enter ASSET_TEST for runtime navigation";
		return true;
	}

	m_RuntimeBlockerDocument = std::move(stagedBlockerDocument);
	m_iSelectedRuntimeRegion = stagedSelectedRuntimeRegion;
	m_NavigationStatus = stagedStatus;

	return Register_RuntimeBlockers();
}

bool_t Client::CMapTool::Register_RuntimeBlockers()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		m_NavigationStatus =
			"Runtime blocker registration is disabled in authoring workspace";
		return false;
	}

	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				m_iAuthoringLevelIndex,
				TEXT("Layer_Player"),
				TEXT("Com_Navigation"),
				0));
	if (nullptr == navigation)
	{
		m_NavigationStatus =
			"Character Navigation component is unavailable";
		return false;
	}

	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region)
			return false;

		const vector<uint32_t> cells =
			m_RuntimeBlockerDocument.Get_CellIndices(index);
		if (cells.empty())
			continue;

		const auto condition =
			m_NavigationConditions.find(region->conditionId);
		const bool_t conditionValue =
			condition != m_NavigationConditions.end() ?
			condition->second :
			false;
		const bool_t initiallyActive =
			conditionValue == region->activateWhenConditionTrue;
		if (!navigation->Register_RuntimeBlocker(
			region->id,
			cells,
			initiallyActive))
		{
			m_NavigationStatus =
				"Failed to register runtime blocker: " +
				region->id +
				"; re-enter AssetTest after editing region cells";
			return false;
		}
	}

	m_NavigationStatus =
		"Navigation ready: " +
		std::to_string(m_RuntimeBlockerDocument.Get_RegionCount()) +
		" runtime blocker regions";
	return true;
}

bool_t Client::CMapTool::Set_NavigationCondition(
	const std::string& conditionId,
	bool_t value)
{
	if (conditionId.empty())
		return false;

	m_NavigationConditions[conditionId] = value;
	if (CMapEditorWorkspaceService::Is_Active())
	{
		m_NavigationStatus =
			"Authoring preview condition changed; runtime Navigation was not mutated";
		return true;
	}
	shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			CGameInstance::Get().Get_Component(
				m_iAuthoringLevelIndex,
				TEXT("Layer_Player"),
				TEXT("Com_Navigation"),
				0));
	if (nullptr == navigation)
		return false;

	bool_t succeeded = true;
	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region ||
			region->conditionId != conditionId ||
			0 == m_RuntimeBlockerDocument.Get_RegionCellCount(index))
		{
			continue;
		}

		const bool_t active =
			value == region->activateWhenConditionTrue;
		succeeded =
			navigation->Set_RuntimeBlockerActive(region->id, active) &&
			succeeded;
	}
	return succeeded;
}

bool_t Client::CMapTool::Save_Navigation()
{
	if (CMapEditorWorkspaceService::Is_Active())
	{
		const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
		if (nullptr == active ||
			EDITOR_NAVIGATION_POLICY::NONE == active->navigationPolicy ||
			!m_NavigationDocument.Is_Ready())
		{
			m_NavigationStatus = "Navigation authoring is unavailable";
			return false;
		}
		MAP_NAVIGATION_CONTRACT expected;
		std::string expectedStatus;
		if (!Resolve_SelectedNavigationContract(expected, expectedStatus))
		{
			m_NavigationStatus = expectedStatus;
			return false;
		}
		/* A selected detail region is a different grid beside the descriptor
		   base, so the guard pins the exact selected paths instead. */
		const std::filesystem::path& expectedSource =
			m_NavigationRegionId.empty() ?
			active->navigationSource : expected.sourcePath;
		const std::filesystem::path& expectedPaint =
			m_NavigationRegionId.empty() ?
			active->navigationPaint : expected.paintPath;
		const std::filesystem::path& expectedBlockers =
			m_NavigationRegionId.empty() ?
			active->navigationBlockers : expected.blockerPath;
		if (!HasSameNavigationPath(m_NavigationSourcePath, expectedSource) ||
			!HasSameNavigationPath(m_NavigationPaintPath, expectedPaint) ||
			!HasSameNavigationPath(m_RuntimeBlockerPath, expectedBlockers))
		{
			m_NavigationStatus =
				"Navigation paths do not match the selected grid";
			return false;
		}
		if (m_DestructionDocument.Is_Ready())
		{
			std::string destructionStatus;
			if (!Validate_CurrentDestructionReferences(destructionStatus))
			{
				m_NavigationStatus = destructionStatus;
				m_DestructionStatus = destructionStatus;
				return false;
			}
		}

		std::string status;
		if (!m_NavigationDocument.Save_Paint(
			expectedPaint, status))
		{
			m_NavigationStatus = status;
			return false;
		}
		if (EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS ==
				active->navigationPolicy &&
			!m_RuntimeBlockerDocument.Save(
				expectedBlockers, status))
		{
			m_NavigationStatus = status;
			return false;
		}
		m_NavigationStatus =
			"Saved Data authoring only; publisher must build runtime navigation";
		return true;
	}

	m_NavigationStatus =
		"Navigation save is only available in the Map Editor workspace";
	return false;
}

bool_t Client::CMapTool::Resolve_SelectedNavigationContract(
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus) const
{
	/* In the Map Editor workspace the active descriptor owns the Area identity
	   and the catalog may not carry it, so the base grid path never went
	   through the catalog either. Regions have to resolve from the same
	   authority or they name a grid that does not exist. */
	const EDITOR_AREA_DESCRIPTOR* active =
		CMapEditorWorkspaceService::Is_Active() ?
		Get_ActiveEditorArea() : nullptr;
	const std::string& areaId = nullptr != active ?
		active->areaId : m_Catalog.Get_AreaId();
	if (areaId.empty())
	{
		outStatus = "Active navigation Area is unknown";
		return false;
	}
	if (m_NavigationRegionId.empty())
	{
		return CMapNavigationContract::Resolve_Area(
			areaId, outContract, outStatus);
	}
	return CMapNavigationContract::Resolve_Region(
		areaId, m_NavigationRegionId, outContract, outStatus);
}

bool_t Client::CMapTool::Select_NavigationRegion(std::string regionId)
{
	if (!regionId.empty() &&
		!CMapNavigationContract::Is_ValidRegionId(regionId))
	{
		m_NavigationStatus = "Navigation region ID is invalid";
		return false;
	}
	if (regionId == m_NavigationRegionId)
		return true;

	const std::string previous = m_NavigationRegionId;
	m_NavigationRegionId = regionId;
	if (Load_NavigationDocument())
		return true;

	/* A region whose navsource does not exist yet is not a failed switch: it
	   is the state every region is created in, and Bake is what writes the
	   file. Keeping the selection is what makes that bake land on the
	   region's own paths instead of the Area's base grid. */
	if (!regionId.empty())
	{
		MAP_NAVIGATION_CONTRACT contract;
		std::string contractStatus;
		std::error_code sourceError;
		if (Resolve_SelectedNavigationContract(contract, contractStatus) &&
			!std::filesystem::exists(contract.sourcePath, sourceError) &&
			!sourceError)
		{
			m_eNavigationMode = NAVIGATION_MODE::BAKE;
			m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
			m_bNavigationStrokeActive = false;
			return true;
		}
	}

	/* A region without a baked navsource is a normal state, and so is a
	   corrupt one: either way the editor must not keep showing the previous
	   grid's cells under the new selection. Load_NavigationDocument already
	   left an empty document and a status, so the selection only goes back
	   when the previous grid still loads. */
	/* Keep why the switch failed: reloading the previous grid succeeds and
	   would otherwise overwrite the message with its own success text. */
	const std::string failure = m_NavigationStatus;
	m_NavigationRegionId = previous;
	if (!Load_NavigationDocument())
		m_NavigationRegionId = regionId;
	m_NavigationStatus = failure;
	return false;
}

bool_t Client::CMapTool::Commit_NavigationRegionManifest()
{
	if (m_NavigationRegionId.empty())
		return true;

	const std::string& areaId = m_Catalog.Get_AreaId();
	std::vector<MAP_NAVIGATION_REGION> staged;
	std::string status;
	if (!CMapNavigationContract::Read_RegionManifest(areaId, staged, status))
	{
		m_NavigationStatus = status;
		return false;
	}

	const auto existing = std::find_if(
		staged.begin(),
		staged.end(),
		[this](const MAP_NAVIGATION_REGION& region)
		{
			return region.regionId == m_NavigationRegionId;
		});
	if (existing != staged.end())
	{
		existing->stepHeight = m_NewNavigationRegionStepHeight;
	}
	else
	{
		MAP_NAVIGATION_REGION added;
		added.regionId = m_NavigationRegionId;
		added.stepHeight = m_NewNavigationRegionStepHeight;
		staged.push_back(std::move(added));
	}

	if (!CMapNavigationContract::Write_RegionManifest(areaId, staged, status))
	{
		m_NavigationStatus = status;
		return false;
	}
	m_NavigationRegions = std::move(staged);
	m_NavigationStatus = status;
	return true;
}

bool_t Client::CMapTool::Try_PickNavigationCell(
	int32_t& outCellX,
	int32_t& outCellZ,
	f32_t& outWorldY)
{
	if (!m_NavigationDocument.Is_Ready())
	{
		m_NavigationStatus = "Navigation source is not loaded; select a baked Region first.";
		return false;
	}

	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked))
	{
		m_NavigationStatus = "No rendered surface under the cursor; click a visible map surface.";
		return false;
	}
	if (!std::isfinite(picked.y))
	{
		m_NavigationStatus = "Picked surface height is invalid; navigation was not changed.";
		return false;
	}
	if (!m_NavigationDocument.World_ToCell(
			XMLoadFloat4(&picked),
			outCellX,
			outCellZ))
	{
		m_NavigationStatus = "Picked surface is outside the selected navigation grid; check Region and Nav Bounds.";
		return false;
	}

	/* A cell the bake left without a surface used to fail the pick outright,
	   which is why an isolated platform could not be selected at all. The cell
	   only has to lie inside the grid; the paint decides what an unresolved
	   cell means for the chosen action. */
	outWorldY = picked.y;
	return true;
}

bool_t Client::CMapTool::Try_PaintNavigation()
{
	int32_t cellX = {};
	int32_t cellZ = {};
	f32_t pickedWorldY = {};
	if (!Try_PickNavigationCell(cellX, cellZ, pickedWorldY))
		return false;

	bool_t changed = false;
	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		const bool_t erase =
			NAVIGATION_EDIT_ACTION::ERASE ==
			m_eNavigationEditAction;
		changed = m_RuntimeBlockerDocument.Paint(
			m_iSelectedRuntimeRegion,
			cellX,
			cellZ,
			m_iBrushRadius,
			!erase);
	}
	else
	{
		NAVGRID_PAINT_OVERRIDE overrideState =
			NAVGRID_PAINT_OVERRIDE::INHERIT;
		switch (m_eNavigationEditAction)
		{
		case NAVIGATION_EDIT_ACTION::APPLY:
			overrideState =
				NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED;
			break;
		case NAVIGATION_EDIT_ACTION::FORCE_WALKABLE:
			overrideState =
				NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE;
			break;
		case NAVIGATION_EDIT_ACTION::ERASE:
		default:
			overrideState = NAVGRID_PAINT_OVERRIDE::INHERIT;
			break;
		}

		changed = m_NavigationDocument.Paint(
			cellX,
			cellZ,
			m_iBrushRadius,
			overrideState,
			pickedWorldY,
			NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE == overrideState &&
				m_bNavigationUsePickedHeight);
	}

	if (changed)
		m_NavigationStatus = "Unsaved";
	return changed;
}

bool_t Client::CMapTool::Try_PlaceNavigationBounds()
{
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) ||
		!std::isfinite(picked.x) ||
		!std::isfinite(picked.y) ||
		!std::isfinite(picked.z))
	{
		m_NavigationBakeStatus =
			"No rendered surface was picked for Nav Bounds";
		return false;
	}

	m_NavigationBakeDesc.position =
		float3_t(
			picked.x,
			picked.y + m_NavigationBakeDesc.size.y * 0.5f,
			picked.z);
	m_NavigationBakeDesc.isReady = true;
	m_eNavigationBoundsState = NAV_BOUNDS_STATE::IDLE;
	m_bNavigationBakeResetConfirmed = false;
	m_bNavigationBakeResetPending = false;
	m_NavigationBakeStatus =
		"Nav Bounds placed; adjust Transform and press Bake";
	return true;
	//���� Grid ������ �ʱ� Bounds�� ����Ѵ�
}

bool_t Client::CMapTool::Is_CellInsideNavigationBounds(
	f32_t worldX,
	f32_t worldZ) const
{
	if (!m_NavigationDocument.Is_Ready())
		return false;

	const NAVGRID_BAKE_DESC& bakeDesc =
		m_NavigationDocument.Get_BakeDesc();
	if (!Is_ValidNavigationBakeDesc(bakeDesc))
		return false;
	const f32_t radians =
		XMConvertToRadians(bakeDesc.yawDegrees);
	const f32_t cosine = std::cos(radians);
	const f32_t sine = std::sin(radians);
	const f32_t offsetX =
		worldX - bakeDesc.position.x;
	const f32_t offsetZ =
		worldZ - bakeDesc.position.z;
	const f32_t localX =
		cosine * offsetX - sine * offsetZ;
	const f32_t localZ =
		sine * offsetX + cosine * offsetZ;
	return
		std::abs(localX) <=
			bakeDesc.size.x * 0.5f + 0.00001f &&
		std::abs(localZ) <=
			bakeDesc.size.z * 0.5f + 0.00001f;
}
