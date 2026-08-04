#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>

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
};

NS_END
