#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

enum class ARENA_CAMERA_MAP
{
	CHARACTER_SELECT,
	KOUKU_SAYDON
};

struct ARENA_CAMERA_PROFILE final
{
	float3_t positionOffset{};
	// Pitch/Yaw/Roll in degrees; positive pitch looks down, yaw zero faces +Z.
	float3_t rotationDegrees{};
	f32_t focusDistance = 1.f;
	f32_t fovYDegrees = 60.f;
	f32_t followResponse = 0.f;
};

class CArenaCameraProfile final
{
public:
	static ARENA_CAMERA_PROFILE Default(ARENA_CAMERA_MAP map);
	static bool_t Validate(const ARENA_CAMERA_PROFILE& profile, std::string& status);
	// Failed reads preserve the caller's profile; Save only replaces this map's file.
	static bool_t Load(ARENA_CAMERA_MAP map, ARENA_CAMERA_PROFILE& outProfile,
		std::string& status);
	static bool_t Save(ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile,
		std::string& status);
	static float3_t LookOffset(const ARENA_CAMERA_PROFILE& profile);
	static std::filesystem::path Path(ARENA_CAMERA_MAP map);
};

NS_END
