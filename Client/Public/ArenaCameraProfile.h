#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include "Network/PacketType.h"

#include <array>
#include <filesystem>
#include <string>

NS_BEGIN(Client)

enum class ARENA_CAMERA_MAP
{
	CHARACTER_SELECT,
	KOUKU_SAYDON,
	BERN,
	VALTAN
};

// Ship-riding lens (Bern only). Defaults are EFTable_CameraSetting 1001 step 1, the
// Camera_Ocean row of every EFTable_VoyageShip base ship, converted with the (x, z, -y)
// basis of the 2026-09-14 camera restoration: FOV 60 (horizontal at 16:9), pitch -45 -> 45,
// yaw 0 -> 90, ZoomDist 1700 cm -> 17 m, RelativeZ -50 cm -> focus y -0.5 m, ratio 2.
struct ARENA_SHIP_CAMERA final
{
	std::string provenance = "EFTable_CameraSetting 1001/1 (VoyageShip.Camera_Ocean)";
	f32_t fovXDegrees = 60.f;
	f32_t pitchDegrees = 45.f;
	f32_t yawDegrees = 90.f;
	// Source ZoomDist is 17 m; this project's ship models are larger relative to the
	// camera, so the framing distance is a project value. See Bern.camera.json.
	f32_t distanceMeters = 40.f;
	f32_t focusOffsetYMeters = -0.5f;
	f32_t followResponse = 2.f;
};

// Optional "shipFog" block (Bern only). The ride looks through far more fogged air than
// the 16 m map camera does: Bern's fog takes the source-exponential branch, which
// accumulates over camera-to-pixel distance past startDistance (16 m), so at the 40 m ship
// lens the authored fog that was invisible before washes the frame. Riding therefore
// switches that fog off; a document can thin it instead by setting disable false.
struct ARENA_SHIP_FOG final
{
	bool_t disable = true;
	// Read only when disable is false; multiplies the scene density.
	f32_t densityScale = 1.f;
	// Negative keeps the scene value.
	f32_t startDistanceMeters = -1.f;
	f32_t maximumOpacity = -1.f;
};

struct ARENA_CAMERA_PROFILE final
{
	float3_t positionOffset{};
	// Pitch/Yaw/Roll in degrees; positive pitch looks down, yaw zero faces +Z.
	float3_t rotationDegrees{};
	f32_t focusDistance = 1.f;
	f32_t fovYDegrees = 60.f;
	f32_t followResponse = 0.f;
	// Explicit source-volume mode; absent legacy JSON keeps its saved manual pose.
	bool_t useSourceCameraRegions = false;
	// Visual multiplier relative to this class's admitted catalog scale.
	f32_t characterSizeMultiplier = 1.f;
	// Enum-indexed in memory, stable class names on disk; reserved DESTROYER stays 1.
	// These are relative to the currently admitted catalog models.
	std::array<f32_t, 8u> classSizeMultipliers{ 1.f, 1.f, 1.f, 0.7f, 1.f, 1.f, 1.f, 1.f };
	f32_t clownSizeMultiplier = 0.7f;
	f32_t marioSizeMultiplier = 1.f;
	// Player pickup hammer offsets in its hand frame; separate from the boss prop.
	float3_t mazeHammerPositionCm{};
	float3_t mazeHammerRotationDegrees{};
	float3_t mazeHammerScale{ 1.f, 1.f, 1.f };
	// Optional "shipCamera" block; Save writes it back only when the document carried one.
	ARENA_SHIP_CAMERA shipCamera{};
	bool_t hasShipCamera = false;
	// Optional "shipFog" block; an omitted block means the fog is off while riding.
	ARENA_SHIP_FOG shipFog{};
	bool_t hasShipFog = false;
};

class CArenaCameraProfile final
{
public:
	static ARENA_CAMERA_PROFILE Default(ARENA_CAMERA_MAP map);
	// Measured settings immediately before the 2026-09-14 source restoration.
	static ARENA_CAMERA_PROFILE BeforeRestoration(ARENA_CAMERA_MAP map);
	static bool_t Validate(const ARENA_CAMERA_PROFILE& profile, std::string& status);
	// Failed reads preserve the caller's profile; Save only replaces this map's file.
	static bool_t Load(ARENA_CAMERA_MAP map, ARENA_CAMERA_PROFILE& outProfile,
		std::string& status, std::string* sourceBaseline = nullptr);
	// An optional baseline is compared before Save and refreshed only on success.
	static bool_t Save(ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile,
		std::string& status, std::string* sourceBaseline = nullptr);
	// Original MidnightC PS export43 convex brush; margin is an exit hysteresis in metres.
	static bool_t Contains_KoukuSourceEntrance(const float3_t& position, f32_t marginMeters = 0.f);
	// The common 16m CDO is a baseline, not proof of every original battle camera.
	static ARENA_CAMERA_PROFILE KoukuSourceProfile(const ARENA_CAMERA_PROFILE& saved, bool_t entrance);
	static float3_t LookOffset(const ARENA_CAMERA_PROFILE& profile);
	// Move the eye around the current focus; lens and focus position stay fixed.
	static bool_t Set_OrbitAroundFocus(ARENA_CAMERA_PROFILE& profile,
		f32_t distance, f32_t pitchDegrees, f32_t yawDegrees, std::string& status);
	static std::filesystem::path Path(ARENA_CAMERA_MAP map);
};

NS_END
