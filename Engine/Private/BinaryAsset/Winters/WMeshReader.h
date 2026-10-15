#pragma once

// 정적/스킨 mesh·geometry metadata와 실패 진단의 CPU 소유 형식을 사용한다.
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

// WMSH에 저장된 본 순서와 inverse-bind다. WModelDecoder가 WSKL의 같은 index/nameHash와 대조한 뒤 결합한다.
struct W_MESH_BONE_DATA
{
	uint64_t nameHash = {};
	string name;
	float4x4_t inverseBind = {};
};

// 입력 바이트를 참조하지 않는 CPU 소유 결과다. materialCount는 사용한 최대 material index+1이며 실제 material 행 수와 다르다.
struct W_MESH_READ_RESULT
{
	bool_t skinned = {false};
	uint16_t fileVersionMinor = {};
	uint32_t materialCount = {};
	vector<MODEL_MESH_DATA> meshes;
	vector<W_MESH_BONE_DATA> bones;
	MODEL_GEOMETRY_METADATA_DATA geometryMetadata;
};

// 정적/skin 정점의 버전별 layout을 공통 MODEL_MESH_DATA로 복사한다. GPU buffer 생성과 모델 배치는 CModel의 책임이다.
class CWMeshReader final
{
public:
	// 파일 읽기 실패는 outMesh를 보존한다. ReadMemory로 진입하면 이전 출력을 비우고 부분적으로 채운다.
	bool_t Read(
		const filesystem::path& meshPath, W_MESH_READ_RESULT& outMesh, MODEL_DECODE_REPORT& outReport) const;

	// 입력을 호출 동안 차용한다. 명시적 false나 catch 뒤 부분 outMesh/report 통계가 남을 수 있으므로 성공한 결과만 소비한다.
	bool_t ReadMemory(
		const uint8_t* pData,
		size_t dataSize,
		W_MESH_READ_RESULT& outMesh,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
