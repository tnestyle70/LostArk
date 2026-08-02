#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

enum class MAP_ASSET_ANCHOR
{
	ORIGIN,
	BOTTOM_CENTER,
};

enum class MAP_ASSET_RENDER_MODE
{
	DEFERRED,
	TRANSLUCENT,
	BACKGROUND,
	ADDITIVE,
};

enum class MAP_ASSET_CULL_MODE
{
	CULL_BACK,
	CULL_FRONT,
	TWO_SIDED,
};

struct MAP_ASSET_RENDER_PROFILE
{
	MAP_ASSET_RENDER_MODE renderMode = MAP_ASSET_RENDER_MODE::DEFERRED;
	MAP_ASSET_CULL_MODE cullMode = MAP_ASSET_CULL_MODE::CULL_BACK;
	float2_t uvScale = float2_t(1.f, 1.f);
	float2_t uvSpeed = float2_t(0.f, 0.f);
	float opacity = 1.f;
	float opacityPower = 1.f;
	float emissiveIntensity = 1.f;
	float specularIntensity = 1.f;
	float specularPower = 50.f;
	float4_t colorTint = float4_t(1.f, 1.f, 1.f, 1.f);
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
	MAP_ASSET_RENDER_PROFILE renderProfile;
};

struct MAP_ASSET_SHARD
{
	std::string shardId;
	std::filesystem::path catalogPath;
	std::filesystem::path placementPath;
	uint32_t assetCount = {};
	uint32_t placementCount = {};
};

class CMapAssetCatalog final
{
public:
	bool_t Load_Default();
	bool_t Load_Area(const std::string& areaId);
	bool_t Load(const std::filesystem::path& path,
		const std::string& expectedAreaId = {});

	const MAP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const std::vector<MAP_ASSET_ENTRY>& Get_Entries() const { return m_Entries; }
	const std::vector<MAP_ASSET_SHARD>& Get_Shards() const { return m_Shards; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	const std::filesystem::path& Get_CatalogPath() const { return m_CatalogPath; }
	const std::filesystem::path& Get_PlacementPath() const { return m_PlacementPath; }
	bool_t Is_Sharded() const { return m_bSharded; }
	bool_t Is_Ready() const { return m_bReady; }

	static std::filesystem::path Get_MapDataRoot();
	static std::filesystem::path Get_AreaSelectionPath();

private:
	std::vector<MAP_ASSET_ENTRY> m_Entries;
	std::unordered_map<std::string, size_t> m_EntryLookup;
	std::vector<MAP_ASSET_SHARD> m_Shards;
	std::string m_AreaId;
	std::string m_Status = "Catalog not loaded";
	std::filesystem::path m_CatalogPath;
	std::filesystem::path m_PlacementPath;
	bool_t m_bSharded = false;
	bool_t m_bReady = false;
};

NS_END
