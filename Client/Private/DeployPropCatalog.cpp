#include "DeployPropCatalog.h"

#include "MapAssetCatalog.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_DEPLOY_PROP_CATALOG";
	constexpr const char* PLACEMENT_MAGIC = "LOSTARK_DEPLOY_PROP_PLACEMENTS";
	constexpr uint32_t CATALOG_FORMAT_VERSION = 2;
	constexpr uint32_t PLACEMENT_FORMAT_VERSION = 1;
	constexpr uint32_t MAX_ASSET_COUNT = 64;
	constexpr uint32_t MAX_PLACEMENT_COUNT = 4096;

	bool_t IsInsideRoot(const std::filesystem::path& root,
		const std::filesystem::path& candidate)
	{
		std::error_code error;
		const std::filesystem::path relative =
			std::filesystem::relative(candidate, root, error);
		if (error || relative.empty() || relative.is_absolute())
			return false;
		const auto first = relative.begin();
		return first != relative.end() && *first != L"..";
	}

	bool_t IsFinite(const DEPLOY_PROP_PLACEMENT& row)
	{
		const f32_t quaternionLength = XMVectorGetX(XMVector4Length(
			XMLoadFloat4(&row.rotationQuaternion)));
		return std::isfinite(row.position.x) &&
			std::isfinite(row.position.y) &&
			std::isfinite(row.position.z) &&
			std::isfinite(quaternionLength) && quaternionLength > 0.000001f &&
			std::isfinite(row.uniformScale) && row.uniformScale > 0.000001f;
	}
}

bool_t CDeployPropCatalog::Load_Default(const std::string& expectedAreaId)
{
	if (expectedAreaId.empty())
	{
		m_Status = "DeployProp expected area is empty";
		return false;
	}
	const std::filesystem::path root = CMapAssetCatalog::Get_MapDataRoot();
	return Load(
		root / (std::filesystem::path(expectedAreaId).wstring() + L".deployassets"),
		root / (std::filesystem::path(expectedAreaId).wstring() + L".deployplacements"),
		expectedAreaId);
}

bool_t CDeployPropCatalog::Load(
	const std::filesystem::path& catalogPath,
	const std::filesystem::path& placementPath,
	const std::string& expectedAreaId)
{
	m_bReady = false;
	std::ifstream catalog(catalogPath, std::ios::binary);
	std::string magic;
	std::string areaId;
	uint32_t version = {};
	uint32_t assetCount = {};
	if (!catalog || !(catalog >> magic >> version >> std::quoted(areaId) >> assetCount) ||
		magic != CATALOG_MAGIC || version != CATALOG_FORMAT_VERSION ||
		areaId != expectedAreaId || 0 == assetCount || assetCount > MAX_ASSET_COUNT)
	{
		m_Status = "DeployProp catalog header is invalid";
		return false;
	}

	const std::filesystem::path assetRoot = CRuntimeAssetRoot::Get();
	std::vector<DEPLOY_PROP_ASSET_ENTRY> stagedAssets;
	std::unordered_set<std::string> assetIds;
	std::unordered_set<std::wstring> prototypeTags;
	stagedAssets.reserve(assetCount);
	for (uint32_t index = 0; index < assetCount; ++index)
	{
		std::string kind;
		std::string intactPath;
		std::string intactPrototype;
		std::string fracturedPath;
		std::string fracturedPrototype;
		uint32_t deferredEmissiveOverlay = {};
		DEPLOY_PROP_ASSET_ENTRY entry{};
		if (!(catalog >> std::quoted(entry.id) >> kind >>
			std::quoted(entry.label) >> std::quoted(intactPath) >>
			std::quoted(intactPrototype) >> std::quoted(fracturedPath) >>
			std::quoted(fracturedPrototype) >> entry.emissiveIntensity >>
			deferredEmissiveOverlay >> std::quoted(entry.evidence)))
		{
			m_Status = "DeployProp catalog row is truncated at " +
				std::to_string(index);
			return false;
		}
		if (kind == "STATIC")
			entry.kind = DEPLOY_PROP_MODEL_KIND::STATIC;
		else if (kind == "ANIM")
			entry.kind = DEPLOY_PROP_MODEL_KIND::ANIM;
		else
		{
			m_Status = "DeployProp catalog kind is invalid";
			return false;
		}
		entry.intactRelativePath = std::filesystem::path(intactPath).lexically_normal();
		entry.intactResolvedPath = CRuntimeAssetRoot::Resolve(entry.intactRelativePath);
		entry.intactPrototypeTag.assign(intactPrototype.begin(), intactPrototype.end());
		entry.fracturedRelativePath = std::filesystem::path(fracturedPath).lexically_normal();
		entry.fracturedPrototypeTag.assign(fracturedPrototype.begin(), fracturedPrototype.end());
		entry.deferredEmissiveOverlay = 0u != deferredEmissiveOverlay;
		if (!fracturedPath.empty())
			entry.fracturedResolvedPath = CRuntimeAssetRoot::Resolve(entry.fracturedRelativePath);

		// A STATIC prop may omit its fractured model when the authored mutation
		// ends at DESPAWNED, which is how the Valtan arena floor sectors are
		// authored. The pair stays all-or-nothing so a half-declared row can
		// never reach the runtime, and ANIM still refuses a fractured model.
		const bool_t declaresFractured =
			!fracturedPath.empty() || !fracturedPrototype.empty();
		const bool_t requiresFractured =
			entry.kind == DEPLOY_PROP_MODEL_KIND::STATIC && declaresFractured;
		if (entry.id.empty() || entry.label.empty() || entry.evidence.empty() ||
			entry.intactRelativePath.is_absolute() ||
			entry.intactRelativePath.extension() != L".wmodel" ||
			entry.intactPrototypeTag.empty() ||
			!IsInsideRoot(assetRoot, entry.intactResolvedPath) ||
			!std::filesystem::is_regular_file(entry.intactResolvedPath) ||
			(requiresFractured && (entry.fracturedRelativePath.is_absolute() ||
				entry.fracturedRelativePath.extension() != L".wmodel" ||
				entry.fracturedPrototypeTag.empty() ||
				!IsInsideRoot(assetRoot, entry.fracturedResolvedPath) ||
				!std::filesystem::is_regular_file(entry.fracturedResolvedPath))) ||
			(declaresFractured &&
				(fracturedPath.empty() || fracturedPrototype.empty())) ||
			(entry.kind == DEPLOY_PROP_MODEL_KIND::ANIM && declaresFractured) ||
			!std::isfinite(entry.emissiveIntensity) ||
			entry.emissiveIntensity < 0.f ||
			deferredEmissiveOverlay > 1u ||
			(entry.deferredEmissiveOverlay &&
				entry.kind != DEPLOY_PROP_MODEL_KIND::STATIC) ||
			!assetIds.insert(entry.id).second ||
			!prototypeTags.insert(entry.intactPrototypeTag).second ||
			(requiresFractured &&
				!prototypeTags.insert(entry.fracturedPrototypeTag).second))
		{
			m_Status = "DeployProp catalog validation failed for " + entry.id;
			return false;
		}
		stagedAssets.push_back(std::move(entry));
	}
	std::string trailing;
	if (catalog >> trailing)
	{
		m_Status = "DeployProp catalog contains trailing data";
		return false;
	}

	std::ifstream placements(placementPath, std::ios::binary);
	std::string placementAreaId;
	uint32_t placementCount = {};
	if (!placements || !(placements >> magic >> version >>
		std::quoted(placementAreaId) >> placementCount) ||
		magic != PLACEMENT_MAGIC || version != PLACEMENT_FORMAT_VERSION ||
		placementAreaId != areaId || 0 == placementCount ||
		placementCount > MAX_PLACEMENT_COUNT)
	{
		m_Status = "DeployProp placement header is invalid";
		return false;
	}

	std::vector<DEPLOY_PROP_PLACEMENT> stagedPlacements;
	std::unordered_set<uint64_t> runtimeIds;
	stagedPlacements.reserve(placementCount);
	for (uint32_t index = 0; index < placementCount; ++index)
	{
		DEPLOY_PROP_PLACEMENT row{};
		uint32_t destructible = {};
		if (!(placements >> row.runtimePlacementId >> row.deployActorId >>
			row.propDefinitionId >> std::quoted(row.sourcePlacementId) >>
			std::quoted(row.assetId) >> row.position.x >> row.position.y >>
			row.position.z >> row.rotationQuaternion.x >>
			row.rotationQuaternion.y >> row.rotationQuaternion.z >>
			row.rotationQuaternion.w >> row.uniformScale >> destructible >>
			row.stateOffActionId >> row.triggerBinaryOccurrenceCount))
		{
			m_Status = "DeployProp placement row is truncated at " +
				std::to_string(index);
			return false;
		}
		row.destructible = 0 != destructible;
		const auto asset = std::find_if(stagedAssets.begin(), stagedAssets.end(),
			[&](const DEPLOY_PROP_ASSET_ENTRY& value)
			{
				return value.id == row.assetId;
			});
		if (0 == row.runtimePlacementId || 0 == row.deployActorId ||
			0 == row.propDefinitionId || row.sourcePlacementId.empty() ||
			asset == stagedAssets.end() || destructible > 1 ||
			!IsFinite(row) || !runtimeIds.insert(row.runtimePlacementId).second)
		{
			m_Status = "DeployProp placement validation failed at " +
				std::to_string(index);
			return false;
		}
		stagedPlacements.push_back(std::move(row));
	}
	if (placements >> trailing)
	{
		m_Status = "DeployProp placements contain trailing data";
		return false;
	}

	m_Assets = std::move(stagedAssets);
	m_Placements = std::move(stagedPlacements);
	m_AreaId = std::move(areaId);
	m_bReady = true;
	m_Status = "DeployProp ready: " + std::to_string(m_Assets.size()) +
		" assets, " + std::to_string(m_Placements.size()) + " placements";
	return true;
}

const DEPLOY_PROP_ASSET_ENTRY* CDeployPropCatalog::Find(
	const std::string& assetId) const
{
	const auto iter = std::find_if(m_Assets.begin(), m_Assets.end(),
		[&](const DEPLOY_PROP_ASSET_ENTRY& entry)
		{
			return entry.id == assetId;
		});
	return iter == m_Assets.end() ? nullptr : &*iter;
}

