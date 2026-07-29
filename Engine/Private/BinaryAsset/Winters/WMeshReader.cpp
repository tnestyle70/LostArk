#include "WMeshReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <cmath>
#include <cstring>
#include <limits>

using namespace Engine::WintersFormat;

namespace
{
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	bool_t CheckedByteCount(uint64_t count, uint64_t stride, size_t limit, size_t& outBytes)
	{
		if (0 != count && stride > (numeric_limits<uint64_t>::max)() / count)
			return false;

		const uint64_t bytes = count * stride;
		if (bytes > limit || bytes > SIZE_MAX)
			return false;

		outBytes = static_cast<size_t>(bytes);
		return true;
	}

	string ReadFixedName(const char* pName, size_t capacity)
	{
		size_t length = {};
		while (length < capacity && '\0' != pName[length])
			++length;
		return string(pName, length);
	}

	bool_t IsFinite3(const float3_t& value)
	{
		return isfinite(value.x) && isfinite(value.y) && isfinite(value.z);
	}

	vector_t SafeNormalize3(vector_t value, vector_t fallback)
	{
		const f32_t lengthSq = XMVectorGetX(XMVector3LengthSq(value));
		return isfinite(lengthSq) && lengthSq > 1e-12f
			? XMVector3Normalize(value)
			: fallback;
	}

	bool_t MakeStaticVertex(const uint8_t* pSource, VTXMESH& outVertex)
	{
		memcpy(&outVertex.vPosition, pSource + 0, sizeof(float3_t));
		memcpy(&outVertex.vNormal, pSource + 12, sizeof(float3_t));
		memcpy(&outVertex.vTexcoord, pSource + 24, sizeof(float2_t));
		memcpy(&outVertex.vTangent, pSource + 32, sizeof(float3_t));

		f32_t handedness = 1.f;
		memcpy(&handedness, pSource + 44, sizeof(f32_t));
		if (!IsFinite3(outVertex.vPosition) || !IsFinite3(outVertex.vNormal) ||
			!IsFinite3(outVertex.vTangent) || !isfinite(outVertex.vTexcoord.x) ||
			!isfinite(outVertex.vTexcoord.y) || !isfinite(handedness))
			return false;

		const vector_t normal = SafeNormalize3(XMLoadFloat3(&outVertex.vNormal), XMVectorSet(0.f, 1.f, 0.f, 0.f));
		const vector_t tangent = SafeNormalize3(XMLoadFloat3(&outVertex.vTangent), XMVectorSet(1.f, 0.f, 0.f, 0.f));
		const vector_t binormal = XMVectorScale(
			SafeNormalize3(XMVector3Cross(normal, tangent), XMVectorSet(0.f, 0.f, 1.f, 0.f)),
			handedness);

		XMStoreFloat3(&outVertex.vNormal, normal);
		XMStoreFloat3(&outVertex.vTangent, tangent);
		XMStoreFloat3(&outVertex.vBinormal, binormal);
		return true;
	}

	bool_t MakeSkinnedVertex(const uint8_t* pSource, uint32_t boneCount, VTXANIMMESH& outVertex)
	{
		memcpy(&outVertex.vPosition, pSource + 0, sizeof(float3_t));
		memcpy(&outVertex.vNormal, pSource + 12, sizeof(float3_t));
		memcpy(&outVertex.vTexcoord, pSource + 24, sizeof(float2_t));
		memcpy(&outVertex.vTangent, pSource + 32, sizeof(float3_t));
		memcpy(&outVertex.vBlendIndices, pSource + 44, sizeof(XMUINT4));
		memcpy(&outVertex.vBlendWeights, pSource + 60, sizeof(float4_t));

		if (!IsFinite3(outVertex.vPosition) || !IsFinite3(outVertex.vNormal) ||
			!IsFinite3(outVertex.vTangent) || !isfinite(outVertex.vTexcoord.x) ||
			!isfinite(outVertex.vTexcoord.y))
			return false;

		const uint32_t indices[4] = {
			outVertex.vBlendIndices.x,
			outVertex.vBlendIndices.y,
			outVertex.vBlendIndices.z,
			outVertex.vBlendIndices.w,
		};
		f32_t* pWeights = &outVertex.vBlendWeights.x;
		f32_t weightSum = {};
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (indices[i] >= boneCount || !isfinite(pWeights[i]) || pWeights[i] < 0.f)
				return false;
			weightSum += pWeights[i];
		}

		if (weightSum <= 1e-6f)
		{
			outVertex.vBlendIndices = XMUINT4(0, 0, 0, 0);
			outVertex.vBlendWeights = float4_t(1.f, 0.f, 0.f, 0.f);
		}
		else
		{
			const f32_t inverseWeight = 1.f / weightSum;
			for (uint32_t i = 0; i < 4; ++i)
				pWeights[i] *= inverseWeight;
		}

		const vector_t normal = SafeNormalize3(XMLoadFloat3(&outVertex.vNormal), XMVectorSet(0.f, 1.f, 0.f, 0.f));
		const vector_t tangent = SafeNormalize3(XMLoadFloat3(&outVertex.vTangent), XMVectorSet(1.f, 0.f, 0.f, 0.f));
		const vector_t binormal = SafeNormalize3(
			XMVector3Cross(normal, tangent),
			XMVectorSet(0.f, 0.f, 1.f, 0.f));
		XMStoreFloat3(&outVertex.vNormal, normal);
		XMStoreFloat3(&outVertex.vTangent, tangent);
		XMStoreFloat3(&outVertex.vBinormal, binormal);
		return true;
	}
}

bool_t CWMeshReader::Read(const filesystem::path& meshPath,
	W_MESH_READ_RESULT& outMesh,
	MODEL_DECODE_REPORT& outReport) const
{
	outMesh = {};

	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(meshPath, bytes))
	{
		outReport.error = "Could not open the binary mesh file.";
		return false;
	}

	try
	{
		CBinaryReader fileReader(bytes.data(), bytes.size());
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
			1 != fileHeader.versionMajor || 0 != fileHeader.flags ||
			fileHeader.contentSize > fileReader.Remaining())
		{
			outReport.error = "Invalid WINT mesh file header.";
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const MESH_META_HEADER meshHeader = reader.Read<MESH_META_HEADER>();
		const bool_t skinned = 0 != (meshHeader.vertexFormatFlags & VF_BONE_WEIGHT);
		if (!HasMagic(meshHeader.magic, WMESH_MAGIC) ||
			0 == meshHeader.submeshCount || meshHeader.submeshCount > MAX_SUBMESHES ||
			meshHeader.boneCount > MAX_BONES ||
			meshHeader.totalVertexCount > MAX_VERTICES ||
			(meshHeader.indexStride != 2 && meshHeader.indexStride != 4) ||
			(skinned && (0 == meshHeader.boneCount || meshHeader.vertexStride != STRIDE_SKINNED)) ||
			(!skinned && meshHeader.vertexStride != STRIDE_STATIC))
		{
			outReport.error = "Invalid WMSH metadata or unsupported bone count.";
			return false;
		}

		vector<SUBMESH_DESC> submeshes(meshHeader.submeshCount);
		reader.ReadBytes(submeshes.data(), sizeof(SUBMESH_DESC) * submeshes.size());

		size_t vertexBlobBytes = {};
		size_t indexBlobBytes = {};
		if (!CheckedByteCount(meshHeader.totalVertexCount, meshHeader.vertexStride, reader.Remaining(), vertexBlobBytes))
		{
			outReport.error = "Vertex block exceeds the WMSH payload.";
			return false;
		}
		const uint8_t* pVertexBlob = reader.Peek();
		reader.Skip(vertexBlobBytes);

		if (!CheckedByteCount(meshHeader.totalIndexCount, meshHeader.indexStride, reader.Remaining(), indexBlobBytes))
		{
			outReport.error = "Index block exceeds the WMSH payload.";
			return false;
		}
		const uint8_t* pIndexBlob = reader.Peek();
		reader.Skip(indexBlobBytes);

		outMesh.bones.resize(meshHeader.boneCount);
		for (uint32_t i = 0; i < meshHeader.boneCount; ++i)
		{
			const MESH_BONE_ENTRY source = reader.Read<MESH_BONE_ENTRY>();
			W_MESH_BONE_DATA& destination = outMesh.bones[i];
			destination.nameHash = source.nameHash;
			destination.name = ReadFixedName(source.name, sizeof(source.name));
			memcpy(&destination.inverseBind, source.offsetMatrix, sizeof(float4x4_t));
			const f32_t* pMatrix = reinterpret_cast<const f32_t*>(&destination.inverseBind);
			for (uint32_t element = 0; element < 16; ++element)
			{
				if (!isfinite(pMatrix[element]))
				{
					outReport.error = "WMSH contains a non-finite inverse-bind matrix.";
					return false;
				}
			}
		}

		if (meshHeader.hasBounding)
		{
			size_t boundsBytes = {};
			if (!CheckedByteCount(meshHeader.submeshCount, 40, reader.Remaining(), boundsBytes))
			{
				outReport.error = "Bounds block exceeds the WMSH payload.";
				return false;
			}
			reader.Skip(boundsBytes);
		}

		outMesh.skinned = skinned;
		outMesh.meshes.reserve(submeshes.size());
		for (const SUBMESH_DESC& sourceMesh : submeshes)
		{
			size_t sourceVertexBytes = {};
			size_t sourceIndexBytes = {};
			if (!CheckedByteCount(sourceMesh.vertexCount, meshHeader.vertexStride, vertexBlobBytes, sourceVertexBytes) ||
				sourceMesh.vertexOffset > vertexBlobBytes || sourceVertexBytes > vertexBlobBytes - sourceMesh.vertexOffset ||
				!CheckedByteCount(sourceMesh.indexCount, meshHeader.indexStride, indexBlobBytes, sourceIndexBytes) ||
				sourceMesh.indexOffset > indexBlobBytes || sourceIndexBytes > indexBlobBytes - sourceMesh.indexOffset)
			{
				outReport.error = "A submesh points outside its vertex or index block.";
				return false;
			}

			MODEL_MESH_DATA mesh{};
			mesh.name = ReadFixedName(sourceMesh.name, sizeof(sourceMesh.name));
			mesh.materialIndex = sourceMesh.materialIndex;
			mesh.vertexKind = skinned ? MODEL_VERTEX_KIND::SKINNED : MODEL_VERTEX_KIND::STATIC;
			mesh.indices.resize(sourceMesh.indexCount);

			const uint8_t* pVertices = pVertexBlob + sourceMesh.vertexOffset;
			if (skinned)
			{
				mesh.skinnedVertices.resize(sourceMesh.vertexCount);
				for (uint32_t i = 0; i < sourceMesh.vertexCount; ++i)
				{
					if (!MakeSkinnedVertex(
						pVertices + static_cast<size_t>(i) * meshHeader.vertexStride,
						meshHeader.boneCount,
						mesh.skinnedVertices[i]))
					{
						outReport.error = "A skinned vertex contains invalid bone data.";
						return false;
					}
				}
			}
			else
			{
				mesh.vertices.resize(sourceMesh.vertexCount);
				for (uint32_t i = 0; i < sourceMesh.vertexCount; ++i)
				{
					if (!MakeStaticVertex(
						pVertices + static_cast<size_t>(i) * meshHeader.vertexStride,
						mesh.vertices[i]))
					{
						outReport.error = "A static vertex contains non-finite data.";
						return false;
					}
				}
			}

			const uint8_t* pIndices = pIndexBlob + sourceMesh.indexOffset;
			for (uint32_t i = 0; i < sourceMesh.indexCount; ++i)
			{
				uint32_t index = {};
				if (2 == meshHeader.indexStride)
				{
					uint16_t sourceIndex = {};
					memcpy(&sourceIndex, pIndices + static_cast<size_t>(i) * 2, sizeof(sourceIndex));
					index = sourceIndex;
				}
				else
				{
					memcpy(&index, pIndices + static_cast<size_t>(i) * 4, sizeof(index));
				}

				if (index >= sourceMesh.vertexCount)
				{
					outReport.error = "A submesh contains an out-of-range index.";
					return false;
				}
				mesh.indices[i] = index;
			}

			outMesh.materialCount = (max)(outMesh.materialCount, sourceMesh.materialIndex + 1);
			outReport.vertexCount += sourceMesh.vertexCount;
			outReport.indexCount += sourceMesh.indexCount;
			outMesh.meshes.push_back(move(mesh));
		}

		return true;
	}
	catch (const exception& exception)
	{
		outReport.error = exception.what();
		return false;
	}
}
