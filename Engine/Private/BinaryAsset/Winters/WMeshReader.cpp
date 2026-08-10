#include "WMeshReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <bcrypt.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>

#pragma comment(lib, "bcrypt.lib")

using namespace Engine::WintersFormat;

namespace
{
	constexpr f32_t TANGENT_HANDEDNESS_ABSOLUTE_TOLERANCE = 1e-6f;

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

	bool_t ComputeSha256(const uint8_t* pData, size_t dataSize,
		array<uint8_t, SHA256_SIZE>& outDigest)
	{
		if (nullptr == pData || dataSize > ULONG_MAX)
			return false;

		BCRYPT_ALG_HANDLE algorithm = { nullptr };
		BCRYPT_HASH_HANDLE hash = { nullptr };
		DWORD objectSize = {};
		DWORD resultSize = {};
		vector<uint8_t> object;
		bool_t succeeded = false;
		if (BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
			goto cleanup;
		if (BCryptGetProperty(
			algorithm, BCRYPT_OBJECT_LENGTH,
			reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize),
			&resultSize, 0) < 0 || sizeof(objectSize) != resultSize || 0 == objectSize)
			goto cleanup;
		object.resize(objectSize);
		if (BCryptCreateHash(
			algorithm, &hash, object.data(), objectSize,
			nullptr, 0, 0) < 0)
			goto cleanup;
		if (BCryptHashData(
			hash, const_cast<PUCHAR>(pData),
			static_cast<ULONG>(dataSize), 0) < 0)
			goto cleanup;
		if (BCryptFinishHash(
			hash, outDigest.data(), static_cast<ULONG>(outDigest.size()), 0) < 0)
			goto cleanup;
		succeeded = true;

	cleanup:
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0);
		return succeeded;
	}

	bool_t IsNonZeroDigest(const uint8_t* pDigest)
	{
		for (uint32_t i = 0; i < SHA256_SIZE; ++i)
		{
			if (0 != pDigest[i])
				return true;
		}
		return false;
	}

	void CopyDigest(array<uint8_t, SHA256_SIZE>& destination,
		const uint8_t* pSource)
	{
		memcpy(destination.data(), pSource, destination.size());
	}

	bool_t NearlyEqual(f32_t left, f32_t right,
		f32_t relativeTolerance = 1e-5f,
		f32_t absoluteTolerance = 1e-5f)
	{
		if (!isfinite(left) || !isfinite(right) ||
			!isfinite(relativeTolerance) || !isfinite(absoluteTolerance) ||
			relativeTolerance < 0.f || absoluteTolerance < 0.f)
			return false;

		const f32_t difference = fabsf(left - right);
		const f32_t scale = (max)(fabsf(left), fabsf(right));
		const f32_t relativeBound = relativeTolerance * scale;
		if (!isfinite(difference) || !isfinite(scale) || !isfinite(relativeBound))
			return false;
		return difference <= (max)(absoluteTolerance, relativeBound);
	}

	bool_t IsValidTangentHandedness(f32_t value)
	{
		if (!isfinite(value))
			return false;
		const f32_t difference = fabsf(fabsf(value) - 1.f);
		return isfinite(difference) &&
			difference <= TANGENT_HANDEDNESS_ABSOLUTE_TOLERANCE;
	}

	bool_t ValidateGeometryMetadata(
		const uint8_t* pPayload,
		size_t payloadSize,
		uint32_t vertexFormatFlags,
		const MESH_GEOMETRY_METADATA_V1& source,
		MODEL_GEOMETRY_METADATA_DATA& destination,
		string& outError)
	{
		if (!HasMagic(source.magic, WGEOMETRY_MAGIC) ||
			1 != source.versionMajor || 0 != source.versionMinor ||
			source.byteSize != sizeof(MESH_GEOMETRY_METADATA_V1) ||
			source.payloadSize != payloadSize)
		{
			outError = "WMSH geometry metadata version or payload size is invalid.";
			return false;
		}

		const bool_t hasColor0 = 0 != (vertexFormatFlags & VF_COLOR0);
		if (0 != (source.evidenceFlags & ~MGEF_KNOWN) ||
			(source.evidenceFlags & MGEF_REQUIRED_PAYLOAD) != MGEF_REQUIRED_PAYLOAD ||
			0 != (source.evidenceFlags & MGEF_PRODUCT_PROVENANCE) ||
			hasColor0 != (0 != (source.evidenceFlags & MGEF_COLOR0_PRESERVED_FROM_GLTF)))
		{
			outError = "WMSH geometry metadata evidence flags do not match its channels.";
			return false;
		}

		const f32_t reciprocalSourceScale = 1.f / source.sourceToWModelScale;
		if (!isfinite(source.sourceToWModelScale) ||
			!isfinite(source.geometryPreScale) ||
			source.sourceToWModelScale <= 0.f ||
			source.geometryPreScale <= 0.f ||
			!isfinite(reciprocalSourceScale) || reciprocalSourceScale <= 0.f ||
			!NearlyEqual(source.geometryPreScale, reciprocalSourceScale, 1e-6f, 0.f))
		{
			outError = "WMSH geometry metadata scale contract is invalid.";
			return false;
		}

		const uint8_t* requiredDigests[] = {
			source.payloadSha256,
			source.sourceGltfSha256,
			source.sourceBufferSetSha256,
			source.sourcePackageObservedUnboundSha256,
			source.sourceObjectPathHash,
			source.legacyConverterObservedUnboundSha256,
			source.geometryToolSha256,
			source.sourceExportReceiptSha256,
			source.legacyCookReceiptSha256,
			source.metadataSha256,
		};
		for (const uint8_t* pDigest : requiredDigests)
		{
			if (!IsNonZeroDigest(pDigest))
			{
				outError = "WMSH geometry metadata contains an empty SHA-256 digest.";
				return false;
			}
		}

		array<uint8_t, SHA256_SIZE> payloadDigest{};
		array<uint8_t, SHA256_SIZE> metadataDigest{};
		if (!ComputeSha256(pPayload, payloadSize, payloadDigest) ||
			0 != memcmp(payloadDigest.data(), source.payloadSha256, SHA256_SIZE))
		{
			outError = "WMSH geometry payload SHA-256 does not match its metadata.";
			return false;
		}
		if (!ComputeSha256(
			reinterpret_cast<const uint8_t*>(&source),
			offsetof(MESH_GEOMETRY_METADATA_V1, metadataSha256),
			metadataDigest) ||
			0 != memcmp(metadataDigest.data(), source.metadataSha256, SHA256_SIZE))
		{
			outError = "WMSH geometry metadata SHA-256 is invalid.";
			return false;
		}

		destination.present = true;
		destination.versionMajor = WINT_VERSION_MAJOR;
		destination.versionMinor = WINT_GEOMETRY_VERSION_MINOR;
		destination.channelMask = vertexFormatFlags;
		destination.evidenceFlags = source.evidenceFlags;
		destination.payloadSize = source.payloadSize;
		destination.sourceToWModelScale = source.sourceToWModelScale;
		destination.geometryPreScale = source.geometryPreScale;
		CopyDigest(destination.payloadSha256, source.payloadSha256);
		CopyDigest(destination.sourceGltfSha256, source.sourceGltfSha256);
		CopyDigest(destination.sourceBufferSetSha256, source.sourceBufferSetSha256);
		CopyDigest(destination.sourcePackageObservedUnboundSha256,
			source.sourcePackageObservedUnboundSha256);
		CopyDigest(destination.sourceObjectPathHash, source.sourceObjectPathHash);
		CopyDigest(destination.legacyConverterObservedUnboundSha256,
			source.legacyConverterObservedUnboundSha256);
		CopyDigest(destination.geometryToolSha256, source.geometryToolSha256);
		CopyDigest(destination.sourceExportReceiptSha256, source.sourceExportReceiptSha256);
		CopyDigest(destination.legacyCookReceiptSha256, source.legacyCookReceiptSha256);
		CopyDigest(destination.metadataSha256, source.metadataSha256);
		CopyDigest(destination.metadataIdentitySha256, source.metadataSha256);
		return true;
	}

	bool_t ValidateBoundsShape(const MESH_BOUNDS_V1& bounds)
	{
		for (uint32_t axis = 0; axis < 3; ++axis)
		{
			const f32_t halfMinimum = 0.5f * bounds.minimum[axis];
			const f32_t halfMaximum = 0.5f * bounds.maximum[axis];
			const f32_t expectedCenter = halfMinimum + halfMaximum;
			if (!isfinite(bounds.minimum[axis]) ||
				!isfinite(bounds.maximum[axis]) ||
				!isfinite(bounds.center[axis]) ||
				!isfinite(halfMinimum) || !isfinite(halfMaximum) ||
				!isfinite(expectedCenter) ||
				bounds.minimum[axis] > bounds.maximum[axis] ||
				!NearlyEqual(bounds.center[axis], expectedCenter))
				return false;
		}
		return isfinite(bounds.radius) && bounds.radius >= 0.f;
	}

	bool_t ValidateBoundsAgainstVertices(
		const MESH_BOUNDS_V1& bounds,
		const vector<VTXMESH>& vertices)
	{
		if (vertices.empty())
			return false;

		float3_t derivedMinimum = vertices.front().vPosition;
		float3_t derivedMaximum = vertices.front().vPosition;
		f32_t derivedRadius = {};
		for (const VTXMESH& vertex : vertices)
		{
			const f32_t values[] = {
				vertex.vPosition.x,
				vertex.vPosition.y,
				vertex.vPosition.z,
			};
			f32_t distanceSquared = {};
			for (uint32_t axis = 0; axis < 3; ++axis)
			{
				(&derivedMinimum.x)[axis] = (min)((&derivedMinimum.x)[axis], values[axis]);
				(&derivedMaximum.x)[axis] = (max)((&derivedMaximum.x)[axis], values[axis]);
				const f32_t delta = values[axis] - bounds.center[axis];
				const f32_t squaredDelta = delta * delta;
				const f32_t accumulatedDistance = distanceSquared + squaredDelta;
				if (!isfinite(delta) || !isfinite(squaredDelta) ||
					!isfinite(accumulatedDistance))
					return false;
				distanceSquared = accumulatedDistance;
			}
			const f32_t distance = sqrtf(distanceSquared);
			if (!isfinite(distance))
				return false;
			derivedRadius = (max)(derivedRadius, distance);
		}

		for (uint32_t axis = 0; axis < 3; ++axis)
		{
			if (!NearlyEqual(bounds.minimum[axis], (&derivedMinimum.x)[axis]) ||
				!NearlyEqual(bounds.maximum[axis], (&derivedMaximum.x)[axis]))
				return false;
		}
		return NearlyEqual(bounds.radius, derivedRadius);
	}

	vector_t SafeNormalize3(vector_t value, vector_t fallback)
	{
		const f32_t lengthSq = XMVectorGetX(XMVector3LengthSq(value));
		return isfinite(lengthSq) && lengthSq > 1e-12f
			? XMVector3Normalize(value)
			: fallback;
	}

	bool_t TryNormalizeGeometryBasis(vector_t value, vector_t& outNormalized)
	{
		const f32_t lengthSquared = XMVectorGetX(XMVector3LengthSq(value));
		if (!isfinite(lengthSquared) || lengthSquared <= 1e-12f)
			return false;

		outNormalized = XMVector3Normalize(value);
		float3_t normalized{};
		XMStoreFloat3(&normalized, outNormalized);
		return IsFinite3(normalized);
	}

	bool_t MakeStaticVertex(const uint8_t* pSource,
		bool_t strictHandedness,
		bool_t hasColor0,
		VTXMESH& outVertex,
		f32_t& outTangentHandedness,
		uint32_t& outColor0Rgba8)
	{
		memcpy(&outVertex.vPosition, pSource + 0, sizeof(float3_t));
		memcpy(&outVertex.vNormal, pSource + 12, sizeof(float3_t));
		memcpy(&outVertex.vTexcoord, pSource + 24, sizeof(float2_t));
		memcpy(&outVertex.vTangent, pSource + 32, sizeof(float3_t));

		f32_t handedness = 1.f;
		memcpy(&handedness, pSource + 44, sizeof(f32_t));
		if (!IsFinite3(outVertex.vPosition) || !IsFinite3(outVertex.vNormal) ||
			!IsFinite3(outVertex.vTangent) || !isfinite(outVertex.vTexcoord.x) ||
			!isfinite(outVertex.vTexcoord.y) || !isfinite(handedness) ||
			(strictHandedness && !IsValidTangentHandedness(handedness)))
			return false;
		outTangentHandedness = handedness;
		outColor0Rgba8 = {};
		if (hasColor0)
			memcpy(&outColor0Rgba8, pSource + STRIDE_STATIC, sizeof(uint32_t));

		vector_t normal{};
		vector_t tangent{};
		vector_t unitBinormal{};
		if (strictHandedness)
		{
			if (!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vNormal), normal) ||
				!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vTangent), tangent) ||
				!TryNormalizeGeometryBasis(XMVector3Cross(normal, tangent), unitBinormal))
				return false;
		}
		else
		{
			normal = SafeNormalize3(
				XMLoadFloat3(&outVertex.vNormal), XMVectorSet(0.f, 1.f, 0.f, 0.f));
			tangent = SafeNormalize3(
				XMLoadFloat3(&outVertex.vTangent), XMVectorSet(1.f, 0.f, 0.f, 0.f));
			unitBinormal = SafeNormalize3(
				XMVector3Cross(normal, tangent), XMVectorSet(0.f, 0.f, 1.f, 0.f));
		}
		const vector_t binormal = XMVectorScale(unitBinormal, handedness);

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
	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(meshPath, bytes))
	{
		outReport.error = "Could not open the binary mesh file.";
		return false;
	}
	return ReadMemory(bytes.data(), bytes.size(), outMesh, outReport);
}

bool_t CWMeshReader::ReadMemory(const uint8_t* pData,
	size_t dataSize,
	W_MESH_READ_RESULT& outMesh,
	MODEL_DECODE_REPORT& outReport) const
{
	outMesh = {};
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WMSH section is empty or truncated.";
		return false;
	}

	try
	{
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
			WINT_VERSION_MAJOR != fileHeader.versionMajor ||
			fileHeader.versionMinor > WINT_GEOMETRY_VERSION_MINOR ||
			0 != fileHeader.flags ||
			fileHeader.contentSize != fileReader.Remaining())
		{
			outReport.error = "Invalid WINT mesh file header.";
			return false;
		}

		const bool_t geometryContract =
			WINT_GEOMETRY_VERSION_MINOR == fileHeader.versionMinor;
		const uint8_t* pContent = fileReader.Peek();
		CBinaryReader reader(pContent, fileHeader.contentSize);
		const MESH_META_HEADER meshHeader = reader.Read<MESH_META_HEADER>();
		const bool_t skinned = 0 != (meshHeader.vertexFormatFlags & VF_BONE_WEIGHT);
		const bool_t hasColor0 = 0 != (meshHeader.vertexFormatFlags & VF_COLOR0);
		const uint32_t expectedStride = hasColor0 ?
			STRIDE_STATIC_COLOR0 : STRIDE_STATIC;
		const bool_t versionedFlagsValid = geometryContract ?
			(!skinned &&
				(meshHeader.vertexFormatFlags & VF_STATIC_BASE) == VF_STATIC_BASE &&
				0 != (meshHeader.vertexFormatFlags & VF_TANGENT_HANDEDNESS) &&
				0 == (meshHeader.vertexFormatFlags &
					~(VF_STATIC_BASE | VF_TANGENT_HANDEDNESS | VF_COLOR0))) :
			((meshHeader.vertexFormatFlags & VF_STATIC_BASE) == VF_STATIC_BASE &&
				0 == (meshHeader.vertexFormatFlags &
					~(VF_STATIC_BASE | VF_BONE_WEIGHT)));
		const bool_t reservedValid = !geometryContract ||
			(0 == meshHeader.reserved[0] &&
				0 == meshHeader.reserved[1] &&
				0 == meshHeader.reserved[2]);
		if (!HasMagic(meshHeader.magic, WMESH_MAGIC) ||
			0 == meshHeader.submeshCount || meshHeader.submeshCount > MAX_SUBMESHES ||
			meshHeader.boneCount > MAX_BONES ||
			meshHeader.totalVertexCount > MAX_VERTICES ||
			(meshHeader.indexStride != 2 && meshHeader.indexStride != 4) ||
			meshHeader.hasBounding > 1 ||
			!versionedFlagsValid || !reservedValid ||
			(geometryContract && (0 == meshHeader.hasBounding ||
				meshHeader.vertexStride != expectedStride)) ||
			(skinned && (0 == meshHeader.boneCount || meshHeader.vertexStride != STRIDE_SKINNED)) ||
			(!geometryContract && !skinned && meshHeader.vertexStride != STRIDE_STATIC))
		{
			outReport.error = "Invalid WMSH version, flags, stride, or bone metadata.";
			return false;
		}

		vector<SUBMESH_DESC> submeshes(meshHeader.submeshCount);
		reader.ReadBytes(submeshes.data(), sizeof(SUBMESH_DESC) * submeshes.size());
		if (geometryContract)
		{
			uint64_t expectedVertexOffset = {};
			uint64_t expectedIndexOffset = {};
			for (const SUBMESH_DESC& submesh : submeshes)
			{
				if (submesh.vertexOffset != expectedVertexOffset ||
					submesh.indexOffset != expectedIndexOffset)
				{
					outReport.error = "Versioned WMSH submesh payload ranges are not contiguous.";
					return false;
				}
				expectedVertexOffset +=
					static_cast<uint64_t>(submesh.vertexCount) * meshHeader.vertexStride;
				expectedIndexOffset +=
					static_cast<uint64_t>(submesh.indexCount) * meshHeader.indexStride;
			}
			if (expectedVertexOffset !=
				static_cast<uint64_t>(meshHeader.totalVertexCount) * meshHeader.vertexStride ||
				expectedIndexOffset !=
				static_cast<uint64_t>(meshHeader.totalIndexCount) * meshHeader.indexStride)
			{
				outReport.error = "Versioned WMSH aggregate vertex or index counts do not match.";
				return false;
			}
		}

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

		vector<MESH_BOUNDS_V1> embeddedBounds;
		if (meshHeader.hasBounding)
		{
			size_t boundsBytes = {};
			if (!CheckedByteCount(
				meshHeader.submeshCount, sizeof(MESH_BOUNDS_V1),
				reader.Remaining(), boundsBytes))
			{
				outReport.error = "Bounds block exceeds the WMSH payload.";
				return false;
			}
			if (geometryContract)
			{
				embeddedBounds.resize(meshHeader.submeshCount);
				reader.ReadBytes(embeddedBounds.data(), boundsBytes);
				for (const MESH_BOUNDS_V1& bounds : embeddedBounds)
				{
					if (!ValidateBoundsShape(bounds))
					{
						outReport.error = "WMSH contains invalid WModel-space bounds metadata.";
						return false;
					}
				}
			}
			else
			{
				reader.Skip(boundsBytes);
			}
		}

		if (geometryContract)
		{
			const size_t geometryPayloadSize = static_cast<size_t>(
				reader.Peek() - pContent);
			if (reader.Remaining() != sizeof(MESH_GEOMETRY_METADATA_V1))
			{
				outReport.error = "WMSH geometry metadata is missing, truncated, or has trailing bytes.";
				return false;
			}
			const MESH_GEOMETRY_METADATA_V1 metadata =
				reader.Read<MESH_GEOMETRY_METADATA_V1>();
			if (!ValidateGeometryMetadata(
				pContent, geometryPayloadSize,
				meshHeader.vertexFormatFlags, metadata,
				outMesh.geometryMetadata, outReport.error))
				return false;
		}
		else if (0 != reader.Remaining())
		{
			outReport.error = "Legacy WMSH contains unsupported trailing payload.";
			return false;
		}

		outMesh.skinned = skinned;
		outMesh.fileVersionMinor = fileHeader.versionMinor;
		outMesh.meshes.reserve(submeshes.size());
		for (uint32_t submeshIndex = 0;
			submeshIndex < submeshes.size(); ++submeshIndex)
		{
			const SUBMESH_DESC& sourceMesh = submeshes[submeshIndex];
			if (sourceMesh.materialIndex >= MAX_MATERIALS)
			{
				outReport.error = "A submesh material index exceeds the supported range.";
				return false;
			}
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
			if (geometryContract && (0 == sourceMesh.vertexCount ||
				0 == sourceMesh.indexCount || 0 != sourceMesh.indexCount % 3))
			{
				outReport.error = "A versioned WMSH submesh is not an indexed triangle list.";
				return false;
			}

			MODEL_MESH_DATA mesh{};
			mesh.name = ReadFixedName(sourceMesh.name, sizeof(sourceMesh.name));
			mesh.materialIndex = sourceMesh.materialIndex;
			mesh.vertexKind = skinned ? MODEL_VERTEX_KIND::SKINNED : MODEL_VERTEX_KIND::STATIC;
			mesh.hasColor0 = geometryContract && hasColor0;
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
				if (geometryContract)
					mesh.tangentHandedness.resize(sourceMesh.vertexCount);
				if (geometryContract && hasColor0)
					mesh.color0Rgba8.resize(sourceMesh.vertexCount);
				for (uint32_t i = 0; i < sourceMesh.vertexCount; ++i)
				{
					f32_t tangentHandedness = { 1.f };
					uint32_t color0Rgba8 = {};
					if (!MakeStaticVertex(
						pVertices + static_cast<size_t>(i) * meshHeader.vertexStride,
						geometryContract,
						hasColor0,
						mesh.vertices[i],
						tangentHandedness,
						color0Rgba8))
					{
						outReport.error = "A static vertex contains invalid geometry channel data.";
						return false;
					}
					if (geometryContract)
						mesh.tangentHandedness[i] = tangentHandedness;
					if (geometryContract && hasColor0)
						mesh.color0Rgba8[i] = color0Rgba8;
				}

				if (geometryContract)
				{
					const MESH_BOUNDS_V1& bounds = embeddedBounds[submeshIndex];
					if (!ValidateBoundsAgainstVertices(bounds, mesh.vertices))
					{
						outReport.error = "WMSH embedded and vertex-derived bounds do not match.";
						return false;
					}
					mesh.embeddedBounds.present = true;
					memcpy(&mesh.embeddedBounds.minimum, bounds.minimum, sizeof(float3_t));
					memcpy(&mesh.embeddedBounds.maximum, bounds.maximum, sizeof(float3_t));
					memcpy(&mesh.embeddedBounds.center, bounds.center, sizeof(float3_t));
					mesh.embeddedBounds.radius = bounds.radius;
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
