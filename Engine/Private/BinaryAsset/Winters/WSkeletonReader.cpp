#include "WSkeletonReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <unordered_set>

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

bool_t CWSkeletonReader::Read(const filesystem::path& skeletonPath,
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

bool_t CWSkeletonReader::ReadMemory(const uint8_t* pData,
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
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
			1 != fileHeader.versionMajor || 0 != fileHeader.flags ||
			fileHeader.contentSize > fileReader.Remaining())
		{
			outReport.error = "Invalid WINT skeleton file header.";
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const SKELETON_META_HEADER skeletonHeader = reader.Read<SKELETON_META_HEADER>();
		if (!HasMagic(skeletonHeader.magic, WSKEL_MAGIC) ||
			0 == skeletonHeader.boneCount || skeletonHeader.boneCount > MAX_BONES ||
			skeletonHeader.socketCount > MAX_SOCKETS)
		{
			outReport.error = "Invalid WSKL metadata or unsupported bone count.";
			return false;
		}

		size_t boneBytes = {};
		size_t socketBytes = {};
		if (!CheckedByteCount(skeletonHeader.boneCount, sizeof(SKELETON_BONE_NODE), reader.Remaining(), boneBytes) ||
			boneBytes > reader.Remaining() || sizeof(GLOBAL_ROOT_MATRIX) > reader.Remaining() - boneBytes ||
			!CheckedByteCount(skeletonHeader.socketCount, sizeof(SOCKET_ENTRY),
				reader.Remaining() - boneBytes - sizeof(GLOBAL_ROOT_MATRIX), socketBytes))
		{
			outReport.error = "WSKL bone, root, or socket block exceeds the payload.";
			return false;
		}

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
				!boneHashes.emplace(destination.nameHash).second ||
				!IsFiniteMatrix(destination.restLocal))
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

		const GLOBAL_ROOT_MATRIX root = reader.Read<GLOBAL_ROOT_MATRIX>();
		memcpy(&outSkeleton.globalInverseRoot, root.globalInverseRoot, sizeof(float4x4_t));
		if (!IsFiniteMatrix(outSkeleton.globalInverseRoot))
		{
			outReport.error = "WSKL contains a non-finite global inverse root.";
			return false;
		}

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
				!socketHashes.emplace(destination.nameHash).second ||
				destination.parentBoneIndex < 0 ||
				destination.parentBoneIndex >= static_cast<int32_t>(outSkeleton.bones.size()) ||
				!IsFiniteMatrix(destination.localOffset))
			{
				outReport.error = "WSKL contains an invalid socket.";
				return false;
			}
		}

		uint64_t skeletonHash = 0xcbf29ce484222325ull;
		for (const MODEL_BONE_DATA& bone : outSkeleton.bones)
		{
			skeletonHash ^= bone.nameHash;
			skeletonHash *= 0x100000001b3ull;
		}
		outSkeleton.skeletonHash = skeletonHash;
		return true;
	}
	catch (const exception& exception)
	{
		outSkeleton = {};
		outReport.error = exception.what();
		return false;
	}
}
