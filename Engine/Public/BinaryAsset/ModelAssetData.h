#pragma once

#include "Engine_Defines.h"

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
};

/* Per-placement atlas coordinates and decode scales. The texture pair belongs
   to its material variant; this payload must never be shared across placements. */
struct MODEL_BAKED_LIGHTING_INSTANCE
{
    float4_t scaleBias = { 0.f, 0.f, 0.f, 0.f };
    float4_t averageScale = { 0.f, 0.f, 0.f, 0.f }; // w: enabled
    float4_t directionalScale = { 0.f, 0.f, 0.f, 0.f };
};

struct MODEL_SURFACE_PARAMETERS
{
	MODEL_SURFACE_FAMILY family = MODEL_SURFACE_FAMILY::LEGACY;
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
    bool_t hasEnvironmentCube = false;
    float4_t environmentColor = { 1.f, 1.f, 1.f, 0.f };
    float2_t environmentRotation = { 0.f, 1.f };
};

struct MODEL_MATERIAL_OVERRIDE
{
	string materialName;
	MODEL_SURFACE_PARAMETERS surface;
	filesystem::path surfaceSpecularPath;
	filesystem::path reflectionPath;
	filesystem::path surfaceDiffusePath;
	filesystem::path surfaceNormalPath;
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;
    filesystem::path bakedAveragePath;
    filesystem::path bakedDirectionalPath;
    filesystem::path environmentCubePath;
    filesystem::path environmentBRDFPath;
};

struct MODEL_MATERIAL_DATA
{
	filesystem::path surfaceSpecularPath;
	string name;
	uint64_t nameHash = {};
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
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;
    filesystem::path bakedAveragePath;
    filesystem::path bakedDirectionalPath;
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
