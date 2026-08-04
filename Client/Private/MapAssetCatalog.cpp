#include "MapAssetCatalog.h"

#include "RuntimeAssetRoot.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG";
	constexpr uint32_t LEGACY_CATALOG_VERSION = 1;
	constexpr uint32_t METADATA_CATALOG_VERSION = 2;
	constexpr uint32_t RENDER_PROFILE_CATALOG_VERSION = 3;
	constexpr uint32_t CATALOG_VERSION = 4;
	constexpr uint32_t MAX_ASSET_COUNT = 512;
	constexpr const char* SHARD_SET_MAGIC = "LOSTARK_MAP_SHARD_SET";
	constexpr uint32_t SHARD_SET_VERSION = 1;
	constexpr uint32_t MAX_SHARD_COUNT = 64;
	constexpr uint32_t MAX_TOTAL_ASSET_COUNT = 4096;
	constexpr uint32_t MAX_SHARD_PLACEMENT_COUNT = 65536;
	constexpr size_t MAX_GROUP_ID_LENGTH = 64;
	constexpr size_t MAX_GROUP_LABEL_LENGTH = 128;
	constexpr size_t MAX_EVIDENCE_LENGTH = 512;
	constexpr size_t MAX_SHARD_FILENAME_LENGTH = 260;

	struct PARSED_MAP_ASSET_ROW
	{
		std::string id;
		std::string label;
		std::string modelPath;
		std::string prototypeTag;
		float3_t defaultScale = float3_t(1.f, 1.f, 1.f);
		std::string anchor;
		std::string groupId;
		std::string groupLabel;
		std::string evidence;
		std::string renderMode = "Opaque";
		std::string cullMode = "Back";
		MAP_ASSET_RENDER_PROFILE renderProfile;
	};

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

	bool_t IsValidScale(const float3_t& scale)
	{
		return std::isfinite(scale.x) && std::isfinite(scale.y) &&
			std::isfinite(scale.z) && scale.x > 0.f &&
			scale.y > 0.f && scale.z > 0.f;
	}

	bool_t IsValidGroupId(const std::string& groupId)
	{
		if (groupId.empty() || groupId.size() > MAX_GROUP_ID_LENGTH)
			return false;

		return std::all_of(groupId.begin(), groupId.end(), [](const char value)
		{
			const unsigned char character = static_cast<unsigned char>(value);
			return 0 != std::isalnum(character) || value == '_' ||
				value == '-' || value == '.';
		});
	}

	bool_t IsValidDisplayText(const std::string& text, const size_t maximumLength)
	{
		if (text.empty() || text.size() > maximumLength)
			return false;

		return std::none_of(text.begin(), text.end(), [](const char value)
		{
			return 0 != std::iscntrl(static_cast<unsigned char>(value));
		});
	}

	bool_t IsValidRenderProfile(const MAP_ASSET_RENDER_PROFILE& profile)
	{
		const auto finite = [](const float value) { return std::isfinite(value); };
		return finite(profile.uvScale.x) && finite(profile.uvScale.y) &&
			profile.uvScale.x > 0.f && profile.uvScale.y > 0.f &&
			finite(profile.uvSpeed.x) && finite(profile.uvSpeed.y) &&
			finite(profile.opacity) && profile.opacity >= 0.f &&
			profile.opacity <= 1.f && finite(profile.opacityPower) &&
			profile.opacityPower >= 0.01f && profile.opacityPower <= 64.f &&
			finite(profile.emissiveIntensity) &&
			profile.emissiveIntensity >= 0.f &&
			finite(profile.specularIntensity) &&
			profile.specularIntensity >= 0.f &&
			finite(profile.specularPower) && profile.specularPower >= 1.f &&
			finite(profile.colorTint.x) && finite(profile.colorTint.y) &&
			finite(profile.colorTint.z) && finite(profile.colorTint.w) &&
			profile.colorTint.x >= 0.f && profile.colorTint.y >= 0.f &&
			profile.colorTint.z >= 0.f && profile.colorTint.w >= 0.f;
	}

	bool_t IsSafeRelativeFilename(const std::string& value,
		const std::filesystem::path& requiredExtension)
	{
		if (value.empty() || value.size() > MAX_SHARD_FILENAME_LENGTH ||
			std::string::npos != value.find('/') ||
			std::string::npos != value.find('\\'))
			return false;

		const std::filesystem::path path(value);
		return !path.empty() && !path.is_absolute() && !path.has_root_path() &&
			!path.has_parent_path() && path == path.filename() &&
			path != std::filesystem::path(".") &&
			path != std::filesystem::path("..") &&
			path.extension() == requiredExtension;
	}

	bool_t EqualRenderProfile(const MAP_ASSET_RENDER_PROFILE& lhs,
		const MAP_ASSET_RENDER_PROFILE& rhs)
	{
		return lhs.renderMode == rhs.renderMode &&
			lhs.cullMode == rhs.cullMode &&
			lhs.uvScale.x == rhs.uvScale.x &&
			lhs.uvScale.y == rhs.uvScale.y &&
			lhs.uvSpeed.x == rhs.uvSpeed.x &&
			lhs.uvSpeed.y == rhs.uvSpeed.y &&
			lhs.opacity == rhs.opacity &&
			lhs.opacityPower == rhs.opacityPower &&
			lhs.emissiveIntensity == rhs.emissiveIntensity &&
			lhs.specularIntensity == rhs.specularIntensity &&
			lhs.specularPower == rhs.specularPower &&
			lhs.colorTint.x == rhs.colorTint.x &&
			lhs.colorTint.y == rhs.colorTint.y &&
			lhs.colorTint.z == rhs.colorTint.z &&
			lhs.colorTint.w == rhs.colorTint.w;
	}

	bool_t EqualAssetEntry(const MAP_ASSET_ENTRY& lhs,
		const MAP_ASSET_ENTRY& rhs)
	{
		return lhs.id == rhs.id && lhs.label == rhs.label &&
			lhs.groupId == rhs.groupId &&
			lhs.groupLabel == rhs.groupLabel &&
			lhs.evidence == rhs.evidence &&
			lhs.modelRelativePath == rhs.modelRelativePath &&
			lhs.resolvedModelPath == rhs.resolvedModelPath &&
			lhs.prototypeTag == rhs.prototypeTag &&
			lhs.defaultScale.x == rhs.defaultScale.x &&
			lhs.defaultScale.y == rhs.defaultScale.y &&
			lhs.defaultScale.z == rhs.defaultScale.z &&
			lhs.anchor == rhs.anchor &&
			EqualRenderProfile(lhs.renderProfile, rhs.renderProfile);
	}
}

bool_t CMapAssetCatalog::Load_Default()
{
	const std::filesystem::path selectionPath = Get_AreaSelectionPath();
	std::ifstream input(selectionPath, std::ios::binary);
	std::string magic;
	std::string selectedAreaId;
	uint32_t version = {};
	if (!input || !(input >> magic >> version >> std::quoted(selectedAreaId)) ||
		magic != "LOSTARK_MAP_AREA_SELECTION" || version != 1 ||
		!IsValidGroupId(selectedAreaId))
	{
		m_Status = "Active map area selection is invalid: " +
			selectionPath.string();
		return false;
	}

	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Active map area selection has trailing data";
		return false;
	}

	return Load_Area(selectedAreaId);
}

bool_t CMapAssetCatalog::Load_Source(
	const std::filesystem::path& catalogPath,
	const std::filesystem::path& placementPath,
	const std::string& expectedAreaId)
{
	const std::filesystem::path normalizedCatalog =
		catalogPath.lexically_normal();
	const std::filesystem::path normalizedPlacements =
		placementPath.lexically_normal();
	if (normalizedCatalog.empty() || normalizedCatalog.is_relative() ||
		normalizedPlacements.empty() || normalizedPlacements.is_relative() ||
		(normalizedCatalog.extension() != L".mapassets" &&
			normalizedCatalog.extension() != L".mapset") ||
		normalizedPlacements.extension() != L".mapplacements")
	{
		m_Status = "Authoring source paths are invalid";
		return false;
	}

	m_SourceCatalogOverride = normalizedCatalog;
	m_SourcePlacementOverride = normalizedPlacements;
	return Load_Area(expectedAreaId);
}

bool_t CMapAssetCatalog::Load_Area(const std::string& areaId)
{
	if (!IsValidGroupId(areaId))
	{
		m_Status = "Map area ID is invalid: " + areaId;
		return false;
	}

	const std::string selectedAreaId = areaId;

	const bool_t hasSourceOverride =
		!m_SourceCatalogOverride.empty();
	const std::filesystem::path mapRoot = hasSourceOverride ?
		m_SourceCatalogOverride.parent_path() : Get_MapDataRoot();
	const std::filesystem::path shardSetPath =
		hasSourceOverride && m_SourceCatalogOverride.extension() == L".mapset" ?
		m_SourceCatalogOverride :
		mapRoot / (std::filesystem::path(selectedAreaId).wstring() + L".mapset");
	std::error_code shardSetError;
	const bool_t hasShardSet = hasSourceOverride ?
		m_SourceCatalogOverride.extension() == L".mapset" :
		std::filesystem::exists(shardSetPath, shardSetError);
	if (shardSetError)
	{
		m_Status = "Could not inspect map shard set: " +
			shardSetPath.string();
		return false;
	}

	if (hasShardSet)
	{
		std::ifstream shardInput(shardSetPath, std::ios::binary);
		std::string shardMagic;
		std::string shardSceneId;
		uint32_t shardVersion = {};
		uint32_t shardCount = {};
		if (!shardInput || !(shardInput >> shardMagic >> shardVersion >>
			std::quoted(shardSceneId) >> shardCount) ||
			shardMagic != SHARD_SET_MAGIC ||
			shardVersion != SHARD_SET_VERSION ||
			shardSceneId != selectedAreaId ||
			0 == shardCount || shardCount > MAX_SHARD_COUNT)
		{
			m_Status = "Map shard set header is invalid: " +
				shardSetPath.string();
			return false;
		}

		std::vector<MAP_ASSET_SHARD> stagedShards;
		stagedShards.reserve(shardCount);
		std::unordered_set<std::string> shardIds;
		std::unordered_set<std::string> catalogFilenames;
		std::unordered_set<std::string> placementFilenames;
		uint32_t declaredAssetTotal = {};
		for (uint32_t index = 0; index < shardCount; ++index)
		{
			std::string shardId;
			std::string catalogFilename;
			std::string placementFilename;
			MAP_ASSET_SHARD shard{};
			if (!(shardInput >> std::quoted(shardId) >>
				std::quoted(catalogFilename) >>
				std::quoted(placementFilename) >> shard.assetCount >>
				shard.placementCount))
			{
				m_Status = "Map shard row is truncated at index " +
					std::to_string(index);
				return false;
			}

			if (!IsValidGroupId(shardId) ||
				!IsSafeRelativeFilename(
					catalogFilename, std::filesystem::path(L".mapassets")) ||
				!IsSafeRelativeFilename(
					placementFilename, std::filesystem::path(L".mapplacements")) ||
				0 == shard.assetCount || shard.assetCount > MAX_ASSET_COUNT ||
				shard.placementCount > MAX_SHARD_PLACEMENT_COUNT ||
				!shardIds.insert(shardId).second ||
				!catalogFilenames.insert(catalogFilename).second ||
				!placementFilenames.insert(placementFilename).second)
			{
				m_Status = "Map shard row is invalid at index " +
					std::to_string(index);
				return false;
			}
			if (declaredAssetTotal > MAX_TOTAL_ASSET_COUNT - shard.assetCount)
			{
				m_Status = "Map shard set exceeds the total asset limit";
				return false;
			}
			declaredAssetTotal += shard.assetCount;

			shard.shardId = std::move(shardId);
			shard.catalogPath = (mapRoot / catalogFilename).lexically_normal();
			shard.placementPath =
				(mapRoot / placementFilename).lexically_normal();
			if (!IsInsideRoot(mapRoot, shard.catalogPath) ||
				!IsInsideRoot(mapRoot, shard.placementPath))
			{
				m_Status = "Map shard path escapes the map data root at index " +
					std::to_string(index);
				return false;
			}

			stagedShards.push_back(std::move(shard));
		}

		std::string shardTrailing;
		if (shardInput >> shardTrailing)
		{
			m_Status = "Map shard set contains unexpected trailing data";
			return false;
		}

		std::vector<MAP_ASSET_ENTRY> stagedEntries;
		std::unordered_map<std::string, size_t> stagedLookup;
		std::unordered_map<std::wstring, std::string> prototypeOwners;
		stagedEntries.reserve(MAX_TOTAL_ASSET_COUNT);
		stagedLookup.reserve(MAX_TOTAL_ASSET_COUNT);
		prototypeOwners.reserve(MAX_TOTAL_ASSET_COUNT);
		for (const MAP_ASSET_SHARD& shard : stagedShards)
		{
			CMapAssetCatalog child;
			if (!child.Load(shard.catalogPath, selectedAreaId))
			{
				m_Status = "Map shard " + shard.shardId + " failed: " +
					child.Get_Status();
				return false;
			}
			if (child.Get_Entries().size() != shard.assetCount)
			{
				m_Status = "Map shard asset count mismatch: " + shard.shardId;
				return false;
			}

			for (const MAP_ASSET_ENTRY& entry : child.Get_Entries())
			{
				const auto existing = stagedLookup.find(entry.id);
				if (existing != stagedLookup.end())
				{
					if (!EqualAssetEntry(stagedEntries[existing->second], entry))
					{
						m_Status = "Conflicting duplicate asset definition: " +
							entry.id;
						return false;
					}
					continue;
				}

				const auto prototype = prototypeOwners.find(entry.prototypeTag);
				if (prototype != prototypeOwners.end())
				{
					m_Status = "Prototype tag collision between " +
						prototype->second + " and " + entry.id;
					return false;
				}
				if (stagedEntries.size() >= MAX_TOTAL_ASSET_COUNT)
				{
					m_Status = "Map shard set exceeds the total asset limit";
					return false;
				}

				prototypeOwners.emplace(entry.prototypeTag, entry.id);
				stagedLookup.emplace(entry.id, stagedEntries.size());
				stagedEntries.push_back(entry);
			}
		}

		m_Entries = std::move(stagedEntries);
		m_EntryLookup = std::move(stagedLookup);
		m_Shards = std::move(stagedShards);
		m_AreaId = std::move(selectedAreaId);
		m_CatalogPath = shardSetPath;
		m_PlacementPath = hasSourceOverride ?
			m_SourcePlacementOverride : std::filesystem::path{};
		m_bSharded = true;
		m_bReady = true;
		m_Status = "Shard set ready: " + std::to_string(m_Shards.size()) +
			" shards / " + std::to_string(m_Entries.size()) + " assets";
		return true;
	}

	const std::filesystem::path catalogPath = hasSourceOverride ?
		m_SourceCatalogOverride :
		mapRoot / (std::filesystem::path(selectedAreaId).wstring() + L".mapassets");
	if (!Load(catalogPath, selectedAreaId))
		return false;

	m_CatalogPath = catalogPath;
	m_PlacementPath = hasSourceOverride ?
		m_SourcePlacementOverride :
		mapRoot / (std::filesystem::path(selectedAreaId).wstring() + L".mapplacements");
	return true;
}

bool_t CMapAssetCatalog::Load(const std::filesystem::path& path,
	const std::string& expectedAreaId)
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_Status = "Catalog missing: " + path.string();
		return false;
	}

	std::string magic;
	uint32_t version = {};
	uint32_t count = {};
	std::string stagedAreaId;
	if (!(input >> magic >> version >> std::quoted(stagedAreaId) >> count) ||
		magic != CATALOG_MAGIC ||
		(version < LEGACY_CATALOG_VERSION || version > CATALOG_VERSION) ||
		stagedAreaId.empty() ||
		(!expectedAreaId.empty() && stagedAreaId != expectedAreaId) ||
		0 == count || count > MAX_ASSET_COUNT)
	{
		m_Status = "Catalog header is invalid";
		return false;
	}

	std::vector<PARSED_MAP_ASSET_ROW> parsedRows;
	parsedRows.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		PARSED_MAP_ASSET_ROW row{};
		if (!(input >> std::quoted(row.id) >> std::quoted(row.label) >>
			std::quoted(row.modelPath) >> std::quoted(row.prototypeTag) >>
			row.defaultScale.x >> row.defaultScale.y >> row.defaultScale.z >>
			row.anchor))
		{
			m_Status = "Catalog row is truncated at index " + std::to_string(index);
			return false;
		}

		if (version >= METADATA_CATALOG_VERSION)
		{
			if (!(input >> std::quoted(row.groupId) >>
				std::quoted(row.groupLabel) >> std::quoted(row.evidence)))
			{
				m_Status = "Catalog metadata is truncated at index " +
					std::to_string(index);
				return false;
			}
		}
		else
		{
			row.groupId = "legacy";
			row.groupLabel = "Legacy Catalog";
			row.evidence = "catalog-v1";
		}

		if (version >= RENDER_PROFILE_CATALOG_VERSION)
		{
			if (!(input >> row.renderMode >> row.cullMode >>
				row.renderProfile.uvScale.x >> row.renderProfile.uvScale.y >>
				row.renderProfile.uvSpeed.x >> row.renderProfile.uvSpeed.y >>
				row.renderProfile.opacity >> row.renderProfile.emissiveIntensity >>
				row.renderProfile.specularIntensity >>
				row.renderProfile.specularPower >>
				row.renderProfile.colorTint.x >> row.renderProfile.colorTint.y >>
				row.renderProfile.colorTint.z >> row.renderProfile.colorTint.w))
			{
				m_Status = "Catalog render profile is truncated at index " +
					std::to_string(index);
				return false;
			}
			if (version >= CATALOG_VERSION &&
				!(input >> row.renderProfile.opacityPower))
			{
				m_Status = "Catalog opacity shaping is truncated at index " +
					std::to_string(index);
				return false;
			}
		}

		parsedRows.push_back(std::move(row));
	}

	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Catalog contains unexpected trailing data";
		return false;
	}

	const std::filesystem::path assetRoot = CRuntimeAssetRoot::Get();
	if (assetRoot.empty() || !std::filesystem::exists(assetRoot))
	{
		m_Status = "LostArk runtime asset root is missing";
		return false;
	}

	std::unordered_set<std::string> ids;
	std::unordered_set<std::wstring> prototypeTags;
	std::vector<MAP_ASSET_ENTRY> stagedEntries;
	stagedEntries.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		const PARSED_MAP_ASSET_ROW& row = parsedRows[index];
		MAP_ASSET_ENTRY entry{};
		entry.id = row.id;
		entry.label = row.label;
		entry.groupId = row.groupId;
		entry.groupLabel = row.groupLabel;
		entry.evidence = row.evidence;
		entry.defaultScale = row.defaultScale;
		entry.renderProfile = row.renderProfile;
		entry.modelRelativePath = std::filesystem::path(row.modelPath).lexically_normal();
		entry.resolvedModelPath = CRuntimeAssetRoot::Resolve(entry.modelRelativePath);
		entry.prototypeTag.assign(row.prototypeTag.begin(), row.prototypeTag.end());
		if (row.anchor == "Origin")
			entry.anchor = MAP_ASSET_ANCHOR::ORIGIN;
		else if (row.anchor == "BottomCenter")
			entry.anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
		else
		{
			m_Status = "Unknown placement anchor for " + entry.id;
			return false;
		}
		if (row.renderMode == "Opaque")
			entry.renderProfile.renderMode = MAP_ASSET_RENDER_MODE::DEFERRED;
		else if (row.renderMode == "Alpha")
			entry.renderProfile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
		else if (row.renderMode == "Sky")
			entry.renderProfile.renderMode = MAP_ASSET_RENDER_MODE::BACKGROUND;
		else if (row.renderMode == "Additive")
			entry.renderProfile.renderMode = MAP_ASSET_RENDER_MODE::ADDITIVE;
		else
		{
			m_Status = "Unknown render mode for " + entry.id;
			return false;
		}
		if (row.cullMode == "Back")
			entry.renderProfile.cullMode = MAP_ASSET_CULL_MODE::CULL_BACK;
		else if (row.cullMode == "Front")
			entry.renderProfile.cullMode = MAP_ASSET_CULL_MODE::CULL_FRONT;
		else if (row.cullMode == "None")
			entry.renderProfile.cullMode = MAP_ASSET_CULL_MODE::TWO_SIDED;
		else
		{
			m_Status = "Unknown cull mode for " + entry.id;
			return false;
		}

		if (entry.id.empty() || entry.label.empty() || entry.prototypeTag.empty() ||
			!IsValidGroupId(entry.groupId) ||
			!IsValidDisplayText(entry.groupLabel, MAX_GROUP_LABEL_LENGTH) ||
			!IsValidDisplayText(entry.evidence, MAX_EVIDENCE_LENGTH) ||
			entry.modelRelativePath.is_absolute() ||
			entry.modelRelativePath.extension() != L".wmodel" ||
			!IsValidScale(entry.defaultScale) ||
			!IsValidRenderProfile(entry.renderProfile) ||
			!ids.insert(entry.id).second ||
			!prototypeTags.insert(entry.prototypeTag).second ||
			!IsInsideRoot(assetRoot, entry.resolvedModelPath) ||
			!std::filesystem::exists(entry.resolvedModelPath))
		{
			m_Status = "Catalog validation failed for " + entry.id;
			return false;
		}

		stagedEntries.push_back(std::move(entry));
	}

	std::unordered_map<std::string, size_t> stagedLookup;
	stagedLookup.reserve(stagedEntries.size());
	for (size_t index = 0; index < stagedEntries.size(); ++index)
		stagedLookup.emplace(stagedEntries[index].id, index);

	m_Entries = std::move(stagedEntries);
	m_EntryLookup = std::move(stagedLookup);
	m_Shards.clear();
	m_AreaId = std::move(stagedAreaId);
	m_CatalogPath = path;
	m_PlacementPath = path.parent_path() /
		(std::filesystem::path(m_AreaId).wstring() + L".mapplacements");
	m_bSharded = false;
	m_bReady = true;
	m_Status = "Catalog ready (v" + std::to_string(version) + "): " +
		std::to_string(m_Entries.size());
	return true;
}

const MAP_ASSET_ENTRY* CMapAssetCatalog::Find(const std::string& assetId) const
{
	const auto iter = m_EntryLookup.find(assetId);
	return iter == m_EntryLookup.end() || iter->second >= m_Entries.size() ?
		nullptr : &m_Entries[iter->second];
}

std::filesystem::path CMapAssetCatalog::Get_MapDataRoot()
{
	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
	if (0 == length || length >= std::size(modulePath))
		return {};

	const std::filesystem::path moduleDirectory =
		std::filesystem::path(modulePath).parent_path();
	const std::filesystem::path adjacentRoot =
		moduleDirectory / L"DataFiles" / L"Map";
	if (std::filesystem::exists(adjacentRoot))
		return adjacentRoot.lexically_normal();

	const std::filesystem::path parentRoot =
		moduleDirectory.parent_path() / L"DataFiles" / L"Map";
	if (std::filesystem::exists(parentRoot))
		return parentRoot.lexically_normal();

	return adjacentRoot.lexically_normal();
}

std::filesystem::path CMapAssetCatalog::Get_AreaSelectionPath()
{
	return Get_MapAuthoringRoot() / L"Editor" / L"ACTIVE.maparea";
}

std::filesystem::path CMapAssetCatalog::Get_MapAuthoringRoot()
{
	return CProjectDataRoot::Resolve(L"Maps");
}

std::filesystem::path CMapAssetCatalog::Get_AuthoringPlacementPath(
	const std::string& areaId)
{
	if (!IsValidGroupId(areaId))
		return {};
	const std::filesystem::path areaPath(areaId);
	return Get_MapAuthoringRoot() / L"Authoring" / areaPath /
		(areaPath.wstring() + L".mapplacements");
}
