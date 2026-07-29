#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

struct W_MESH_BONE_DATA
{
	uint64_t nameHash = {};
	string name;
	float4x4_t inverseBind = {};
};

struct W_MESH_READ_RESULT
{
	bool_t skinned = { false };
	uint32_t materialCount = {};
	vector<MODEL_MESH_DATA> meshes;
	vector<W_MESH_BONE_DATA> bones;
};

class CWMeshReader final
{
public:
	bool_t Read(const filesystem::path& meshPath,
		W_MESH_READ_RESULT& outMesh,
		MODEL_DECODE_REPORT& outReport) const;
	bool_t ReadMemory(const uint8_t* pData,
		size_t dataSize,
		W_MESH_READ_RESULT& outMesh,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
