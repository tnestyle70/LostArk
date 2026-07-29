#pragma once

#include "Engine_Defines.h"

#include <filesystem>

NS_BEGIN(Engine)

struct MODEL_MATERIAL_DATA
{
	filesystem::path diffusePath;
	filesystem::path normalPath;
	filesystem::path specularPath;
	filesystem::path emissivePath;
	filesystem::path opacityPath;
	filesystem::path ormPath;
	filesystem::path metallicPath;
	filesystem::path roughnessPath;
	filesystem::path ambientOcclusionPath;
};

enum class MODEL_VERTEX_KIND : uint8_t
{
	STATIC,
	SKINNED,
};

struct MODEL_MESH_DATA
{
	string name;
	uint32_t materialIndex = {};
	MODEL_VERTEX_KIND vertexKind = { MODEL_VERTEX_KIND::STATIC };
	vector<VTXMESH> vertices;
	vector<VTXANIMMESH> skinnedVertices;
	vector<uint32_t> indices;
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
};

NS_END
