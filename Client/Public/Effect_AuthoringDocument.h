#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_Distribution.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 12u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	LIGHT,
	SCREEN_POST,
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
	ALPHA_ONE_SIDED_DEPTH_READ,
	ADDITIVE_ONE_SIDED_DEPTH_READ,
	END
};

struct EFFECT_RESOURCE_BINDING_DESC final
{
	std::string strSlotId;
	std::string strAssetId;
};

enum class EFFECT_SOURCE_MATERIAL_STATUS : uint8_t
{
	SOURCE_EXACT,
	RUNTIME_EXACT,
	RECONSTRUCTED_PROFILE,
	UNSUPPORTED,
	MISSING_RESOURCE,
	END
};

struct EFFECT_NAMED_FLOAT_DESC final
{
	std::string strName;
	std::string strGroup;
	f32_t fValue = 0.f;
};

struct EFFECT_NAMED_FLOAT4_DESC final
{
	std::string strName;
	std::string strGroup;
	float4_t vValue{};
};

struct EFFECT_NAMED_BOOL_DESC final
{
	std::string strName;
	std::string strGroup;
	bool_t bValue = false;
};

enum class EFFECT_TEXTURE_ADDRESS_MODE : uint8_t
{
	WRAP,
	CLAMP,
	END
};

enum class EFFECT_TEXTURE_COLOR_SPACE : uint8_t
{
	LINEAR,
	SRGB,
	END
};

struct EFFECT_NAMED_TEXTURE_DESC final
{
	std::string strName;
	std::string strGroup;
	std::string strSourceObjectPath;
	std::string strAssetId;
	EFFECT_TEXTURE_ADDRESS_MODE eAddressU =
		EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
	EFFECT_TEXTURE_ADDRESS_MODE eAddressV =
		EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
	EFFECT_TEXTURE_COLOR_SPACE eColorSpace =
		EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
	std::string strSamplingEvidence = "legacy_default";
};

struct EFFECT_SOURCE_MATERIAL_DESC final
{
	bool_t bEnabled = false;
	std::string strProfileId;
	std::string strRuntimeShaderProfileId;
	std::string strParentMaterialPath;
	EFFECT_SOURCE_MATERIAL_STATUS eStatus =
		EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED;
	std::vector<EFFECT_NAMED_TEXTURE_DESC> Textures;
	std::vector<EFFECT_NAMED_FLOAT_DESC> Scalars;
	std::vector<EFFECT_NAMED_FLOAT4_DESC> Vectors;
	std::vector<EFFECT_NAMED_BOOL_DESC> StaticSwitches;
	std::array<std::string, 4u> DynamicParameterSemantics = {
		"unbound", "unbound", "unbound", "unbound"
	};
	std::string strSubUVMode = "none";
};

struct EFFECT_MATERIAL_DESC final
{
	std::string strTemplateId = "effect.standard";
	std::string strSourceMaterialPath;
	EFFECT_RENDER_PROFILE eRenderProfile =
		EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	EFFECT_SOURCE_MATERIAL_DESC SourceMaterial;
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
	float3_t vInitialPositionMin = { 0.f, 0.f, 0.f };
	float3_t vInitialPositionMax = { 0.f, 0.f, 0.f };
	float3_t vInitialVelocityMin = { -0.5f, 1.f, -0.5f };
	float3_t vInitialVelocityMax = { 0.5f, 2.f, 0.5f };
	float3_t vAcceleration = { 0.f, -1.f, 0.f };
	float2_t vStartSize = { 0.2f, 0.2f };
	float2_t vEndSize = { 0.f, 0.f };
	bool_t bLocalSpace = true;
	bool_t bBillboard = true;
};

enum class EFFECT_SOURCE_LITERAL_KIND : uint8_t
{
	BOOLEAN,
	NUMBER,
	STRING,
	END
};

struct EFFECT_SOURCE_LITERAL_DESC final
{
	std::string strPropertyPath;
	EFFECT_SOURCE_LITERAL_KIND eKind = EFFECT_SOURCE_LITERAL_KIND::END;
	bool_t bBoolean = false;
	f64_t fNumber = 0.0;
	std::string strString;
};

struct EFFECT_SOURCE_MODULE_DESC final
{
	std::string strStableId;
	std::string strClassName;
	std::string strObjectPath;
	std::vector<EFFECT_SOURCE_LITERAL_DESC> Literals;
	std::vector<EFFECT_DISTRIBUTION_DESC> Distributions;
};

struct EFFECT_PARTICLE_BURST_DESC final
{
	f32_t fTimeSeconds = 0.f;
	uint32_t iCountMinimum = 0u;
	uint32_t iCountMaximum = 0u;
};

struct EFFECT_CASCADE_RECIPE_DESC final
{
	bool_t bEnabled = false;
	std::string strRendererShape;
	f32_t fEmitterDelaySeconds = 0.f;
	f32_t fEmitterDurationSeconds = 0.f;
	uint32_t iEmitterLoopCount = 1u;
	std::vector<EFFECT_PARTICLE_BURST_DESC> Bursts;
	std::vector<EFFECT_SOURCE_MODULE_DESC> Modules;
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

enum class EFFECT_PRESENTATION_RUNTIME_STATUS : uint8_t
{
	RECONSTRUCTED_PROFILE,
	END
};

enum class EFFECT_LIGHT_PROFILE : uint8_t
{
	POINT_RECONSTRUCTED_V1,
	END
};

enum class EFFECT_SCREEN_POST_PROFILE : uint8_t
{
	RGB_NOISE_RECONSTRUCTED_V1,
	ZOOM_BLUR_RECONSTRUCTED_V1,
	FILM_NOISE_RECONSTRUCTED_V1,
	END
};

struct EFFECT_LIGHT_DETAIL_DESC final
{
	bool_t bEnabled = false;
	EFFECT_LIGHT_PROFILE eProfile = EFFECT_LIGHT_PROFILE::END;
	EFFECT_PRESENTATION_RUNTIME_STATUS eStatus =
		EFFECT_PRESENTATION_RUNTIME_STATUS::END;
	f32_t fRange = 1.f;
	f32_t fIntensity = 1.f;
	float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t vAmbient = { 0.f, 0.f, 0.f, 1.f };
	f32_t fFalloffExponent = 0.f;
};

struct EFFECT_SCREEN_POST_DETAIL_DESC final
{
	bool_t bEnabled = false;
	EFFECT_SCREEN_POST_PROFILE eProfile = EFFECT_SCREEN_POST_PROFILE::END;
	EFFECT_PRESENTATION_RUNTIME_STATUS eStatus =
		EFFECT_PRESENTATION_RUNTIME_STATUS::END;
	f32_t fIntensity = 0.f;
	f32_t fSecondaryIntensity = 0.f;
	f32_t fFrequency = 1.f;
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	uint32_t iRandomSeed = 1u;
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
	EFFECT_LIGHT_DETAIL_DESC Light;
	EFFECT_SCREEN_POST_DETAIL_DESC ScreenPost;
};

enum class EFFECT_SOURCE_PRESENTATION_STATUS : uint8_t
{
	SOURCE_EXACT,
	RECONSTRUCTED,
	UNRESOLVED,
	END
};

enum class EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND : uint8_t
{
	NUMBER,
	BOOLEAN,
	VECTOR,
	STRING,
	END
};

enum class EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS : uint8_t
{
	SOURCE_EXPLICIT,
	SOURCE_DISTRIBUTION,
	UNRESOLVED_CLASS_DEFAULT,
	END
};

struct EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC final
{
	std::string strName;
	EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND eKind =
		EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND::END;
	EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS eStatus =
		EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS::END;
	std::string strSourcePropertyPath;
	f64_t fNumberValue = 0.0;
	bool_t bBoolValue = false;
	float4_t vVectorValue{};
	std::string strStringValue;
};

struct EFFECT_SOURCE_PRESENTATION_DESC final
{
	bool_t bEnabled = false;
	std::string strSchema;
	uint32_t iVersion = 0u;
	std::string strProfileId;
	EFFECT_SOURCE_PRESENTATION_STATUS eStatus =
		EFFECT_SOURCE_PRESENTATION_STATUS::END;
	std::string strSourceObjectPath;
	std::string strSourceActionCueId;
	std::string strSourceEventId;
	uint32_t iSourceOccurrenceIndex = 0u;
	f32_t fSourceTimeSeconds = 0.f;
	std::vector<EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC> Parameters;
};

struct EFFECT_ACTION_CUE_ATTACHMENT_DESC final
{
	bool_t bEnabled = false;
	bool_t bFollow = false;
	std::string strSourceAnchorSlotId;
	std::string strRuntimeAnchorSlotId;
	std::string strRuntimeBoneName;
	EFFECT_TRANSFORM_DESC SocketLocalTransform;
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	std::string strDisplayName;
	std::string strGroupId;
	std::string strSourceNode;
	bool_t bVisible = true;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	std::vector<EFFECT_RESOURCE_BINDING_DESC> ResourceBindings;
	EFFECT_MATERIAL_DESC Material;
	EFFECT_ACTION_CUE_ATTACHMENT_DESC ActionCueAttachment;
	EFFECT_DETAIL_DESC Detail;
	EFFECT_CASCADE_RECIPE_DESC SourceRecipe;
	EFFECT_SOURCE_PRESENTATION_DESC SourcePresentation;
};

struct EFFECT_MODEL_CUE_DESC final
{
	std::string strCueId;
	std::string strModelAssetId;
	std::string strClipName;
	f32_t fStartDelaySeconds = 0.f;
	f32_t fDurationSeconds = 1.f;
	EFFECT_TRANSFORM_DESC LocalTransform;
	float3_t vAssetPreScale = { 1.f, 1.f, 1.f };
	float3_t vAssetPreRotationDegrees = { 0.f, 0.f, 0.f };
	bool_t bVisible = true;
};

struct EFFECT_PARTICLE_SYSTEM_DESC final
{
	f32_t fUniformScaleMultiplier = 1.f;
	f32_t fYawOffsetDegrees = 0.f;
	f32_t fDirectionYawDegrees = 0.f;
	f32_t fInitialSpeedMultiplier = 1.f;
};

struct EFFECT_DOCUMENT_DESC final
{
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strDisplayName;
	EFFECT_PARTICLE_SYSTEM_DESC ParticleSystem;
	std::vector<EFFECT_MODEL_CUE_DESC> ModelCues;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

inline void Apply_EffectElementDetailDraft(
	EFFECT_ELEMENT_DESC& Target,
	const EFFECT_ELEMENT_DESC& Draft)
{
	Target.strDisplayName = Draft.strDisplayName;
	Target.strGroupId = Draft.strGroupId;
	Target.strSourceNode = Draft.strSourceNode;
	Target.bVisible = Draft.bVisible;
	Target.Detail = Draft.Detail;
	Target.Material = Draft.Material;
	Target.SourceRecipe = Draft.SourceRecipe;
}

NS_END
