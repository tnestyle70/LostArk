#include "MapPlacementRuntime.h"

#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr const wchar_t* MAP_ASSET_PROTOTYPE =
		TEXT("Prototype_GameObject_MapAsset");
	constexpr const wchar_t* MAP_BATCH_PROTOTYPE =
		TEXT("Prototype_GameObject_MapStaticBatch");
	constexpr const wchar_t* MAP_BATCH_LAYER =
		TEXT("Layer_MapStaticBatch");

	struct MAP_LOAD_STAGE final
	{
		MAP_LOAD_SCOPE loadScope;
		CMapAssetCatalog catalog;
		std::vector<MAP_PLACEMENT_RECORD> records;
	};
	std::mutex g_MapLoadStageMutex;
	std::unordered_map<std::string, MAP_LOAD_STAGE> g_MapLoadStages;

	bool_t EqualScope(
		const MAP_LOAD_SCOPE& left,
		const MAP_LOAD_SCOPE& right)
	{
		return left.isEnabled == right.isEnabled &&
			left.includeBackground == right.includeBackground &&
			left.minimumX == right.minimumX &&
			left.minimumZ == right.minimumZ &&
			left.maximumX == right.maximumX &&
			left.maximumZ == right.maximumZ &&
			left.excludedAssetGroupId == right.excludedAssetGroupId &&
			left.frustumCulling.bypass == right.frustumCulling.bypass &&
			left.frustumCulling.diagnostics == right.frustumCulling.diagnostics &&
			left.frustumCulling.baseMargin == right.frustumCulling.baseMargin &&
			left.frustumCulling.largeObjectRadiusThreshold ==
				right.frustumCulling.largeObjectRadiusThreshold &&
			left.frustumCulling.largeObjectAbsoluteMargin ==
				right.frustumCulling.largeObjectAbsoluteMargin &&
			left.frustumCulling.largeObjectRelativeMargin ==
				right.frustumCulling.largeObjectRelativeMargin &&
			left.frustumCulling.rejectHysteresisFrames ==
				right.frustumCulling.rejectHysteresisFrames;
	}

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}
}

CMapPlacementRuntime::~CMapPlacementRuntime()
{
	Clear();
}

bool_t CMapPlacementRuntime::Load_Area(
	uint32_t levelIndex,
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope)
{
	if (levelIndex >= ETOUI(LEVEL::END))
	{
		m_Status = "Target map level index is invalid";
		return false;
	}

	CMapAssetCatalog stagedCatalog;
	std::vector<MAP_PLACEMENT_RECORD> document;
	const bool_t usedLoaderStage = Try_GetCachedLoadStage(
		areaId, loadScope, stagedCatalog, document);
	if (!usedLoaderStage)
	{
		if (!stagedCatalog.Load_Area(areaId))
		{
			m_Status = stagedCatalog.Get_Status();
			return false;
		}

		if (!Read_Placements(stagedCatalog, document, m_Status))
			return false;
		Apply_LoadScope(stagedCatalog, loadScope, document);
	}
	if (document.empty())
	{
		m_Status = "Fixed map area has no placement records: " + areaId;
		return false;
	}

	std::vector<MAP_RUNTIME_PLACED_ENTRY> stagedPlacements;
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY> stagedBatches;
	CMapAssetRenderUtils::Begin_FrustumDiagnostics(
		areaId, loadScope.frustumCulling);
	if (!Stage_PlacementRuntime(
		levelIndex,
		stagedCatalog,
		document,
		stagedPlacements,
		stagedBatches,
		loadScope.frustumCulling))
	{
		Remove_PlacementRuntime(
			levelIndex, stagedPlacements, stagedBatches);
		m_Status = "Map runtime staging rolled back";
		return false;
	}

	Clear();
	m_iLevelIndex = levelIndex;
	m_FrustumCulling = loadScope.frustumCulling;
	m_Catalog = std::move(stagedCatalog);
	m_Placements = std::move(stagedPlacements);
	m_StaticBatches = std::move(stagedBatches);

	const size_t fallbackCount = static_cast<size_t>(std::count_if(
		m_Placements.begin(), m_Placements.end(),
		[](const MAP_RUNTIME_PLACED_ENTRY& entry)
		{
			return nullptr != entry.object;
		}));
	m_Status = std::string(usedLoaderStage ? "Committed staged " : "Loaded ") +
		std::to_string(m_Placements.size()) +
		" placements / " + std::to_string(m_StaticBatches.size()) +
		" batches / " + std::to_string(fallbackCount) + " fallbacks";
	return true;
}

void CMapPlacementRuntime::Apply_LoadScope(
	const CMapAssetCatalog& catalog,
	const MAP_LOAD_SCOPE& loadScope,
	std::vector<MAP_PLACEMENT_RECORD>& records)
{
	if (!loadScope.isEnabled)
		return;

	records.erase(
		std::remove_if(
			records.begin(),
			records.end(),
			[&catalog, &loadScope](const MAP_PLACEMENT_RECORD& record)
			{
				const MAP_ASSET_ENTRY* pAsset =
					catalog.Find(record.assetId);
				if (nullptr != pAsset &&
					!loadScope.excludedAssetGroupId.empty() &&
					pAsset->groupId == loadScope.excludedAssetGroupId)
				{
					return true;
				}
				if (loadScope.Contains(record.position))
					return false;

				return nullptr == pAsset || !loadScope.includeBackground ||
					MAP_ASSET_RENDER_MODE::BACKGROUND !=
						pAsset->renderProfile.renderMode;
			}),
			records.end());
}

void CMapPlacementRuntime::Cache_LoadStage(
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope,
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_PLACEMENT_RECORD>& records)
{
	if (areaId.empty() || !loadScope.isEnabled || records.empty())
		return;

	std::scoped_lock lock{ g_MapLoadStageMutex };
	g_MapLoadStages.insert_or_assign(
		areaId,
		MAP_LOAD_STAGE{ loadScope, catalog, records });
}

bool_t CMapPlacementRuntime::Try_GetCachedLoadStage(
	const std::string& areaId,
	const MAP_LOAD_SCOPE& loadScope,
	CMapAssetCatalog& outCatalog,
	std::vector<MAP_PLACEMENT_RECORD>& outRecords)
{
	if (!loadScope.isEnabled)
		return false;

	std::scoped_lock lock{ g_MapLoadStageMutex };
	const auto iter = g_MapLoadStages.find(areaId);
	if (iter == g_MapLoadStages.end() ||
		!EqualScope(iter->second.loadScope, loadScope))
	{
		return false;
	}

	outCatalog = iter->second.catalog;
	outRecords = iter->second.records;
	return true;
}

void CMapPlacementRuntime::Clear()
{
	if (m_iLevelIndex < ETOUI(LEVEL::END))
	{
		Remove_PlacementRuntime(
			m_iLevelIndex, m_Placements, m_StaticBatches);
	}
	else
	{
		m_Placements.clear();
		m_StaticBatches.clear();
	}

	m_iLevelIndex = ETOUI(LEVEL::END);
#ifdef _DEBUG
	m_DebugPreviewIds.clear();
	m_DebugHiddenPlacements.clear();
#endif
}

bool_t CMapPlacementRuntime::Try_Get_PlacementBounds(
	float3_t& outMinimum,
	float3_t& outMaximum) const
{
	bool_t hasBounds = false;
	float3_t minimum{};
	float3_t maximum{};
	for (const MAP_RUNTIME_PLACED_ENTRY& entry : m_Placements)
	{
		const MAP_ASSET_ENTRY* asset =
			m_Catalog.Find(entry.record.assetId);
		if (!entry.record.visible || nullptr == asset ||
			MAP_ASSET_RENDER_MODE::BACKGROUND ==
				asset->renderProfile.renderMode ||
			!IsFinite(entry.record.position))
		{
			continue;
		}

		if (!hasBounds)
		{
			minimum = entry.record.position;
			maximum = entry.record.position;
			hasBounds = true;
			continue;
		}

		minimum.x = (std::min)(minimum.x, entry.record.position.x);
		minimum.y = (std::min)(minimum.y, entry.record.position.y);
		minimum.z = (std::min)(minimum.z, entry.record.position.z);
		maximum.x = (std::max)(maximum.x, entry.record.position.x);
		maximum.y = (std::max)(maximum.y, entry.record.position.y);
		maximum.z = (std::max)(maximum.z, entry.record.position.z);
	}

	if (!hasBounds)
		return false;

	outMinimum = minimum;
	outMaximum = maximum;
	return true;
}

bool_t CMapPlacementRuntime::Read_Placements(
	const CMapAssetCatalog& catalog,
	std::vector<MAP_PLACEMENT_RECORD>& outRecords,
	std::string& outStatus)
{
	if (!catalog.Is_Ready())
	{
		outStatus = "Map catalog is not ready";
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> stagedRecords;
	if (!catalog.Is_Sharded())
	{
		if (!CMapPlacementDocument::Read(
			catalog.Get_PlacementPath(),
			catalog,
			stagedRecords,
			outStatus))
		{
			return false;
		}
	}
	else
	{
		size_t declaredTotal = {};
		for (const MAP_ASSET_SHARD& shard : catalog.Get_Shards())
		{
			if (declaredTotal > (std::numeric_limits<size_t>::max)() -
				shard.placementCount)
			{
				outStatus = "Shard placement count overflow";
				return false;
			}
			declaredTotal += shard.placementCount;
		}

		stagedRecords.reserve(declaredTotal);
		std::unordered_set<uint64_t> runtimeIds;
		std::unordered_set<std::string> sourceIds;
		runtimeIds.reserve(declaredTotal);
		sourceIds.reserve(declaredTotal);

		for (const MAP_ASSET_SHARD& shard : catalog.Get_Shards())
		{
			std::error_code placementError;
			if (!std::filesystem::is_regular_file(
				shard.placementPath, placementError))
			{
				outStatus = "Shard placement file is missing: " +
					shard.shardId;
				return false;
			}

			std::vector<MAP_PLACEMENT_RECORD> shardDocument;
			if (!CMapPlacementDocument::Read(
				shard.placementPath,
				catalog,
				shardDocument,
				outStatus))
			{
				outStatus = "Shard " + shard.shardId + " failed: " +
					outStatus;
				return false;
			}
			if (shardDocument.size() != shard.placementCount)
			{
				outStatus = "Shard placement count mismatch: " +
					shard.shardId;
				return false;
			}

			for (MAP_PLACEMENT_RECORD& record : shardDocument)
			{
				if (!runtimeIds.insert(record.placementId).second ||
					!sourceIds.insert(record.sourcePlacementId).second)
				{
					outStatus = "Duplicate placement ID across shards: " +
						shard.shardId;
					return false;
				}
				stagedRecords.push_back(std::move(record));
			}
		}
	}

	outRecords = std::move(stagedRecords);
	outStatus = "Placement document ready";
	return true;
}

bool_t CMapPlacementRuntime::Create_Placement(
	uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	const MAP_PLACEMENT_RECORD& record,
	MAP_RUNTIME_PLACED_ENTRY& outEntry,
	const MAP_FRUSTUM_CULLING_POLICY& frustumCulling,
	const std::vector<Engine::MODEL_MATERIAL_OVERRIDE>* materialOverrides)
{
	const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
	if (nullptr == asset ||
		!CMapPlacementDocument::Is_Valid(record, catalog))
	{
		return false;
	}

	const std::wstring layerTag = Make_LayerTag(record.sourceLevel);
	CMapAssetObject::MAP_ASSET_DESC desc{};
	desc.prototypeLevelIndex = levelIndex;
	desc.placementId = record.placementId;
	desc.assetId = asset->id;
	desc.assetGroupId = asset->groupId;
	desc.modelPrototypeTag = asset->prototypeTag;
	desc.position = record.position;
	desc.rotationQuaternion = record.rotationQuaternion;
	desc.signedScale = record.signedScale;
	desc.applyBottomCenter =
		MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset->anchor;
	desc.visible = record.visible;
	desc.renderProfile = asset->renderProfile;
	desc.bakedLighting = record.bakedLighting;
	if (nullptr != materialOverrides)
	{
		Engine::MODEL_ASSET_LOAD_DESC load;
		load.assetRoot = CRuntimeAssetRoot::Get();
		load.meshPath = asset->resolvedModelPath;
		load.materialOverrides = *materialOverrides;
		desc.materialVariant = std::move(load);
	}
	desc.frustumCulling = frustumCulling;
	if (const MAP_ASSET_WATER_PROFILE* water = catalog.Find_Water(asset->id))
	{
		desc.hasWaterProfile = true;
		desc.waterProfile = *water;
	}

	/* Allocate the record strings before attaching an object to the live layer. */
	MAP_RUNTIME_PLACED_ENTRY stagedEntry;
	stagedEntry.record = record;
	stagedEntry.layerTag = layerTag;
	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		levelIndex,
		MAP_ASSET_PROTOTYPE,
		levelIndex,
		layerTag,
		&desc,
		&gameObject)))
	{
		return false;
	}

	shared_ptr<CMapAssetObject> mapObject =
		dynamic_pointer_cast<CMapAssetObject>(gameObject);
	if (nullptr == mapObject)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			levelIndex, layerTag, gameObject);
		return false;
	}

	stagedEntry.object = std::move(mapObject);
	outEntry = std::move(stagedEntry);
	return true;
}

bool_t CMapPlacementRuntime::Stage_PlacementRuntime(
	uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_PLACEMENT_RECORD>& records,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& outPlacements,
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& outBatches,
	const MAP_FRUSTUM_CULLING_POLICY& frustumCulling)
{
	using BATCH_KEY = std::pair<std::string, bool_t>;
	std::map<BATCH_KEY, std::vector<const MAP_PLACEMENT_RECORD*>> groups;

	for (const MAP_PLACEMENT_RECORD& record : records)
	{
		const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
		if (nullptr == asset)
			return false;
		if (!Is_BatchEligible(*asset))
			continue;

		const bool_t mirrored = record.signedScale.x *
			record.signedScale.y * record.signedScale.z < 0.f;
		groups[{ record.assetId, mirrored }].push_back(&record);
	}

	std::unordered_map<uint64_t, shared_ptr<CMapStaticBatchObject>>
		batchByPlacement;
	batchByPlacement.reserve(records.size());

	for (const auto& [key, placements] : groups)
	{
		const MAP_ASSET_ENTRY* asset = catalog.Find(key.first);
		if (nullptr == asset)
			return false;

		shared_ptr<Engine::CModel> model =
			dynamic_pointer_cast<Engine::CModel>(
				CGameInstance::Get().Clone_Prototype(
					levelIndex, asset->prototypeTag));
		if (nullptr == model)
			return false;

		if (!model->Has_LocalBounds())
			continue;

		CMapStaticBatchObject::DESC desc{};
		desc.PrototypeLevelIndex = levelIndex;
		desc.AssetId = asset->id;
		desc.AssetGroupId = asset->groupId;
		desc.ModelPrototypeTag = asset->prototypeTag;
		desc.RenderProfile = asset->renderProfile;
		desc.FrustumCulling = frustumCulling;
		desc.Mirrored = key.second;
		desc.Instances.reserve(placements.size());

		bool_t batchIsValid = true;
		for (const MAP_PLACEMENT_RECORD* record : placements)
		{
			FMapStaticInstance instance{};
			if (nullptr == record || FAILED(Build_StaticInstance(
				*asset, model, *record, instance)))
			{
				batchIsValid = false;
				break;
			}
			desc.Instances.push_back(instance);
		}
		if (!batchIsValid)
			continue;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			levelIndex,
			MAP_BATCH_PROTOTYPE,
			levelIndex,
			MAP_BATCH_LAYER,
			&desc,
			&gameObject)))
		{
			return false;
		}

		shared_ptr<CMapStaticBatchObject> batch =
			dynamic_pointer_cast<CMapStaticBatchObject>(gameObject);
		if (nullptr == batch)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				levelIndex, MAP_BATCH_LAYER, gameObject);
			return false;
		}

		outBatches.push_back({ asset->id, key.second, batch });
		for (const MAP_PLACEMENT_RECORD* record : placements)
		{
			const auto [iter, inserted] = batchByPlacement.emplace(
				record->placementId, batch);
			UNREFERENCED_PARAMETER(iter);
			if (!inserted)
				return false;
		}
	}

	outPlacements.reserve(records.size());
	for (const MAP_PLACEMENT_RECORD& record : records)
	{
		const auto batch = batchByPlacement.find(record.placementId);
		if (batch != batchByPlacement.end())
		{
			MAP_RUNTIME_PLACED_ENTRY entry{};
			entry.record = record;
			entry.layerTag = MAP_BATCH_LAYER;
			entry.batch = batch->second;
			outPlacements.push_back(std::move(entry));
			continue;
		}

		MAP_RUNTIME_PLACED_ENTRY fallback{};
		if (!Create_Placement(
			levelIndex, catalog, record, fallback, frustumCulling))
			return false;
		outPlacements.push_back(std::move(fallback));
	}

	return outPlacements.size() == records.size();
}

void CMapPlacementRuntime::Remove_PlacementRuntime(
	uint32_t levelIndex,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& batches)
{
	for (MAP_RUNTIME_PLACED_ENTRY& entry : placements)
	{
		if (nullptr != entry.object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				levelIndex,
				entry.layerTag,
				static_pointer_cast<CGameObject>(entry.object));
		}
	}

	for (MAP_RUNTIME_STATIC_BATCH_ENTRY& entry : batches)
	{
		if (nullptr != entry.object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				levelIndex,
				MAP_BATCH_LAYER,
				static_pointer_cast<CGameObject>(entry.object));
		}
	}

	placements.clear();
	batches.clear();
}

bool_t CMapPlacementRuntime::Set_RuntimeVisible(
	MAP_RUNTIME_PLACED_ENTRY& entry,
	bool_t visible)
{
	if (nullptr != entry.object)
	{
		entry.object->Set_Visible(visible);
		return true;
	}
	if (nullptr != entry.batch)
	{
		return SUCCEEDED(entry.batch->Set_InstanceVisible(
			entry.record.placementId, visible));
	}
	return false;
}

bool_t CMapPlacementRuntime::Try_GetRuntimeVisible(
	const MAP_RUNTIME_PLACED_ENTRY& entry,
	bool_t& outVisible)
{
	if (nullptr != entry.object)
	{
		outVisible = entry.object->Is_Visible();
		return true;
	}
	if (nullptr != entry.batch)
	{
		return SUCCEEDED(entry.batch->Try_GetInstanceVisible(
			entry.record.placementId, outVisible));
	}
	return false;
}

bool_t CMapPlacementRuntime::Set_RuntimeSuppressed(
	MAP_RUNTIME_PLACED_ENTRY& entry,
	bool_t suppressed)
{
	if (nullptr != entry.object)
	{
		entry.object->Set_StageSuppressed(suppressed);
		return true;
	}
	if (nullptr != entry.batch)
	{
		return SUCCEEDED(entry.batch->Set_InstanceSuppressed(
			entry.record.placementId, suppressed));
	}
	return false;
}

#ifdef _DEBUG
bool_t CMapPlacementRuntime::Replace_DebugPlacementPreview(
	const std::vector<MAP_DEBUG_PLACEMENT_PREVIEW>& previews,
	const std::vector<std::string>& hiddenSourcePlacementIds,
	std::string& outStatus)
{
	if (m_iLevelIndex >= ETOUI(LEVEL::END) || previews.empty() ||
		previews.size() > 64u || hiddenSourcePlacementIds.empty())
	{
		outStatus = "Preview requires a loaded map, placements, and explicit hidden source IDs.";
		return false;
	}
	auto isPreview = [&](const uint64_t id) {
		return m_DebugPreviewIds.end() != std::find(m_DebugPreviewIds.begin(), m_DebugPreviewIds.end(), id);
	};
	if (!m_DebugHiddenPlacements.empty())
	{
		if (hiddenSourcePlacementIds.size() != m_DebugHiddenPlacements.size() ||
			!std::equal(hiddenSourcePlacementIds.begin(), hiddenSourcePlacementIds.end(),
				m_DebugHiddenPlacements.begin(), [](const auto& id, const auto& entry) {
					return id == entry.sourcePlacementId;
				}))
		{
			outStatus = "Restore the current preview before changing its hidden source IDs.";
			return false;
		}
	}
	std::vector<DEBUG_HIDDEN_PLACEMENT> hidden = m_DebugHiddenPlacements;
	std::vector<size_t> hiddenIndices;
	std::vector<bool_t> priorVisibility;
	std::unordered_set<std::string> hiddenIds;
	for (const std::string& sourceId : hiddenSourcePlacementIds)
	{
		if (sourceId.empty() || !hiddenIds.insert(sourceId).second)
		{
			outStatus = "Preview hidden source IDs must be nonempty and unique.";
			return false;
		}
		size_t index = m_Placements.size();
		for (size_t i = 0; i < m_Placements.size(); ++i)
		{
			if (!isPreview(m_Placements[i].record.placementId) &&
				m_Placements[i].record.sourcePlacementId == sourceId)
			{
				if (index != m_Placements.size())
				{
					outStatus = "Preview target source ID is ambiguous: " + sourceId;
					return false;
				}
				index = i;
			}
		}
		bool_t visible = false;
		if (index == m_Placements.size() || !Try_GetRuntimeVisible(m_Placements[index], visible))
		{
			outStatus = "Preview target is missing or has no visibility consumer: " + sourceId;
			return false;
		}
		hiddenIndices.push_back(index);
		priorVisibility.push_back(visible);
		if (m_DebugHiddenPlacements.empty())
			hidden.push_back({ m_Placements[index].record.placementId, sourceId,
				m_Placements[index].record.visible, visible });
	}
	std::unordered_set<uint64_t> usedIds;
	std::unordered_set<std::string> usedSourceIds;
	for (const auto& entry : m_Placements)
	{
		if (!isPreview(entry.record.placementId))
		{
			usedIds.insert(entry.record.placementId);
			usedSourceIds.insert(entry.record.sourcePlacementId);
		}
	}
	for (const auto& preview : previews)
	{
		if (!CMapPlacementDocument::Is_Valid(preview.record, m_Catalog) ||
			!usedIds.insert(preview.record.placementId).second ||
			!usedSourceIds.insert(preview.record.sourcePlacementId).second)
		{
			outStatus = "Preview placement is invalid or collides with an existing stable ID.";
			return false;
		}
	}

	std::string committedStatus = "Preview committed; source placements and their baked lighting remain unchanged.";
	std::vector<MAP_RUNTIME_PLACED_ENTRY> staged;
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY> noBatches;
	std::vector<MAP_RUNTIME_PLACED_ENTRY> committed;
	std::vector<MAP_RUNTIME_PLACED_ENTRY> previous;
	std::vector<uint64_t> ids;
	try
	{
		staged.reserve(previews.size());
		ids.reserve(previews.size());
		committed.reserve(m_Placements.size() + previews.size());
		previous.reserve(m_DebugPreviewIds.size());
		for (const auto& entry : m_Placements)
		{
			if (isPreview(entry.record.placementId)) previous.push_back(entry);
			else committed.push_back(entry);
		}
		for (const auto& preview : previews)
		{
			MAP_PLACEMENT_RECORD record = preview.record;
			record.visible = false;
			staged.emplace_back();
			if (!Create_Placement(m_iLevelIndex, m_Catalog, record, staged.back(),
				m_FrustumCulling, preview.materialOverrides.empty() ? nullptr : &preview.materialOverrides))
			{
				Remove_PlacementRuntime(m_iLevelIndex, staged, noBatches);
				outStatus = "Preview material/object creation failed; previous selection retained: " + record.assetId;
				return false;
			}
			staged.back().record.visible = preview.record.visible;
			ids.push_back(record.placementId);
			committed.push_back(staged.back());
		}
		for (auto& entry : committed)
			if (hiddenIds.count(entry.record.sourcePlacementId)) entry.record.visible = false;
	}
	catch (const std::exception& error)
	{
		Remove_PlacementRuntime(m_iLevelIndex, staged, noBatches);
		outStatus = std::string("Preview staging failed; previous selection retained: ") + error.what();
		return false;
	}

	auto rollbackVisibility = [&]() {
		for (size_t i = 0; i < hiddenIndices.size(); ++i)
			(void)Set_RuntimeVisible(m_Placements[hiddenIndices[i]], priorVisibility[i]);
		Remove_PlacementRuntime(m_iLevelIndex, staged, noBatches);
	};
	for (const size_t index : hiddenIndices)
	{
		if (!Set_RuntimeVisible(m_Placements[index], false))
		{
			rollbackVisibility();
			outStatus = "Preview visibility commit failed; previous selection retained.";
			return false;
		}
	}
	for (auto& entry : staged)
	{
		if (!Set_RuntimeVisible(entry, entry.record.visible))
		{
			rollbackVisibility();
			outStatus = "Preview activation failed; previous selection retained.";
			return false;
		}
	}
	/* A removed object may still be in this frame's render queue. */
	for (auto& entry : previous) (void)Set_RuntimeVisible(entry, false);
	Remove_PlacementRuntime(m_iLevelIndex, previous, noBatches);
	m_Placements.swap(committed);
	m_DebugPreviewIds.swap(ids);
	m_DebugHiddenPlacements.swap(hidden);
	outStatus.swap(committedStatus);
	return true;
}

bool_t CMapPlacementRuntime::Clear_DebugPlacementPreview(std::string& outStatus)
{
	if (m_DebugPreviewIds.empty())
	{
		outStatus = "Original central floor is active.";
		return true;
	}
	std::vector<size_t> indices;
	std::vector<bool_t> previousVisibility;
	for (const auto& hidden : m_DebugHiddenPlacements)
	{
		const auto found = std::find_if(m_Placements.begin(), m_Placements.end(),
			[&](const auto& entry) { return entry.record.placementId == hidden.placementId; });
		bool_t visible = false;
		if (found == m_Placements.end() || !Try_GetRuntimeVisible(*found, visible))
		{
			outStatus = "Cannot restore the missing original central placement; preview retained.";
			return false;
		}
		indices.push_back(static_cast<size_t>(found - m_Placements.begin()));
		previousVisibility.push_back(visible);
	}
	std::string restoredStatus = "Original central floor and star restored.";
	for (size_t i = 0; i < indices.size(); ++i)
	{
		if (!Set_RuntimeVisible(m_Placements[indices[i]], m_DebugHiddenPlacements[i].runtimeVisible))
		{
			for (size_t j = 0; j < indices.size(); ++j)
				(void)Set_RuntimeVisible(m_Placements[indices[j]], previousVisibility[j]);
			outStatus = "Original visibility restore failed; preview retained.";
			return false;
		}
	}
	for (size_t i = 0; i < indices.size(); ++i)
		m_Placements[indices[i]].record.visible = m_DebugHiddenPlacements[i].recordVisible;
	for (auto iter = m_Placements.begin(); iter != m_Placements.end(); )
	{
		if (m_DebugPreviewIds.end() == std::find(m_DebugPreviewIds.begin(),
			m_DebugPreviewIds.end(), iter->record.placementId))
		{
			++iter;
			continue;
		}
		if (iter->object)
		{
			iter->object->Set_Visible(false);
			CGameInstance::Get().Remove_GameObject_from_Layer(m_iLevelIndex, iter->layerTag, iter->object);
		}
		iter = m_Placements.erase(iter);
	}
	m_DebugPreviewIds.clear();
	m_DebugHiddenPlacements.clear();
	outStatus.swap(restoredStatus);
	return true;
}

bool_t CMapPlacementRuntime::Set_DebugSourceLevelVisible(
	const std::string& sourceLevel,
	const bool_t visible,
	const size_t expectedPlacementCount)
{
	if (sourceLevel.empty() || 0u == expectedPlacementCount)
		return false;

	std::vector<MAP_RUNTIME_PLACED_ENTRY*> matches;
	for (MAP_RUNTIME_PLACED_ENTRY& entry : m_Placements)
	{
		if (entry.record.sourceLevel == sourceLevel)
			matches.push_back(&entry);
	}
	if (matches.size() != expectedPlacementCount)
		return false;

	for (MAP_RUNTIME_PLACED_ENTRY* entry : matches)
	{
		if (nullptr == entry || !Set_RuntimeVisible(*entry, visible))
		{
			/* The Debug override is all-or-baseline. A partial proxy group is
			more misleading than no proxy, so restore every authored flag. */
			for (MAP_RUNTIME_PLACED_ENTRY* rollback : matches)
			{
				if (nullptr != rollback)
					(void)Set_RuntimeVisible(
						*rollback, rollback->record.visible);
			}
			return false;
		}
	}
	return true;
}

bool_t CMapPlacementRuntime::Restore_DebugSourceLevelVisibility(
	const std::string& sourceLevel,
	const size_t expectedPlacementCount)
{
	if (sourceLevel.empty() || 0u == expectedPlacementCount)
		return false;

	std::vector<MAP_RUNTIME_PLACED_ENTRY*> matches;
	for (MAP_RUNTIME_PLACED_ENTRY& entry : m_Placements)
	{
		if (entry.record.sourceLevel == sourceLevel)
			matches.push_back(&entry);
	}
	if (matches.size() != expectedPlacementCount)
		return false;

	bool_t restored = true;
	for (MAP_RUNTIME_PLACED_ENTRY* entry : matches)
	{
		if (nullptr == entry ||
			!Set_RuntimeVisible(*entry, entry->record.visible))
		{
			restored = false;
		}
	}
	return restored;
}
#endif

bool_t CMapPlacementRuntime::Is_BatchEligible(
	const MAP_ASSET_ENTRY& asset)
{
    // Native character/prop programs submit per-material render rows to the
    // shared actor light pass. The map instance shader has no such row inputs.
    return MAP_ASSET_RENDER_MODE::DEFERRED == asset.renderProfile.renderMode &&
        std::none_of(asset.materialOverrides.begin(), asset.materialOverrides.end(),
            [&asset](const Engine::MODEL_MATERIAL_OVERRIDE& material) {
                const auto& surface = material.surface;
                const bool_t changedMode = surface.renderMode != Engine::MODEL_SURFACE_RENDER_MODE::INHERIT &&
                    surface.renderMode != Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED;
                const bool_t changedCull = surface.cullMode != Engine::MODEL_SURFACE_CULL_MODE::INHERIT &&
                    ((surface.cullMode == Engine::MODEL_SURFACE_CULL_MODE::CULL_BACK && asset.renderProfile.cullMode != MAP_ASSET_CULL_MODE::CULL_BACK) ||
                     (surface.cullMode == Engine::MODEL_SURFACE_CULL_MODE::CULL_FRONT && asset.renderProfile.cullMode != MAP_ASSET_CULL_MODE::CULL_FRONT) ||
                     (surface.cullMode == Engine::MODEL_SURFACE_CULL_MODE::TWO_SIDED && asset.renderProfile.cullMode != MAP_ASSET_CULL_MODE::TWO_SIDED));
                return surface.family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER || changedMode || changedCull;
            });
}

bool_t CMapPlacementRuntime::Compute_PlacementWorld(
	const MAP_ASSET_ENTRY& asset,
	const shared_ptr<Engine::CModel>& model,
	const MAP_PLACEMENT_RECORD& record,
	float4x4_t& outWorld)
{
	if (nullptr == model || !model->Has_LocalBounds())
		return false;

	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	if (!IsFinite(minimum) || !IsFinite(maximum) ||
		minimum.x > maximum.x || minimum.y > maximum.y ||
		minimum.z > maximum.z)
	{
		return false;
	}

	vector_t rotation = XMQuaternionNormalize(
		XMLoadFloat4(&record.rotationQuaternion));
	if (XMVectorGetW(rotation) < 0.f)
		rotation = XMVectorNegate(rotation);

	matrix_t world = XMMatrixScaling(
		record.signedScale.x,
		record.signedScale.y,
		record.signedScale.z) * XMMatrixRotationQuaternion(rotation);

	float3_t worldOrigin = record.position;
	if (MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset.anchor)
	{
		const vector_t localAnchor = XMVectorSet(
			(minimum.x + maximum.x) * 0.5f, minimum.y,
			(minimum.z + maximum.z) * 0.5f, 1.f);
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, world));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}

	world.r[3] = XMVectorSet(
		worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f);
	XMStoreFloat4x4(&outWorld, world);
	return true;
}

bool_t CMapPlacementRuntime::Try_Get_PlacementWorldBounds(
	const MAP_ASSET_ENTRY& asset,
	const shared_ptr<Engine::CModel>& model,
	const MAP_PLACEMENT_RECORD& record,
	float3_t& outMinimum,
	float3_t& outMaximum)
{
	float4x4_t placementWorld{};
	if (!Compute_PlacementWorld(asset, model, record, placementWorld))
		return false;

	const matrix_t world = XMLoadFloat4x4(&placementWorld);
	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	float3_t worldMinimum{};
	float3_t worldMaximum{};
	for (uint32_t corner = 0u; corner < 8u; ++corner)
	{
		const vector_t local = XMVectorSet(
			0u == (corner & 1u) ? minimum.x : maximum.x,
			0u == (corner & 2u) ? minimum.y : maximum.y,
			0u == (corner & 4u) ? minimum.z : maximum.z,
			1.f);
		float3_t transformed{};
		XMStoreFloat3(&transformed, XMVector3TransformCoord(local, world));
		if (!IsFinite(transformed))
			return false;
		if (0u == corner)
		{
			worldMinimum = transformed;
			worldMaximum = transformed;
			continue;
		}
		worldMinimum.x = (std::min)(worldMinimum.x, transformed.x);
		worldMinimum.y = (std::min)(worldMinimum.y, transformed.y);
		worldMinimum.z = (std::min)(worldMinimum.z, transformed.z);
		worldMaximum.x = (std::max)(worldMaximum.x, transformed.x);
		worldMaximum.y = (std::max)(worldMaximum.y, transformed.y);
		worldMaximum.z = (std::max)(worldMaximum.z, transformed.z);
	}
	outMinimum = worldMinimum;
	outMaximum = worldMaximum;
	return true;
}

shared_ptr<Engine::CModel> CMapPlacementRuntime::Find_PlacementModel(
	const uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	const std::string& assetId,
	std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache)
{
	const auto cached = modelCache.find(assetId);
	if (cached != modelCache.end())
		return cached->second;
	const MAP_ASSET_ENTRY* asset = catalog.Find(assetId);
	if (nullptr == asset)
		return nullptr;
	shared_ptr<Engine::CModel> model = dynamic_pointer_cast<Engine::CModel>(
		CGameInstance::Get().Clone_Prototype(levelIndex, asset->prototypeTag));
	modelCache.emplace(assetId, model);
	return model;
}

bool_t CMapPlacementRuntime::Try_Resolve_PickedPlacement(
	const uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
	const float3_t& worldPoint,
	uint64_t& outPlacementId,
	size_t& outContainingCount)
{
	if (!IsFinite(worldPoint))
		return false;

	/* The picked pixel lies on a drawn surface, so the owner's bounds contain
	   it up to rasterization error; this is that tolerance in metres. */
	constexpr f32_t PICK_EPSILON = 0.05f;
	const vector_t point = XMLoadFloat3(&worldPoint);
	uint64_t containedId = 0u;
	f32_t containedVolume = std::numeric_limits<f32_t>::infinity();
	size_t containedCount = 0u;
	uint64_t sphereId = 0u;
	f32_t sphereDistance = std::numeric_limits<f32_t>::infinity();
	for (const MAP_RUNTIME_PLACED_ENTRY& entry : placements)
	{
		const MAP_ASSET_ENTRY* asset = catalog.Find(entry.record.assetId);
		bool_t visible = false;
		/* A backdrop encloses the whole stage and a hidden entry is not what
		   the pixel shows, so neither can be the picked object. */
		if (nullptr == asset ||
			MAP_ASSET_RENDER_MODE::BACKGROUND == asset->renderProfile.renderMode ||
			!Try_GetRuntimeVisible(entry, visible) || !visible)
		{
			continue;
		}
		const shared_ptr<Engine::CModel> model = Find_PlacementModel(
			levelIndex, catalog, entry.record.assetId, modelCache);
		float4x4_t placementWorld{};
		if (nullptr == model ||
			!Compute_PlacementWorld(*asset, model, entry.record, placementWorld))
		{
			continue;
		}
		const matrix_t world = XMLoadFloat4x4(&placementWorld);
		vector_t determinant{};
		const matrix_t inverse = XMMatrixInverse(&determinant, world);
		const f32_t determinantValue = XMVectorGetX(determinant);
		if (!std::isfinite(determinantValue) ||
			std::abs(determinantValue) < 0.000001f)
		{
			continue;
		}
		float3_t local{};
		XMStoreFloat3(&local, XMVector3TransformCoord(point, inverse));
		if (!IsFinite(local))
			continue;

		const float3_t& minimum = model->Get_LocalBoundsMin();
		const float3_t& maximum = model->Get_LocalBoundsMax();
		const float3_t& scale = entry.record.signedScale;
		const float3_t epsilon(
			PICK_EPSILON / (std::max)(std::abs(scale.x), 0.000001f),
			PICK_EPSILON / (std::max)(std::abs(scale.y), 0.000001f),
			PICK_EPSILON / (std::max)(std::abs(scale.z), 0.000001f));
		if (local.x >= minimum.x - epsilon.x && local.x <= maximum.x + epsilon.x &&
			local.y >= minimum.y - epsilon.y && local.y <= maximum.y + epsilon.y &&
			local.z >= minimum.z - epsilon.z && local.z <= maximum.z + epsilon.z)
		{
			++containedCount;
			const f32_t volume = (maximum.x - minimum.x) *
				(maximum.y - minimum.y) * (maximum.z - minimum.z) *
				std::abs(scale.x * scale.y * scale.z);
			if (volume < containedVolume)
			{
				containedVolume = volume;
				containedId = entry.record.placementId;
			}
			continue;
		}
		if (0u != containedCount)
			continue;

		/* No containing box so far: remember the nearest bounding sphere. */
		FMapStaticInstance instance{};
		if (FAILED(Build_StaticInstance(*asset, model, entry.record, instance)))
			continue;
		const f32_t distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(
			point, XMLoadFloat3(&instance.WorldBoundsCenter))));
		if (std::isfinite(distance) && distance <= instance.WorldBoundsRadius &&
			distance < sphereDistance)
		{
			sphereDistance = distance;
			sphereId = entry.record.placementId;
		}
	}
	if (0u != containedCount)
	{
		outPlacementId = containedId;
		outContainingCount = containedCount;
		return true;
	}
	if (0u != sphereId)
	{
		outPlacementId = sphereId;
		outContainingCount = 0u;
		return true;
	}
	return false;
}

CMapPlacementRuntime::PLACEMENT_TRANSFORM_RESULT
CMapPlacementRuntime::Apply_PlacementTransform(
	const uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
	MAP_RUNTIME_PLACED_ENTRY& entry,
	const MAP_PLACEMENT_RECORD& staged,
	std::string& outStatus)
{
	const MAP_ASSET_ENTRY* asset = catalog.Find(staged.assetId);
	if (nullptr == asset || staged.placementId != entry.record.placementId ||
		!CMapPlacementDocument::Is_Valid(staged, catalog))
	{
		outStatus = "Transform edit rejected by placement validation";
		return PLACEMENT_TRANSFORM_RESULT::REJECTED;
	}

	if (nullptr != entry.object)
	{
		entry.object->Set_PlacementTransform(
			staged.position, staged.rotationQuaternion, staged.signedScale);
		entry.record = staged;
		return PLACEMENT_TRANSFORM_RESULT::APPLIED;
	}
	if (nullptr != entry.batch)
	{
		const bool_t oldMirrored = entry.record.signedScale.x *
			entry.record.signedScale.y * entry.record.signedScale.z < 0.f;
		const bool_t newMirrored = staged.signedScale.x *
			staged.signedScale.y * staged.signedScale.z < 0.f;
		if (oldMirrored == newMirrored)
		{
			const shared_ptr<Engine::CModel> model = Find_PlacementModel(
				levelIndex, catalog, staged.assetId, modelCache);
			FMapStaticInstance instance{};
			if (nullptr != model &&
				SUCCEEDED(Build_StaticInstance(*asset, model, staged, instance)) &&
				SUCCEEDED(entry.batch->Update_Instance(
					staged.placementId, instance)))
			{
				entry.record = staged;
				return PLACEMENT_TRANSFORM_RESULT::APPLIED;
			}
		}
		else
		{
			/* A mirror parity flip changes the batch pass, so the entry moves
			   to a standalone object; the next Reload rebatches it. */
			MAP_RUNTIME_PLACED_ENTRY migrated{};
			if (Create_Placement(levelIndex, catalog, staged, migrated))
			{
				if (SUCCEEDED(entry.batch->Set_InstanceVisible(
					staged.placementId, false)))
				{
					entry.layerTag = std::move(migrated.layerTag);
					entry.object = std::move(migrated.object);
					entry.batch.reset();
					entry.record = staged;
					return PLACEMENT_TRANSFORM_RESULT::APPLIED;
				}
				CGameInstance::Get().Remove_GameObject_from_Layer(
					levelIndex, migrated.layerTag,
					static_pointer_cast<CGameObject>(migrated.object));
			}
		}
	}
	outStatus = "Transform edit failed; previous state preserved";
	return PLACEMENT_TRANSFORM_RESULT::FAILED;
}

HRESULT CMapPlacementRuntime::Build_StaticInstance(
	const MAP_ASSET_ENTRY& asset,
	const shared_ptr<Engine::CModel>& model,
	const MAP_PLACEMENT_RECORD& record,
	FMapStaticInstance& outInstance)
{
	float4x4_t placementWorld{};
	if (!Compute_PlacementWorld(asset, model, record, placementWorld))
		return E_FAIL;

	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	const float3_t localCenter(
		(minimum.x + maximum.x) * 0.5f,
		(minimum.y + maximum.y) * 0.5f,
		(minimum.z + maximum.z) * 0.5f);
	const vector_t halfExtents = XMVectorSet(
		(maximum.x - minimum.x) * 0.5f,
		(maximum.y - minimum.y) * 0.5f,
		(maximum.z - minimum.z) * 0.5f,
		0.f);
	f32_t localRadius = XMVectorGetX(XMVector3Length(halfExtents));
	localRadius = localRadius > 0.05f ? localRadius : 0.05f;

	const matrix_t world = XMLoadFloat4x4(&placementWorld);
	matrix_t linearWorld = world;
	linearWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	const f32_t determinant = XMVectorGetX(
		XMMatrixDeterminant(linearWorld));
	if (!std::isfinite(determinant) ||
		std::abs(determinant) < 0.000001f)
	{
		return E_FAIL;
	}

	const matrix_t worldInvTranspose = XMMatrixTranspose(
		XMMatrixInverse(nullptr, linearWorld));
	const f32_t maximumScale = (std::max)({
		std::abs(record.signedScale.x),
		std::abs(record.signedScale.y),
		std::abs(record.signedScale.z) });
	const f32_t worldRadius =
		localRadius * maximumScale * 1.02f + 0.05f;
	float3_t worldCenter{};
	XMStoreFloat3(&worldCenter,
		XMVector3TransformCoord(XMLoadFloat3(&localCenter), world));
	if (!IsFinite(worldCenter) || !std::isfinite(worldRadius) ||
		worldRadius <= 0.f)
	{
		return E_FAIL;
	}

	outInstance = {};
	outInstance.PlacementId = record.placementId;
	outInstance.BakedLighting = record.bakedLighting;
	outInstance.Visible = record.visible;
	outInstance.WorldBoundsCenter = worldCenter;
	outInstance.WorldBoundsRadius = worldRadius;
	XMStoreFloat4x4(&outInstance.World, world);
	XMStoreFloat4x4(
		&outInstance.WorldInvTranspose, worldInvTranspose);
	return S_OK;
}

std::wstring CMapPlacementRuntime::Make_LayerTag(
	const std::string& sourceLevel)
{
	std::wstring layerTag = L"Layer_MapAsset_";
	layerTag.append(sourceLevel.begin(), sourceLevel.end());
	return layerTag;
}

bool_t Client::CMapPlacementRuntime::Read_SelfMotions(
	const std::string& areaId,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	std::vector<MAP_RUNTIME_SELF_MOTION_ENTRY>& outMotions)
{
	outMotions.clear();
	if (areaId.empty() || placements.empty())
		return true;

	const std::filesystem::path path = CProjectDataRoot::Resolve(
		std::filesystem::path("Maps") / "Authoring" / areaId /
		(areaId + ".mapmotions.json"));
	std::error_code existsError{};
	/* Most areas author none, so absence is the normal case. */
	if (path.empty() || !std::filesystem::is_regular_file(path, existsError) ||
		existsError)
	{
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input.is_open())
		return false;
	const std::string text(
		(std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());
	input.close();

	Client::DATA_JSON_VALUE root{};
	std::string parseError;
	if (!Client::CDataJson::Parse(text, root, parseError) || !root.Is_Object())
		return false;
	const auto& rootObject = root.Get_Object();
	const auto schemaIter = rootObject.find("schema");
	const auto motionsIter = rootObject.find("motions");
	if (schemaIter == rootObject.end() || !schemaIter->second.Is_String() ||
		"lostark.map-self-motions" != schemaIter->second.Get_String() ||
		motionsIter == rootObject.end() || !motionsIter->second.Is_Array())
	{
		return false;
	}

	std::unordered_map<uint64_t, size_t> byPlacement;
	byPlacement.reserve(placements.size());
	for (size_t index = 0u; index < placements.size(); ++index)
		byPlacement[placements[index].record.placementId] = index;

	for (const Client::DATA_JSON_VALUE& value : motionsIter->second.Get_Array())
	{
		if (!value.Is_Object())
			continue;
		const auto& row = value.Get_Object();
		const auto idIter = row.find("placementId");
		const auto kindIter = row.find("kind");
		const auto axisIter = row.find("axis");
		if (idIter == row.end() || !idIter->second.Is_String() ||
			kindIter == row.end() || !kindIter->second.Is_String() ||
			axisIter == row.end() || !axisIter->second.Is_String())
		{
			continue;
		}

		uint64_t placementId = 0ull;
		try
		{
			placementId = std::stoull(idIter->second.Get_String());
		}
		catch (const std::exception&)
		{
			continue;
		}
		const auto placedIter = byPlacement.find(placementId);
		if (placedIter == byPlacement.end())
			continue;

		MAP_SELF_MOTION motion{};
		motion.placementId = placementId;
		const std::string& kind = kindIter->second.Get_String();
		if ("ROTATION_CYCLIC" == kind)
			motion.kind = MAP_SELF_MOTION_KIND::ROTATION_CYCLIC;
		else if ("ROTATION_ACYCLIC" == kind)
			motion.kind = MAP_SELF_MOTION_KIND::ROTATION_ACYCLIC;
		else if ("LOCATION_CYCLIC" == kind)
			motion.kind = MAP_SELF_MOTION_KIND::LOCATION_CYCLIC;
		else if ("LOCATION_ACYCLIC" == kind)
			motion.kind = MAP_SELF_MOTION_KIND::LOCATION_ACYCLIC;
		else
			continue;
		const std::string& axis = axisIter->second.Get_String();
		if ("X" == axis)
			motion.axis = MAP_SELF_MOTION_AXIS::X;
		else if ("Y" == axis)
			motion.axis = MAP_SELF_MOTION_AXIS::Y;
		else if ("Z" == axis)
			motion.axis = MAP_SELF_MOTION_AXIS::Z;
		else
			continue;

		const auto readNumber = [&row](const char* key, f32_t& outValue)
		{
			const auto iter = row.find(key);
			if (iter == row.end() || !iter->second.Is_Number())
				return;
			const double raw = iter->second.Get_Number();
			if (std::isfinite(raw))
				outValue = static_cast<f32_t>(raw);
		};
		readNumber("cycleSeconds", motion.cycleSeconds);
		readNumber("range", motion.range);
		readNumber("startPhaseSeconds", motion.startPhaseSeconds);
		/* A cyclic row with no cycle would divide by zero every frame. */
		if ((MAP_SELF_MOTION_KIND::ROTATION_CYCLIC == motion.kind ||
			MAP_SELF_MOTION_KIND::LOCATION_CYCLIC == motion.kind) &&
			motion.cycleSeconds <= 0.f)
		{
			continue;
		}

		MAP_RUNTIME_SELF_MOTION_ENTRY entry{};
		entry.motion = motion;
		entry.placementIndex = placedIter->second;
		const MAP_PLACEMENT_RECORD& record =
			placements[placedIter->second].record;
		entry.basePosition = record.position;
		entry.baseRotation = record.rotationQuaternion;
		outMotions.push_back(entry);
	}
	return true;
}

void Client::CMapPlacementRuntime::Sample_SelfMotions(
	const std::vector<MAP_RUNTIME_SELF_MOTION_ENTRY>& motions,
	const f32_t elapsedSeconds,
	const uint32_t levelIndex,
	const CMapAssetCatalog& catalog,
	std::unordered_map<std::string, shared_ptr<Engine::CModel>>& modelCache,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements)
{
	if (motions.empty() || !std::isfinite(elapsedSeconds))
		return;
	for (const MAP_RUNTIME_SELF_MOTION_ENTRY& entry : motions)
	{
		if (entry.placementIndex >= placements.size())
			continue;
		MAP_RUNTIME_PLACED_ENTRY& placed = placements[entry.placementIndex];
		const MAP_SELF_MOTION& motion = entry.motion;
		const f32_t time = elapsedSeconds +
			motion.startPhaseSeconds;

		f32_t amount = 0.f;
		switch (motion.kind)
		{
		case MAP_SELF_MOTION_KIND::ROTATION_CYCLIC:
		case MAP_SELF_MOTION_KIND::LOCATION_CYCLIC:
			amount = motion.range * std::sin(
				6.283185307f * time / motion.cycleSeconds);
			break;
		case MAP_SELF_MOTION_KIND::ROTATION_ACYCLIC:
		case MAP_SELF_MOTION_KIND::LOCATION_ACYCLIC:
			/* No cycle authored: turn at range per second, which is how the
			   original keeps these spinning rather than swinging. */
			amount = motion.range * time;
			break;
		default:
			continue;
		}
		if (!std::isfinite(amount))
			continue;

		MAP_PLACEMENT_RECORD sampled = placed.record;
		sampled.position = entry.basePosition;
		sampled.rotationQuaternion = entry.baseRotation;
		const bool_t isRotation =
			MAP_SELF_MOTION_KIND::ROTATION_CYCLIC == motion.kind ||
			MAP_SELF_MOTION_KIND::ROTATION_ACYCLIC == motion.kind;
		if (isRotation)
		{
			const f32_t radians = XMConvertToRadians(amount);
			const vector_t axis =
				MAP_SELF_MOTION_AXIS::X == motion.axis
				? XMVectorSet(1.f, 0.f, 0.f, 0.f)
				: (MAP_SELF_MOTION_AXIS::Y == motion.axis
					? XMVectorSet(0.f, 1.f, 0.f, 0.f)
					: XMVectorSet(0.f, 0.f, 1.f, 0.f));
			const vector_t swing = XMQuaternionRotationAxis(axis, radians);
			const vector_t base = XMVectorSet(
				entry.baseRotation.x, entry.baseRotation.y,
				entry.baseRotation.z, entry.baseRotation.w);
			XMStoreFloat4(&sampled.rotationQuaternion,
				XMQuaternionNormalize(XMQuaternionMultiply(base, swing)));
		}
		else
		{
			float3_t offset = entry.basePosition;
			if (MAP_SELF_MOTION_AXIS::X == motion.axis)
				offset.x += amount;
			else if (MAP_SELF_MOTION_AXIS::Y == motion.axis)
				offset.y += amount;
			else
				offset.z += amount;
			sampled.position = offset;
		}

		if (nullptr != placed.object)
		{
			placed.object->Set_PlacementTransform(sampled.position,
				sampled.rotationQuaternion, sampled.signedScale);
			placed.record.position = sampled.position;
			placed.record.rotationQuaternion = sampled.rotationQuaternion;
		}
		else if (nullptr != placed.batch)
		{
			// Motion owns the transform, while a Sequence or arena owns visibility.
			// Rebuilding from the authored record must not reveal a hidden batch.
			if (!Try_GetRuntimeVisible(placed, sampled.visible))
				continue;
			const MAP_ASSET_ENTRY* asset = catalog.Find(sampled.assetId);
			if (nullptr == asset)
				continue;
			auto modelIter = modelCache.find(sampled.assetId);
			if (modelIter == modelCache.end())
			{
				modelIter = modelCache.emplace(sampled.assetId,
					dynamic_pointer_cast<Engine::CModel>(
						CGameInstance::Get().Clone_Prototype(
							levelIndex, asset->prototypeTag))).first;
			}
			FMapStaticInstance instance{};
			if (nullptr != modelIter->second &&
				SUCCEEDED(Build_StaticInstance(
					*asset, modelIter->second, sampled, instance)) &&
				SUCCEEDED(placed.batch->Update_Instance(
					sampled.placementId, instance)))
			{
				placed.record.position = sampled.position;
				placed.record.rotationQuaternion = sampled.rotationQuaternion;
			}
		}
	}
}

bool_t Client::CMapPlacementRuntime::Load_SelfMotions(
	const std::string& areaId)
{
	m_fSelfMotionElapsedSeconds = 0.f;
	return Read_SelfMotions(areaId, m_Placements, m_SelfMotions);
}

void Client::CMapPlacementRuntime::Update_SelfMotions(const f32_t fTimeDelta)
{
	if (m_SelfMotions.empty() || !std::isfinite(fTimeDelta))
		return;
	m_fSelfMotionElapsedSeconds += fTimeDelta;
	if (m_fSelfMotionElapsedSeconds > SELF_MOTION_WRAP_SECONDS)
		m_fSelfMotionElapsedSeconds -= SELF_MOTION_WRAP_SECONDS;
	Sample_SelfMotions(m_SelfMotions, m_fSelfMotionElapsedSeconds,
		m_iLevelIndex, m_Catalog, m_SelfMotionModels, m_Placements);
}

#ifdef _DEBUG
void Client::CMapPlacementRuntime::Rebase_AuthoringSelfMotions(
	const std::vector<MAP_PLACEMENT_RECORD>& records)
{
	std::unordered_map<uint64_t, const MAP_PLACEMENT_RECORD*> authored;
	for (const auto& record : records) authored.emplace(record.placementId, &record);
	std::unordered_map<uint64_t, size_t> live;
	for (size_t index = 0; index < m_Placements.size(); ++index)
		live.emplace(m_Placements[index].record.placementId, index);
	for (auto& motion : m_SelfMotions)
	{
		const auto source = authored.find(motion.motion.placementId);
		const auto placement = live.find(motion.motion.placementId);
		if (source == authored.end() || placement == live.end())
		{
			// Retain the definition so Reload can restore a deleted placement.
			// The sampler skips this out-of-range index until the ID returns.
			motion.placementIndex = static_cast<size_t>(-1);
			continue;
		}
		motion.placementIndex = placement->second;
		motion.basePosition = source->second->position;
		motion.baseRotation = source->second->rotationQuaternion;
	}
}
#endif
