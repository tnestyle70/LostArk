#pragma once

#include "Client_Defines.h"
#include "NavGridPaintDocument.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct NAVGRID_BAKE_PLACEMENT final
{
	std::string assetId;
	std::filesystem::path modelPath;
	float4x4_t world = {};
};

struct NAVGRID_BAKE_RESULT final
{
	NAVGRID_AUTHORING_DESC desc;
	std::vector<NAV_SOURCE_CELL> cells;
	uint32_t placementCount = {};
	uint64_t triangleCount = {};
	uint32_t resolvedCellCount = {};
};

class CNavGridBaker final
{
public:
	static bool_t Build_Desc(
		const std::string& areaId,
		const NAVGRID_BAKE_DESC& bakeDesc,
		NAVGRID_AUTHORING_DESC& outDesc,
		std::string& outStatus);

	static bool_t Build(
		const std::string& areaId,
		const NAVGRID_BAKE_DESC& bakeDesc,
		const std::vector<NAVGRID_BAKE_PLACEMENT>& placements,
		NAVGRID_BAKE_RESULT& outResult,
		std::string& outStatus);

	static bool_t Save_Source(
		const NAVGRID_BAKE_RESULT& result,
		const std::filesystem::path& path,
		std::string& outStatus);
};

NS_END
