#pragma once

#include <cstdint>

namespace Engine::WintersFormat
{
	constexpr char WINTERS_MAGIC[4] = { 'W', 'I', 'N', 'T' };
	constexpr char WMESH_MAGIC[4] = { 'W', 'M', 'S', 'H' };
	constexpr char WMAT_MAGIC[4] = { 'W', 'M', 'A', 'T' };
	constexpr char WMAT_V2_MAGIC[4] = { 'W', 'M', 'A', '2' };
	constexpr char WMAT_V3_MAGIC[4] = { 'W', 'M', 'A', '3' };
	constexpr char WSKEL_MAGIC[4] = { 'W', 'S', 'K', 'L' };
	constexpr char WANIM_MAGIC[4] = { 'W', 'A', 'N', 'M' };
	constexpr char WMODEL_MAGIC[4] = { 'W', 'M', 'O', 'D' };
	constexpr char WGEOMETRY_MAGIC[4] = { 'W', 'G', 'E', 'O' };

	constexpr uint16_t WINT_VERSION_MAJOR = 1;
	constexpr uint16_t WINT_LEGACY_VERSION_MINOR = 0;
	constexpr uint16_t WINT_GEOMETRY_VERSION_MINOR = 1;
	constexpr uint32_t VF_POSITION = 1u << 0;
	constexpr uint32_t VF_NORMAL = 1u << 1;
	constexpr uint32_t VF_TEXCOORD0 = 1u << 2;
	constexpr uint32_t VF_TANGENT = 1u << 3;
	constexpr uint32_t VF_BONE_WEIGHT = 1u << 4;
	constexpr uint32_t VF_TANGENT_HANDEDNESS = 1u << 5;
	constexpr uint32_t VF_COLOR0 = 1u << 6;
	constexpr uint32_t VF_STATIC_BASE =
		VF_POSITION | VF_NORMAL | VF_TEXCOORD0 | VF_TANGENT;
	constexpr uint32_t STRIDE_STATIC = 48;
	constexpr uint32_t STRIDE_STATIC_COLOR0 = 52;
	constexpr uint32_t STRIDE_SKINNED = 76;
	constexpr uint32_t SHA256_SIZE = 32;
	constexpr uint32_t MAX_SUBMESHES = 2048;
	constexpr uint32_t MAX_BONES = 512;
	constexpr uint32_t MAX_SOCKETS = 256;
	constexpr uint32_t MAX_VERTICES = 10'000'000;
	constexpr uint32_t MAX_MATERIALS = 4096;
	constexpr uint32_t MAX_ANIMATION_CHANNELS = 1024;
	constexpr uint32_t MAX_ANIMATION_KEYS = 1'000'000;
	constexpr uint32_t MAX_ANIMATION_EVENTS = 100'000;
	constexpr uint32_t MAX_MODEL_SECTIONS = 4096;

	enum class MODEL_SECTION_TYPE : uint32_t
	{
		MESH = 1,
		MATERIAL = 2,
		SKELETON = 3,
		ANIMATION = 4,
	};

#pragma pack(push, 1)
	struct FILE_HEADER
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t flags;
		uint32_t contentSize;
	};

	struct MODEL_META_HEADER
	{
		char magic[4];
		uint32_t sectionCount;
		uint32_t animationCount;
		uint32_t flags;
		uint32_t reserved[4];
	};

	struct MODEL_SECTION_DESC
	{
		uint32_t type;
		uint32_t index;
		uint64_t offset;
		uint64_t size;
		char name[40];
	};

	struct MESH_META_HEADER
	{
		char magic[4];
		uint32_t submeshCount;
		uint32_t boneCount;
		uint32_t vertexFormatFlags;
		uint32_t vertexStride;
		uint32_t totalVertexCount;
		uint32_t totalIndexCount;
		uint32_t indexStride;
		uint8_t hasBounding;
		uint8_t reserved[3];
	};

	struct SUBMESH_DESC
	{
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[20];
	};

	struct MESH_BONE_ENTRY
	{
		uint64_t nameHash;
		char name[32];
		int32_t parentIndex;
		float offsetMatrix[16];
		uint32_t channelFlag;
		uint8_t reserved[16];
	};

	struct MESH_BOUNDS_V1
	{
		float minimum[3];
		float maximum[3];
		float center[3];
		float radius;
	};

	enum MESH_GEOMETRY_EVIDENCE_FLAG : uint32_t
	{
		MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF = 1u << 0,
		MGEF_COLOR0_PRESERVED_FROM_GLTF = 1u << 1,
		MGEF_BOUNDS_WMODEL_SPACE = 1u << 2,
		MGEF_SOURCE_GLTF_SHA256 = 1u << 3,
		MGEF_SOURCE_BUFFER_SET_SHA256 = 1u << 4,
		MGEF_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 = 1u << 5,
		MGEF_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED = 1u << 6,
		MGEF_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 = 1u << 7,
		MGEF_GEOMETRY_TOOL_SHA256 = 1u << 8,
		MGEF_SOURCE_EXPORT_RECEIPT_SHA256 = 1u << 9,
		MGEF_LEGACY_COOK_RECEIPT_SHA256 = 1u << 10,
		MGEF_CLEAN_SOURCE_EXPORT = 1u << 11,
		MGEF_UPK_TO_GLTF_EXACT = 1u << 12,
		MGEF_PIVOT_EXACT = 1u << 13,
	};

	constexpr uint32_t MGEF_REQUIRED_PAYLOAD =
		MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF |
		MGEF_BOUNDS_WMODEL_SPACE |
		MGEF_SOURCE_GLTF_SHA256 |
		MGEF_SOURCE_BUFFER_SET_SHA256 |
		MGEF_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 |
		MGEF_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED |
		MGEF_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 |
		MGEF_GEOMETRY_TOOL_SHA256 |
		MGEF_SOURCE_EXPORT_RECEIPT_SHA256 |
		MGEF_LEGACY_COOK_RECEIPT_SHA256;
	constexpr uint32_t MGEF_PRODUCT_PROVENANCE =
		MGEF_CLEAN_SOURCE_EXPORT |
		MGEF_UPK_TO_GLTF_EXACT |
		MGEF_PIVOT_EXACT;
	constexpr uint32_t MGEF_KNOWN =
		MGEF_REQUIRED_PAYLOAD |
		MGEF_COLOR0_PRESERVED_FROM_GLTF |
		MGEF_PRODUCT_PROVENANCE;

	struct MESH_GEOMETRY_METADATA_V1
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t byteSize;
		uint32_t evidenceFlags;
		uint32_t payloadSize;
		float sourceToWModelScale;
		float geometryPreScale;
		uint8_t payloadSha256[SHA256_SIZE];
		uint8_t sourceGltfSha256[SHA256_SIZE];
		uint8_t sourceBufferSetSha256[SHA256_SIZE];
		uint8_t sourcePackageObservedUnboundSha256[SHA256_SIZE];
		uint8_t sourceObjectPathHash[SHA256_SIZE];
		uint8_t legacyConverterObservedUnboundSha256[SHA256_SIZE];
		uint8_t geometryToolSha256[SHA256_SIZE];
		uint8_t sourceExportReceiptSha256[SHA256_SIZE];
		uint8_t legacyCookReceiptSha256[SHA256_SIZE];
		uint8_t metadataSha256[SHA256_SIZE];
	};

	struct MATERIAL_META_HEADER
	{
		char magic[4];
		uint32_t materialCount;
	};

	struct MATERIAL_ENTRY
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t diffusePath[260];
	};

	struct MATERIAL_ENTRY_V2
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t baseColorPath[260];
		wchar_t normalPath[260];
		wchar_t specularPath[260];
		wchar_t emissivePath[260];
		wchar_t opacityPath[260];
		wchar_t ormPath[260];
		wchar_t metallicPath[260];
		wchar_t roughnessPath[260];
		wchar_t ambientOcclusionPath[260];
	};

	/* V2 plus the source game's colour-region contract: a _cm mask texture
	whose channels select dye regions, three per-region tint colours and one
	whole-material multiplier. Identity is all-ones tints with an empty mask
	path; the diffuse textures of these assets are mostly achromatic and the
	colour lives here. */
	struct MATERIAL_ENTRY_V3
	{
		uint32_t materialIndex;
		uint64_t materialHash;
		char name[64];
		wchar_t baseColorPath[260];
		wchar_t normalPath[260];
		wchar_t specularPath[260];
		wchar_t emissivePath[260];
		wchar_t opacityPath[260];
		wchar_t ormPath[260];
		wchar_t metallicPath[260];
		wchar_t roughnessPath[260];
		wchar_t ambientOcclusionPath[260];
		wchar_t colorMaskPath[260];
		float diffuseTint[4];
		float regionTintA[4];
		float regionTintB[4];
		float regionTintC[4];
	};

	struct SKELETON_META_HEADER
	{
		char magic[4];
		uint32_t boneCount;
		uint32_t socketCount;
		uint32_t reserved[5];
	};

	struct SKELETON_BONE_NODE
	{
		uint64_t nameHash;
		char name[64];
		int32_t parentIndex;
		float restTransform[16];
		uint32_t childCount;
		uint32_t firstChildIndex;
		uint32_t reserved[27];
	};

	struct GLOBAL_ROOT_MATRIX
	{
		float globalInverseRoot[16];
		uint32_t reserved[16];
	};

	struct SOCKET_ENTRY
	{
		char name[32];
		uint64_t nameHash;
		int32_t parentBoneIndex;
		float localOffset[16];
		uint32_t reserved[5];
	};

	struct ANIMATION_META_HEADER
	{
		char magic[4];
		uint32_t channelCount;
		float durationTicks;
		float ticksPerSecond;
		uint32_t totalKeyCount;
		uint32_t eventCount;
		uint8_t isLoop;
		uint8_t reserved[7];
	};

	struct ANIMATION_CHANNEL
	{
		uint64_t boneNameHash;
		uint32_t positionKeyCount;
		uint32_t positionOffset;
		uint32_t rotationKeyCount;
		uint32_t rotationOffset;
		uint32_t scaleKeyCount;
		uint32_t scaleOffset;
		int32_t cachedBoneIndex;
		uint32_t reserved;
	};

	struct VECTOR_KEY
	{
		float timeTicks;
		float x;
		float y;
		float z;
	};

	struct QUATERNION_KEY
	{
		float timeTicks;
		float x;
		float y;
		float z;
		float w;
	};

	struct ANIMATION_EVENT
	{
		float timeTicks;
		uint16_t type;
		uint16_t reserved0;
		uint32_t skillId;
		uint32_t paramU32;
		float paramF32;
		uint64_t stringHash;
		uint32_t reserved1;
	};

	struct ANIMATION_TRAILER
	{
		uint64_t skeletonHash;
	};
#pragma pack(pop)

	static_assert(sizeof(FILE_HEADER) == 16);
	static_assert(sizeof(MODEL_META_HEADER) == 32);
	static_assert(sizeof(MODEL_SECTION_DESC) == 64);
	static_assert(sizeof(MESH_META_HEADER) == 36);
	static_assert(sizeof(SUBMESH_DESC) == 48);
	static_assert(sizeof(MESH_BONE_ENTRY) == 128);
	static_assert(sizeof(MESH_BOUNDS_V1) == 40);
	static_assert(sizeof(MESH_GEOMETRY_METADATA_V1) == 348);
	static_assert(sizeof(MATERIAL_ENTRY) == 596);
	static_assert(sizeof(MATERIAL_ENTRY_V2) == 4756);
	static_assert(sizeof(MATERIAL_ENTRY_V3) == 5340);
	static_assert(sizeof(SKELETON_META_HEADER) == 32);
	static_assert(sizeof(SKELETON_BONE_NODE) == 256);
	static_assert(sizeof(GLOBAL_ROOT_MATRIX) == 128);
	static_assert(sizeof(SOCKET_ENTRY) == 128);
	static_assert(sizeof(ANIMATION_META_HEADER) == 32);
	static_assert(sizeof(ANIMATION_CHANNEL) == 40);
	static_assert(sizeof(VECTOR_KEY) == 16);
	static_assert(sizeof(QUATERNION_KEY) == 20);
	static_assert(sizeof(ANIMATION_EVENT) == 32);
	static_assert(sizeof(ANIMATION_TRAILER) == 8);
}
