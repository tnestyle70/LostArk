// WModelDecoder가 사용하는 분리 WANM/내장 section 해석 진입점을 구현한다.
#include "WAnimationReader.h"

// 파일 bytes를 소유 배열로 읽고 입력 범위를 확인하는 차용 커서를 사용한다.
#include "BinaryAsset/BinaryReader.h"
// 저장된 header·채널·키·event·trailer의 메모리 형식과 개수 상한을 사용한다.
#include "WFormatTypes.h"

// 시간·벡터·quaternion 성분의 유한성을 검사한다.
#include <cmath>
// 4-byte magic 비교와 정렬에 의존하지 않는 key 값 복사에 사용한다.
#include <cstring>
// key/event byte 수의 곱셈 전에 size_t 상한을 확인한다.
#include <limits>
// 차용한 골격의 본 hash를 채널이 사용할 index에 대응시킨다.
#include <unordered_map>
// 한 본에 여러 채널이 선언되는 입력을 거절한다.
#include <unordered_set>

using namespace Engine::WintersFormat;

namespace
{
	// 읽기가 끝난 고정 header의 첫 4바이트를 비교한다. 버퍼 크기 검사는 이 helper의 책임이 아니다.
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	// offset은 key block 시작 기준 바이트 수다. 곱셈 overflow와 뺄셈 전 범위를 검사해 구간의 끝을 확인한다.
	template <typename T>
	bool_t SpanFits(uint32_t offset, uint32_t count, size_t blockSize)
	{
		if (0 != count && sizeof(T) > (numeric_limits<size_t>::max)() / count)
			return false;
		const size_t bytes = sizeof(T) * static_cast<size_t>(count);
		return offset <= blockSize && bytes <= blockSize - offset;
	}

	// tick 시간은 0 이상, duration+0.001 이내의 비감소 순서다. 같은 시각의 중복 key를 거절하는 조건은 아니다.
	bool_t IsValidTime(f32_t time, f32_t duration, f32_t previous)
	{
		return isfinite(time) && time >= 0.f && time <= duration + 1e-3f && time >= previous;
	}

	// 위치/scale key를 값으로 복사한다. span 검사 후 resize하므로 중간 값 실패는 일부 outKeys를 남기며 호출자의 지역 channel이 폐기한다.
	bool_t ReadVectorKeys(
		const uint8_t* pKeyBlock,
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
			if (!IsValidTime(source.timeTicks, duration, previous) || !isfinite(source.x) ||
				!isfinite(source.y) || !isfinite(source.z))
				return false;

			outKeys[i].timeTicks = source.timeTicks;
			outKeys[i].value = float3_t(source.x, source.y, source.z);
			previous = source.timeTicks;
		}
		return true;
	}

	// 회전 key는 finite와 길이 하한을 확인한 뒤 단위 quaternion으로 정규화한다. source quaternion의 길이를 그대로 보존하지 않는다.
	bool_t ReadQuaternionKeys(
		const uint8_t* pKeyBlock,
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
			memcpy(
				&source, pKeyBlock + offset + static_cast<size_t>(i) * sizeof(QUATERNION_KEY), sizeof(source));
			if (!IsValidTime(source.timeTicks, duration, previous) || !isfinite(source.x) ||
				!isfinite(source.y) || !isfinite(source.z) || !isfinite(source.w))
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

// 분리 WANM 파일명을 clip 이름으로 사용한다. bytes는 ReadMemory가 소유 vector로 복사할 때까지만 살아 있으면 된다.
bool_t CWAnimationReader::Read(
	const filesystem::path& animationPath,
	const MODEL_SKELETON_DATA& skeleton,
	MODEL_ANIMATION_DATA& outAnimation,
	MODEL_DECODE_REPORT& outReport) const
{
	vector<uint8_t> bytes;
	if (!CBinaryReader::LoadFile(animationPath, bytes))
	{
		outReport.error = "Could not open a declared WANM file: " + animationPath.string();
		return false;
	}
	return ReadMemory(
		bytes.data(), bytes.size(), animationPath.stem().string(), skeleton, outAnimation, outReport);
}

// WModelDecoder가 제공한 골격에 animation 채널을 연결한다. outAnimation은 초기화하며 report 전체는 초기화하지 않는다.
bool_t CWAnimationReader::ReadMemory(
	const uint8_t* pData,
	size_t dataSize,
	const string& animationName,
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
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WANM section is empty or truncated.";
		return false;
	}

	try
	{
		// 선언 contentSize가 입력보다 크지 않은지만 검사한다. 외부 입력의 남는 바이트나 minor version을 여기서 모두 거절하지는 않는다.
		CBinaryReader fileReader(pData, dataSize);
		const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
		if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) || 1 != fileHeader.versionMajor ||
			0 != fileHeader.flags || fileHeader.contentSize > fileReader.Remaining())
		{
			outReport.error = "Invalid WINT animation file header: " + animationName;
			return false;
		}

		CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
		const ANIMATION_META_HEADER animationHeader = reader.Read<ANIMATION_META_HEADER>();
		if (!HasMagic(animationHeader.magic, WANIM_MAGIC) ||
			animationHeader.channelCount > MAX_ANIMATION_CHANNELS ||
			animationHeader.totalKeyCount > MAX_ANIMATION_KEYS ||
			animationHeader.eventCount > MAX_ANIMATION_EVENTS || !isfinite(animationHeader.durationTicks) ||
			animationHeader.durationTicks <= 0.f || !isfinite(animationHeader.ticksPerSecond) ||
			animationHeader.ticksPerSecond <= 0.f)
		{
			outReport.error = "Invalid WANM metadata: " + animationName;
			return false;
		}

		// 채널 표를 먼저 복사한다. key block의 offset은 파일 시작이 아닌 아래에서 확정하는 key 구간을 기준으로 해석한다.
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

		// event와 trailer의 고정 크기를 뒤에서 빼 나머지를 key block으로 본다. pKeyBlock은 입력을 차용하며 channel offset의 기준점이다.
		const size_t keyBlockSize = reader.Remaining() - eventBytes - sizeof(ANIMATION_TRAILER);
		const uint8_t* pKeyBlock = reader.Peek();
		reader.Skip(keyBlockSize);

		// 본 hash→index 조회표를 만들어 채널마다 전체 본 배열을 다시 찾지 않는다. 제공된 skeleton의 hash 중복 검증은 skeleton reader 경계다.
		unordered_map<uint64_t, int32_t> boneIndices;
		for (uint32_t i = 0; i < skeleton.bones.size(); ++i)
			boneIndices.emplace(skeleton.bones[i].nameHash, static_cast<int32_t>(i));

		outAnimation.name = animationName;
		outAnimation.durationTicks = animationHeader.durationTicks;
		outAnimation.ticksPerSecond = animationHeader.ticksPerSecond;
		outAnimation.defaultLoop = 0 != animationHeader.isLoop;
		outAnimation.channels.reserve(sourceChannels.size());

		// 채널은 본마다 최대 한 개이며 cachedBoneIndex가 있다면 hash와 대조한다. key span 간 겹침/모든 바이트 사용까지 검사하지는 않는다.
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
			if (!ReadVectorKeys(
					pKeyBlock,
					keyBlockSize,
					source.positionOffset,
					source.positionKeyCount,
					animationHeader.durationTicks,
					destination.positionKeys) ||
				!ReadQuaternionKeys(
					pKeyBlock,
					keyBlockSize,
					source.rotationOffset,
					source.rotationKeyCount,
					animationHeader.durationTicks,
					destination.rotationKeys) ||
				!ReadVectorKeys(
					pKeyBlock,
					keyBlockSize,
					source.scaleOffset,
					source.scaleKeyCount,
					animationHeader.durationTicks,
					destination.scaleKeys))
			{
				outReport.error = "WANM contains an invalid key span or key value.";
				return false;
			}

			decodedKeyCount +=
				static_cast<uint64_t>(source.positionKeyCount) + source.rotationKeyCount + source.scaleKeyCount;
			outAnimation.channels.push_back(move(destination));
		}

		// 각 채널이 선언한 위치·회전·크기 key 수의 합을 metadata와 대조한다. byte 구간의 중복 사용 검사는 별도다.
		if (decodedKeyCount != animationHeader.totalKeyCount)
		{
			outReport.error = "WANM total key count does not match its channel table.";
			return false;
		}

		// event 시간/float 인자를 읽지만 type의 의미나 event 시간 정렬은 검증하지 않는다. 현재 CAnimation::Initialize는 이 events를 재생 상태로 옮기지 않는다.
		outAnimation.events.resize(animationHeader.eventCount);
		for (uint32_t i = 0; i < animationHeader.eventCount; ++i)
		{
			const ANIMATION_EVENT source = reader.Read<ANIMATION_EVENT>();
			if (!isfinite(source.timeTicks) || source.timeTicks < 0.f ||
				source.timeTicks > animationHeader.durationTicks + 1e-3f || !isfinite(source.paramF32))
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

		// 골격 reader가 계산한 본 이름 hash 순서와 trailer를 대조한다. 성공한 clip만 report의 key 개수에 합산한다.
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
	// 명시적 false와 달리 이 catch에서는 누적한 animation 출력을 비운다. error 메시지는 호출자 report에 남는다.
	catch (const exception& exception)
	{
		outAnimation = {};
		outReport.error = exception.what();
		return false;
	}
}
