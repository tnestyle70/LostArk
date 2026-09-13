#include "Light_Manager.h"
#include "Engine_RenderTypes.h"

#include "Light.h"
#include "Presentation_Manager.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include <array>

#include <cmath>

namespace
{
	bool_t IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	bool_t IsValidSceneLight(const LIGHT_DESC& Light)
	{
		if (Light.staticShadowChannel > 15u || (Light.eReceiver != LIGHT_RECEIVER::ALL &&
			Light.eReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER &&
			Light.eReceiver != LIGHT_RECEIVER::UNBAKED) ||
			(LIGHT::POINT != Light.eType && LIGHT::SPOT != Light.eType &&
			LIGHT::DIRECTIONAL != Light.eType) || !IsFinite4(Light.vDiffuse) ||
			!IsFinite4(Light.vAmbient) || !IsFinite4(Light.vSpecular) ||
			!std::isfinite(Light.fFalloffExponent) ||
			Light.fFalloffExponent <= 0.f ||
			Light.vDiffuse.x < 0.f || Light.vDiffuse.y < 0.f ||
			Light.vDiffuse.z < 0.f || Light.vAmbient.x < 0.f ||
			Light.vAmbient.y < 0.f || Light.vAmbient.z < 0.f ||
			Light.vSpecular.x < 0.f || Light.vSpecular.y < 0.f ||
			Light.vSpecular.z < 0.f)
		{
			return false;
		}
		if (LIGHT::POINT != Light.eType)
		{
			const float fDirectionLengthSquared =
				Light.vDirection.x * Light.vDirection.x +
				Light.vDirection.y * Light.vDirection.y +
				Light.vDirection.z * Light.vDirection.z;
			if (!IsFinite4(Light.vDirection) ||
				!std::isfinite(fDirectionLengthSquared) ||
				fDirectionLengthSquared <= 0.000001f)
				return false;
		}
		if (LIGHT::DIRECTIONAL != Light.eType &&
			(!IsFinite4(Light.vPosition) || !std::isfinite(Light.fRange) ||
			 Light.fRange <= 0.f))
			return false;
		return LIGHT::SPOT != Light.eType ||
			(std::isfinite(Light.fSpotInnerCos) &&
			 std::isfinite(Light.fSpotOuterCos) &&
			 Light.fSpotOuterCos > 0.f &&
			 Light.fSpotOuterCos <= Light.fSpotInnerCos &&
			 Light.fSpotOuterCos < 1.f && Light.fSpotInnerCos <= 1.f);
	}
}

CLight_Manager::CLight_Manager()
{
}

CLight_Manager::~CLight_Manager()
{
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	vector<LIGHT_DESC> SceneLights{ LightDesc };
	return Replace_SceneLights(std::move(SceneLights));
}

HRESULT CLight_Manager::Replace_SceneLights(vector<LIGHT_DESC> SceneLights)
{
	if (SceneLights.size() > 16u)
		return E_INVALIDARG;
	for (const LIGHT_DESC& LightDesc : SceneLights)
	{
		if (!IsValidSceneLight(LightDesc))
			return E_INVALIDARG;
	}
	m_SceneLights.swap(SceneLights);
	return S_OK;
}

HRESULT CLight_Manager::Render_Lights(
    shared_ptr<class CShader> pShader,
    shared_ptr<class CVIBuffer_Rect> pVIBuffer,
    bool_t bEnableSceneDirectionalShadow, LIGHT_RECEIVER ePassReceiver, bool_t bSourceLightMask)
{
    if (!pShader || !pVIBuffer || (ePassReceiver != LIGHT_RECEIVER::ALL &&
        ePassReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER)) return E_INVALIDARG;

    // Scene admission permits 16 and Presentation_Manager permits 384 transient lights.
    // Keep the bounded GPU record array and CPU staging contract together.
    constexpr uint32_t maximumLightInstances = 400u;
    struct LightInstance
    {
        float4_t direction, position, diffuse, ambient, specular, attenuation;
        uint32_t flags[4]; // receiver, static channel, directional shadow, source local bounds
    };
    static_assert(sizeof(LightInstance) == 112u);
    std::array<LightInstance, maximumLightInstances> instances;
    std::array<LIGHT, maximumLightInstances> types;
    uint32_t count = 0u;
    bool_t directionalShadowConsumed = false;
    bool_t staticShadowConsumed = false;
    const auto stage = [&](const LIGHT_DESC& light, bool_t scene) -> HRESULT
    {
        if (light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER &&
            ePassReceiver == LIGHT_RECEIVER::ALL) return S_OK;
        if (!CLight::Is_ValidDesc(light) || count >= maximumLightInstances) return E_INVALIDARG;
        const bool_t directional = light.eType == LIGHT::DIRECTIONAL;
        const bool_t shadow = scene && directional && bEnableSceneDirectionalShadow && !directionalShadowConsumed;
        const bool_t staticShadow = scene && directional && !staticShadowConsumed;
        auto& record = instances[count];
        record.direction = light.vDirection;
        record.position = light.vPosition;
        record.diffuse = light.vDiffuse;
        record.ambient = light.vAmbient;
        record.specular = light.vSpecular;
        record.attenuation = float4_t(light.fRange, light.fFalloffExponent,
            light.fSpotInnerCos, light.fSpotOuterCos);
        record.flags[0] = static_cast<uint32_t>(light.eReceiver);
        record.flags[1] = light.staticShadowChannel != 0u ? light.staticShadowChannel : (staticShadow ? 1u : 0u);
        record.flags[2] = shadow ? 1u : 0u;
        record.flags[3] = !directional && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ? 1u : 0u;
        types[count++] = light.eType;
        if (shadow) directionalShadowConsumed = true;
        if (staticShadow) staticShadowConsumed = true;
        return S_OK;
    };
    for (const auto& light : m_SceneLights)
        if (FAILED(stage(light, true))) return E_FAIL;
    for (const auto& light : CPresentation_Manager::Get().Get_TransientLights())
        if (FAILED(stage(light, false))) return E_FAIL;
    if (count == 0u) return S_OK;
    if (FAILED(pShader->Bind_RawValue("g_LightInstances", instances.data(),
        sizeof(LightInstance) * count))) return E_FAIL;

    const bool_t sourceMask = bSourceLightMask && ePassReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER;
    // Append-only shader pass contract: old 0–21, instance directional/point/spot 22–24,
    // source-stencil variants 25–27. Never reorder types or combine their FP16 sums.
    for (uint32_t first = 0u; first < count;)
    {
        uint32_t end = first + 1u;
        while (end < count && types[end] == types[first]) ++end;
        const uint32_t typePass = types[first] == LIGHT::DIRECTIONAL ? 0u :
            types[first] == LIGHT::POINT ? 1u : 2u;
        const uint32_t pass = 22u + typePass + (sourceMask ? 3u : 0u);
        if (FAILED(pShader->Bind_RawValue("g_LightInstanceOffset", &first, sizeof(first))) ||
            FAILED(pShader->Begin(pass)) ||
            FAILED(pVIBuffer->Render_Instanced(end - first))) return E_FAIL;
        first = end;
    }
    return S_OK;
}

unique_ptr<CLight_Manager> CLight_Manager::Create()
{
    return unique_ptr<CLight_Manager>(new CLight_Manager());
}
