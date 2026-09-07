#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"
#include "MapPlacementDocument.h"
#include "MapStaticBatchObject.h"

#include <memory>
#include <string>
#include <unordered_map>
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

/* One EFActorMotion oscillator lifted from the original stage. The stage
   authors gave these no start and no end: a placement carrying one rocks
   on its axis for as long as the map is loaded, which is why they are not
   sequences and need no trigger. Every placement starts at its own phase,
   so a row of identical cards reads as a wave rather than one motion. */
enum class MAP_SELF_MOTION_KIND : uint8_t
{
	ROTATION_CYCLIC,
	ROTATION_ACYCLIC,
	LOCATION_CYCLIC,
	LOCATION_ACYCLIC,
};

enum class MAP_SELF_MOTION_AXIS : uint8_t
{
	X,
	Y,
	Z,
};

struct MAP_SELF_MOTION
{
	uint64_t placementId = {};
	MAP_SELF_MOTION_KIND kind = MAP_SELF_MOTION_KIND::ROTATION_CYCLIC;
	MAP_SELF_MOTION_AXIS axis = MAP_SELF_MOTION_AXIS::X;
	/* Seconds for one full swing. Zero means the authored row carried no
	   cycle, which only happens for the acyclic kinds. */
	f32_t cycleSeconds = 0.f;
	/* Degrees for a rotation, metres for a translation. */
	f32_t range = 0.f;
	f32_t startPhaseSeconds = 0.f;
};

struct MAP_RUNTIME_SELF_MOTION_ENTRY
{
	MAP_SELF_MOTION motion;
	/* The placed pose the oscillation is measured from, captured once so a
	   replay never accumulates onto an already-offset transform. */
	float3_t basePosition = {};
	float4_t baseRotation = float4_t(0.f, 0.f, 0.f, 1.f);
	size_t placementIndex = 0u;
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
	/* Presentation drivers such as an authored world sequence write the sampled
	   pose back onto the live entries. The set of placements itself stays owned
	   here; only their transforms and visibility are written. */
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_MutablePlacements()
	{
		return m_Placements;
	}
	/* Reads <AreaId>.mapmotions.json and binds each row to a live placement.
	   A missing document is not an error: most areas author none. */
	bool_t Load_SelfMotions(const std::string& areaId);
	/* Same work against a caller-owned placement vector, so the Map Editor
	   -- which keeps its own entries rather than a runtime -- shows the
	   idle motion its authors are checking. */
	static bool_t Read_SelfMotions(
		const std::string& areaId,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		std::vector<MAP_RUNTIME_SELF_MOTION_ENTRY>& outMotions);
	static void Sample_SelfMotions(
		const std::vector<MAP_RUNTIME_SELF_MOTION_ENTRY>& motions,
		f32_t elapsedSeconds,
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements);
	/* Wraps far enough out that float precision stays fine, on a multiple of
	   both authored cycles so nothing jumps at the seam. */
	static constexpr f32_t SELF_MOTION_WRAP_SECONDS = 840.f;
	/* Advances every bound oscillator and writes the sampled pose onto its
	   placement. Sequence playback writes the same transforms, so a sequence
	   driving a placement wins for as long as it runs. */
	void Update_SelfMotions(f32_t fTimeDelta);
	size_t Get_SelfMotionCount() const { return m_SelfMotions.size(); }

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
	static bool_t Try_GetRuntimeVisible(
		const MAP_RUNTIME_PLACED_ENTRY& entry,
		bool_t& outVisible);
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
	std::vector<MAP_RUNTIME_SELF_MOTION_ENTRY> m_SelfMotions;
	f32_t m_fSelfMotionElapsedSeconds = 0.f;
	/* Batched placements need a model to rebuild their instance from; the
	   clone is kept per asset so an oscillating batch does not clone one
	   every frame. */
	std::unordered_map<std::string, shared_ptr<Engine::CModel>> m_SelfMotionModels;
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY> m_StaticBatches;
	std::string m_Status = "Map runtime not loaded";
};

NS_END
