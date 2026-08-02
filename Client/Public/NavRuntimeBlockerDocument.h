#pragma once

#include "NavGridPaintDocument.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct NAV_RUNTIME_BLOCKER_REGION final
{
	std::string id;
	std::string conditionId;
	bool_t activateWhenConditionTrue = true;
	std::vector<uint8_t> cells;
};

class CNavRuntimeBlockerDocument final
{
public:
	static constexpr uint32_t MAX_REGION_COUNT = 256;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const NAVGRID_AUTHORING_DESC& expectedDesc,
		std::string& outStatus);
	bool_t Add_Region(
		const std::string& id,
		const std::string& conditionId,
		bool_t activateWhenConditionTrue,
		std::string& outStatus);
	bool_t Paint(
		size_t regionIndex,
		int32_t cellX,
		int32_t cellZ,
		uint32_t brushRadius,
		bool_t included);
	bool_t Save(
		const std::filesystem::path& path,
		std::string& outStatus);

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	const NAVGRID_AUTHORING_DESC& Get_Desc() const { return m_Desc; }
	size_t Get_RegionCount() const { return m_Regions.size(); }
	const NAV_RUNTIME_BLOCKER_REGION* Get_Region(size_t index) const;
	uint32_t Get_RegionCellCount(size_t index) const;
	bool_t Is_CellInRegion(size_t regionIndex, uint32_t cellIndex) const;
	vector<uint32_t> Get_CellIndices(size_t regionIndex) const;

private:
	NAVGRID_AUTHORING_DESC m_Desc;
	std::vector<NAV_RUNTIME_BLOCKER_REGION> m_Regions;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
