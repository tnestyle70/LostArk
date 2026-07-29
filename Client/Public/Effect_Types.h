#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

constexpr uint32_t EFFECT_ASSET_SCHEMA_VERSION = 3;
constexpr uint64_t INVALID_EFFECT_ELEMENT_ID = 0;
constexpr uint32_t EFFECT_LOOP_FOREVER = 0;
constexpr uint32_t EFFECT_MAX_EMITTERS = 128;
constexpr uint32_t EFFECT_MAX_MODULES_PER_EMITTER = 64;
constexpr uint32_t EFFECT_MAX_PARTICLES_PER_EMITTER = 65536;
constexpr uint32_t EFFECT_MAX_TRAIL_POINTS = 4096;
constexpr uint32_t EFFECT_MAX_BEAM_SEGMENTS = 256;
constexpr uint32_t EFFECT_MAX_SUBUV_DIMENSION = 256;

enum class EFFECT_PROVENANCE : uint8_t
{
	UNKNOWN,
	GAME_ORIGINAL,
	DERIVED_CONVERSION,
	RECONSTRUCTED,
	AUTHORED_VARIANT,
	END
};

enum class EFFECT_EMITTER_TYPE : uint8_t
{
	SPRITE,
	MESH,
	BEAM,
	RIBBON,
	ANIM_TRAIL,
	END
};

enum class EFFECT_MODULE_TYPE : uint8_t
{
	REQUIRED,
	SPAWN,
	LIFETIME,
	INITIAL_LOCATION,
	INITIAL_VELOCITY,
	INITIAL_SIZE,
	INITIAL_COLOR,
	SIZE_OVER_LIFE,
	ALPHA_OVER_LIFE,
	VELOCITY_OVER_LIFE,
	COLLISION,
	EVENT,
	LOD,
	INITIAL_ROTATION,
	ROTATION_RATE,
	COLOR_OVER_LIFE,
	SUB_UV,
	MESH_ANIMATION,
	BEAM,
	TRAIL,
	END
};

enum class EFFECT_DISTRIBUTION_TYPE : uint8_t
{
	CONSTANT,
	UNIFORM_RANGE,
	END
};

enum class EFFECT_SIMULATION_SPACE : uint8_t
{
	LOCAL,
	WORLD,
	END
};

enum class EFFECT_BLEND_MODE : uint8_t
{
	ALPHA,
	ADDITIVE,
	END
};

enum class EFFECT_SORT_MODE : uint8_t
{
	NONE,
	DISTANCE_TO_VIEW,
	AGE_OLDEST_FIRST,
	AGE_NEWEST_FIRST,
	END
};

struct EFFECT_DISTRIBUTION_FLOAT_DESC
{
	EFFECT_DISTRIBUTION_TYPE eType = { EFFECT_DISTRIBUTION_TYPE::CONSTANT };
	f32_t fConstant = {};
	f32_t fMin = {};
	f32_t fMax = {};
};

struct EFFECT_DISTRIBUTION_VECTOR_DESC
{
	EFFECT_DISTRIBUTION_TYPE eType = { EFFECT_DISTRIBUTION_TYPE::CONSTANT };
	float3_t vConstant = {};
	float3_t vMin = {};
	float3_t vMax = {};
};

struct EFFECT_DISTRIBUTION_COLOR_DESC
{
	EFFECT_DISTRIBUTION_TYPE eType = { EFFECT_DISTRIBUTION_TYPE::CONSTANT };
	float4_t vConstant = { 1.f, 1.f, 1.f, 1.f };
	float4_t vMin = { 1.f, 1.f, 1.f, 1.f };
	float4_t vMax = { 1.f, 1.f, 1.f, 1.f };
};

struct EFFECT_REQUIRED_MODULE_DESC
{
	string strTextureAssetId;
	string strMeshAssetId;
	string strMaterialAssetId;
};

struct EFFECT_SPAWN_MODULE_DESC
{
	f32_t fRatePerSecond = {};
	uint32_t iBurstCount = {};
	uint32_t iMaxParticles = { 1000 };
};

struct EFFECT_LIFETIME_MODULE_DESC
{
	EFFECT_DISTRIBUTION_FLOAT_DESC Lifetime = {
		EFFECT_DISTRIBUTION_TYPE::CONSTANT, 1.f, 1.f, 1.f
	};
};

struct EFFECT_INITIAL_LOCATION_MODULE_DESC
{
	EFFECT_DISTRIBUTION_VECTOR_DESC Location;
};

struct EFFECT_INITIAL_VELOCITY_MODULE_DESC
{
	EFFECT_DISTRIBUTION_VECTOR_DESC Velocity;
};

struct EFFECT_INITIAL_SIZE_MODULE_DESC
{
	EFFECT_DISTRIBUTION_VECTOR_DESC Size = {
		EFFECT_DISTRIBUTION_TYPE::CONSTANT,
		{ 1.f, 1.f, 1.f },
		{ 1.f, 1.f, 1.f },
		{ 1.f, 1.f, 1.f }
	};
};

struct EFFECT_INITIAL_COLOR_MODULE_DESC
{
	EFFECT_DISTRIBUTION_COLOR_DESC Color;
};

struct EFFECT_CURVE_KEY
{
	f32_t fTime = {};
	f32_t fValue = {};
};

struct EFFECT_CURVE_FLOAT_DESC
{
	vector<EFFECT_CURVE_KEY> Keys = {
		{ 0.f, 1.f },
		{ 1.f, 1.f }
	};
};

struct EFFECT_CURVE_MODULE_DESC
{
	EFFECT_CURVE_FLOAT_DESC Curve;
};

struct EFFECT_COLLISION_MODULE_DESC
{
	float3_t vMinBounds = { -12.f, -8.f, -2.f };
	float3_t vMaxBounds = { 12.f, 12.f, 2.f };
	f32_t fRestitution = { 0.5f };
};

struct EFFECT_EVENT_MODULE_DESC
{
	string strEventName = "OnParticleHalfLife";
	f32_t fNormalizedTime = { 0.5f };
};

struct EFFECT_LOD_MODULE_DESC
{
	f32_t fNearDistance = {};
	f32_t fFarDistance = { 100.f };
	f32_t fFarSpawnScale = { 0.25f };
};

struct EFFECT_INITIAL_ROTATION_MODULE_DESC
{
	// Euler angles in degrees.
	EFFECT_DISTRIBUTION_VECTOR_DESC RotationDegrees;
};

struct EFFECT_ROTATION_RATE_MODULE_DESC
{
	// Euler angular velocity in degrees per second.
	EFFECT_DISTRIBUTION_VECTOR_DESC RotationRateDegreesPerSecond;
};

struct EFFECT_COLOR_OVER_LIFE_MODULE_DESC
{
	// Per-channel multipliers evaluated with particle relative time.
	EFFECT_CURVE_FLOAT_DESC Red;
	EFFECT_CURVE_FLOAT_DESC Green;
	EFFECT_CURVE_FLOAT_DESC Blue;
	EFFECT_CURVE_FLOAT_DESC Alpha;
};

struct EFFECT_SUB_UV_MODULE_DESC
{
	uint32_t iColumns = { 1 };
	uint32_t iRows = { 1 };
	f32_t fFramesPerSecond = { 30.f };
	uint32_t iStartFrame = {};
	uint32_t iEndFrame = {};
	bool_t isLoop = { true };
	bool_t useParticleRelativeTime = { true };
};

struct EFFECT_MESH_ANIMATION_MODULE_DESC
{
	uint32_t iAnimationIndex = {};
	f32_t fPlayRate = { 1.f };
	bool_t isLoop = { true };
};

struct EFFECT_BEAM_MODULE_DESC
{
	float3_t vSourceOffset = {};
	float3_t vTargetOffset = {};
	f32_t fWidth = { 1.f };
	uint32_t iSegments = { 1 };
	f32_t fNoiseAmplitude = {};
};

struct EFFECT_TRAIL_MODULE_DESC
{
	f32_t fWidth = { 1.f };
	f32_t fPointLifetime = { 0.5f };
	uint32_t iMaxPoints = { 64 };
	string strSourceBone;
	string strTargetBone;
};

struct EFFECT_MODULE_DESC
{
	uint64_t iModuleId = { INVALID_EFFECT_ELEMENT_ID };
	string strName;
	EFFECT_MODULE_TYPE eType = { EFFECT_MODULE_TYPE::END };
	bool_t isEnabled = { true };

	/*
	 * File/UI friendly payloads.
	 * eType selects the payload used by the runtime. Keeping them as plain
	 * data prevents the renderer, ImGui, and file loader from owning one
	 * another.
	 */
	EFFECT_REQUIRED_MODULE_DESC Required;
	EFFECT_SPAWN_MODULE_DESC Spawn;
	EFFECT_LIFETIME_MODULE_DESC Lifetime;
	EFFECT_INITIAL_LOCATION_MODULE_DESC InitialLocation;
	EFFECT_INITIAL_VELOCITY_MODULE_DESC InitialVelocity;
	EFFECT_INITIAL_SIZE_MODULE_DESC InitialSize;
	EFFECT_INITIAL_COLOR_MODULE_DESC InitialColor;
	EFFECT_CURVE_MODULE_DESC SizeOverLife;
	EFFECT_CURVE_MODULE_DESC AlphaOverLife;
	EFFECT_CURVE_MODULE_DESC VelocityOverLife;
	EFFECT_COLLISION_MODULE_DESC Collision;
	EFFECT_EVENT_MODULE_DESC Event;
	EFFECT_LOD_MODULE_DESC LOD;
	EFFECT_INITIAL_ROTATION_MODULE_DESC InitialRotation;
	EFFECT_ROTATION_RATE_MODULE_DESC RotationRate;
	EFFECT_COLOR_OVER_LIFE_MODULE_DESC ColorOverLife;
	EFFECT_SUB_UV_MODULE_DESC SubUV;
	EFFECT_MESH_ANIMATION_MODULE_DESC MeshAnimation;
	EFFECT_BEAM_MODULE_DESC Beam;
	EFFECT_TRAIL_MODULE_DESC Trail;
};

struct EFFECT_EMITTER_DESC
{
	uint64_t iEmitterId = { INVALID_EFFECT_ELEMENT_ID };
	string strName;
	EFFECT_EMITTER_TYPE eType = { EFFECT_EMITTER_TYPE::SPRITE };
	bool_t isEnabled = { true };
	EFFECT_SIMULATION_SPACE eSimulationSpace = { EFFECT_SIMULATION_SPACE::LOCAL };
	EFFECT_BLEND_MODE eBlendMode = { EFFECT_BLEND_MODE::ALPHA };
	EFFECT_SORT_MODE eSortMode = { EFFECT_SORT_MODE::NONE };
	f32_t fDelay = {};
	f32_t fDuration = { 1.f };
	uint32_t iLoopCount = { 1 };
	vector<EFFECT_MODULE_DESC> Modules;
};

struct EFFECT_ASSET_DESC
{
	uint32_t iSchemaVersion = { EFFECT_ASSET_SCHEMA_VERSION };
	string strAssetId;
	string strName;
	EFFECT_PROVENANCE eProvenance = { EFFECT_PROVENANCE::UNKNOWN };
	f32_t fDuration = { 1.f };
	f32_t fWarmupTime = {};
	vector<EFFECT_EMITTER_DESC> Emitters;
};

struct EFFECT_PARTICLE
{
	float3_t vPosition = {};
	float3_t vVelocity = {};
	float3_t vInitialVelocity = {};
	float3_t vSize = { 1.f, 1.f, 1.f };
	float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
	float3_t vRotation = {};
	float3_t vRotationRate = {};
	float3_t vInitialSize = { 1.f, 1.f, 1.f };
	float4_t vInitialColor = { 1.f, 1.f, 1.f, 1.f };
	f32_t fAge = {};
	f32_t fLifetime = { 1.f };
	f32_t fRelativeTime = {};
	uint32_t iRandomSeed = {};
	uint32_t iSubUVFrame = {};
	bool_t isEventTriggered = { false };
	bool_t isAlive = { false };
};

struct EFFECT_PARTICLE_SIMULATION_STATS
{
	uint32_t iAliveCount = {};
	uint64_t iTotalSpawned = {};
	uint64_t iTotalKilled = {};
	uint64_t iDroppedByCapacity = {};
	uint64_t iCollisionCount = {};
	uint64_t iEventCount = {};
};

struct EFFECT_PARTICLE_COUNT_VALIDATION_RESULT
{
	bool_t isPassed = { false };
	uint32_t iAliveAfterHalfSecond = {};
	uint32_t iAliveAfterOneSecond = {};
	uint32_t iAliveAfterOneAndHalfSeconds = {};
	uint64_t iTotalSpawned = {};
	uint64_t iTotalKilled = {};
};

NS_END
