#include "Light.h"

#include "GameInstance.h"

CLight::CLight()
{
}

CLight::~CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
	m_LightDesc = LightDesc;

	return S_OK;
}

HRESULT CLight::Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
    uint32_t            iPassIndex = {};

    if (LIGHT::DIRECTIONAL == m_LightDesc.eType)
    {
        if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof m_LightDesc.vDirection)))
            return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::DIRECTIONAL);
    }

    else if (LIGHT::POINT == m_LightDesc.eType)
    {
        if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof m_LightDesc.vPosition)))
            return E_FAIL;
        if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof m_LightDesc.fRange)))
            return E_FAIL;

        iPassIndex = ETOUI(DEFERRED::POINT);
    }


    if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof m_LightDesc.vDiffuse)))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof m_LightDesc.vAmbient)))
        return E_FAIL;


    if (FAILED(pShader->Begin(iPassIndex)))
        return E_FAIL;

    if (FAILED(pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

unique_ptr<CLight> CLight::Create(const LIGHT_DESC& LightDesc)
{
    auto pInstance = unique_ptr<CLight>(new CLight());

    if (FAILED(pInstance->Initialize(LightDesc)))
    {
        MSG_BOX("Failed to Created : CLight");
        return nullptr;
    }

    return pInstance;
}
