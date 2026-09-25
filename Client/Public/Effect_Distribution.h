#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <optional>
#include <span>
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
	WORLD_SAMPLE,
	END
};

enum class EFFECT_PARAMETER_VALUE_KIND : uint8_t
{
	SCALAR,
	VECTOR3,
	END
};

struct EFFECT_PARAMETER_INPUT final
{
	std::string strName;
	EFFECT_PARAMETER_VALUE_KIND eKind = EFFECT_PARAMETER_VALUE_KIND::END;
	f32_t fScalarValue = 0.f;
	float3_t vVectorValue = { 0.f, 0.f, 0.f };
};

enum class EFFECT_PARAMETER_MODE : uint8_t
{
	DIRECT,
	NORMAL,
	END
};

struct EFFECT_PARAMETER_MAPPING_DESC final
{
	std::vector<EFFECT_PARAMETER_MODE> Modes;
	float4_t vMinInput = { 0.f, 0.f, 0.f, 0.f };
	float4_t vMaxInput = { 0.f, 0.f, 0.f, 0.f };
	float4_t vMinOutput = { 0.f, 0.f, 0.f, 0.f };
	float4_t vMaxOutput = { 0.f, 0.f, 0.f, 0.f };
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

struct EFFECT_SOURCE_ADMISSION_DESC final
{
	bool_t bAllowed = false;
	std::vector<std::string> Blockers;
};

struct EFFECT_DISTRIBUTION_DESC final
{
	std::string strPropertyPath;
	std::string strReferenceId;
	std::string strOccurrenceId;
	std::string strPayloadStatus;
	std::string strFidelity;
	EFFECT_SOURCE_ADMISSION_DESC ExecutionAdmission;
	std::string strSourceClass;
	std::string strSourceObjectPath;
	std::string strParameterName;
	EFFECT_DISTRIBUTION_PARAMETER_BINDING eParameterBinding =
		EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
	std::optional<EFFECT_PARAMETER_MAPPING_DESC> ParameterMapping;
	uint32_t iComponentCount = 1u;
	uint32_t iOperation = 1u;
	uint32_t iRandomLockAxes = 0u;
	uint32_t iLookupTableChunkSize = 0u;
	uint32_t iLookupTableNumElements = 0u;
	f32_t fLookupTableTimeScale = 0.f;
	f32_t fLookupTableStartTime = 0.f;
	float4_t vDefaultMinimum = { 0.f, 0.f, 0.f, 0.f };
	float4_t vDefaultMaximum = { 0.f, 0.f, 0.f, 0.f };
	// Operation 4 preserves unbaked DistributionVectorUniformRange:
	// two range scalars, then MaxHigh/MaxLow/MinHigh/MinLow XYZ vectors.
	std::vector<f32_t> LookupTable;
	std::vector<EFFECT_DISTRIBUTION_KEY_DESC> Keys;
};

class CEffectDistribution final
{
public:
	static bool_t Validate_ParameterInputs(
		std::span<const EFFECT_PARAMETER_INPUT> Inputs,
		std::string& strOutError);
	// Inputs are raw values already sampled at the owning WORLD occurrence time.
	// Missing or mismatched input fails without changing OutValue.
	static bool_t Resolve_WorldParameter(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		std::span<const EFFECT_PARAMETER_INPUT> Inputs,
		float4_t& OutValue,
		std::string& strOutError);
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
