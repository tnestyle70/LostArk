#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 5u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

enum class EFFECT_RESOURCE_SLOT : uint8_t
{
	MESH_MODEL,
	BASE_TEXTURE,
	NOISE_TEXTURE,
	MASK_TEXTURE,
	EMISSIVE_TEXTURE,
	DISSOLVE_TEXTURE,
	END
};

enum class EFFECT_RESOURCE_FILE_KIND : uint8_t
{
	MODEL,
	TEXTURE,
	END
};

enum class EFFECT_RENDER_PROFILE : uint8_t
{
	OPAQUE_BACK_DEPTH_WRITE,
	ALPHA_TWO_SIDED_DEPTH_READ,
	ADDITIVE_TWO_SIDED_DEPTH_READ,
	END
};

struct EFFECT_RESOURCE_BINDING_DESC final
{
	EFFECT_RESOURCE_SLOT eSlot = EFFECT_RESOURCE_SLOT::END;
	std::string strAssetId;
};

struct EFFECT_MATERIAL_DESC final
{
	EFFECT_RENDER_PROFILE eRenderProfile =
		EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
};

struct EFFECT_TRANSFORM_DESC final
{
	float3_t vPosition = { 0.f, 0.f, 0.f };
	float3_t vRotationDegrees = { 0.f, 0.f, 0.f };
	float3_t vRevolutionDegreesPerSecond = { 0.f, 0.f, 0.f };
	float3_t vScale = { 1.f, 1.f, 1.f };
	float3_t vVelocityPerSecond = { 0.f, 0.f, 0.f };
};

struct EFFECT_COLOR_DESC final
{
	float4_t vColorOffset = { 0.f, 0.f, 0.f, 0.f };
	float4_t vColorMultiply = { 1.f, 1.f, 1.f, 1.f };
	f32_t fColorClip = 0.f;
	f32_t fEmissiveIntensity = 1.f;
	f32_t fDistortionIntensity = 0.f;
	bool_t bDistortionOnBaseMaterial = false;
	f32_t fRadialTime = 0.f;
	f32_t fRadialIntensity = 0.f;
};

struct EFFECT_UV_DESC final
{
	float2_t vStart = { 0.f, 0.f };
	float2_t vSpeed = { 0.f, 0.f };
	bool_t bWave = false;
	float2_t vWaveAmplitude = { 0.f, 0.f };
	f32_t fWaveFrequency = 1.f;
	bool_t bSequence = false;
	bool_t bLoop = false;
	f32_t fSequenceTerm = 0.1f;
	int32_t iTileColumns = 1;
	int32_t iTileRows = 1;
	int32_t iTileIndex = 0;
};

struct EFFECT_TIMING_DESC final
{
	f32_t fStartDelaySeconds = 0.f;
	f32_t fLifeTimeSeconds = 1.f;
	f32_t fAfterImageSeconds = 0.f;
	f32_t fDissolveStartNormalized = 1.f;
};

struct EFFECT_MESH_DETAIL_DESC final
{
	bool_t bUseModelMaterial = true;
};

struct EFFECT_SPRITE_DETAIL_DESC final
{
	bool_t bBillboard = true;
};

struct EFFECT_DECAL_DETAIL_DESC final
{
	float2_t vSize = { 1.f, 1.f };
	f32_t fDepth = 0.25f;
};

struct EFFECT_LINEAR_LERP_DESC final
{
	bool_t bPosition = false;
	float3_t vEndPosition = { 0.f, 0.f, 0.f };
	bool_t bRotation = false;
	float3_t vEndRotationDegrees = { 0.f, 0.f, 0.f };
	bool_t bRevolution = false;
	float3_t vEndRevolutionDegreesPerSecond = { 0.f, 0.f, 0.f };
	bool_t bScale = false;
	float3_t vEndScale = { 1.f, 1.f, 1.f };
	bool_t bVelocity = false;
	float3_t vEndVelocityPerSecond = { 0.f, 0.f, 0.f };
	bool_t bColorOffset = false;
	float4_t vEndColorOffset = { 0.f, 0.f, 0.f, 0.f };
	bool_t bColorMultiply = false;
	float4_t vEndColorMultiply = { 1.f, 1.f, 1.f, 1.f };
	bool_t bEmissiveIntensity = false;
	f32_t fEndEmissiveIntensity = 1.f;
};

struct EFFECT_PARTICLE_DESC final
{
	uint32_t iMaxParticles = 256u;
	f32_t fSpawnRatePerSecond = 20.f;
	uint32_t iBurstCount = 0u;
	uint32_t iRandomSeed = 1u;
	float2_t vLifeTimeSeconds = { 0.5f, 1.f };
	float3_t vInitialVelocityMin = { -0.5f, 1.f, -0.5f };
	float3_t vInitialVelocityMax = { 0.5f, 2.f, 0.5f };
	float3_t vAcceleration = { 0.f, -1.f, 0.f };
	float2_t vStartSize = { 0.2f, 0.2f };
	float2_t vEndSize = { 0.f, 0.f };
	bool_t bLocalSpace = true;
	bool_t bBillboard = true;
};

struct EFFECT_TRAIL_DESC final
{
	uint32_t iMaxPoints = 64u;
	f32_t fPointLifeTimeSeconds = 0.35f;
	f32_t fSampleIntervalSeconds = 1.f / 60.f;
	f32_t fMinimumDistance = 0.01f;
	f32_t fStartWidth = 0.2f;
	f32_t fEndWidth = 0.f;
	bool_t bFaceCamera = true;
};

struct EFFECT_AFTERIMAGE_DESC final
{
	f32_t fSampleIntervalSeconds = 0.05f;
	uint32_t iMaxCopies = 16u;
	f32_t fAlphaExponent = 1.f;
};

struct EFFECT_DETAIL_DESC final
{
	EFFECT_TRANSFORM_DESC Transform;
	EFFECT_COLOR_DESC Color;
	EFFECT_UV_DESC UV;
	EFFECT_TIMING_DESC Timing;
	EFFECT_MESH_DETAIL_DESC Mesh;
	EFFECT_SPRITE_DETAIL_DESC Sprite;
	EFFECT_DECAL_DETAIL_DESC Decal;
	EFFECT_LINEAR_LERP_DESC LinearLerp;
	EFFECT_PARTICLE_DESC Particle;
	EFFECT_TRAIL_DESC Trail;
	EFFECT_AFTERIMAGE_DESC AfterImage;
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	std::vector<EFFECT_RESOURCE_BINDING_DESC> ResourceBindings;
	EFFECT_MATERIAL_DESC Material;
	EFFECT_DETAIL_DESC Detail;
};

struct EFFECT_DOCUMENT_DESC final
{
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strDisplayName;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
