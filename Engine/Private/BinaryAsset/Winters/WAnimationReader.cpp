#include "WAnimationReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace Engine::WintersFormat;

namespace
{
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	template<typename T>
	bool_t SpanFits(uint32_t offset, uint32_t count, size_t blockSize)
	{
		if (0 != count && sizeof(T) > (numeric_limits<size_t>::max)() / count)
			return false;
		const size_t bytes = sizeof(T) * static_cast<size_t>(count);
		return offset <= blockSize && bytes <= blockSize - offset;
	}

	bool_t IsValidTime(f32_t time, f32_t duration, f32_t previous)
	{
		return isfinite(time) && time >= 0.f && time <= duration + 1e-3f && time >= previous;
	}

	bool_t ReadVectorKeys(const uint8_t* pKeyBlock,
		size_t keyBlockSize,
		uint32_t offset,
		uint32_t count,
		f32_t duration,
		vector<MODEL_VECTOR_KEY_DATA>& outKeys)
	{
		if (!SpanFits<VECTOR_KEY>(offset, count, keyBlockSize))
			return false;

		outKeys.resize(count);
		f32_t previous = -1.f;
		for (uint32_t i = 0; i < count; ++i)
		{
			VECTOR_KEY source{};
			memcpy(&source, pKeyBlock + offset + static_cast<size_t>(i) * sizeof(VECTOR_KEY), sizeof(source));
			if (!IsValidTime(source.timeTicks, duration, previous) ||
				!isfinite(source.x) || !isfinite(source.y) || !isfinite(source.z))
				return false;

			outKeys[i].timeTicks = source.timeTicks;
			outKeys[i].value = float3_t(source.x, source.y, source.z);
			previous = source.timeTicks;
		}
		return true;
	}

	bool_t ReadQuaternionKeys(const uint8_t* pKeyBlock,
		size_t keyBlockSize,
		uint32_t offset,
		uint32_t count,
		f32_t duration,
		vector<MODEL_QUAT_KEY_DATA>& outKeys)
	{
		if (!SpanFits<QUATERNION_KEY>(offset, count, keyBlockSize))
			return false;

		outKeys.resize(count);
		f32_t previous = -1.f;
		for (uint32_t i = 0; i < count; ++i)
		{
			QUATERNION_KEY source{};
			memcpy(&source, pKeyBlock + offset + static_cast<size_t>(i) * sizeof(QUATERNION_KEY), sizeof(source));
			if (!IsValidTime(source.timeTicks, duration, previous) ||
				!isfinite(source.x) || !isfinite(source.y) ||
				!isfinite(source.z) || !isfinite(source.w))
				return false;

			vector_t quaternion = XMVectorSet(source.x, source.y, source.z, source.w);
			const f32_t lengthSq = XMVectorGetX(XMVector4LengthSq(quaternion));
			if (!isfinite(lengthSq) || lengthSq <= 1e-12f)
				return false;
			quaternion = XMQuaternionNormalize(quaternion);

			outKeys[i].timeTicks = source.timeTicks;
			XMStoreFloat4(&outKeys[i].value, quaternion);
			previous = source.timeTicks;
		}
		return true;
	}
}

bool_t CWAnimationReader::Read(const filesystem::path& animationPath,
	const MODEL_SKELETON_DATA& skeleton,
	MODEL_ANIMATION_DATA& outAnimation,
	MODEL_DECODE_REPORT& outReport) const
{
	outAnimation = {};
	if (skeleton.bones.empty())
	{
		outReport.error = "WANM requires a decoded skeleton.";
		return false;
	}

	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(animationPath, bytes))
	{
		outReport.error = "Could not open a declared WANM file: " + animationPath.string();
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
			outReport.error = "Invalid WINT animation file header: " + animationPath.string();
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const ANIMATION_META_HEADER animationHeader = reader.Read<ANIMATION_META_HEADER>();
		if (!HasMagic(animationHeader.magic, WANIM_MAGIC) ||
			animationHeader.channelCount > MAX_ANIMATION_CHANNELS ||
			animationHeader.totalKeyCount > MAX_ANIMATION_KEYS ||
			animationHeader.eventCount > MAX_ANIMATION_EVENTS ||
			!isfinite(animationHeader.durationTicks) || animationHeader.durationTicks <= 0.f ||
			!isfinite(animationHeader.ticksPerSecond) || animationHeader.ticksPerSecond <= 0.f)
		{
			outReport.error = "Invalid WANM metadata: " + animationPath.string();
			return false;
		}

		vector<ANIMATION_CHANNEL> sourceChannels(animationHeader.channelCount);
		if (!sourceChannels.empty())
			reader.ReadBytes(sourceChannels.data(), sizeof(ANIMATION_CHANNEL) * sourceChannels.size());

		if (0 != animationHeader.eventCount &&
			sizeof(ANIMATION_EVENT) > (numeric_limits<size_t>::max)() / animationHeader.eventCount)
		{
			outReport.error = "WANM event byte count overflow.";
			return false;
		}
		const size_t eventBytes = sizeof(ANIMATION_EVENT) * static_cast<size_t>(animationHeader.eventCount);
		if (reader.Remaining() < eventBytes + sizeof(ANIMATION_TRAILER))
		{
			outReport.error = "WANM key, event, or trailer block is truncated.";
			return false;
		}

		const size_t keyBlockSize = reader.Remaining() - eventBytes - sizeof(ANIMATION_TRAILER);
		const uint8_t* pKeyBlock = reader.Peek();
		reader.Skip(keyBlockSize);

		unordered_map<uint64_t, int32_t> boneIndices;
		for (uint32_t i = 0; i < skeleton.bones.size(); ++i)
			boneIndices.emplace(skeleton.bones[i].nameHash, static_cast<int32_t>(i));

		outAnimation.name = animationPath.stem().string();
		outAnimation.durationTicks = animationHeader.durationTicks;
		outAnimation.ticksPerSecond = animationHeader.ticksPerSecond;
		outAnimation.defaultLoop = 0 != animationHeader.isLoop;
		outAnimation.channels.reserve(sourceChannels.size());

		unordered_set<uint64_t> channelBones;
		uint64_t decodedKeyCount = {};
		for (const ANIMATION_CHANNEL& source : sourceChannels)
		{
			const auto bone = boneIndices.find(source.boneNameHash);
			if (bone == boneIndices.end() || !channelBones.emplace(source.boneNameHash).second)
			{
				outReport.error = "WANM contains an unresolved or duplicate bone channel.";
				return false;
			}

			if (source.cachedBoneIndex >= 0 &&
				(source.cachedBoneIndex >= static_cast<int32_t>(skeleton.bones.size()) ||
				skeleton.bones[source.cachedBoneIndex].nameHash != source.boneNameHash))
			{
				outReport.error = "WANM cached bone index disagrees with its bone hash.";
				return false;
			}

			MODEL_ANIMATION_CHANNEL_DATA destination{};
			destination.boneNameHash = source.boneNameHash;
			destination.resolvedBoneIndex = bone->second;
			if (!ReadVectorKeys(pKeyBlock, keyBlockSize,
				source.positionOffset, source.positionKeyCount,
				animationHeader.durationTicks, destination.positionKeys) ||
				!ReadQuaternionKeys(pKeyBlock, keyBlockSize,
					source.rotationOffset, source.rotationKeyCount,
					animationHeader.durationTicks, destination.rotationKeys) ||
				!ReadVectorKeys(pKeyBlock, keyBlockSize,
					source.scaleOffset, source.scaleKeyCount,
					animationHeader.durationTicks, destination.scaleKeys))
			{
				outReport.error = "WANM contains an invalid key span or key value.";
				return false;
			}

			decodedKeyCount += static_cast<uint64_t>(source.positionKeyCount) +
				source.rotationKeyCount + source.scaleKeyCount;
			outAnimation.channels.push_back(move(destination));
		}

		if (decodedKeyCount != animationHeader.totalKeyCount)
		{
			outReport.error = "WANM total key count does not match its channel table.";
			return false;
		}

		outAnimation.events.resize(animationHeader.eventCount);
		for (uint32_t i = 0; i < animationHeader.eventCount; ++i)
		{
			const ANIMATION_EVENT source = reader.Read<ANIMATION_EVENT>();
			if (!isfinite(source.timeTicks) || source.timeTicks < 0.f ||
				source.timeTicks > animationHeader.durationTicks + 1e-3f ||
				!isfinite(source.paramF32))
			{
				outReport.error = "WANM contains an invalid animation event.";
				return false;
			}

			MODEL_ANIMATION_EVENT_DATA& destination = outAnimation.events[i];
			destination.timeTicks = source.timeTicks;
			destination.type = source.type;
			destination.skillId = source.skillId;
			destination.paramU32 = source.paramU32;
			destination.paramF32 = source.paramF32;
			destination.stringHash = source.stringHash;
		}

		const ANIMATION_TRAILER trailer = reader.Read<ANIMATION_TRAILER>();
		if (trailer.skeletonHash != skeleton.skeletonHash)
		{
			outReport.error = "WANM skeleton hash does not match the companion WSKL.";
			return false;
		}

		outAnimation.skeletonHash = trailer.skeletonHash;
		outReport.animationKeyCount += decodedKeyCount;
		return true;
	}
	catch (const exception& exception)
	{
		outAnimation = {};
		outReport.error = exception.what();
		return false;
	}
}
