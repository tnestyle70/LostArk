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

struct ARENA_CAMERA_PROFILE final
{
	float3_t positionOffset{};
	// Pitch/Yaw/Roll in degrees; positive pitch looks down, yaw zero faces +Z.
	float3_t rotationDegrees{};
	f32_t focusDistance = 1.f;
	f32_t fovYDegrees = 60.f;
	f32_t followResponse = 0.f;
	// Visual multiplier relative to this class's admitted catalog scale.
	f32_t characterSizeMultiplier = 1.f;
	// Enum-indexed in memory, stable class names on disk; reserved DESTROYER stays 1.
	// These are relative to the currently admitted catalog models.
	std::array<f32_t, 7u> classSizeMultipliers{ 1.f, 1.f, 1.f, 1.6f, 1.f, 0.7f, 1.f };
	f32_t clownSizeMultiplier = 0.7f;
	f32_t marioSizeMultiplier = 1.f;
	// Player pickup hammer offsets in its hand frame; separate from the boss prop.
	float3_t mazeHammerPositionCm{};
	float3_t mazeHammerRotationDegrees{};
	float3_t mazeHammerScale{ 1.f, 1.f, 1.f };
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
	static float3_t LookOffset(const ARENA_CAMERA_PROFILE& profile);
	// Move the eye around the current focus; lens and focus position stay fixed.
	static bool_t Set_OrbitAroundFocus(ARENA_CAMERA_PROFILE& profile,
		f32_t distance, f32_t pitchDegrees, f32_t yawDegrees, std::string& status);
	static std::filesystem::path Path(ARENA_CAMERA_MAP map);
};

NS_END
