#include "Light.h"
#include "Light_Manager.h"
#include "Presentation_Manager.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <vector>

using namespace Engine;

static_assert(sizeof(LIGHT_DESC) == 92u);
static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);

namespace
{
	LIGHT_DESC MakeValidPointLight()
	{
		LIGHT_DESC Desc{};
		Desc.eType = LIGHT::POINT;
		Desc.vPosition = float4_t(1.f, 2.f, 3.f, 1.f);
		Desc.fRange = 10.f;
		Desc.vDiffuse = float4_t(1.f, 0.5f, 0.25f, 1.f);
		Desc.vAmbient = float4_t(0.1f, 0.1f, 0.1f, 1.f);
		Desc.vSpecular = float4_t(1.f, 1.f, 1.f, 1.f);
		return Desc;
	}

	int Fail(const char* Message)
	{
		std::cerr << "PointLightFalloffContractHarness FAIL: " << Message << '\n';
		return 1;
	}
}

int main()
{
	const LIGHT_DESC DefaultDesc{};
	if (std::bit_cast<std::uint32_t>(DefaultDesc.fFalloffExponent) !=
		std::bit_cast<std::uint32_t>(1.f))
	{
		return Fail("LIGHT_DESC default exponent is not bit-exact 1.0f");
	}

	const LIGHT_DESC Valid = MakeValidPointLight();
	auto Light = CLight::Create(Valid);
	if (nullptr == Light)
		return Fail("valid point light did not initialize");

	for (const float InvalidRange : {
		0.f, -1.f, std::numeric_limits<float>::quiet_NaN(),
		std::numeric_limits<float>::infinity() })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (nullptr != CLight::Create(Invalid))
			return Fail("CLight::Create accepted invalid point range");
		if (E_INVALIDARG != Light->Initialize(Invalid))
			return Fail("CLight::Initialize accepted invalid point range");
		if (E_INVALIDARG != CLight::Render_Desc(Invalid, nullptr, nullptr))
			return Fail("CLight::Render_Desc did not reject before shader bind");
	}

	LIGHT_DESC InvalidExponent = Valid;
	InvalidExponent.fFalloffExponent = 0.f;
	if (nullptr != CLight::Create(InvalidExponent) ||
		E_INVALIDARG != Light->Initialize(InvalidExponent) ||
		E_INVALIDARG != CLight::Render_Desc(
			InvalidExponent, nullptr, nullptr))
	{
		return Fail("final CLight boundary accepted invalid exponent");
	}

	auto LightManager = CLight_Manager::Create();
	if (nullptr == LightManager || FAILED(LightManager->Replace_SceneLights(
		std::vector<LIGHT_DESC>{ Valid, Valid })) ||
		2u != LightManager->Get_SceneLightCount())
	{
		return Fail("valid scene-light baseline was not staged");
	}

	for (const float InvalidRange : { 0.f, -1.f })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (E_INVALIDARG != LightManager->Replace_SceneLights(
			std::vector<LIGHT_DESC>{ Invalid }) ||
			2u != LightManager->Get_SceneLightCount())
		{
			return Fail("scene invalid range changed prior state");
		}
	}

	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	Presentation.Set_TransientLightsEnabled(true);
	Presentation.Clear_TransientLights();
	if (FAILED(Presentation.Add_TransientLight(Valid)) ||
		1u != Presentation.Get_TransientLights().size())
	{
		return Fail("valid transient-light baseline was not pushed");
	}

	for (const float InvalidRange : { 0.f, -1.f })
	{
		LIGHT_DESC Invalid = Valid;
		Invalid.fRange = InvalidRange;
		if (E_FAIL != Presentation.Add_TransientLight(Invalid) ||
			1u != Presentation.Get_TransientLights().size() ||
			std::bit_cast<std::uint32_t>(
				Presentation.Get_TransientLights().front().fRange) !=
			std::bit_cast<std::uint32_t>(Valid.fRange))
		{
			return Fail("transient invalid range pushed or changed prior state");
		}
	}

	if (E_FAIL != Presentation.Add_TransientLight(InvalidExponent) ||
		1u != Presentation.Get_TransientLights().size())
	{
		return Fail("transient invalid exponent pushed or changed prior state");
	}

	Presentation.Clear_TransientLights();
	std::cout <<
		"PointLightFalloffContractHarness PASS: abi/default; final-boundary; "
		"scene-rollback; transient-no-push\n";
	return 0;
}
