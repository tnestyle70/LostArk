#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CMapAssetCatalog;

struct MAP_PLACEMENT_RECORD
{
	uint64_t placementId = {};
	std::string sourcePlacementId;
	std::string sourceLevel;
	std::string transformSource;
	std::string assetId;
	float3_t position = {};
	float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t signedScale = float3_t(1.f, 1.f, 1.f);
	bool_t visible = true;
};

class CMapPlacementDocument final
{
public:
	static constexpr uint32_t MAX_PLACEMENT_COUNT = 65536;
	static constexpr uint64_t MAX_EDITOR_PLACEMENT_ID = 0x7fffffffffffffffull;

	static bool_t Read(const std::filesystem::path& path,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_PLACEMENT_RECORD>& outRecords,
		std::string& outStatus);
	static bool_t Write(const std::filesystem::path& path,
		const std::string& areaId,
		const std::vector<MAP_PLACEMENT_RECORD>& records,
		const CMapAssetCatalog& catalog,
		std::string& outStatus,
		std::vector<MAP_PLACEMENT_RECORD>* outStoredRecords = nullptr);
	static bool_t Is_Valid(const MAP_PLACEMENT_RECORD& record,
		const CMapAssetCatalog& catalog);
};

NS_END
