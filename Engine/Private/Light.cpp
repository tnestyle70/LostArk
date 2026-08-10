#include "Light.h"

#include "GameInstance.h"

#include <cmath>

namespace
{
	bool_t IsValidLightAttenuation(const LIGHT_DESC& LightDesc)
	{
		if (!std::isfinite(LightDesc.fFalloffExponent) ||
			LightDesc.fFalloffExponent <= 0.f)
		{
			return false;
		}
		return LIGHT::POINT != LightDesc.eType ||
			(std::isfinite(LightDesc.fRange) && LightDesc.fRange > 0.f);
	}
}

CLight::CLight()
{
}

CLight::~CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
	if (!IsValidLightAttenuation(LightDesc))
		return E_INVALIDARG;

	m_LightDesc = LightDesc;

	return S_OK;
}

HRESULT CLight::Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	return Render_Desc(
		m_LightDesc, std::move(pShader), std::move(pVIBuffer), false);
}

HRESULT CLight::Render_Desc(
	const LIGHT_DESC& LightDesc,
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer,
	bool_t bApplyDirectionalShadow)
{
	if (!IsValidLightAttenuation(LightDesc))
		return E_INVALIDARG;

    uint32_t            iPassIndex = {};

    if (LIGHT::DIRECTIONAL == LightDesc.eType)
    {
		const uint32_t iApplyDirectionalShadow =
			bApplyDirectionalShadow ? 1u : 0u;
        if (FAILED(pShader->Bind_RawValue("g_vLightDir", &LightDesc.vDirection, sizeof LightDesc.vDirection)))
            return E_FAIL;
		if (FAILED(pShader->Bind_RawValue(
			"g_iApplyDirectionalShadow", &iApplyDirectionalShadow,
			sizeof(iApplyDirectionalShadow))))
			return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::DIRECTIONAL);
    }

    else if (LIGHT::POINT == LightDesc.eType)
    {
		const uint32_t iApplyDirectionalShadow = 0u;
		if (FAILED(pShader->Bind_RawValue(
			"g_iApplyDirectionalShadow", &iApplyDirectionalShadow,
			sizeof(iApplyDirectionalShadow))))
			return E_FAIL;
        if (FAILED(pShader->Bind_RawValue("g_vLightPos", &LightDesc.vPosition, sizeof LightDesc.vPosition)))
            return E_FAIL;
        if (FAILED(pShader->Bind_RawValue("g_fLightRange", &LightDesc.fRange, sizeof LightDesc.fRange)))
            return E_FAIL;
		if (FAILED(pShader->Bind_RawValue(
			"g_fLightFalloffExponent", &LightDesc.fFalloffExponent,
			sizeof LightDesc.fFalloffExponent)))
			return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::POINT);
    }


    if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &LightDesc.vDiffuse, sizeof LightDesc.vDiffuse)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &LightDesc.vAmbient, sizeof LightDesc.vAmbient)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &LightDesc.vSpecular, sizeof LightDesc.vSpecular)))
        return E_FAIL;


    if (FAILED(pShader->Begin(iPassIndex)))
        return E_FAIL;

    if (FAILED(pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

unique_ptr<CLight> CLight::Create(const LIGHT_DESC& LightDesc)
{
	if (!IsValidLightAttenuation(LightDesc))
		return nullptr;

    auto pInstance = unique_ptr<CLight>(new CLight());

    if (FAILED(pInstance->Initialize(LightDesc)))
    {
        MSG_BOX("Failed to Created : CLight");
        return nullptr;
    }

    return pInstance;
}
