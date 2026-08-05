#include "NavGridPaintDocument.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace
{
	constexpr const char* SOURCE_MAGIC = "LOSTARK_NAVGRID_SOURCE";
	constexpr const char* PAINT_MAGIC = "LOSTARK_NAVGRID_PAINT";
	constexpr uint32_t LEGACY_SOURCE_VERSION = 1;
	constexpr uint32_t SOURCE_VERSION = 2;
	constexpr uint32_t LEGACY_PAINT_VERSION = 1;
	constexpr uint32_t PAINT_VERSION = 2;

	bool_t IsSameFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t IsValidBakeDesc(const Client::NAVGRID_BAKE_DESC& desc)
	{
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

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code error;
		const bool_t destinationExists =
			std::filesystem::exists(destination, error);
		if (error)
			return false;

		if (destinationExists &&
			ReplaceFileW(
				destination.c_str(),
				temporary.c_str(),
				nullptr,
				REPLACEFILE_WRITE_THROUGH,
				nullptr,
				nullptr))
		{
			return true;
		}

		return MoveFileExW(
			temporary.c_str(),
			destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	void RemoveTemporaryFile(const std::filesystem::path& temporary)
	{
		std::error_code error;
		std::filesystem::remove(temporary, error);
	}
}

bool_t Client::CNavGridPaintDocument::Load(
	const std::filesystem::path& sourcePath,
	const std::filesystem::path& paintPath,
	std::string& outStatus)
{
	std::ifstream source(sourcePath, std::ios::binary);
	if (!source)
	{
		outStatus = "Could not open NavGrid source: " + sourcePath.string();
		return false;
	}

	std::string sourceMagic;
	uint32_t sourceVersion = {};
	NAVGRID_AUTHORING_DESC stagedDesc;
	uint64_t declaredCellCount = {};
	if (!(source >>
		sourceMagic >>
		sourceVersion >>
		std::quoted(stagedDesc.areaId) >>
		stagedDesc.width >>
		stagedDesc.height >>
		stagedDesc.cellSize >>
		stagedDesc.originX >>
		stagedDesc.originZ) ||
		sourceMagic != SOURCE_MAGIC ||
		(sourceVersion != LEGACY_SOURCE_VERSION &&
			sourceVersion != SOURCE_VERSION) ||
		stagedDesc.areaId.empty() ||
		0 == stagedDesc.width ||
		0 == stagedDesc.height ||
		!std::isfinite(stagedDesc.cellSize) ||
		stagedDesc.cellSize <= 0.f ||
		!std::isfinite(stagedDesc.originX) ||
		!std::isfinite(stagedDesc.originZ))
	{
		outStatus = "NavGrid source header is invalid";
		return false;
	}

	if (SOURCE_VERSION == sourceVersion)
	{
		uint32_t isReady = {};
		if (!(source >>
			stagedDesc.bake.position.x >>
			stagedDesc.bake.position.y >>
			stagedDesc.bake.position.z >>
			stagedDesc.bake.size.x >>
			stagedDesc.bake.size.y >>
			stagedDesc.bake.size.z >>
			stagedDesc.bake.yawDegrees >>
			stagedDesc.bake.maxSlopeDegrees >>
			isReady >>
			declaredCellCount) ||
			isReady > 1)
		{
			outStatus = "NavGrid source bake settings are invalid";
			return false;
		}

		stagedDesc.bake.cellSize = stagedDesc.cellSize;
		stagedDesc.bake.isReady = 0 != isReady;
		if (!IsValidBakeDesc(stagedDesc.bake))
		{
			outStatus = "NavGrid source bake settings are invalid";
			return false;
		}
	}
	else if (!(source >> declaredCellCount))
	{
		outStatus = "NavGrid source cell count is missing";
		return false;
	}

	const uint64_t cellCount64 =
		static_cast<uint64_t>(stagedDesc.width) * stagedDesc.height;
	if (0 == cellCount64 ||
		cellCount64 > MAX_CELL_COUNT ||
		cellCount64 != declaredCellCount)
	{
		outStatus = "NavGrid source cell count is invalid";
		return false;
	}

	const uint32_t cellCount = static_cast<uint32_t>(cellCount64);
	std::vector<NAV_SOURCE_CELL> stagedSourceCells(cellCount);
	std::vector<uint8_t> stagedSeen(cellCount, 0);
	for (uint32_t row = 0; row < cellCount; ++row)
	{
		int32_t cellX = {};
		int32_t cellZ = {};
		uint32_t surfaceResolved = {};
		uint32_t baseWalkable = {};
		f32_t height = {};
		if (!(source >> cellX >> cellZ >> surfaceResolved) ||
			(SOURCE_VERSION == sourceVersion &&
				!(source >> baseWalkable)) ||
			!(source >> height) ||
			cellX < 0 ||
			cellZ < 0 ||
			cellX >= static_cast<int32_t>(stagedDesc.width) ||
			cellZ >= static_cast<int32_t>(stagedDesc.height) ||
			surfaceResolved > 1 ||
			baseWalkable > 1 ||
			baseWalkable > surfaceResolved ||
			!std::isfinite(height))
		{
			outStatus = "NavGrid source row is invalid";
			return false;
		}

		const uint32_t index =
			static_cast<uint32_t>(cellZ) * stagedDesc.width +
			static_cast<uint32_t>(cellX);
		if (0 != stagedSeen[index])
		{
			outStatus = "NavGrid source has duplicate cells";
			return false;
		}

		stagedSeen[index] = 1;
		stagedSourceCells[index].surfaceResolved =
			0 != surfaceResolved;
		stagedSourceCells[index].baseWalkable =
			SOURCE_VERSION == sourceVersion ?
			0 != baseWalkable :
			0 != surfaceResolved;
		stagedSourceCells[index].height =
			0 != surfaceResolved ? height : 0.f;
	}

	if (LEGACY_SOURCE_VERSION == sourceVersion)
	{
		f32_t minimumHeight = (std::numeric_limits<f32_t>::max)();
		f32_t maximumHeight = -(std::numeric_limits<f32_t>::max)();
		for (const NAV_SOURCE_CELL& cell : stagedSourceCells)
		{
			if (!cell.surfaceResolved)
				continue;
			minimumHeight = (std::min)(minimumHeight, cell.height);
			maximumHeight = (std::max)(maximumHeight, cell.height);
		}

		stagedDesc.bake.position.x =
			stagedDesc.originX +
			static_cast<f32_t>(stagedDesc.width) *
			stagedDesc.cellSize * 0.5f;
		stagedDesc.bake.position.z =
			stagedDesc.originZ +
			static_cast<f32_t>(stagedDesc.height) *
			stagedDesc.cellSize * 0.5f;
		stagedDesc.bake.size.x =
			static_cast<f32_t>(stagedDesc.width) *
			stagedDesc.cellSize;
		stagedDesc.bake.size.z =
			static_cast<f32_t>(stagedDesc.height) *
			stagedDesc.cellSize;
		stagedDesc.bake.cellSize = stagedDesc.cellSize;
		stagedDesc.bake.maxSlopeDegrees = 50.f;
		stagedDesc.bake.yawDegrees = 0.f;
		stagedDesc.bake.isReady = true;

		if (minimumHeight <= maximumHeight)
		{
			constexpr f32_t verticalMargin = 2.f;
			stagedDesc.bake.position.y =
				(minimumHeight + maximumHeight) * 0.5f;
			stagedDesc.bake.size.y =
				(maximumHeight - minimumHeight) +
				verticalMargin * 2.f;
		}
	}

	source >> std::ws;
	if (source.peek() != std::char_traits<char>::eof())
	{
		outStatus = "NavGrid source has trailing data";
		return false;
	}

	std::vector<NAVGRID_PAINT_OVERRIDE> stagedCellOverrides(
		cellCount,
		NAVGRID_PAINT_OVERRIDE::INHERIT);
	std::error_code existsError;
	const bool_t paintExists =
		std::filesystem::exists(paintPath, existsError);
	if (existsError)
	{
		outStatus = "Could not inspect NavGrid paint file";
		return false;
	}

	if (paintExists)
	{
		std::ifstream paint(paintPath, std::ios::binary);
		std::string paintMagic;
		uint32_t paintVersion = {};
		std::string paintAreaId;
		uint32_t paintWidth = {};
		uint32_t paintHeight = {};
		f32_t paintCellSize = {};
		f32_t paintOriginX = {};
		f32_t paintOriginZ = {};
		uint64_t overrideCount = {};
		if (!paint ||
			!(paint >>
				paintMagic >>
				paintVersion >>
				std::quoted(paintAreaId) >>
				paintWidth >>
				paintHeight >>
				paintCellSize >>
				paintOriginX >>
				paintOriginZ >>
				overrideCount) ||
			paintMagic != PAINT_MAGIC ||
			(paintVersion != LEGACY_PAINT_VERSION &&
				paintVersion != PAINT_VERSION) ||
			paintAreaId != stagedDesc.areaId ||
			paintWidth != stagedDesc.width ||
			paintHeight != stagedDesc.height ||
			!IsSameFloat(paintCellSize, stagedDesc.cellSize) ||
			!IsSameFloat(paintOriginX, stagedDesc.originX) ||
			!IsSameFloat(paintOriginZ, stagedDesc.originZ) ||
			overrideCount > cellCount)
		{
			outStatus = "NavGrid paint header does not match source";
			return false;
		}

		for (uint64_t row = 0; row < overrideCount; ++row)
		{
			int32_t cellX = {};
			int32_t cellZ = {};
			std::string overrideName;
			if (!(paint >> cellX >> cellZ) ||
				(PAINT_VERSION == paintVersion &&
					!(paint >> overrideName)) ||
				cellX < 0 ||
				cellZ < 0 ||
				cellX >= static_cast<int32_t>(stagedDesc.width) ||
				cellZ >= static_cast<int32_t>(stagedDesc.height))
			{
				outStatus = "NavGrid paint row is invalid";
				return false;
			}

			const uint32_t index =
				static_cast<uint32_t>(cellZ) * stagedDesc.width +
				static_cast<uint32_t>(cellX);
			if (NAVGRID_PAINT_OVERRIDE::INHERIT !=
				stagedCellOverrides[index])
			{
				outStatus = "NavGrid paint has duplicate cells";
				return false;
			}

			NAVGRID_PAINT_OVERRIDE overrideState =
				NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED;
			if (PAINT_VERSION == paintVersion)
			{
				if ("BLOCKED" == overrideName)
				{
					overrideState =
						NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED;
				}
				else if ("WALKABLE" == overrideName)
				{
					overrideState =
						NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE;
				}
				else
				{
					outStatus = "NavGrid paint override is invalid";
					return false;
				}
			}

			if (!stagedSourceCells[index].surfaceResolved)
			{
				outStatus = "NavGrid paint targets an unresolved cell";
				return false;
			}
			stagedCellOverrides[index] = overrideState;
		}

		paint >> std::ws;
		if (paint.peek() != std::char_traits<char>::eof())
		{
			outStatus = "NavGrid paint has trailing data";
			return false;
		}
	}

	const bool_t hasResolvedHeight = std::any_of(
		stagedSourceCells.begin(),
		stagedSourceCells.end(),
		[](const NAV_SOURCE_CELL& cell)
		{
			return cell.surfaceResolved;
		});
	if (!hasResolvedHeight)
	{
		outStatus = "NavGrid source has no usable height";
		return false;
	}

	m_Desc = std::move(stagedDesc);
	m_SourceCells = std::move(stagedSourceCells);
	m_CellOverrides = std::move(stagedCellOverrides);
	m_isReady = true;
	m_isDirty = false;
	outStatus = paintExists
		? "Loaded navigation"
		: "Loaded navigation; using baked walkability";
	return true;
}

bool_t Client::CNavGridPaintDocument::Paint(
	int32_t cellX,
	int32_t cellZ,
	uint32_t brushRadius,
	NAVGRID_PAINT_OVERRIDE overrideState)
{
	if (!m_isReady ||
		brushRadius > MAX_BRUSH_RADIUS ||
		!Is_ValidCell(cellX, cellZ) ||
		(overrideState != NAVGRID_PAINT_OVERRIDE::INHERIT &&
			overrideState != NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED &&
			overrideState != NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE))
	{
		return false;
	}

	const int32_t radius = static_cast<int32_t>(brushRadius);
	const int32_t radiusSquared = radius * radius;
	bool_t changed = false;
	for (int32_t offsetZ = -radius; offsetZ <= radius; ++offsetZ)
	{
		for (int32_t offsetX = -radius; offsetX <= radius; ++offsetX)
		{
			if (offsetX * offsetX + offsetZ * offsetZ > radiusSquared)
				continue;

			const int32_t targetX = cellX + offsetX;
			const int32_t targetZ = cellZ + offsetZ;
			if (!Is_ValidCell(targetX, targetZ))
				continue;

			const uint32_t index = To_Index(targetX, targetZ);
			if (!Has_ResolvedHeight(index))
				continue;

			if (m_CellOverrides[index] == overrideState)
				continue;

			m_CellOverrides[index] = overrideState;
			changed = true;
		}
	}

	if (changed)
		m_isDirty = true;
	return changed;
}

bool_t Client::CNavGridPaintDocument::Save_Paint(
	const std::filesystem::path& paintPath,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "NavGrid document is not loaded";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		paintPath.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create NavGrid paint directory";
		return false;
	}

	std::filesystem::path temporary = paintPath;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary NavGrid paint";
		return false;
	}

	const uint32_t blockedCount = Get_BlockedCount();
	const uint32_t forcedWalkableCount = Get_ForcedWalkableCount();
	const uint32_t overrideCount = blockedCount + forcedWalkableCount;
	output <<
		PAINT_MAGIC << ' ' <<
		PAINT_VERSION << ' ' <<
		std::quoted(m_Desc.areaId) << ' ' <<
		m_Desc.width << ' ' <<
		m_Desc.height << ' ' <<
		std::setprecision(9) <<
		m_Desc.cellSize << ' ' <<
		m_Desc.originX << ' ' <<
		m_Desc.originZ << ' ' <<
		overrideCount << '\n';

	for (uint32_t cellZ = 0; cellZ < m_Desc.height; ++cellZ)
	{
		for (uint32_t cellX = 0; cellX < m_Desc.width; ++cellX)
		{
			const uint32_t index = cellZ * m_Desc.width + cellX;
			if (NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED ==
				m_CellOverrides[index])
			{
				output << cellX << ' ' << cellZ << " BLOCKED\n";
			}
			else if (NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE ==
				m_CellOverrides[index])
			{
				output << cellX << ' ' << cellZ << " WALKABLE\n";
			}
		}
	}

	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(paintPath, temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus = "Failed to commit NavGrid paint atomically";
		return false;
	}

	m_isDirty = false;
	outStatus =
		"Saved NavGrid paint: " +
		std::to_string(blockedCount) +
		" blocked, " +
		std::to_string(forcedWalkableCount) +
		" forced walkable cells";
	return true;
}

bool_t Client::CNavGridPaintDocument::Export_Runtime(
	const std::filesystem::path& runtimePath,
	std::string& outStatus) const
{
	if (!m_isReady)
	{
		outStatus = "Navigation is not loaded";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		runtimePath.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create navigation runtime directory";
		return false;
	}

	const uint32_t cellCount = Get_CellCount();
	uint32_t walkableCount = {};
	std::vector<uint8_t> walkable(cellCount, 0);
	std::vector<f32_t> heights(cellCount, 0.f);
	for (uint32_t index = 0; index < cellCount; ++index)
	{
		const bool_t hasSurface =
			m_SourceCells[index].surfaceResolved;
		bool_t isWalkable = false;
		if (hasSurface)
		{
			switch (m_CellOverrides[index])
			{
			case NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED:
				isWalkable = false;
				break;
			case NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE:
				isWalkable = true;
				break;
			case NAVGRID_PAINT_OVERRIDE::INHERIT:
			default:
				isWalkable = m_SourceCells[index].baseWalkable;
				break;
			}
		}
		walkable[index] = isWalkable ? 1 : 0;
		heights[index] =
			hasSurface ? m_SourceCells[index].height : 0.f;
		if (isWalkable)
			++walkableCount;
	}

	std::filesystem::path temporary = runtimePath;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary runtime navigation";
		return false;
	}

	output.write(
		reinterpret_cast<const char*>(&m_Desc.width),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.height),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.cellSize),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.originX),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.originZ),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(walkable.data()),
		static_cast<std::streamsize>(walkable.size()));
	output.write(
		reinterpret_cast<const char*>(heights.data()),
		static_cast<std::streamsize>(heights.size() * sizeof(f32_t)));
	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(runtimePath, temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus = "Failed to commit runtime navigation";
		return false;
	}

	outStatus =
		"Exported navigation: " +
		std::to_string(walkableCount) +
		" walkable cells";
	return true;
}

bool_t Client::CNavGridPaintDocument::Is_ValidCell(
	int32_t cellX,
	int32_t cellZ) const
{
	return m_isReady &&
		cellX >= 0 &&
		cellZ >= 0 &&
		cellX < static_cast<int32_t>(m_Desc.width) &&
		cellZ < static_cast<int32_t>(m_Desc.height);
}

bool_t Client::CNavGridPaintDocument::World_ToCell(
	fvector_t worldPosition,
	int32_t& outCellX,
	int32_t& outCellZ) const
{
	if (!m_isReady || m_Desc.cellSize <= 0.f)
		return false;

	const f32_t worldX = XMVectorGetX(worldPosition);
	const f32_t worldZ = XMVectorGetZ(worldPosition);
	if (!std::isfinite(worldX) || !std::isfinite(worldZ))
		return false;

	const f32_t cellX = (worldX - m_Desc.originX) / m_Desc.cellSize;
	const f32_t cellZ = (worldZ - m_Desc.originZ) / m_Desc.cellSize;
	if (!std::isfinite(cellX) ||
		!std::isfinite(cellZ) ||
		cellX < 0.f ||
		cellZ < 0.f ||
		cellX >= static_cast<f32_t>(m_Desc.width) ||
		cellZ >= static_cast<f32_t>(m_Desc.height))
	{
		return false;
	}

	outCellX = static_cast<int32_t>(std::floor(cellX));
	outCellZ = static_cast<int32_t>(std::floor(cellZ));
	return Is_ValidCell(outCellX, outCellZ);
}

uint32_t Client::CNavGridPaintDocument::To_Index(
	int32_t cellX,
	int32_t cellZ) const
{
	return static_cast<uint32_t>(cellZ) * m_Desc.width +
		static_cast<uint32_t>(cellX);
}

NAVGRID_AUTHORING_CELL_STATE
Client::CNavGridPaintDocument::Get_CellState(uint32_t index) const
{
	if (!Has_ResolvedHeight(index))
		return NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE;
	if (NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE ==
		m_CellOverrides[index])
	{
		return NAVGRID_AUTHORING_CELL_STATE::WALKABLE;
	}
	if (NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED ==
		m_CellOverrides[index] ||
		!m_SourceCells[index].baseWalkable)
		return NAVGRID_AUTHORING_CELL_STATE::BLOCKED;
	return NAVGRID_AUTHORING_CELL_STATE::WALKABLE;
}

bool_t Client::CNavGridPaintDocument::Has_ResolvedHeight(
	uint32_t index) const
{
	return m_isReady &&
		index < m_SourceCells.size() &&
		m_SourceCells[index].surfaceResolved;
}

f32_t Client::CNavGridPaintDocument::Get_CellHeight(
	uint32_t index) const
{
	return Has_ResolvedHeight(index) ?
		m_SourceCells[index].height :
		0.f;
}

uint32_t Client::CNavGridPaintDocument::Get_CellCount() const
{
	return static_cast<uint32_t>(m_SourceCells.size());
}

uint32_t Client::CNavGridPaintDocument::Get_BlockedCount() const
{
	uint32_t count = {};
	for (uint32_t index = 0; index < m_SourceCells.size(); ++index)
	{
		if (m_SourceCells[index].surfaceResolved &&
			NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED ==
			m_CellOverrides[index])
		{
			++count;
		}
	}
	return count;
}

uint32_t Client::CNavGridPaintDocument::Get_ForcedWalkableCount() const
{
	uint32_t count = {};
	for (uint32_t index = 0; index < m_SourceCells.size(); ++index)
	{
		if (m_SourceCells[index].surfaceResolved &&
			NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE ==
			m_CellOverrides[index])
		{
			++count;
		}
	}
	return count;
}

uint32_t Client::CNavGridPaintDocument::Get_ResolvedHeightCount() const
{
	return static_cast<uint32_t>(std::count_if(
		m_SourceCells.begin(),
		m_SourceCells.end(),
		[](const NAV_SOURCE_CELL& cell)
		{
			return cell.surfaceResolved;
		}));
}
