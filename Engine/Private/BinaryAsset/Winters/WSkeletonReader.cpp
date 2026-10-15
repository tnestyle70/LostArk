// WModelDecoder가 사용할 골격·socket의 CPU 해석 진입점을 구현한다.
#include "WSkeletonReader.h"

// 파일 소유 byte 배열과 범위 검사 커서로 내장/분리 골격을 같은 경로에서 읽는다.
#include "BinaryAsset/BinaryReader.h"
// 저장된 본·root·socket 구조체와 magic·개수 상한을 사용한다.
#include "WFormatTypes.h"

// 변환 행렬의 모든 성분이 유한한지 검사한다.
#include <cmath>
// magic 비교와 고정 크기 행렬의 값 복사에 사용한다.
#include <cstring>
// 배열 count와 stride의 곱이 정수 범위를 넘는지 검사한다.
#include <limits>
// 본 이름 hash와 socket 이름 hash의 중복을 각각 검사한다.
#include <unordered_set>

using namespace Engine::WintersFormat;

namespace
{
	// 검증된 header 저장소에서 4바이트 magic을 비교한다. 문자열 끝을 찾는 함수가 아니다.
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	// 배열 count×stride가 overflow하지 않고 남은 payload 안에 드는지 검사한다. 실패하면 outBytes는 바꾸지 않는다.
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

	// NUL 또는 고정 폭 끝까지 이름을 복사한다. hash를 문자열에서 다시 계산하지는 않는다.
	string ReadFixedName(const char* pName, size_t capacity)
	{
		size_t length = {};
		while (length < capacity && '\0' != pName[length])
			++length;
		return string(pName, length);
	}

	// 행렬 16성분의 finite 여부만 검사한다. 역행렬 존재나 정규직교/좌표계 일치까지 보장하지 않는다.
	bool_t IsFiniteMatrix(const float4x4_t& matrix)
	{
		const f32_t* pValues = reinterpret_cast<const f32_t*>(&matrix);
		for (uint32_t i = 0; i < 16; ++i)
		{
			if (!isfinite(pValues[i]))
				return false;
		}
		return true;
	}
}

// 분리 파일을 지역 bytes로 읽은 뒤 같은 ReadMemory 경로로 보낸다. 열기 실패 시 기존 골격은 보존한다.
bool_t CWSkeletonReader::Read(
	const filesystem::path& skeletonPath,
	MODEL_SKELETON_DATA& outSkeleton,
	MODEL_DECODE_REPORT& outReport) const
{
	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(skeletonPath, bytes))
	{
		outReport.error = "Could not open the companion WSKL file.";
		return false;
	}
	return ReadMemory(bytes.data(), bytes.size(), outSkeleton, outReport);
}

// 이 함수가 소유 vector로 값을 복사하므로 성공 뒤 입력 파일 버퍼를 유지할 필요가 없다. 중간 검증 실패의 부분 결과는 바깥 stagedAsset이 폐기한다.
bool_t CWSkeletonReader::ReadMemory(
	const uint8_t* pData,
	size_t dataSize,
	MODEL_SKELETON_DATA& outSkeleton,
	MODEL_DECODE_REPORT& outReport) const
{
	outSkeleton = {};
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WSKL section is empty or truncated.";
		return false;
	}

	try
	{
		// 외부 WINT header를 먼저 검사하고 선언 contentSize 안으로 차용 커서 범위를 좁힌다.
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) || 1 != fileHeader.versionMajor ||
			0 != fileHeader.flags || fileHeader.contentSize > fileReader.Remaining())
		{
			outReport.error = "Invalid WINT skeleton file header.";
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const SKELETON_META_HEADER skeletonHeader = reader.Read<SKELETON_META_HEADER>();
		if (!HasMagic(skeletonHeader.magic, WSKEL_MAGIC) || 0 == skeletonHeader.boneCount ||
			skeletonHeader.boneCount > MAX_BONES || skeletonHeader.socketCount > MAX_SOCKETS)
		{
			outReport.error = "Invalid WSKL metadata or unsupported bone count.";
			return false;
		}

		// 본 배열·global root·socket 배열이 순서대로 들어가는지 바이트 수부터 검사한다. 남은 trailing bytes를 끝에서 거절하는 검사는 없다.
		size_t boneBytes = {};
		size_t socketBytes = {};
		if (!CheckedByteCount(
				skeletonHeader.boneCount, sizeof(SKELETON_BONE_NODE), reader.Remaining(), boneBytes) ||
			boneBytes > reader.Remaining() || sizeof(GLOBAL_ROOT_MATRIX) > reader.Remaining() - boneBytes ||
			!CheckedByteCount(
				skeletonHeader.socketCount,
				sizeof(SOCKET_ENTRY),
				reader.Remaining() - boneBytes - sizeof(GLOBAL_ROOT_MATRIX),
				socketBytes))
		{
			outReport.error = "WSKL bone, root, or socket block exceeds the payload.";
			return false;
		}

		// 본 이름 hash는 0/중복을 거절하고 parentIndex는 -1 또는 앞선 본만 허용한다. 여러 root는 허용하며 childCount/firstChildIndex는 사용하지 않는다.
		outSkeleton.bones.resize(skeletonHeader.boneCount);
		unordered_set<uint64_t> boneHashes;
		uint32_t rootCount = {};
		for (uint32_t i = 0; i < skeletonHeader.boneCount; ++i)
		{
			const SKELETON_BONE_NODE source = reader.Read<SKELETON_BONE_NODE>();
			MODEL_BONE_DATA& destination = outSkeleton.bones[i];
			destination.nameHash = source.nameHash;
			destination.name = ReadFixedName(source.name, sizeof(source.name));
			destination.parentIndex = source.parentIndex;
			memcpy(&destination.restLocal, source.restTransform, sizeof(float4x4_t));
			XMStoreFloat4x4(&destination.inverseBind, XMMatrixIdentity());

			if (destination.name.empty() || 0 == destination.nameHash ||
				!boneHashes.emplace(destination.nameHash).second || !IsFiniteMatrix(destination.restLocal))
			{
				outReport.error = "WSKL contains an invalid or duplicate bone.";
				return false;
			}

			if (-1 == destination.parentIndex)
				++rootCount;
			else if (destination.parentIndex < 0 || destination.parentIndex >= static_cast<int32_t>(i))
			{
				outReport.error = "WSKL bone hierarchy is not parent-before-child.";
				return false;
			}
		}

		if (0 == rootCount)
		{
			outReport.error = "WSKL has no root bone.";
			return false;
		}

		// 저장된 global inverse root를 그대로 옮긴다. inverse-bind는 우선 identity이며 WModelDecoder가 WMSH 값을 연결한다.
		const GLOBAL_ROOT_MATRIX root = reader.Read<GLOBAL_ROOT_MATRIX>();
		memcpy(&outSkeleton.globalInverseRoot, root.globalInverseRoot, sizeof(float4x4_t));
		if (!IsFiniteMatrix(outSkeleton.globalInverseRoot))
		{
			outReport.error = "WSKL contains a non-finite global inverse root.";
			return false;
		}

		// socket 이름 hash와 parent 본 범위를 확인한다. localOffset은 parent 본 기준 행렬이며 실제 부착 월드 행렬은 소비자가 합성한다.
		outSkeleton.sockets.resize(skeletonHeader.socketCount);
		unordered_set<uint64_t> socketHashes;
		for (uint32_t i = 0; i < skeletonHeader.socketCount; ++i)
		{
			const SOCKET_ENTRY source = reader.Read<SOCKET_ENTRY>();
			MODEL_SOCKET_DATA& destination = outSkeleton.sockets[i];
			destination.nameHash = source.nameHash;
			destination.name = ReadFixedName(source.name, sizeof(source.name));
			destination.parentBoneIndex = source.parentBoneIndex;
			memcpy(&destination.localOffset, source.localOffset, sizeof(float4x4_t));
			if (destination.name.empty() || 0 == destination.nameHash ||
				!socketHashes.emplace(destination.nameHash).second || destination.parentBoneIndex < 0 ||
				destination.parentBoneIndex >= static_cast<int32_t>(outSkeleton.bones.size()) ||
				!IsFiniteMatrix(destination.localOffset))
			{
				outReport.error = "WSKL contains an invalid socket.";
				return false;
			}
		}

		// 순서대로 본 nameHash를 XOR/곱하여 animation 결합용 hash를 만든다. 부모 관계/rest 행렬까지 포함한 골격 내용 digest가 아니다.
		uint64_t skeletonHash = 0xcbf29ce484222325ull;
		for (const MODEL_BONE_DATA& bone : outSkeleton.bones)
		{
			skeletonHash ^= bone.nameHash;
			skeletonHash *= 0x100000001b3ull;
		}
		outSkeleton.skeletonHash = skeletonHash;
		return true;
	}
	// 표준 예외에는 부분 골격을 비운다. 위 명시적 false 반환은 같은 초기화를 수행하지 않는다.
	catch (const exception& exception)
	{
		outSkeleton = {};
		outReport.error = exception.what();
		return false;
	}
}
