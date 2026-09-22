#include "ArenaCameraProfile.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace
{
	constexpr const char* SCHEMA = "lostark.arena-camera";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::array<const char*, 8u> CLASS_SIZE_KEYS{
		"LANCE_MASTER", "GUNSLINGER", "SLAYER", "ARTIST", nullptr, "DIMENSIONMASTER", "WARLORD", "GUARDIANKNIGHT" };

	const char* AreaId(const ARENA_CAMERA_MAP map)
	{
		switch (map)
		{
		case ARENA_CAMERA_MAP::CHARACTER_SELECT: return "LV_LOBBY_CLASSSELECT_SL00";
		case ARENA_CAMERA_MAP::KOUKU_SAYDON: return "LV_LUT_MIDNIGHTC_ED";
		case ARENA_CAMERA_MAP::BERN: return "LV_BER_BERNCASTLE";
		case ARENA_CAMERA_MAP::VALTAN: return "LV_LUT_HEARTRB_ED";
		default: return nullptr;
		}
	}

	bool_t InRange(const f32_t value, const f32_t minimum, const f32_t maximum)
	{
		return std::isfinite(value) && value >= minimum && value <= maximum;
	}

	bool_t ReadFloat(const DATA_JSON_VALUE* value, f32_t& out)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			std::abs(value->Get_Number()) > (std::numeric_limits<f32_t>::max)())
			return false;
		out = static_cast<f32_t>(value->Get_Number());
		return true;
	}

	bool_t ReadVector(const DATA_JSON_VALUE* value, float3_t& out)
	{
		if (nullptr == value || !value->Is_Array() || value->Get_Array().size() != 3u)
			return false;
		const auto& values = value->Get_Array();
		return ReadFloat(&values[0], out.x) && ReadFloat(&values[1], out.y) &&
			ReadFloat(&values[2], out.z);
	}

	bool_t Parse(const std::string& text, const ARENA_CAMERA_MAP map,
		ARENA_CAMERA_PROFILE& out, std::string& status)
	{
		DATA_JSON_VALUE root;
		DATA_JSON_PARSE_LIMITS limits;
		limits.iMaximumBytes = 8192u;
		limits.iMaximumDepth = 4u;
		limits.iMaximumValues = 64u;
		if (!CDataJson::Parse(text, root, status, limits))
			return false;
		const char* area = AreaId(map);
		const size_t expectedFields = 8u + (root.Find("characterSizeMultiplier") ? 1u : 0u) +
			(root.Find("classSizeMultipliers") ? 1u : 0u) + (root.Find("clownSizeMultiplier") ? 1u : 0u) +
			(root.Find("marioSizeMultiplier") ? 1u : 0u) + (root.Find("useSourceCameraRegions") ? 1u : 0u) +
			(root.Find("mazeHammerPositionCm") ? 1u : 0u) +
			(root.Find("mazeHammerRotationDegrees") ? 1u : 0u) + (root.Find("mazeHammerScale") ? 1u : 0u);
		if (nullptr == area || !root.Is_Object() || root.Get_Object().size() != expectedFields)
		{
			status = "Arena camera document requires eight supported fields and optional source-region/character size fields.";
			return false;
		}
		const auto* schema = root.Find("schema");
		const auto* version = root.Find("formatVersion");
		const auto* areaId = root.Find("areaId");
		if (nullptr == schema || !schema->Is_String() || schema->Get_String() != SCHEMA ||
			nullptr == version || !version->Is_Number() || version->Was_FloatingPointToken() ||
			version->Get_Number() != FORMAT_VERSION || nullptr == areaId ||
			!areaId->Is_String() || areaId->Get_String() != area)
		{
			status = "Arena camera schema, integer formatVersion, or selected areaId is invalid.";
			return false;
		}
		ARENA_CAMERA_PROFILE staged;
		if (const auto* regions = root.Find("useSourceCameraRegions"))
		{
			if (!regions->Is_Boolean() || (regions->Get_Boolean() && map != ARENA_CAMERA_MAP::KOUKU_SAYDON))
			{ status = "Source camera regions require a boolean and are supported only for KoukuSaydon."; return false; }
			staged.useSourceCameraRegions = regions->Get_Boolean();
		}
		if (!ReadVector(root.Find("positionOffset"), staged.positionOffset) ||
			!ReadVector(root.Find("rotationDegrees"), staged.rotationDegrees) ||
			!ReadFloat(root.Find("focusDistance"), staged.focusDistance) ||
			!ReadFloat(root.Find("fovYDegrees"), staged.fovYDegrees) ||
			!ReadFloat(root.Find("followResponse"), staged.followResponse) ||
			(root.Find("characterSizeMultiplier") &&
				!ReadFloat(root.Find("characterSizeMultiplier"), staged.characterSizeMultiplier)) ||
			(root.Find("clownSizeMultiplier") && !ReadFloat(root.Find("clownSizeMultiplier"), staged.clownSizeMultiplier)) ||
			(root.Find("marioSizeMultiplier") && !ReadFloat(root.Find("marioSizeMultiplier"), staged.marioSizeMultiplier)) ||
			(root.Find("mazeHammerPositionCm") && !ReadVector(root.Find("mazeHammerPositionCm"), staged.mazeHammerPositionCm)) ||
			(root.Find("mazeHammerRotationDegrees") && !ReadVector(root.Find("mazeHammerRotationDegrees"), staged.mazeHammerRotationDegrees)) ||
			(root.Find("mazeHammerScale") && !ReadVector(root.Find("mazeHammerScale"), staged.mazeHammerScale)))
		{
			status = "Arena camera pose and lens fields must contain finite numbers.";
			return false;
		}
		if (const auto* sizes = root.Find("classSizeMultipliers"))
		{
			// Before Guardian Knight was added, valid profiles saved six named classes.
			// Only that version may omit its new key; every original class stays required.
			const bool legacySixClasses = sizes->Is_Object() && sizes->Get_Object().size() == 6u &&
				sizes->Find("GUARDIANKNIGHT") == nullptr;
			if (!sizes->Is_Object() || (!legacySixClasses && sizes->Get_Object().size() != 7u))
			{ status = "Class size tuning requires the seven playable class names or the six-class legacy profile."; return false; }
			for (size_t i = 0u; i < CLASS_SIZE_KEYS.size(); ++i)
			{
				if (!CLASS_SIZE_KEYS[i] || (legacySixClasses && i == 7u)) continue;
				if (!ReadFloat(sizes->Find(CLASS_SIZE_KEYS[i]), staged.classSizeMultipliers[i]))
				{ status = "Class size tuning has an unknown/missing class or nonfinite multiplier."; return false; }
			}
		}
		if (!CArenaCameraProfile::Validate(staged, status))
			return false;
		out = staged;
		return true;
	}

	bool_t ReadText(const std::filesystem::path& path, std::string& out, std::string& status)
	{
		std::ifstream input(path, std::ios::binary | std::ios::ate);
		if (!input.is_open())
		{
			status = "Cannot read arena camera profile: " + path.string();
			return false;
		}
		const auto size = input.tellg();
		if (size < 0 || size > 8192)
		{
			status = "Arena camera profile exceeds the 8192-byte document limit: " + path.string();
			return false;
		}
		input.seekg(0);
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
		{
			status = "Arena camera profile read failed: " + path.string();
			return false;
		}
		out = buffer.str();
		return true;
	}

	std::string Serialize(const ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile)
	{
		std::ostringstream output;
		output.imbue(std::locale::classic());
		output << std::setprecision(std::numeric_limits<f32_t>::max_digits10)
			<< "{\n  \"schema\": \"" << SCHEMA << "\",\n  \"formatVersion\": " << FORMAT_VERSION
			<< ",\n  \"areaId\": \"" << AreaId(map) << "\",\n  \"positionOffset\": ["
			<< profile.positionOffset.x << ", " << profile.positionOffset.y << ", "
			<< profile.positionOffset.z << "],\n  \"rotationDegrees\": ["
			<< profile.rotationDegrees.x << ", " << profile.rotationDegrees.y << ", "
			<< profile.rotationDegrees.z << "],\n  \"focusDistance\": " << profile.focusDistance
			<< ",\n  \"fovYDegrees\": " << profile.fovYDegrees
			<< ",\n  \"followResponse\": " << profile.followResponse
			<< ",\n  \"useSourceCameraRegions\": " << (profile.useSourceCameraRegions ? "true" : "false")
			<< ",\n  \"characterSizeMultiplier\": " << profile.characterSizeMultiplier
			<< ",\n  \"classSizeMultipliers\": {";
		bool first = true;
		for (size_t i = 0u; i < CLASS_SIZE_KEYS.size(); ++i)
		{
			if (!CLASS_SIZE_KEYS[i]) continue;
			output << (first ? "" : ",") << "\n    \"" << CLASS_SIZE_KEYS[i] << "\": " << profile.classSizeMultipliers[i];
			first = false;
		}
		output << "\n  },\n  \"clownSizeMultiplier\": " << profile.clownSizeMultiplier
			<< ",\n  \"marioSizeMultiplier\": " << profile.marioSizeMultiplier
			<< ",\n  \"mazeHammerPositionCm\": [" << profile.mazeHammerPositionCm.x << ", " << profile.mazeHammerPositionCm.y << ", " << profile.mazeHammerPositionCm.z << "]"
			<< ",\n  \"mazeHammerRotationDegrees\": [" << profile.mazeHammerRotationDegrees.x << ", " << profile.mazeHammerRotationDegrees.y << ", " << profile.mazeHammerRotationDegrees.z << "]"
			<< ",\n  \"mazeHammerScale\": [" << profile.mazeHammerScale.x << ", " << profile.mazeHammerScale.y << ", " << profile.mazeHammerScale.z << "]\n}\n";
		return output.str();
	}
}

std::filesystem::path CArenaCameraProfile::Path(const ARENA_CAMERA_MAP map)
{
	switch (map)
	{
	case ARENA_CAMERA_MAP::CHARACTER_SELECT:
		return CProjectDataRoot::Resolve(L"Camera/CharacterSelect.camera.json");
	case ARENA_CAMERA_MAP::KOUKU_SAYDON:
		return CProjectDataRoot::Resolve(L"Camera/KoukuSaydon.camera.json");
	case ARENA_CAMERA_MAP::BERN:
		return CProjectDataRoot::Resolve(L"Camera/Bern.camera.json");
	case ARENA_CAMERA_MAP::VALTAN:
		return CProjectDataRoot::Resolve(L"Camera/Valtan.camera.json");
	default: return {};
	}
}

ARENA_CAMERA_PROFILE CArenaCameraProfile::Default(const ARENA_CAMERA_MAP map)
{
	if (nullptr == AreaId(map))
		throw std::invalid_argument("Unknown arena camera map.");
	// EFTable_IsometricCamera and the raid EFChangePlayerCameraVolume defaults.
	// UE centimetres (X,Y,Z) map to runtime metres (X,Z,-Y). The lens is
	// horizontal at 16:9 in the source; DirectXMath consumes vertical degrees.
	const f32_t distance = map == ARENA_CAMERA_MAP::VALTAN ? 18.f : 16.f;
	const f32_t horizontalFov = map == ARENA_CAMERA_MAP::VALTAN ? 55.f : 50.f;
	ARENA_CAMERA_PROFILE profile;
	profile.useSourceCameraRegions = map == ARENA_CAMERA_MAP::KOUKU_SAYDON;
	profile.positionOffset = { -distance * 0.5f, distance * std::sqrt(0.5f) - 0.1f,
		distance * 0.5f };
	profile.rotationDegrees = { 45.f, 135.f, 0.f };
	profile.focusDistance = distance;
	profile.fovYDegrees = XMConvertToDegrees(2.f * std::atan(
		std::tan(XMConvertToRadians(horizontalFov) * 0.5f) / (16.f / 9.f)));
	// Project follow damping is independent of the source lens/boom restoration.
	profile.followResponse = map == ARENA_CAMERA_MAP::BERN ? 0.f :
		map == ARENA_CAMERA_MAP::VALTAN ? 18.f : 12.f;
	return profile;
}

bool_t CArenaCameraProfile::Contains_KoukuSourceEntrance(const float3_t& position, const f32_t marginMeters)
{
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
		!std::isfinite(marginMeters) || marginMeters < 0.f)
		return false;
	// PS export43 Location + BrushComponent export5 VertexData, UE cm -> (X,Z,-Y)m.
	// This rotated entrance brush is around Z -87..2, not the Saydon arena at Z738.
	constexpr std::array<float3_t, 4u> corners{{
		{-3.818115234375f, 0.f, 1.409208984375f},
		{38.275830078125f, 0.f, -14.84708984375f},
		{10.631866760254f, 0.f, -86.428344726563f},
		{-31.462043457031f, 0.f, -70.172307128906f}}};
	constexpr f32_t tolerance = 0.00005f;
	if (position.y < -7.741909713745f - marginMeters - tolerance ||
		position.y > 16.491906204224f + marginMeters + tolerance)
		return false;
	for (size_t i = 0; i < corners.size(); ++i)
	{
		const auto& a = corners[i];
		const auto& b = corners[(i + 1u) % corners.size()];
		const f32_t dx = b.x - a.x, dz = b.z - a.z;
		const f32_t cross = dx * (position.z - a.z) - dz * (position.x - a.x);
		if (cross > (marginMeters + tolerance) * std::hypot(dx, dz))
			return false;
	}
	return true;
}

ARENA_CAMERA_PROFILE CArenaCameraProfile::KoukuSourceProfile(
	const ARENA_CAMERA_PROFILE& saved, const bool_t entrance)
{
	auto resolved = saved;
	const auto source = Default(ARENA_CAMERA_MAP::KOUKU_SAYDON);
	const f32_t distance = entrance ? 19.f : 16.f;
	resolved.positionOffset = {-distance * .5f, distance * std::sqrt(.5f) - .1f, distance * .5f};
	resolved.rotationDegrees = source.rotationDegrees;
	resolved.focusDistance = distance;
	resolved.fovYDegrees = source.fovYDegrees;
	return resolved;
}

ARENA_CAMERA_PROFILE CArenaCameraProfile::BeforeRestoration(const ARENA_CAMERA_MAP map)
{
	ARENA_CAMERA_PROFILE profile;
	switch (map)
	{
	case ARENA_CAMERA_MAP::CHARACTER_SELECT:
		profile.positionOffset = { 0.f, 6.f, 4.f };
		profile.rotationDegrees = { 55.f, -180.f, 0.f };
		profile.focusDistance = 7.f;
		profile.fovYDegrees = 70.f;
		profile.followResponse = 12.f;
		break;
	case ARENA_CAMERA_MAP::KOUKU_SAYDON:
		profile.positionOffset = { -3.1500001f, 7.75f, 3.1500001f };
		profile.rotationDegrees = { 53.1100006f, 132.f, -1.75f };
		profile.focusDistance = 7.75241899f;
		profile.fovYDegrees = 60.f;
		profile.followResponse = 12.f;
		break;
	case ARENA_CAMERA_MAP::BERN:
	case ARENA_CAMERA_MAP::VALTAN:
		profile.positionOffset = { 0.4f, 7.5f, 4.5f };
		profile.rotationDegrees = {
			XMConvertToDegrees(std::atan2(6.3f, std::hypot(0.4f, 4.5f))),
			XMConvertToDegrees(std::atan2(-0.4f, -4.5f)), 0.f };
		profile.focusDistance = std::sqrt(0.4f * 0.4f + 6.3f * 6.3f + 4.5f * 4.5f);
		profile.fovYDegrees = 60.f;
		profile.followResponse = map == ARENA_CAMERA_MAP::BERN ? 0.f : 18.f;
		break;
	default: throw std::invalid_argument("Unknown arena camera map.");
	}
	return profile;
}

bool_t CArenaCameraProfile::Validate(const ARENA_CAMERA_PROFILE& profile, std::string& status)
{
	if (!InRange(profile.positionOffset.x, -1000.f, 1000.f) ||
		!InRange(profile.positionOffset.y, -1000.f, 1000.f) ||
		!InRange(profile.positionOffset.z, -1000.f, 1000.f))
		status = "Position offset components must be finite and within -1000..1000 metres.";
	else if (!InRange(profile.rotationDegrees.x, -89.f, 89.f) ||
		!InRange(profile.rotationDegrees.y, -180.f, 180.f) ||
		!InRange(profile.rotationDegrees.z, -180.f, 180.f))
		status = "Rotation requires pitch -89..89 and yaw/roll -180..180 degrees.";
	else if (!InRange(profile.focusDistance, 0.1f, 1000.f))
		status = "Focus distance must be finite and within 0.1..1000 metres.";
	else if (!InRange(profile.fovYDegrees, 10.f, 150.f))
		status = "Vertical FOV must be finite and within 10..150 degrees.";
	else if (!InRange(profile.followResponse, 0.f, 60.f))
		status = "Follow response must be finite and within 0..60 (0 is immediate).";
	else if (!InRange(profile.characterSizeMultiplier, 0.25f, 4.f))
		status = "Character size multiplier must be finite and within 0.25..4.";
	else if (!std::all_of(profile.classSizeMultipliers.begin(), profile.classSizeMultipliers.end(),
		[](const f32_t value) { return InRange(value, 0.25f, 4.f); }) || profile.classSizeMultipliers[4] != 1.f)
		status = "Class size multipliers must be finite and within 0.25..4; reserved classes stay at 1.";
	else if (!InRange(profile.clownSizeMultiplier, 0.25f, 4.f) || !InRange(profile.marioSizeMultiplier, 0.25f, 4.f))
		status = "Clown and Mario size multipliers must be finite and within 0.25..4.";
	else if (!InRange(profile.mazeHammerPositionCm.x, -1000.f, 1000.f) ||
		!InRange(profile.mazeHammerPositionCm.y, -1000.f, 1000.f) || !InRange(profile.mazeHammerPositionCm.z, -1000.f, 1000.f) ||
		!InRange(profile.mazeHammerRotationDegrees.x, -3600.f, 3600.f) || !InRange(profile.mazeHammerRotationDegrees.y, -3600.f, 3600.f) ||
		!InRange(profile.mazeHammerRotationDegrees.z, -3600.f, 3600.f) || !InRange(profile.mazeHammerScale.x, .05f, 8.f) ||
		!InRange(profile.mazeHammerScale.y, .05f, 8.f) || !InRange(profile.mazeHammerScale.z, .05f, 8.f))
		status = "Maze hammer requires finite position +/-1000 cm, rotation +/-3600 deg and scale 0.05..8.";
	else
	{
		status.clear();
		return true;
	}
	return false;
}

float3_t CArenaCameraProfile::LookOffset(const ARENA_CAMERA_PROFILE& profile)
{
	const auto rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(profile.rotationDegrees.x),
		XMConvertToRadians(profile.rotationDegrees.y),
		XMConvertToRadians(profile.rotationDegrees.z));
	const auto forward = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotation);
	float3_t look;
	XMStoreFloat3(&look, XMVectorMultiplyAdd(forward,
		XMVectorReplicate(profile.focusDistance), XMLoadFloat3(&profile.positionOffset)));
	return look;
}

bool_t CArenaCameraProfile::Set_OrbitAroundFocus(ARENA_CAMERA_PROFILE& profile,
	f32_t distance, f32_t pitchDegrees, f32_t yawDegrees, std::string& status)
{
	if (!Validate(profile, status)) return false;
	const float3_t focus = LookOffset(profile);
	ARENA_CAMERA_PROFILE staged = profile;
	staged.focusDistance = distance;
	staged.rotationDegrees.x = pitchDegrees;
	staged.rotationDegrees.y = yawDegrees;
	if (!Validate(staged, status)) return false;
	staged.positionOffset = {};
	const float3_t direction = LookOffset(staged);
	staged.positionOffset = {focus.x - direction.x, focus.y - direction.y, focus.z - direction.z};
	if (!Validate(staged, status)) return false;
	profile = staged;
	return true;
}

bool_t CArenaCameraProfile::Load(const ARENA_CAMERA_MAP map, ARENA_CAMERA_PROFILE& outProfile,
	std::string& status, std::string* sourceBaseline)
{
	const auto path = Path(map);
	if (path.empty())
	{
		status = "Unknown arena camera map or unresolved project Data root.";
		return false;
	}
	std::string text;
	ARENA_CAMERA_PROFILE staged;
	if (!ReadText(path, text, status) || !Parse(text, map, staged, status))
		return false;
	outProfile = staged;
	if (nullptr != sourceBaseline) *sourceBaseline = text;
	status = "Loaded arena camera profile: " + path.string();
	return true;
}

bool_t CArenaCameraProfile::Save(const ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile,
	std::string& status, std::string* sourceBaseline)
{
	const auto path = Path(map);
	if (path.empty())
	{
		status = "Unknown arena camera map or unresolved project Data root.";
		return false;
	}
	if (!Validate(profile, status))
		return false;
	if (profile.useSourceCameraRegions && map != ARENA_CAMERA_MAP::KOUKU_SAYDON)
	{ status = "Source camera regions are supported only for KoukuSaydon."; return false; }
	std::error_code error;
	const bool_t existed = std::filesystem::exists(path, error);
	std::string previous;
	if (error || (existed && !ReadText(path, previous, status)))
	{
		if (error) status = "Cannot inspect arena camera destination: " + error.message();
		return false;
	}
	if (nullptr != sourceBaseline && (previous != *sourceBaseline ||
		(existed && sourceBaseline->empty())))
	{
		status = "Camera file changed since Load. Draft and disk preserved; Reload saved before editing again.";
		return false;
	}
	std::filesystem::create_directories(path.parent_path(), error);
	if (error)
	{
		status = "Cannot create arena camera directory: " + error.message();
		return false;
	}
	static std::atomic_uint64_t serial{ 0u };
	auto temporary = path;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64()) + L"." + std::to_wstring(++serial);
	const HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == file)
	{
		status = "Cannot create arena camera temporary file (Win32 " +
			std::to_string(GetLastError()) + ").";
		return false;
	}
	const std::string text = Serialize(map, profile);
	DWORD written = 0u;
	const bool_t writtenOk = FALSE != WriteFile(file, text.data(),
		static_cast<DWORD>(text.size()), &written, nullptr) && written == text.size() &&
		FALSE != FlushFileBuffers(file);
	CloseHandle(file);
	auto reject = [&](const std::string& reason)
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		status = reason;
		return false;
	};
	if (!writtenOk)
		return reject("Arena camera temporary write failed; previous file preserved.");
	std::string stagedText;
	ARENA_CAMERA_PROFILE staged;
	if (!ReadText(temporary, stagedText, status) || !Parse(stagedText, map, staged, status))
		return reject("Arena camera saved-file validation failed: " + status);
	std::string current;
	const bool_t stillExists = std::filesystem::exists(path, error);
	if (error || stillExists != existed ||
		(stillExists && (!ReadText(path, current, status) || current != previous)))
		return reject("Arena camera file changed during Save; current file and draft preserved.");
	if (FALSE == MoveFileExW(temporary.c_str(), path.c_str(),
		(existed ? MOVEFILE_REPLACE_EXISTING : 0u) | MOVEFILE_WRITE_THROUGH))
		return reject("Arena camera atomic replace failed (Win32 " +
			std::to_string(GetLastError()) + "); previous file preserved.");
	if (nullptr != sourceBaseline) *sourceBaseline = text;
	status = "Saved arena camera profile: " + path.string();
	return true;
}
