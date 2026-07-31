#include "NavRuntimeBlockerDocument.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* MAGIC = "LOSTARK_NAVGRID_BLOCKERS";
	constexpr uint32_t VERSION = 1;

	bool_t IsSameFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t IsValidId(const std::string& value)
	{
		if (value.empty() || value.size() > 128)
			return false;

		return std::all_of(
			value.begin(),
			value.end(),
			[](unsigned char character)
			{
				return std::isalnum(character) ||
					'_' == character ||
					'-' == character ||
					'.' == character;
			});
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
}

bool_t Client::CNavRuntimeBlockerDocument::Load(
	const std::filesystem::path& path,
	const NAVGRID_AUTHORING_DESC& expectedDesc,
	std::string& outStatus)
{
	if (expectedDesc.areaId.empty() ||
		0 == expectedDesc.width ||
		0 == expectedDesc.height ||
		expectedDesc.cellSize <= 0.f)
	{
		outStatus = "Runtime blocker grid identity is invalid";
		return false;
	}

	std::error_code existsError;
	const bool_t exists = std::filesystem::exists(path, existsError);
	if (existsError)
	{
		outStatus = "Could not inspect runtime blocker document";
		return false;
	}

	if (!exists)
	{
		m_Desc = expectedDesc;
		m_Regions.clear();
		m_isReady = true;
		m_isDirty = false;
		outStatus = "No runtime blocker document; start with zero regions";
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	std::string magic;
	uint32_t version = {};
	NAVGRID_AUTHORING_DESC stagedDesc;
	uint64_t regionCount = {};
	if (!input ||
		!(input >>
			magic >>
			version >>
			std::quoted(stagedDesc.areaId) >>
			stagedDesc.width >>
			stagedDesc.height >>
			stagedDesc.cellSize >>
			stagedDesc.originX >>
			stagedDesc.originZ >>
			regionCount) ||
		magic != MAGIC ||
		version != VERSION ||
		stagedDesc.areaId != expectedDesc.areaId ||
		stagedDesc.width != expectedDesc.width ||
		stagedDesc.height != expectedDesc.height ||
		!IsSameFloat(stagedDesc.cellSize, expectedDesc.cellSize) ||
		!IsSameFloat(stagedDesc.originX, expectedDesc.originX) ||
		!IsSameFloat(stagedDesc.originZ, expectedDesc.originZ) ||
		regionCount > MAX_REGION_COUNT)
	{
		outStatus = "Runtime blocker header does not match NavGrid source";
		return false;
	}

	const uint32_t cellCount = stagedDesc.width * stagedDesc.height;
	std::vector<NAV_RUNTIME_BLOCKER_REGION> stagedRegions;
	stagedRegions.reserve(static_cast<size_t>(regionCount));
	std::unordered_set<std::string> seenIds;
	for (uint64_t regionRow = 0; regionRow < regionCount; ++regionRow)
	{
		std::string rowMagic;
		NAV_RUNTIME_BLOCKER_REGION region;
		uint32_t activateWhenTrue = {};
		uint64_t regionCellCount = {};
		if (!(input >>
			rowMagic >>
			std::quoted(region.id) >>
			std::quoted(region.conditionId) >>
			activateWhenTrue >>
			regionCellCount) ||
			rowMagic != "REGION" ||
			!IsValidId(region.id) ||
			!IsValidId(region.conditionId) ||
			activateWhenTrue > 1 ||
			regionCellCount > cellCount ||
			!seenIds.emplace(region.id).second)
		{
			outStatus = "Runtime blocker region header is invalid";
			return false;
		}

		region.activateWhenConditionTrue = 0 != activateWhenTrue;
		region.cells.assign(cellCount, 0);
		for (uint64_t cellRow = 0; cellRow < regionCellCount; ++cellRow)
		{
			int32_t cellX = {};
			int32_t cellZ = {};
			if (!(input >> cellX >> cellZ) ||
				cellX < 0 ||
				cellZ < 0 ||
				cellX >= static_cast<int32_t>(stagedDesc.width) ||
				cellZ >= static_cast<int32_t>(stagedDesc.height))
			{
				outStatus = "Runtime blocker cell is invalid";
				return false;
			}

			const uint32_t index =
				static_cast<uint32_t>(cellZ) * stagedDesc.width +
				static_cast<uint32_t>(cellX);
			if (0 != region.cells[index])
			{
				outStatus = "Runtime blocker region has duplicate cells";
				return false;
			}
			region.cells[index] = 1;
		}

		stagedRegions.push_back(std::move(region));
	}

	input >> std::ws;
	if (input.peek() != std::char_traits<char>::eof())
	{
		outStatus = "Runtime blocker document has trailing data";
		return false;
	}

	m_Desc = stagedDesc;
	m_Regions = std::move(stagedRegions);
	m_isReady = true;
	m_isDirty = false;
	outStatus =
		"Loaded " +
		std::to_string(m_Regions.size()) +
		" runtime blocker regions";
	return true;
}

bool_t Client::CNavRuntimeBlockerDocument::Add_Region(
	const std::string& id,
	const std::string& conditionId,
	bool_t activateWhenConditionTrue,
	std::string& outStatus)
{
	if (!m_isReady ||
		!IsValidId(id) ||
		!IsValidId(conditionId) ||
		m_Regions.size() >= MAX_REGION_COUNT)
	{
		outStatus = "Runtime blocker id/condition is invalid";
		return false;
	}

	if (std::any_of(
		m_Regions.begin(),
		m_Regions.end(),
		[&id](const NAV_RUNTIME_BLOCKER_REGION& region)
		{
			return region.id == id;
		}))
	{
		outStatus = "Runtime blocker id already exists";
		return false;
	}

	NAV_RUNTIME_BLOCKER_REGION region;
	region.id = id;
	region.conditionId = conditionId;
	region.activateWhenConditionTrue = activateWhenConditionTrue;
	region.cells.assign(m_Desc.width * m_Desc.height, 0);
	m_Regions.push_back(std::move(region));
	m_isDirty = true;
	outStatus = "Added runtime blocker region: " + id;
	return true;
}

bool_t Client::CNavRuntimeBlockerDocument::Paint(
	size_t regionIndex,
	int32_t cellX,
	int32_t cellZ,
	uint32_t brushRadius,
	bool_t included)
{
	if (!m_isReady ||
		regionIndex >= m_Regions.size() ||
		brushRadius > CNavGridPaintDocument::MAX_BRUSH_RADIUS ||
		cellX < 0 ||
		cellZ < 0 ||
		cellX >= static_cast<int32_t>(m_Desc.width) ||
		cellZ >= static_cast<int32_t>(m_Desc.height))
	{
		return false;
	}

	NAV_RUNTIME_BLOCKER_REGION& region = m_Regions[regionIndex];
	const int32_t radius = static_cast<int32_t>(brushRadius);
	const int32_t radiusSquared = radius * radius;
	const uint8_t newValue = included ? 1 : 0;
	bool_t changed = false;
	for (int32_t offsetZ = -radius; offsetZ <= radius; ++offsetZ)
	{
		for (int32_t offsetX = -radius; offsetX <= radius; ++offsetX)
		{
			if (offsetX * offsetX + offsetZ * offsetZ > radiusSquared)
				continue;

			const int32_t targetX = cellX + offsetX;
			const int32_t targetZ = cellZ + offsetZ;
			if (targetX < 0 ||
				targetZ < 0 ||
				targetX >= static_cast<int32_t>(m_Desc.width) ||
				targetZ >= static_cast<int32_t>(m_Desc.height))
			{
				continue;
			}

			const uint32_t index =
				static_cast<uint32_t>(targetZ) * m_Desc.width +
				static_cast<uint32_t>(targetX);
			if (region.cells[index] != newValue)
			{
				region.cells[index] = newValue;
				changed = true;
			}
		}
	}

	if (changed)
		m_isDirty = true;
	return changed;
}

bool_t Client::CNavRuntimeBlockerDocument::Save(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "Runtime blocker document is not loaded";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create runtime blocker directory";
		return false;
	}

	std::filesystem::path temporary = path;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary runtime blocker file";
		return false;
	}

	output <<
		MAGIC << ' ' <<
		VERSION << ' ' <<
		std::quoted(m_Desc.areaId) << ' ' <<
		m_Desc.width << ' ' <<
		m_Desc.height << ' ' <<
		std::setprecision(9) <<
		m_Desc.cellSize << ' ' <<
		m_Desc.originX << ' ' <<
		m_Desc.originZ << ' ' <<
		m_Regions.size() << '\n';

	for (const NAV_RUNTIME_BLOCKER_REGION& region : m_Regions)
	{
		const uint32_t cellCount = static_cast<uint32_t>(std::count(
			region.cells.begin(),
			region.cells.end(),
			static_cast<uint8_t>(1)));
		output <<
			"REGION " <<
			std::quoted(region.id) << ' ' <<
			std::quoted(region.conditionId) << ' ' <<
			(region.activateWhenConditionTrue ? 1 : 0) << ' ' <<
			cellCount << '\n';
		for (uint32_t index = 0; index < region.cells.size(); ++index)
		{
			if (0 != region.cells[index])
				output << index % m_Desc.width << ' ' <<
					index / m_Desc.width << '\n';
		}
	}

	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit runtime blocker file atomically";
		return false;
	}

	m_isDirty = false;
	outStatus =
		"Saved " +
		std::to_string(m_Regions.size()) +
		" runtime blocker regions";
	return true;
}

const Client::NAV_RUNTIME_BLOCKER_REGION*
Client::CNavRuntimeBlockerDocument::Get_Region(size_t index) const
{
	return index < m_Regions.size() ? &m_Regions[index] : nullptr;
}

uint32_t Client::CNavRuntimeBlockerDocument::Get_RegionCellCount(
	size_t index) const
{
	const NAV_RUNTIME_BLOCKER_REGION* region = Get_Region(index);
	if (nullptr == region)
		return 0;

	return static_cast<uint32_t>(std::count(
		region->cells.begin(),
		region->cells.end(),
		static_cast<uint8_t>(1)));
}

bool_t Client::CNavRuntimeBlockerDocument::Is_CellInRegion(
	size_t regionIndex,
	uint32_t cellIndex) const
{
	const NAV_RUNTIME_BLOCKER_REGION* region = Get_Region(regionIndex);
	return nullptr != region &&
		cellIndex < region->cells.size() &&
		0 != region->cells[cellIndex];
}

vector<uint32_t> Client::CNavRuntimeBlockerDocument::Get_CellIndices(
	size_t regionIndex) const
{
	vector<uint32_t> indices;
	const NAV_RUNTIME_BLOCKER_REGION* region = Get_Region(regionIndex);
	if (nullptr == region)
		return indices;

	indices.reserve(Get_RegionCellCount(regionIndex));
	for (uint32_t index = 0; index < region->cells.size(); ++index)
	{
		if (0 != region->cells[index])
			indices.push_back(index);
	}
	return indices;
}
