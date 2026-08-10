#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class EFFECT_DISTRIBUTION_INTERPOLATION : uint8_t
{
	CONSTANT,
	LINEAR,
	CUBIC,
	END
};

enum class EFFECT_DISTRIBUTION_PARAMETER_BINDING : uint8_t
{
	NONE,
	ACTION_CUE,
	END
};

struct EFFECT_DISTRIBUTION_KEY_DESC final
{
	f32_t fTime = 0.f;
	float4_t vMinimum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vMaximum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vArriveTangentMinimum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vLeaveTangentMinimum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vArriveTangentMaximum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vLeaveTangentMaximum = { 0.f, 0.f, 0.f, 0.f };
	EFFECT_DISTRIBUTION_INTERPOLATION eInterpolation =
		EFFECT_DISTRIBUTION_INTERPOLATION::LINEAR;
};

struct EFFECT_DISTRIBUTION_DESC final
{
	std::string strPropertyPath;
	std::string strSourceClass;
	std::string strSourceObjectPath;
	std::string strParameterName;
	EFFECT_DISTRIBUTION_PARAMETER_BINDING eParameterBinding =
		EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
	uint32_t iComponentCount = 1u;
	uint32_t iOperation = 1u;
	uint32_t iRandomLockAxes = 0u;
	uint32_t iLookupTableChunkSize = 0u;
	uint32_t iLookupTableNumElements = 0u;
	f32_t fLookupTableTimeScale = 0.f;
	f32_t fLookupTableStartTime = 0.f;
	float4_t vDefaultMinimum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vDefaultMaximum = { 0.f, 0.f, 0.f, 0.f };
	std::vector<f32_t> LookupTable;
	std::vector<EFFECT_DISTRIBUTION_KEY_DESC> Keys;
};

class CEffectDistribution final
{
public:
	static float4_t Evaluate(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		f32_t fTime,
		f32_t fRandomUnit);
	static float4_t Evaluate(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		f32_t fTime,
		const float4_t& vRandomUnits);
	static bool_t Validate(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		std::string& strOutError);
};

NS_END
