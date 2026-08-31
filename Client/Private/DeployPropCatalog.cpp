#include "DeployPropCatalog.h"

#include "MapAssetCatalog.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_DEPLOY_PROP_CATALOG";
	constexpr const char* PLACEMENT_MAGIC = "LOSTARK_DEPLOY_PROP_PLACEMENTS";
	/* Keep the historical name as the v2 compatibility marker because the
	   Valtan extraction contract audits that exact admission boundary. */
	constexpr uint32_t CATALOG_FORMAT_VERSION = 2;
	constexpr uint32_t CATALOG_FORMAT_VERSION_V2 = CATALOG_FORMAT_VERSION;
	constexpr uint32_t CATALOG_FORMAT_VERSION_V3 = 3;
	constexpr uint32_t PLACEMENT_FORMAT_VERSION_V1 = 1;
	constexpr uint32_t PLACEMENT_FORMAT_VERSION_V2 = 2;
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

	bool_t IsKnownAsset(
		const std::vector<DEPLOY_PROP_ASSET_ENTRY>& assets,
		const std::string& assetId)
	{
		return assets.end() != std::find_if(
			assets.begin(), assets.end(),
			[&](const DEPLOY_PROP_ASSET_ENTRY& asset)
			{
				return asset.id == assetId;
			});
	}

	bool_t IsValidAnimationRole(const std::string& role)
	{
		if (role.size() > 256u)
			return false;
		return role.end() == std::find_if(
			role.begin(), role.end(),
			[](const unsigned char value)
			{
				return value < 0x20u;
			});
	}

	bool_t ValidatePlacement(
		const DEPLOY_PROP_PLACEMENT& row,
		const std::vector<DEPLOY_PROP_ASSET_ENTRY>& assets)
	{
		if (0u == row.runtimePlacementId || row.sourcePlacementId.empty() ||
			!IsKnownAsset(assets, row.assetId) || !IsFinite(row))
		{
			return false;
		}

		if (DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT == row.provenance)
		{
			return 0u != row.deployActorId && 0u != row.propDefinitionId;
		}
		if (DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED != row.provenance)
			return false;

		/* Project-authored rows cannot impersonate source evidence. Their stable
		   runtime/source IDs are project IDs and every extractor-only field stays
		   zero until a real source row is imported. */
		return 0u == row.deployActorId && 0u == row.propDefinitionId &&
			0u == row.stateOffActionId &&
			0u == row.triggerBinaryOccurrenceCount;
	}

	const char* ToProvenanceToken(
		const DEPLOY_PROP_PLACEMENT_PROVENANCE provenance)
	{
		return DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED == provenance ?
			"PROJECT_AUTHORED" : "SOURCE_EXACT";
	}

	bool_t ParseProvenanceToken(
		const std::string& token,
		DEPLOY_PROP_PLACEMENT_PROVENANCE& outProvenance)
	{
		if ("SOURCE_EXACT" == token)
		{
			outProvenance =
				DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT;
			return true;
		}
		if ("PROJECT_AUTHORED" == token)
		{
			outProvenance =
				DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED;
			return true;
		}
		return false;
	}

	std::filesystem::path MakeTemporaryPlacementPath(
		const std::filesystem::path& target)
	{
		static std::atomic<uint64_t> serial = 0u;
		const uint64_t timestamp = static_cast<uint64_t>(
			std::chrono::steady_clock::now().time_since_epoch().count());
		return std::filesystem::path(target.wstring() + L".tmp." +
			std::to_wstring(timestamp) + L"." +
			std::to_wstring(++serial));
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
	m_PlacementPath.clear();
	std::ifstream catalog(catalogPath, std::ios::binary);
	std::string magic;
	std::string areaId;
	uint32_t catalogVersion = {};
	uint32_t assetCount = {};
	if (!catalog || !(catalog >> magic >> catalogVersion >>
		std::quoted(areaId) >> assetCount) || magic != CATALOG_MAGIC ||
		(catalogVersion != CATALOG_FORMAT_VERSION_V2 &&
			catalogVersion != CATALOG_FORMAT_VERSION_V3) ||
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
		if (CATALOG_FORMAT_VERSION_V3 == catalogVersion &&
			!(catalog >> std::quoted(entry.animationRoles.intactClip) >>
				std::quoted(entry.animationRoles.fracturedClip)))
		{
			m_Status = "DeployProp catalog animation roles are truncated at " +
				std::to_string(index);
			return false;
		}
		entry.strictAnimationRoles =
			CATALOG_FORMAT_VERSION_V3 == catalogVersion;
		if (kind == "STATIC")
			entry.kind = DEPLOY_PROP_MODEL_KIND::STATIC;
		else if (kind == "ANIM")
		{
			entry.kind = DEPLOY_PROP_MODEL_KIND::ANIM;
			if (CATALOG_FORMAT_VERSION_V2 == catalogVersion)
			{
				entry.animationRoles.intactClip = "on";
				entry.animationRoles.fracturedClip = "off";
			}
		}
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
			(entry.kind == DEPLOY_PROP_MODEL_KIND::STATIC &&
				(!entry.animationRoles.intactClip.empty() ||
					!entry.animationRoles.fracturedClip.empty())) ||
			!IsValidAnimationRole(entry.animationRoles.intactClip) ||
			!IsValidAnimationRole(entry.animationRoles.fracturedClip) ||
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
	uint32_t placementVersion = {};
	uint32_t placementCount = {};
	if (!placements || !(placements >> magic >> placementVersion >>
		std::quoted(placementAreaId) >> placementCount) ||
		magic != PLACEMENT_MAGIC ||
		(placementVersion != PLACEMENT_FORMAT_VERSION_V1 &&
			placementVersion != PLACEMENT_FORMAT_VERSION_V2) ||
		placementAreaId != areaId ||
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
		if (PLACEMENT_FORMAT_VERSION_V2 == placementVersion)
		{
			std::string provenance;
			if (!(placements >> provenance) ||
				!ParseProvenanceToken(provenance, row.provenance))
			{
				m_Status = "DeployProp placement provenance is invalid at " +
					std::to_string(index);
				return false;
			}
		}
		else
		{
			row.provenance =
				DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT;
		}
		if (destructible > 1 || !ValidatePlacement(row, stagedAssets) ||
			!runtimeIds.insert(row.runtimePlacementId).second)
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
	m_PlacementPath = placementPath.lexically_normal();
	m_bReady = true;
	m_Status = "DeployProp ready: " + std::to_string(m_Assets.size()) +
		" assets, " + std::to_string(m_Placements.size()) + " placements";
	return true;
}

bool_t CDeployPropCatalog::Save_Placements()
{
	if (m_PlacementPath.empty())
	{
		m_Status = "DeployProp placement save path is empty";
		return false;
	}
	return Save_Placements(m_PlacementPath);
}

bool_t CDeployPropCatalog::Save_Placements(
	const std::filesystem::path& placementPath)
{
	if (!m_bReady || m_AreaId.empty() || placementPath.empty() ||
		m_Placements.size() > MAX_PLACEMENT_COUNT)
	{
		m_Status = "DeployProp placement save input is invalid";
		return false;
	}

	std::unordered_set<uint64_t> runtimeIds;
	runtimeIds.reserve(m_Placements.size());
	for (const DEPLOY_PROP_PLACEMENT& row : m_Placements)
	{
		if (!ValidatePlacement(row, m_Assets) ||
			!runtimeIds.insert(row.runtimePlacementId).second)
		{
			m_Status = "DeployProp placement save validation failed";
			return false;
		}
	}

	std::error_code error;
	const std::filesystem::path parent = placementPath.parent_path();
	if (!parent.empty())
	{
		std::filesystem::create_directories(parent, error);
		if (error)
		{
			m_Status = "DeployProp placement directory could not be created";
			return false;
		}
	}

	const std::filesystem::path temporary =
		MakeTemporaryPlacementPath(placementPath);
	{
		std::ofstream output(
			temporary, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			m_Status = "DeployProp temporary placement file could not be opened";
			return false;
		}

		output << PLACEMENT_MAGIC << ' ' << PLACEMENT_FORMAT_VERSION_V2 << ' '
			<< std::quoted(m_AreaId) << ' ' << m_Placements.size() << '\n';
		output << std::setprecision(
			std::numeric_limits<f32_t>::max_digits10);
		for (const DEPLOY_PROP_PLACEMENT& row : m_Placements)
		{
			output << row.runtimePlacementId << ' '
				<< row.deployActorId << ' '
				<< row.propDefinitionId << ' '
				<< std::quoted(row.sourcePlacementId) << ' '
				<< std::quoted(row.assetId) << ' '
				<< row.position.x << ' ' << row.position.y << ' '
				<< row.position.z << ' '
				<< row.rotationQuaternion.x << ' '
				<< row.rotationQuaternion.y << ' '
				<< row.rotationQuaternion.z << ' '
				<< row.rotationQuaternion.w << ' '
				<< row.uniformScale << ' '
				<< (row.destructible ? 1u : 0u) << ' '
				<< row.stateOffActionId << ' '
				<< row.triggerBinaryOccurrenceCount << ' '
				<< ToProvenanceToken(row.provenance) << '\n';
		}
		output.flush();
		if (!output)
		{
			output.close();
			std::filesystem::remove(temporary, error);
			m_Status = "DeployProp temporary placement write failed";
			return false;
		}
	}

	if (!MoveFileExW(
		temporary.c_str(), placementPath.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::filesystem::remove(temporary, error);
		m_Status = "DeployProp atomic placement replace failed";
		return false;
	}

	m_PlacementPath = placementPath.lexically_normal();
	m_Status = "DeployProp placements saved atomically: " +
		std::to_string(m_Placements.size());
	return true;
}

bool_t CDeployPropCatalog::Add_ProjectAuthoredPlacement(
	DEPLOY_PROP_PLACEMENT placement)
{
	if (!m_bReady || m_Placements.size() >= MAX_PLACEMENT_COUNT ||
		0u == placement.runtimePlacementId)
	{
		m_Status = "DeployProp project placement add input is invalid";
		return false;
	}
	placement.provenance =
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED;
	if (placement.sourcePlacementId.empty())
	{
		placement.sourcePlacementId =
			"project." + std::to_string(placement.runtimePlacementId);
	}
	if (!ValidatePlacement(placement, m_Assets) ||
		m_Placements.end() != std::find_if(
			m_Placements.begin(), m_Placements.end(),
			[&](const DEPLOY_PROP_PLACEMENT& row)
			{
				return row.runtimePlacementId == placement.runtimePlacementId;
			}))
	{
		m_Status = "DeployProp project placement add validation failed";
		return false;
	}

	m_Placements.push_back(std::move(placement));
	m_Status = "DeployProp project placement added";
	return true;
}

bool_t CDeployPropCatalog::Update_ProjectAuthoredPlacement(
	const DEPLOY_PROP_PLACEMENT& placement)
{
	if (!m_bReady ||
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED !=
			placement.provenance ||
		!ValidatePlacement(placement, m_Assets))
	{
		m_Status = "DeployProp project placement update input is invalid";
		return false;
	}

	const auto iter = std::find_if(
		m_Placements.begin(), m_Placements.end(),
		[&](const DEPLOY_PROP_PLACEMENT& row)
		{
			return row.runtimePlacementId == placement.runtimePlacementId;
		});
	if (iter == m_Placements.end() ||
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED != iter->provenance ||
		iter->sourcePlacementId != placement.sourcePlacementId)
	{
		m_Status = "DeployProp project placement update target is immutable";
		return false;
	}

	*iter = placement;
	m_Status = "DeployProp project placement updated";
	return true;
}

bool_t CDeployPropCatalog::Remove_ProjectAuthoredPlacement(
	const uint64_t runtimePlacementId)
{
	if (!m_bReady || 0u == runtimePlacementId)
	{
		m_Status = "DeployProp project placement remove input is invalid";
		return false;
	}

	const auto iter = std::find_if(
		m_Placements.begin(), m_Placements.end(),
		[&](const DEPLOY_PROP_PLACEMENT& row)
		{
			return row.runtimePlacementId == runtimePlacementId;
		});
	if (iter == m_Placements.end() ||
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED != iter->provenance)
	{
		m_Status = "DeployProp project placement remove target is immutable";
		return false;
	}

	m_Placements.erase(iter);
	m_Status = "DeployProp project placement removed";
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

