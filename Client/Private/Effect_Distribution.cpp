#include "Effect_Distribution.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace
{
	using namespace Client;
	constexpr size_t COOKED_LOOKUP_RANGE_VALUE_COUNT = 2u;

	bool_t Canonical_ParameterName(const std::string& Name, std::string& Out)
	{
		if (Name.empty() || Name.size() > 128u) return false;
		Out = Name;
		bool_t bVisible = false;
		for (char& Character : Out)
		{
			const auto Value = static_cast<unsigned char>(Character);
			if (Value < 0x20u || Value == 0x7fu) return false;
			bVisible = bVisible || Value > 0x20u;
			if (Value >= 'A' && Value <= 'Z')
				Character = static_cast<char>(Value - 'A' + 'a');
		}
		return bVisible;
	}

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
	if (4u == Distribution.iOperation)
	{
		// Unbaked UDistributionVectorUniformRange selects Max or Min first,
		// then samples X/Y/Z within that branch; crossing the gap is invalid.
		if (Distribution.LookupTable.size() != 14u)
			return {};
		const size_t iHigh = COOKED_LOOKUP_RANGE_VALUE_COUNT +
			(vRandomUnits.w < 0.5f ? 0u : 6u);
		const auto& Values = Distribution.LookupTable;
		return {
			std::lerp(Values[iHigh], Values[iHigh + 3u],
				std::clamp(vRandomUnits.x, 0.f, 1.f)),
			std::lerp(Values[iHigh + 1u], Values[iHigh + 4u],
				std::clamp(vRandomUnits.y, 0.f, 1.f)),
			std::lerp(Values[iHigh + 2u], Values[iHigh + 5u],
				std::clamp(vRandomUnits.z, 0.f, 1.f)), 0.f
		};
	}
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
			// Constant distributions never consume the upper range. Keep the exact
			// lower interpolation and random operations unchanged.
			if (2u != Distribution.iOperation && 3u != Distribution.iOperation)
				return Minimum;
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
			// A key owns its exact timestamp, including a Constant segment jump.
			if (fTime >= End.fTime)
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
			// Constant distributions never consume the upper range. Keep the exact
			// lower interpolation and random operations unchanged.
			if (2u != Distribution.iOperation && 3u != Distribution.iOperation)
				return Minimum;
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

bool_t Client::CEffectDistribution::Validate_ParameterInputs(
	const std::span<const EFFECT_PARAMETER_INPUT> Inputs,
	std::string& strOutError)
{
	if (Inputs.size() > 64u)
	{ strOutError = "Too many WORLD particle parameter inputs."; return false; }
	std::vector<std::string> Names;
	Names.reserve(Inputs.size());
	for (const auto& Input : Inputs)
	{
		std::string Name;
		if (!Canonical_ParameterName(Input.strName, Name) ||
			Input.eKind >= EFFECT_PARAMETER_VALUE_KIND::END ||
			(Input.eKind == EFFECT_PARAMETER_VALUE_KIND::SCALAR &&
			 !std::isfinite(Input.fScalarValue)) ||
			(Input.eKind == EFFECT_PARAMETER_VALUE_KIND::VECTOR3 &&
			 (!std::isfinite(Input.vVectorValue.x) || !std::isfinite(Input.vVectorValue.y) ||
			  !std::isfinite(Input.vVectorValue.z))) ||
			std::find(Names.begin(), Names.end(), Name) != Names.end())
		{ strOutError = "WORLD particle parameter input name, type, value or uniqueness is invalid."; return false; }
		Names.push_back(std::move(Name));
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDistribution::Resolve_WorldParameter(
	const EFFECT_DISTRIBUTION_DESC& Distribution,
	const std::span<const EFFECT_PARAMETER_INPUT> Inputs,
	float4_t& OutValue,
	std::string& strOutError)
{
	if (Distribution.eParameterBinding != EFFECT_DISTRIBUTION_PARAMETER_BINDING::WORLD_SAMPLE)
	{ strOutError = "Distribution is not bound to a WORLD particle parameter."; return false; }
	if (!Validate(Distribution, strOutError) || !Validate_ParameterInputs(Inputs, strOutError))
		return false;
	std::string Name;
	(void)Canonical_ParameterName(Distribution.strParameterName, Name);
	const EFFECT_PARAMETER_INPUT* Found = nullptr;
	for (const auto& Input : Inputs)
	{
		std::string InputName;
		(void)Canonical_ParameterName(Input.strName, InputName);
		if (InputName == Name) { Found = &Input; break; }
	}
	const auto Kind = Distribution.iComponentCount == 1u ?
		EFFECT_PARAMETER_VALUE_KIND::SCALAR : EFFECT_PARAMETER_VALUE_KIND::VECTOR3;
	if (!Found || Found->eKind != Kind)
	{ strOutError = "WORLD particle parameter is missing or has the wrong type: " + Distribution.strParameterName; return false; }
	const auto& Mapping = *Distribution.ParameterMapping;
	float4_t Staged = Distribution.vDefaultMinimum;
	for (uint32_t Index = 0u; Index < Distribution.iComponentCount; ++Index)
	{
		const double Input = Kind == EFFECT_PARAMETER_VALUE_KIND::SCALAR ?
			Found->fScalarValue : (&Found->vVectorValue.x)[Index];
		double Value = Input;
		if (Mapping.Modes[Index] == EFFECT_PARAMETER_MODE::NORMAL)
		{
			const double Minimum = (&Mapping.vMinInput.x)[Index];
			const double Range = static_cast<double>((&Mapping.vMaxInput.x)[Index]) - Minimum;
			const double Ratio = Range == 0.0 ? 0.0 : std::clamp((Input - Minimum) / Range, 0.0, 1.0);
			Value = std::lerp(static_cast<double>((&Mapping.vMinOutput.x)[Index]),
				static_cast<double>((&Mapping.vMaxOutput.x)[Index]), Ratio);
		}
		if (!std::isfinite(Value) || std::abs(Value) > FLT_MAX)
		{ strOutError = "WORLD particle parameter result is not finite f32."; return false; }
		(&Staged.x)[Index] = static_cast<f32_t>(Value);
	}
	OutValue = Staged;
	strOutError.clear();
	return true;
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
		Distribution.iOperation > 4u ||
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
	const bool_t bWorldParameter = Distribution.eParameterBinding ==
		EFFECT_DISTRIBUTION_PARAMETER_BINDING::WORLD_SAMPLE;
	if (Distribution.ParameterMapping.has_value() != bWorldParameter)
	{ strOutError = "WORLD particle parameter mapping and binding disagree."; return false; }
	if (bWorldParameter)
	{
		std::string Name, SourceClass;
		const auto& Mapping = *Distribution.ParameterMapping;
		if (!Canonical_ParameterName(Distribution.strParameterName, Name) ||
			!Canonical_ParameterName(Distribution.strSourceClass, SourceClass) ||
			!((SourceClass == "distributionfloatparticleparameter" && Distribution.iComponentCount == 1u) ||
			  (SourceClass == "distributionvectorparticleparameter" && Distribution.iComponentCount == 3u)) ||
			Mapping.Modes.size() != Distribution.iComponentCount ||
			!IsFinite(Mapping.vMinInput) || !IsFinite(Mapping.vMaxInput) ||
			!IsFinite(Mapping.vMinOutput) || !IsFinite(Mapping.vMaxOutput) ||
			Distribution.iOperation != 1u || Distribution.iRandomLockAxes != 0u ||
			Distribution.iLookupTableChunkSize != 0u || Distribution.iLookupTableNumElements != 0u ||
			Distribution.fLookupTableTimeScale != 0.f || Distribution.fLookupTableStartTime != 0.f ||
			!Distribution.LookupTable.empty() || !Distribution.Keys.empty())
		{ strOutError = "WORLD particle parameter source class, mapping or constant payload is invalid."; return false; }
		for (uint32_t Index = 0u; Index < 4u; ++Index)
		{
			if ((&Distribution.vDefaultMinimum.x)[Index] != (&Distribution.vDefaultMaximum.x)[Index] ||
				(Index < Distribution.iComponentCount ? Mapping.Modes[Index] >= EFFECT_PARAMETER_MODE::END :
				 ((&Mapping.vMinInput.x)[Index] != 0.f || (&Mapping.vMaxInput.x)[Index] != 0.f ||
				  (&Mapping.vMinOutput.x)[Index] != 0.f || (&Mapping.vMaxOutput.x)[Index] != 0.f)))
			{ strOutError = "WORLD particle parameter mode, source constant or unused mapping component is invalid."; return false; }
		}
	}
	for (const f32_t fValue : Distribution.LookupTable)
	{
		if (!std::isfinite(fValue))
		{
			strOutError = "Effect distribution lookup table is not finite.";
			return false;
		}
	}
	if (4u == Distribution.iOperation)
	{
		if (Distribution.strSourceClass != "distributionvectoruniformrange" ||
			Distribution.strSourceObjectPath.empty() ||
			Distribution.iComponentCount != 3u ||
			Distribution.iRandomLockAxes != 0u ||
			Distribution.iLookupTableChunkSize != 12u ||
			Distribution.iLookupTableNumElements != 4u ||
			Distribution.fLookupTableTimeScale != 0.f ||
			Distribution.fLookupTableStartTime != 0.f ||
			Distribution.LookupTable.size() != 14u ||
			!Distribution.Keys.empty())
		{
			strOutError =
				"Effect unbaked vector random-range payload shape is invalid.";
			return false;
		}
	}
	else if (!Distribution.LookupTable.empty())
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
