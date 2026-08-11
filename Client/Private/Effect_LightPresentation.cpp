#include "Effect_LightPresentation.h"

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

bool_t Client::Try_BuildEffectPointLightDesc(
	const EFFECT_EVALUATED_LIGHT& Evaluated,
	LIGHT_DESC& OutLight)
{
	if (!IsFinite3(Evaluated.vWorldPosition) ||
		!std::isfinite(Evaluated.fRange) || Evaluated.fRange <= 0.f ||
		!std::isfinite(Evaluated.fIntensity) || Evaluated.fIntensity < 0.f ||
		!IsFinite4(Evaluated.vColor) || !IsFinite4(Evaluated.vAmbient) ||
		!std::isfinite(Evaluated.fFalloffExponent) ||
		Evaluated.fFalloffExponent <= 0.f)
	{
		return false;
	}

	LIGHT_DESC Staged{};
	Staged.eType = LIGHT::POINT;
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
	if (!IsFinite4(Staged.vDiffuse) || !IsFinite4(Staged.vAmbient))
		return false;

	OutLight = Staged;
	return true;
}
