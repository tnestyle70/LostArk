#include "MapNavigationContract.h"

#include "MapAssetCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>

namespace
{
	constexpr size_t MAX_AREA_ID_LENGTH = 64;
	constexpr const char* AREA_SELECTION_MAGIC =
		"LOSTARK_MAP_AREA_SELECTION";
	constexpr uint32_t AREA_SELECTION_VERSION = 1;
	constexpr const wchar_t* PROTOTYPE_PREFIX =
		L"Prototype_Component_Navigation_";

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
