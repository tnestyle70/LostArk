#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class NAVGRID_AUTHORING_CELL_STATE : uint8_t
{
	WALKABLE,
	BLOCKED,
	NO_SURFACE,
};

enum class NAVGRID_PAINT_OVERRIDE : uint8_t
{
	INHERIT,
	FORCE_BLOCKED,
	FORCE_WALKABLE,
};

struct NAVGRID_BAKE_DESC final
{
	float3_t position = {};
	float3_t size = float3_t(30.f, 10.f, 30.f);
	f32_t yawDegrees = 0.f;
	f32_t cellSize = 0.5f;
	f32_t maxSlopeDegrees = 50.f;
	bool_t isReady = false;
};

struct NAVGRID_AUTHORING_DESC final
{
	std::string areaId;
	uint32_t width = {};
	uint32_t height = {};
	f32_t cellSize = {};
	f32_t originX = {};
	f32_t originZ = {};
	NAVGRID_BAKE_DESC bake;
};

struct NAV_SOURCE_CELL final
{
	bool_t surfaceResolved = false;
	bool_t baseWalkable = false;
	f32_t height = {};
};

class CNavGridPaintDocument final
{
public:
	static constexpr uint32_t MAX_CELL_COUNT = 1000000;
	static constexpr uint32_t MAX_BRUSH_RADIUS = 8;

public:
	bool_t Load(
		const std::filesystem::path& sourcePath,
		const std::filesystem::path& paintPath,
		std::string& outStatus);
	bool_t Paint(
		int32_t cellX,
		int32_t cellZ,
		uint32_t brushRadius,
		NAVGRID_PAINT_OVERRIDE overrideState);
	bool_t Save_Paint(
		const std::filesystem::path& paintPath,
		std::string& outStatus);
	bool_t Export_Runtime(
		const std::filesystem::path& runtimePath,
		std::string& outStatus) const;

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	bool_t Is_ValidCell(int32_t cellX, int32_t cellZ) const;
	bool_t World_ToCell(
		fvector_t worldPosition,
		int32_t& outCellX,
		int32_t& outCellZ) const;
	uint32_t To_Index(int32_t cellX, int32_t cellZ) const;
	NAVGRID_AUTHORING_CELL_STATE Get_CellState(uint32_t index) const;
	bool_t Has_ResolvedHeight(uint32_t index) const;
	// MapTool overlay-only height; runtime export uses m_SourceCells directly.
	f32_t Get_CellHeight(uint32_t index) const;
	uint32_t Get_CellCount() const;
	uint32_t Get_BlockedCount() const;
	uint32_t Get_ForcedWalkableCount() const;
	uint32_t Get_HeightOverrideCount() const;
	uint32_t Get_ResolvedHeightCount() const;
	const NAVGRID_AUTHORING_DESC& Get_Desc() const { return m_Desc; }
	const NAVGRID_BAKE_DESC& Get_BakeDesc() const { return m_Desc.bake; }

private:
	NAVGRID_AUTHORING_DESC m_Desc;
	std::vector<NAV_SOURCE_CELL> m_BakedSourceCells;
	std::vector<NAV_SOURCE_CELL> m_SourceCells;
	std::vector<NAVGRID_PAINT_OVERRIDE> m_CellOverrides;
	/* NaN means that the bake owns the height. A finite value is an authored
	height correction for a resolved surface whose upper overlapping mesh was
	selected by the single-layer bake. */
	std::vector<f32_t> m_CellHeightOverrides;
	std::vector<f32_t> m_OverlayHeights;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
