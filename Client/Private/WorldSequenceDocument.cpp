#include "WorldSequenceDocument.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "SourceCharacterMaterialParameters.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-sequences";
	constexpr uint32_t FORMAT_VERSION = 3;
	constexpr uint32_t LEGACY_FORMAT_VERSION = 1;
	constexpr f32_t MIN_SCALE = 0.000001f;
	constexpr f32_t MIN_RUNTIME_SCALE_DETERMINANT = 0.000001f;
	constexpr f32_t MAX_COMPONENT = 100000.f;
	constexpr uintmax_t MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u;

	bool_t Is_ValidUtf8DisplayText(const std::string& value)
	{
		for (size_t offset = 0u; offset < value.size();)
		{
			const uint8_t first = static_cast<uint8_t>(value[offset]);
			if (first < 0x80u)
			{
				if (first < 0x20u || 0x7fu == first)
					return false;
				++offset;
				continue;
			}
			size_t length = 0u;
			uint32_t codePoint = 0u;
			uint32_t minimum = 0u;
			if (first >= 0xc2u && first <= 0xdfu)
			{
				length = 2u;
				codePoint = first & 0x1fu;
				minimum = 0x80u;
			}
			else if (first >= 0xe0u && first <= 0xefu)
			{
				length = 3u;
				codePoint = first & 0x0fu;
				minimum = 0x800u;
			}
			else if (first >= 0xf0u && first <= 0xf4u)
			{
				length = 4u;
				codePoint = first & 0x07u;
				minimum = 0x10000u;
			}
			else
			{
				return false;
			}
			if (offset + length > value.size())
				return false;
			for (size_t index = 1u; index < length; ++index)
			{
				const uint8_t next = static_cast<uint8_t>(value[offset + index]);
				if ((next & 0xc0u) != 0x80u)
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if (codePoint < minimum || codePoint > 0x10ffffu ||
				(codePoint >= 0xd800u && codePoint <= 0xdfffu))
			{
				return false;
			}
			offset += length;
		}
		return true;
	}

	bool_t Is_IntegerNumber(const DATA_JSON_VALUE* value)
	{
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number()) &&
			std::floor(value->Get_Number()) == value->Get_Number();
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Is_ObjectShape(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> required,
		const std::initializer_list<const char_t*> optional)
	{
		if (!value.Is_Object()) return false;
		for (const char_t* key : required)
			if (nullptr == value.Find(key)) return false;
		for (const auto& entry : value.Get_Object())
		{
			const auto matches = [&entry](const char_t* key) { return entry.first == key; };
			if (std::none_of(required.begin(), required.end(), matches) &&
				std::none_of(optional.begin(), optional.end(), matches)) return false;
		}
		return true;
	}

	bool_t Is_BoundedFloat3(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
			std::abs(value.x) <= MAX_COMPONENT && std::abs(value.y) <= MAX_COMPONENT &&
			std::abs(value.z) <= MAX_COMPONENT;
	}

	bool_t Is_ResourcePath(const std::string& value, const bool_t model)
	{
		if (value.empty() || value.size() > 1024u || value.front() == '/' ||
			value.find(':') != std::string::npos || value.find('\\') != std::string::npos ||
			!Is_ValidUtf8DisplayText(value)) return false;
		std::istringstream parts(value);
		std::string part;
		while (std::getline(parts, part, '/'))
			if (part.empty() || part == "." || part == "..") return false;
		std::string extension = value.size() > 7u ? value.substr(value.size() - 7u) : std::string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](const unsigned char character) { return static_cast<char_t>(std::tolower(character)); });
		return value.back() != '/' && (!model || extension == ".wmodel");
	}

    bool_t Validate_MaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS* out = nullptr)
    {
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS packed;
        if (profile.materialName.empty() || profile.materialName.size() > 63u ||
            !Is_ValidUtf8DisplayText(profile.materialName) || profile.sourceMaterial.empty() ||
            profile.sourceMaterial.size() > 512u || !Is_ValidUtf8DisplayText(profile.sourceMaterial) ||
            profile.family != "source.character.monster-pbr-masked.v1" ||
            !SourceCharacterMaterial::Configure(profile.family, profile.parameters, packed)) return false;
        for (const auto& [name, values] : profile.parameters)
            for (const auto value : values)
                if (!std::isfinite(value) || std::abs(value) > 1000000.f) return false;
        const uint32_t required = packed.baseTextureMask | packed.lightTextureMask;
        uint32_t supplied = 0u;
        for (const auto& texture : profile.textures)
        {
            if (texture.expressionIndex >= Engine::SOURCE_CHARACTER_TEXTURE_COUNT ||
                !Is_ResourcePath(texture.assetId, false)) return false;
            const auto bit = 1u << texture.expressionIndex;
            if ((supplied & bit) != 0u || (required & bit) == 0u) return false;
            supplied |= bit;
        }
        if (supplied != required) return false;
        if (out) *out = packed;
        return true;
    }

	/* Authored rows must agree with the seeded fields they replace, so a
	   document never carries two different answers for the emission count. */
	bool_t Is_ValidEmissionList(const WORLD_SEQUENCE_OBJECT_MOTION& motion)
	{
		if (motion.emissions.empty()) return true;
		if (motion.emissions.size() > 128u || motion.count != motion.emissions.size() ||
			0u != motion.intervalMs || 0.f != motion.spreadDegrees) return false;
		for (const auto& emission : motion.emissions)
		{
			if (!Is_BoundedFloat3(emission.positionOffset) || !std::isfinite(emission.yawDegrees) ||
				emission.yawDegrees < -36000.f || emission.yawDegrees > 36000.f ||
				emission.startDelayMs > CWorldSequenceDocument::MAX_DURATION_MS) return false;
		}
		return true;
	}

	bool_t Read_Uint32(
		const DATA_JSON_VALUE* value,
		uint32_t& outValue,
		const uint32_t maximum = UINT32_MAX)
	{
		if (!Is_IntegerNumber(value) || value->Get_Number() < 0.0 ||
			value->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(value->Get_Number());
		return true;
	}

	bool_t Read_FiniteFloat(const DATA_JSON_VALUE* value, f32_t& outValue)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Read_Float3(const DATA_JSON_VALUE* value, float3_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		return Read_FiniteFloat(&values[0], outValue.x) &&
			Read_FiniteFloat(&values[1], outValue.y) &&
			Read_FiniteFloat(&values[2], outValue.z);
	}

	bool_t Read_Quaternion(const DATA_JSON_VALUE* value, float4_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			4u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		if (!Read_FiniteFloat(&values[0], outValue.x) ||
			!Read_FiniteFloat(&values[1], outValue.y) ||
			!Read_FiniteFloat(&values[2], outValue.z) ||
			!Read_FiniteFloat(&values[3], outValue.w))
		{
			return false;
		}
		const vector_t raw = XMLoadFloat4(&outValue);
		const f32_t length = XMVectorGetX(XMVector4Length(raw));
		if (!std::isfinite(length) ||
			std::abs(length - 1.f) > 0.001f)
			return false;
		if (outValue.w < 0.f)
		{
			outValue.x = -outValue.x;
			outValue.y = -outValue.y;
			outValue.z = -outValue.z;
			outValue.w = -outValue.w;
		}
		return true;
	}

	bool_t Parse_Uint64String(const DATA_JSON_VALUE* value, uint64_t& outValue)
	{
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		const std::string& text = value->Get_String();
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Parse_Uint64Text(const std::string& text, uint64_t& outValue)
	{
		if (text.empty())
			return false;
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Is_FiniteTransform(const WORLD_SEQUENCE_TRANSFORM_KEY& key)
	{
		const auto finiteBounded = [](const f32_t value)
		{
			return std::isfinite(value) && std::abs(value) <= MAX_COMPONENT;
		};
		if (!finiteBounded(key.positionOffset.x) ||
			!finiteBounded(key.positionOffset.y) ||
			!finiteBounded(key.positionOffset.z) ||
			!finiteBounded(key.scaleMultiplier.x) ||
			!finiteBounded(key.scaleMultiplier.y) ||
			!finiteBounded(key.scaleMultiplier.z) ||
			key.scaleMultiplier.x < MIN_SCALE ||
			key.scaleMultiplier.y < MIN_SCALE ||
			key.scaleMultiplier.z < MIN_SCALE)
		{
			return false;
		}
		const vector_t quaternion = XMLoadFloat4(&key.rotationQuaternion);
		const f32_t length = XMVectorGetX(XMVector4Length(quaternion));
		return std::isfinite(length) && std::abs(length - 1.f) <= 0.001f &&
			key.rotationQuaternion.w >= 0.f;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) && !existsError &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CWorldSequenceDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus)
{
	CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Load");
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		if (existsError)
		{
			outStatus = "Could not inspect world sequence document";
			return false;
		}
		Reset_Empty(expectedAreaId);
		outStatus = "No world sequence document; starting empty";
		return true;
	}
	if (!std::filesystem::is_regular_file(path, existsError) || existsError)
	{
		outStatus = "World sequence document is not a regular file";
		return false;
	}
	const uintmax_t fileBytes = std::filesystem::file_size(path, existsError);
	if (existsError || fileBytes > MAX_DOCUMENT_BYTES)
	{
		outStatus = existsError ?
			"Could not inspect world sequence document size" :
			"World sequence document exceeds the 16 MiB parse limit";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open world sequence document file";
		return false;
	}
	std::string text;
	try
	{
		text.resize(static_cast<size_t>(fileBytes));
	}
	catch (const std::bad_alloc&)
	{
		outStatus = "Could not allocate bounded world sequence input";
		return false;
	}
	if (!text.empty())
		input.read(text.data(), static_cast<std::streamsize>(text.size()));
	if (input.bad() || input.gcount() != static_cast<std::streamsize>(text.size()) ||
		std::char_traits<char_t>::eof() != input.peek())
	{
		outStatus = "World sequence document changed or failed while reading";
		return false;
	}
	DATA_JSON_VALUE root;
	std::string parseError;
	bool_t parsed = false;
	{
		CProfilerScope parseScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Parse");
		parsed = CDataJson::Parse(text, root, parseError);
	}
	if (!parsed ||
		!Is_ObjectShape(root,
			{ "schema", "formatVersion", "areaId", "revision",
			  "templates", "instances" }, { "objectResources" }))
	{
		outStatus = "World sequence JSON root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areaId = root.Find("areaId");
	const DATA_JSON_VALUE* revision = root.Find("revision");
	const DATA_JSON_VALUE* templates = root.Find("templates");
	const DATA_JSON_VALUE* instances = root.Find("instances");
	uint32_t parsedFormatVersion = 0;
	uint32_t parsedRevision = 0;
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != SCHEMA ||
		!Read_Uint32(version, parsedFormatVersion) ||
		(parsedFormatVersion < LEGACY_FORMAT_VERSION || parsedFormatVersion > FORMAT_VERSION) ||
		nullptr == areaId || !areaId->Is_String() ||
		areaId->Get_String() != expectedAreaId ||
		!Read_Uint32(revision, parsedRevision) || 0u == parsedRevision ||
		nullptr == templates || !templates->Is_Array() ||
		templates->Get_Array().size() > MAX_TEMPLATE_COUNT ||
		nullptr == instances || !instances->Is_Array() ||
		instances->Get_Array().size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence header is invalid or belongs to another Area";
		return false;
	}

	CWorldSequenceDocument staged;
	staged.m_AreaId = expectedAreaId;
	staged.m_iRevision = parsedRevision;
	const DATA_JSON_VALUE* objects = root.Find("objectResources");
	if ((parsedFormatVersion < 3u && nullptr != objects) ||
		(parsedFormatVersion == 3u && (nullptr == objects || !objects->Is_Array() ||
			objects->Get_Array().size() > MAX_INSTANCE_COUNT)))
	{
		outStatus = "World object resource list is invalid";
		return false;
	}
	if (nullptr != objects)
	{
		for (const DATA_JSON_VALUE& row : objects->Get_Array())
		{
			WORLD_SEQUENCE_OBJECT_RESOURCE object;
			if (!Is_ObjectShape(row, { "objectId", "displayName", "modelAssetId", "modelPreScale",
				"animated", "scale" }, { "diffuseTextureAssetId", "sequenceInstanceId", "anchorKind", "defaultMotionInstanceId", "anchorBossArchetypeId", "anchorBone", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings", "motionInstanceIds" }) ||
				!row.Find("objectId")->Is_String() || !row.Find("displayName")->Is_String() ||
				!row.Find("modelAssetId")->Is_String() || !row.Find("animated")->Is_Boolean() ||
				!Read_FiniteFloat(row.Find("modelPreScale"), object.modelPreScale) ||
				!Read_Float3(row.Find("scale"), object.scale))
			{
				outStatus = "World object resource fields are invalid";
				return false;
			}
			object.objectId = row.Find("objectId")->Get_String();
			object.displayName = row.Find("displayName")->Get_String();
			object.modelAssetId = row.Find("modelAssetId")->Get_String();
			object.animated = row.Find("animated")->Get_Boolean();
			if (const auto* members = row.Find("motionInstanceIds"))
			{
				if (!members->Is_Array() || members->Get_Array().empty() || members->Get_Array().size() > 32u)
				{ outStatus = "Object group requires 1..32 motion instance IDs"; return false; }
				for (const auto& member : members->Get_Array())
				{
					if (!member.Is_String()) { outStatus = "Object group member ID must be text"; return false; }
					object.motionInstanceIds.push_back(member.Get_String());
				}
			}
			if (const auto* motion = row.Find("defaultMotionInstanceId"))
			{
				if (!motion->Is_String()) { outStatus = "Default Motion instance ID must be text"; return false; }
				object.defaultMotionInstanceId = motion->Get_String();
			}
			if (const auto* anchor = row.Find("anchorKind"))
			{
				if (!anchor->Is_String()) { outStatus = "World object resource anchor must be WORLD, PLAYER or BOSS"; return false; }
				object.anchorKind = anchor->Get_String();
			}
			for (const char_t* key : { "anchorBossArchetypeId", "anchorBone" })
			{
				const auto* field = row.Find(key);
				if (!field) continue;
				if (!field->Is_String()) { outStatus = "World object boss anchor fields must be text"; return false; }
				(std::string(key) == "anchorBone" ? object.anchorBone : object.anchorBossArchetypeId) = field->Get_String();
			}
			for (const char_t* key : { "diffuseTextureAssetId", "sequenceInstanceId" })
			{
				const auto* field = row.Find(key);
				if (nullptr == field) continue;
				if (!field->Is_String()) { outStatus = "Invalid world object optional path/reference"; return false; }
				(std::string(key) == "sequenceInstanceId" ? object.sequenceInstanceId :
					object.diffuseTextureAssetId) = field->Get_String();
			}
            if (const auto* source = row.Find("materialSourceModelAssetId"))
            {
                if (!source->Is_String() || !Is_ResourcePath(source->Get_String(), true))
                { outStatus = "Invalid world object material source model: " + object.objectId; return false; }
                object.materialSourceModelAssetId = source->Get_String();
            }
            if (const auto* bindings = row.Find("mapMaterialBindings"))
            {
                if (!bindings->Is_Array() || bindings->Get_Array().size() > 64u)
                { outStatus = "Invalid world object map material bindings"; return false; }
                for (const auto& binding : bindings->Get_Array())
                {
                    if (!Is_ObjectShape(binding, { "materialName", "sourceAssetId", "sourceMaterialName" }, { "diffuseTextureAssetId" }) ||
                        !binding.Find("materialName")->Is_String() || !binding.Find("sourceAssetId")->Is_String() ||
                        !binding.Find("sourceMaterialName")->Is_String())
                    { outStatus = "Invalid world object map material binding"; return false; }
                    WORLD_SEQUENCE_MAP_MATERIAL_BINDING material;
                    material.materialName = binding.Find("materialName")->Get_String();
                    material.sourceAssetId = binding.Find("sourceAssetId")->Get_String();
                    material.sourceMaterialName = binding.Find("sourceMaterialName")->Get_String();
                    if (const auto* diffuse = binding.Find("diffuseTextureAssetId"))
                    {
                        if (!diffuse->Is_String() || !Is_ResourcePath(diffuse->Get_String(), false))
                        { outStatus = "Invalid map material diffuse texture"; return false; }
                        material.diffuseTextureAssetId = diffuse->Get_String();
                    }
                    object.mapMaterialBindings.push_back(std::move(material));
                }
            }
            if (const auto* value = row.Find("materialProfile"))
            {
                WORLD_SEQUENCE_MATERIAL_PROFILE profile;
                if (!Is_ExactObject(*value, { "materialName", "sourceMaterial", "family", "parameters", "textures" }) ||
                    !value->Find("materialName")->Is_String() || !value->Find("sourceMaterial")->Is_String() ||
                    !value->Find("family")->Is_String() || !value->Find("textures")->Is_Array() ||
                    !SourceCharacterMaterial::Read(*value->Find("parameters"), profile.parameters))
                { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
                profile.materialName = value->Find("materialName")->Get_String();
                profile.sourceMaterial = value->Find("sourceMaterial")->Get_String();
                profile.family = value->Find("family")->Get_String();
                for (const auto& texture : value->Find("textures")->Get_Array())
                {
                    WORLD_SEQUENCE_MATERIAL_TEXTURE input;
                    if (!Is_ExactObject(texture, { "expressionIndex", "assetId", "colorSpace" }) ||
                        !Read_Uint32(texture.Find("expressionIndex"), input.expressionIndex) ||
                        !texture.Find("assetId")->Is_String() || !texture.Find("colorSpace")->Is_String() ||
                        (texture.Find("colorSpace")->Get_String() != "srgb" && texture.Find("colorSpace")->Get_String() != "linear"))
                    { outStatus = "Invalid world object material texture: " + object.objectId; return false; }
                    input.assetId = texture.Find("assetId")->Get_String();
                    input.srgb = texture.Find("colorSpace")->Get_String() == "srgb";
                    profile.textures.push_back(std::move(input));
                }
                if (!Validate_MaterialProfile(profile))
                { outStatus = "World object material input contract failed: " + object.objectId; return false; }
                object.materialProfile = std::move(profile);
            }
			staged.m_ObjectResources.push_back(std::move(object));
		}
	}
	for (const DATA_JSON_VALUE& templateValue : templates->Get_Array())
	{
		const bool_t validTemplateShape =
			LEGACY_FORMAT_VERSION == parsedFormatVersion ?
			Is_ExactObject(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks" }) :
			Is_ObjectShape(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks", "animationTracks" }, { "objectMotion", "effectTracks" });
		if (!validTemplateShape)
		{
			outStatus = "World sequence template shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* sequenceId = templateValue.Find("sequenceId");
		const DATA_JSON_VALUE* displayName = templateValue.Find("displayName");
		const DATA_JSON_VALUE* category = templateValue.Find("category");
		const DATA_JSON_VALUE* interpolation = templateValue.Find("interpolation");
		const DATA_JSON_VALUE* tracks = templateValue.Find("tracks");
		const DATA_JSON_VALUE* animationTracks =
			templateValue.Find("animationTracks");
		WORLD_SEQUENCE_TEMPLATE parsedTemplate;
		if (nullptr == sequenceId || !sequenceId->Is_String() ||
			nullptr == displayName || !displayName->Is_String() ||
			nullptr == category || !category->Is_String() ||
			!Read_Uint32(templateValue.Find("durationMs"),
				parsedTemplate.durationMs, MAX_DURATION_MS) ||
			nullptr == interpolation || !interpolation->Is_String() ||
			!Try_ParseInterpolation(interpolation->Get_String(),
				parsedTemplate.interpolation) ||
			nullptr == tracks || !tracks->Is_Array() ||
			tracks->Get_Array().size() > MAX_TRACK_COUNT ||
			(2u <= parsedFormatVersion &&
				(nullptr == animationTracks || !animationTracks->Is_Array() ||
					animationTracks->Get_Array().size() > MAX_TRACK_COUNT ||
					tracks->Get_Array().size() +
						animationTracks->Get_Array().size() > MAX_TRACK_COUNT)))
		{
			outStatus = "World sequence template fields are invalid";
			return false;
		}
		parsedTemplate.sequenceId = sequenceId->Get_String();
		parsedTemplate.displayName = displayName->Get_String();
		parsedTemplate.category = category->Get_String();
		if (const DATA_JSON_VALUE* motion = templateValue.Find("objectMotion"))
		{
			auto& value = parsedTemplate.objectMotion;
			if (parsedFormatVersion < 3u || !Is_ObjectShape(*motion,
				{ "velocity", "acceleration", "angularVelocityDegrees", "revolutionDegreesPerSecond",
				  "revolutionOffset", "count", "intervalMs", "spreadDegrees", "seed" }, { "spawnHalfExtents", "emissions" }) ||
				!Read_Float3(motion->Find("velocity"), value.velocity) ||
				!Read_Float3(motion->Find("acceleration"), value.acceleration) ||
				!Read_Float3(motion->Find("angularVelocityDegrees"), value.angularVelocityDegrees) ||
				!Read_Float3(motion->Find("revolutionDegreesPerSecond"), value.revolutionDegreesPerSecond) ||
				!Read_Float3(motion->Find("revolutionOffset"), value.revolutionOffset) ||
				(motion->Find("spawnHalfExtents") && !Read_Float3(motion->Find("spawnHalfExtents"), value.spawnHalfExtents)) ||
				!Read_Uint32(motion->Find("count"), value.count, 128u) ||
				!Read_Uint32(motion->Find("intervalMs"), value.intervalMs, MAX_DURATION_MS) ||
				!Read_FiniteFloat(motion->Find("spreadDegrees"), value.spreadDegrees) ||
				!Read_Uint32(motion->Find("seed"), value.seed))
			{
				outStatus = "World object motion is invalid";
				return false;
			}
			if (const DATA_JSON_VALUE* emissions = motion->Find("emissions"))
			{
				if (!emissions->Is_Array() || emissions->Get_Array().size() > 128u)
				{
					outStatus = "World object emissions are invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& emissionValue : emissions->Get_Array())
				{
					WORLD_SEQUENCE_OBJECT_EMISSION emission;
					if (!Is_ExactObject(emissionValue, { "positionOffset", "yawDegrees", "startDelayMs" }) ||
						!Read_Float3(emissionValue.Find("positionOffset"), emission.positionOffset) ||
						!Read_FiniteFloat(emissionValue.Find("yawDegrees"), emission.yawDegrees) ||
						!Read_Uint32(emissionValue.Find("startDelayMs"), emission.startDelayMs, MAX_DURATION_MS))
					{
						outStatus = "World object emission row is invalid";
						return false;
					}
					value.emissions.push_back(emission);
				}
			}
		}

		for (const DATA_JSON_VALUE& trackValue : tracks->Get_Array())
		{
			if (!Is_ExactObject(trackValue, { "slotId", "keys" }))
			{
				outStatus = "World sequence track shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
			const DATA_JSON_VALUE* keys = trackValue.Find("keys");
			WORLD_SEQUENCE_TRACK parsedTrack;
			if (nullptr == slotId || !slotId->Is_String() ||
				nullptr == keys || !keys->Is_Array() ||
				keys->Get_Array().size() > MAX_KEY_COUNT)
			{
				outStatus = "World sequence track fields are invalid";
				return false;
			}
			parsedTrack.slotId = slotId->Get_String();
			for (const DATA_JSON_VALUE& keyValue : keys->Get_Array())
			{
				if (!Is_ExactObject(keyValue,
					{ "timeMs", "positionOffset", "rotationQuaternion",
					  "scaleMultiplier", "visible" }))
				{
					outStatus = "World sequence key shape is invalid";
					return false;
				}
				WORLD_SEQUENCE_TRANSFORM_KEY parsedKey;
				const DATA_JSON_VALUE* visible = keyValue.Find("visible");
				if (!Read_Uint32(keyValue.Find("timeMs"), parsedKey.timeMs,
						MAX_DURATION_MS) ||
					!Read_Float3(keyValue.Find("positionOffset"),
						parsedKey.positionOffset) ||
					!Read_Quaternion(keyValue.Find("rotationQuaternion"),
						parsedKey.rotationQuaternion) ||
					!Read_Float3(keyValue.Find("scaleMultiplier"),
						parsedKey.scaleMultiplier) ||
					nullptr == visible || !visible->Is_Boolean())
				{
					outStatus = "World sequence key fields are invalid";
					return false;
				}
				parsedKey.visible = visible->Get_Boolean();
				parsedTrack.keys.push_back(parsedKey);
			}
			parsedTemplate.tracks.push_back(std::move(parsedTrack));
		}
		if (2u <= parsedFormatVersion)
		{
			for (const DATA_JSON_VALUE& trackValue :
				animationTracks->Get_Array())
			{
				if (!Is_ObjectShape(trackValue,
						{ "slotId", "clipName", "playbackRate", "loop",
						  "holdLastFrame" }, { "startMs", "displayName" }))
				{
					outStatus = "World sequence animation track shape is invalid";
					return false;
				}
				const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
				const DATA_JSON_VALUE* clipName = trackValue.Find("clipName");
				const DATA_JSON_VALUE* trackDisplayName = trackValue.Find("displayName");
				const DATA_JSON_VALUE* loop = trackValue.Find("loop");
				const DATA_JSON_VALUE* holdLastFrame =
					trackValue.Find("holdLastFrame");
				WORLD_SEQUENCE_ANIMATION_TRACK parsedTrack;
				if (nullptr == slotId || !slotId->Is_String() ||
					nullptr == clipName || !clipName->Is_String() ||
					(nullptr != trackDisplayName && !trackDisplayName->Is_String()) ||
					!Read_FiniteFloat(trackValue.Find("playbackRate"),
						parsedTrack.playbackRate) ||
					nullptr == loop || !loop->Is_Boolean() ||
					nullptr == holdLastFrame || !holdLastFrame->Is_Boolean())
				{
					outStatus = "World sequence animation track fields are invalid";
					return false;
				}
				const DATA_JSON_VALUE* startMs = trackValue.Find("startMs");
				if (nullptr != startMs &&
					!Read_Uint32(startMs, parsedTrack.startMs, MAX_DURATION_MS))
				{
					outStatus = "World sequence animation track start is invalid";
					return false;
				}
				parsedTrack.slotId = slotId->Get_String();
				parsedTrack.clipName = clipName->Get_String();
				if (nullptr != trackDisplayName)
					parsedTrack.displayName = trackDisplayName->Get_String();
				parsedTrack.loop = loop->Get_Boolean();
				parsedTrack.holdLastFrame = holdLastFrame->Get_Boolean();
				parsedTemplate.animationTracks.push_back(std::move(parsedTrack));
			}
		}
		if (const auto* effects = templateValue.Find("effectTracks"))
		{
			if (parsedFormatVersion < 3u || !effects->Is_Array() || effects->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object effectTracks must be a bounded v3 array"; return false; }
			for (const auto& row : effects->Get_Array())
			{
				WORLD_SEQUENCE_EFFECT_TRACK effect;
				if (!Is_ExactObject(row, { "effectTrackId", "slotId", "resourceKind", "resourceId",
					"timing", "startMs", "durationMs", "positionOffset", "rotationDegrees", "scale" }))
				{ outStatus = "World Object effect track shape is invalid"; return false; }
				for (const char* key : { "effectTrackId", "slotId", "resourceKind", "resourceId", "timing" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object effect identity must be text"; return false; }
				effect.effectTrackId = row.Find("effectTrackId")->Get_String();
				effect.slotId = row.Find("slotId")->Get_String();
				effect.resourceKind = row.Find("resourceKind")->Get_String();
				effect.resourceId = row.Find("resourceId")->Get_String();
				effect.timing = row.Find("timing")->Get_String();
				if (!Read_Uint32(row.Find("startMs"), effect.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), effect.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), effect.positionOffset) ||
					!Read_Float3(row.Find("rotationDegrees"), effect.rotationDegrees) ||
					!Read_Float3(row.Find("scale"), effect.scale))
				{ outStatus = "World Object effect timing or transform is invalid"; return false; }
				parsedTemplate.effectTracks.push_back(std::move(effect));
			}
		}

		staged.m_Templates.push_back(std::move(parsedTemplate));
	}

	for (const DATA_JSON_VALUE& instanceValue : instances->Get_Array())
	{
		if (!Is_ObjectShape(instanceValue,
			{ "instanceId", "templateId", "enabled", "startDelayMs",
			  "playbackSpeed", "bindings" }, { "anchorKind", "position", "motionEnd", "nextMotionId", "walkableSurface" }))
		{
			outStatus = "World sequence instance shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* instanceId = instanceValue.Find("instanceId");
		const DATA_JSON_VALUE* templateId = instanceValue.Find("templateId");
		const DATA_JSON_VALUE* enabled = instanceValue.Find("enabled");
		const DATA_JSON_VALUE* bindings = instanceValue.Find("bindings");
		WORLD_SEQUENCE_INSTANCE parsedInstance;
		if (nullptr == instanceId || !instanceId->Is_String() ||
			nullptr == templateId || !templateId->Is_String() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!Read_Uint32(instanceValue.Find("startDelayMs"),
				parsedInstance.startDelayMs, MAX_DURATION_MS) ||
			!Read_FiniteFloat(instanceValue.Find("playbackSpeed"),
				parsedInstance.playbackSpeed) ||
			nullptr == bindings || !bindings->Is_Array() ||
			bindings->Get_Array().size() > MAX_TRACK_COUNT)
		{
			outStatus = "World sequence instance fields are invalid";
			return false;
		}
		parsedInstance.instanceId = instanceId->Get_String();
		parsedInstance.templateId = templateId->Get_String();
		parsedInstance.enabled = enabled->Get_Boolean();
		const DATA_JSON_VALUE* anchor = instanceValue.Find("anchorKind");
		const DATA_JSON_VALUE* position = instanceValue.Find("position");
		if ((parsedFormatVersion < 3u && (anchor || position)) ||
			(anchor && !anchor->Is_String()) ||
			(position && !Read_Float3(position, parsedInstance.position)))
		{ outStatus = "World object instance anchor is invalid"; return false; }
		if (anchor) parsedInstance.anchorKind = anchor->Get_String();
		const DATA_JSON_VALUE* motionEnd = instanceValue.Find("motionEnd");
		const DATA_JSON_VALUE* nextMotionId = instanceValue.Find("nextMotionId");
		if ((parsedFormatVersion < 3u && (motionEnd || nextMotionId)) ||
			(motionEnd && (!motionEnd->Is_String() ||
				!Try_ParseMotionEnd(motionEnd->Get_String(), parsedInstance.motionEnd))) ||
			(nextMotionId && !nextMotionId->Is_String()))
		{ outStatus = "World object motion completion is invalid"; return false; }
		if (nextMotionId) parsedInstance.nextMotionId = nextMotionId->Get_String();
		if (const DATA_JSON_VALUE* surface = instanceValue.Find("walkableSurface"))
		{
			WORLD_SEQUENCE_WALKABLE_SURFACE parsed;
			if (parsedFormatVersion < 3u || !Is_ExactObject(*surface, { "radiusM", "localHeightM" }) ||
				!Read_FiniteFloat(surface->Find("radiusM"), parsed.radiusM) ||
				!Read_FiniteFloat(surface->Find("localHeightM"), parsed.localHeightM))
			{ outStatus = "Invalid walkable surface fields: " + parsedInstance.instanceId; return false; }
			parsedInstance.walkableSurface = parsed;
		}

		for (const DATA_JSON_VALUE& bindingValue : bindings->Get_Array())
		{
			const bool_t validBindingShape =
				LEGACY_FORMAT_VERSION == parsedFormatVersion ?
				Is_ExactObject(bindingValue, { "slotId", "placementId" }) :
				Is_ExactObject(bindingValue,
					{ "slotId", "targetKind", "targetId" });
			if (!validBindingShape)
			{
				outStatus = "World sequence binding shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = bindingValue.Find("slotId");
			WORLD_SEQUENCE_BINDING parsedBinding;
			if (nullptr == slotId || !slotId->Is_String())
			{
				outStatus = "World sequence binding fields are invalid";
				return false;
			}
			parsedBinding.slotId = slotId->Get_String();
			if (LEGACY_FORMAT_VERSION == parsedFormatVersion)
			{
				uint64_t placementId = 0;
				if (!Parse_Uint64String(bindingValue.Find("placementId"),
					placementId))
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetKind =
					WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
				parsedBinding.targetId = std::to_string(placementId);
			}
			else
			{
				const DATA_JSON_VALUE* targetKind =
					bindingValue.Find("targetKind");
				const DATA_JSON_VALUE* targetId = bindingValue.Find("targetId");
				if (nullptr == targetKind || !targetKind->Is_String() ||
					!Try_ParseTargetKind(targetKind->Get_String(),
						parsedBinding.targetKind) ||
					nullptr == targetId || !targetId->Is_String())
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetId = targetId->Get_String();
				if (parsedFormatVersion < 3u && parsedBinding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object resource binding requires formatVersion 3"; return false; }
			}
			parsedInstance.bindings.push_back(std::move(parsedBinding));
		}
		staged.m_Instances.push_back(std::move(parsedInstance));
	}

	if (!staged.Validate(availablePlacements, availableDeployPlacements,
		outStatus))
		return false;
	*this = std::move(staged);
	outStatus = "Loaded world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Save(
	const std::filesystem::path& path,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	if (!Validate(availablePlacements, availableDeployPlacements, outStatus))
		return false;
	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create world sequence authoring directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create world sequence temporary file";
		return false;
	}
	output << std::setprecision(9)
		<< "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(m_AreaId) << "\",\n"
		<< "  \"revision\": " << m_iRevision << ",\n"
		<< "  \"objectResources\": [";
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& object = m_ObjectResources[index];
		output << (index == 0u ? "\n" : ",\n") << "    {\n"
			<< "      \"objectId\": \"" << CDataJson::Escape(object.objectId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(object.displayName) << "\",\n"
			<< "      \"modelAssetId\": \"" << CDataJson::Escape(object.modelAssetId) << "\",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(object.anchorKind) << "\",\n"
			<< "      \"diffuseTextureAssetId\": \"" << CDataJson::Escape(object.diffuseTextureAssetId) << "\",\n"
			<< "      \"modelPreScale\": " << object.modelPreScale << ",\n"
			<< "      \"animated\": " << (object.animated ? "true" : "false") << ",\n"
			<< "      \"scale\": [" << object.scale.x << ", " << object.scale.y << ", " << object.scale.z << "],\n"
			<< "      \"sequenceInstanceId\": \"" << CDataJson::Escape(object.sequenceInstanceId) << "\"";
		if (object.anchorKind == "BOSS")
			output << ",\n      \"anchorBossArchetypeId\": \"" << CDataJson::Escape(object.anchorBossArchetypeId)
				<< "\",\n      \"anchorBone\": \"" << CDataJson::Escape(object.anchorBone) << "\"";
		if (!object.defaultMotionInstanceId.empty())
			output << ",\n      \"defaultMotionInstanceId\": \"" << CDataJson::Escape(object.defaultMotionInstanceId) << "\"";
		if (!object.motionInstanceIds.empty())
		{
			output << ",\n      \"motionInstanceIds\": [";
			for (size_t i = 0; i < object.motionInstanceIds.size(); ++i)
				output << (i ? ", " : "") << "\"" << CDataJson::Escape(object.motionInstanceIds[i]) << "\"";
			output << "]";
		}
        if (!object.materialSourceModelAssetId.empty())
            output << ",\n      \"materialSourceModelAssetId\": \"" << CDataJson::Escape(object.materialSourceModelAssetId) << "\"";
        if (!object.mapMaterialBindings.empty())
        {
            output << ",\n      \"mapMaterialBindings\": [";
            for (size_t i = 0; i < object.mapMaterialBindings.size(); ++i)
            {
                const auto& binding = object.mapMaterialBindings[i];
                output << (i ? "," : "") << "\n        {\"materialName\": \"" << CDataJson::Escape(binding.materialName)
                    << "\", \"sourceAssetId\": \"" << CDataJson::Escape(binding.sourceAssetId)
                    << "\", \"sourceMaterialName\": \"" << CDataJson::Escape(binding.sourceMaterialName) << "\"";
                if (!binding.diffuseTextureAssetId.empty()) output << ", \"diffuseTextureAssetId\": \"" << CDataJson::Escape(binding.diffuseTextureAssetId) << "\"";
                output << "}";
            }
            output << "\n      ]";
        }
        if (object.materialProfile)
        {
            const auto& profile = *object.materialProfile;
            output << ",\n      \"materialProfile\": {\n        \"materialName\": \"" << CDataJson::Escape(profile.materialName)
                << "\",\n        \"sourceMaterial\": \"" << CDataJson::Escape(profile.sourceMaterial)
                << "\",\n        \"family\": \"" << CDataJson::Escape(profile.family) << "\",\n        \"parameters\": {";
            bool first = true;
            for (const auto& [name, value] : profile.parameters)
            {
                output << (first ? "" : ",") << "\n          \"" << CDataJson::Escape(name) << "\": ["
                    << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << "]";
                first = false;
            }
            output << "\n        },\n        \"textures\": [";
            for (size_t i = 0u; i < profile.textures.size(); ++i)
            {
                const auto& texture = profile.textures[i];
                output << (i ? "," : "") << "\n          {\"expressionIndex\": " << texture.expressionIndex
                    << ", \"assetId\": \"" << CDataJson::Escape(texture.assetId)
                    << "\", \"colorSpace\": \"" << (texture.srgb ? "srgb" : "linear") << "\"}";
            }
            output << "\n        ]\n      }";
        }
		output << "\n    }";
	}
	output << (m_ObjectResources.empty() ? "],\n" : "\n  ],\n")
		<< "  \"templates\": [";
	for (size_t templateIndex = 0; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& value = m_Templates[templateIndex];
		output << (0u == templateIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"sequenceId\": \"" << CDataJson::Escape(value.sequenceId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(value.displayName) << "\",\n"
			<< "      \"category\": \"" << CDataJson::Escape(value.category) << "\",\n"
			<< "      \"durationMs\": " << value.durationMs << ",\n"
			<< "      \"interpolation\": \"" << Interpolation_ToString(value.interpolation) << "\",\n"
			<< "      \"objectMotion\": {\n";
		const auto& motion = value.objectMotion;
		const auto writeVector = [&output](const char_t* name, const float3_t& vector)
		{
			output << "        \"" << name << "\": [" << vector.x << ", " << vector.y << ", " << vector.z << "],\n";
		};
		writeVector("velocity", motion.velocity);
		writeVector("acceleration", motion.acceleration);
		writeVector("angularVelocityDegrees", motion.angularVelocityDegrees);
		writeVector("revolutionDegreesPerSecond", motion.revolutionDegreesPerSecond);
		writeVector("revolutionOffset", motion.revolutionOffset);
		if (motion.spawnHalfExtents.x != 0.f || motion.spawnHalfExtents.y != 0.f || motion.spawnHalfExtents.z != 0.f)
			writeVector("spawnHalfExtents", motion.spawnHalfExtents);
		output << "        \"count\": " << motion.count << ", \"intervalMs\": " << motion.intervalMs
			<< ", \"spreadDegrees\": " << motion.spreadDegrees << ", \"seed\": " << motion.seed;
		if (!motion.emissions.empty())
		{
			output << ",\n        \"emissions\": [";
			for (size_t emissionIndex = 0; emissionIndex < motion.emissions.size(); ++emissionIndex)
			{
				const auto& emission = motion.emissions[emissionIndex];
				output << (0u == emissionIndex ? "\n" : ",\n")
					<< "          {\"positionOffset\": [" << emission.positionOffset.x << ", " << emission.positionOffset.y
					<< ", " << emission.positionOffset.z << "], \"yawDegrees\": " << emission.yawDegrees
					<< ", \"startDelayMs\": " << emission.startDelayMs << "}";
			}
			output << "\n        ]";
		}
		output << "\n      },\n"
			<< "      \"tracks\": [";
		for (size_t trackIndex = 0; trackIndex < value.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& track = value.tracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        {\n"
				<< "          \"slotId\": \"" << CDataJson::Escape(track.slotId) << "\",\n"
				<< "          \"keys\": [";
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				output << (0u == keyIndex ? "\n" : ",\n")
					<< "            {\n"
					<< "              \"timeMs\": " << key.timeMs << ",\n"
					<< "              \"positionOffset\": [" << key.positionOffset.x << ", "
					<< key.positionOffset.y << ", " << key.positionOffset.z << "],\n"
					<< "              \"rotationQuaternion\": [" << key.rotationQuaternion.x << ", "
					<< key.rotationQuaternion.y << ", " << key.rotationQuaternion.z << ", "
					<< key.rotationQuaternion.w << "],\n"
					<< "              \"scaleMultiplier\": [" << key.scaleMultiplier.x << ", "
					<< key.scaleMultiplier.y << ", " << key.scaleMultiplier.z << "],\n"
					<< "              \"visible\": " << (key.visible ? "true" : "false") << "\n"
					<< "            }";
			}
			output << (track.keys.empty() ? "]\n" : "\n          ]\n")
				<< "        }";
		}
		output << (value.tracks.empty() ? "],\n" : "\n      ],\n")
			<< "      \"animationTracks\": [";
		for (size_t trackIndex = 0;
			trackIndex < value.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& track =
				value.animationTracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \""
				<< CDataJson::Escape(track.slotId)
				<< "\", \"clipName\": \""
				<< CDataJson::Escape(track.clipName)
				<< "\"";
			if (!track.displayName.empty())
				output << ", \"displayName\": \"" << CDataJson::Escape(track.displayName) << "\"";
			output << ", \"startMs\": " << track.startMs
					<< ", \"playbackRate\": " << track.playbackRate
				<< ", \"loop\": " << (track.loop ? "true" : "false")
				<< ", \"holdLastFrame\": "
				<< (track.holdLastFrame ? "true" : "false") << " }";
		}
		output << (value.animationTracks.empty() ? "]" : "\n      ]");
		if (!value.effectTracks.empty())
		{
			output << ",\n      \"effectTracks\": [";
			for (size_t index = 0; index < value.effectTracks.size(); ++index)
			{
				const auto& effect = value.effectTracks[index];
				output << (index ? ",\n" : "\n") << "        { \"effectTrackId\": \"" << CDataJson::Escape(effect.effectTrackId)
					<< "\", \"slotId\": \"" << CDataJson::Escape(effect.slotId)
					<< "\", \"resourceKind\": \"" << effect.resourceKind
					<< "\", \"resourceId\": \"" << CDataJson::Escape(effect.resourceId)
					<< "\", \"timing\": \"" << effect.timing
					<< "\", \"startMs\": " << effect.startMs << ", \"durationMs\": " << effect.durationMs
					<< ", \"positionOffset\": [" << effect.positionOffset.x << ", " << effect.positionOffset.y << ", " << effect.positionOffset.z
					<< "], \"rotationDegrees\": [" << effect.rotationDegrees.x << ", " << effect.rotationDegrees.y << ", " << effect.rotationDegrees.z
					<< "], \"scale\": [" << effect.scale.x << ", " << effect.scale.y << ", " << effect.scale.z << "] }";
			}
			output << "\n      ]";
		}
		output << "\n    }";
	}
	output << (m_Templates.empty() ? "],\n" : "\n  ],\n")
		<< "  \"instances\": [";
	for (size_t instanceIndex = 0; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& value = m_Instances[instanceIndex];
		output << (0u == instanceIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"instanceId\": \"" << CDataJson::Escape(value.instanceId) << "\",\n"
			<< "      \"templateId\": \"" << CDataJson::Escape(value.templateId) << "\",\n"
			<< "      \"enabled\": " << (value.enabled ? "true" : "false") << ",\n"
			<< "      \"startDelayMs\": " << value.startDelayMs << ",\n"
			<< "      \"playbackSpeed\": " << value.playbackSpeed << ",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(value.anchorKind) << "\",\n"
			<< "      \"position\": [" << value.position.x << ", " << value.position.y << ", " << value.position.z << "],\n"
			<< "      \"motionEnd\": \"" << MotionEnd_ToString(value.motionEnd) << "\",\n"
			<< "      \"nextMotionId\": \"" << CDataJson::Escape(value.nextMotionId) << "\",\n"
			;
		if (value.walkableSurface)
			output << "      \"walkableSurface\": { \"radiusM\": " << value.walkableSurface->radiusM
				<< ", \"localHeightM\": " << value.walkableSurface->localHeightM << " },\n";
		output << "      \"bindings\": [";
		for (size_t bindingIndex = 0; bindingIndex < value.bindings.size();
			++bindingIndex)
		{
			const WORLD_SEQUENCE_BINDING& binding = value.bindings[bindingIndex];
			output << (0u == bindingIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \"" << CDataJson::Escape(binding.slotId)
				<< "\", \"targetKind\": \""
				<< TargetKind_ToString(binding.targetKind)
				<< "\", \"targetId\": \""
				<< CDataJson::Escape(binding.targetId) << "\" }";
		}
		output << (value.bindings.empty() ? "]\n" : "\n      ]\n")
			<< "    }";
	}
	output << (m_Instances.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	bool_t writeSucceeded = output.good();
	output.close();
	writeSucceeded = writeSucceeded && !output.fail();
	if (!writeSucceeded || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit world sequence document atomically";
		return false;
	}
	outStatus = "Saved world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Validate(
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Validate");
	if (m_AreaId.empty() || m_AreaId.size() > 128u || 0u == m_iRevision ||
		m_Templates.size() > MAX_TEMPLATE_COUNT ||
		m_Instances.size() > MAX_INSTANCE_COUNT || m_ObjectResources.size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence document header is invalid";
		return false;
	}
	std::unordered_set<std::string> objectIds;
	for (const WORLD_SEQUENCE_OBJECT_RESOURCE& object : m_ObjectResources)
	{
		if (!object.motionInstanceIds.empty())
		{
			if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
				object.displayName.empty() || object.displayName.size() > 128u || !Is_ValidUtf8DisplayText(object.displayName) ||
				object.motionInstanceIds.size() > 32u || object.anchorKind != "WORLD" ||
				!object.modelAssetId.empty() || !object.sequenceInstanceId.empty() || !object.defaultMotionInstanceId.empty() ||
				object.animated || !object.diffuseTextureAssetId.empty() || object.materialProfile ||
				!object.materialSourceModelAssetId.empty() || !object.mapMaterialBindings.empty() ||
				!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty() ||
				!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE || object.modelPreScale > MAX_COMPONENT ||
				!Is_BoundedFloat3(object.scale) || object.scale.x != 1.f || object.scale.y != 1.f || object.scale.z != 1.f)
			{ outStatus = "Invalid model-less Object group: " + object.objectId; return false; }
			std::unordered_set<std::string> members;
			for (const auto& id : object.motionInstanceIds)
			{
				const auto* instance = Find_Instance(id);
				if (!Is_ValidStableId(id) || !members.insert(id).second || !instance || instance->anchorKind != "WORLD" ||
					instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP || instance->bindings.size() != 1u ||
					instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object group needs unique existing Map Object motions ending with Stop: " + id; return false; }
				const auto* model = Find_ObjectResource(instance->bindings.front().targetId);
				if (!model || model->modelAssetId.empty() || !model->motionInstanceIds.empty())
				{ outStatus = "Object group member must bind a model, not another group: " + id; return false; }
			}
			continue;
		}
		const bool_t alias = !object.sequenceInstanceId.empty();
		if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
			object.displayName.empty() || object.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(object.displayName) ||
			(object.anchorKind != "WORLD" && object.anchorKind != "PLAYER" && object.anchorKind != "BOSS") ||
			(object.anchorKind == "BOSS" ? !Is_ValidStableId(object.anchorBossArchetypeId) :
				(!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty())) ||
			object.anchorBone.size() > 128u || !Is_ValidUtf8DisplayText(object.anchorBone) ||
			(alias && object.anchorKind != "WORLD") ||
			!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE ||
			object.modelPreScale > MAX_COMPONENT || !Is_BoundedFloat3(object.scale) ||
			object.scale.x < MIN_SCALE || object.scale.y < MIN_SCALE || object.scale.z < MIN_SCALE ||
			(alias ? (!object.modelAssetId.empty() || !Is_ValidStableId(object.sequenceInstanceId) ||
				nullptr == Find_Instance(object.sequenceInstanceId) || !object.diffuseTextureAssetId.empty() || object.animated) :
				(!Is_ResourcePath(object.modelAssetId, true) ||
					(!object.diffuseTextureAssetId.empty() && !Is_ResourcePath(object.diffuseTextureAssetId, false)))))
		{
			outStatus = "Invalid or duplicate world object resource: " + object.objectId;
			return false;
		}
        if ((!object.materialSourceModelAssetId.empty() && (alias || !Is_ResourcePath(object.materialSourceModelAssetId, true))) ||
            object.mapMaterialBindings.size() > 64u || (alias && !object.mapMaterialBindings.empty()))
        { outStatus = "Invalid world object material source: " + object.objectId; return false; }
        std::unordered_set<std::string> materialNames;
        if (object.materialProfile) materialNames.insert(object.materialProfile->materialName);
        for (const auto& binding : object.mapMaterialBindings)
            if (binding.materialName.empty() || binding.materialName.size() > 63u || !Is_ValidUtf8DisplayText(binding.materialName) ||
                !Is_ValidStableId(binding.sourceAssetId) || binding.sourceMaterialName.empty() || binding.sourceMaterialName.size() > 63u ||
                !Is_ValidUtf8DisplayText(binding.sourceMaterialName) || !materialNames.insert(binding.materialName).second ||
                (!binding.diffuseTextureAssetId.empty() && !Is_ResourcePath(binding.diffuseTextureAssetId, false)))
            { outStatus = "Invalid or duplicate world object map material binding: " + object.objectId; return false; }
        if (object.materialProfile && (alias || !Validate_MaterialProfile(*object.materialProfile)))
        { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
		if (!object.defaultMotionInstanceId.empty())
		{
			const auto* motion = Find_Instance(object.defaultMotionInstanceId);
			if (!Is_ValidStableId(object.defaultMotionInstanceId) || nullptr == motion || !motion->enabled ||
				(alias ? object.defaultMotionInstanceId != object.sequenceInstanceId :
					(motion->bindings.size() != 1u || motion->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
					 motion->bindings.front().targetId != object.objectId)))
			{
				outStatus = "Default Motion must be an enabled instance of the same Object: " + object.objectId;
				return false;
			}
		}
	}
	std::unordered_set<std::string> templateIds;
	for (const WORLD_SEQUENCE_TEMPLATE& value : m_Templates)
	{
		if (!Is_ValidStableId(value.sequenceId) ||
			!templateIds.insert(value.sequenceId).second ||
			value.displayName.empty() || value.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(value.displayName) ||
			value.category.empty() || value.category.size() > 64u ||
			!Is_ValidUtf8DisplayText(value.category) ||
			0u == value.durationMs || value.durationMs > MAX_DURATION_MS ||
			(WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation &&
				WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP != value.interpolation) ||
			(value.tracks.empty() && value.animationTracks.empty()) ||
			value.tracks.size() + value.animationTracks.size() + value.effectTracks.size() > MAX_TRACK_COUNT)
		{
			outStatus = "Invalid or duplicate world sequence template: " +
				value.sequenceId;
			return false;
		}
		const auto& motion = value.objectMotion;
		if (!Is_BoundedFloat3(motion.velocity) || !Is_BoundedFloat3(motion.acceleration) ||
			!Is_BoundedFloat3(motion.angularVelocityDegrees) ||
			!Is_BoundedFloat3(motion.revolutionDegreesPerSecond) || !Is_BoundedFloat3(motion.revolutionOffset) ||
			!Is_BoundedFloat3(motion.spawnHalfExtents) || motion.spawnHalfExtents.x < 0.f ||
			motion.spawnHalfExtents.y < 0.f || motion.spawnHalfExtents.z < 0.f ||
			motion.count < 1u || motion.count > 128u || motion.intervalMs > MAX_DURATION_MS ||
			(value.effectTracks.empty() && motion.LastEmissionDelayMs() >= value.durationMs) ||
			!Is_ValidEmissionList(motion) ||
			!std::isfinite(motion.spreadDegrees) || motion.spreadDegrees < 0.f || motion.spreadDegrees > (value.effectTracks.empty() ? 180.f : 360.f))
		{ outStatus = "Invalid object motion in template: " + value.sequenceId; return false; }
		std::unordered_set<std::string> effectIds;
		for (const auto& effect : value.effectTracks)
		{
			const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
				[&](const auto& track) { return track.slotId == effect.slotId; }) ||
				std::any_of(value.animationTracks.begin(), value.animationTracks.end(),
					[&](const auto& track) { return track.slotId == effect.slotId; });
			if (!Is_ValidStableId(effect.effectTrackId) || !effectIds.insert(effect.effectTrackId).second ||
				!Is_ValidStableId(effect.slotId) || !slotExists || !Is_ValidStableId(effect.resourceId) ||
				(effect.resourceKind != "LEAF" && effect.resourceKind != "GROUP") ||
				(effect.timing != "TIME" && effect.timing != "MOTION_END") ||
				(effect.timing == "MOTION_END" && effect.startMs != 0u) ||
				effect.startMs > value.durationMs || effect.durationMs == 0u || effect.durationMs > MAX_DURATION_MS ||
				!Is_BoundedFloat3(effect.positionOffset) || !Is_BoundedFloat3(effect.rotationDegrees) ||
				!Is_BoundedFloat3(effect.scale) || effect.scale.x < MIN_SCALE || effect.scale.y < MIN_SCALE || effect.scale.z < MIN_SCALE ||
				value.PresentationSpanMs() > MAX_DURATION_MS)
			{ outStatus = "Invalid World Object effect track: " + value.sequenceId + "/" + effect.effectTrackId; return false; }
		}
		std::unordered_set<std::string> slotIds;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
		{
			if (!Is_ValidStableId(track.slotId) ||
				!slotIds.insert(track.slotId).second || track.keys.size() < 2u ||
				track.keys.size() > MAX_KEY_COUNT || 0u != track.keys.front().timeMs ||
				value.durationMs != track.keys.back().timeMs)
			{
				outStatus = "Invalid track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			bool_t mirrored = false;
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				const bool_t keyMirrored = key.scaleMultiplier.x *
					key.scaleMultiplier.y * key.scaleMultiplier.z < 0.f;
				if (!Is_FiniteTransform(key) || keyMirrored ||
					key.timeMs > value.durationMs ||
					(0u != keyIndex &&
						track.keys[keyIndex - 1u].timeMs >= key.timeMs) ||
					(0u != keyIndex && keyMirrored != mirrored))
				{
					outStatus = "Invalid keyframe in world sequence template: " +
						value.sequenceId + "/" + track.slotId;
					return false;
				}
				mirrored = keyMirrored;
			}
		}
		/* An animation slot may carry an ordered clip chain, so its rows are
		   checked against the slot's previous start instead of a plain unique
		   set. A slot still may not be both a transform and an animation slot. */
		std::unordered_map<std::string, uint32_t> animationSlotStarts;
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
			value.animationTracks)
		{
			const auto chained = animationSlotStarts.find(track.slotId);
			const bool_t firstOfSlot = animationSlotStarts.end() == chained;
			/* A slot may carry both a transform track and a clip chain so one
			   binding can walk an animated prop while it plays. Only a second
			   animation chain on the same slot is a conflict. */
			if (!Is_ValidStableId(track.slotId) || track.clipName.empty() ||
				track.clipName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.clipName) ||
				track.displayName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.displayName) ||
				!std::isfinite(track.playbackRate) || track.playbackRate < 0.05f ||
				track.playbackRate > 8.f ||
				track.startMs >= value.durationMs ||
				(firstOfSlot && 0u != track.startMs) ||
				(!firstOfSlot && track.startMs <= chained->second))
			{
				outStatus = "Invalid animation track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			animationSlotStarts[track.slotId] = track.startMs;
		}
	}

	/* One binding drives one slot, so a slot whose clips are chained still
	   needs exactly one. */
	const auto Count_BoundSlots =
		[](const WORLD_SEQUENCE_TEMPLATE& value) -> size_t
	{
		std::unordered_set<std::string> slots;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
			slots.insert(track.slotId);
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track : value.animationTracks)
			slots.insert(track.slotId);
		return slots.size();
	};

	std::unordered_set<std::string> instanceIds;
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const WORLD_SEQUENCE_TEMPLATE* targetTemplate = Find_Template(value.templateId);
		if (!Is_ValidStableId(value.instanceId) ||
			!instanceIds.insert(value.instanceId).second || nullptr == targetTemplate ||
			value.startDelayMs > MAX_DURATION_MS ||
			!std::isfinite(value.playbackSpeed) || value.playbackSpeed < 0.05f ||
			value.playbackSpeed > 8.f ||
			(value.anchorKind != "WORLD" && value.anchorKind != "PLAYER" && value.anchorKind != "BOSS") ||
			!Is_BoundedFloat3(value.position) ||
			value.bindings.size() != Count_BoundSlots(*targetTemplate))
		{
			outStatus = "Invalid world sequence instance: " + value.instanceId;
			return false;
		}
		if (!targetTemplate->effectTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Effect lanes require one Object Resource binding: " + value.instanceId; return false; }
		if (value.walkableSurface)
		{
			const auto& surface = *value.walkableSurface;
			if (!std::isfinite(surface.radiusM) || surface.radiusM < 0.001f || surface.radiusM > 1000.f ||
				!std::isfinite(surface.localHeightM) || std::abs(surface.localHeightM) > 10000.f ||
				value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT ||
				value.anchorKind != "WORLD" || value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP ||
				targetTemplate->tracks.size() != 1u || !targetTemplate->animationTracks.empty())
			{ outStatus = "Walkable surface requires one static Map placement: " + value.instanceId; return false; }
			const auto& keys = targetTemplate->tracks.front().keys;
			const auto& first = keys.front();
			for (const auto& key : keys)
			{
				if (std::abs(key.rotationQuaternion.x) > 0.00001f || std::abs(key.rotationQuaternion.z) > 0.00001f ||
					key.positionOffset.x != first.positionOffset.x || key.positionOffset.y != first.positionOffset.y ||
					key.positionOffset.z != first.positionOffset.z || key.scaleMultiplier.x != first.scaleMultiplier.x ||
					key.scaleMultiplier.y != first.scaleMultiplier.y || key.scaleMultiplier.z != first.scaleMultiplier.z ||
					key.scaleMultiplier.x <= 0.f || key.scaleMultiplier.y <= 0.f ||
					std::abs(key.scaleMultiplier.x - key.scaleMultiplier.z) > 0.00001f)
				{ outStatus = "Walkable surface needs fixed position/scale and Y rotation only: " + value.instanceId; return false; }
			}
		}
		std::unordered_set<std::string> boundSlots;
		std::unordered_set<std::string> boundTargets;
		for (const WORLD_SEQUENCE_BINDING& binding : value.bindings)
		{
			const auto transformSlot = std::find_if(targetTemplate->tracks.begin(),
				targetTemplate->tracks.end(),
				[&binding](const WORLD_SEQUENCE_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
			const auto animationSlot = std::find_if(
				targetTemplate->animationTracks.begin(),
				targetTemplate->animationTracks.end(),
				[&binding](const WORLD_SEQUENCE_ANIMATION_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
			uint64_t targetId = 0;
			const std::string uniqueTarget =
				std::string(TargetKind_ToString(binding.targetKind)) + ":" +
				binding.targetId;
			const bool_t hasTransformSlot =
				targetTemplate->tracks.end() != transformSlot;
			const bool_t hasAnimationSlot =
				targetTemplate->animationTracks.end() != animationSlot;
			/* A Deploy target may carry a transform track alongside its clip
			   chain so one binding can walk an animated prop while it plays.
			   A map placement has no clips, so an animation slot there is
			   still a mistake. */
			const bool_t bindingShapeIsValid =
				WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT ==
					binding.targetKind ?
				hasAnimationSlot : (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
				(hasTransformSlot || hasAnimationSlot) : (hasTransformSlot && !hasAnimationSlot));
			if (!boundSlots.insert(binding.slotId).second ||
				(binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
					!Is_ValidStableId(binding.targetId) : !Parse_Uint64Text(binding.targetId, targetId)) ||
				!boundTargets.insert(uniqueTarget).second ||
				!bindingShapeIsValid)
			{
				outStatus = "Invalid binding in world sequence instance: " +
					value.instanceId;
				return false;
			}
			/* The binding's own kind decides which target table admits it. A
			   Deploy slot that also carries a transform track is still a
			   Deploy binding. */
			if (WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE == binding.targetKind)
			{
				const auto* object = Find_ObjectResource(binding.targetId);
				if (!object || object->modelAssetId.empty() || !object->sequenceInstanceId.empty() ||
					(hasAnimationSlot && !object->animated) ||
					((value.anchorKind == "BOSS" || object->anchorKind == "BOSS") &&
					 (value.anchorKind != "BOSS" || object->anchorKind != "BOSS" || value.bindings.size() != 1u)))
				{ outStatus = "Invalid object resource binding: " + value.instanceId + "/" + binding.slotId; return false; }
				continue;
			}
			if (value.anchorKind != "WORLD" || value.position.x != 0.f || value.position.y != 0.f || value.position.z != 0.f)
			{ outStatus = "Placed sequences cannot use object instance anchors: " + value.instanceId; return false; }
			if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
			{
				const auto deploy = availableDeployPlacements.find(targetId);
				if (availableDeployPlacements.end() == deploy ||
					!deploy->second.animationTargetSupported)
				{
					outStatus = "Invalid animated Deploy binding in world sequence instance: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
				/* Every clip of the chain must exist on the prop, not just the
				   first, so a mistyped later beat fails here instead of part
				   way through the cutscene. */
				for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
					targetTemplate->animationTracks)
				{
					if (track.slotId != binding.slotId)
						continue;
					if (deploy->second.animationClips.end() == std::find(
						deploy->second.animationClips.begin(),
						deploy->second.animationClips.end(), track.clipName))
					{
						outStatus = "Invalid animated Deploy clip in world sequence instance: " +
							value.instanceId + "/" + track.clipName;
						return false;
					}
				}
				continue;
			}
			const auto placement = availablePlacements.find(targetId);
			if (WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind ||
				availablePlacements.end() == placement ||
				!placement->second.sequenceTargetSupported)
			{
				outStatus = "Invalid map binding in world sequence instance: " +
					value.instanceId + "/" + binding.slotId;
				return false;
			}
			const float3_t& baselineScale =
				placement->second.signedScale;
			if (value.walkableSurface && (baselineScale.x <= 0.f || baselineScale.y <= 0.f ||
				std::abs(baselineScale.x - baselineScale.z) > 0.00001f))
			{ outStatus = "Walkable surface placement scale must be positive and uniform in X/Z: " + value.instanceId; return false; }

			for (const WORLD_SEQUENCE_TRANSFORM_KEY& key : transformSlot->keys)
			{
				const double scaleX = static_cast<double>(baselineScale.x) *
					static_cast<double>(key.scaleMultiplier.x);
				const double scaleY = static_cast<double>(baselineScale.y) *
					static_cast<double>(key.scaleMultiplier.y);
				const double scaleZ = static_cast<double>(baselineScale.z) *
					static_cast<double>(key.scaleMultiplier.z);
				const f32_t composedX = static_cast<f32_t>(scaleX);
				const f32_t composedY = static_cast<f32_t>(scaleY);
				const f32_t composedZ = static_cast<f32_t>(scaleZ);
				const double determinant = static_cast<double>(composedX) *
					static_cast<double>(composedY) *
					static_cast<double>(composedZ);
				const f32_t runtimeDeterminant =
					static_cast<f32_t>(determinant);
				if (!std::isfinite(composedX) || !std::isfinite(composedY) ||
					!std::isfinite(composedZ) || !std::isfinite(determinant) ||
					!std::isfinite(runtimeDeterminant) ||
					std::abs(runtimeDeterminant) < MIN_RUNTIME_SCALE_DETERMINANT)
				{
					outStatus = "Sequence scale would create a singular map transform: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
			}
		}
	}
	/* Resolve completion links only after every instance and binding is valid.
	   A motion changes the existing object, so it cannot switch resource or slot. */
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const bool_t next = value.motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT;
		if (std::string_view(MotionEnd_ToString(value.motionEnd)) == "INVALID" ||
			(next ? !Is_ValidStableId(value.nextMotionId) : !value.nextMotionId.empty()) ||
			(value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP &&
				(value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)))
		{ outStatus = "Invalid world object motion completion: " + value.instanceId; return false; }
		if (!next) continue;
		const auto* target = Find_Instance(value.nextMotionId);
		if (!target || !target->enabled || target->bindings.size() != 1u ||
			target->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
			target->bindings.front().targetId != value.bindings.front().targetId ||
			target->bindings.front().slotId != value.bindings.front().slotId ||
			Find_Template(value.templateId)->objectMotion.count != 1u ||
			Find_Template(target->templateId)->objectMotion.count != 1u)
		{ outStatus = "NEXT motion must target an enabled single object state with the same resource and slot: " + value.instanceId; return false; }
	}
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		std::unordered_set<std::string> visited;
		const WORLD_SEQUENCE_INSTANCE* current = &value;
		uint32_t depth = 0u;
		while (current->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
		{
			if (!visited.insert(current->instanceId).second || ++depth > 32u)
			{ outStatus = "World object NEXT motion chain contains a cycle or exceeds 32 links: " + value.instanceId; return false; }
			current = Find_Instance(current->nextMotionId);
		}
	}
	outStatus = "World sequence document is valid";
	return true;
}

void Client::CWorldSequenceDocument::Reset_Empty(const std::string& areaId)
{
	m_AreaId = areaId;
	m_iRevision = 1;
	m_Templates.clear();
	m_Instances.clear();
	m_ObjectResources.clear();
}

void Client::CWorldSequenceDocument::Touch()
{
	if (m_iRevision < (std::numeric_limits<uint32_t>::max)())
		++m_iRevision;
}

Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(const std::string& sequenceId)
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(
	const std::string& sequenceId) const
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(const std::string& instanceId)
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(
	const std::string& instanceId) const
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId)
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId) const
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

bool_t Client::CWorldSequenceDocument::Is_Equivalent(
	const CWorldSequenceDocument& other) const
{
	const auto sameFloat = [](const f32_t left, const f32_t right)
	{
		return left == right;
	};
	const auto sameFloat3 = [&sameFloat](
		const float3_t& left, const float3_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z);
	};
	const auto sameFloat4 = [&sameFloat](
		const float4_t& left, const float4_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z) && sameFloat(left.w, right.w);
	};
	if (m_AreaId != other.m_AreaId || m_iRevision != other.m_iRevision ||
		m_Templates.size() != other.m_Templates.size() ||
		m_Instances.size() != other.m_Instances.size() ||
		m_ObjectResources.size() != other.m_ObjectResources.size())
	{
		return false;
	}
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& left = m_ObjectResources[index];
		const auto& right = other.m_ObjectResources[index];
		if (left.objectId != right.objectId || left.displayName != right.displayName ||
			left.anchorKind != right.anchorKind || left.anchorBossArchetypeId != right.anchorBossArchetypeId ||
			left.anchorBone != right.anchorBone ||
			left.modelAssetId != right.modelAssetId || left.diffuseTextureAssetId != right.diffuseTextureAssetId ||
            left.materialProfile != right.materialProfile ||
            left.materialSourceModelAssetId != right.materialSourceModelAssetId || left.mapMaterialBindings != right.mapMaterialBindings ||
			left.modelPreScale != right.modelPreScale || left.animated != right.animated ||
			!sameFloat3(left.scale, right.scale) || left.sequenceInstanceId != right.sequenceInstanceId ||
			left.motionInstanceIds != right.motionInstanceIds ||
			left.defaultMotionInstanceId != right.defaultMotionInstanceId) return false;
	}
	for (size_t templateIndex = 0u; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& left = m_Templates[templateIndex];
		const WORLD_SEQUENCE_TEMPLATE& right = other.m_Templates[templateIndex];
		if (left.sequenceId != right.sequenceId ||
			left.displayName != right.displayName ||
			left.category != right.category || left.durationMs != right.durationMs ||
			left.interpolation != right.interpolation ||
			left.tracks.size() != right.tracks.size() ||
			left.animationTracks.size() != right.animationTracks.size() ||
			left.effectTracks.size() != right.effectTracks.size() ||
			!sameFloat3(left.objectMotion.velocity, right.objectMotion.velocity) ||
			!sameFloat3(left.objectMotion.acceleration, right.objectMotion.acceleration) ||
			!sameFloat3(left.objectMotion.angularVelocityDegrees, right.objectMotion.angularVelocityDegrees) ||
			!sameFloat3(left.objectMotion.revolutionDegreesPerSecond, right.objectMotion.revolutionDegreesPerSecond) ||
			!sameFloat3(left.objectMotion.revolutionOffset, right.objectMotion.revolutionOffset) ||
			!sameFloat3(left.objectMotion.spawnHalfExtents, right.objectMotion.spawnHalfExtents) ||
			left.objectMotion.count != right.objectMotion.count || left.objectMotion.intervalMs != right.objectMotion.intervalMs ||
			left.objectMotion.spreadDegrees != right.objectMotion.spreadDegrees || left.objectMotion.seed != right.objectMotion.seed ||
			left.objectMotion.emissions.size() != right.objectMotion.emissions.size())
		{
			return false;
		}
		for (size_t index = 0; index < left.objectMotion.emissions.size(); ++index)
		{
			const auto& a = left.objectMotion.emissions[index]; const auto& b = right.objectMotion.emissions[index];
			if (!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat(a.yawDegrees, b.yawDegrees) ||
				a.startDelayMs != b.startDelayMs) return false;
		}
		for (size_t index = 0; index < left.effectTracks.size(); ++index)
		{
			const auto& a = left.effectTracks[index]; const auto& b = right.effectTracks[index];
			if (a.effectTrackId != b.effectTrackId || a.slotId != b.slotId || a.resourceKind != b.resourceKind ||
				a.resourceId != b.resourceId || a.timing != b.timing || a.startMs != b.startMs || a.durationMs != b.durationMs ||
				!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat3(a.rotationDegrees, b.rotationDegrees) ||
				!sameFloat3(a.scale, b.scale)) return false;
		}
		for (size_t trackIndex = 0u; trackIndex < left.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& leftTrack = left.tracks[trackIndex];
			const WORLD_SEQUENCE_TRACK& rightTrack = right.tracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.keys.size() != rightTrack.keys.size())
			{
				return false;
			}
			for (size_t keyIndex = 0u; keyIndex < leftTrack.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& leftKey = leftTrack.keys[keyIndex];
				const WORLD_SEQUENCE_TRANSFORM_KEY& rightKey = rightTrack.keys[keyIndex];
				if (leftKey.timeMs != rightKey.timeMs ||
					!sameFloat3(leftKey.positionOffset, rightKey.positionOffset) ||
					!sameFloat4(leftKey.rotationQuaternion,
						rightKey.rotationQuaternion) ||
					!sameFloat3(leftKey.scaleMultiplier,
						rightKey.scaleMultiplier) ||
					leftKey.visible != rightKey.visible)
				{
					return false;
				}
			}
		}
		for (size_t trackIndex = 0u;
			trackIndex < left.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& leftTrack =
				left.animationTracks[trackIndex];
			const WORLD_SEQUENCE_ANIMATION_TRACK& rightTrack =
				right.animationTracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.startMs != rightTrack.startMs ||
				leftTrack.clipName != rightTrack.clipName ||
				leftTrack.displayName != rightTrack.displayName ||
				!sameFloat(leftTrack.playbackRate, rightTrack.playbackRate) ||
				leftTrack.loop != rightTrack.loop ||
				leftTrack.holdLastFrame != rightTrack.holdLastFrame)
			{
				return false;
			}
		}
	}
	for (size_t instanceIndex = 0u; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& left = m_Instances[instanceIndex];
		const WORLD_SEQUENCE_INSTANCE& right = other.m_Instances[instanceIndex];
		if (left.instanceId != right.instanceId ||
			left.templateId != right.templateId || left.enabled != right.enabled ||
			left.startDelayMs != right.startDelayMs ||
			!sameFloat(left.playbackSpeed, right.playbackSpeed) ||
			left.bindings.size() != right.bindings.size() || left.anchorKind != right.anchorKind ||
			!sameFloat3(left.position, right.position) ||
			left.motionEnd != right.motionEnd || left.nextMotionId != right.nextMotionId)
		{
			return false;
		}
		if (left.walkableSurface.has_value() != right.walkableSurface.has_value() ||
			(left.walkableSurface && (!sameFloat(left.walkableSurface->radiusM, right.walkableSurface->radiusM) ||
				!sameFloat(left.walkableSurface->localHeightM, right.walkableSurface->localHeightM)))) return false;
		for (size_t bindingIndex = 0u; bindingIndex < left.bindings.size();
			++bindingIndex)
		{
			if (left.bindings[bindingIndex].slotId !=
				right.bindings[bindingIndex].slotId ||
				left.bindings[bindingIndex].targetKind !=
					right.bindings[bindingIndex].targetKind ||
				left.bindings[bindingIndex].targetId !=
					right.bindings[bindingIndex].targetId)
			{
				return false;
			}
		}
	}
	return true;
}

const char_t* Client::CWorldSequenceDocument::Interpolation_ToString(
	const WORLD_SEQUENCE_INTERPOLATION interpolation)
{
	switch (interpolation)
	{
	case WORLD_SEQUENCE_INTERPOLATION::LINEAR:
		return "LINEAR";
	case WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP:
		return "SMOOTH_STEP";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseInterpolation(
	const std::string& value,
	WORLD_SEQUENCE_INTERPOLATION& outInterpolation)
{
	if ("LINEAR" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR;
	else if ("SMOOTH_STEP" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::TargetKind_ToString(
	const WORLD_SEQUENCE_TARGET_KIND targetKind)
{
	switch (targetKind)
	{
	case WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT:
		return "MAP_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT:
		return "DEPLOY_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE:
		return "OBJECT_RESOURCE";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseTargetKind(
	const std::string& value,
	WORLD_SEQUENCE_TARGET_KIND& outTargetKind)
{
	if ("MAP_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	else if ("DEPLOY_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT;
	else if ("OBJECT_RESOURCE" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::MotionEnd_ToString(
	const WORLD_SEQUENCE_MOTION_END motionEnd)
{
	switch (motionEnd)
	{
	case WORLD_SEQUENCE_MOTION_END::STOP: return "STOP";
	case WORLD_SEQUENCE_MOTION_END::HOLD: return "HOLD";
	case WORLD_SEQUENCE_MOTION_END::LOOP: return "LOOP";
	case WORLD_SEQUENCE_MOTION_END::NEXT: return "NEXT";
	default: return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseMotionEnd(
	const std::string& value, WORLD_SEQUENCE_MOTION_END& outMotionEnd)
{
	if (value == "STOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::STOP;
	else if (value == "HOLD") outMotionEnd = WORLD_SEQUENCE_MOTION_END::HOLD;
	else if (value == "LOOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::LOOP;
	else if (value == "NEXT") outMotionEnd = WORLD_SEQUENCE_MOTION_END::NEXT;
	else return false;
	return true;
}

bool_t Client::CWorldSequenceDocument::Is_ValidStableId(
	const std::string& value)
{
	return !value.empty() && value.size() <= 128u &&
		std::all_of(value.begin(), value.end(), [](const unsigned char character)
		{
			return 0 != std::isalnum(character) || character == '_' ||
				character == '-' || character == '.';
		});
}

bool_t Client::CWorldSequenceDocument::Build_MaterialOverride(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
    const std::filesystem::path& resourceRoot, Engine::MODEL_MATERIAL_OVERRIDE& out)
{
    Engine::MODEL_MATERIAL_OVERRIDE staged;
    if (!resourceRoot.is_absolute() || !Validate_MaterialProfile(profile, &staged.surface.sourceCharacter)) return false;
    staged.materialName = profile.materialName;
    staged.surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
    for (const auto& texture : profile.textures)
    {
        auto& input = staged.sourceCharacterTextures[texture.expressionIndex];
        input.path = (resourceRoot / texture.assetId).lexically_normal();
        input.srgb = texture.srgb;
    }
    out = std::move(staged);
    return true;
}
