#include "MapAssetCatalog.h"

#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG";
	constexpr uint32_t CATALOG_VERSION = 1;
	constexpr uint32_t MAX_ASSET_COUNT = 512;

	std::filesystem::path GetDataFilePath(const wchar_t* pFileName)
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0 == length || length >= std::size(modulePath))
			return {};

		return (std::filesystem::path(modulePath).parent_path() /
			L"DataFiles" / L"Map" / pFileName).lexically_normal();
	}

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
}

bool_t CMapAssetCatalog::Load_Default()
{
	return Load(Get_DefaultCatalogPath());
}

bool_t CMapAssetCatalog::Load(const std::filesystem::path& path)
{
	m_Entries.clear();
	m_AreaId.clear();
	m_bReady = false;

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_Status = "Catalog missing: " + path.string();
		return false;
	}

	std::string magic;
	uint32_t version = {};
	uint32_t count = {};
	if (!(input >> magic >> version >> std::quoted(m_AreaId) >> count) ||
		magic != CATALOG_MAGIC || version != CATALOG_VERSION ||
		m_AreaId.empty() || 0 == count || count > MAX_ASSET_COUNT)
	{
		m_Status = "Catalog header is invalid";
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
	m_Entries.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		MAP_ASSET_ENTRY entry{};
		std::string modelPath;
		std::string prototypeTag;
		std::string anchor;
		if (!(input >> std::quoted(entry.id) >> std::quoted(entry.label) >>
			std::quoted(modelPath) >> std::quoted(prototypeTag) >>
			entry.defaultScale.x >> entry.defaultScale.y >> entry.defaultScale.z >> anchor))
		{
			m_Status = "Catalog row is truncated at index " + std::to_string(index);
			m_Entries.clear();
			return false;
		}

		entry.modelRelativePath = std::filesystem::path(modelPath).lexically_normal();
		entry.resolvedModelPath = CRuntimeAssetRoot::Resolve(entry.modelRelativePath);
		entry.prototypeTag.assign(prototypeTag.begin(), prototypeTag.end());
		if (anchor == "Origin")
			entry.anchor = MAP_ASSET_ANCHOR::ORIGIN;
		else if (anchor == "BottomCenter")
			entry.anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
		else
		{
			m_Status = "Unknown placement anchor for " + entry.id;
			m_Entries.clear();
			return false;
		}

		if (entry.id.empty() || entry.label.empty() || entry.prototypeTag.empty() ||
			entry.modelRelativePath.is_absolute() ||
			entry.modelRelativePath.extension() != L".wmodel" ||
			!IsValidScale(entry.defaultScale) || !ids.insert(entry.id).second ||
			!prototypeTags.insert(entry.prototypeTag).second ||
			!IsInsideRoot(assetRoot, entry.resolvedModelPath) ||
			!std::filesystem::exists(entry.resolvedModelPath))
		{
			m_Status = "Catalog validation failed for " + entry.id;
			m_Entries.clear();
			return false;
		}

		m_Entries.push_back(std::move(entry));
	}

	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Catalog contains unexpected trailing data";
		m_Entries.clear();
		return false;
	}

	m_bReady = true;
	m_Status = "Catalog ready: " + std::to_string(m_Entries.size());
	return true;
}

const MAP_ASSET_ENTRY* CMapAssetCatalog::Find(const std::string& assetId) const
{
	const auto iter = std::find_if(m_Entries.begin(), m_Entries.end(),
		[&](const MAP_ASSET_ENTRY& entry) { return entry.id == assetId; });
	return iter == m_Entries.end() ? nullptr : &*iter;
}

std::filesystem::path CMapAssetCatalog::Get_DefaultCatalogPath()
{
	return GetDataFilePath(L"BG_RAD_VALTAN_A.mapassets");
}

std::filesystem::path CMapAssetCatalog::Get_DefaultPlacementPath()
{
	return GetDataFilePath(L"BG_RAD_VALTAN_A.mapplacements");
}
