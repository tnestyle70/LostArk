#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class MAP_ASSET_ANCHOR
{
	ORIGIN,
	BOTTOM_CENTER,
};

struct MAP_ASSET_ENTRY
{
	std::string id;
	std::string label;
	std::string groupId;
	std::string groupLabel;
	std::string evidence;
	std::filesystem::path modelRelativePath;
	std::filesystem::path resolvedModelPath;
	std::wstring prototypeTag;
	float3_t defaultScale = float3_t(1.f, 1.f, 1.f);
	MAP_ASSET_ANCHOR anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
};

class CMapAssetCatalog final
{
public:
	bool_t Load_Default();
	bool_t Load(const std::filesystem::path& path,
		const std::string& expectedAreaId = {});

	const MAP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const std::vector<MAP_ASSET_ENTRY>& Get_Entries() const { return m_Entries; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	const std::filesystem::path& Get_CatalogPath() const { return m_CatalogPath; }
	const std::filesystem::path& Get_PlacementPath() const { return m_PlacementPath; }
	bool_t Is_Ready() const { return m_bReady; }

	static std::filesystem::path Get_MapDataRoot();
	static std::filesystem::path Get_AreaSelectionPath();

private:
	std::vector<MAP_ASSET_ENTRY> m_Entries;
	std::string m_AreaId;
	std::string m_Status = "Catalog not loaded";
	std::filesystem::path m_CatalogPath;
	std::filesystem::path m_PlacementPath;
	bool_t m_bReady = false;
};

NS_END
