#pragma once

#include <cstdint>

namespace Engine::WintersFormat
{
	constexpr char WINTERS_MAGIC[4] = { 'W', 'I', 'N', 'T' };
	constexpr char WMESH_MAGIC[4] = { 'W', 'M', 'S', 'H' };
	constexpr char WMAT_MAGIC[4] = { 'W', 'M', 'A', 'T' };
	constexpr char WSKEL_MAGIC[4] = { 'W', 'S', 'K', 'L' };
	constexpr char WANIM_MAGIC[4] = { 'W', 'A', 'N', 'M' };

	constexpr uint32_t VF_BONE_WEIGHT = 1u << 4;
	constexpr uint32_t STRIDE_STATIC = 48;
	constexpr uint32_t STRIDE_SKINNED = 76;
	constexpr uint32_t MAX_SUBMESHES = 2048;
	constexpr uint32_t MAX_BONES = 512;
	constexpr uint32_t MAX_SOCKETS = 256;
	constexpr uint32_t MAX_VERTICES = 10'000'000;
	constexpr uint32_t MAX_MATERIALS = 4096;
	constexpr uint32_t MAX_ANIMATION_CHANNELS = 1024;
	constexpr uint32_t MAX_ANIMATION_KEYS = 1'000'000;
	constexpr uint32_t MAX_ANIMATION_EVENTS = 100'000;

#pragma pack(push, 1)
	struct FILE_HEADER
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t flags;
		uint32_t contentSize;
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
	static_assert(sizeof(MESH_META_HEADER) == 36);
	static_assert(sizeof(SUBMESH_DESC) == 48);
	static_assert(sizeof(MESH_BONE_ENTRY) == 128);
	static_assert(sizeof(MATERIAL_ENTRY) == 596);
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
