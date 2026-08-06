#include "Effect_Distribution.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace
{
	using namespace Client;
	constexpr size_t COOKED_LOOKUP_RANGE_VALUE_COUNT = 2u;

	float4_t Add(const float4_t& A, const float4_t& B)
	{
		return { A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w };
	}

	float4_t Multiply(const float4_t& Value, const f32_t fScale)
	{
		return {
			Value.x * fScale, Value.y * fScale,
			Value.z * fScale, Value.w * fScale
		};
	}

	float4_t Lerp(const float4_t& A, const float4_t& B, const f32_t fRatio)
	{
		return Add(Multiply(A, 1.f - fRatio), Multiply(B, fRatio));
	}

	float4_t Hermite(
		const float4_t& Start,
		const float4_t& StartTangent,
		const float4_t& End,
		const float4_t& EndTangent,
		const f32_t fRatio,
		const f32_t fDuration)
	{
		const f32_t fRatio2 = fRatio * fRatio;
		const f32_t fRatio3 = fRatio2 * fRatio;
		const f32_t fH00 = 2.f * fRatio3 - 3.f * fRatio2 + 1.f;
		const f32_t fH10 = fRatio3 - 2.f * fRatio2 + fRatio;
		const f32_t fH01 = -2.f * fRatio3 + 3.f * fRatio2;
		const f32_t fH11 = fRatio3 - fRatio2;
		return Add(
			Add(Multiply(Start, fH00),
				Multiply(StartTangent, fH10 * fDuration)),
			Add(Multiply(End, fH01),
				Multiply(EndTangent, fH11 * fDuration)));
	}

	float4_t SelectRange(
		const float4_t& Minimum,
		const float4_t& Maximum,
		const uint32_t iOperation,
		const float4_t& RandomUnits)
	{
		if (2u == iOperation)
		{
			return {
				std::lerp(Minimum.x, Maximum.x, std::clamp(RandomUnits.x, 0.f, 1.f)),
				std::lerp(Minimum.y, Maximum.y, std::clamp(RandomUnits.y, 0.f, 1.f)),
				std::lerp(Minimum.z, Maximum.z, std::clamp(RandomUnits.z, 0.f, 1.f)),
				std::lerp(Minimum.w, Maximum.w, std::clamp(RandomUnits.w, 0.f, 1.f))
			};
		}
		if (3u == iOperation)
			return RandomUnits.x < 0.5f ? Minimum : Maximum;
		return Minimum;
	}

	size_t ResolveTableChunkSize(
		const EFFECT_DISTRIBUTION_DESC& Distribution)
	{
		if (0u != Distribution.iLookupTableChunkSize)
			return Distribution.iLookupTableChunkSize;
		return static_cast<size_t>(Distribution.iComponentCount) *
			(Distribution.iOperation >= 2u ? 2u : 1u);
	}

	float4_t ReadTableValue(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		const size_t iEntry,
		const bool_t bMaximum)
	{
		const size_t iComponentCount = Distribution.iComponentCount;
		const size_t iChunkSize = ResolveTableChunkSize(Distribution);
		const size_t iOffset = COOKED_LOOKUP_RANGE_VALUE_COUNT +
			iEntry * iChunkSize +
			(bMaximum && iChunkSize >= iComponentCount * 2u ?
				iComponentCount : 0u);
		float4_t Result{};
		f32_t* pOutput = &Result.x;
		for (size_t iComponent = 0u; iComponent < iComponentCount;
			++iComponent)
		{
			const size_t iValue = iOffset + iComponent;
			if (iValue < Distribution.LookupTable.size())
				pOutput[iComponent] = Distribution.LookupTable[iValue];
		}
		return Result;
	}

	bool_t IsFinite(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}
}

float4_t Client::CEffectDistribution::Evaluate(
	const EFFECT_DISTRIBUTION_DESC& Distribution,
	const f32_t fTime,
	const f32_t fRandomUnit)

{
	return Evaluate(Distribution, fTime,
		float4_t(fRandomUnit, fRandomUnit, fRandomUnit, fRandomUnit));
}

float4_t Client::CEffectDistribution::Evaluate(
	const EFFECT_DISTRIBUTION_DESC& Distribution,
	const f32_t fTime,
	const float4_t& vRandomUnits)
{
	if (!Distribution.LookupTable.empty())
	{
		const size_t iChunkSize = ResolveTableChunkSize(Distribution);
		const size_t iPayloadCount =
			Distribution.LookupTable.size() >= COOKED_LOOKUP_RANGE_VALUE_COUNT ?
			Distribution.LookupTable.size() - COOKED_LOOKUP_RANGE_VALUE_COUNT : 0u;
		const size_t iEntryCount = 0u == iChunkSize ? 0u :
			iPayloadCount / iChunkSize;
		if (iEntryCount > 0u)
		{
			const f32_t fLookup = Distribution.fLookupTableTimeScale > 0.f ?
				(fTime - Distribution.fLookupTableStartTime) *
				Distribution.fLookupTableTimeScale : 0.f;
			const f32_t fClamped = (std::max)(0.f,
				(std::min)(static_cast<f32_t>(iEntryCount - 1u), fLookup));
			const size_t iStart = static_cast<size_t>(std::floor(fClamped));
			const size_t iEnd = (std::min)(iStart + 1u, iEntryCount - 1u);
			const f32_t fRatio = fClamped - static_cast<f32_t>(iStart);
			const float4_t Minimum = Lerp(
				ReadTableValue(Distribution, iStart, false),
				ReadTableValue(Distribution, iEnd, false), fRatio);
			const float4_t Maximum = Lerp(
				ReadTableValue(Distribution, iStart, true),
				ReadTableValue(Distribution, iEnd, true), fRatio);
			return SelectRange(Minimum, Maximum,
				Distribution.iOperation, vRandomUnits);
		}
	}

	if (!Distribution.Keys.empty())
	{
		const auto& Keys = Distribution.Keys;
		if (fTime <= Keys.front().fTime)
			return SelectRange(Keys.front().vMinimum,
				Keys.front().vMaximum, Distribution.iOperation, vRandomUnits);
		if (fTime >= Keys.back().fTime)
			return SelectRange(Keys.back().vMinimum,
				Keys.back().vMaximum, Distribution.iOperation, vRandomUnits);
		for (size_t iKey = 0u; iKey + 1u < Keys.size(); ++iKey)
		{
			const EFFECT_DISTRIBUTION_KEY_DESC& Start = Keys[iKey];
			const EFFECT_DISTRIBUTION_KEY_DESC& End = Keys[iKey + 1u];
			if (fTime > End.fTime)
				continue;
			const f32_t fDuration = End.fTime - Start.fTime;
			const f32_t fRatio = fDuration <= 0.f ? 0.f :
				(fTime - Start.fTime) / fDuration;
			if (EFFECT_DISTRIBUTION_INTERPOLATION::CONSTANT ==
				Start.eInterpolation)
			{
				return SelectRange(Start.vMinimum, Start.vMaximum,
					Distribution.iOperation, vRandomUnits);
			}
			const float4_t Minimum =
				EFFECT_DISTRIBUTION_INTERPOLATION::CUBIC ==
				Start.eInterpolation ?
				Hermite(Start.vMinimum, Start.vLeaveTangentMinimum,
					End.vMinimum, End.vArriveTangentMinimum,
					fRatio, fDuration) :
				Lerp(Start.vMinimum, End.vMinimum, fRatio);
			const float4_t Maximum =
				EFFECT_DISTRIBUTION_INTERPOLATION::CUBIC ==
				Start.eInterpolation ?
				Hermite(Start.vMaximum, Start.vLeaveTangentMaximum,
					End.vMaximum, End.vArriveTangentMaximum,
					fRatio, fDuration) :
				Lerp(Start.vMaximum, End.vMaximum, fRatio);
			return SelectRange(Minimum, Maximum,
				Distribution.iOperation, vRandomUnits);
		}
	}
	return SelectRange(Distribution.vDefaultMinimum,
		Distribution.vDefaultMaximum, Distribution.iOperation, vRandomUnits);
}

bool_t Client::CEffectDistribution::Validate(
	const EFFECT_DISTRIBUTION_DESC& Distribution,
	std::string& strOutError)
{
	if (Distribution.strPropertyPath.empty() ||
		Distribution.strPropertyPath.size() > 256u ||
		Distribution.strSourceClass.size() > 128u ||
		Distribution.strSourceObjectPath.size() > 512u ||
		Distribution.iComponentCount < 1u ||
		Distribution.iComponentCount > 4u ||
		Distribution.iOperation > 3u ||
		Distribution.iRandomLockAxes > 4u ||
		Distribution.iLookupTableChunkSize > 32u ||
		Distribution.iLookupTableNumElements > 4u ||
		!std::isfinite(Distribution.fLookupTableTimeScale) ||
		!std::isfinite(Distribution.fLookupTableStartTime) ||
		!IsFinite(Distribution.vDefaultMinimum) ||
		!IsFinite(Distribution.vDefaultMaximum) ||
		Distribution.LookupTable.size() > 16384u ||
		Distribution.Keys.size() > 4096u)
	{
		strOutError = "Effect distribution metadata or size is invalid.";
		return false;
	}
	for (const f32_t fValue : Distribution.LookupTable)
	{
		if (!std::isfinite(fValue))
		{
			strOutError = "Effect distribution lookup table is not finite.";
			return false;
		}
	}
	if (!Distribution.LookupTable.empty())
	{
		const size_t iChunkSize = ResolveTableChunkSize(Distribution);
		const uint32_t iExpectedNumElements =
			Distribution.iOperation >= 2u ? 2u : 1u;
		const size_t iRequiredValues =
			static_cast<size_t>(Distribution.iComponentCount) *
			iExpectedNumElements;
		if (Distribution.LookupTable.size() <
				COOKED_LOOKUP_RANGE_VALUE_COUNT + iRequiredValues ||
			iChunkSize != iRequiredValues ||
			(0u != Distribution.iLookupTableNumElements &&
				Distribution.iLookupTableNumElements != iExpectedNumElements) ||
			0u != (Distribution.LookupTable.size() -
				COOKED_LOOKUP_RANGE_VALUE_COUNT) % iChunkSize)
		{
			strOutError =
				"Effect cooked distribution lookup payload shape is invalid.";
			return false;
		}
	}
	f32_t fPreviousTime = -FLT_MAX;
	for (const EFFECT_DISTRIBUTION_KEY_DESC& Key : Distribution.Keys)
	{
		if (!std::isfinite(Key.fTime) || Key.fTime < fPreviousTime ||
			!IsFinite(Key.vMinimum) || !IsFinite(Key.vMaximum) ||
			!IsFinite(Key.vArriveTangentMinimum) ||
			!IsFinite(Key.vLeaveTangentMinimum) ||
			!IsFinite(Key.vArriveTangentMaximum) ||
			!IsFinite(Key.vLeaveTangentMaximum) ||
			Key.eInterpolation >= EFFECT_DISTRIBUTION_INTERPOLATION::END)
		{
			strOutError = "Effect distribution curve key is invalid.";
			return false;
		}
		fPreviousTime = Key.fTime;
	}
	strOutError.clear();
	return true;
}
