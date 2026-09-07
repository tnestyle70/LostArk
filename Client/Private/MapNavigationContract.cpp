#include "MapNavigationContract.h"

#include "MapAssetCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <system_error>

namespace
{
	constexpr size_t MAX_AREA_ID_LENGTH = 64;
	constexpr const char* AREA_SELECTION_MAGIC =
		"LOSTARK_MAP_AREA_SELECTION";
	constexpr uint32_t AREA_SELECTION_VERSION = 1;
	constexpr const wchar_t* PROTOTYPE_PREFIX =
		L"Prototype_Component_Navigation_";
	constexpr const char* REGION_MANIFEST_MAGIC =
		"LOSTARK_NAVGRID_REGIONS";
	constexpr uint32_t REGION_MANIFEST_VERSION = 1;
	constexpr size_t MAX_REGION_ID_LENGTH = 32;
	constexpr size_t MAX_REGION_COUNT = 64;

	std::wstring ToWideAscii(const std::string& value)
	{
		return std::wstring(value.begin(), value.end());
	}

	bool_t IsMissingPathError(const std::error_code& error)
	{
		return error == std::errc::no_such_file_or_directory ||
			(error.category() == std::system_category() &&
				(ERROR_FILE_NOT_FOUND == error.value() ||
					ERROR_PATH_NOT_FOUND == error.value()));
	}
}

bool_t Client::CMapNavigationContract::Resolve_Active(
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus)
{
	const std::filesystem::path selectionPath =
		CMapAssetCatalog::Get_AreaSelectionPath();
	std::ifstream input(selectionPath, std::ios::binary);
	std::string magic;
	std::string areaId;
	uint32_t version = {};
	if (!input || !(input >> magic >> version >> std::quoted(areaId)) ||
		magic != AREA_SELECTION_MAGIC ||
		version != AREA_SELECTION_VERSION ||
		!Is_ValidAreaId(areaId))
	{
		outStatus = "Active map area selection is invalid: " +
			selectionPath.string();
		return false;
	}

	std::string trailing;
	if (input >> trailing)
	{
		outStatus = "Active map area selection has trailing data";
		return false;
	}

	return Resolve_Area(areaId, outContract, outStatus);
}

bool_t Client::CMapNavigationContract::Resolve_Area(
	const std::string& areaId,
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus)
{
	if (!Is_ValidAreaId(areaId))
	{
		outStatus = "Navigation area ID is invalid";
		return false;
	}

	const std::filesystem::path mapRoot =
		CMapAssetCatalog::Get_MapDataRoot();
	const std::filesystem::path authoringRoot =
		CProjectDataRoot::Resolve(L"Navigation");
	if (mapRoot.empty() || mapRoot.parent_path().empty() ||
		authoringRoot.empty())
	{
		outStatus = "Navigation data root is unavailable";
		return false;
	}

	const std::filesystem::path runtimeRoot =
		(mapRoot.parent_path() / L"Navigation").lexically_normal();
	const std::wstring stem = ToWideAscii(areaId);

	MAP_NAVIGATION_CONTRACT staged;
	staged.areaId = areaId;
	staged.sourcePath = authoringRoot / (stem + L".navsource");
	staged.paintPath = authoringRoot / (stem + L".navpaint");
	staged.runtimePath = runtimeRoot / (stem + L".navgrid");
	staged.blockerPath = authoringRoot / (stem + L".navblockers");
	staged.prototypeTag = PROTOTYPE_PREFIX + stem;

	std::error_code runtimeError;
	const std::filesystem::file_status runtimeStatus =
		std::filesystem::status(staged.runtimePath, runtimeError);
	if (runtimeError && !IsMissingPathError(runtimeError))
	{
		outStatus = "Could not inspect navigation runtime: " +
			staged.runtimePath.string() + " (" +
			runtimeError.message() + ")";
		return false;
	}
	if (runtimeError)
	{
		staged.runtimeGridAvailable = false;
	}
	else if (!std::filesystem::exists(runtimeStatus))
	{
		staged.runtimeGridAvailable = false;
	}
	else if (!std::filesystem::is_regular_file(runtimeStatus))
	{
		outStatus = "Navigation runtime is not a regular file: " +
			staged.runtimePath.string();
		return false;
	}
	else
	{
		staged.runtimeGridAvailable = true;
	}

	outContract = std::move(staged);
	outStatus = outContract.runtimeGridAvailable ?
		"Navigation runtime ready for " + areaId :
		"Navigation bootstrap pending for " + areaId;
	return true;
}

bool_t Client::CMapNavigationContract::Is_ValidAreaId(
	const std::string& areaId)
{
	if (areaId.empty() || areaId.size() > MAX_AREA_ID_LENGTH)
		return false;

	return std::all_of(
		areaId.begin(),
		areaId.end(),
		[](const char value)
		{
			const unsigned char character =
				static_cast<unsigned char>(value);
			return 0 != std::isalnum(character) ||
				'_' == value || '-' == value || '.' == value;
		});
}

bool_t Client::CMapNavigationContract::Is_ValidRegionId(
	const std::string& regionId)
{
	/* The dot is what separates the Area from its region in a grid id, so a
	   region id must not contain one. */
	if (regionId.empty() || regionId.size() > MAX_REGION_ID_LENGTH)
		return false;

	return std::all_of(
		regionId.begin(),
		regionId.end(),
		[](const char value)
		{
			const unsigned char character =
				static_cast<unsigned char>(value);
			return 0 != std::isalnum(character) ||
				'_' == value || '-' == value;
		});
}

bool_t Client::CMapNavigationContract::Resolve_Region(
	const std::string& areaId,
	const std::string& regionId,
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus)
{
	if (!Is_ValidAreaId(areaId) || !Is_ValidRegionId(regionId))
	{
		outStatus = "Navigation region ID is invalid";
		return false;
	}
	return Resolve_Area(areaId + "." + regionId, outContract, outStatus);
}

std::filesystem::path
Client::CMapNavigationContract::Resolve_RegionManifestPath(
	const std::string& areaId)
{
	const std::filesystem::path authoringRoot =
		CProjectDataRoot::Resolve(L"Navigation");
	if (authoringRoot.empty() || !Is_ValidAreaId(areaId))
		return {};
	return authoringRoot / (ToWideAscii(areaId) + L".navregions");
}

bool_t Client::CMapNavigationContract::Read_RegionManifest(
	const std::string& areaId,
	std::vector<MAP_NAVIGATION_REGION>& outRegions,
	std::string& outStatus)
{
	outRegions.clear();
	const std::filesystem::path path = Resolve_RegionManifestPath(areaId);
	if (path.empty())
	{
		outStatus = "Navigation data root is unavailable";
		return false;
	}

	std::error_code existsError;
	const bool_t exists = std::filesystem::exists(path, existsError);
	if (existsError && !IsMissingPathError(existsError))
	{
		outStatus = "Could not inspect navigation region manifest: " +
			path.string();
		return false;
	}
	if (!exists)
	{
		// No manifest means no detail regions, which is the normal state.
		outStatus = "No navigation regions for " + areaId;
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	std::string magic;
	std::string stagedAreaId;
	uint32_t version = {};
	uint32_t regionCount = {};
	if (!input || !(input >> magic >> version >> std::quoted(stagedAreaId) >>
		regionCount) ||
		magic != REGION_MANIFEST_MAGIC ||
		version != REGION_MANIFEST_VERSION ||
		stagedAreaId != areaId ||
		regionCount > MAX_REGION_COUNT)
	{
		outStatus = "Navigation region manifest header is invalid: " +
			path.string();
		return false;
	}

	std::vector<MAP_NAVIGATION_REGION> staged;
	staged.reserve(regionCount);
	for (uint32_t index = 0; index < regionCount; ++index)
	{
		std::string rowMagic;
		MAP_NAVIGATION_REGION region;
		if (!(input >> rowMagic >> std::quoted(region.regionId) >>
			region.stepHeight) ||
			rowMagic != "REGION" ||
			!Is_ValidRegionId(region.regionId) ||
			!std::isfinite(region.stepHeight) ||
			region.stepHeight < 0.f ||
			staged.end() != std::find_if(
				staged.begin(),
				staged.end(),
				[&region](const MAP_NAVIGATION_REGION& existing)
				{
					return existing.regionId == region.regionId;
				}))
		{
			outStatus = "Navigation region manifest row is invalid: " +
				path.string();
			return false;
		}
		staged.push_back(std::move(region));
	}

	input >> std::ws;
	if (!input.eof())
	{
		outStatus = "Navigation region manifest has trailing data: " +
			path.string();
		return false;
	}

	outRegions = std::move(staged);
	outStatus = "Loaded " + std::to_string(outRegions.size()) +
		" navigation regions for " + areaId;
	return true;
}

bool_t Client::CMapNavigationContract::Write_RegionManifest(
	const std::string& areaId,
	const std::vector<MAP_NAVIGATION_REGION>& regions,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_RegionManifestPath(areaId);
	if (path.empty() || regions.size() > MAX_REGION_COUNT)
	{
		outStatus = "Navigation region manifest target is invalid";
		return false;
	}

	std::ostringstream text;
	text << REGION_MANIFEST_MAGIC << ' ' << REGION_MANIFEST_VERSION << ' ' <<
		std::quoted(areaId) << ' ' << regions.size() << '\n';
	for (const MAP_NAVIGATION_REGION& region : regions)
	{
		if (!Is_ValidRegionId(region.regionId) ||
			!std::isfinite(region.stepHeight) ||
			region.stepHeight < 0.f)
		{
			outStatus = "Navigation region row is invalid: " + region.regionId;
			return false;
		}
		text << "REGION " << std::quoted(region.regionId) << ' ' <<
			std::setprecision(9) << region.stepHeight << '\n';
	}

	/* Written beside the target and moved over it so a failed write never
	   leaves a half manifest that would fail the next Area load. */
	std::filesystem::path staged = path;
	staged += L".staging";
	std::error_code error;
	std::filesystem::remove(staged, error);
	error.clear();
	{
		std::ofstream output(staged, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			outStatus = "Could not open navigation region manifest for writing";
			return false;
		}
		const std::string payload = text.str();
		output.write(payload.data(),
			static_cast<std::streamsize>(payload.size()));
		output.flush();
		const bool_t wroteSuccessfully = output.good();
		output.close();
		if (!wroteSuccessfully)
		{
			std::filesystem::remove(staged, error);
			outStatus = "Could not write navigation region manifest";
			return false;
		}
	}
	std::filesystem::rename(staged, path, error);
	if (error)
	{
		std::error_code cleanupError;
		std::filesystem::remove(staged, cleanupError);
		outStatus = "Could not promote navigation region manifest";
		return false;
	}

	outStatus = "Saved " + std::to_string(regions.size()) +
		" navigation regions for " + areaId;
	return true;
}
