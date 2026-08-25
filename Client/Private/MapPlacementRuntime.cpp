#include "MapPlacementRuntime.h"

#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
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
	const MAP_FRUSTUM_CULLING_POLICY& frustumCulling)
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
	desc.frustumCulling = frustumCulling;

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

	outEntry = {};
	outEntry.record = record;
	outEntry.layerTag = layerTag;
	outEntry.object = std::move(mapObject);
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

#ifdef _DEBUG
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
	return MAP_ASSET_RENDER_MODE::DEFERRED ==
		asset.renderProfile.renderMode;
}

HRESULT CMapPlacementRuntime::Build_StaticInstance(
	const MAP_ASSET_ENTRY& asset,
	const shared_ptr<Engine::CModel>& model,
	const MAP_PLACEMENT_RECORD& record,
	FMapStaticInstance& outInstance)
{
	if (nullptr == model || !model->Has_LocalBounds())
		return E_FAIL;

	const float3_t& minimum = model->Get_LocalBoundsMin();
	const float3_t& maximum = model->Get_LocalBoundsMax();
	if (!IsFinite(minimum) || !IsFinite(maximum) ||
		minimum.x > maximum.x || minimum.y > maximum.y ||
		minimum.z > maximum.z)
	{
		return E_FAIL;
	}

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
			localCenter.x, minimum.y, localCenter.z, 1.f);
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, world));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}

	world.r[3] = XMVectorSet(
		worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f);

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
