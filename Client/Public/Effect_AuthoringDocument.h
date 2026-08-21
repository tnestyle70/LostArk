#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_Distribution.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 13u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;
inline constexpr uint32_t EFFECT_SOURCE_CONTRACT_FORMAT_VERSION = 14u;

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

enum class EFFECT_RENDERER_TYPE : uint8_t
{
	STANDALONE_MESH,
	LEGACY_STANDALONE_SPRITE,
	MESH_PARTICLE,
	SPRITE_PARTICLE,
	DECAL_PARTICLE,
	ANIM_TRAIL,
	CASCADE_RIBBON,
	LIGHT_PARTICLE,
	SCREEN_POST,
	END
};

enum class EFFECT_SOURCE_SPACE : uint8_t
{
	CLIENT_METERS_V1,
	UE3_CASCADE_V1,
	SCREEN_SPACE_V1,
	END
};

struct EFFECT_RENDERER_DESC final
{
	EFFECT_RENDERER_TYPE eType = EFFECT_RENDERER_TYPE::END;
	EFFECT_SOURCE_SPACE eSourceSpace = EFFECT_SOURCE_SPACE::END;
};

enum class EFFECT_RESOURCE_SLOT : uint8_t
{
	MESH_MODEL,
	BASE_TEXTURE,
	NOISE_TEXTURE,
	MASK_TEXTURE,
	EMISSIVE_TEXTURE,
	DISSOLVE_TEXTURE,
	/* Second layer of the three slots the original Cascade materials actually
	   duplicate: diff_tex1/diff_tex2, opacity_tex/alpha_tex and
	   uv_noise_tex/uv_noise_01_tex. Appended after DISSOLVE_TEXTURE so the
	   existing serialized slot token order stays stable. */
	BASE2_TEXTURE,
	MASK2_TEXTURE,
	NOISE2_TEXTURE,
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

inline constexpr std::string_view
	EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX =
		"materialExecutionLane:";
inline constexpr std::string_view
	EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX =
		"sourceMaterialTexture:";

inline std::string Build_EffectMaterialExecutionLaneStableSlotId(
	const std::string_view strLaneId)
{
	return std::string(EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX) +
		std::string(strLaneId);
}

inline bool_t Try_ParseEffectMaterialExecutionLaneStableSlotId(
	const std::string_view strSlotId,
	std::string_view& strOutLaneId)
{
	if (strSlotId.size() <=
			EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX.size() ||
		0 != strSlotId.compare(0u,
			EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX.size(),
			EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX))
	{
		strOutLaneId = {};
		return false;
	}
	strOutLaneId = strSlotId.substr(
		EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX.size());
	return !strOutLaneId.empty();
}

inline std::string Build_EffectSourceMaterialTextureStableSlotId(
	const std::string_view strParameterName)
{
	return std::string(EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX) +
		std::string(strParameterName);
}

inline bool_t Try_ParseEffectSourceMaterialTextureStableSlotId(
	const std::string_view strSlotId,
	std::string_view& strOutParameterName)
{
	if (strSlotId.size() <=
			EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX.size() ||
		0 != strSlotId.compare(0u,
			EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX.size(),
			EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX))
	{
		strOutParameterName = {};
		return false;
	}
	strOutParameterName = strSlotId.substr(
		EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX.size());
	return !strOutParameterName.empty();
}

enum class EFFECT_SOURCE_MATERIAL_STATUS : uint8_t
{
	SOURCE_EXACT,
	RUNTIME_EXACT,
	RECONSTRUCTED_PROFILE,
	UNSUPPORTED,
	MISSING_RESOURCE,
	END
};

/* Compiler-captured source render evidence. UNKNOWN is deliberately the
   default so existing v13 documents stay byte-identical until their compiler
   explicitly supplies the field. */
enum class EFFECT_SOURCE_BLEND_CLASS : uint8_t
{
	UNKNOWN,
	ADDITIVE,
	TRANSLUCENT,
	MASKED,
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

enum class EFFECT_MATERIAL_EXECUTION_BACKEND : uint8_t
{
	GENERIC,
	RUNTIME_MATERIAL_V2,
	ARTIST_VISUAL_V4,
	LOCAL_DECAL,
	STANDARD_COLOR_V1,
	END
};

enum class EFFECT_STANDARD_COLOR_CHANNEL : uint8_t
{
	INVALID,
	R,
	G,
	B,
	A,
	RGB,
	END
};

enum class EFFECT_STANDARD_COLOR_EMISSIVE_MODE : uint8_t
{
	NONE,
	BASE_RADIANCE,
	END
};

enum class EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE : uint8_t
{
	INVALID,
	CARRIER_ALPHA,
	END
};

enum class EFFECT_STANDARD_COLOR_DISSOLVE_MODE : uint8_t
{
	NONE,
	LANE_THRESHOLD,
	END
};

enum class EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY : uint8_t
{
	INVALID,
	FAIL_CLOSED,
	END
};

enum class EFFECT_MATERIAL_TEXTURE_FILTER : uint8_t
{
	POINT,
	LINEAR,
	ANISOTROPIC,
	END
};

enum class EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE : uint8_t
{
	WRAP,
	MIRROR,
	CLAMP,
	BORDER,
	END
};

enum class EFFECT_MATERIAL_COMPARISON_FUNCTION : uint8_t
{
	NEVER,
	LESS,
	EQUAL,
	LESS_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_EQUAL,
	ALWAYS,
	END
};

struct EFFECT_MATERIAL_SAMPLER_DESC final
{
	EFFECT_MATERIAL_TEXTURE_FILTER eFilter =
		EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR;
	EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE eAddressU =
		EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
	EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE eAddressV =
		EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
	EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE eAddressW =
		EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
	f32_t fMipLodBias = 0.f;
	uint32_t iMaxAnisotropy = 1u;
	EFFECT_MATERIAL_COMPARISON_FUNCTION eComparison =
		EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER;
	float4_t vBorderColor = { 0.f, 0.f, 0.f, 0.f };
	f32_t fMinLod = 0.f;
	f32_t fMaxLod = 1000.f;
};

struct EFFECT_MATERIAL_TEXTURE_LANE_DESC final
{
	std::string strLaneId;
	std::string strRole;
	std::string strAssetId;
	uint32_t iTextureRegister = 0u;
	uint32_t iSamplerRegister = 0u;
	std::string strSourceChannel;
	EFFECT_TEXTURE_COLOR_SPACE eColorSpace =
		EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
	EFFECT_MATERIAL_SAMPLER_DESC Sampler;
};

struct EFFECT_STANDARD_COLOR_V1_DESC final
{
	uint32_t iPacketVersion = 0u;
	std::string strBaseRadianceLaneId;
	EFFECT_STANDARD_COLOR_CHANNEL eBaseRadianceChannel =
		EFFECT_STANDARD_COLOR_CHANNEL::INVALID;
	std::string strCoverageLaneId;
	EFFECT_STANDARD_COLOR_CHANNEL eCoverageChannel =
		EFFECT_STANDARD_COLOR_CHANNEL::INVALID;
	EFFECT_STANDARD_COLOR_EMISSIVE_MODE eEmissiveMode =
		EFFECT_STANDARD_COLOR_EMISSIVE_MODE::NONE;
	EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE eLifetimeEnvelope =
		EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::INVALID;
	EFFECT_STANDARD_COLOR_DISSOLVE_MODE eDissolveMode =
		EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE;
	std::string strDissolveLaneId;
	EFFECT_STANDARD_COLOR_CHANNEL eDissolveChannel =
		EFFECT_STANDARD_COLOR_CHANNEL::INVALID;
	f32_t fDissolveSoftness = 0.f;
	EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY eMissingLanePolicy =
		EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::INVALID;
};

struct EFFECT_MATERIAL_SCALAR_PARAMETER_DESC final
{
	std::string strName;
	uint32_t iPackedIndex = 0u;
	f32_t fValue = 0.f;
};

struct EFFECT_MATERIAL_VECTOR_PARAMETER_DESC final
{
	std::string strName;
	uint32_t iPackedIndex = 0u;
	float4_t vValue{};
};

struct EFFECT_MATERIAL_EXECUTION_DESC final
{
	bool_t bEnabled = false;
	/* A disabled recipe normally means ordinary generic rendering. FailClosed
	   instead means the source pass is unresolved and must never fall back to
	   generic/white rendering. */
	bool_t bFailClosed = false;
	/* Authoring-only relaxation of the FailClosed marker. It is set only when
	   the element already owns its exact source resources - an exact WModel for
	   a mesh carrier, an existing exact DDS for a sprite carrier - and only the
	   material semantics remain unproven. Such an element draws an explicitly
	   approximate preview so the artist can tune transform, size and colour.
	   Product use requires a hash-pinned, exact-cue user approval and keeps the
	   PRODUCT_APPROVED_APPROXIMATE label; an unapproved carrier remains denied.
	   It never stands on its own: it is meaningless unless bFailClosed is also
	   true. */
	bool_t bAuthoringApproximate = false;
	uint32_t iVersion = 1u;
	EFFECT_MATERIAL_EXECUTION_BACKEND eBackend =
		EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC;
	uint32_t iOpcode = 0u;
	uint32_t iPassIndex = 0u;
	std::string strRasterizerState;
	std::string strDepthStencilState;
	std::string strBlendState;
	uint32_t iStencilReference = 0u;
	uint32_t iTextureLaneCount = 0u;
	uint32_t iTextureMask = 0u;
	std::vector<EFFECT_MATERIAL_TEXTURE_LANE_DESC> TextureLanes;
	EFFECT_STANDARD_COLOR_V1_DESC StandardColorV1;
	uint32_t iDynamicConsumedMask = 0u;
	uint32_t iDynamicSuppressedMask = 0u;
	uint32_t iParticleColorPolicy = 0u;
	uint32_t iParticleColorConsumedMask = 0u;
	uint32_t iParticleColorSuppressedMask = 0u;
	uint32_t iScalarCount = 0u;
	uint32_t iVectorCount = 0u;
	uint32_t iInputCount = 0u;
	std::array<uint32_t, 2u> InputConsumedMask{};
	std::array<uint32_t, 2u> InputSuppressedMask{};
	std::array<uint32_t, 3u> VectorComponentConsumedMask{};
	std::array<uint32_t, 3u> VectorComponentSuppressedMask{};
	uint32_t iStaticInputCount = 0u;
	uint32_t iStaticSelectedMask = 0u;
	uint32_t iStaticConsumedMask = 0u;
	uint32_t iStaticSuppressedMask = 0u;
	uint32_t iRenderInputCount = 0u;
	uint32_t iRenderConsumedMask = 0u;
	uint32_t iRenderSuppressedMask = 0u;
	std::vector<EFFECT_MATERIAL_SCALAR_PARAMETER_DESC> Scalars;
	std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC> Vectors;
	std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC> ArtistParameters;
	std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC> Colors;
};

/* A hard fail-closed row is retained only as source evidence.  An explicitly
   authoring-approximate row is still an execution target and therefore must
   satisfy every ordinary runtime carrier/admission validator before preview. */
inline bool_t Is_EffectAuthoringExecutionTarget(
	const EFFECT_MATERIAL_EXECUTION_DESC& Execution)
{
	return !Execution.bFailClosed || Execution.bAuthoringApproximate;
}

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
	EFFECT_SOURCE_BLEND_CLASS eSourceBlendClass =
		EFFECT_SOURCE_BLEND_CLASS::UNKNOWN;
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
	EFFECT_MATERIAL_EXECUTION_DESC Execution;
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
	/* Geometry import scale applied by CModel exactly once before particle
	   StartSize/Element Transform. Track A WModel carriers use 0.01. */
	f32_t fModelPreScale = 1.f;
	// UE source order: roll(X), pitch(Y), yaw(Z), in degrees.
	float3_t vSourceTypeDataRotationDegrees = { 0.f, 0.f, 0.f };
};

struct EFFECT_SPRITE_DETAIL_DESC final
{
	bool_t bBillboard = true;
	f32_t fBillboardRollDegrees = 0.f;
	/* A billboard's world orientation is rebuilt from the camera every frame,
	   so Detail.Transform rotation cannot reach it and the only rotation a
	   billboarded quad has is this roll about the view axis. The constant above
	   sets where it starts; this rate is what actually makes it spin. */
	f32_t fBillboardRollDegreesPerSecond = 0.f;
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

/* Cascade owns the spawn volume in particlemodulelocation*; the authored Detail
   only had an axis-aligned min/max box, so a ring or a sphere shell could not be
   written by hand at all.  POINT keeps the historical box, which is what every
   document written before this field existed still means. */
enum class EFFECT_PARTICLE_SPAWN_SHAPE : uint8_t
{
	POINT,
	SPHERE,
	RING,
	BOX,
	END
};

/* Cascade owns the emission direction in particlemodulevelocity* and in the
   velocity flag of the location primitives.  FIXED keeps the historical
   component-wise min/max velocity box. */
enum class EFFECT_PARTICLE_VELOCITY_MODE : uint8_t
{
	FIXED,
	OUTWARD,
	INWARD,
	CONE,
	END
};

/* The attractor is an authored motion layer, not a recovered Cascade module.
   ROOT_LOCAL lets several independently transformed occurrences converge on
   one effect/caster centre. ELEMENT_LOCAL keeps a self-contained carrier
   centred on its own authored transform. */
enum class EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE : uint8_t
{
	ROOT_LOCAL,
	ELEMENT_LOCAL,
	END
};

struct EFFECT_PARTICLE_SPAWN_SHAPE_DESC final
{
	EFFECT_PARTICLE_SPAWN_SHAPE eKind = EFFECT_PARTICLE_SPAWN_SHAPE::POINT;
	/* Outer radius in Client metres for SPHERE and RING. */
	f32_t fRadius = 0.f;
	/* Inner radius in Client metres.  RING uses it as the hole, SPHERE as the
	   shell floor; must stay at or below fRadius. */
	f32_t fInnerRadius = 0.f;
	/* Half-extents in Client metres for BOX. */
	float3_t vExtents = { 0.f, 0.f, 0.f };
	/* Angular span the shape is allowed to occupy, centred on +X of the Element.
	   360 is the whole circle or sphere. */
	f32_t fArcDegrees = 360.f;
};

struct EFFECT_PARTICLE_INITIAL_VELOCITY_DESC final
{
	EFFECT_PARTICLE_VELOCITY_MODE eMode = EFFECT_PARTICLE_VELOCITY_MODE::FIXED;
	/* Metres per second, [minimum, maximum]; ignored while eMode is FIXED. */
	float2_t vSpeedRange = { 0.f, 0.f };
	/* Half-angle of the CONE around the Element +Y axis, in degrees. */
	f32_t fConeAngleDegrees = 0.f;
};

struct EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC final
{
	bool_t bEnabled = false;
	EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE eTargetSpace =
		EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE::ROOT_LOCAL;
	/* Metres in the selected target space. */
	float3_t vTargetOffset = { 0.f, 0.f, 0.f };
	/* Particle-normalized active interval. */
	float2_t vActiveNormalized = { 0.f, 1.f };
	/* World metres per second squared. Tangential acceleration is signed around
	   world +Y, independent of Element pitch and non-uniform scale. */
	f32_t fRadialAcceleration = 0.f;
	f32_t fTangentialAcceleration = 0.f;
	/* World metres per second; required and positive while enabled. */
	f32_t fMaximumSpeed = 1.f;
	/* World-metre radius. Entering it captures the particle at the target. */
	f32_t fConvergenceRadius = 0.05f;
	/* Per-second damping, weighted by the physically derived braking radius. */
	f32_t fArrivalDamping = 0.f;

	bool_t Is_Default() const
	{
		return !bEnabled &&
			eTargetSpace ==
				EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE::ROOT_LOCAL &&
			vTargetOffset.x == 0.f && vTargetOffset.y == 0.f &&
			vTargetOffset.z == 0.f && vActiveNormalized.x == 0.f &&
			vActiveNormalized.y == 1.f && fRadialAcceleration == 0.f &&
			fTangentialAcceleration == 0.f && fMaximumSpeed == 1.f &&
			fConvergenceRadius == 0.05f && fArrivalDamping == 0.f;
	}
};

/* Authored trim over a source-owned Element.

   Clearing SourceRecipe.bEnabled is not a per-axis gate: it swaps the whole
   simulator, and only 109 of the 4,609 source-owned particle Elements in the
   corpus have a module stack the authored schema can express.  The other 4,500
   carry rotation, mesh rotation, camera offset, sub-UV, orbit or a shader
   dynamic parameter that the flip would silently stop running.

   These factors multiply the source's own count, size, lifetime, velocity,
   rotation, alpha and emitter delay instead of replacing the source recipe,
   which leaves every module the authored schema does not understand running. */
struct EFFECT_PARTICLE_SOURCE_SCALE_DESC final
{
	/* Scales the source spawn rate and burst count together. */
	f32_t fCount = 1.f;
	/* Scales the source start size on both axes. */
	f32_t fSize = 1.f;
	/* Scales the source particle lifetime. */
	f32_t fLifeTime = 1.f;
	/* Scales the source spawn velocity, so the burst reaches further or
	   nearer without touching the modules that produced its direction. */
	f32_t fSpeed = 1.f;
	/* Scales source-owned initial rotation and update rotation rates for both
	   the sprite roll and the mesh particle spin. */
	f32_t fRotation = 1.f;
	/* Scales the source colour alpha. The RGB the modules chose is kept. */
	f32_t fAlpha = 1.f;
	/* Scales the emitter's own start delay, which slides this Element inside
	   the cue without editing the cue. */
	f32_t fSpawnDelay = 1.f;

	bool_t Is_Default() const
	{
		return 1.f == fCount && 1.f == fSize && 1.f == fLifeTime &&
			1.f == fSpeed && 1.f == fRotation && 1.f == fAlpha &&
			1.f == fSpawnDelay;
	}
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
	uint32_t iDynamicParameterComponentMask = 0u;
	float4_t vDynamicParameterStart{};
	float4_t vDynamicParameterEnd{};
	/* Both default to the historical POINT/FIXED behaviour, so a document that
	   omits them spawns exactly as it did before the fields existed. */
	EFFECT_PARTICLE_SPAWN_SHAPE_DESC SpawnShape;
	EFFECT_PARTICLE_INITIAL_VELOCITY_DESC InitialVelocity;
	EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC TargetAttractor;
	/* Read only while SourceRecipe.bEnabled; all ones means untouched. */
	EFFECT_PARTICLE_SOURCE_SCALE_DESC SourceScale;
};

enum class EFFECT_SOURCE_LITERAL_KIND : uint8_t
{
	BOOLEAN,
	NUMBER,
	STRING,
	END
};

enum class EFFECT_SOURCE_COVERAGE_STATUS : uint8_t
{
	SOURCE_DECODED,
	DETERMINISTIC_CONVERSION,
	METADATA_ONLY,
	UNRESOLVED,
	END
};

struct EFFECT_SOURCE_PROPERTY_COVERAGE_DESC final
{
	std::string strPropertyPath;
	std::string strStorage;
	std::string strProvenance;
	EFFECT_SOURCE_COVERAGE_STATUS eStatus =
		EFFECT_SOURCE_COVERAGE_STATUS::END;
	std::vector<std::string> Blockers;
};

struct EFFECT_SOURCE_MODULE_COVERAGE_DESC final
{
	std::string strModuleStableId;
	std::string strExactSourceClass;
	std::string strAliasId;
	std::string strNormalizedClass;
	EFFECT_SOURCE_COVERAGE_STATUS eStatus =
		EFFECT_SOURCE_COVERAGE_STATUS::END;
	std::vector<std::string> Blockers;
	std::vector<EFFECT_SOURCE_PROPERTY_COVERAGE_DESC> Properties;
};

struct EFFECT_SOURCE_MODULE_REFERENCE_DESC final
{
	uint32_t iOrder = 0u;
	uint32_t iSourceReferenceIndex = 0u;
	std::string strRole;
	std::string strSourceObjectId;
	std::string strSourceRecordSha256;
};

struct EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC final
{
	uint32_t iSourceIndex = 0u;
	std::string strName;
	uint32_t iSourceTypeCode = 0u;
	uint32_t iSourceRecordByteOffset = 0u;
	std::string strType;
	f64_t fScalarValue = 0.0;
	float3_t vVectorValue = { 0.f, 0.f, 0.f };
	uint32_t iSourceValueByteOffset = 0u;
};

struct EFFECT_SOURCE_COMPILER_EVIDENCE_DESC final
{
	std::string strArtifactFileSha256;
	std::string strArtifactSelfSha256;
	std::string strEvidenceId;
	std::string strSourceEvidenceStatus;
	std::string strSourceCueId;
	std::string strSourceOccurrenceId;
	std::string strSourceSystemId;
	std::string strSourceEmitterPath;
	std::string strSourceEmitterNodeId;
	std::string strLodSelectionPolicy;
	std::string strSelectedLodPath;
	std::string strSelectedLodNodeId;
	uint32_t iSelectedLodArrayIndex = 0u;
	std::string strSelectedLodLevelProvenance;
	std::string strSelectedLodEnabledProvenance;
	uint32_t iNonSelectedLodCount = 0u;
	std::vector<EFFECT_SOURCE_MODULE_REFERENCE_DESC> ModuleReferenceOrder;
	float3_t vCueSourcePositionUeUnits = { 0.f, 0.f, 0.f };
	EFFECT_TRANSFORM_DESC CueLocalTransform;
	std::vector<EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC> ParameterOverrides;
	std::vector<std::string> CompositionOrder;
	std::string strLocalReferenceClosureFileSha256;
	std::string strLocalReferenceClosureSelfSha256;
	std::string strGeometryParityFileSha256;
	std::string strGeometryParitySelfSha256;
};

struct EFFECT_SOURCE_MATERIAL_ADMISSION_DESC final
{
	std::string strStatus;
	std::vector<std::string> SourceMaterialPaths;
	std::string strMaterialRecipeId;
	std::string strRenderStateRecipeId;
	std::vector<std::string> Blockers;
};

struct EFFECT_SOURCE_GEOMETRY_BINDING_DESC final
{
	bool_t bEnabled = false;
	std::string strAssetId;
	std::string strReceiptFileSha256;
	std::string strReceiptSelfSha256;
	f32_t fCarrierGeometryPreScale = 1.f;
	std::string strParticleScaleSemantics;
	std::string strStatus;
	std::vector<std::string> Blockers;
};

struct EFFECT_SOURCE_LITERAL_DESC final
{
	std::string strPropertyPath;
	EFFECT_SOURCE_LITERAL_KIND eKind = EFFECT_SOURCE_LITERAL_KIND::END;
	bool_t bBoolean = false;
	f64_t fNumber = 0.0;
	std::string strString;
};

enum class EFFECT_SOURCE_TYPED_FIELD_KIND : uint8_t
{
	BOOLEAN,
	NUMBER,
	STRING,
	VECTOR,
	END
};

struct EFFECT_SOURCE_TYPED_FIELD_DESC final
{
	std::string strPropertyPath;
	EFFECT_SOURCE_TYPED_FIELD_KIND eKind =
		EFFECT_SOURCE_TYPED_FIELD_KIND::END;
	bool_t bBoolean = false;
	f64_t fNumber = 0.0;
	std::string strString;
	float4_t vVector = { 0.f, 0.f, 0.f, 0.f };
};

struct EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC final
{
	std::string strReferenceKind;
	std::string strReferenceId;
	std::string strDefinitionId;
	std::string strOccurrenceId;
	std::string strModuleStableId;
	std::string strPropertyPath;
	std::string strProvenance;
	std::vector<EFFECT_SOURCE_TYPED_FIELD_DESC> ExactPayload;
	std::vector<EFFECT_SOURCE_TYPED_FIELD_DESC> CurrentDefaultEvidence;
	EFFECT_SOURCE_ADMISSION_DESC ExecutionAdmission;
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
	std::string strSourceContractProfileId;
	std::string strSourceContractSha256;
	std::string strSourceGraphSha256;
	std::string strSourceClosureSha256;
	std::string strSourceMaterialClosureSha256;
	uint32_t iSourcePeakActiveParticles = 0u;
	f32_t fEmitterDelaySeconds = 0.f;
	f32_t fEmitterDurationSeconds = 0.f;
	uint32_t iEmitterLoopCount = 1u;
	std::vector<EFFECT_PARTICLE_BURST_DESC> Bursts;
	std::vector<EFFECT_SOURCE_MODULE_DESC> Modules;
	std::vector<EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC>
		LocalReferenceBindings;
	std::vector<EFFECT_SOURCE_MODULE_COVERAGE_DESC> ModuleCoverage;
	EFFECT_SOURCE_COMPILER_EVIDENCE_DESC CompilerEvidence;
	EFFECT_SOURCE_ADMISSION_DESC CompiledExecutionAdmission;
	EFFECT_SOURCE_MATERIAL_ADMISSION_DESC MaterialAdmission;
	EFFECT_SOURCE_GEOMETRY_BINDING_DESC GeometryBinding;
};

struct EFFECT_TRAIL_DESC final
{
	uint32_t iMaxPoints = 64u;
	f32_t fPointLifeTimeSeconds = 0.35f;
	f32_t fSampleIntervalSeconds = 1.f / 60.f;
	f32_t fMinimumDistance = 0.01f;
	f32_t fStartWidth = 0.2f;
	f32_t fEndWidth = 0.f;
	/* Reconstructed Ribbon-only geometry inputs.  Zero keeps the authored
	   legacy Trail contract (point-index U and no typed tessellation). */
	f32_t fTilingDistanceWorldUnits = 0.f;
	f32_t fDistanceTessellationStepWorldUnits = 0.f;
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
	f32_t fFalloffExponent = 1.f;
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
	/* Root-snapshot ActionCues are authored in the source character basis.
	   Bone-follow cues already inherit that basis from the prepared model bone,
	   so this correction is valid only for enabled, non-follow attachments. */
	f32_t fSnapshotRootSourceBasisYawDegrees = 0.f;
	EFFECT_TRANSFORM_DESC SocketLocalTransform;
};

struct EFFECT_TRANSFORM_INHERITANCE_DESC final
{
	bool_t bEnabled = false;
	std::string strMasterElementId;
};

/* Values the artist re-bound or retuned in the Effect Tool.

   The compiler owns the source recipe, the source material identity and the
   exact resource bindings, and it rewrites all of them on every
   re-materialize. Without this block a Particle carrier has no artist-owned
   material surface at all, so a re-import silently discards every DDS rebind
   and parameter tweak - only Decal carried an exception. Overrides are
   re-applied last, after the compiler stage refreshes the element.

   An override is a tuning decision, never a claim of exactness: it never
   changes graph provenance and never grants product admission. */
/* Each override records the compiler value it replaced so the tool can offer
   an exact "Reset to compiler value" without a re-materialize round trip. The
   baseline is refreshed on every re-import, so a reset always lands on the
   current compiler answer rather than a stale one. */
struct EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC final
{
	/* slotId must already exist on the compiler-produced element. */
	std::string strSlotId;
	std::string strAssetId;
	std::string strCompilerAssetId;
};

struct EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC final
{
	/* Name must already be declared by the element's source profile. */
	std::string strName;
	f32_t fValue = 0.f;
	f32_t fCompilerValue = 0.f;
};

struct EFFECT_AUTHORING_COLOR_OVERRIDE_DESC final
{
	std::string strName;
	float4_t vValue{};
	float4_t vCompilerValue{};
};

struct EFFECT_AUTHORING_OVERRIDES_DESC final
{
	std::vector<EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC> ResourceBindings;
	std::vector<EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC> Scalars;
	std::vector<EFFECT_AUTHORING_COLOR_OVERRIDE_DESC> Colors;

	bool_t Is_Empty() const
	{
		return ResourceBindings.empty() && Scalars.empty() && Colors.empty();
	}
};

/* Fidelity is a display contract only. It never decides admission: an
   ARTIST_TUNED row can still be APPROXIMATE and refused for product. */
enum class EFFECT_AUTHORING_FIDELITY : uint8_t
{
	EXACT,
	RECONSTRUCTED,
	APPROXIMATE,
	INCOMPLETE,
	END
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	std::string strDisplayName;
	std::string strGroupId;
	std::string strSourceNode;
	bool_t bVisible = true;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	EFFECT_RENDERER_DESC Renderer;
	std::vector<EFFECT_RESOURCE_BINDING_DESC> ResourceBindings;
	/* Resources-relative ids the original emitter referenced but that did not
	   fit the material template's slots. Kept so nothing the source declared
	   is lost, and so the Tool can offer them as swap candidates. They are
	   never bound or drawn on their own. */
	std::vector<std::string> UnboundSourceResources;
	EFFECT_MATERIAL_DESC Material;
	EFFECT_ACTION_CUE_ATTACHMENT_DESC ActionCueAttachment;
	EFFECT_TRANSFORM_INHERITANCE_DESC TransformInheritance;
	EFFECT_DETAIL_DESC Detail;
	EFFECT_CASCADE_RECIPE_DESC SourceRecipe;
	EFFECT_SOURCE_PRESENTATION_DESC SourcePresentation;
	EFFECT_AUTHORING_OVERRIDES_DESC AuthoringOverrides;
};

inline bool_t Is_EffectPresentationExecutionTarget(
	const EFFECT_ELEMENT_DESC& Element)
{
	switch (Element.eKind)
	{
	case EFFECT_ELEMENT_KIND::LIGHT:
		return Element.Detail.Light.bEnabled &&
			Element.Detail.Light.eProfile != EFFECT_LIGHT_PROFILE::END &&
			Element.Detail.Light.eStatus !=
				EFFECT_PRESENTATION_RUNTIME_STATUS::END;
	case EFFECT_ELEMENT_KIND::SCREEN_POST:
		return Element.Detail.ScreenPost.bEnabled &&
			Element.Detail.ScreenPost.eProfile !=
				EFFECT_SCREEN_POST_PROFILE::END &&
			Element.Detail.ScreenPost.eStatus !=
				EFFECT_PRESENTATION_RUNTIME_STATUS::END;
	case EFFECT_ELEMENT_KIND::MESH:
	case EFFECT_ELEMENT_KIND::SPRITE:
	case EFFECT_ELEMENT_KIND::PARTICLE:
	case EFFECT_ELEMENT_KIND::DECAL:
	case EFFECT_ELEMENT_KIND::TRAIL:
	case EFFECT_ELEMENT_KIND::END:
	default:
		return false;
	}
}

/* G3's compiler-derived authoring family is intentionally separate from the
   Effect Tool's legacy six-way creation selector. It is a read-only
   classification of renderer shape, source blend evidence, and SubUV. */
enum class EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE : uint8_t
{
	UNKNOWN,
	SPRITE,
	MESH,
	DECAL,
	END
};

enum class EFFECT_GENERIC_AUTHORING_SUBUV_CLASS : uint8_t
{
	UNKNOWN,
	NONE,
	SUBUV,
	END
};

enum class EFFECT_GENERIC_AUTHORING_FAMILY : uint8_t
{
	SPRITE_ADDITIVE,
	SPRITE_TRANSLUCENT,
	MESH_TRANSLUCENT,
	SPRITE_TRANSLUCENT_SUBUV,
	MESH_ADDITIVE,
	SPRITE_ADDITIVE_SUBUV,
	MESH_MASKED,
	DECAL_TRANSLUCENT,
	SPRITE_MASKED,
	MESH_MASKED_SUBUV,
	MESH_TRANSLUCENT_SUBUV,
	DECAL_ADDITIVE,
	MESH_UNKNOWN,
	END
};

inline EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE
Resolve_EffectGenericAuthoringRendererShape(
	const EFFECT_ELEMENT_DESC& Element)
{
	if (Element.SourceRecipe.strRendererShape == "sprite")
		return EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::SPRITE;
	if (Element.SourceRecipe.strRendererShape == "mesh")
		return EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::MESH;
	if (Element.SourceRecipe.strRendererShape == "decal" ||
		Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
	{
		return EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::DECAL;
	}
	return EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::UNKNOWN;
}

inline EFFECT_SOURCE_BLEND_CLASS Resolve_EffectGenericAuthoringBlendClass(
	const EFFECT_ELEMENT_DESC& Element)
{
	if (Element.Material.SourceMaterial.eSourceBlendClass !=
		EFFECT_SOURCE_BLEND_CLASS::UNKNOWN)
	{
		return Element.Material.SourceMaterial.eSourceBlendClass;
	}

	/* renderProfile is the compiler's bounded fallback when source evidence was
	   unavailable. Opaque is not guessed as Masked; only explicit source
	   evidence may make that claim. */
	switch (Element.Material.eRenderProfile)
	{
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
		return EFFECT_SOURCE_BLEND_CLASS::TRANSLUCENT;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
		return EFFECT_SOURCE_BLEND_CLASS::ADDITIVE;
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
	case EFFECT_RENDER_PROFILE::END:
	default:
		return EFFECT_SOURCE_BLEND_CLASS::UNKNOWN;
	}
}

inline EFFECT_GENERIC_AUTHORING_SUBUV_CLASS
Resolve_EffectGenericAuthoringSubUVClass(const EFFECT_ELEMENT_DESC& Element)
{
	const std::string& strMode = Element.Material.SourceMaterial.strSubUVMode;
	if (strMode == "none")
		return EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::NONE;
	if (strMode == "psuvim_random" ||
		strMode == "psuvim_linear_blend" ||
		strMode == "psuvim_linear_blend_random_flip_square")
	{
		return EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::SUBUV;
	}
	return EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::UNKNOWN;
}

inline EFFECT_GENERIC_AUTHORING_FAMILY Resolve_EffectGenericAuthoringFamily(
	const EFFECT_ELEMENT_DESC& Element)
{
	const EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE eShape =
		Resolve_EffectGenericAuthoringRendererShape(Element);
	const EFFECT_SOURCE_BLEND_CLASS eBlend =
		Resolve_EffectGenericAuthoringBlendClass(Element);
	const EFFECT_GENERIC_AUTHORING_SUBUV_CLASS eSubUV =
		Resolve_EffectGenericAuthoringSubUVClass(Element);

	if (eShape == EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::MESH &&
		eBlend == EFFECT_SOURCE_BLEND_CLASS::UNKNOWN)
	{
		return EFFECT_GENERIC_AUTHORING_FAMILY::MESH_UNKNOWN;
	}
	if (eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::UNKNOWN)
		return EFFECT_GENERIC_AUTHORING_FAMILY::END;

	if (eShape == EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::SPRITE)
	{
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::ADDITIVE)
		{
			return eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::SUBUV ?
				EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_ADDITIVE_SUBUV :
				EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_ADDITIVE;
		}
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::TRANSLUCENT)
		{
			return eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::SUBUV ?
				EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_TRANSLUCENT_SUBUV :
				EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_TRANSLUCENT;
		}
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::MASKED &&
			eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::NONE)
		{
			return EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_MASKED;
		}
	}
	else if (eShape == EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::MESH)
	{
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::ADDITIVE &&
			eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::NONE)
		{
			return EFFECT_GENERIC_AUTHORING_FAMILY::MESH_ADDITIVE;
		}
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::TRANSLUCENT)
		{
			return eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::SUBUV ?
				EFFECT_GENERIC_AUTHORING_FAMILY::MESH_TRANSLUCENT_SUBUV :
				EFFECT_GENERIC_AUTHORING_FAMILY::MESH_TRANSLUCENT;
		}
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::MASKED)
		{
			return eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::SUBUV ?
				EFFECT_GENERIC_AUTHORING_FAMILY::MESH_MASKED_SUBUV :
				EFFECT_GENERIC_AUTHORING_FAMILY::MESH_MASKED;
		}
	}
	else if (eShape == EFFECT_GENERIC_AUTHORING_RENDERER_SHAPE::DECAL &&
		eSubUV == EFFECT_GENERIC_AUTHORING_SUBUV_CLASS::NONE)
	{
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::ADDITIVE)
			return EFFECT_GENERIC_AUTHORING_FAMILY::DECAL_ADDITIVE;
		if (eBlend == EFFECT_SOURCE_BLEND_CLASS::TRANSLUCENT)
			return EFFECT_GENERIC_AUTHORING_FAMILY::DECAL_TRANSLUCENT;
	}
	return EFFECT_GENERIC_AUTHORING_FAMILY::END;
}

inline const char_t* Get_EffectGenericAuthoringFamilyLabel(
	const EFFECT_GENERIC_AUTHORING_FAMILY eFamily)
{
	switch (eFamily)
	{
	case EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_ADDITIVE:
		return "Sprite Additive";
	case EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_TRANSLUCENT:
		return "Sprite Translucent";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_TRANSLUCENT:
		return "Mesh Translucent";
	case EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_TRANSLUCENT_SUBUV:
		return "Sprite Translucent SubUV";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_ADDITIVE:
		return "Mesh Additive";
	case EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_ADDITIVE_SUBUV:
		return "Sprite Additive SubUV";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_MASKED:
		return "Mesh Masked";
	case EFFECT_GENERIC_AUTHORING_FAMILY::DECAL_TRANSLUCENT:
		return "Decal Translucent";
	case EFFECT_GENERIC_AUTHORING_FAMILY::SPRITE_MASKED:
		return "Sprite Masked";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_MASKED_SUBUV:
		return "Mesh Masked SubUV";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_TRANSLUCENT_SUBUV:
		return "Mesh Translucent SubUV";
	case EFFECT_GENERIC_AUTHORING_FAMILY::DECAL_ADDITIVE:
		return "Decal Additive";
	case EFFECT_GENERIC_AUTHORING_FAMILY::MESH_UNKNOWN:
		return "Mesh Unknown";
	case EFFECT_GENERIC_AUTHORING_FAMILY::END:
	default:
		return "Unknown";
	}
}

inline EFFECT_AUTHORING_FIDELITY Get_EffectAuthoringFidelity(
	const EFFECT_MATERIAL_EXECUTION_DESC& Execution)
{
	if (Execution.bEnabled)
		return EFFECT_AUTHORING_FIDELITY::EXACT;
	if (Execution.bAuthoringApproximate)
		return EFFECT_AUTHORING_FIDELITY::APPROXIMATE;
	if (Execution.bFailClosed)
		return EFFECT_AUTHORING_FIDELITY::INCOMPLETE;
	return EFFECT_AUTHORING_FIDELITY::RECONSTRUCTED;
}

inline const char_t* Get_EffectAuthoringFidelityLabel(
	const EFFECT_AUTHORING_FIDELITY eFidelity)
{
	switch (eFidelity)
	{
	case EFFECT_AUTHORING_FIDELITY::EXACT: return "EXACT";
	case EFFECT_AUTHORING_FIDELITY::RECONSTRUCTED: return "RECONSTRUCTED";
	case EFFECT_AUTHORING_FIDELITY::APPROXIMATE: return "APPROXIMATE";
	case EFFECT_AUTHORING_FIDELITY::INCOMPLETE: return "INCOMPLETE";
	default: return "UNKNOWN";
	}
}

enum class EFFECT_MODEL_CUE_ALPHA_MODE : uint8_t
{
	OPAQUE_SURFACE,
	MASKED_SURFACE,
	TRANSLUCENT_SURFACE,
	END
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
	float4_t vColorMultiply = { 1.f, 1.f, 1.f, 1.f };
	f32_t fOpacity = 1.f;
	EFFECT_MODEL_CUE_ALPHA_MODE eAlphaMode =
		EFFECT_MODEL_CUE_ALPHA_MODE::OPAQUE_SURFACE;
	bool_t bHoldLastFrame = false;
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
	uint32_t iLoadedFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	bool_t bSourceContract = false;
	std::string strEffectAssetId;
	std::string strDisplayName;
	EFFECT_PARTICLE_SYSTEM_DESC ParticleSystem;
	std::vector<EFFECT_MODEL_CUE_DESC> ModelCues;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

inline bool_t Try_ApplyEffectMasterGroupTranslation(
	EFFECT_DOCUMENT_DESC& Document,
	const EFFECT_ELEMENT_DESC& DraftMaster,
	std::string& strOutError)
{
	strOutError.clear();
	EFFECT_ELEMENT_DESC* pCommittedMaster = nullptr;
	for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.strElementId == DraftMaster.strElementId)
		{
			pCommittedMaster = &Element;
			break;
		}
	}
	if (nullptr == pCommittedMaster)
	{
		strOutError =
			"Move Group With This Mesh rejected: the selected Element is missing.";
		return false;
	}
	if (EFFECT_ELEMENT_KIND::MESH != pCommittedMaster->eKind ||
		EFFECT_ELEMENT_KIND::MESH != DraftMaster.eKind)
	{
		strOutError =
			"Move Group With This Mesh is available only for standalone Mesh elements.";
		return false;
	}
	const bool_t bManualGroup =
		pCommittedMaster->strGroupId.starts_with("manual.") ||
		pCommittedMaster->strGroupId.starts_with("authored.baseline.") ||
		pCommittedMaster->strGroupId.starts_with("authored.trackb.");
	if (!bManualGroup ||
		DraftMaster.strGroupId != pCommittedMaster->strGroupId)
	{
		strOutError =
			"Move Group With This Mesh requires one unchanged manual.*, authored.baseline.*, or authored.trackb.* group.";
		return false;
	}

	const auto SameFloat3 = [](const float3_t& Left, const float3_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
	};
	if (!SameFloat3(DraftMaster.Detail.Transform.vRotationDegrees,
		pCommittedMaster->Detail.Transform.vRotationDegrees) ||
		!SameFloat3(DraftMaster.Detail.Transform.vScale,
			pCommittedMaster->Detail.Transform.vScale))
	{
		strOutError =
			"Move Group With This Mesh is translation-only; revert master Rotation and Scale to committed values.";
		return false;
	}
	const auto IsZeroFloat3 = [](const float3_t& Value)
	{
		return 0.f == Value.x && 0.f == Value.y && 0.f == Value.z;
	};
	const auto SameTransform = [&SameFloat3](
		const EFFECT_TRANSFORM_DESC& Left,
		const EFFECT_TRANSFORM_DESC& Right)
	{
		return SameFloat3(Left.vPosition, Right.vPosition) &&
			SameFloat3(Left.vRotationDegrees, Right.vRotationDegrees) &&
			SameFloat3(Left.vRevolutionDegreesPerSecond,
				Right.vRevolutionDegreesPerSecond) &&
			SameFloat3(Left.vScale, Right.vScale) &&
			SameFloat3(Left.vVelocityPerSecond, Right.vVelocityPerSecond);
	};
	const auto SameAttachment = [&SameTransform](
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Left,
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Right)
	{
		return Left.bEnabled == Right.bEnabled &&
			Left.bFollow == Right.bFollow &&
			Left.strSourceAnchorSlotId == Right.strSourceAnchorSlotId &&
			Left.strRuntimeAnchorSlotId == Right.strRuntimeAnchorSlotId &&
			Left.strRuntimeBoneName == Right.strRuntimeBoneName &&
			Left.fSnapshotRootSourceBasisYawDegrees ==
				Right.fSnapshotRootSourceBasisYawDegrees &&
			SameTransform(Left.SocketLocalTransform, Right.SocketLocalTransform);
	};
	if (!SameAttachment(DraftMaster.ActionCueAttachment,
		pCommittedMaster->ActionCueAttachment))
	{
		strOutError =
			"Move Group With This Mesh rejects a changed master attachment space.";
		return false;
	}
	const auto HasDynamicTransform = [&IsZeroFloat3](
		const EFFECT_ELEMENT_DESC& Element)
	{
		const EFFECT_TRANSFORM_DESC& Transform = Element.Detail.Transform;
		const EFFECT_LINEAR_LERP_DESC& LinearLerp = Element.Detail.LinearLerp;
		return !IsZeroFloat3(Transform.vRevolutionDegreesPerSecond) ||
			!IsZeroFloat3(Transform.vVelocityPerSecond) ||
			LinearLerp.bPosition || LinearLerp.bRotation ||
			LinearLerp.bRevolution || LinearLerp.bScale ||
			LinearLerp.bVelocity;
	};
	if (HasDynamicTransform(DraftMaster))
	{
		strOutError =
			"Move Group With This Mesh rejects dynamic transform groups.";
		return false;
	}

	size_t iGroupMemberCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.strGroupId != pCommittedMaster->strGroupId)
			continue;
		++iGroupMemberCount;
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			strOutError =
				"Move Group With This Mesh rejects Particle and Screen Post group members.";
			return false;
		}
		if (HasDynamicTransform(Element))
		{
			strOutError =
				"Move Group With This Mesh rejects dynamic transform groups.";
			return false;
		}
		if (!SameAttachment(Element.ActionCueAttachment,
			pCommittedMaster->ActionCueAttachment))
		{
			strOutError =
				"Move Group With This Mesh requires every follower to use the same attachment parent space.";
			return false;
		}
	}
	if (iGroupMemberCount < 2u)
	{
		strOutError =
			"Move Group With This Mesh requires at least one follower in the same group.";
		return false;
	}

	const float3_t Delta = {
		DraftMaster.Detail.Transform.vPosition.x -
			pCommittedMaster->Detail.Transform.vPosition.x,
		DraftMaster.Detail.Transform.vPosition.y -
			pCommittedMaster->Detail.Transform.vPosition.y,
		DraftMaster.Detail.Transform.vPosition.z -
			pCommittedMaster->Detail.Transform.vPosition.z
	};
	if (!std::isfinite(Delta.x) || !std::isfinite(Delta.y) ||
		!std::isfinite(Delta.z))
	{
		strOutError =
			"Move Group With This Mesh rejected a non-finite translation delta.";
		return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.strGroupId != pCommittedMaster->strGroupId ||
			Element.strElementId == pCommittedMaster->strElementId)
			continue;
		const float3_t& Position = Element.Detail.Transform.vPosition;
		if (!std::isfinite(Position.x + Delta.x) ||
			!std::isfinite(Position.y + Delta.y) ||
			!std::isfinite(Position.z + Delta.z))
		{
			strOutError =
				"Move Group With This Mesh rejected a non-finite follower position.";
			return false;
		}
	}
	for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.strGroupId != pCommittedMaster->strGroupId ||
			Element.strElementId == pCommittedMaster->strElementId)
			continue;
		Element.Detail.Transform.vPosition.x += Delta.x;
		Element.Detail.Transform.vPosition.y += Delta.y;
		Element.Detail.Transform.vPosition.z += Delta.z;
	}
	return true;
}

inline void Apply_EffectElementDetailDraft(
	EFFECT_ELEMENT_DESC& Target,
	const EFFECT_ELEMENT_DESC& Draft)
{
	Target.strDisplayName = Draft.strDisplayName;
	Target.strGroupId = Draft.strGroupId;
	Target.strSourceNode = Draft.strSourceNode;
	Target.bVisible = Draft.bVisible;
	Target.ResourceBindings = Draft.ResourceBindings;
	Target.Detail = Draft.Detail;
	Target.Material = Draft.Material;
	Target.SourceRecipe = Draft.SourceRecipe;
	Target.AuthoringOverrides = Draft.AuthoringOverrides;
}

NS_END
