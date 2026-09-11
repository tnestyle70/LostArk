#include "Light_Manager.h"

#include "Light.h"
#include "Presentation_Manager.h"

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
	bool_t bEnableSceneDirectionalShadow, LIGHT_RECEIVER ePassReceiver)
{
    if (ePassReceiver != LIGHT_RECEIVER::ALL && ePassReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER) return E_INVALIDARG;
	HRESULT hResult = S_OK;
	bool_t bSceneDirectionalShadowConsumed = false;
    bool_t staticShadowConsumed = false;
    for (const LIGHT_DESC& LightDesc : m_SceneLights)
    {
        if (LightDesc.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER && ePassReceiver == LIGHT_RECEIVER::ALL) continue;
		const bool_t bApplyShadow =
			bEnableSceneDirectionalShadow &&
			!bSceneDirectionalShadowConsumed &&
			LIGHT::DIRECTIONAL == LightDesc.eType;
        // Source component shadow atlases target the scene's existing main
        // directional light. Later directional/effect lights retain identity.
        const bool_t applyStaticShadow = !staticShadowConsumed && LIGHT::DIRECTIONAL == LightDesc.eType;
        if (applyStaticShadow) staticShadowConsumed = true;
		if (FAILED(CLight::Render_Desc(
			LightDesc, pShader, pVIBuffer, bApplyShadow, applyStaticShadow)))
		{
			hResult = E_FAIL;
			break;
		}
		if (bApplyShadow)
			bSceneDirectionalShadowConsumed = true;
    }
	if (SUCCEEDED(hResult))
	{
		for (const LIGHT_DESC& LightDesc :
			CPresentation_Manager::Get().Get_TransientLights())
		{
            if (LightDesc.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER && ePassReceiver == LIGHT_RECEIVER::ALL) continue;
			if (FAILED(CLight::Render_Desc(
				LightDesc, pShader, pVIBuffer, false)))
			{
				hResult = E_FAIL;
				break;
			}
		}
	}
	// Render_Lights is called for ordinary and source-character passes.
	// CRenderer::Draw owns Clear_Frame on both success and failure.
	return hResult;
}

unique_ptr<CLight_Manager> CLight_Manager::Create()
{
    return unique_ptr<CLight_Manager>(new CLight_Manager());
}
