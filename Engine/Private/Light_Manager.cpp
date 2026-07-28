#include "Light_Manager.h"

#include "Light.h"

CLight_Manager::CLight_Manager()
{
}

CLight_Manager::~CLight_Manager()
{
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
    auto        pLight = CLight::Create(LightDesc);
    if (nullptr == pLight)
        return E_FAIL;

    m_Lights.push_back(move(pLight));

    return S_OK;
}

HRESULT CLight_Manager::Render_Lights(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
    for (auto& pLight : m_Lights)
    {
        if (FAILED(pLight->Render(pShader, pVIBuffer)))
            return E_FAIL;
    }


    return S_OK;
}

unique_ptr<CLight_Manager> CLight_Manager::Create()
{
    return unique_ptr<CLight_Manager>(new CLight_Manager());
}
