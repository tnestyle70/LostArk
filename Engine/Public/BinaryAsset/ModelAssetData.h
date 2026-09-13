#pragma once

#include "Engine_Defines.h"
#include "Engine_VertexTypes.h"

#include <array>
#include <filesystem>

NS_BEGIN(Engine)

/* The source game's colour-region contract (WMA3): a mask texture's channels
select dye regions and these tints colour them. The diffuse textures of those
assets are mostly achromatic -- the colour lives here. All-ones with isEnabled
false is identity, and every pre-V3 material reads as that. */
struct MODEL_COLOR_TINT
{
	bool_t isEnabled = { false };
	/* Hair dyes differently from clothing: its diffuse *is* the region mask, so the tint
	replaces the sampled colour instead of multiplying it, and the strand shading comes from
	the mask's green channel. A material declares this by naming its own diffuse as the
	colour mask, which no clothing material ever does -- there the two are separate maps. */
	bool_t isHairMask = { false };
	float4_t vDiffuse = { 1.f, 1.f, 1.f, 1.f };
	float4_t vRegionA = { 1.f, 1.f, 1.f, 1.f };
	float4_t vRegionB = { 1.f, 1.f, 1.f, 1.f };
	float4_t vRegionC = { 1.f, 1.f, 1.f, 1.f };
};

/* Evaluated legacy surface families. The Client resolves source material
identities; the Engine owns only the immutable inputs used by these programs. */
enum class MODEL_SURFACE_FAMILY : uint32_t
{
	LEGACY = 0,
	SPECULAR_TEXTURE_REFLECTION = 1,
	DIFFUSE_SPECULAR_REFLECTION = 2,
	PBR_SEAMLESS_OPAQUE = 3,
	PBR_OPAQUE = 4,
	SOURCE_SPECULAR_OPAQUE = 5,
	SOURCE_CHARACTER = 6,
	SOURCE_OVERLAY_OPAQUE = 7,
	SOURCE_BG_OPAQUE_MASKED = 8,
	SOURCE_FOLIAGE_MASKED = 9,
	SOURCE_GRASS_MASKED = 10,
    SOURCE_SNOWICE_OPAQUE = 11,
    SOURCE_VERTEXBLEND_OPAQUE = 12,
    SOURCE_WET_OPAQUE = 13,
};

/* Per-placement atlas coordinates and decode scales. The texture pair belongs
   to its material variant; this payload must never be shared across placements. */
struct MODEL_BAKED_LIGHTING_INSTANCE
{
    float4_t scaleBias = { 0.f, 0.f, 0.f, 0.f };
    float4_t averageScale = { 0.f, 0.f, 0.f, 0.f }; // w: enabled
    float4_t directionalScale = { 0.f, 0.f, 0.f, 0.f };
    float4_t shadowScaleBias = { 0.f, 0.f, 0.f, 0.f };
};

/* Selected native character programs share one CModel/CMaterial route. The
   Client compiles named source parameters into the checked program's uniform
   layout; these arrays are private render inputs, never avatar identities. */
inline constexpr uint32_t SOURCE_CHARACTER_CONSTANT_COUNT = 64u;
inline constexpr uint32_t SOURCE_CHARACTER_TEXTURE_COUNT = 16u;
struct MODEL_SOURCE_CHARACTER_PARAMETERS
{
    uint32_t program = 0u;
    uint32_t baseTextureMask = 0u;
    uint32_t lightTextureMask = 0u;
    uint32_t requiredExtraUVMask = 0u; // bit 0: TEXCOORD1, bit 1: TEXCOORD2
    std::array<float4_t, SOURCE_CHARACTER_CONSTANT_COUNT> baseConstants{};
    std::array<float4_t, SOURCE_CHARACTER_CONSTANT_COUNT> lightConstants{};
};

struct MODEL_SOURCE_CHARACTER_TEXTURE
{
    filesystem::path path;
    bool_t srgb = false;
};

enum class MODEL_SURFACE_RENDER_MODE : uint32_t
{
    INHERIT, DEFERRED, TRANSLUCENT, BACKGROUND, ADDITIVE, WATER
};
enum class MODEL_SURFACE_CULL_MODE : uint32_t
{
    INHERIT, CULL_BACK, CULL_FRONT, TWO_SIDED
};

struct MODEL_SOURCE_SPECIAL_PARAMETERS
{
    uint32_t flags = 0u; // ice: detail/spec texture; vertex blend: G/B layers
    f32_t normalTiling = 1.f;
    float4_t iceCoreColor = {};
    float4_t iceOuterColor = {};
    float4_t iceBlend = { 1.f, .5f, .5f, 0.f }; // mask UV, blend, sharpness, emission
    f32_t iceBumpOffset = .3f;
    float4_t wetParameters = { 1.f, .5f, 1.f, 1.f }; // normal, sharpness, opacity, specular
    f32_t wetSpecularPower = 50.f;
    std::array<float4_t, 4> blendDiffuse{}; // RGB tint, brightness
    std::array<float4_t, 4> blendSpecular{}; // RGB tint, intensity
    std::array<float4_t, 4> blendLayers{}; // UV XY, normal strength, specular power
    f32_t blendSharpness = .5f;
    bool_t maskSRGB = false;
    bool_t blendGSRGB = true;
    bool_t blendBSRGB = true;
};

struct MODEL_SURFACE_PARAMETERS
{
    MODEL_SURFACE_RENDER_MODE renderMode = MODEL_SURFACE_RENDER_MODE::INHERIT;
    MODEL_SURFACE_CULL_MODE cullMode = MODEL_SURFACE_CULL_MODE::INHERIT;
	MODEL_SURFACE_FAMILY family = MODEL_SURFACE_FAMILY::LEGACY;
	MODEL_SOURCE_CHARACTER_PARAMETERS sourceCharacter;
    // Native BG static-switch branches; selected source textures only are required.
    // normal=1, bump=2, specular=4, specular texture=8, reflection=16,
    // world reflection=32, masked=64, vertex-alpha normal=128, simple=256, seamless=512.
    // Native texture address: diffuse mirror V=1024, normal mirror U=2048, normal mirror V=4096.
    // linear reflection=8192, fixed base normal UV=16384, detail normal=32768.
    uint32_t sourceBgFlags = 0u;
    float4_t sourceBgBump = { 0.f, 0.f, 1.f, 0.f }; // offset, intensity, brightness, reserved
    float4_t sourceBgUV = { 0.f, 1.f, 0.f, 0.f }; // sin, cos, fixed move X/Y
    uint32_t sourceBgFlicker = 0u; // 0 steady, 1 nested, 2 linear
    float2_t sourceBgSubspecular = { 0.f, 60.f }; // independent view lobe: intensity, power
    float4_t sourceBgRimlight = { 0.f, 0.f, 0.f, 3.f }; // intensity-scaled RGB, power
    f32_t sourceBgSpecularSaturation = 1.f;
    float2_t sourceBgPanning = { 0.f, 0.f };

    // Native foliage: normal=1, saturation=2, specular=4, specular texture=8,
    // transmission=16, emissive=32, emissive flicker=64.
    MODEL_SOURCE_SPECIAL_PARAMETERS sourceSpecial;
    uint32_t sourceFoliageFlags = 0u;
    float4_t sourceFoliageTransmission = { 0.f, 0.f, 0.f, 1.f };
    bool_t sourceFoliageMaskSRGB = false;

    // Source opaque overlay uses vertex R coverage and vertex A normal strength.
    float4_t overlayColor = { 1.f, 1.f, 1.f, 1.f };
    f32_t overlayTiling = 1.f;
    f32_t overlayNormalIntensity = 1.f;
    f32_t overlaySharpness = 1.f;
    f32_t overlayBrightness = 1.f;
    f32_t overlaySaturation = 1.f;
    f32_t overlaySpecularIntensity = 1.f;
    bool_t overlaySRGB = true;
    bool_t overlaySeparateSpecular = false;
    // normal, overlay normal, vertex paint, explicit direction, inverse height,
    // detail normal, mask, fixed base-normal UV, specular.
    uint32_t sourceOverlayFlags = 263u;
    float4_t sourceOverlayDirection = { 0.f, 1.f, 0.f, 0.f }; // runtime world XYZ, amount
	f32_t diffuseBrightness = 1.f;
	f32_t normalIntensity = 1.f;
	f32_t specularIntensity = 1.f;
	f32_t specularPower = 50.f;
	f32_t reflectionIntensity = 0.f;
	f32_t reflectionContrast = 0.5f;
	f32_t reflectionTiling = 1.f;
	f32_t diffuseSaturation = 1.f;
	float4_t diffuseColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t specularColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t reflectionColor = { 1.f, 1.f, 1.f, 1.f };
	bool_t diffuseSRGB = true;
	bool_t specularSRGB = false;
	bool_t reflectionSRGB = false;
	bool_t ormSRGB = false;
	bool_t castsShadow = true;
	bool_t hasEmissive = false;
	float4_t emissiveColor = { 1.f, 1.f, 1.f, 1.f };
	f32_t emissiveIntensity = 0.f;
	float2_t emissiveUVTiling = { 1.f, 1.f };
	f32_t emissiveFlickerMinimum = 0.f;
	f32_t emissiveFlickerSpeed = 0.f;
	f32_t emissivePhaseOffset = 0.f;
	bool_t emissiveSRGB = true;
	float2_t uvTiling = { 1.f, 1.f };
	bool_t uvFixedNormal = false;
	bool_t useWorldReflection = false;
	f32_t detailNormalIntensity = 0.f;
	f32_t detailNormalTiling = 1.f;
	f32_t metallicIntensity = 1.f;
	f32_t metallicPower = 1.f;
	f32_t roughnessIntensity = 1.f;
	f32_t roughnessPower = 1.f;
	f32_t aoIntensity = 1.f;
	f32_t aoPower = 1.f;
	f32_t specularPBRIntensity = 0.5f;
	f32_t nonmetallicBrightness = 1.f;
	f32_t metallicBrightness = 1.f;
	f32_t minimumRoughness = 0.04f;
	float2_t reflectionOriginOffset = { 0.f, 0.f };
	f32_t vertexAlpha = 1.f;
    bool_t hasBakedLighting = false;
    bool_t bakedLightingSRGB = false;
    bool_t hasStaticShadow = false;
    uint32_t staticShadowChannel = 0u;
    float4_t staticShadowTransfer = { 0.f, 1.f, 1.f, 0.f }; // bias, scale, exponent, reserved
    bool_t hasEnvironmentCube = false;
    float4_t environmentColor = { 1.f, 1.f, 1.f, 0.f };
    float2_t environmentRotation = { 0.f, 1.f };
};

struct MODEL_MATERIAL_OVERRIDE
{
	string materialName;
	bool_t hasDiffuseAddressU = false;
	bool_t diffuseMirrorU = false;
	MODEL_SURFACE_PARAMETERS surface;
	filesystem::path surfaceSpecularPath;
	filesystem::path reflectionPath;
	filesystem::path surfaceDiffusePath;
	filesystem::path surfaceNormalPath;
	filesystem::path overlayDiffusePath;
	filesystem::path overlayNormalPath;
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;
    filesystem::path sourceFoliageMaskPath;
    filesystem::path sourceSpecialMaskPath;
    filesystem::path sourceBlendDiffuseGPath;
    filesystem::path sourceBlendDiffuseBPath;
    filesystem::path sourceBlendNormalGPath;
    filesystem::path sourceBlendNormalBPath;
	filesystem::path surfaceEmissivePath;
	std::array<MODEL_SOURCE_CHARACTER_TEXTURE, SOURCE_CHARACTER_TEXTURE_COUNT> sourceCharacterTextures;
    filesystem::path bakedAveragePath;
    filesystem::path bakedDirectionalPath;
    filesystem::path staticShadowPath;
    filesystem::path environmentCubePath;
    filesystem::path environmentBRDFPath;
};

struct MODEL_MATERIAL_DATA
{
	filesystem::path surfaceSpecularPath;
	string name;
	uint64_t nameHash = {};
	bool_t diffuseMirrorU = false;
	filesystem::path diffusePath;
	filesystem::path normalPath;
	filesystem::path specularPath;
	filesystem::path emissivePath;
	filesystem::path opacityPath;
	filesystem::path ormPath;
	filesystem::path metallicPath;
	filesystem::path roughnessPath;
	filesystem::path ambientOcclusionPath;
	filesystem::path colorMaskPath;
	MODEL_COLOR_TINT colorTint;
	MODEL_SURFACE_PARAMETERS surface;
	filesystem::path reflectionPath;
	filesystem::path surfaceDiffusePath;
	filesystem::path surfaceNormalPath;
	filesystem::path overlayDiffusePath;
	filesystem::path overlayNormalPath;
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;
    filesystem::path sourceFoliageMaskPath;
    filesystem::path sourceSpecialMaskPath;
    filesystem::path sourceBlendDiffuseGPath;
    filesystem::path sourceBlendDiffuseBPath;
    filesystem::path sourceBlendNormalGPath;
    filesystem::path sourceBlendNormalBPath;
	filesystem::path surfaceEmissivePath;
	std::array<MODEL_SOURCE_CHARACTER_TEXTURE, SOURCE_CHARACTER_TEXTURE_COUNT> sourceCharacterTextures;
    filesystem::path bakedAveragePath;
    filesystem::path bakedDirectionalPath;
    filesystem::path staticShadowPath;
    filesystem::path environmentCubePath;
    filesystem::path environmentBRDFPath;
};

enum class MODEL_VERTEX_KIND : uint8_t
{
	STATIC,
	SKINNED,
};

enum MODEL_GEOMETRY_EVIDENCE_FLAG : uint32_t
{
	MODEL_GEOMETRY_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF = 1u << 0,
	MODEL_GEOMETRY_COLOR0_PRESERVED_FROM_GLTF = 1u << 1,
	MODEL_GEOMETRY_BOUNDS_WMODEL_SPACE = 1u << 2,
	MODEL_GEOMETRY_SOURCE_GLTF_SHA256 = 1u << 3,
	MODEL_GEOMETRY_SOURCE_BUFFER_SET_SHA256 = 1u << 4,
	MODEL_GEOMETRY_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 = 1u << 5,
	MODEL_GEOMETRY_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED = 1u << 6,
	MODEL_GEOMETRY_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 = 1u << 7,
	MODEL_GEOMETRY_TOOL_SHA256 = 1u << 8,
	MODEL_GEOMETRY_SOURCE_EXPORT_RECEIPT_SHA256 = 1u << 9,
	MODEL_GEOMETRY_LEGACY_COOK_RECEIPT_SHA256 = 1u << 10,
	MODEL_GEOMETRY_CLEAN_SOURCE_EXPORT = 1u << 11,
	MODEL_GEOMETRY_UPK_TO_GLTF_EXACT = 1u << 12,
	MODEL_GEOMETRY_PIVOT_EXACT = 1u << 13,
	MODEL_GEOMETRY_TEXCOORD1_PRESERVED_FROM_GLTF = 1u << 14,
	MODEL_GEOMETRY_TANGENT_HANDEDNESS_PROJECT_RECONSTRUCTED = 1u << 15,
	MODEL_GEOMETRY_NATIVE_PARALLEL_BASIS_PRESERVED = 1u << 16,
	MODEL_GEOMETRY_TEXCOORD2_PRESERVED_FROM_GLTF = 1u << 17,
};

struct MODEL_MESH_BOUNDS_DATA
{
	bool_t present = { false };
	float3_t minimum = {};
	float3_t maximum = {};
	float3_t center = {};
	f32_t radius = {};
};

struct MODEL_GEOMETRY_METADATA_DATA
{
	bool_t present = { false };
	uint16_t versionMajor = {};
	uint16_t versionMinor = {};
	uint32_t channelMask = {};
	uint32_t evidenceFlags = {};
	uint32_t payloadSize = {};
	f32_t sourceToWModelScale = { 1.f };
	f32_t geometryPreScale = { 1.f };
	array<uint8_t, 32> payloadSha256 = {};
	array<uint8_t, 32> sourceGltfSha256 = {};
	array<uint8_t, 32> sourceBufferSetSha256 = {};
	array<uint8_t, 32> sourcePackageObservedUnboundSha256 = {};
	array<uint8_t, 32> sourceObjectPathHash = {};
	array<uint8_t, 32> legacyConverterObservedUnboundSha256 = {};
	array<uint8_t, 32> geometryToolSha256 = {};
	array<uint8_t, 32> sourceExportReceiptSha256 = {};
	array<uint8_t, 32> legacyCookReceiptSha256 = {};
	array<uint8_t, 32> metadataSha256 = {};
	array<uint8_t, 32> metadataIdentitySha256 = {};
};

struct MODEL_MESH_DATA
{
	string name;
	uint32_t materialIndex = {};
	MODEL_VERTEX_KIND vertexKind = { MODEL_VERTEX_KIND::STATIC };
	vector<VTXMESH> vertices;
	vector<VTXANIMMESH> skinnedVertices;
	vector<uint32_t> indices;
	vector<f32_t> tangentHandedness;
	vector<uint32_t> color0Rgba8;
	bool_t hasColor0 = { false };
	bool_t hasTexcoord1 = { false };
	bool_t hasTexcoord2 = { false };
	MODEL_MESH_BOUNDS_DATA embeddedBounds;
};

struct MODEL_BONE_DATA
{
	uint64_t nameHash = {};
	string name;
	int32_t parentIndex = { -1 };
	float4x4_t restLocal = {};
	float4x4_t inverseBind = {};
};

struct MODEL_SOCKET_DATA
{
	uint64_t nameHash = {};
	string name;
	int32_t parentBoneIndex = { -1 };
	float4x4_t localOffset = {};
};

struct MODEL_SKELETON_DATA
{
	uint64_t skeletonHash = {};
	float4x4_t globalInverseRoot = {};
	vector<MODEL_BONE_DATA> bones;
	vector<MODEL_SOCKET_DATA> sockets;
};

struct MODEL_VECTOR_KEY_DATA
{
	f32_t timeTicks = {};
	float3_t value = {};
};

struct MODEL_QUAT_KEY_DATA
{
	f32_t timeTicks = {};
	float4_t value = { 0.f, 0.f, 0.f, 1.f };
};

struct MODEL_ANIMATION_CHANNEL_DATA
{
	uint64_t boneNameHash = {};
	int32_t resolvedBoneIndex = { -1 };
	vector<MODEL_VECTOR_KEY_DATA> positionKeys;
	vector<MODEL_QUAT_KEY_DATA> rotationKeys;
	vector<MODEL_VECTOR_KEY_DATA> scaleKeys;
};

struct MODEL_ANIMATION_EVENT_DATA
{
	f32_t timeTicks = {};
	uint16_t type = {};
	uint32_t skillId = {};
	uint32_t paramU32 = {};
	f32_t paramF32 = {};
	uint64_t stringHash = {};
};

struct MODEL_ANIMATION_CATALOG_ENTRY
{
	string name;
	f32_t durationTicks = {};
	f32_t ticksPerSecond = {};
};

struct MODEL_ANIMATION_DATA
{
	string name;
	uint64_t skeletonHash = {};
	f32_t durationTicks = {};
	f32_t ticksPerSecond = {};
	bool_t defaultLoop = { true };
	vector<MODEL_ANIMATION_CHANNEL_DATA> channels;
	vector<MODEL_ANIMATION_EVENT_DATA> events;
};

struct MODEL_ASSET_DATA
{
	vector<MODEL_MESH_DATA> meshes;
	vector<MODEL_MATERIAL_DATA> materials;
	MODEL_GEOMETRY_METADATA_DATA geometryMetadata;
	bool_t hasSkeleton = { false };
	MODEL_SKELETON_DATA skeleton;
	vector<MODEL_ANIMATION_DATA> animations;
};

struct MODEL_ASSET_LOAD_DESC
{
	filesystem::path assetRoot;
	filesystem::path meshPath;
	filesystem::path materialPath;
	filesystem::path skeletonPath;
	vector<filesystem::path> animationPaths;
	filesystem::path fallbackDiffusePath;
	string defaultAnimationName;
	vector<MODEL_MATERIAL_OVERRIDE> materialOverrides;
};

struct MODEL_DECODE_REPORT
{
	bool_t succeeded = { false };
	bool_t usedSkinnedBindPose = { false };
	filesystem::path meshPath;
	string decoderName;
	string error;
	uint32_t meshCount = {};
	uint32_t materialCount = {};
	uint32_t boneCount = {};
	uint32_t animationCount = {};
	uint64_t vertexCount = {};
	uint64_t indexCount = {};
	uint64_t animationKeyCount = {};
	bool_t hasGeometryMetadata = { false };
	uint32_t geometryEvidenceFlags = {};
};

NS_END
