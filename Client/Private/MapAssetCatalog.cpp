#include "MapAssetCatalog.h"

#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG";
	constexpr uint32_t LEGACY_CATALOG_VERSION = 1;
	constexpr uint32_t METADATA_CATALOG_VERSION = 2;
	constexpr uint32_t RENDER_PROFILE_CATALOG_VERSION = 3;
	constexpr uint32_t CATALOG_VERSION = 4;
	constexpr uint32_t MAX_ASSET_COUNT = 512;
	constexpr size_t MAX_GROUP_ID_LENGTH = 64;
	constexpr size_t MAX_GROUP_LABEL_LENGTH = 128;
	constexpr size_t MAX_EVIDENCE_LENGTH = 512;

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

	const std::filesystem::path mapRoot = Get_MapDataRoot();
	const std::filesystem::path catalogPath = mapRoot /
		(std::filesystem::path(selectedAreaId).wstring() + L".mapassets");
	if (!Load(catalogPath, selectedAreaId))
		return false;

	m_CatalogPath = catalogPath;
	m_PlacementPath = mapRoot /
		(std::filesystem::path(selectedAreaId).wstring() + L".mapplacements");
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

	m_Entries = std::move(stagedEntries);
	m_AreaId = std::move(stagedAreaId);
	m_CatalogPath = path;
	m_PlacementPath = path.parent_path() /
		(std::filesystem::path(m_AreaId).wstring() + L".mapplacements");
	m_bReady = true;
	m_Status = "Catalog ready (v" + std::to_string(version) + "): " +
		std::to_string(m_Entries.size());
	return true;
}

const MAP_ASSET_ENTRY* CMapAssetCatalog::Find(const std::string& assetId) const
{
	const auto iter = std::find_if(m_Entries.begin(), m_Entries.end(),
		[&](const MAP_ASSET_ENTRY& entry) { return entry.id == assetId; });
	return iter == m_Entries.end() ? nullptr : &*iter;
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
	return Get_MapDataRoot() / L"ACTIVE.maparea";
}
