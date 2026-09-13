#include "Effect_LightPresentation.h"
#include "Engine_RenderTypes.h"

#include <cmath>

namespace
{
	bool_t IsFinite3(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}
}

bool_t Client::Try_BuildEffectLightDesc(
	const EFFECT_EVALUATED_LIGHT& Evaluated,
	LIGHT_DESC& OutLight)
{
	if (!IsFinite3(Evaluated.vWorldPosition) ||
		!std::isfinite(Evaluated.fRange) || Evaluated.fRange <= 0.f ||
		!std::isfinite(Evaluated.fIntensity) || Evaluated.fIntensity < 0.f ||
		!IsFinite4(Evaluated.vColor) || !IsFinite4(Evaluated.vAmbient) ||
		!std::isfinite(Evaluated.fSpecularIntensity) || Evaluated.fSpecularIntensity < 0.f ||
		!std::isfinite(Evaluated.fFalloffExponent) ||
		Evaluated.fFalloffExponent <= 0.f)
	{
		return false;
	}

	LIGHT_DESC Staged{};
	switch (Evaluated.eProfile)
	{
	case EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1: Staged.eType = LIGHT::POINT; break;
	case EFFECT_LIGHT_PROFILE::SPOT_RECONSTRUCTED_V1: Staged.eType = LIGHT::SPOT; break;
	case EFFECT_LIGHT_PROFILE::DIRECTIONAL_RECONSTRUCTED_V1: Staged.eType = LIGHT::DIRECTIONAL; break;
	default: return false;
	}
	if (Staged.eType != LIGHT::POINT)
	{
		const float3_t& Direction = Evaluated.vWorldDirection;
		const f32_t LengthSquared = Direction.x * Direction.x +
			Direction.y * Direction.y + Direction.z * Direction.z;
		if (!IsFinite3(Direction) || !std::isfinite(LengthSquared) || LengthSquared <= 0.000001f)
			return false;
		const f32_t InverseLength = 1.f / std::sqrt(LengthSquared);
		Staged.vDirection = { Direction.x * InverseLength,
			Direction.y * InverseLength, Direction.z * InverseLength, 0.f };
	}
	if (Staged.eType == LIGHT::SPOT)
	{
		const f32_t Inner = Evaluated.fInnerConeDegrees;
		const f32_t Outer = Evaluated.fOuterConeDegrees;
		if (!std::isfinite(Inner) || !std::isfinite(Outer) ||
			Inner < 0.f || Outer <= 0.f || Inner > Outer || Outer >= 90.f)
			return false;
		Staged.fSpotInnerCos = std::cos(DirectX::XMConvertToRadians(Inner));
		Staged.fSpotOuterCos = std::cos(DirectX::XMConvertToRadians(Outer));
		if (Staged.fSpotOuterCos <= 0.f || Staged.fSpotOuterCos >= 1.f)
			return false;
	}
	Staged.vPosition = {
		Evaluated.vWorldPosition.x,
		Evaluated.vWorldPosition.y,
		Evaluated.vWorldPosition.z,
		1.f };
	Staged.fRange = Evaluated.fRange;
	Staged.fFalloffExponent = Evaluated.fFalloffExponent;
	Staged.vDiffuse = {
		Evaluated.vColor.x * Evaluated.fIntensity,
		Evaluated.vColor.y * Evaluated.fIntensity,
		Evaluated.vColor.z * Evaluated.fIntensity,
		Evaluated.vColor.w };
	Staged.vAmbient = {
		Evaluated.vAmbient.x * Evaluated.fIntensity,
		Evaluated.vAmbient.y * Evaluated.fIntensity,
		Evaluated.vAmbient.z * Evaluated.fIntensity,
		Evaluated.vAmbient.w };
	Staged.vSpecular = { 0.f, 0.f, 0.f, 0.f };
	if (Evaluated.fSpecularIntensity > 0.f)
	{
		Staged.vSpecular = { Staged.vDiffuse.x * Evaluated.fSpecularIntensity,
			Staged.vDiffuse.y * Evaluated.fSpecularIntensity,
			Staged.vDiffuse.z * Evaluated.fSpecularIntensity, 0.f };
	}
	if (!IsFinite4(Staged.vDiffuse) || !IsFinite4(Staged.vAmbient) || !IsFinite4(Staged.vSpecular))
		return false;

	OutLight = Staged;
	return true;
}

bool_t Client::Try_BuildEffectPointLightDesc(
	const EFFECT_EVALUATED_LIGHT& Evaluated,
	LIGHT_DESC& OutLight)
{
	return Evaluated.eProfile == EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1 &&
		Try_BuildEffectLightDesc(Evaluated, OutLight);
}
