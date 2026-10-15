// WModelDecoder에 넘길 CPU mesh·본 대응 정보·재질 슬롯 수를 구성한다.
#include "WMeshReader.h"
// packed 입력을 GPU buffer 생성에 쓰는 VTXMESH/VTXANIMMESH 원소로 변환한다.
#include "Engine_VertexTypes.h"

// 파일 bytes의 소유 배열과 section 범위를 검사하는 차용 커서를 사용한다.
#include "BinaryAsset/BinaryReader.h"
// 버전별 정점 stride와 header·submesh·geometry 증거의 저장 ABI를 사용한다.
#include "WFormatTypes.h"

// 저장 payload와 metadata를 대조할 SHA-256 provider·hash handle API를 사용한다.
#include <bcrypt.h>
// 고정 크기 SHA-256 결과를 값으로 소유한다.
#include <array>
// 성분의 유한성, tangent 부호 허용 오차와 bounds 거리 계산에 사용한다.
#include <cmath>
// metadataSha256 필드 이전 byte 범위를 offsetof로 정한다.
#include <cstddef>
// 입력 정렬에 의존하지 않고 magic·정점·행렬·digest를 비교하거나 복사한다.
#include <cstring>
// 배열 개수와 stride 곱이 uint64 범위를 넘는지 확인한다.
#include <limits>

// 위 BCrypt 호출을 Windows CNG 라이브러리에 연결한다.
#pragma comment(lib, "bcrypt.lib")

using namespace Engine::WintersFormat;

namespace
{
	constexpr f32_t TANGENT_HANDEDNESS_ABSOLUTE_TOLERANCE = 1e-6f;

	// 호출자가 읽어 놓은 header 저장소의 4바이트를 비교한다. NUL 종료 문자열을 요구하지 않는다.
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	// count×stride overflow와 남은 영역을 먼저 검사한다. outBytes는 성공할 때만 기록한다.
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

	// 고정 폭 이름을 NUL 또는 capacity까지 복사한다. 파일 안 nameHash와 문자열의 일치를 계산하지 않는다.
	string ReadFixedName(const char* pName, size_t capacity)
	{
		size_t length = {};
		while (length < capacity && '\0' != pName[length])
			++length;
		return string(pName, length);
	}

	// 정점 위치·방향과 정규화 결과의 세 성분을 검사한다. 벡터 길이와 cross의 퇴화 여부는 호출 경로에서 따로 판단한다.
	bool_t IsFinite3(const float3_t& value)
	{
		return isfinite(value.x) && isfinite(value.y) && isfinite(value.z);
	}

	// payload/metadata의 자체 일관성 검사에 사용하는 SHA-256이다. 성공 경로와 API 실패 goto에서 BCrypt handle을 정리한다.
	// vector 할당 예외까지 RAII로 handle을 회수하는 helper는 아니며 바깥 reader가 예외를 진단한다.
	bool_t ComputeSha256(const uint8_t* pData, size_t dataSize, array<uint8_t, SHA256_SIZE>& outDigest)
	{
		if (nullptr == pData || dataSize > ULONG_MAX)
			return false;

		BCRYPT_ALG_HANDLE algorithm = {nullptr};
		BCRYPT_HASH_HANDLE hash = {nullptr};
		DWORD objectSize = {};
		DWORD resultSize = {};
		vector<uint8_t> object;
		bool_t succeeded = false;
		if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
			goto cleanup;
		if (BCryptGetProperty(
				algorithm,
				BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectSize),
				sizeof(objectSize),
				&resultSize,
				0) < 0 ||
			sizeof(objectSize) != resultSize || 0 == objectSize)
			goto cleanup;
		object.resize(objectSize);
		if (BCryptCreateHash(algorithm, &hash, object.data(), objectSize, nullptr, 0, 0) < 0)
			goto cleanup;
		if (BCryptHashData(hash, const_cast<PUCHAR>(pData), static_cast<ULONG>(dataSize), 0) < 0)
			goto cleanup;
		if (BCryptFinishHash(hash, outDigest.data(), static_cast<ULONG>(outDigest.size()), 0) < 0)
			goto cleanup;
		succeeded = true;

	cleanup:
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0);
		return succeeded;
	}

	// digest가 전부 0인 placeholder만 거절한다. 원본 파일을 찾아 해당 digest와 대조하는 검사는 아니다.
	bool_t IsNonZeroDigest(const uint8_t* pDigest)
	{
		for (uint32_t i = 0; i < SHA256_SIZE; ++i)
		{
			if (0 != pDigest[i])
				return true;
		}
		return false;
	}

	// 고정 크기 source digest를 결과 배열에 복사한다. 양쪽 저장소는 호출 동안 유효해야 한다.
	void CopyDigest(array<uint8_t, SHA256_SIZE>& destination, const uint8_t* pSource)
	{
		memcpy(destination.data(), pSource, destination.size());
	}

	// bounds/scale 비교는 절대 오차와 상대 오차 중 큰 한계를 쓴다. 차/곱 계산이 비유한 값이 되면 실패한다.
	bool_t NearlyEqual(
		f32_t left, f32_t right, f32_t relativeTolerance = 1e-5f, f32_t absoluteTolerance = 1e-5f)
	{
		if (!isfinite(left) || !isfinite(right) || !isfinite(relativeTolerance) ||
			!isfinite(absoluteTolerance) || relativeTolerance < 0.f || absoluteTolerance < 0.f)
			return false;

		const f32_t difference = fabsf(left - right);
		const f32_t scale = (max)(fabsf(left), fabsf(right));
		const f32_t relativeBound = relativeTolerance * scale;
		if (!isfinite(difference) || !isfinite(scale) || !isfinite(relativeBound))
			return false;
		return difference <= (max)(absoluteTolerance, relativeBound);
	}

	// handedness는 유한한 ±1에서 절대 오차 1e-6 이내여야 한다. normal/tangent의 직교성 검사와는 별도다.
	bool_t IsValidTangentHandedness(f32_t value)
	{
		if (!isfinite(value))
			return false;
		const f32_t difference = fabsf(fabsf(value) - 1.f);
		return isfinite(difference) && difference <= TANGENT_HANDEDNESS_ABSOLUTE_TOLERANCE;
	}

	// geometry payload와 metadata의 SHA를 재계산하고 채널/배율/증거 flag 조합을 대조한다.
	// 저장된 원본 경로/도구 digest를 외부 정본과 인증하지 않으며 PRODUCT_PROVENANCE flag는 이 경로에서 거절한다.
	bool_t ValidateGeometryMetadata(
		const uint8_t* pPayload,
		size_t payloadSize,
		uint32_t vertexFormatFlags,
		uint16_t fileVersionMinor,
		const MESH_GEOMETRY_METADATA_V1& source,
		MODEL_GEOMETRY_METADATA_DATA& destination,
		string& outError)
	{
		if (!HasMagic(source.magic, WGEOMETRY_MAGIC) || 1 != source.versionMajor || 0 != source.versionMinor ||
			source.byteSize != sizeof(MESH_GEOMETRY_METADATA_V1) || source.payloadSize != payloadSize)
		{
			outError = "WMSH geometry metadata version or payload size is invalid.";
			return false;
		}

		// 실제 정점 채널과 파일이 선언한 보존/재구성 flag를 맞춘다. native 평행 basis 보존은 UV1과 비재구성 조건이 필요하다.
		const bool_t hasColor0 = 0 != (vertexFormatFlags & VF_COLOR0);
		const bool_t hasUV1 = 0 != (vertexFormatFlags & VF_TEXCOORD1);
		const bool_t hasUV2 = 0 != (vertexFormatFlags & VF_TEXCOORD2);
		const bool_t reconstructed =
			0 != (source.evidenceFlags & MGEF_TANGENT_HANDEDNESS_PROJECT_RECONSTRUCTED);
		const uint32_t required = reconstructed
			? (MGEF_REQUIRED_PAYLOAD & ~MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF)
			: MGEF_REQUIRED_PAYLOAD;
		if (0 != (source.evidenceFlags & ~MGEF_KNOWN) || (source.evidenceFlags & required) != required ||
			(0 != (source.evidenceFlags & MGEF_NATIVE_PARALLEL_BASIS_PRESERVED) &&
			 (!hasUV1 || reconstructed)) ||
			(reconstructed &&
			 (fileVersionMinor < WINT_UV1_VERSION_MINOR ||
			  0 != (source.evidenceFlags & MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF))) ||
			hasUV1 != (0 != (source.evidenceFlags & MGEF_TEXCOORD1_PRESERVED_FROM_GLTF)) ||
			hasUV2 != (0 != (source.evidenceFlags & MGEF_TEXCOORD2_PRESERVED_FROM_GLTF)) ||
			0 != (source.evidenceFlags & MGEF_PRODUCT_PROVENANCE) ||
			hasColor0 != (0 != (source.evidenceFlags & MGEF_COLOR0_PRESERVED_FROM_GLTF)))
		{
			outError = "WMSH geometry metadata evidence flags do not match its channels.";
			return false;
		}

		// sourceToWModelScale의 역수가 geometryPreScale인지 상대 오차로 대조한다. 단위 변환을 여기서 정점에 다시 곱하지 않는다.
		const f32_t reciprocalSourceScale = 1.f / source.sourceToWModelScale;
		if (!isfinite(source.sourceToWModelScale) || !isfinite(source.geometryPreScale) ||
			source.sourceToWModelScale <= 0.f || source.geometryPreScale <= 0.f ||
			!isfinite(reciprocalSourceScale) || reciprocalSourceScale <= 0.f ||
			!NearlyEqual(source.geometryPreScale, reciprocalSourceScale, 1e-6f, 0.f))
		{
			outError = "WMSH geometry metadata scale contract is invalid.";
			return false;
		}

		// 필수 digest 필드가 빈 placeholder인지 먼저 확인하고, 아래에서 이 파일의 payload와 metadata digest만 재계산한다.
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

		// 전체 metadata 검증 뒤 값과 digest를 소유 결과에 복사한다. false 경로에서는 destination을 채우지 않는다.
		destination.present = true;
		destination.versionMajor = WINT_VERSION_MAJOR;
		destination.versionMinor = fileVersionMinor;
		destination.channelMask = vertexFormatFlags;
		destination.evidenceFlags = source.evidenceFlags;
		destination.payloadSize = source.payloadSize;
		destination.sourceToWModelScale = source.sourceToWModelScale;
		destination.geometryPreScale = source.geometryPreScale;
		CopyDigest(destination.payloadSha256, source.payloadSha256);
		CopyDigest(destination.sourceGltfSha256, source.sourceGltfSha256);
		CopyDigest(destination.sourceBufferSetSha256, source.sourceBufferSetSha256);
		CopyDigest(destination.sourcePackageObservedUnboundSha256, source.sourcePackageObservedUnboundSha256);
		CopyDigest(destination.sourceObjectPathHash, source.sourceObjectPathHash);
		CopyDigest(
			destination.legacyConverterObservedUnboundSha256, source.legacyConverterObservedUnboundSha256);
		CopyDigest(destination.geometryToolSha256, source.geometryToolSha256);
		CopyDigest(destination.sourceExportReceiptSha256, source.sourceExportReceiptSha256);
		CopyDigest(destination.legacyCookReceiptSha256, source.legacyCookReceiptSha256);
		CopyDigest(destination.metadataSha256, source.metadataSha256);
		CopyDigest(destination.metadataIdentitySha256, source.metadataSha256);
		return true;
	}

	// AABB min/max 순서와 중심=(min+max)/2, 반경의 finite/비음수를 확인한다. 각 값을 먼저 반으로 나눠 큰 합의 overflow를 줄인다.
	bool_t ValidateBoundsShape(const MESH_BOUNDS_V1& bounds)
	{
		for (uint32_t axis = 0; axis < 3; ++axis)
		{
			const f32_t halfMinimum = 0.5f * bounds.minimum[axis];
			const f32_t halfMaximum = 0.5f * bounds.maximum[axis];
			const f32_t expectedCenter = halfMinimum + halfMaximum;
			if (!isfinite(bounds.minimum[axis]) || !isfinite(bounds.maximum[axis]) ||
				!isfinite(bounds.center[axis]) || !isfinite(halfMinimum) || !isfinite(halfMaximum) ||
				!isfinite(expectedCenter) || bounds.minimum[axis] > bounds.maximum[axis] ||
				!NearlyEqual(bounds.center[axis], expectedCenter))
				return false;
		}
		return isfinite(bounds.radius) && bounds.radius >= 0.f;
	}

	// 정적 정점 전체로 AABB와 저장 중심 기준 최대 거리를 다시 구한다. 구는 최소 구가 아니라 AABB 중심의 보수적 구다.
	bool_t ValidateBoundsAgainstVertices(const MESH_BOUNDS_V1& bounds, const vector<VTXMESH>& vertices)
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
				if (!isfinite(delta) || !isfinite(squaredDelta) || !isfinite(accumulatedDistance))
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

	// legacy basis는 길이가 너무 작거나 비유한 경우 지정 축으로 대체한다. versioned strict basis는 아래 별도 실패 경로를 쓴다.
	vector_t SafeNormalize3(vector_t value, vector_t fallback)
	{
		const f32_t lengthSq = XMVectorGetX(XMVector3LengthSq(value));
		return isfinite(lengthSq) && lengthSq > 1e-12f ? XMVector3Normalize(value) : fallback;
	}

	// strict 경로는 퇴화 basis를 기본 축으로 숨기지 않는다. 계산된 단위 벡터도 finite인지 확인한다.
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

	// 정적 packed 정점의 바이트 offset에서 위치/N/UV/T와 추가 채널을 복사한다. B는 normalize(cross(N,T))×handedness로 만든다.
	// N/T를 각각 정규화하지만 T를 N에 직교 투영하는 Gram-Schmidt 단계는 없다.
	bool_t MakeStaticVertex(
		const uint8_t* pSource,
		bool_t strictHandedness,
		bool_t preserveNativeParallelBasis,
		bool_t hasColor0,
		bool_t hasUV1,
		bool_t hasUV2,
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

		outVertex.vTexcoord1 = {};
		if (hasUV1)
		{
			memcpy(
				&outVertex.vTexcoord1,
				pSource + (hasColor0 ? STRIDE_STATIC_COLOR0 : STRIDE_STATIC),
				sizeof(float2_t));
			if (!isfinite(outVertex.vTexcoord1.x) || !isfinite(outVertex.vTexcoord1.y))
				return false;
		}

		outVertex.vTexcoord2 = {};
		if (hasUV2)
		{
			memcpy(
				&outVertex.vTexcoord2,
				pSource + (hasColor0 ? STRIDE_STATIC_COLOR0 : STRIDE_STATIC) + sizeof(float2_t),
				sizeof(float2_t));
			if (!isfinite(outVertex.vTexcoord2.x) || !isfinite(outVertex.vTexcoord2.y))
				return false;
		}

		vector_t normal{};
		vector_t tangent{};
		vector_t unitBinormal{};
		if (strictHandedness)
		{
			if (!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vNormal), normal) ||
				!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vTangent), tangent))
				return false;
			const vector_t cross = XMVector3Cross(normal, tangent);
			if (!TryNormalizeGeometryBasis(cross, unitBinormal))
			{
				// native packed N/T가 정확히 평행하면 bitangent가 없다. 명시된 보존 flag에서만 0을 유지한다.
				// 임의 축을 만들지 않으며 flag가 없거나 단지 거의 평행한 데이터는 실패한다.
				if (!preserveNativeParallelBasis || XMVectorGetX(XMVector3LengthSq(cross)) != 0.f)
					return false;
				unitBinormal = XMVectorZero();
			}
		}
		else
		{
			normal = SafeNormalize3(XMLoadFloat3(&outVertex.vNormal), XMVectorSet(0.f, 1.f, 0.f, 0.f));
			tangent = SafeNormalize3(XMLoadFloat3(&outVertex.vTangent), XMVectorSet(1.f, 0.f, 0.f, 0.f));
			unitBinormal = SafeNormalize3(XMVector3Cross(normal, tangent), XMVectorSet(0.f, 0.f, 1.f, 0.f));
		}
		const vector_t binormal = XMVectorScale(unitBinormal, handedness);

		XMStoreFloat3(&outVertex.vNormal, normal);
		XMStoreFloat3(&outVertex.vTangent, tangent);
		XMStoreFloat3(&outVertex.vBinormal, binormal);
		return true;
	}

	// skin 정점은 네 본 index와 비음수 weight를 읽고 합이 작으면 0번 본 하나로 대체한다.
	// 개별 weight의 finite는 검사하지만 합의 overflow를 별도 거절하지 않는다. strict 버전만 저장된 handedness를 소비한다.
	bool_t MakeSkinnedVertex(
		const uint8_t* pSource, uint32_t boneCount, bool_t strictHandedness, VTXANIMMESH& outVertex)
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

		vector_t normal{}, tangent{}, binormal{};
		if (strictHandedness)
		{
			f32_t handedness = 0.f;
			memcpy(&handedness, pSource + STRIDE_SKINNED, sizeof(handedness));
			if (!IsValidTangentHandedness(handedness) ||
				!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vNormal), normal) ||
				!TryNormalizeGeometryBasis(XMLoadFloat3(&outVertex.vTangent), tangent) ||
				!TryNormalizeGeometryBasis(XMVector3Cross(normal, tangent), binormal))
				return false;
			binormal = XMVectorScale(binormal, handedness);
		}
		else
		{
			normal = SafeNormalize3(XMLoadFloat3(&outVertex.vNormal), XMVectorSet(0.f, 1.f, 0.f, 0.f));
			tangent = SafeNormalize3(XMLoadFloat3(&outVertex.vTangent), XMVectorSet(1.f, 0.f, 0.f, 0.f));
			binormal = SafeNormalize3(XMVector3Cross(normal, tangent), XMVectorSet(0.f, 0.f, 1.f, 0.f));
		}
		XMStoreFloat3(&outVertex.vNormal, normal);
		XMStoreFloat3(&outVertex.vTangent, tangent);
		XMStoreFloat3(&outVertex.vBinormal, binormal);
		return true;
	}
}

// 파일 버퍼를 지역에서 소유하고 ReadMemory가 끝날 때까지 유지한다. 파일 읽기 실패는 기존 outMesh를 건드리지 않는다.
bool_t CWMeshReader::Read(
	const filesystem::path& meshPath, W_MESH_READ_RESULT& outMesh, MODEL_DECODE_REPORT& outReport) const
{
	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(meshPath, bytes))
	{
		outReport.error = "Could not open the binary mesh file.";
		return false;
	}
	return ReadMemory(bytes.data(), bytes.size(), outMesh, outReport);
}

// outMesh를 먼저 비우고 버전별 header→배열 범위→추가 metadata→정점/index 순으로 검사한다.
// 성공 전에도 bones/metadata/완료한 submesh와 report 통계가 갱신되므로 실패 결과는 호출자가 폐기한다.
bool_t CWMeshReader::ReadMemory(
	const uint8_t* pData, size_t dataSize, W_MESH_READ_RESULT& outMesh, MODEL_DECODE_REPORT& outReport) const
{
	outMesh = {};
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WMSH section is empty or truncated.";
		return false;
	}

	try
	{
		// mesh는 선언 contentSize와 실제 남은 바이트가 정확히 같아야 한다. header 승인 뒤 version별 layout을 선택한다.
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) || WINT_VERSION_MAJOR != fileHeader.versionMajor ||
			fileHeader.versionMinor > WINT_SKINNED_BASIS_UV_VERSION_MINOR || 0 != fileHeader.flags ||
			fileHeader.contentSize != fileReader.Remaining())
		{
			outReport.error = "Invalid WINT mesh file header.";
			return false;
		}

		// 정적 geometry minor 1/2/4, skin UV 3/6, signed basis 5/6의 layout을 분리한다. minor 0만 legacy 조합을 사용한다.
		const bool_t geometryContract = (WINT_GEOMETRY_VERSION_MINOR <= fileHeader.versionMinor &&
										 fileHeader.versionMinor <= WINT_UV1_VERSION_MINOR) ||
			fileHeader.versionMinor == WINT_STATIC_UV2_VERSION_MINOR;
		const bool_t skinnedBasisUVContract = fileHeader.versionMinor == WINT_SKINNED_BASIS_UV_VERSION_MINOR;
		const bool_t skinnedUVContract =
			skinnedBasisUVContract || fileHeader.versionMinor == WINT_SKINNED_UV_VERSION_MINOR;
		const bool_t skinnedBasisContract =
			skinnedBasisUVContract || fileHeader.versionMinor == WINT_SKINNED_BASIS_VERSION_MINOR;
		const uint8_t* pContent = fileReader.Peek();
		CBinaryReader reader(pContent, fileHeader.contentSize);
		const MESH_META_HEADER meshHeader = reader.Read<MESH_META_HEADER>();
		const bool_t skinned = 0 != (meshHeader.vertexFormatFlags & VF_BONE_WEIGHT);
		const bool_t hasColor0 = 0 != (meshHeader.vertexFormatFlags & VF_COLOR0);
		const bool_t hasUV1 = 0 != (meshHeader.vertexFormatFlags & VF_TEXCOORD1);
		const bool_t hasUV2 = 0 != (meshHeader.vertexFormatFlags & VF_TEXCOORD2);
		const uint32_t expectedStride = (hasColor0 ? STRIDE_STATIC_COLOR0 : STRIDE_STATIC) +
			(hasUV1 ? sizeof(float2_t) : 0) + (hasUV2 ? sizeof(float2_t) : 0);
		// version별 허용 채널 bit와 stride를 함께 묶는다. UV2만 단독 존재하거나 정적/skin layout이 섞이는 조합을 거절한다.
		const bool_t versionedFlagsValid = skinnedBasisUVContract
			? (skinned && hasUV1 &&
			   (meshHeader.vertexFormatFlags & (VF_STATIC_BASE | VF_TANGENT_HANDEDNESS)) ==
				   (VF_STATIC_BASE | VF_TANGENT_HANDEDNESS) &&
			   0 ==
				   (meshHeader.vertexFormatFlags &
					~(VF_STATIC_BASE | VF_BONE_WEIGHT | VF_TANGENT_HANDEDNESS | VF_TEXCOORD1 | VF_TEXCOORD2)))
			: skinnedBasisContract
			? (meshHeader.vertexFormatFlags == (VF_STATIC_BASE | VF_BONE_WEIGHT | VF_TANGENT_HANDEDNESS))
			: skinnedUVContract
			? (skinned && hasUV1 && (meshHeader.vertexFormatFlags & VF_STATIC_BASE) == VF_STATIC_BASE &&
			   0 ==
				   (meshHeader.vertexFormatFlags &
					~(VF_STATIC_BASE | VF_BONE_WEIGHT | VF_TEXCOORD1 | VF_TEXCOORD2)))
			: geometryContract
			? (!skinned && (hasUV1 == (fileHeader.versionMinor >= WINT_UV1_VERSION_MINOR)) &&
			   (hasUV2 == (fileHeader.versionMinor == WINT_STATIC_UV2_VERSION_MINOR)) &&
			   (meshHeader.vertexFormatFlags & VF_STATIC_BASE) == VF_STATIC_BASE &&
			   0 != (meshHeader.vertexFormatFlags & VF_TANGENT_HANDEDNESS) &&
			   0 ==
				   (meshHeader.vertexFormatFlags &
					~(VF_STATIC_BASE | VF_TANGENT_HANDEDNESS | VF_COLOR0 | VF_TEXCOORD1 | VF_TEXCOORD2)))
			: ((meshHeader.vertexFormatFlags & VF_STATIC_BASE) == VF_STATIC_BASE &&
			   0 == (meshHeader.vertexFormatFlags & ~(VF_STATIC_BASE | VF_BONE_WEIGHT)));
		const bool_t reservedValid = (!geometryContract && !skinnedUVContract && !skinnedBasisContract) ||
			(0 == meshHeader.reserved[0] && 0 == meshHeader.reserved[1] && 0 == meshHeader.reserved[2]);
		if (!HasMagic(meshHeader.magic, WMESH_MAGIC) || 0 == meshHeader.submeshCount ||
			meshHeader.submeshCount > MAX_SUBMESHES || meshHeader.boneCount > MAX_BONES ||
			meshHeader.totalVertexCount > MAX_VERTICES ||
			(meshHeader.indexStride != 2 && meshHeader.indexStride != 4) || meshHeader.hasBounding > 1 ||
			!versionedFlagsValid || !reservedValid ||
			(geometryContract && (0 == meshHeader.hasBounding || meshHeader.vertexStride != expectedStride)) ||
			(skinned &&
			 (0 == meshHeader.boneCount ||
			  meshHeader.vertexStride != (skinnedBasisContract ? STRIDE_SKINNED_BASIS : STRIDE_SKINNED))) ||
			(!geometryContract && !skinned && meshHeader.vertexStride != STRIDE_STATIC))
		{
			outReport.error = "Invalid WMSH version, flags, stride, or bone metadata.";
			return false;
		}

		vector<SUBMESH_DESC> submeshes(meshHeader.submeshCount);
		reader.ReadBytes(submeshes.data(), sizeof(SUBMESH_DESC) * submeshes.size());
		// versioned submesh offset은 앞 payload 끝에서 연속이어야 하며 합계도 header count와 맞춰야 한다. offset 단위는 정점 수가 아닌 바이트다.
		if (geometryContract || skinnedUVContract || skinnedBasisContract)
		{
			uint64_t expectedVertexOffset = {};
			uint64_t expectedIndexOffset = {};
			for (const SUBMESH_DESC& submesh : submeshes)
			{
				if (submesh.vertexOffset != expectedVertexOffset || submesh.indexOffset != expectedIndexOffset)
				{
					outReport.error = "Versioned WMSH submesh payload ranges are not contiguous.";
					return false;
				}
				expectedVertexOffset += static_cast<uint64_t>(submesh.vertexCount) * meshHeader.vertexStride;
				expectedIndexOffset += static_cast<uint64_t>(submesh.indexCount) * meshHeader.indexStride;
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

		// 큰 정점/index blob은 복사하지 않고 입력을 차용한다. 뒤 submesh 반복에서 범위 확인 후 소유 vector로 변환한다.
		size_t vertexBlobBytes = {};
		size_t indexBlobBytes = {};
		if (!CheckedByteCount(
				meshHeader.totalVertexCount, meshHeader.vertexStride, reader.Remaining(), vertexBlobBytes))
		{
			outReport.error = "Vertex block exceeds the WMSH payload.";
			return false;
		}
		const uint8_t* pVertexBlob = reader.Peek();
		reader.Skip(vertexBlobBytes);

		if (!CheckedByteCount(
				meshHeader.totalIndexCount, meshHeader.indexStride, reader.Remaining(), indexBlobBytes))
		{
			outReport.error = "Index block exceeds the WMSH payload.";
			return false;
		}
		const uint8_t* pIndexBlob = reader.Peek();
		reader.Skip(indexBlobBytes);

		// 본의 inverse-bind 16성분은 finite인지 검사한다. mesh/skeleton 본 이름 순서 일치는 상위 decoder가 담당한다.
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
		vector<uint32_t> skinnedUVFlags(meshHeader.submeshCount);
		vector<vector<float2_t>> skinnedUV1(meshHeader.submeshCount);
		vector<vector<float2_t>> skinnedUV2(meshHeader.submeshCount);
		// 저장 bounds는 정적 geometry 계약에서만 읽고 검증한다. 다른 버전에서는 크기만 확인해 건너뛴다.
		if (meshHeader.hasBounding)
		{
			size_t boundsBytes = {};
			if (!CheckedByteCount(
					meshHeader.submeshCount, sizeof(MESH_BOUNDS_V1), reader.Remaining(), boundsBytes))
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

		// 정적 geometry는 payload 바로 뒤에 정확히 metadata 하나가 있어야 한다. 저장된 bounds는 정점 변환 뒤 다시 대조한다.
		if (geometryContract)
		{
			const size_t geometryPayloadSize = static_cast<size_t>(reader.Peek() - pContent);
			if (reader.Remaining() != sizeof(MESH_GEOMETRY_METADATA_V1))
			{
				outReport.error = "WMSH geometry metadata is missing, truncated, or has trailing bytes.";
				return false;
			}
			const MESH_GEOMETRY_METADATA_V1 metadata = reader.Read<MESH_GEOMETRY_METADATA_V1>();
			if (!ValidateGeometryMetadata(
					pContent,
					geometryPayloadSize,
					meshHeader.vertexFormatFlags,
					fileHeader.versionMinor,
					metadata,
					outMesh.geometryMetadata,
					outReport.error))
				return false;
		}
		// skin 추가 UV는 WUVS payload SHA와 submesh별 정점 수/channel mask를 확인한다. UV1 없는 UV2는 거절한다.
		else if (skinnedUVContract)
		{
			const MESH_SKINNED_UV_HEADER uvHeader = reader.Read<MESH_SKINNED_UV_HEADER>();
			array<uint8_t, SHA256_SIZE> digest{};
			if (!HasMagic(uvHeader.magic, "WUVS") || uvHeader.version != 1u ||
				uvHeader.submeshCount != meshHeader.submeshCount ||
				uvHeader.payloadSize != reader.Remaining() ||
				!ComputeSha256(reader.Peek(), reader.Remaining(), digest) ||
				0 != memcmp(digest.data(), uvHeader.payloadSha256, SHA256_SIZE))
			{
				outReport.error = "WMSH skinned UV header, size, or SHA-256 is invalid.";
				return false;
			}
			uint32_t aggregateFlags = 0u;
			for (uint32_t i = 0; i < meshHeader.submeshCount; ++i)
			{
				const uint32_t vertexCount = reader.Read<uint32_t>();
				const uint32_t flags = reader.Read<uint32_t>();
				if (vertexCount != submeshes[i].vertexCount || 0 != (flags & ~(VF_TEXCOORD1 | VF_TEXCOORD2)) ||
					(0 != (flags & VF_TEXCOORD2) && 0 == (flags & VF_TEXCOORD1)))
				{
					outReport.error = "WMSH skinned UV submesh count or channel mask is invalid.";
					return false;
				}
				skinnedUVFlags[i] = flags;
				aggregateFlags |= flags;
				for (const uint32_t channel : {VF_TEXCOORD1, VF_TEXCOORD2})
				{
					if (0 == (flags & channel))
						continue;
					size_t uvBytes = 0u;
					if (!CheckedByteCount(vertexCount, sizeof(float2_t), reader.Remaining(), uvBytes))
					{
						outReport.error = "WMSH skinned UV channel is truncated.";
						return false;
					}
					auto& values = channel == VF_TEXCOORD1 ? skinnedUV1[i] : skinnedUV2[i];
					values.resize(vertexCount);
					reader.ReadBytes(values.data(), uvBytes);
					for (const float2_t& uv : values)
						if (!isfinite(uv.x) || !isfinite(uv.y))
						{
							outReport.error = "WMSH skinned UV channel is non-finite.";
							return false;
						}
				}
			}
			if (reader.Remaining() != 0u ||
				aggregateFlags != (meshHeader.vertexFormatFlags & (VF_TEXCOORD1 | VF_TEXCOORD2)))
			{
				outReport.error = "WMSH skinned UV masks disagree with the complete payload.";
				return false;
			}
		}
		else if (0 != reader.Remaining())
		{
			outReport.error = "Legacy WMSH contains unsupported trailing payload.";
			return false;
		}

		// 각 submesh를 완성한 뒤 결과 vector에 추가한다. index는 submesh 내부 정점 기준이며 runtime 저장은 모두 uint32_t로 통일한다.
		outMesh.skinned = skinned;
		outMesh.fileVersionMinor = fileHeader.versionMinor;
		outMesh.meshes.reserve(submeshes.size());
		for (uint32_t submeshIndex = 0; submeshIndex < submeshes.size(); ++submeshIndex)
		{
			const SUBMESH_DESC& sourceMesh = submeshes[submeshIndex];
			if (sourceMesh.materialIndex >= MAX_MATERIALS)
			{
				outReport.error = "A submesh material index exceeds the supported range.";
				return false;
			}
			size_t sourceVertexBytes = {};
			size_t sourceIndexBytes = {};
			if (!CheckedByteCount(
					sourceMesh.vertexCount, meshHeader.vertexStride, vertexBlobBytes, sourceVertexBytes) ||
				sourceMesh.vertexOffset > vertexBlobBytes ||
				sourceVertexBytes > vertexBlobBytes - sourceMesh.vertexOffset ||
				!CheckedByteCount(
					sourceMesh.indexCount, meshHeader.indexStride, indexBlobBytes, sourceIndexBytes) ||
				sourceMesh.indexOffset > indexBlobBytes ||
				sourceIndexBytes > indexBlobBytes - sourceMesh.indexOffset)
			{
				outReport.error = "A submesh points outside its vertex or index block.";
				return false;
			}
			if ((geometryContract || skinnedUVContract || skinnedBasisContract) &&
				(0 == sourceMesh.vertexCount || 0 == sourceMesh.indexCount || 0 != sourceMesh.indexCount % 3))
			{
				outReport.error = "A versioned WMSH submesh is not an indexed triangle list.";
				return false;
			}

			MODEL_MESH_DATA mesh{};
			mesh.name = ReadFixedName(sourceMesh.name, sizeof(sourceMesh.name));
			mesh.materialIndex = sourceMesh.materialIndex;
			mesh.vertexKind = skinned ? MODEL_VERTEX_KIND::SKINNED : MODEL_VERTEX_KIND::STATIC;
			mesh.hasColor0 = geometryContract && hasColor0;
			mesh.hasTexcoord1 = (geometryContract && hasUV1) ||
				(skinnedUVContract && 0 != (skinnedUVFlags[submeshIndex] & VF_TEXCOORD1));
			mesh.hasTexcoord2 = (geometryContract && hasUV2) ||
				(skinnedUVContract && 0 != (skinnedUVFlags[submeshIndex] & VF_TEXCOORD2));
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
							skinnedBasisContract,
							mesh.skinnedVertices[i]))
					{
						outReport.error = skinnedBasisContract
							? "A signed-basis WMSH skinned vertex contains invalid bone or tangent-basis data."
							: "A skinned vertex contains invalid bone data.";
						return false;
					}
					if (mesh.hasTexcoord1)
						mesh.skinnedVertices[i].vTexcoord1 = skinnedUV1[submeshIndex][i];
					if (mesh.hasTexcoord2)
						mesh.skinnedVertices[i].vTexcoord2 = skinnedUV2[submeshIndex][i];
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
					f32_t tangentHandedness = {1.f};
					uint32_t color0Rgba8 = {};
					if (!MakeStaticVertex(
							pVertices + static_cast<size_t>(i) * meshHeader.vertexStride,
							geometryContract,
							geometryContract &&
								0 !=
									(outMesh.geometryMetadata.evidenceFlags &
									 MGEF_NATIVE_PARALLEL_BASIS_PRESERVED),
							hasColor0,
							hasUV1,
							hasUV2,
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

			// 16/32비트 저장 index를 공통 uint32로 복사하고 현재 submesh의 정점 범위를 벗어나면 실패한다.
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

			// 완료한 submesh만 통계에 합산한다. 다음 submesh 실패 시 이 누적값을 되돌리지 않는다.
			outMesh.materialCount = (max)(outMesh.materialCount, sourceMesh.materialIndex + 1);
			outReport.vertexCount += sourceMesh.vertexCount;
			outReport.indexCount += sourceMesh.indexCount;
			outMesh.meshes.push_back(move(mesh));
		}

		return true;
	}
	// 표준 예외를 error로 변환하지만 outMesh/report를 초기화하지 않는다. 명시적 false와 함께 바깥 staged decode에서 격리한다.
	catch (const exception& exception)
	{
		outReport.error = exception.what();
		return false;
	}
}
