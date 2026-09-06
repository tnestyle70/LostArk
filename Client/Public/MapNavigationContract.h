#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct MAP_NAVIGATION_CONTRACT final
{
	std::string areaId;
	std::filesystem::path sourcePath;
	std::filesystem::path paintPath;
	std::filesystem::path runtimePath;
	std::filesystem::path blockerPath;
	std::wstring prototypeTag;
	bool_t runtimeGridAvailable = false;
};

/* One row of Data/Navigation/<AreaId>.navregions. regionId is the stable
   authoring id; the grid it names is "<AreaId>.<regionId>" and owns its own
   navsource/navpaint/navgrid. stepHeight is that grid's runtime adjacent-step
   limit, republished into its .navpolicy. */
struct MAP_NAVIGATION_REGION final
{
	std::string regionId;
	f32_t stepHeight = 1.f;
};

class CMapNavigationContract final
{
public:
	static bool_t Resolve_Active(
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus);
	static bool_t Resolve_Area(
		const std::string& areaId,
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus);
	static bool_t Is_ValidAreaId(const std::string& areaId);
	/* Resolves the paths of the detail grid "<AreaId>.<regionId>". The grid is
	   an ordinary navigation grid, so this only builds the composed id and
	   defers to Resolve_Area. */
	static bool_t Resolve_Region(
		const std::string& areaId,
		const std::string& regionId,
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus);
	/* A missing manifest is not an error: it means the Area has no detail
	   regions. A malformed one is, so the caller keeps its previous list. */
	static bool_t Read_RegionManifest(
		const std::string& areaId,
		std::vector<MAP_NAVIGATION_REGION>& outRegions,
		std::string& outStatus);
	static bool_t Write_RegionManifest(
		const std::string& areaId,
		const std::vector<MAP_NAVIGATION_REGION>& regions,
		std::string& outStatus);
	static std::filesystem::path Resolve_RegionManifestPath(
		const std::string& areaId);
	static bool_t Is_ValidRegionId(const std::string& regionId);
};

NS_END
