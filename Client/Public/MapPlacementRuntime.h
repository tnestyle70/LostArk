#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"
#include "MapPlacementDocument.h"
#include "MapStaticBatchObject.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Engine)

class CModel;

NS_END

NS_BEGIN(Client)

class CMapAssetObject;

struct MAP_RUNTIME_PLACED_ENTRY
{
	MAP_PLACEMENT_RECORD record;
	std::wstring layerTag;
	shared_ptr<CMapAssetObject> object;
	shared_ptr<CMapStaticBatchObject> batch;
};

struct MAP_RUNTIME_STATIC_BATCH_ENTRY
{
	std::string assetId;
	bool_t mirrored = false;
	shared_ptr<CMapStaticBatchObject> object;
};

class CMapPlacementRuntime final
{
public:
	CMapPlacementRuntime() = default;
	~CMapPlacementRuntime();

	CMapPlacementRuntime(const CMapPlacementRuntime&) = delete;
	CMapPlacementRuntime& operator=(const CMapPlacementRuntime&) = delete;

	bool_t Load_Area(
		uint32_t levelIndex,
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope = {});
	void Clear();

	const CMapAssetCatalog& Get_Catalog() const { return m_Catalog; }
	const std::string& Get_Status() const { return m_Status; }
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_Placements() const
	{
		return m_Placements;
	}
	bool_t Try_Get_PlacementBounds(
		float3_t& outMinimum,
		float3_t& outMaximum) const;

	static bool_t Read_Placements(
		const CMapAssetCatalog& catalog,
		std::vector<MAP_PLACEMENT_RECORD>& outRecords,
		std::string& outStatus);
	static void Apply_LoadScope(
		const CMapAssetCatalog& catalog,
		const MAP_LOAD_SCOPE& loadScope,
		std::vector<MAP_PLACEMENT_RECORD>& records);
	static void Cache_LoadStage(
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope,
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_PLACEMENT_RECORD>& records);
	static bool_t Try_GetCachedLoadStage(
		const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope,
		CMapAssetCatalog& outCatalog,
		std::vector<MAP_PLACEMENT_RECORD>& outRecords);

	static bool_t Create_Placement(
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		const MAP_PLACEMENT_RECORD& record,
		MAP_RUNTIME_PLACED_ENTRY& outEntry,
		const MAP_FRUSTUM_CULLING_POLICY& frustumCulling = {});

	static bool_t Stage_PlacementRuntime(
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_PLACEMENT_RECORD>& records,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& outPlacements,
		std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& outBatches,
		const MAP_FRUSTUM_CULLING_POLICY& frustumCulling = {});

	static void Remove_PlacementRuntime(
		uint32_t levelIndex,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& batches);

	static bool_t Set_RuntimeVisible(
		MAP_RUNTIME_PLACED_ENTRY& entry,
		bool_t visible);
#ifdef _DEBUG
	/* Debug presentation may address an authored occurrence group by its stable
	sourceLevel. It never exposes vector order or prototype identity to callers. */
	bool_t Set_DebugSourceLevelVisible(
		const std::string& sourceLevel,
		bool_t visible,
		size_t expectedPlacementCount);
	bool_t Restore_DebugSourceLevelVisibility(
		const std::string& sourceLevel,
		size_t expectedPlacementCount);
#endif

	static bool_t Is_BatchEligible(const MAP_ASSET_ENTRY& asset);

	static HRESULT Build_StaticInstance(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<Engine::CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		FMapStaticInstance& outInstance);

	static std::wstring Make_LayerTag(const std::string& sourceLevel);

private:
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	CMapAssetCatalog m_Catalog;
	std::vector<MAP_RUNTIME_PLACED_ENTRY> m_Placements;
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY> m_StaticBatches;
	std::string m_Status = "Map runtime not loaded";
};

NS_END
