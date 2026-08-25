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
	/* A translucent surface that also refracts what is behind it. The source
	   BlendMode alone cannot say this -- an ordinary translucent decal shares
	   it -- so an asset only becomes WATER when the published water
	   presentation document carries a row for it. */
	WATER,
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

/* One row of <AreaId>.mapwater.json. Every value is the source
   MaterialInstanceConstant parameter of the same name, folded down the parent
   chain by the publisher, so nothing here is tuned in code. An asset whose
   catalog render mode is WATER must have exactly one of these or the Area
   fails to load: a water pass with identity parameters would draw an opaque
   plate and look like a bug rather than a missing document. */
struct MAP_ASSET_WATER_PROFILE
{
	std::string materialName;
	/* Resources-relative IDs. Declared by the document and validated by the
	   publisher; the runtime does not bind them yet, so they stay empty of
	   meaning for drawing until the auxiliary texture path is opened. */
	std::string detailNormalTexture;
	std::string reflectionTexture;
	std::string foamTexture;
	float opacity = 1.f;
	float opacityPower = 1.f;
	float fresnelIntensity = 0.f;
	float fresnelPower = 1.f;
	float screenDistortionIntensity = 0.f;
	float normalIntensity = 0.f;
	float detailNormalIntensity = 0.f;
	float normalDistortionIntensity = 0.f;
	float reflectionIntensity = 0.f;
	float reflectionUv = 1.f;
	float depthBias = 0.f;
	float diffuseTiling = 1.f;
	float4_t diffuseColor = float4_t(1.f, 1.f, 1.f, 1.f);
	float4_t reflectionColor = float4_t(1.f, 1.f, 1.f, 1.f);
	float4_t normalTilingPanning = float4_t(1.f, 1.f, 0.f, 0.f);
	float4_t detailNormalTilingPanning = float4_t(1.f, 1.f, 0.f, 0.f);
	float4_t reflectionTilingPanning = float4_t(1.f, 1.f, 0.f, 0.f);
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
	bool_t Load_Source(
		const std::filesystem::path& catalogPath,
		const std::filesystem::path& placementPath,
		const std::string& expectedAreaId);
	bool_t Load(const std::filesystem::path& path,
		const std::string& expectedAreaId = {});

	const MAP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const MAP_ASSET_WATER_PROFILE* Find_Water(const std::string& assetId) const;
	const std::vector<MAP_ASSET_ENTRY>& Get_Entries() const { return m_Entries; }
	const std::vector<MAP_ASSET_SHARD>& Get_Shards() const { return m_Shards; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	const std::filesystem::path& Get_CatalogPath() const { return m_CatalogPath; }
	const std::filesystem::path& Get_PlacementPath() const { return m_PlacementPath; }
	bool_t Is_Sharded() const { return m_bSharded; }
	bool_t Is_Ready() const { return m_bReady; }

	static std::filesystem::path Get_MapDataRoot();
	static std::filesystem::path Get_MapAuthoringRoot();
	static std::filesystem::path Get_AuthoringPlacementPath(
		const std::string& areaId);
	static std::filesystem::path Get_AreaSelectionPath();

private:
	/* Reads <AreaId>.mapwater.json beside the runtime catalog when it exists,
	   then requires that the WATER render modes and the water rows agree in
	   both directions. Called at the end of a successful Area load. */
	bool_t Load_WaterPresentation(const std::string& areaId);

private:
	std::vector<MAP_ASSET_ENTRY> m_Entries;
	std::unordered_map<std::string, size_t> m_EntryLookup;
	std::unordered_map<std::string, MAP_ASSET_WATER_PROFILE> m_WaterProfiles;
	std::vector<MAP_ASSET_SHARD> m_Shards;
	std::string m_AreaId;
	std::string m_Status = "Catalog not loaded";
	std::filesystem::path m_CatalogPath;
	std::filesystem::path m_PlacementPath;
	bool_t m_bSharded = false;
	bool_t m_bReady = false;
	std::filesystem::path m_SourceCatalogOverride;
	std::filesystem::path m_SourcePlacementOverride;
};

NS_END
