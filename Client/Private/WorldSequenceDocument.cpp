#include "WorldSequenceDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-sequences";
	constexpr uint32_t FORMAT_VERSION = 2;
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
	if (!CDataJson::Parse(text, root, parseError) ||
		!Is_ExactObject(root,
			{ "schema", "formatVersion", "areaId", "revision",
			  "templates", "instances" }))
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
		(LEGACY_FORMAT_VERSION != parsedFormatVersion &&
			FORMAT_VERSION != parsedFormatVersion) ||
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
	for (const DATA_JSON_VALUE& templateValue : templates->Get_Array())
	{
		const bool_t validTemplateShape =
			LEGACY_FORMAT_VERSION == parsedFormatVersion ?
			Is_ExactObject(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks" }) :
			Is_ExactObject(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks", "animationTracks" });
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
			(FORMAT_VERSION == parsedFormatVersion &&
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
		if (FORMAT_VERSION == parsedFormatVersion)
		{
			for (const DATA_JSON_VALUE& trackValue :
				animationTracks->Get_Array())
			{
				if (!Is_ExactObject(trackValue,
						{ "slotId", "clipName", "playbackRate", "loop",
						  "holdLastFrame" }) &&
					!Is_ExactObject(trackValue,
						{ "slotId", "clipName", "playbackRate", "loop",
						  "holdLastFrame", "startMs" }))
				{
					outStatus = "World sequence animation track shape is invalid";
					return false;
				}
				const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
				const DATA_JSON_VALUE* clipName = trackValue.Find("clipName");
				const DATA_JSON_VALUE* loop = trackValue.Find("loop");
				const DATA_JSON_VALUE* holdLastFrame =
					trackValue.Find("holdLastFrame");
				WORLD_SEQUENCE_ANIMATION_TRACK parsedTrack;
				if (nullptr == slotId || !slotId->Is_String() ||
					nullptr == clipName || !clipName->Is_String() ||
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
				parsedTrack.loop = loop->Get_Boolean();
				parsedTrack.holdLastFrame = holdLastFrame->Get_Boolean();
				parsedTemplate.animationTracks.push_back(std::move(parsedTrack));
			}
		}
		staged.m_Templates.push_back(std::move(parsedTemplate));
	}

	for (const DATA_JSON_VALUE& instanceValue : instances->Get_Array())
	{
		if (!Is_ExactObject(instanceValue,
			{ "instanceId", "templateId", "enabled", "startDelayMs",
			  "playbackSpeed", "bindings" }))
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
				<< "\", \"startMs\": " << track.startMs
					<< ", \"playbackRate\": " << track.playbackRate
				<< ", \"loop\": " << (track.loop ? "true" : "false")
				<< ", \"holdLastFrame\": "
				<< (track.holdLastFrame ? "true" : "false") << " }";
		}
		output << (value.animationTracks.empty() ? "]\n" : "\n      ]\n")
			<< "    }";
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
			<< "      \"bindings\": [";
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
	if (m_AreaId.empty() || m_AreaId.size() > 128u || 0u == m_iRevision ||
		m_Templates.size() > MAX_TEMPLATE_COUNT ||
		m_Instances.size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence document header is invalid";
		return false;
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
			value.tracks.size() + value.animationTracks.size() > MAX_TRACK_COUNT)
		{
			outStatus = "Invalid or duplicate world sequence template: " +
				value.sequenceId;
			return false;
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
			value.bindings.size() != Count_BoundSlots(*targetTemplate))
		{
			outStatus = "Invalid world sequence instance: " + value.instanceId;
			return false;
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
				hasAnimationSlot : (hasTransformSlot && !hasAnimationSlot);
			if (!boundSlots.insert(binding.slotId).second ||
				!Parse_Uint64Text(binding.targetId, targetId) ||
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
	outStatus = "World sequence document is valid";
	return true;
}

void Client::CWorldSequenceDocument::Reset_Empty(const std::string& areaId)
{
	m_AreaId = areaId;
	m_iRevision = 1;
	m_Templates.clear();
	m_Instances.clear();
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
		m_Instances.size() != other.m_Instances.size())
	{
		return false;
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
			left.animationTracks.size() != right.animationTracks.size())
		{
			return false;
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
			left.bindings.size() != right.bindings.size())
		{
			return false;
		}
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
	else
		return false;
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
