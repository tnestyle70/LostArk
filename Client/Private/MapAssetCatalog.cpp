#include "MapAssetCatalog.h"

#include "DataJson.h"
#include "SourceCharacterMaterialParameters.h"
#include "RuntimeAssetRoot.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG";
	constexpr uint32_t LEGACY_CATALOG_VERSION = 1;
	constexpr uint32_t METADATA_CATALOG_VERSION = 2;
	constexpr uint32_t RENDER_PROFILE_CATALOG_VERSION = 3;
	constexpr uint32_t OPACITY_SHAPING_CATALOG_VERSION = 4;
	constexpr uint32_t MATERIAL_REFERENCE_CATALOG_VERSION = 5;
	constexpr uint32_t CATALOG_VERSION = MATERIAL_REFERENCE_CATALOG_VERSION;
	constexpr uint32_t MAX_ASSET_COUNT = 2048;
	constexpr const char* SHARD_SET_MAGIC = "LOSTARK_MAP_SHARD_SET";
	constexpr uint32_t SHARD_SET_VERSION = 1;
	constexpr uint32_t MAX_SHARD_COUNT = 64;
	constexpr uint32_t MAX_TOTAL_ASSET_COUNT = 32768;
	constexpr uint32_t MAX_SHARD_PLACEMENT_COUNT = 65536;
	constexpr size_t MAX_GROUP_ID_LENGTH = 64;
	constexpr size_t MAX_GROUP_LABEL_LENGTH = 128;
	constexpr size_t MAX_EVIDENCE_LENGTH = 512;
	constexpr size_t MAX_SHARD_FILENAME_LENGTH = 260;
	/* One landscape component spans 39.68 world units and its baked atlas
	   maps to that footprint, so world height must advance the substituted
	   texture axis at the same rate the authored UV advances across it. */
	constexpr float LANDSCAPE_TRIPLANAR_HEIGHT_SCALE = 1.f / 39.68f;

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
			finite(profile.triplanarHeightScale) &&
			profile.triplanarHeightScale >= 0.f &&
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
	const std::string& expectedAreaId,
	const std::filesystem::path& materialsPath)
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

	CMapAssetCatalog staged;
	staged.m_SourceCatalogOverride = normalizedCatalog;
	staged.m_SourcePlacementOverride = normalizedPlacements;
	staged.m_SourceMaterialOverride = materialsPath.lexically_normal();
	if (!staged.Load_AreaStaged(expectedAreaId) ||
		!staged.Resolve_MaterialDocumentPath() ||
		!staged.Load_MaterialOverrides())
	{
		m_Status = staged.Get_Status();
		return false;
	}

	const std::wstring authoringNamespace =
		L"MapEditorArea:" +
		std::wstring(expectedAreaId.begin(), expectedAreaId.end()) + L":";
	for (MAP_ASSET_ENTRY& entry : staged.m_Entries)
		entry.prototypeTag = authoringNamespace + entry.prototypeTag;
	*this = std::move(staged);
	return true;
}

bool_t CMapAssetCatalog::Load_Area(const std::string& areaId)
{
	CMapAssetCatalog staged;
	if (!staged.Load_AreaStaged(areaId) ||
		!staged.Resolve_MaterialDocumentPath() ||
		!staged.Load_MaterialOverrides())
	{
		m_Status = staged.Get_Status();
		return false;
	}
	*this = std::move(staged);
	return true;
}

bool_t CMapAssetCatalog::Resolve_MaterialDocumentPath()
{
	m_MaterialDocumentPath.clear();
	if (!m_SourceCatalogOverride.empty())
	{
		const std::filesystem::path expected =
			(Get_MapAuthoringRoot() / L"Authoring" / m_AreaId /
				(m_AreaId + ".mapmaterials.json")).lexically_normal();
		if (m_SourceMaterialOverride.empty())
		{
			std::error_code error;
			if (!m_MaterialDocumentFilename.empty() ||
				std::filesystem::exists(expected, error) || error)
			{
				m_Status = "Map material authoring source has no explicit declaration: " +
					expected.string();
				return false;
			}
			return true;
		}
		if (m_SourceMaterialOverride.is_relative() ||
			m_SourceMaterialOverride != expected)
		{
			m_Status = "Map material authoring path is not canonical: " +
				m_SourceMaterialOverride.string();
			return false;
		}
		m_MaterialDocumentPath = m_SourceMaterialOverride;
	}
	else if (!m_MaterialDocumentFilename.empty())
	{
		m_MaterialDocumentPath =
			m_CatalogPath.parent_path() / m_MaterialDocumentFilename;
	}
	if (m_MaterialDocumentPath.empty())
		return true;
	std::error_code error;
	if (!std::filesystem::is_regular_file(m_MaterialDocumentPath, error) || error)
	{
		m_Status = "Declared map material document is missing: " +
			m_MaterialDocumentPath.string();
		return false;
	}
	return true;
}

bool_t CMapAssetCatalog::Load_MaterialOverrides()
{
	if (m_MaterialDocumentPath.empty())
		return true;
	std::ifstream input(m_MaterialDocumentPath, std::ios::binary);
	const std::string text((std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());
	DATA_JSON_VALUE root;
	std::string error;
    DATA_JSON_PARSE_LIMITS materialLimits;
    // Per-component RNM/SDF variants are bounded by the existing material and
    // placement row limits. Their measured Bern document exceeds the general
    // 16 MB/1M-value JSON default; unrelated domain limits remain unchanged.
    materialLimits.iMaximumBytes = 128u * 1024u * 1024u;
    materialLimits.iMaximumValues = 8'000'000u;
	if (!input || !CDataJson::Parse(text, root, error, materialLimits) || !root.Is_Object())
	{
		m_Status = "Map material document parse failed: " + error;
		return false;
	}
	const auto exactFields = [](const DATA_JSON_VALUE& value,
		const std::unordered_set<std::string>& fields)
	{
		if (!value.Is_Object() || value.Get_Object().size() != fields.size())
			return false;
		return std::all_of(value.Get_Object().begin(), value.Get_Object().end(),
			[&fields](const auto& pair) { return fields.contains(pair.first); });
	};
	const auto readString = [](const DATA_JSON_VALUE& value, const char* key,
		std::string& result)
	{
		const auto* item = value.Find(key);
		if (!item || !item->Is_String() || item->Get_String().empty())
			return false;
		result = item->Get_String();
		return true;
	};
	const auto readNumber = [](const DATA_JSON_VALUE& value, const char* key,
		float& result)
	{
		const auto* item = value.Find(key);
		if (!item || !item->Is_Number() || !std::isfinite(item->Get_Number()) ||
			item->Get_Number() < 0.0 ||
			item->Get_Number() > (std::numeric_limits<float>::max)())
			return false;
		result = static_cast<float>(item->Get_Number());
		return std::isfinite(result);
	};
	const auto readColor = [](const DATA_JSON_VALUE& value, const char* key,
		float4_t& result)
	{
		const auto* item = value.Find(key);
		if (!item || !item->Is_Array() || item->Get_Array().size() != 4u)
			return false;
		float components[4]{};
		for (size_t index = 0; index < 4u; ++index)
		{
			const auto& component = item->Get_Array()[index];
			if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
				component.Get_Number() < 0.0 ||
				component.Get_Number() > (std::numeric_limits<float>::max)())
				return false;
			components[index] = static_cast<float>(component.Get_Number());
			if (!std::isfinite(components[index]))
				return false;
		}
		result = float4_t(components[0], components[1], components[2], components[3]);
		return true;
	};
	std::string schema;
	std::string areaId;
	const auto* version = root.Find("formatVersion");
	const auto* rows = root.Find("materials");
    const auto* placementRows = root.Find("placementLighting");
    std::unordered_set<std::string> rootFields = { "schema", "formatVersion", "areaId", "materials" };
    if (placementRows) rootFields.insert("placementLighting");
	if (!exactFields(root, rootFields) ||
        (placementRows && (!version || !version->Is_Number() || version->Get_Number() != 2.0 || !placementRows->Is_Array())) ||
		!readString(root, "schema", schema) || schema != "lostark.map-materials" ||
		!version || !version->Is_Number() || (version->Get_Number() != 1.0 && version->Get_Number() != 2.0) ||
		!readString(root, "areaId", areaId) || areaId != m_AreaId ||
		!rows || !rows->Is_Array() || rows->Get_Array().empty() ||
		rows->Get_Array().size() > MAX_TOTAL_ASSET_COUNT)
	{
		m_Status = "Map material document header is invalid";
		return false;
	}
	std::unordered_map<std::string, std::vector<Engine::MODEL_MATERIAL_OVERRIDE>> staged;
	std::unordered_set<std::string> keys;
    std::unordered_map<std::string, std::pair<Engine::MODEL_SURFACE_RENDER_MODE, Engine::MODEL_SURFACE_CULL_MODE>> drawPolicies;
    for (const auto& sourceRow : rows->Get_Array())
    {
        if (!sourceRow.Is_Object()) { m_Status = "Map material row must be an object"; return false; }
        auto materialFields = sourceRow.Get_Object();
        materialFields.erase("renderMode"); materialFields.erase("cullMode");
        const auto* sourceBaked = sourceRow.Find("bakedLighting");
        const auto* sourceShadow = sourceBaked && sourceBaked->Is_Object() ? sourceBaked->Find("staticShadow") : nullptr;
        if (sourceShadow)
        {
            auto bakedFields = sourceBaked->Get_Object(); bakedFields.erase("staticShadow");
            materialFields["bakedLighting"] = DATA_JSON_VALUE::Object(std::move(bakedFields));
        }

        const auto row = DATA_JSON_VALUE::Object(std::move(materialFields));
		std::string assetId, family, sourceMaterial, reflectionTexture;
		Engine::MODEL_MATERIAL_OVERRIDE material;
		if (!readString(row, "assetId", assetId) ||
			!readString(row, "materialName", material.materialName) ||
			!readString(row, "family", family) ||
			!readString(row, "sourceMaterial", sourceMaterial) ||
			!IsValidDisplayText(material.materialName, 63u) ||
			!IsValidDisplayText(sourceMaterial, MAX_EVIDENCE_LENGTH) ||
			!Find(assetId) || !keys.insert(assetId + "\n" + material.materialName).second)
		{
			m_Status = "Map material identity is invalid, duplicate, or unknown: " + assetId;
			return false;
		}
        if (sourceShadow)
        {
            std::string texture, lightGuid, basis;
            float width = 0.f, exponent = 0.f, channel = 0.f;
            if (!exactFields(*sourceShadow, { "texture", "lightGuid", "penumbraWidth", "penumbraBasis", "shadowExponent", "lightChannel" }) ||
                !readString(*sourceShadow, "texture", texture) || !readString(*sourceShadow, "lightGuid", lightGuid) ||
                !readString(*sourceShadow, "penumbraBasis", basis) || basis != "PROJECT_ADAPTER" ||
                !readNumber(*sourceShadow, "penumbraWidth", width) || width <= 0.f || width > 1.f || !std::isfinite(1.f / width) ||
                !readNumber(*sourceShadow, "shadowExponent", exponent) || exponent <= 0.f || exponent > 128.f ||
                !readNumber(*sourceShadow, "lightChannel", channel) || channel < 1.f || channel > 15.f || floorf(channel) != channel ||
                lightGuid.size() != 32u || !std::all_of(lightGuid.begin(), lightGuid.end(), [](char c) {
                    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }))
            { m_Status = "Invalid static shadow transfer or source GUID: " + assetId; return false; }
            const std::filesystem::path relative(texture);
            std::error_code shadowError;
            material.staticShadowPath = CRuntimeAssetRoot::Resolve(relative);
            if (relative.is_absolute() || relative.has_root_path() || texture.find(':') != std::string::npos ||
                relative.extension() != L".dds" || material.staticShadowPath.empty() ||
                !IsInsideRoot(CRuntimeAssetRoot::Get(), material.staticShadowPath) ||
                !std::filesystem::is_regular_file(material.staticShadowPath, shadowError) || shadowError)
            { m_Status = "Invalid static shadow texture: " + assetId; return false; }
            material.surface.hasStaticShadow = true;
            material.surface.staticShadowChannel = static_cast<uint32_t>(channel);
            // Source PS consumes this bias/scale/exponent form. Width and center
            // are explicitly PROJECT_ADAPTER while source CPU setup is unknown.
            material.surface.staticShadowTransfer = float4_t(width * 0.5f - 0.5f, 1.f / width, exponent, 0.f);
        }
        auto renderMode = Engine::MODEL_SURFACE_RENDER_MODE::INHERIT;
        auto cullMode = Engine::MODEL_SURFACE_CULL_MODE::INHERIT;
        if (const auto* mode = sourceRow.Find("renderMode"))
        {
            if (version->Get_Number() != 2.0 || !mode->Is_String() || family == "diffuse-sampler")
            { m_Status = "Invalid per-material render mode: " + assetId; return false; }
            const auto& name = mode->Get_String();
            if (name == "deferred") renderMode = Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED;
            else if (name == "translucent") renderMode = Engine::MODEL_SURFACE_RENDER_MODE::TRANSLUCENT;
            else if (name == "background") renderMode = Engine::MODEL_SURFACE_RENDER_MODE::BACKGROUND;
            else if (name == "additive") renderMode = Engine::MODEL_SURFACE_RENDER_MODE::ADDITIVE;
            else if (name == "water") renderMode = Engine::MODEL_SURFACE_RENDER_MODE::WATER;
            else { m_Status = "Unknown per-material render mode: " + name; return false; }
        }
        if (const auto* mode = sourceRow.Find("cullMode"))
        {
            if (version->Get_Number() != 2.0 || !mode->Is_String() || family == "diffuse-sampler")
            { m_Status = "Invalid per-material cull mode: " + assetId; return false; }
            const auto& name = mode->Get_String();
            if (name == "back") cullMode = Engine::MODEL_SURFACE_CULL_MODE::CULL_BACK;
            else if (name == "front") cullMode = Engine::MODEL_SURFACE_CULL_MODE::CULL_FRONT;
            else if (name == "none") cullMode = Engine::MODEL_SURFACE_CULL_MODE::TWO_SIDED;
            else { m_Status = "Unknown per-material cull mode: " + name; return false; }
        }
        drawPolicies.emplace(assetId + "\n" + material.materialName, std::make_pair(renderMode, cullMode));
		if (family == "diffuse-sampler")
		{
			std::string addressU, sourceTexture;
			if (!exactFields(row, { "assetId", "materialName", "sourceMaterial", "family", "sourceTexture", "addressU" }) ||
				!readString(row, "addressU", addressU) || (addressU != "WRAP" && addressU != "MIRROR") ||
				!readString(row, "sourceTexture", sourceTexture) || !IsValidDisplayText(sourceTexture, MAX_EVIDENCE_LENGTH))
			{ m_Status = "Invalid map diffuse sampler: " + assetId + "/" + material.materialName; return false; }
			material.hasDiffuseAddressU = true;
			material.diffuseMirrorU = addressU == "MIRROR";
			staged[assetId].push_back(std::move(material));
			continue;
		}
        if (family.starts_with("source."))
        {
            const auto reject = [&](const char* reason) {
                m_Status = "Map native material " + assetId + "/" + material.materialName + ": " + reason;
                return false;
            };
            const auto* parameters = row.Find("parameters");
            const auto* textures = row.Find("textures");
            SourceCharacterMaterial::PARAMETER_VALUES values;
            std::unordered_set<std::string> nativeFields = { "assetId", "materialName", "sourceMaterial", "family", "parameters", "textures" };
            if (row.Find("bakedLighting")) nativeFields.insert("bakedLighting");
            if (version->Get_Number() != 2.0 || !exactFields(row, nativeFields) ||
                !parameters || !textures || !textures->Is_Array() ||
                !SourceCharacterMaterial::Read(*parameters, values) ||
                !SourceCharacterMaterial::Configure(family, values, material.surface.sourceCharacter)) return reject("invalid native program inputs");
            material.surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
            const uint32_t required = material.surface.sourceCharacter.baseTextureMask | material.surface.sourceCharacter.lightTextureMask;
            const bool texturelessHelper = family == "source.map.black.v1" || family == "source.map.shadow-modulate.v1";
            if (texturelessHelper != (required == 0u) || (texturelessHelper && !textures->Get_Array().empty()))
                return reject("invalid textureless native helper");
            uint32_t supplied = 0u;
            for (const auto& texture : textures->Get_Array())
            {
                const auto* index = texture.Find("expressionIndex");
                std::string asset, space;
                if (!exactFields(texture, { "expressionIndex", "assetId", "colorSpace" }) ||
                    !index || !index->Is_Number() || index->Get_Number() < 0.0 ||
                    index->Get_Number() >= Engine::SOURCE_CHARACTER_TEXTURE_COUNT || std::floor(index->Get_Number()) != index->Get_Number() ||
                    !readString(texture, "assetId", asset) || !readString(texture, "colorSpace", space) ||
                    (space != "linear" && space != "srgb")) return reject("invalid native texture entry");
                const uint32_t slot = static_cast<uint32_t>(index->Get_Number());
                if ((supplied & (1u << slot)) != 0u || (required & (1u << slot)) == 0u) return reject("duplicate or unused native texture");
                const std::filesystem::path relative(asset);
                const auto path = CRuntimeAssetRoot::Resolve(relative);
                std::error_code ec;
                if (relative.is_absolute() || relative.has_root_path() || asset.find(':') != std::string::npos ||
                    relative.extension() != L".dds" || path.empty() || !IsInsideRoot(CRuntimeAssetRoot::Get(), path) ||
                    !std::filesystem::is_regular_file(path, ec) || ec) return reject("missing or unsafe native texture");
                material.sourceCharacterTextures[slot].path = path;
                material.sourceCharacterTextures[slot].srgb = space == "srgb";
                supplied |= 1u << slot;
            }
            if (supplied != required) return reject("missing selected native texture");
            if (const auto* baked = row.Find("bakedLighting"))
            {
                const auto program = material.surface.sourceCharacter.program;
                std::string space;
                const bool supportsBaked = (program >= 80u && program <= 83u) ||
                    (program >= 40u && program <= 63u && program != 47u && program != 53u && program != 55u);
                if (!supportsBaked ||
                    !exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) ||
                    !readString(*baked, "colorSpace", space) || (space != "linear" && space != "srgb"))
                    return reject("unsupported native baked lighting");
                const auto texture = [&](const char* key, std::filesystem::path& output) {
                    std::string value;
                    if (!readString(*baked, key, value)) return false;
                    const std::filesystem::path relative(value);
                    std::error_code ec;
                    output = CRuntimeAssetRoot::Resolve(relative);
                    return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos &&
                        relative.extension() == L".dds" && !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) &&
                        std::filesystem::is_regular_file(output, ec) && !ec;
                };
                if (!texture("averageTexture", material.bakedAveragePath) ||
                    !texture("directionalTexture", material.bakedDirectionalPath)) return reject("invalid native lightmap texture");
                material.surface.hasBakedLighting = true;
                material.surface.bakedLightingSRGB = space == "srgb";
            }
            staged[assetId].push_back(std::move(material));
            continue;
        }
        if (family == "bg_base_opa_overlay")
        {
            auto& surface = material.surface;
            surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE;
            const auto reject = [&](const char* reason) {
                m_Status = "Map source overlay " + assetId + "/" + material.materialName + ": " + reason;
                return false;
            };
            std::unordered_set<std::string> fields = {
                "assetId", "materialName", "sourceMaterial", "family", "textureColorSpace",
                "diffuseBrightness", "diffuseSaturation", "normalIntensity", "specularIntensity", "specularPower",
                "diffuseColor", "specularColor", "overlayColor", "overlayTiling", "overlayNormalIntensity",
                "overlaySharpness", "overlayBrightness", "overlaySaturation", "overlaySpecularIntensity",
                "castsShadow", "diffuseTexture", "normalTexture", "overlayDiffuseTexture", "overlayNormalTexture"
            };
            if (const auto* flags = row.Find("sourceOverlayFlags"))
            {
                if (!flags->Is_Number() || flags->Get_Number() < 0 || flags->Get_Number() > 511 ||
                    std::floor(flags->Get_Number()) != flags->Get_Number()) return reject("invalid overlay flags");
                surface.sourceOverlayFlags = static_cast<uint32_t>(flags->Get_Number());
                fields.insert("sourceOverlayFlags");
                if ((surface.sourceOverlayFlags & 1u) == 0u) fields.erase("normalTexture");
                if ((surface.sourceOverlayFlags & 2u) == 0u) fields.erase("overlayNormalTexture");
            }
            for (const char* optional : { "uvTiling", "specularTexture", "bakedLighting", "sourceDirection", "sourceUV", "detailNormalTexture", "detailNormalIntensity", "detailNormalTiling" })
                if (row.Find(optional)) fields.insert(optional);
            if (version->Get_Number() != 2.0 || !exactFields(row, fields)) return reject("invalid fields or version");
            const std::pair<const char*, float*> scalars[] = {
                { "diffuseBrightness", &surface.diffuseBrightness }, { "diffuseSaturation", &surface.diffuseSaturation },
                { "normalIntensity", &surface.normalIntensity }, { "specularIntensity", &surface.specularIntensity },
                { "specularPower", &surface.specularPower }, { "overlayTiling", &surface.overlayTiling },
                { "overlayNormalIntensity", &surface.overlayNormalIntensity }, { "overlaySharpness", &surface.overlaySharpness },
                { "overlayBrightness", &surface.overlayBrightness }, { "overlaySaturation", &surface.overlaySaturation },
                { "overlaySpecularIntensity", &surface.overlaySpecularIntensity }
            };
            for (const auto& value : scalars)
                if (!readNumber(row, value.first, *value.second)) return reject(value.first);
            if (surface.specularPower < 0.f || surface.overlayTiling <= 0.f ||
                !readColor(row, "diffuseColor", surface.diffuseColor) ||
                !readColor(row, "specularColor", surface.specularColor) ||
                !readColor(row, "overlayColor", surface.overlayColor)) return reject("invalid surface values");
            const auto* shadow = row.Find("castsShadow");
            if (!shadow || !shadow->Is_Boolean()) return reject("invalid castsShadow");
            surface.castsShadow = shadow->Get_Boolean();
            const auto* spaces = row.Find("textureColorSpace");
            std::unordered_set<std::string> colorFields = { "diffuse", "normal", "overlayDiffuse", "overlayNormal" };
            if (row.Find("specularTexture")) colorFields.insert("specular");
            if (!spaces || !exactFields(*spaces, colorFields))
                return reject("invalid color spaces");
            for (const char* key : { "diffuse", "normal", "overlayDiffuse", "overlayNormal" })
            {
                std::string value;
                if (!readString(*spaces, key, value) || (value != "srgb" && value != "linear")) return reject(key);
                const std::string slot(key);
                if ((slot == "normal" || slot == "overlayNormal") && value != "linear") return reject("normal must be linear");
                if (slot == "diffuse") surface.diffuseSRGB = value == "srgb";
                if (slot == "overlayDiffuse") surface.overlaySRGB = value == "srgb";
            }
            const auto texture = [&](const DATA_JSON_VALUE& object, const char* key, std::filesystem::path& output) {
                std::string value;
                if (!readString(object, key, value)) return false;
                const std::filesystem::path relative(value);
                std::error_code ec;
                output = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos &&
                    relative.extension() == L".dds" && !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) &&
                    std::filesystem::is_regular_file(output, ec) && !ec;
            };
            if (!texture(row, "diffuseTexture", material.surfaceDiffusePath) ||
                ((surface.sourceOverlayFlags & 1u) != 0u && !texture(row, "normalTexture", material.surfaceNormalPath)) ||
                !texture(row, "overlayDiffuseTexture", material.overlayDiffusePath) ||
                ((surface.sourceOverlayFlags & 2u) != 0u && !texture(row, "overlayNormalTexture", material.overlayNormalPath))) return reject("missing or invalid texture");
            for (const auto& entry : { std::pair<const char*, float4_t*>{ "sourceDirection", &surface.sourceOverlayDirection },
                { "sourceUV", &surface.sourceBgUV } })
            {
                const auto* vector = row.Find(entry.first);
                if (!vector) continue;
                if (!vector->Is_Array() || vector->Get_Array().size() != 4u) return reject("invalid overlay vector");
                float* out = &entry.second->x;
                for (size_t i = 0; i < 4u; ++i) {
                    const auto& c = vector->Get_Array()[i];
                    if (!c.Is_Number() || !std::isfinite(c.Get_Number()) || std::abs(c.Get_Number()) > 1000000.0) return reject("invalid overlay vector component");
                    out[i] = static_cast<float>(c.Get_Number());
                }
            }
            const bool hasDetail = (surface.sourceOverlayFlags & 32u) != 0u;
            if (hasDetail != (row.Find("detailNormalTexture") != nullptr) ||
                hasDetail != (row.Find("detailNormalIntensity") != nullptr) ||
                hasDetail != (row.Find("detailNormalTiling") != nullptr)) return reject("inconsistent overlay detail branch");
            if (hasDetail &&
                (!texture(row, "detailNormalTexture", material.detailNormalPath) ||
                 !readNumber(row, "detailNormalIntensity", surface.detailNormalIntensity) ||
                 !readNumber(row, "detailNormalTiling", surface.detailNormalTiling))) return reject("invalid overlay detail normal");
            if (const auto* baked = row.Find("bakedLighting"))
            {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) ||
                    !readString(*baked, "colorSpace", space) || (space != "linear" && space != "srgb") ||
                    !texture(*baked, "averageTexture", material.bakedAveragePath) ||
                    !texture(*baked, "directionalTexture", material.bakedDirectionalPath)) return reject("invalid baked lighting");
                surface.hasBakedLighting = true;
                surface.bakedLightingSRGB = space == "srgb";
            }
            if (const auto* uv = row.Find("uvTiling"))
            {
                if (!uv->Is_Array() || uv->Get_Array().size() != 2u) return reject("invalid overlay UV tiling");
                float components[2]{};
                for (size_t i = 0; i < 2u; ++i) {
                    const auto& c = uv->Get_Array()[i];
                    if (!c.Is_Number() || !std::isfinite(c.Get_Number()) || c.Get_Number() <= 0.0 ||
                        c.Get_Number() > (std::numeric_limits<float>::max)()) return reject("invalid overlay UV component");
                    components[i] = static_cast<float>(c.Get_Number());
                }
                surface.uvTiling = float2_t(components[0], components[1]);
            }
            if (row.Find("specularTexture"))
            {
                std::string color;
                if (!readString(*spaces, "specular", color) || (color != "srgb" && color != "linear") ||
                    !texture(row, "specularTexture", material.surfaceSpecularPath)) return reject("invalid overlay specular");
                surface.overlaySeparateSpecular = true;
                surface.specularSRGB = color == "srgb";
            }
            staged[assetId].push_back(std::move(material));
            continue;
        }
        if (family == "bg-source-snowice-opaque" || family == "bg-source-vertexblend-opaque" || family == "bg-source-wet-opaque")
        {
            auto& surface = material.surface;
            auto& special = surface.sourceSpecial;
            const bool ice = family == "bg-source-snowice-opaque";
            const bool blend = family == "bg-source-vertexblend-opaque";
            surface.family = ice ? MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE :
                blend ? MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE : MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE;
            const auto reject = [&](const char* reason) { m_Status = "Invalid source special material: " + assetId + "/" + material.materialName + " / " + reason; return false; };
            std::unordered_set<std::string> fields = { "assetId", "materialName", "sourceMaterial", "family", "textureColorSpace", "castsShadow",
                "diffuseColor", "specularColor", "reflectionColor", "normalIntensity", "diffuseBrightness", "specularIntensity", "specularPower",
                "uvTiling", "detailNormalIntensity", "detailNormalTiling", "diffuseTexture", "normalTexture" };
            if (ice || blend)
            {
                const auto* flags = row.Find("sourceSpecialFlags");
                if (!flags || !flags->Is_Number() || flags->Get_Number() < 0 || flags->Get_Number() > 3 ||
                    std::floor(flags->Get_Number()) != flags->Get_Number()) return reject("flags");
                special.flags = static_cast<uint32_t>(flags->Get_Number());
                fields.insert("sourceSpecialFlags");
            }
            if (ice) for (const char* key : { "sourceNormalTiling", "sourceIceCoreColor", "sourceIceOuterColor", "sourceIceBlend", "sourceIceBumpOffset" }) fields.insert(key);
            else if (blend) for (const char* key : { "sourceBlendDiffuse", "sourceBlendSpecular", "sourceBlendLayers", "sourceBlendSharpness", "sourceRimlight" }) fields.insert(key);
            else for (const char* key : { "sourceNormalTiling", "sourceWetParameters", "sourceWetSpecularPower" }) fields.insert(key);
            struct SpecialTexture { const char* key; const char* space; std::filesystem::path* path; bool_t* srgb; bool required; };
            const SpecialTexture textures[] = {
                { "diffuseTexture", "diffuse", &material.surfaceDiffusePath, &surface.diffuseSRGB, true },
                { "normalTexture", "normal", &material.surfaceNormalPath, nullptr, true },
                { "specularTexture", "specular", &material.surfaceSpecularPath, &surface.specularSRGB, !blend && (!ice || (special.flags & 2u)) },
                { "reflectionTexture", "reflection", &material.reflectionPath, &surface.reflectionSRGB, !blend },
                { "detailNormalTexture", "detailNormal", &material.detailNormalPath, nullptr, blend || (ice && (special.flags & 1u)) },
                { "specialMaskTexture", "specialMask", &material.sourceSpecialMaskPath, &special.maskSRGB, ice },
                { "overlayDiffuseTexture", "overlay", &material.overlayDiffusePath, &surface.overlaySRGB, blend },
                { "overlayNormalTexture", "overlayNormal", &material.overlayNormalPath, nullptr, blend },
                { "blendDiffuseGTexture", "blendG", &material.sourceBlendDiffuseGPath, &special.blendGSRGB, blend && (special.flags & 1u) },
                { "blendNormalGTexture", "blendGNormal", &material.sourceBlendNormalGPath, nullptr, blend && (special.flags & 1u) },
                { "blendDiffuseBTexture", "blendB", &material.sourceBlendDiffuseBPath, &special.blendBSRGB, blend && (special.flags & 2u) },
                { "blendNormalBTexture", "blendBNormal", &material.sourceBlendNormalBPath, nullptr, blend && (special.flags & 2u) }
            };
            std::unordered_set<std::string> spaceFields;
            for (const auto& texture : textures) if (texture.required) { fields.insert(texture.key); spaceFields.insert(texture.space); }
            if (row.Find("bakedLighting")) fields.insert("bakedLighting");
            if (version->Get_Number() != 2.0 || !exactFields(row, fields)) return reject("fields or version");
            const auto* spaces = row.Find("textureColorSpace");
            if (!spaces || !exactFields(*spaces, spaceFields)) return reject("color space fields");
            const auto texturePath = [&](const DATA_JSON_VALUE& owner, const char* key, std::filesystem::path& output) {
                std::string name; if (!readString(owner, key, name)) return false;
                const std::filesystem::path relative(name); std::error_code ec;
                output = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && name.find(':') == std::string::npos &&
                    relative.extension() == L".dds" && !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) &&
                    std::filesystem::is_regular_file(output, ec) && !ec;
            };
            for (const auto& texture : textures)
            {
                if (!texture.required) continue;
                std::string color;
                if (!texturePath(row, texture.key, *texture.path) || !readString(*spaces, texture.space, color) ||
                    (color != "srgb" && color != "linear") || (!texture.srgb && color != "linear")) return reject(texture.key);
                if (texture.srgb) *texture.srgb = color == "srgb";
            }
            for (const auto& pair : { std::pair<const char*, float*>{ "normalIntensity", &surface.normalIntensity },
                { "diffuseBrightness", &surface.diffuseBrightness }, { "specularIntensity", &surface.specularIntensity },
                { "specularPower", &surface.specularPower }, { "detailNormalIntensity", &surface.detailNormalIntensity },
                { "detailNormalTiling", &surface.detailNormalTiling } })
                if (!readNumber(row, pair.first, *pair.second)) return reject(pair.first);
            for (const auto& pair : { std::pair<const char*, float4_t*>{ "diffuseColor", &surface.diffuseColor },
                { "specularColor", &surface.specularColor }, { "reflectionColor", &surface.reflectionColor } })
                if (!readColor(row, pair.first, *pair.second)) return reject(pair.first);
            const auto* uv = row.Find("uvTiling");
            if (!uv || !uv->Is_Array() || uv->Get_Array().size() != 2u) return reject("UV tiling");
            for (size_t i = 0; i < 2u; ++i) {
                const auto& value = uv->Get_Array()[i];
                if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number() <= 0.0 || value.Get_Number() > 1000000.0) return reject("UV value");
                (&surface.uvTiling.x)[i] = static_cast<float>(value.Get_Number());
            }
            const auto* casts = row.Find("castsShadow");
            if (!casts || !casts->Is_Boolean() || surface.detailNormalTiling <= 0.f) return reject("shadow or detail UV");
            surface.castsShadow = casts->Get_Boolean();
            if (!blend && (!readNumber(row, "sourceNormalTiling", special.normalTiling) || special.normalTiling <= 0.f)) return reject("normal tiling");
            if (ice)
            {
                if (!readColor(row, "sourceIceCoreColor", special.iceCoreColor) || !readColor(row, "sourceIceOuterColor", special.iceOuterColor) ||
                    !readColor(row, "sourceIceBlend", special.iceBlend)) return reject("ice vectors");
                const auto* offset = row.Find("sourceIceBumpOffset");
                if (!offset || !offset->Is_Number() || !std::isfinite(offset->Get_Number()) || std::abs(offset->Get_Number()) > 1000000.0) return reject("ice offset");
                special.iceBumpOffset = static_cast<float>(offset->Get_Number());
            }
            else if (blend)
            {
                if (!readNumber(row, "sourceBlendSharpness", special.blendSharpness) || !readColor(row, "sourceRimlight", surface.sourceBgRimlight)) return reject("blend sharpness or rim");
                for (const auto& pair : { std::pair<const char*, std::array<float4_t, 4>*>{ "sourceBlendDiffuse", &special.blendDiffuse },
                    { "sourceBlendSpecular", &special.blendSpecular }, { "sourceBlendLayers", &special.blendLayers } })
                {
                    const auto* layers = row.Find(pair.first);
                    if (!layers || !layers->Is_Array() || layers->Get_Array().size() != 4u) return reject(pair.first);
                    for (size_t i = 0; i < 4u; ++i) {
                        const auto& layer = layers->Get_Array()[i];
                        if (!layer.Is_Array() || layer.Get_Array().size() != 4u) return reject("blend layer arity");
                        for (size_t j = 0; j < 4u; ++j) {
                            const auto& value = layer.Get_Array()[j];
                            if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number() < 0.0 || value.Get_Number() > 1000000.0) return reject("blend layer value");
                            (&(*pair.second)[i].x)[j] = static_cast<float>(value.Get_Number());
                        }
                    }
                }
                for (size_t i = 0; i < 4u; ++i)
                    if ((i < 2u || (special.flags & (1u << (i - 2u)))) && (special.blendLayers[i].x <= 0.f || special.blendLayers[i].y <= 0.f)) return reject("blend layer UV");
            }
            else if (!readColor(row, "sourceWetParameters", special.wetParameters) || !readNumber(row, "sourceWetSpecularPower", special.wetSpecularPower)) return reject("wet values");
            if (const auto* baked = row.Find("bakedLighting"))
            {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) || !readString(*baked, "colorSpace", space) ||
                    (space != "linear" && space != "srgb") || !texturePath(*baked, "averageTexture", material.bakedAveragePath) ||
                    !texturePath(*baked, "directionalTexture", material.bakedDirectionalPath)) return reject("baked lighting");
                surface.hasBakedLighting = true; surface.bakedLightingSRGB = space == "srgb";
            }
            staged[assetId].push_back(std::move(material));
            continue;
        }
        if (family == "bg-source-foliage-masked" || family == "bg-source-grass-masked")
        {
            auto& surface = material.surface;
            const bool grass = family == "bg-source-grass-masked";
            surface.family = grass ? Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED : Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED;
            const auto reject = [&](const char* reason) { m_Status = "Invalid source foliage material: " + assetId + "/" + material.materialName + " / " + reason; return false; };
            std::unordered_set<std::string> fields = { "assetId", "materialName", "sourceMaterial", "family", "sourceFlags", "transmissionColor",
                "diffuseBrightness", "normalIntensity", "specularIntensity", "specularPower", "diffuseSaturation", "diffuseColor", "specularColor", "castsShadow", "diffuseTexture", "textureColorSpace" };
            for (const char* optional : { "normalTexture", "specularTexture", "maskTexture", "emissive", "bakedLighting" }) if (row.Find(optional)) fields.insert(optional);
            if (version->Get_Number() != 2.0 || !exactFields(row, fields)) return reject("fields");
            const auto* flags = row.Find("sourceFlags");
            if (!flags || !flags->Is_Number() || flags->Get_Number() < 0.0 || flags->Get_Number() > 127.0 || std::floor(flags->Get_Number()) != flags->Get_Number()) return reject("flags");
            surface.sourceFoliageFlags = static_cast<uint32_t>(flags->Get_Number());
            if ((surface.sourceFoliageFlags & 8u) && !(surface.sourceFoliageFlags & 4u)) return reject("specular branch");
            for (const auto& pair : { std::pair<const char*, float*>("diffuseBrightness", &surface.diffuseBrightness), { "normalIntensity", &surface.normalIntensity },
                { "specularIntensity", &surface.specularIntensity }, { "specularPower", &surface.specularPower }, { "diffuseSaturation", &surface.diffuseSaturation } })
                if (!readNumber(row, pair.first, *pair.second)) return reject(pair.first);
            if (!readColor(row, "diffuseColor", surface.diffuseColor) || !readColor(row, "specularColor", surface.specularColor) ||
                !readColor(row, "transmissionColor", surface.sourceFoliageTransmission)) return reject("colors");
            const auto* shadow = row.Find("castsShadow");
            if (!shadow || !shadow->Is_Boolean()) return reject("shadow");
            surface.castsShadow = shadow->Get_Boolean();
            const auto* spaces = row.Find("textureColorSpace");
            if (!spaces || !exactFields(*spaces, { "diffuse", "normal", "specular", "mask" })) return reject("color spaces");
            for (const auto& pair : { std::pair<const char*, bool_t*>("diffuse", &surface.diffuseSRGB), { "specular", &surface.specularSRGB }, { "mask", &surface.sourceFoliageMaskSRGB } })
            {
                std::string name;
                if (!readString(*spaces, pair.first, name) || (name != "srgb" && name != "linear")) return reject(pair.first);
                *pair.second = name == "srgb";
            }
            std::string normalSpace;
            if (!readString(*spaces, "normal", normalSpace) || normalSpace != "linear") return reject("normal space");
            const auto texture = [&](const DATA_JSON_VALUE& object, const char* key, std::filesystem::path& output) {
                std::string value;
                if (!readString(object, key, value)) return false;
                const std::filesystem::path relative(value);
                std::error_code ec;
                output = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos && relative.extension() == L".dds" &&
                    !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) && std::filesystem::is_regular_file(output, ec) && !ec;
            };
            if (!texture(row, "diffuseTexture", material.surfaceDiffusePath) ||
                ((surface.sourceFoliageFlags & 1u) && !texture(row, "normalTexture", material.surfaceNormalPath)) ||
                ((surface.sourceFoliageFlags & 8u) && !texture(row, "specularTexture", material.surfaceSpecularPath)) ||
                (!grass && !texture(row, "maskTexture", material.sourceFoliageMaskPath))) return reject("selected textures");
            if (const auto* baked = row.Find("bakedLighting"))
            {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) || !readString(*baked, "colorSpace", space) ||
                    (space != "linear" && space != "srgb") || !texture(*baked, "averageTexture", material.bakedAveragePath) ||
                    !texture(*baked, "directionalTexture", material.bakedDirectionalPath)) return reject("baked lighting");
                surface.hasBakedLighting = true; surface.bakedLightingSRGB = space == "srgb";
            }
            if (const auto* emissive = row.Find("emissive"))
            {
                const auto* flicker = emissive->Find("flicker"); const auto* tiling = emissive->Find("uvTiling");
                std::string space;
                if (!(surface.sourceFoliageFlags & 32u) || !exactFields(*emissive, { "texture", "color", "intensity", "uvTiling", "colorSpace", "flicker" }) ||
                    !texture(*emissive, "texture", material.surfaceEmissivePath) || !readColor(*emissive, "color", surface.emissiveColor) ||
                    !readNumber(*emissive, "intensity", surface.emissiveIntensity) || !readString(*emissive, "colorSpace", space) || (space != "srgb" && space != "linear") ||
                    !flicker || !exactFields(*flicker, { "minimum", "speed", "phaseOffset" }) || !readNumber(*flicker, "minimum", surface.emissiveFlickerMinimum) ||
                    !readNumber(*flicker, "speed", surface.emissiveFlickerSpeed) || !tiling || !tiling->Is_Array() || tiling->Get_Array().size() != 2u) return reject("emissive");
                const auto* phase = flicker->Find("phaseOffset");
                if (!phase || !phase->Is_Number() || !std::isfinite(phase->Get_Number()) || std::abs(phase->Get_Number()) > 1000000.0) return reject("emissive phase");
                surface.emissivePhaseOffset = static_cast<float>(phase->Get_Number());
                float components[2]{};
                for (size_t i = 0; i < 2u; ++i) {
                    const auto& value = tiling->Get_Array()[i];
                    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number() <= 0.0 || value.Get_Number() > 1000000.0) return reject("emissive tiling");
                    components[i] = static_cast<float>(value.Get_Number());
                }
                surface.emissiveUVTiling = float2_t(components[0], components[1]);
                surface.emissiveSRGB = space == "srgb"; surface.hasEmissive = true;
            }
            else if (surface.sourceFoliageFlags & 32u) return reject("missing emissive");
            staged[assetId].push_back(std::move(material));
            continue;
        }
        if (family == "bg-source-opaque-masked")
        {
            auto& surface = material.surface;
            surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED;
            const auto reject = [&](const char* reason) {
                m_Status = "Map source BG " + assetId + "/" + material.materialName + ": " + reason;
                return false;
            };
            std::unordered_set<std::string> fields = {
                "assetId", "materialName", "sourceMaterial", "family", "textureColorSpace",
                "diffuseBrightness", "normalIntensity", "specularIntensity", "specularPower",
                "reflectionIntensity", "reflectionContrast", "reflectionTiling", "diffuseSaturation",
                "diffuseColor", "specularColor", "reflectionColor", "uvTiling", "reflectionOriginOffset",
                "castsShadow", "diffuseTexture", "sourceFlags", "sourceBump", "sourceUV", "flickerMode", "addressU"
            };
            for (const char* optional : { "normalTexture", "specularTexture", "reflectionTexture", "bakedLighting", "emissive", "detailNormalTexture", "detailNormalIntensity", "detailNormalTiling", "sourceSubspecular", "sourceRimlight", "sourceSpecularSaturation", "sourcePanning" })
                if (row.Find(optional)) fields.insert(optional);
            if (version->Get_Number() != 2.0 || !exactFields(row, fields)) return reject("invalid fields or version");
            const auto* flags = row.Find("sourceFlags");
            const auto* mode = row.Find("flickerMode");
            if (!flags || !flags->Is_Number() || flags->Get_Number() < 0.0 || flags->Get_Number() > 65535.0 ||
                std::floor(flags->Get_Number()) != flags->Get_Number() || !mode || !mode->Is_Number() ||
                mode->Get_Number() < 0.0 || mode->Get_Number() > 2.0 || std::floor(mode->Get_Number()) != mode->Get_Number())
                return reject("invalid BG flags or flicker mode");
            surface.sourceBgFlags = static_cast<uint32_t>(flags->Get_Number());
            surface.sourceBgFlicker = static_cast<uint32_t>(mode->Get_Number());
            if (((surface.sourceBgFlags & 8u) && !(surface.sourceBgFlags & 4u)) ||
                ((surface.sourceBgFlags & 32u) && !(surface.sourceBgFlags & 16u))) return reject("inconsistent BG flags");
            for (const auto& vector : { std::pair<const char*, float4_t*>("sourceBump", &surface.sourceBgBump),
                std::pair<const char*, float4_t*>("sourceUV", &surface.sourceBgUV) })
            {
                const auto* value = row.Find(vector.first);
                if (!value || !value->Is_Array() || value->Get_Array().size() != 4u) return reject(vector.first);
                float components[4]{};
                for (size_t i = 0; i < 4u; ++i) {
                    const auto& c = value->Get_Array()[i];
                    if (!c.Is_Number() || !std::isfinite(c.Get_Number()) ||
                        std::abs(c.Get_Number()) > (std::numeric_limits<float>::max)()) return reject(vector.first);
                    components[i] = static_cast<float>(c.Get_Number());
                }
                *vector.second = float4_t(components[0], components[1], components[2], components[3]);
            }
            if (std::abs(surface.sourceBgUV.x * surface.sourceBgUV.x + surface.sourceBgUV.y * surface.sourceBgUV.y - 1.f) > 0.0001f)
                return reject("source UV rotation must be unit length");
            std::string address;
            if (!readString(row, "addressU", address) || (address != "WRAP" && address != "MIRROR")) return reject("invalid source addressU");
            material.diffuseMirrorU = address == "MIRROR";
            const std::pair<const char*, float*> scalars[] = {
                { "diffuseBrightness", &surface.diffuseBrightness }, { "normalIntensity", &surface.normalIntensity },
                { "specularIntensity", &surface.specularIntensity }, { "specularPower", &surface.specularPower },
                { "reflectionIntensity", &surface.reflectionIntensity }, { "reflectionContrast", &surface.reflectionContrast },
                { "reflectionTiling", &surface.reflectionTiling }, { "diffuseSaturation", &surface.diffuseSaturation }
            };
            for (const auto& value : scalars)
            {
                if (std::string(value.first) == "normalIntensity")
                {
                    const auto* number = row.Find(value.first);
                    if (!number || !number->Is_Number() || !std::isfinite(number->Get_Number()) ||
                        std::abs(number->Get_Number()) > (std::numeric_limits<float>::max)()) return reject(value.first);
                    *value.second = static_cast<float>(number->Get_Number());
                }
                else if (!readNumber(row, value.first, *value.second)) return reject(value.first);
            }
            if (surface.specularPower < 0.f || surface.reflectionTiling <= 0.f ||
                !readColor(row, "diffuseColor", surface.diffuseColor) ||
                !readColor(row, "specularColor", surface.specularColor) ||
                !readColor(row, "reflectionColor", surface.reflectionColor)) return reject("invalid surface values");
            const auto* shadow = row.Find("castsShadow");
            if (!shadow || !shadow->Is_Boolean()) return reject("invalid castsShadow");
            surface.castsShadow = shadow->Get_Boolean();
            const std::pair<const char*, float2_t*> vectors[] = {
                { "uvTiling", &surface.uvTiling }, { "reflectionOriginOffset", &surface.reflectionOriginOffset }
            };
            for (const auto& vector : vectors)
            {
                const auto* value = row.Find(vector.first);
                if (!value || !value->Is_Array() || value->Get_Array().size() != 2u) return reject(vector.first);
                float components[2]{};
                for (size_t i=0; i<2u; ++i) {
                    const auto& component = value->Get_Array()[i];
                    if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                        std::abs(component.Get_Number()) > (std::numeric_limits<float>::max)()) return reject(vector.first);
                    components[i] = static_cast<float>(component.Get_Number());
                }
                *vector.second = float2_t(components[0], components[1]);
            }
            for (const auto& vector : { std::pair<const char*, float2_t*>{ "sourceSubspecular", &surface.sourceBgSubspecular },
                { "sourcePanning", &surface.sourceBgPanning } })
            {
                const auto* value = row.Find(vector.first);
                if (!value) continue;
                if (!value->Is_Array() || value->Get_Array().size() != 2u) return reject(vector.first);
                float* components = &vector.second->x;
                for (size_t i=0; i<2u; ++i) {
                    const auto& c=value->Get_Array()[i];
                    if (!c.Is_Number() || !std::isfinite(c.Get_Number()) || std::abs(c.Get_Number()) > 1000000.0 ||
                        (std::string(vector.first) == "sourceSubspecular" && c.Get_Number() < 0.0)) return reject(vector.first);
                    components[i] = static_cast<float>(c.Get_Number());
                }
            }
            if (row.Find("sourceRimlight") && !readColor(row, "sourceRimlight", surface.sourceBgRimlight)) return reject("invalid source rimlight");
            if (row.Find("sourceSpecularSaturation") && !readNumber(row, "sourceSpecularSaturation", surface.sourceBgSpecularSaturation)) return reject("invalid source specular saturation");
            const auto* spaces = row.Find("textureColorSpace");
            if (!spaces || !exactFields(*spaces, { "diffuse", "normal", "specular", "reflection" })) return reject("invalid color spaces");
            for (const char* key : { "diffuse", "normal", "specular", "reflection" }) {
                std::string value;
                if (!readString(*spaces, key, value) || (value != "srgb" && value != "linear")) return reject(key);
                const std::string slot(key);
                if (slot == "normal" && value != "linear") return reject("normal must be linear");
                if (slot == "diffuse") surface.diffuseSRGB = value == "srgb";
                else if (slot == "specular") surface.specularSRGB = value == "srgb";
                else if (slot == "reflection") surface.reflectionSRGB = value == "srgb";
            }
            const auto texture = [&](const DATA_JSON_VALUE& object, const char* key, std::filesystem::path& output) {
                std::string value;
                if (!readString(object, key, value)) return false;
                const std::filesystem::path relative(value);
                std::error_code ec;
                output = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos &&
                    relative.extension() == L".dds" && !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) &&
                    std::filesystem::is_regular_file(output, ec) && !ec;
            };
            if (!texture(row, "diffuseTexture", material.surfaceDiffusePath) ||
                ((surface.sourceBgFlags & 1u) && !texture(row, "normalTexture", material.surfaceNormalPath)) ||
                ((surface.sourceBgFlags & 8u) && !texture(row, "specularTexture", material.surfaceSpecularPath)) ||
                ((surface.sourceBgFlags & 16u) && !texture(row, "reflectionTexture", material.reflectionPath))) return reject("missing selected source texture");
            if ((surface.sourceBgFlags & 32768u) != 0u &&
                (!texture(row, "detailNormalTexture", material.detailNormalPath) ||
                 !readNumber(row, "detailNormalIntensity", surface.detailNormalIntensity) ||
                 !readNumber(row, "detailNormalTiling", surface.detailNormalTiling))) return reject("detail normal");
            if (const auto* baked = row.Find("bakedLighting")) {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) ||
                    !readString(*baked, "colorSpace", space) || (space != "linear" && space != "srgb") ||
                    !texture(*baked, "averageTexture", material.bakedAveragePath) ||
                    !texture(*baked, "directionalTexture", material.bakedDirectionalPath)) return reject("invalid baked lighting");
                surface.hasBakedLighting = true;
                surface.bakedLightingSRGB = space == "srgb";
            }
            if (const auto* emissive = row.Find("emissive"))
            {
                std::string colorSpace;
                const auto* flicker = emissive->Find("flicker");
                const auto* tiling = emissive->Find("uvTiling");
                if (!exactFields(*emissive, { "texture", "color", "intensity", "uvTiling", "colorSpace", "flicker" }) ||
                    !texture(*emissive, "texture", material.surfaceEmissivePath) ||
                    !readColor(*emissive, "color", surface.emissiveColor) ||
                    !readNumber(*emissive, "intensity", surface.emissiveIntensity) ||
                    !readString(*emissive, "colorSpace", colorSpace) ||
                    (colorSpace != "linear" && colorSpace != "srgb") ||
                    !flicker || !exactFields(*flicker, { "minimum", "speed", "phaseOffset" }) ||
                    !readNumber(*flicker, "minimum", surface.emissiveFlickerMinimum) ||
                    !readNumber(*flicker, "speed", surface.emissiveFlickerSpeed) ||
                    !tiling || !tiling->Is_Array() || tiling->Get_Array().size() != 2u)
                    return reject("invalid source emissive inputs");
                const auto* phase = flicker->Find("phaseOffset");
                if (!phase || !phase->Is_Number() || !std::isfinite(phase->Get_Number()) ||
                    std::abs(phase->Get_Number()) > (std::numeric_limits<float>::max)())
                    return reject("invalid emissive phase");
                surface.emissivePhaseOffset = static_cast<float>(phase->Get_Number());
                float components[2]{};
                for (size_t i = 0; i < 2u; ++i)
                {
                    const auto& component = tiling->Get_Array()[i];
                    if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                        component.Get_Number() <= 0.0 ||
                        component.Get_Number() > (std::numeric_limits<float>::max)())
                        return reject("invalid emissive UV tiling");
                    components[i] = static_cast<float>(component.Get_Number());
                }
                surface.emissiveUVTiling = float2_t(components[0], components[1]);
                surface.emissiveSRGB = colorSpace == "srgb";
                surface.hasEmissive = true;
            }
            staged[assetId].push_back(std::move(material));
            continue;
        }
        if (!readString(row, "reflectionTexture", reflectionTexture))
        { m_Status = "Map material reflection texture is missing: " + assetId; return false; }
        if (family == "bg_seamless-specular_opa")
        {
            auto& surface = material.surface;
            surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE;
            const auto reject = [&](const char* reason) {
                m_Status = "Map source specular " + assetId + "/" + material.materialName + ": " + reason;
                return false;
            };
            std::unordered_set<std::string> fields = {
                "assetId", "materialName", "sourceMaterial", "family", "textureColorSpace",
                "diffuseBrightness", "normalIntensity", "specularIntensity", "specularPower",
                "reflectionIntensity", "reflectionContrast", "reflectionTiling", "diffuseSaturation",
                "diffuseColor", "specularColor", "reflectionColor", "uvTiling", "reflectionOriginOffset",
                "castsShadow", "diffuseTexture", "normalTexture", "specularTexture", "reflectionTexture"
            };
            if (row.Find("bakedLighting")) fields.insert("bakedLighting");
            if (version->Get_Number() != 2.0 || !exactFields(row, fields)) return reject("invalid fields or version");
            const std::pair<const char*, float*> scalars[] = {
                { "diffuseBrightness", &surface.diffuseBrightness }, { "normalIntensity", &surface.normalIntensity },
                { "specularIntensity", &surface.specularIntensity }, { "specularPower", &surface.specularPower },
                { "reflectionIntensity", &surface.reflectionIntensity }, { "reflectionContrast", &surface.reflectionContrast },
                { "reflectionTiling", &surface.reflectionTiling }, { "diffuseSaturation", &surface.diffuseSaturation }
            };
            for (const auto& value : scalars)
                if (!readNumber(row, value.first, *value.second)) return reject(value.first);
            if (surface.specularPower < 1.f || surface.reflectionTiling <= 0.f ||
                !readColor(row, "diffuseColor", surface.diffuseColor) ||
                !readColor(row, "specularColor", surface.specularColor) ||
                !readColor(row, "reflectionColor", surface.reflectionColor)) return reject("invalid surface values");
            const auto* shadow = row.Find("castsShadow");
            if (!shadow || !shadow->Is_Boolean()) return reject("invalid castsShadow");
            surface.castsShadow = shadow->Get_Boolean();
            const std::pair<const char*, float2_t*> vectors[] = {
                { "uvTiling", &surface.uvTiling }, { "reflectionOriginOffset", &surface.reflectionOriginOffset }
            };
            for (const auto& vector : vectors)
            {
                const auto* value = row.Find(vector.first);
                if (!value || !value->Is_Array() || value->Get_Array().size() != 2u) return reject(vector.first);
                float components[2]{};
                for (size_t i=0; i<2u; ++i) {
                    const auto& component = value->Get_Array()[i];
                    if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                        std::abs(component.Get_Number()) > (std::numeric_limits<float>::max)()) return reject(vector.first);
                    components[i] = static_cast<float>(component.Get_Number());
                }
                *vector.second = float2_t(components[0], components[1]);
            }
            if (surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f) return reject("non-positive UV tiling");
            const auto* spaces = row.Find("textureColorSpace");
            if (!spaces || !exactFields(*spaces, { "diffuse", "normal", "specular", "reflection" })) return reject("invalid color spaces");
            for (const char* key : { "diffuse", "normal", "specular", "reflection" }) {
                std::string value;
                if (!readString(*spaces, key, value) || (value != "srgb" && value != "linear")) return reject(key);
                const std::string slot(key);
                if (slot == "normal" && value != "linear") return reject("normal must be linear");
                if (slot == "diffuse") surface.diffuseSRGB = value == "srgb";
                else if (slot == "specular") surface.specularSRGB = value == "srgb";
                else if (slot == "reflection") surface.reflectionSRGB = value == "srgb";
            }
            const auto texture = [&](const DATA_JSON_VALUE& object, const char* key, std::filesystem::path& output) {
                std::string value;
                if (!readString(object, key, value)) return false;
                const std::filesystem::path relative(value);
                std::error_code ec;
                output = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos &&
                    relative.extension() == L".dds" && !output.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), output) &&
                    std::filesystem::is_regular_file(output, ec) && !ec;
            };
            if (!texture(row, "diffuseTexture", material.surfaceDiffusePath) ||
                !texture(row, "normalTexture", material.surfaceNormalPath) ||
                !texture(row, "specularTexture", material.surfaceSpecularPath) ||
                !texture(row, "reflectionTexture", material.reflectionPath)) return reject("missing or invalid texture");
            if (const auto* baked = row.Find("bakedLighting")) {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) ||
                    !readString(*baked, "colorSpace", space) || (space != "linear" && space != "srgb") ||
                    !texture(*baked, "averageTexture", material.bakedAveragePath) ||
                    !texture(*baked, "directionalTexture", material.bakedDirectionalPath)) return reject("invalid baked lighting");
                surface.hasBakedLighting = true;
                surface.bakedLightingSRGB = space == "srgb";
            }
            staged[assetId].push_back(std::move(material));
            continue;
        }
		if (family == "bg_base_pbr_seamless_opa" || family == "bg_base_pbr_opa")
		{
			auto& pbr = material.surface;
			pbr.family = family == "bg_base_pbr_seamless_opa" ?
				Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE : Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE;
			const auto reject = [&](const std::string& reason)
			{
				m_Status = "Map PBR material " + assetId + "/" + material.materialName + ": " + reason;
				return false;
			};
			std::unordered_set<std::string> pbrFields = {
				"assetId", "materialName", "sourceMaterial", "family",
				"textureColorSpace", "diffuseColor", "reflectionColor", "uvTiling",
				"reflectionOriginOffset", "diffuseBrightness", "normalIntensity", "reflectionIntensity",
				"reflectionContrast", "reflectionTiling", "diffuseSaturation", "detailNormalIntensity",
				"detailNormalTiling", "metallicIntensity", "metallicPower", "roughnessIntensity",
				"roughnessPower", "aoIntensity", "aoPower", "specularPBRIntensity",
				"nonmetallicBrightness", "metallicBrightness", "minimumRoughness", "vertexAlpha",
				"uvFixedNormal", "useWorldReflection", "castsShadow", "diffuseTexture",
				"normalTexture", "detailNormalTexture", "ormTexture", "reflectionTexture"
            };
            if (row.Find("bakedLighting")) pbrFields.insert("bakedLighting");
            if (row.Find("environment")) pbrFields.insert("environment");
            if (row.Find("emissive")) pbrFields.insert("emissive");
            if (version->Get_Number() != 2.0 || !exactFields(row, pbrFields)) return reject("invalid version or fields");
			const std::pair<const char*, float*> numbers[] = {
				{ "diffuseBrightness", &pbr.diffuseBrightness },
				{ "normalIntensity", &pbr.normalIntensity },
				{ "reflectionIntensity", &pbr.reflectionIntensity },
				{ "reflectionContrast", &pbr.reflectionContrast },
				{ "reflectionTiling", &pbr.reflectionTiling },
				{ "diffuseSaturation", &pbr.diffuseSaturation },
				{ "detailNormalIntensity", &pbr.detailNormalIntensity },
				{ "detailNormalTiling", &pbr.detailNormalTiling },
				{ "metallicIntensity", &pbr.metallicIntensity },
				{ "metallicPower", &pbr.metallicPower },
				{ "roughnessIntensity", &pbr.roughnessIntensity },
				{ "roughnessPower", &pbr.roughnessPower },
				{ "aoIntensity", &pbr.aoIntensity },
				{ "aoPower", &pbr.aoPower },
				{ "specularPBRIntensity", &pbr.specularPBRIntensity },
				{ "nonmetallicBrightness", &pbr.nonmetallicBrightness },
				{ "metallicBrightness", &pbr.metallicBrightness },
				{ "minimumRoughness", &pbr.minimumRoughness },
				{ "vertexAlpha", &pbr.vertexAlpha },
			};
			for (const auto& number : numbers)
				if (!readNumber(row, number.first, *number.second)) return reject(number.first);
			if (pbr.minimumRoughness <= 0.f || pbr.minimumRoughness > 1.f ||
				pbr.vertexAlpha > 1.f || pbr.detailNormalTiling <= 0.f || pbr.reflectionTiling <= 0.f ||
				!readColor(row, "diffuseColor", pbr.diffuseColor) ||
				!readColor(row, "reflectionColor", pbr.reflectionColor)) return reject("invalid scalar bounds or colors");
			const std::pair<const char*, bool_t*> switches[] = {
				{ "uvFixedNormal", &pbr.uvFixedNormal },
				{ "useWorldReflection", &pbr.useWorldReflection },
				{ "castsShadow", &pbr.castsShadow },
			};
			for (const auto& flag : switches)
			{
				const auto* value = row.Find(flag.first);
				if (!value || !value->Is_Boolean()) return reject(flag.first);
				*flag.second = value->Get_Boolean();
			}
			const std::pair<const char*, float2_t*> vectors[] = {
				{ "uvTiling", &pbr.uvTiling }, { "reflectionOriginOffset", &pbr.reflectionOriginOffset }
			};
			for (const auto& vector : vectors)
			{
				const auto* value = row.Find(vector.first);
				if (!value || !value->Is_Array() || value->Get_Array().size() != 2u) return reject(vector.first);
				float components[2]{};
				for (size_t i = 0; i < 2u; ++i)
				{
					const auto& component = value->Get_Array()[i];
					if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
						std::abs(component.Get_Number()) > (std::numeric_limits<float>::max)()) return reject(vector.first);
					components[i] = static_cast<float>(component.Get_Number());
				}
				*vector.second = float2_t(components[0], components[1]);
			}
			if (pbr.uvTiling.x <= 0.f || pbr.uvTiling.y <= 0.f) return reject("non-positive UV tiling");
			const auto* spaces = row.Find("textureColorSpace");
			if (!spaces || !exactFields(*spaces, { "diffuse", "normal", "detailNormal", "reflection", "orm" }))
				return reject("invalid texture color spaces");
			for (const char* key : { "diffuse", "normal", "detailNormal", "reflection", "orm" })
			{
				std::string value;
				if (!readString(*spaces, key, value) || (value != "srgb" && value != "linear")) return reject(key);
				const std::string slot(key);
				if ((slot == "normal" || slot == "detailNormal") && value != "linear") return reject("normal must be linear");
				if (slot == "diffuse") pbr.diffuseSRGB = value == "srgb";
				else if (slot == "reflection") pbr.reflectionSRGB = value == "srgb";
				else if (slot == "orm") pbr.ormSRGB = value == "srgb";
			}
			const std::pair<const char*, std::filesystem::path*> textures[] = {
				{ "diffuseTexture", &material.surfaceDiffusePath },
				{ "normalTexture", &material.surfaceNormalPath },
				{ "detailNormalTexture", &material.detailNormalPath },
				{ "ormTexture", &material.surfaceORMPath },
				{ "reflectionTexture", &material.reflectionPath },
			};
			for (const auto& texture : textures)
			{
				std::string value;
				if (!readString(row, texture.first, value)) return reject(texture.first);
				const std::filesystem::path relative(value);
				std::error_code ec;
				*texture.second = CRuntimeAssetRoot::Resolve(relative);
				if (relative.is_absolute() || relative.has_root_path() || value.find(':') != std::string::npos ||
					relative.extension() != L".dds" || texture.second->empty() ||
					!IsInsideRoot(CRuntimeAssetRoot::Get(), *texture.second) ||
					!std::filesystem::is_regular_file(*texture.second, ec) || ec) return reject(texture.first);
			}
            const auto lightingTexture = [&](const DATA_JSON_VALUE& object, const char* field, std::filesystem::path& result)
            {
                std::string value;
                if (!readString(object, field, value)) return false;
                const std::filesystem::path relative(value);
                std::error_code ec;
                result = CRuntimeAssetRoot::Resolve(relative);
                return !relative.is_absolute() && !relative.has_root_path() && value.find(':') == std::string::npos &&
                    relative.extension() == L".dds" && !result.empty() && IsInsideRoot(CRuntimeAssetRoot::Get(), result) &&
                    std::filesystem::is_regular_file(result, ec) && !ec;
            };
            if (const auto* baked = row.Find("bakedLighting"))
            {
                std::string space;
                if (!exactFields(*baked, { "averageTexture", "directionalTexture", "colorSpace" }) ||
                    !readString(*baked, "colorSpace", space) || (space != "linear" && space != "srgb") ||
                    !lightingTexture(*baked, "averageTexture", material.bakedAveragePath) ||
                    !lightingTexture(*baked, "directionalTexture", material.bakedDirectionalPath))
                    return reject("invalid baked lighting texture inputs");
                pbr.hasBakedLighting = true;
                pbr.bakedLightingSRGB = space == "srgb";
            }
            if (const auto* emissive = row.Find("emissive"))
            {
                std::string colorSpace;
                const auto* flicker = emissive->Find("flicker");
                const auto* tiling = emissive->Find("uvTiling");
                if (!exactFields(*emissive, { "texture", "color", "intensity", "uvTiling", "colorSpace", "flicker" }) ||
                    !lightingTexture(*emissive, "texture", material.surfaceEmissivePath) ||
                    !readColor(*emissive, "color", pbr.emissiveColor) ||
                    !readNumber(*emissive, "intensity", pbr.emissiveIntensity) ||
                    !readString(*emissive, "colorSpace", colorSpace) ||
                    (colorSpace != "linear" && colorSpace != "srgb") ||
                    !flicker || !exactFields(*flicker, { "minimum", "speed", "phaseOffset" }) ||
                    !readNumber(*flicker, "minimum", pbr.emissiveFlickerMinimum) ||
                    pbr.emissiveFlickerMinimum > 1.f ||
                    !readNumber(*flicker, "speed", pbr.emissiveFlickerSpeed) ||
                    !tiling || !tiling->Is_Array() || tiling->Get_Array().size() != 2u)
                    return reject("invalid source emissive inputs");
                const auto* phase = flicker->Find("phaseOffset");
                if (!phase || !phase->Is_Number() || !std::isfinite(phase->Get_Number()) ||
                    std::abs(phase->Get_Number()) > (std::numeric_limits<float>::max)())
                    return reject("invalid emissive phase");
                pbr.emissivePhaseOffset = static_cast<float>(phase->Get_Number());
                float components[2]{};
                for (size_t i = 0; i < 2u; ++i)
                {
                    const auto& component = tiling->Get_Array()[i];
                    if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                        component.Get_Number() <= 0.0 ||
                        component.Get_Number() > (std::numeric_limits<float>::max)())
                        return reject("invalid emissive UV tiling");
                    components[i] = static_cast<float>(component.Get_Number());
                }
                pbr.emissiveUVTiling = float2_t(components[0], components[1]);
                pbr.emissiveSRGB = colorSpace == "srgb";
                pbr.hasEmissive = true;
            }
            if (const auto* environment = row.Find("environment"))
            {
                if (!exactFields(*environment, { "cubeTexture", "brdfTexture", "color", "rotation" }) ||
                    !lightingTexture(*environment, "cubeTexture", material.environmentCubePath) ||
                    !lightingTexture(*environment, "brdfTexture", material.environmentBRDFPath) ||
                    !readColor(*environment, "color", pbr.environmentColor))
                    return reject("invalid environment cube inputs");
                const auto* rotation = environment->Find("rotation");
                if (!rotation || !rotation->Is_Array() || rotation->Get_Array().size() != 2u)
                    return reject("invalid environment rotation");
                float components[2]{};
                for (size_t i=0; i<2; ++i)
                {
                    const auto& component = rotation->Get_Array()[i];
                    if (!component.Is_Number() || !std::isfinite(component.Get_Number()) || std::abs(component.Get_Number()) > 1.0)
                        return reject("invalid environment rotation component");
                    components[i] = static_cast<float>(component.Get_Number());
                }
                if (std::abs(components[0]*components[0]+components[1]*components[1]-1.f) > 0.0001f)
                    return reject("environment rotation must be unit length");
                pbr.environmentRotation = float2_t(components[0],components[1]);
                pbr.hasEnvironmentCube = true;
            }
			staged[assetId].push_back(std::move(material));
			continue;
		}
		std::unordered_set<std::string> fields = {
			"assetId", "materialName", "sourceMaterial", "family",
			"diffuseBrightness", "normalIntensity", "specularIntensity", "specularPower",
			"reflectionIntensity", "reflectionContrast", "diffuseColor", "specularColor",
			"reflectionColor", "reflectionTexture", "textureColorSpace"
		};
		auto& surface = material.surface;
		if (family == "bg_seamless-specular_msk")
		{
			surface.family = Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION;
			fields.insert("reflectionTiling");
			if (!readNumber(row, "reflectionTiling", surface.reflectionTiling) ||
				surface.reflectionTiling <= 0.f)
			{
				m_Status = "Map material reflection tiling is invalid: " + assetId;
				return false;
			}
		}
		else if (family == "bg_base_msk")
		{
			surface.family = Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION;
			fields.insert("diffuseSaturation");
			if (!readNumber(row, "diffuseSaturation", surface.diffuseSaturation))
			{
				m_Status = "Map material diffuse saturation is invalid: " + assetId;
				return false;
			}
		}
		else
		{
			m_Status = "Map material family is unsupported: " + family;
			return false;
		}
		if (!exactFields(row, fields) ||
			!readNumber(row, "diffuseBrightness", surface.diffuseBrightness) ||
			!readNumber(row, "normalIntensity", surface.normalIntensity) ||
			!readNumber(row, "specularIntensity", surface.specularIntensity) ||
			!readNumber(row, "specularPower", surface.specularPower) || surface.specularPower < 1.f ||
			!readNumber(row, "reflectionIntensity", surface.reflectionIntensity) ||
			!readNumber(row, "reflectionContrast", surface.reflectionContrast) ||
			!readColor(row, "diffuseColor", surface.diffuseColor) ||
			!readColor(row, "specularColor", surface.specularColor) ||
			!readColor(row, "reflectionColor", surface.reflectionColor))
		{
			m_Status = "Map material fields or numeric inputs are invalid: " + assetId;
			return false;
		}
		const auto* colorSpace = row.Find("textureColorSpace");
		if (!colorSpace || !exactFields(*colorSpace, { "diffuse", "specular", "reflection" }))
		{
			m_Status = "Map material texture color spaces are invalid: " + assetId;
			return false;
		}
		const auto readColorSpace = [&readString, &colorSpace](const char* key, bool_t& result)
		{
			std::string value;
			if (!readString(*colorSpace, key, value) || (value != "srgb" && value != "linear"))
				return false;
			result = value == "srgb";
			return true;
		};
		const std::filesystem::path relative(reflectionTexture);
		std::error_code reflectionError;
		material.reflectionPath = CRuntimeAssetRoot::Resolve(relative);
		if (!readColorSpace("diffuse", surface.diffuseSRGB) ||
			!readColorSpace("specular", surface.specularSRGB) ||
			!readColorSpace("reflection", surface.reflectionSRGB) ||
			relative.is_absolute() || relative.has_root_path() ||
			reflectionTexture.find(':') != std::string::npos ||
			relative.extension() != L".dds" || material.reflectionPath.empty() ||
			!IsInsideRoot(CRuntimeAssetRoot::Get(), material.reflectionPath) ||
			!std::filesystem::is_regular_file(material.reflectionPath, reflectionError) || reflectionError)
		{
			m_Status = "Map material reflection path or color space is invalid: " + assetId;
			return false;
		}
		staged[assetId].push_back(std::move(material));
	}
    std::unordered_map<std::string, MAP_PLACEMENT_LIGHTING> stagedLighting;
    if (placementRows)
    {
        if (placementRows->Get_Array().size() > 65536u)
        { m_Status = "Too many placement lighting rows"; return false; }
        for (const auto& row : placementRows->Get_Array())
        {
            MAP_PLACEMENT_LIGHTING lighting;
            std::string sourceId;
            const auto readVector = [&](const char* name, float* values, size_t count)
            {
                const auto* vector = row.Find(name);
                if (!vector || !vector->Is_Array() || vector->Get_Array().size() != count) return false;
                for (size_t i=0; i<count; ++i)
                {
                    const auto& value = vector->Get_Array()[i];
                    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number() < 0.0 ||
                        value.Get_Number() > (std::numeric_limits<float>::max)()) return false;
                    values[i] = static_cast<float>(value.Get_Number());
                }
                return true;
            };
            float scale[2]{}, bias[2]{}, average[3]{}, directional[3]{};
            const bool hasShadowScale = row.Find("shadowCoordinateScale") != nullptr;
            const bool hasShadowBias = row.Find("shadowCoordinateBias") != nullptr;
            std::unordered_set<std::string> lightingFields = { "sourcePlacementId", "assetId", "coordinateScale", "coordinateBias", "averageScale", "directionalScale" };
            if (hasShadowScale) lightingFields.insert("shadowCoordinateScale");
            if (hasShadowBias) lightingFields.insert("shadowCoordinateBias");

            if (!exactFields(row, lightingFields) ||
                !readString(row, "sourcePlacementId", sourceId) || !IsValidDisplayText(sourceId, MAX_EVIDENCE_LENGTH) ||
                !readString(row, "assetId", lighting.assetId) || !staged.contains(lighting.assetId) ||
                !readVector("coordinateScale", scale, 2) || !readVector("coordinateBias", bias, 2) ||
                !readVector("averageScale", average, 3) || !readVector("directionalScale", directional, 3) ||
                scale[0] <= 0.f || scale[1] <= 0.f || scale[0]+bias[0] > 1.00001f || scale[1]+bias[1] > 1.00001f)
            { m_Status = "Invalid placement lighting row: " + sourceId; return false; }
            const auto& materials = staged.at(lighting.assetId);
            if (std::none_of(materials.begin(), materials.end(), [](const auto& material) { return material.surface.hasBakedLighting; }))
            { m_Status = "Placement lighting has no baked material: " + sourceId; return false; }
            const bool materialHasShadow = std::any_of(materials.begin(), materials.end(), [](const auto& material) { return material.surface.hasStaticShadow; });
            float shadowScale[2]{}, shadowBias[2]{};
            if (hasShadowScale != hasShadowBias || materialHasShadow != hasShadowScale ||
                (hasShadowScale && (!readVector("shadowCoordinateScale", shadowScale, 2) ||
                    !readVector("shadowCoordinateBias", shadowBias, 2) || shadowScale[0] <= 0.f || shadowScale[1] <= 0.f ||
                    shadowScale[0] + shadowBias[0] > 1.00001f || shadowScale[1] + shadowBias[1] > 1.00001f)))
            { m_Status = "Invalid static shadow placement coordinates: " + sourceId; return false; }
            lighting.inputs.shadowScaleBias = float4_t(shadowScale[0], shadowScale[1], shadowBias[0], shadowBias[1]);
            lighting.inputs.scaleBias = float4_t(scale[0],scale[1],bias[0],bias[1]);
            lighting.inputs.averageScale = float4_t(average[0],average[1],average[2],1.f);
            lighting.inputs.directionalScale = float4_t(directional[0],directional[1],directional[2],0.f);
            if (!stagedLighting.emplace(sourceId, std::move(lighting)).second)
            { m_Status = "Duplicate placement lighting source ID: " + sourceId; return false; }
        }
    }
	for (auto& entry : m_Entries)
	{
		const auto found = staged.find(entry.id);
		if (found != staged.end())
		{
            for (auto& material : found->second)
            {
                const auto policy = drawPolicies.at(entry.id + "\n" + material.materialName);
                material.surface.renderMode = policy.first;
                material.surface.cullMode = policy.second;
            }
            // A mixed model queues shadows if any slot can cast; each draw still
            // checks its own immutable material policy.
            entry.renderProfile.castsShadow = std::any_of(found->second.begin(), found->second.end(),
                [](const auto& row) { return row.hasDiffuseAddressU || row.surface.castsShadow; });
			entry.materialOverrides = std::move(found->second);
		}
	}
	m_PlacementLighting = std::move(stagedLighting);
	return true;
}

const MAP_PLACEMENT_LIGHTING* CMapAssetCatalog::Find_PlacementLighting(const std::string& sourcePlacementId) const
{
    const auto found = m_PlacementLighting.find(sourcePlacementId);
    return found == m_PlacementLighting.end() ? nullptr : &found->second;
}

bool_t CMapAssetCatalog::Load_AreaStaged(const std::string& areaId)
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

		std::string stagedMaterialFilename;
		bool_t hasMaterialDeclaration = false;
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
			if (!hasMaterialDeclaration)
			{
				stagedMaterialFilename = child.m_MaterialDocumentFilename;
				hasMaterialDeclaration = true;
			}
			else if (stagedMaterialFilename != child.m_MaterialDocumentFilename)
			{
				m_Status = "Map shards disagree on their required material document: " +
					shard.shardId;
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
		m_MaterialDocumentFilename = std::move(stagedMaterialFilename);
		m_AreaId = std::move(selectedAreaId);
		m_CatalogPath = shardSetPath;
		m_PlacementPath = hasSourceOverride ?
			m_SourcePlacementOverride : std::filesystem::path{};
		m_bSharded = true;
		m_bReady = true;
		if (!Load_WaterPresentation(m_AreaId))
		{
			m_bReady = false;
			return false;
		}
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
	if (!Load_WaterPresentation(m_AreaId))
	{
		m_bReady = false;
		return false;
	}
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

	std::string stagedMaterialFilename;
	if (version >= MATERIAL_REFERENCE_CATALOG_VERSION &&
		(!(input >> std::quoted(stagedMaterialFilename)) ||
			!IsSafeRelativeFilename(stagedMaterialFilename, L".json") ||
			stagedMaterialFilename != stagedAreaId + ".mapmaterials.json"))
	{
		m_Status = "Catalog material document reference is invalid";
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
			if (version >= OPACITY_SHAPING_CATALOG_VERSION &&
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
		else if (row.renderMode == "Water")
			entry.renderProfile.renderMode = MAP_ASSET_RENDER_MODE::WATER;
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

		/* The landscape group is the only catalog kind whose mesh is a
		   heightfield with authored top-down UV, so it is also the only one
		   whose cliff faces need the substituted height axis. */
		if ("landscape" == entry.groupId)
		{
			entry.renderProfile.triplanarHeightScale =
				LANDSCAPE_TRIPLANAR_HEIGHT_SCALE;
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
	m_MaterialDocumentFilename = std::move(stagedMaterialFilename);
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

const MAP_ASSET_WATER_PROFILE* CMapAssetCatalog::Find_Water(
	const std::string& assetId) const
{
	const auto iter = m_WaterProfiles.find(assetId);
	return iter == m_WaterProfiles.end() ? nullptr : &iter->second;
}

bool_t CMapAssetCatalog::Load_WaterPresentation(const std::string& areaId)
{
	m_WaterProfiles.clear();

	const auto countWaterAssets = [this]()
	{
		size_t count = 0;
		for (const MAP_ASSET_ENTRY& entry : m_Entries)
		{
			if (MAP_ASSET_RENDER_MODE::WATER == entry.renderProfile.renderMode)
				++count;
		}
		return count;
	};

	const std::filesystem::path documentPath =
		m_SourceCatalogOverride.empty() ?
		Get_MapDataRoot() /
			(std::filesystem::path(areaId).wstring() + L".mapwater.json") :
		Get_MapAuthoringRoot() / L"Authoring" / areaId /
			(std::filesystem::path(areaId).wstring() + L".mapwater.json");

	std::error_code existsError;
	if (!std::filesystem::exists(documentPath, existsError) || existsError)
	{
		/* No document is only valid when nothing claims to be water. */
		if (0 != countWaterAssets())
		{
			m_Status = "Map water document is missing for a WATER asset: " +
				documentPath.string();
			return false;
		}
		return true;
	}

	std::ifstream input(documentPath, std::ios::binary);
	if (!input)
	{
		m_Status = "Map water document could not be opened: " +
			documentPath.string();
		return false;
	}
	const std::string text(
		(std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_Status = "Map water document parse failed: " + parseError;
		return false;
	}

	const auto readNumber = [](const DATA_JSON_VALUE& owner,
		const char* key, float& outValue)
	{
		const DATA_JSON_VALUE* value = owner.Find(key);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<float>(value->Get_Number());
		return true;
	};
	const auto readString = [](const DATA_JSON_VALUE& owner,
		const char* key, std::string& outValue)
	{
		const DATA_JSON_VALUE* value = owner.Find(key);
		if (nullptr == value || !value->Is_String())
			return false;
		outValue = value->Get_String();
		return true;
	};
	const auto readVector = [](const DATA_JSON_VALUE& owner,
		const char* key, float4_t& outValue)
	{
		const DATA_JSON_VALUE* value = owner.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			4u != value->Get_Array().size())
		{
			return false;
		}
		float components[4]{};
		for (size_t index = 0; index < 4u; ++index)
		{
			const DATA_JSON_VALUE& component = value->Get_Array()[index];
			if (!component.Is_Number() ||
				!std::isfinite(component.Get_Number()))
			{
				return false;
			}
			components[index] = static_cast<float>(component.Get_Number());
		}
		outValue = float4_t(
			components[0], components[1], components[2], components[3]);
		return true;
	};

	std::string schema;
	std::string documentAreaId;
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* waters = root.Find("waters");
	if (!readString(root, "schema", schema) ||
		schema != "lostark.map-water-presentation" ||
		nullptr == version || !version->Is_Number() ||
		1.0 != version->Get_Number() ||
		!readString(root, "areaId", documentAreaId) ||
		documentAreaId != areaId ||
		nullptr == waters || !waters->Is_Array() ||
		waters->Get_Array().size() > 64u)
	{
		m_Status = "Map water document header is invalid: " +
			documentPath.string();
		return false;
	}

	std::unordered_map<std::string, MAP_ASSET_WATER_PROFILE> staged;
	for (const DATA_JSON_VALUE& row : waters->Get_Array())
	{
		if (!row.Is_Object())
		{
			m_Status = "Map water row is not an object";
			return false;
		}
		std::string assetId;
		MAP_ASSET_WATER_PROFILE profile{};
		if (!readString(row, "assetId", assetId) || assetId.empty() ||
			!readString(row, "materialName", profile.materialName) ||
			profile.materialName.empty() ||
			!readString(row, "detailNormalTexture", profile.detailNormalTexture) ||
			!readString(row, "reflectionTexture", profile.reflectionTexture) ||
			!readString(row, "foamTexture", profile.foamTexture) ||
			!readNumber(row, "opacity", profile.opacity) ||
			!readNumber(row, "opacityPower", profile.opacityPower) ||
			!readNumber(row, "fresnelIntensity", profile.fresnelIntensity) ||
			!readNumber(row, "fresnelPower", profile.fresnelPower) ||
			!readNumber(row, "screenDistortionIntensity",
				profile.screenDistortionIntensity) ||
			!readNumber(row, "normalIntensity", profile.normalIntensity) ||
			!readNumber(row, "detailNormalIntensity",
				profile.detailNormalIntensity) ||
			!readNumber(row, "normalDistortionIntensity",
				profile.normalDistortionIntensity) ||
			!readNumber(row, "reflectionIntensity", profile.reflectionIntensity) ||
			!readNumber(row, "reflectionUv", profile.reflectionUv) ||
			!readNumber(row, "depthBias", profile.depthBias) ||
			!readNumber(row, "diffuseTiling", profile.diffuseTiling) ||
			!readVector(row, "diffuseColor", profile.diffuseColor) ||
			!readVector(row, "reflectionColor", profile.reflectionColor) ||
			!readVector(row, "normalTilingPanning", profile.normalTilingPanning) ||
			!readVector(row, "detailNormalTilingPanning",
				profile.detailNormalTilingPanning) ||
			!readVector(row, "reflectionTilingPanning",
				profile.reflectionTilingPanning))
		{
			m_Status = "Map water row is invalid in " + documentPath.string();
			return false;
		}
		if (!staged.emplace(assetId, std::move(profile)).second)
		{
			m_Status = "Duplicate map water asset row: " + assetId;
			return false;
		}
	}

	/* Both directions, so neither a water asset without parameters nor a row
	   whose asset was never switched to WATER can pass unnoticed. */
	for (const MAP_ASSET_ENTRY& entry : m_Entries)
	{
		const bool_t isWater =
			MAP_ASSET_RENDER_MODE::WATER == entry.renderProfile.renderMode;
		const bool_t hasRow = staged.find(entry.id) != staged.end();
		if (isWater != hasRow && (isWater || hasRow))
		{
			m_Status = isWater ?
				"WATER asset has no map water row: " + entry.id :
				"Map water row is not a WATER asset: " + entry.id;
			return false;
		}
	}

	m_WaterProfiles = std::move(staged);
	return true;
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
