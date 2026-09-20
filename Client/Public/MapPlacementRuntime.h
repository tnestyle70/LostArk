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

#ifdef _DEBUG
struct MAP_DEBUG_PLACEMENT_PREVIEW
{
	MAP_PLACEMENT_RECORD record;
	std::vector<Engine::MODEL_MATERIAL_OVERRIDE> materialOverrides;
};
#endif

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
#ifdef _DEBUG
	void Rebase_AuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records);
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& Get_AuthoringBatches()
	{
		return m_StaticBatches;
	}
#endif
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
		const MAP_FRUSTUM_CULLING_POLICY& frustumCulling = {},
		const std::vector<Engine::MODEL_MATERIAL_OVERRIDE>* materialOverrides = nullptr);

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
	/* Cinematic stage overlay. It never changes the logical visibility above, so
	   clearing it (suppressed == false) restores exactly the state gameplay set. */
	static bool_t Set_RuntimeSuppressed(
		MAP_RUNTIME_PLACED_ENTRY& entry,
		bool_t suppressed);
#ifdef _DEBUG
	/* Stages ordinary CMapAssetObjects and commits only after both material and
	visibility operations succeed. The original placement order stays stable. */
	bool_t Replace_DebugPlacementPreview(
		const std::vector<MAP_DEBUG_PLACEMENT_PREVIEW>& previews,
		const std::vector<std::string>& hiddenSourcePlacementIds,
		std::string& outStatus);
	bool_t Clear_DebugPlacementPreview(std::string& outStatus);
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

	/* World matrix of one placement -- S*R*T with the BOTTOM_CENTER anchor
	   offset -- shared by the batch instance, the viewport pick and the
	   selection outline so no consumer can drift from what is drawn. The
	   model must carry local bounds. */
	static bool_t Compute_PlacementWorld(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<Engine::CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		float4x4_t& outWorld);
	/* World-space AABB of the rotated local bounds. */
	static bool_t Try_Get_PlacementWorldBounds(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<Engine::CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		float3_t& outMinimum,
		float3_t& outMaximum);
	/* One model clone per asset for bounds queries, kept by the caller for
	   as long as its level lives. A failed clone is remembered as null. */
	static shared_ptr<Engine::CModel> Find_PlacementModel(
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		const std::string& assetId,
		std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache);
	/* Resolves a picked world point to the visible, non-backdrop placement
	   whose rotated local bounds contain it, choosing the smallest volume
	   among nested boxes; outContainingCount reports how many did. With no
	   containing box the nearest bounding sphere wins and the count is 0. */
	static bool_t Try_Resolve_PickedPlacement(
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
		const float3_t& worldPoint,
		uint64_t& outPlacementId,
		size_t& outContainingCount);

	enum class PLACEMENT_TRANSFORM_RESULT
	{
		APPLIED,
		/* The staged record failed validation; nothing changed. */
		REJECTED,
		/* The runtime could not take the pose; the entry keeps its state. */
		FAILED,
	};
	/* Writes the staged pose onto the live entry: a standalone object takes
	   it directly, a batched instance is rebuilt in place, and a mirror parity
	   flip migrates the entry to a standalone object because the batch pass
	   changes. On APPLIED entry.record becomes the staged record. */
	static PLACEMENT_TRANSFORM_RESULT Apply_PlacementTransform(
		uint32_t levelIndex,
		const CMapAssetCatalog& catalog,
		std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
		MAP_RUNTIME_PLACED_ENTRY& entry,
		const MAP_PLACEMENT_RECORD& staged,
		std::string& outStatus);
#ifdef _DEBUG
	/* True while a Debug preview (Character Select floor swap) adds and hides
	   placements; editing tools stay read-only until it is cleared. */
	bool_t Has_DebugPlacementPreview() const { return !m_DebugPreviewIds.empty(); }
#endif

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
	MAP_FRUSTUM_CULLING_POLICY m_FrustumCulling{};
#ifdef _DEBUG
	struct DEBUG_HIDDEN_PLACEMENT
	{
		uint64_t placementId = 0u;
		std::string sourcePlacementId;
		bool_t recordVisible = false;
		bool_t runtimeVisible = false;
	};
	std::vector<uint64_t> m_DebugPreviewIds;
	std::vector<DEBUG_HIDDEN_PLACEMENT> m_DebugHiddenPlacements;
#endif
};

NS_END
