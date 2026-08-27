#include "MapEffectDocument.h"

#include "DataJson.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <system_error>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.map-effect-presentation";
	constexpr uint32_t FORMAT_VERSION = 1u;

	constexpr uint64_t MAXIMUM_MAP_EFFECT_BYTES = 4u * 1024u * 1024u;

	bool_t ReadTextFile(
		const std::filesystem::path& path,
		std::string& outText,
		std::string* outStatus = nullptr)
	{
		outText.clear();
		const HANDLE file = CreateFileW(path.c_str(), GENERIC_READ,
			FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			if (nullptr != outStatus)
				*outStatus = "Map Effect source could not be opened";
			return false;
		}
		LARGE_INTEGER size{};
		if (FALSE == GetFileSizeEx(file, &size) || size.QuadPart <= 0ll ||
			static_cast<uint64_t>(size.QuadPart) > MAXIMUM_MAP_EFFECT_BYTES)
		{
			CloseHandle(file);
			if (nullptr != outStatus)
				*outStatus = "Map Effect source is empty or exceeds 4 MiB";
			return false;
		}
		outText.resize(static_cast<size_t>(size.QuadPart));
		size_t offset = 0u;
		while (offset < outText.size())
		{
			const DWORD request = static_cast<DWORD>((std::min)(
				outText.size() - offset,
				static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD read = 0u;
			if (FALSE == ReadFile(file, outText.data() + offset,
					request, &read, nullptr) || 0u == read)
			{
				CloseHandle(file);
				outText.clear();
				if (nullptr != outStatus)
					*outStatus = "Map Effect source read was incomplete";
				return false;
			}
			offset += read;
		}
		CloseHandle(file);
		return true;
	}

	std::filesystem::path MakeSaveTransactionPath(
		const std::filesystem::path& destination,
		const wchar_t* role)
	{
		static std::atomic_uint64_t counter = 0u;
		return destination.wstring() + L"." + role + L"." +
			std::to_wstring(std::chrono::steady_clock::now().
				time_since_epoch().count()) + L"." +
			std::to_wstring(counter.fetch_add(1u, std::memory_order_relaxed));
	}

	bool_t WriteRawAtomicIfUnchanged(
		const std::filesystem::path& path,
		const std::string_view replacement,
		const std::string_view expected,
		std::string& outStatus)
	{
		outStatus.clear();
		std::string observed;
		if (!ReadTextFile(path, observed, &outStatus) || observed != expected)
		{
			outStatus = "Map Effect source changed on disk after it was loaded; Reload before saving";
			return false;
		}
		const std::filesystem::path temporary =
			MakeSaveTransactionPath(path, L"tmp");
		const HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0u,
			nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			outStatus = "Map Effect temporary source could not be created";
			return false;
		}
		size_t offset = 0u;
		bool_t wrote = true;
		while (offset < replacement.size())
		{
			const DWORD request = static_cast<DWORD>((std::min)(
				replacement.size() - offset,
				static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD written = 0u;
			if (FALSE == WriteFile(file, replacement.data() + offset,
					request, &written, nullptr) || 0u == written)
			{
				wrote = false;
				break;
			}
			offset += written;
		}
		const bool_t flushed = wrote && FALSE != FlushFileBuffers(file);
		CloseHandle(file);
		if (!flushed)
		{
			DeleteFileW(temporary.c_str());
			outStatus = "Map Effect temporary source write/flush failed";
			return false;
		}

		/* Second exact-byte CAS immediately before the write-through replace. */
		observed.clear();
		if (!ReadTextFile(path, observed, &outStatus) || observed != expected)
		{
			DeleteFileW(temporary.c_str());
			outStatus = "Map Effect source changed during save; temporary data was discarded";
			return false;
		}
		if (FALSE == MoveFileExW(temporary.c_str(), path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			DeleteFileW(temporary.c_str());
			outStatus = "Map Effect write-through source replace failed";
			return false;
		}
		outStatus = "Map Effect source replaced by exact-byte two-pass CAS";
		return true;
	}

	bool_t IsExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t IsStableId(const std::string& value, const size_t maximumLength)
	{
		if (value.empty() || value.size() > maximumLength)
			return false;
		return std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return std::isalnum(character) || character == '.' ||
					character == '_' || character == '-';
			});
	}

	bool_t ReadString(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String())
			return false;
		outValue = value->Get_String();
		return true;
	}

	bool_t ReadFinite(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const double minimum,
		const double maximum,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < minimum || number > maximum)
			return false;
		outValue = static_cast<f32_t>(number);
		return std::isfinite(outValue);
	}

	bool_t ReadUInt32(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 || number > maximum ||
			std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t ParsePlacementId(const std::string& text, uint64_t& outValue)
	{
		if (text.empty() || text.size() > 20u ||
			(text.size() > 1u && text.front() == '0'))
		{
			return false;
		}
		uint64_t parsed = 0u;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), parsed, 10);
		if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
			0u == parsed)
		{
			return false;
		}
		outValue = parsed;
		return true;
	}

	template <size_t Count>
	bool_t ReadVector(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const double minimum,
		const double maximum,
		std::array<f32_t, Count>& outValues)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			value->Get_Array().size() != Count)
		{
			return false;
		}
		std::array<f32_t, Count> staged{};
		for (size_t index = 0u; index < Count; ++index)
		{
			const DATA_JSON_VALUE& component = value->Get_Array()[index];
			if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
				component.Get_Number() < minimum ||
				component.Get_Number() > maximum)
			{
				return false;
			}
			staged[index] = static_cast<f32_t>(component.Get_Number());
			if (!std::isfinite(staged[index]))
				return false;
		}
		outValues = staged;
		return true;
	}

	std::string EscapeJson(const std::string& value)
	{
		std::string result;
		result.reserve(value.size() + 8u);
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '\\': result += "\\\\"; break;
			case '"': result += "\\\""; break;
			case '\b': result += "\\b"; break;
			case '\f': result += "\\f"; break;
			case '\n': result += "\\n"; break;
			case '\r': result += "\\r"; break;
			case '\t': result += "\\t"; break;
			default:
				if (character < 0x20u)
				{
					std::ostringstream escaped;
					escaped << "\\u" << std::hex << std::setw(4) <<
						std::setfill('0') << static_cast<uint32_t>(character);
					result += escaped.str();
				}
				else
				{
					result.push_back(static_cast<char_t>(character));
				}
				break;
			}
		}
		return result;
	}

	const char_t* ToString(const MAP_EFFECT_ORIENTATION_POLICY value)
	{
		switch (value)
		{
		case MAP_EFFECT_ORIENTATION_POLICY::WORLD: return "WORLD";
		case MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD:
			return "CAMERA_FACING_WORLD";
		default: return "";
		}
	}

	const char_t* ToString(const MAP_EFFECT_ACTIVATION_POLICY value)
	{
		switch (value)
		{
		case MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE: return "LEVEL_ACTIVE";
		case MAP_EFFECT_ACTIVATION_POLICY::SERVER_PATTERN_WINDOW:
			return "SERVER_PATTERN_WINDOW";
		default: return "";
		}
	}

	const char_t* ToString(const MAP_EFFECT_PLAYBACK_POLICY value)
	{
		switch (value)
		{
		case MAP_EFFECT_PLAYBACK_POLICY::LOCAL_LOOP: return "LOCAL_LOOP";
		case MAP_EFFECT_PLAYBACK_POLICY::SERVER_CLOCK_SAMPLE:
			return "SERVER_CLOCK_SAMPLE";
		default: return "";
		}
	}
}

bool_t Client::CMapEffectDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	std::string& outStatus)
{
	std::string rawBaseline;
	return Load_WithRawBaseline(
		path, expectedAreaId, rawBaseline, outStatus);
}

bool_t Client::CMapEffectDocument::Load_WithRawBaseline(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	std::string& outRawBaseline,
	std::string& outStatus)
{
	outRawBaseline.clear();
	if (path.empty() || !ReadTextFile(path, outRawBaseline, &outStatus))
	{
		outStatus = "Map Effect document is unreadable: " + path.string();
		return false;
	}
	if (!Parse(outRawBaseline, expectedAreaId, outStatus))
	{
		outRawBaseline.clear();
		return false;
	}
	return true;
}

bool_t Client::CMapEffectDocument::Parse(
	const std::string& text,
	const std::string& expectedAreaId,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	if (text.empty() || expectedAreaId.empty() ||
		!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "Map Effect document parse failed: " + parseError;
		return false;
	}
	if (!IsExactObject(root,
		{ "schema", "formatVersion", "areaId", "presentations" }))
	{
		outStatus = "Map Effect document root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string areaId;
	uint32_t version = 0u;
	const DATA_JSON_VALUE* presentations = root.Find("presentations");
	if (!ReadString(root, "schema", schema) || schema != SCHEMA ||
		!ReadUInt32(root, "formatVersion", FORMAT_VERSION, version) ||
		version != FORMAT_VERSION ||
		!ReadString(root, "areaId", areaId) || areaId != expectedAreaId ||
		!IsStableId(areaId, 128u) || nullptr == presentations ||
		!presentations->Is_Array() || presentations->Get_Array().empty() ||
		presentations->Get_Array().size() > MAX_PRESENTATION_COUNT)
	{
		outStatus = "Map Effect document header is invalid";
		return false;
	}

	std::vector<MAP_EFFECT_SURFACE_PRESENTATION> stagedSurfaces;
	std::vector<MAP_EFFECT_WORLD_PRESENTATION> stagedWorldEffects;
	std::unordered_set<std::string> independentIds;
	std::unordered_set<std::string> placementIds;
	std::unordered_set<uint64_t> surfacePlacementIds;
	for (const DATA_JSON_VALUE& row : presentations->Get_Array())
	{
		std::string kind;
		std::string independentId;
		std::string displayName;
		if (!ReadString(row, "presentationKind", kind) ||
			!ReadString(row, "independentEffectId", independentId) ||
			!IsStableId(independentId, 160u) ||
			!ReadString(row, "displayName", displayName) || displayName.empty() ||
			displayName.size() > 160u ||
			!independentIds.insert(independentId).second)
		{
			outStatus = "Map Effect presentation identity is invalid or duplicated";
			return false;
		}

		if (kind == "DEPLOY_SURFACE_OVERLAY")
		{
			if (!IsExactObject(row, {
				"independentEffectId", "displayName", "presentationKind",
				"owners", "visibleStates", "materialIndex",
				"emissiveIntensity", "emissiveColor", "maskPower" }))
			{
				outStatus = "Map Effect surface row has unexpected properties";
				return false;
			}
			const DATA_JSON_VALUE* owners = row.Find("owners");
			const DATA_JSON_VALUE* visibleStates = row.Find("visibleStates");
			MAP_EFFECT_SURFACE_PRESENTATION surface;
			surface.independentEffectId = independentId;
			surface.displayName = displayName;
			std::array<f32_t, 4u> color{};
			if (nullptr == owners || !owners->Is_Array() ||
				owners->Get_Array().empty() ||
				owners->Get_Array().size() > MAX_SURFACE_OWNER_COUNT ||
				nullptr == visibleStates || !visibleStates->Is_Array() ||
				visibleStates->Get_Array().empty() ||
				!ReadUInt32(row, "materialIndex", 255u, surface.materialIndex) ||
				!ReadFinite(row, "emissiveIntensity", 0.0, 64.0,
					surface.emissiveIntensity) ||
				!ReadVector(row, "emissiveColor", 0.0, 16.0, color) ||
				!ReadFinite(row, "maskPower", 0.01, 32.0, surface.maskPower))
			{
				outStatus = "Map Effect surface fields are invalid";
				return false;
			}
			std::unordered_set<std::string> groupIds;
			for (const DATA_JSON_VALUE& owner : owners->Get_Array())
			{
				std::string groupId;
				std::string placementIdText;
				uint64_t placementId = 0u;
				if (!IsExactObject(owner, { "groupId", "placementId" }) ||
					!ReadString(owner, "groupId", groupId) ||
					!IsStableId(groupId, 160u) ||
					!ReadString(owner, "placementId", placementIdText) ||
					!ParsePlacementId(placementIdText, placementId) ||
					!groupIds.insert(groupId).second ||
					!surfacePlacementIds.insert(placementId).second)
				{
					outStatus = "Map Effect surface owner is invalid or duplicated";
					return false;
				}
				surface.owners.push_back({ std::move(groupId), placementId });
			}
			/* Product rendering currently exposes one exact gate: the overlay is
			   part of the intact Deploy surface and disappears when the Server
			   advances it to BREAKING.  Reject broader authoring here so a document
			   accepted by the parser cannot later be refused by the runtime. */
			if (1u != visibleStates->Get_Array().size() ||
				!visibleStates->Get_Array().front().Is_String() ||
				"INTACT" != visibleStates->Get_Array().front().Get_String())
			{
				outStatus = "Map Effect surface visibility must be exactly [INTACT]";
				return false;
			}
			surface.visibleStates.push_back("INTACT");
			surface.emissiveColor =
				{ color[0], color[1], color[2], color[3] };
			stagedSurfaces.push_back(std::move(surface));
			continue;
		}

		if (kind == "EFFECT_DOCUMENT")
		{
			if (!IsExactObject(row, {
				"independentEffectId", "displayName", "presentationKind",
				"placementId", "effectAssetId", "position",
				"rotationQuaternion", "scale", "orientationPolicy",
				"activationPolicy", "activationSetId", "activationWindows",
				"playbackPolicy" }))
			{
				outStatus = "Map Effect world row has unexpected properties";
				return false;
			}
			MAP_EFFECT_WORLD_PRESENTATION world;
			world.independentEffectId = independentId;
			world.displayName = displayName;
			std::string orientation;
			std::string activation;
			std::string playback;
			std::array<f32_t, 3u> position{};
			std::array<f32_t, 4u> rotation{};
			std::array<f32_t, 3u> scale{};
			const DATA_JSON_VALUE* activationWindows =
				row.Find("activationWindows");
			if (!ReadString(row, "placementId", world.placementId) ||
				!IsStableId(world.placementId, 160u) ||
				!placementIds.insert(world.placementId).second ||
				!ReadString(row, "effectAssetId", world.effectAssetId) ||
				!IsStableId(world.effectAssetId, 160u) ||
				!ReadVector(row, "position", -100000.0, 100000.0, position) ||
				!ReadVector(row, "rotationQuaternion", -1.0, 1.0, rotation) ||
				!ReadVector(row, "scale", 0.001, 1000.0, scale) ||
				!ReadString(row, "orientationPolicy", orientation) ||
				!ReadString(row, "activationPolicy", activation) ||
				!ReadString(row, "activationSetId", world.activationSetId) ||
				nullptr == activationWindows || !activationWindows->Is_Array() ||
				!ReadString(row, "playbackPolicy", playback))
			{
				outStatus = "Map Effect world fields are invalid";
				return false;
			}
			const double lengthSquared =
				static_cast<double>(rotation[0]) * rotation[0] +
				static_cast<double>(rotation[1]) * rotation[1] +
				static_cast<double>(rotation[2]) * rotation[2] +
				static_cast<double>(rotation[3]) * rotation[3];
			if (lengthSquared < 0.999 || lengthSquared > 1.001)
			{
				outStatus = "Map Effect world quaternion is not normalized";
				return false;
			}
			if (orientation == "WORLD")
				world.orientationPolicy = MAP_EFFECT_ORIENTATION_POLICY::WORLD;
			else if (orientation == "CAMERA_FACING_WORLD")
				world.orientationPolicy =
					MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD;
			else
			{
				outStatus = "Map Effect orientation policy is invalid";
				return false;
			}
			if (activation == "LEVEL_ACTIVE")
				world.activationPolicy = MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE;
			else if (activation == "SERVER_PATTERN_WINDOW")
				world.activationPolicy =
					MAP_EFFECT_ACTIVATION_POLICY::SERVER_PATTERN_WINDOW;
			else
			{
				outStatus = "Map Effect activation policy is invalid";
				return false;
			}
			if ((world.activationPolicy == MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE &&
				 (!world.activationSetId.empty() ||
				  !activationWindows->Get_Array().empty())) ||
				(world.activationPolicy ==
				 MAP_EFFECT_ACTIVATION_POLICY::SERVER_PATTERN_WINDOW &&
				 (!IsStableId(world.activationSetId, 160u) ||
				  activationWindows->Get_Array().empty() ||
				  activationWindows->Get_Array().size() > 32u)))
			{
				outStatus = "Map Effect activation set is invalid";
				return false;
			}
			std::unordered_set<std::string> activationTuples;
			uint32_t previousOffset = 0u;
			for (size_t windowIndex = 0u;
				windowIndex < activationWindows->Get_Array().size(); ++windowIndex)
			{
				const DATA_JSON_VALUE& value =
					activationWindows->Get_Array()[windowIndex];
				MAP_EFFECT_ACTIVATION_WINDOW window;
				if (!IsExactObject(value,
					{ "patternId", "stageId", "effectTimelineOffsetMs" }) ||
					!ReadString(value, "patternId", window.patternId) ||
					!IsStableId(window.patternId, 128u) ||
					!ReadString(value, "stageId", window.stageId) ||
					!IsStableId(window.stageId, 128u) ||
					!ReadUInt32(value, "effectTimelineOffsetMs", 3600000u,
						window.effectTimelineOffsetMs) ||
					(windowIndex > 0u &&
					 window.effectTimelineOffsetMs <= previousOffset) ||
					!activationTuples.insert(
						window.patternId + "\n" + window.stageId).second)
				{
					outStatus = "Map Effect activation window is invalid or duplicated";
					return false;
				}
				previousOffset = window.effectTimelineOffsetMs;
				world.activationWindows.push_back(std::move(window));
			}
			if (playback == "LOCAL_LOOP")
				world.playbackPolicy = MAP_EFFECT_PLAYBACK_POLICY::LOCAL_LOOP;
			else if (playback == "SERVER_CLOCK_SAMPLE")
				world.playbackPolicy =
					MAP_EFFECT_PLAYBACK_POLICY::SERVER_CLOCK_SAMPLE;
			else
			{
				outStatus = "Map Effect playback policy is invalid";
				return false;
			}
			if ((world.activationPolicy ==
				 MAP_EFFECT_ACTIVATION_POLICY::SERVER_PATTERN_WINDOW) !=
				(world.playbackPolicy ==
				 MAP_EFFECT_PLAYBACK_POLICY::SERVER_CLOCK_SAMPLE))
			{
				outStatus = "Map Effect activation/playback policies do not match";
				return false;
			}
			world.position = { position[0], position[1], position[2] };
			world.rotationQuaternion =
				{ rotation[0], rotation[1], rotation[2], rotation[3] };
			world.scale = { scale[0], scale[1], scale[2] };
			stagedWorldEffects.push_back(std::move(world));
			continue;
		}

		outStatus = "Map Effect presentation kind is unsupported: " + kind;
		return false;
	}

	m_AreaId = std::move(areaId);
	m_Surfaces = std::move(stagedSurfaces);
	m_WorldEffects = std::move(stagedWorldEffects);
	m_isReady = true;
	outStatus = "Map Effect presentation ready: " +
		std::to_string(m_Surfaces.size()) + " surfaces, " +
		std::to_string(m_WorldEffects.size()) + " world Effects";
	return true;
}

std::string Client::CMapEffectDocument::Serialize() const
{
	if (!m_isReady)
		return {};
	std::ostringstream output;
	output << std::setprecision(9);
	output << "{\n  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << EscapeJson(m_AreaId) << "\",\n"
		<< "  \"presentations\": [\n";
	size_t emitted = 0u;
	const size_t total = m_Surfaces.size() + m_WorldEffects.size();
	for (const MAP_EFFECT_SURFACE_PRESENTATION& surface : m_Surfaces)
	{
		output << "    {\n"
			<< "      \"independentEffectId\": \""
			<< EscapeJson(surface.independentEffectId) << "\",\n"
			<< "      \"displayName\": \"" << EscapeJson(surface.displayName)
			<< "\",\n"
			<< "      \"presentationKind\": \"DEPLOY_SURFACE_OVERLAY\",\n"
			<< "      \"owners\": [\n";
		for (size_t index = 0u; index < surface.owners.size(); ++index)
		{
			const MAP_EFFECT_SURFACE_OWNER& owner = surface.owners[index];
			output << "        { \"groupId\": \"" << EscapeJson(owner.groupId)
				<< "\", \"placementId\": \"" << owner.placementId << "\" }"
				<< (index + 1u == surface.owners.size() ? "\n" : ",\n");
		}
		output << "      ],\n      \"visibleStates\": [";
		for (size_t index = 0u; index < surface.visibleStates.size(); ++index)
		{
			output << "\"" << EscapeJson(surface.visibleStates[index]) << "\""
				<< (index + 1u == surface.visibleStates.size() ? "" : ", ");
		}
		output << "],\n"
			<< "      \"materialIndex\": " << surface.materialIndex << ",\n"
			<< "      \"emissiveIntensity\": " << surface.emissiveIntensity << ",\n"
			<< "      \"emissiveColor\": [" << surface.emissiveColor.x << ", "
			<< surface.emissiveColor.y << ", " << surface.emissiveColor.z << ", "
			<< surface.emissiveColor.w << "],\n"
			<< "      \"maskPower\": " << surface.maskPower << "\n    }"
			<< (++emitted == total ? "\n" : ",\n");
	}
	for (const MAP_EFFECT_WORLD_PRESENTATION& world : m_WorldEffects)
	{
		output << "    {\n"
			<< "      \"independentEffectId\": \""
			<< EscapeJson(world.independentEffectId) << "\",\n"
			<< "      \"displayName\": \"" << EscapeJson(world.displayName)
			<< "\",\n"
			<< "      \"presentationKind\": \"EFFECT_DOCUMENT\",\n"
			<< "      \"placementId\": \"" << EscapeJson(world.placementId)
			<< "\",\n"
			<< "      \"effectAssetId\": \"" << EscapeJson(world.effectAssetId)
			<< "\",\n"
			<< "      \"position\": [" << world.position.x << ", "
			<< world.position.y << ", " << world.position.z << "],\n"
			<< "      \"rotationQuaternion\": ["
			<< world.rotationQuaternion.x << ", " << world.rotationQuaternion.y
			<< ", " << world.rotationQuaternion.z << ", "
			<< world.rotationQuaternion.w << "],\n"
			<< "      \"scale\": [" << world.scale.x << ", " << world.scale.y
			<< ", " << world.scale.z << "],\n"
			<< "      \"orientationPolicy\": \""
			<< ToString(world.orientationPolicy) << "\",\n"
			<< "      \"activationPolicy\": \""
			<< ToString(world.activationPolicy) << "\",\n"
			<< "      \"activationSetId\": \""
			<< EscapeJson(world.activationSetId) << "\",\n"
			<< "      \"activationWindows\": [\n";
		for (size_t windowIndex = 0u;
			windowIndex < world.activationWindows.size(); ++windowIndex)
		{
			const MAP_EFFECT_ACTIVATION_WINDOW& window =
				world.activationWindows[windowIndex];
			output << "        { \"patternId\": \""
				<< EscapeJson(window.patternId) << "\", \"stageId\": \""
				<< EscapeJson(window.stageId)
				<< "\", \"effectTimelineOffsetMs\": "
				<< window.effectTimelineOffsetMs << " }"
				<< (windowIndex + 1u == world.activationWindows.size() ?
					"\n" : ",\n");
		}
		output << "      ],\n"
			<< "      \"playbackPolicy\": \""
			<< ToString(world.playbackPolicy) << "\"\n    }"
			<< (++emitted == total ? "\n" : ",\n");
	}
	output << "  ]\n}\n";
	return output.str();
}

bool_t Client::CMapEffectDocument::Save_AtomicIfUnchanged(
	const std::filesystem::path& path,
	const std::string_view expectedRawBytes,
	std::string& outStatus) const
{
	outStatus.clear();
	if (!m_isReady || path.empty() || m_AreaId.empty())
	{
		outStatus = "Map Effect authoring save has no ready document or path";
		return false;
	}
	const std::string canonical = Serialize();
	CMapEffectDocument roundTrip;
	if (canonical.empty() ||
		!roundTrip.Parse(canonical, m_AreaId, outStatus) ||
		roundTrip.Serialize() != canonical)
	{
		outStatus = "Map Effect authoring draft failed canonical round-trip: " +
			outStatus;
		return false;
	}

	if (expectedRawBytes.empty())
	{
		outStatus = "Map Effect existing source save requires an exact raw-byte baseline";
		return false;
	}
	return WriteRawAtomicIfUnchanged(
		path, canonical, expectedRawBytes, outStatus);
}

bool_t Client::CMapEffectDocument::Restore_RawBytesAtomicIfUnchanged(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	const std::string_view replacementRawBytes,
	const std::string_view expectedCurrentRawBytes,
	std::string& outStatus)
{
	CMapEffectDocument replacement;
	if (replacementRawBytes.empty() || expectedCurrentRawBytes.empty() ||
		!replacement.Parse(std::string(replacementRawBytes),
			expectedAreaId, outStatus))
	{
		outStatus = "Map Effect raw rollback payload is invalid: " + outStatus;
		return false;
	}
	return WriteRawAtomicIfUnchanged(path, replacementRawBytes,
		expectedCurrentRawBytes, outStatus);
}

void Client::CMapEffectDocument::Clear()
{
	m_AreaId.clear();
	m_Surfaces.clear();
	m_WorldEffects.clear();
	m_isReady = false;
}

const Client::MAP_EFFECT_SURFACE_PRESENTATION*
Client::CMapEffectDocument::Find_Surface(
	const std::string& independentEffectId) const
{
	const auto found = std::find_if(m_Surfaces.begin(), m_Surfaces.end(),
		[&independentEffectId](const auto& row)
		{
			return row.independentEffectId == independentEffectId;
		});
	return found == m_Surfaces.end() ? nullptr : &*found;
}

Client::MAP_EFFECT_SURFACE_PRESENTATION*
Client::CMapEffectDocument::Edit_Surface(
	const std::string& independentEffectId)
{
	const auto found = std::find_if(m_Surfaces.begin(), m_Surfaces.end(),
		[&independentEffectId](const auto& row)
		{
			return row.independentEffectId == independentEffectId;
		});
	return found == m_Surfaces.end() ? nullptr : &*found;
}

const Client::MAP_EFFECT_WORLD_PRESENTATION*
Client::CMapEffectDocument::Find_WorldEffect(
	const std::string& independentEffectId) const
{
	const auto found = std::find_if(m_WorldEffects.begin(), m_WorldEffects.end(),
		[&independentEffectId](const auto& row)
		{
			return row.independentEffectId == independentEffectId;
		});
	return found == m_WorldEffects.end() ? nullptr : &*found;
}

Client::MAP_EFFECT_WORLD_PRESENTATION*
Client::CMapEffectDocument::Edit_WorldEffect(
	const std::string& independentEffectId)
{
	const auto found = std::find_if(m_WorldEffects.begin(),
		m_WorldEffects.end(), [&independentEffectId](const auto& row)
		{
			return row.independentEffectId == independentEffectId;
		});
	return found == m_WorldEffects.end() ? nullptr : &*found;
}

bool_t Client::CMapEffectDocument::Add_WorldEffectForAuthoring(
	const MAP_EFFECT_WORLD_PRESENTATION& presentation,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "Map Effect authoring document is not ready";
		return false;
	}
	CMapEffectDocument staged = *this;
	staged.m_WorldEffects.push_back(presentation);
	CMapEffectDocument validated;
	if (!validated.Parse(staged.Serialize(), m_AreaId, outStatus))
		return false;
	*this = std::move(validated);
	return true;
}

bool_t Client::CMapEffectDocument::Semantically_Equals(
	const CMapEffectDocument& other) const
{
	return m_isReady && other.m_isReady && m_AreaId == other.m_AreaId &&
		m_Surfaces == other.m_Surfaces &&
		m_WorldEffects == other.m_WorldEffects;
}
